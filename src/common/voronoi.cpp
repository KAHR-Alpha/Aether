/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <voronoi.h>
#include <bitmap3.h>

//#define VORONOI_DEBUG

extern std::ofstream plog;

//################
//   EventQueue
//################

int FortuneVoronoi::EventQueue::add_circle_event(Event &c_event)
{
    unsigned int i,N=events.size();
    
    // Processing
    
    c_event.ID=ID_counter;
    ID_counter++;
    
    unsigned int k=N;
    
    for(i=0;i<N;i++)
    {
        if(c_event<events[i])
        {
            k=i;
            break;
        }
    }
    
    if(k==N) events.push_back(c_event);
    else
    {
        events.resize(N+1);
        
        for(i=N;i>k;i--)
            events[i]=events[i-1];
        
        events[k]=c_event;
    }
    
    return c_event.ID;
}

void FortuneVoronoi::EventQueue::forget_event(int ID)
{
    unsigned int i,k=events.size();
    
    for(i=0;i<events.size();i++)
    {
        if(events[i].ID==ID)
        {
            k=i;
            break;
        }
    }
    
    if(k==events.size()) return;
    
    for(i=k;i<events.size()-1;i++)
        events[i]=events[i+1];
    
    events.resize(events.size()-1);
}

bool FortuneVoronoi::EventQueue::is_empty()
{
    return events.size()==0;
}

FortuneVoronoi::Event FortuneVoronoi::EventQueue::next_event()
{
    Event event_out=events[0];
    
    for(unsigned int i=0;i<events.size()-1;i++)
        events[i]=events[i+1];
    
    events.resize(events.size()-1);
    
    return event_out;
}

void FortuneVoronoi::EventQueue::resize(int N_sites)
{
    events.resize(N_sites);
}

void FortuneVoronoi::EventQueue::set_site_event(int i,double x,double y)
{
    events[i].ID=ID_counter;
    ID_counter++;
    
    events[i].site=i;
    events[i].x=x;
    events[i].y=y;
    events[i].circle_event=false;
}

void FortuneVoronoi::EventQueue::sort_site_events()
{
    std::sort(events.begin(),events.end());
}

//####################
//   FortuneVoronoi
//####################

double FortuneVoronoi::parabola_intersection(Site const &A,Site const &B,double l,bool pos_value)
{   
    double Ax=A.x, Ay=A.y;
    double Bx=B.x, By=B.y;
    
    const double eps=std::numeric_limits<double>::epsilon();
    
         if(Ay==l) return Ax;
    else if(By==l) return Bx;
    else if(Ay==By) return 0.5*(Ax+Bx);
    
//         if(std::abs(Ay-l)/std::max(std::abs(Ay),std::abs(l))<eps) return Ax;
//    else if(std::abs(By-l)/std::max(std::abs(By),std::abs(l))<eps) return Bx;
//    else if(std::abs(Ay-By)/std::max(std::abs(Ay),std::abs(By))<eps) return 0.5*(Ax+Bx);
    
    double a=By-Ay;
    double b=2*(Bx*(Ay-l)-Ax*(By-l));
    double c=(By-l)*(Ax*Ax+Ay*Ay-l*l)-(Ay-l)*(Bx*Bx+By*By-l*l);
    
    double d=b*b-4*a*c;
    
    #ifdef VORONOI_DEBUG
    std::cout<<"\n";
//    chk_var(Ax);
//    chk_var(Bx);
//    chk_var(Ay);
//    chk_var(By);
//    chk_var(l);
//    chk_var(Ay-l);
////    chk_var(std::abs(Ay-l)/std::max(std::abs(Ay),std::abs(l)));
//    chk_var(By-l);
////    chk_var(std::abs(By-l)/std::max(std::abs(By),std::abs(l)));
//    chk_var(a);
//    chk_var(b);
//    chk_var(c);
    chk_var(d/(b*b));
    chk_var(d);
    chk_var(std::numeric_limits<double>::epsilon());
    chk_var(pos_value);
    #endif
    
    if(std::abs(d/(b*b))<100.0*std::numeric_limits<double>::epsilon()) d=0;
    
    if(d<0) return 0;
    
    double x1=std::sqrt(d);
    
    double x2=(-b+x1)/(2.0*a);
           x1=(-b-x1)/(2.0*a);
    
    if(x1>x2) std::swap(x1,x2);
    
    if(pos_value) return x2;
    else return x1;
}

double FortuneVoronoi::breakpoints_span(Site const &S1,Site const &S2,Site const &S3,double l,bool pos_val_1,bool pos_val_2)
{
    double i1=parabola_intersection(S1,S2,l,pos_val_1);
    double i2=parabola_intersection(S2,S3,l,pos_val_2);
    
//    chk_var(i1);
//    chk_var(i2);
    
//    chk_var((i1-bx_min)/(bx_max-bx_min));
//    chk_var((i2-bx_min)/(bx_max-bx_min));
    
    return i2-i1;
}

bool FortuneVoronoi::closing_breakpoints(Site const &S1,Site const &S2,Site const &S3,double l,bool pos_val_1,bool pos_val_2,double delta)
{
    
    double span_1=breakpoints_span(S1,S2,S3,l,pos_val_1,pos_val_2);
    double span_2=breakpoints_span(S1,S2,S3,l-delta,pos_val_1,pos_val_2);
    
//    chk_var(pos_val_1);
//    chk_var(pos_val_2);
//    chk_var(span_1);
//    chk_var(span_2);
    
    if(sgn(span_1)!=sgn(span_2) || std::abs(span_2)<std::abs(span_1)) return true;
    
    return false;
}

bool in_boundary(Vertex const &V,double x_min,double x_max,double y_min,double y_max)
{
    if(V.loc.x<x_min || V.loc.x>x_max || V.loc.y<y_min || V.loc.y>y_max) return false;
    
    return true;
}

template<typename T>
T mod(T val,T val_mod)
{
    while(val<0) val+=val_mod;
    while(val>=val_mod) val-=val_mod;
    
    return val;
}

void FortuneVoronoi::boundary_intersection(double x_min,double x_max,double y_min,double y_max)
{
    Vector3 P_avg;
    
    for(unsigned int i=0;i<sites.size();i++)
    {
        P_avg.x+=sites[i].x;
        P_avg.y+=sites[i].y;
    }
    
    P_avg/=sites.size();
        
    std::vector<BoxCross> box_cross;
    
    chk_var(box_cross.size());
    
    chk_var(points.size());

    for(unsigned int i=0;i<points.size();i++)
    {
        Site &S1=sites[points[i].site_prev];
        Site &S2=sites[points[i].site_next];
        
        Vector3 P1(S1.x,S1.y,0);
        Vector3 P2(S2.x,S2.y,0);
        
        Vector3 Pc=0.5*(P1+P2);
        Vector3 v=P1-Pc;
        
        std::swap(v.x,v.y);
        v.y=-v.y;
        
        if(scalar_prod(v,P_avg-Pc)>=0) v=-v;
        
        v.normalize();
        
        // Generating new vertices way outsite the box
        
        Vertex tmp_vertex;
        
        double t=10*std::sqrt((bx_max-bx_min)*(bx_max-bx_min)+(by_max-by_min)*(by_max-by_min));
        
        tmp_vertex.loc=Pc+v*t;
        v_arr->push_back(tmp_vertex);
        
        (*polys)[points[i].site_prev].add_vertex(current_vertex);
        (*polys)[points[i].site_next].add_vertex(current_vertex);
        
        current_vertex++;
    }
    
//    boundary_vertex_generation(box_cross);
    
    box_cross.clear();
    
    for(unsigned int i=0;i<polys->size();i++)
    {
        Polygon2D &poly=(*polys)[i];
        
        poly.reorder_vertices(*v_arr);
        
        for(int j=0;j<poly.N;j++)
        {
            Vertex &V=(*v_arr)[poly.vertex_index[j]];
            
            if(!in_boundary(V,x_min,x_max,y_min,y_max))
            {
                Vertex &Vm=(*v_arr)[poly.vertex_index[mod(j-1,poly.N)]];
                Vertex &Vp=(*v_arr)[poly.vertex_index[mod(j+1,poly.N)]];
                
                if(in_boundary(Vm,x_min,x_max,y_min,y_max))
                {
                    Vector3 v=V.loc-Vm.loc;
                    v.normalize();
                    
                    BoxCross c(i,Vm.loc,v);
                    box_cross.push_back(c);
                }
                
                if(in_boundary(Vp,x_min,x_max,y_min,y_max))
                {
                    Vector3 v=V.loc-Vp.loc;
                    v.normalize();
                    
                    BoxCross c(i,Vp.loc,v);
                    box_cross.push_back(c);
                }
            }
        }
        
        bool run=true;
        
        while(run)
        {
            bool mod=false;
            
            for(int j=0;j<poly.N;j++)
            {
                Vertex &V=(*v_arr)[poly.vertex_index[j]];
                
                if(!in_boundary(V,x_min,x_max,y_min,y_max))
                {
                    poly.forget_vertex(j);
                    mod=true;
                    
                    break;
                }
            }
            
            run=mod;
        }
    }
    
    boundary_vertex_generation(box_cross);
    
    for(unsigned int i=0;i<polys->size();i++)
    {
        Polygon2D &poly=(*polys)[i];
        poly.reorder_vertices(*v_arr);
    }
}

void FortuneVoronoi::boundary_vertex_generation(std::vector<BoxCross> const &box_cross)
{
    class CornerCheck
    {
        public:
            int poly;
            bool x1,x2,y1,y2;
            
            CornerCheck()
                :x1(false), x2(false), y1(false), y2(false)
            {}
            
            void set(int i)
            {
                     if(i==0) x1=true;
                else if(i==1) x2=true;
                else if(i==2) y1=true;
                else if(i==3) y2=true;
            }
            
            bool is_corner()
            {
                return (x1 && (y1 || y2)) || (x2 && (y1 || y2));
            }
    };
    
    std::vector<double> t(4);
    std::vector<CornerCheck> corner_check;
    
    for(unsigned int i=0;i<box_cross.size();i++)
    {
        int const &poly=box_cross[i].poly;
        Vector3 const &P=box_cross[i].P;
        Vector3 const &v=box_cross[i].v;
        
        if(v.x==0) { t[0]=t[1]=-1; }
        else
        {
            t[0]=(bx_min-P.x)/v.x;
            t[1]=(bx_max-P.x)/v.x;
        }
        
        if(v.y==0) { t[2]=t[3]=-1; }
        else
        {
            t[2]=(by_min-P.y)/v.y;
            t[3]=(by_max-P.y)/v.y;
        }
        
        double t_min=std::numeric_limits<double>::max();
        int k_min=-1;
        
        for(int j=0;j<4;j++)
        {
            if(t[j]>=0 && t[j]<=t_min)
            {
                t_min=t[j];
                k_min=j;
            }
        }
        
        Vertex tmp_vertex;
        tmp_vertex.loc=P+v*t_min;
        v_arr->push_back(tmp_vertex);
        (*polys)[poly].add_vertex(current_vertex);
        
        current_vertex++;
        
        bool unknown_corner=true;
        for(unsigned int j=0;j<corner_check.size();j++)
        {
            if(corner_check[j].poly==poly)
            {
                corner_check[j].set(k_min);
                unknown_corner=false;
                break;
            }
        }
        
        if(unknown_corner)
        {
            CornerCheck c;
            c.poly=poly; c.set(k_min);
            
            corner_check.push_back(c);
        }
    }
    
    for(unsigned int i=0;i<corner_check.size();i++)
    {
        if(corner_check[i].is_corner())
        {
            Vertex tmp_vertex;
            
            if(corner_check[i].x1) tmp_vertex.loc.x=bx_min;
            else tmp_vertex.loc.x=bx_max;
            
            if(corner_check[i].y1) tmp_vertex.loc.y=by_min;
            else tmp_vertex.loc.y=by_max;
            
            v_arr->push_back(tmp_vertex);
            (*polys)[corner_check[i].poly].add_vertex(current_vertex);
            
            current_vertex++;
        }
    }
}

int FortuneVoronoi::event_to_arc(int event_ID)
{
    for(std::size_t i=0;i<arcs.size();i++)
    {
        if(arcs[i].event_ID==event_ID) return i;
    }
    return -1;
}

void FortuneVoronoi::generate(std::vector<Vertex> *v_arr_,
                              std::vector<Polygon2D> *polys_,
                              std::vector<Vector3> const &sites_,
                              double x_min,double x_max,double y_min,double y_max)
{
    bx_min=x_min;
    bx_max=x_max;
    
    by_min=y_min;
    by_max=y_max;
    
    // Initialization
    
    v_arr=v_arr_;
    v_arr->clear();
    
    current_vertex=0;
    
    polys=polys_;
    for(unsigned int i=0;i<polys->size();i++)
    {
        (*polys)[i].clear();
    }
    
    int N=sites_.size();
    
    sites.resize(N);
    queue.resize(N);
    
    for(int i=0;i<N;i++)
    {
        sites[i].x=sites_[i].x;
        sites[i].y=sites_[i].y;
    }
    
    for(int i=0;i<N;i++)
    {
        double x=sites[i].x;
        double y=sites[i].y;
        
//        std::cout<<x<<" "<<y<<std::endl;
        
        queue.set_site_event(i,x,y);
    }
    
    queue.sort_site_events();
    
    ProgTimeDisp dsp(queue.events.size());
    int it=0;
    
    #ifdef VORONOI_DEBUG
    for(int k=0;k<2000;k++)
    {
        double y=y_max+(y_min-y_max)*k/1999.0;
        
//        print_parabolas(700,"tmp_2/v_"+std::to_string(k)+".png",y);
    }
    #endif
    
    while(!queue.is_empty())
    {
        Event event=queue.next_event();
        #ifdef VORONOI_DEBUG
        std::cout<<"__________________________________"<<std::endl;
        std::cout<<event.circle_event<<" "<<event.site<<" "<<(sites[event.site].x-bx_min)/(bx_max-bx_min)<<" "<<(sites[event.site].y-by_min)/(by_max-by_min)<<" "<<event.x<<" "<<event.y<<std::endl<<std::endl;
        
//        std::cout<<"arcs ";
//        for(std::size_t i=0;i<arcs.size();i++)
//        {
//            std::cout<<arcs[i].site<<" ";
//        }
//        std::cout<<std::endl;

        std::cout<<"points ";
        for(std::size_t i=0;i<points.size();i++)
        {
            std::cout<<points[i].site_prev<<points[i].site_next<<" ";
        }
        std::cout<<std::endl;
        #endif
        
        if(event.circle_event)
        {
            bool valid_event=true;
            
            for(std::size_t i=0;i<queue.events.size();i++)
            {
                if(queue.events[i].site==event.site)
                {
                    int k1=event_to_arc(event.ID);
                    int k2=event_to_arc(queue.events[i].ID);
                    
                    if(k2>=0)
                    {
                        int s11=points[k1-1].site_prev;
                        int s12=points[k1-1].site_next;
                        int s13=points[k1  ].site_next;
                        
                        int s21=points[k2-1].site_prev;
                        int s22=points[k2-1].site_next;
                        int s23=points[k2  ].site_next;
                        
                        if((s11==s21 && s13==s23) || (s11==s23 && s13==s21))
                        {
//                            std::cout<<"Duplicate check"<<std::endl;
//                            std::cout<<s11<<" "<<s12<<" "<<s13<<std::endl;
//                            std::cout<<s21<<" "<<s22<<" "<<s23<<std::endl;
                            
                            double span1=breakpoints_span(sites[s11],sites[s12],sites[s13],event.y,
                                                          points[k1-1].pos_value,
                                                          points[k1  ].pos_value);
                            
                            double span2=breakpoints_span(sites[s21],sites[s22],sites[s23],event.y,
                                                          points[k2-1].pos_value,
                                                          points[k2  ].pos_value);
                            
                            if(std::abs(span2)<=std::abs(span1)) valid_event=false;
                            else queue.forget_event(queue.events[i].ID);
                            
                            break;
                        }
                    }
                }
            }
            
            #ifdef VORONOI_DEBUG
            chk_var(valid_event);
            #endif
            if(valid_event)
            {
                #ifdef VORONOI_DEBUG
                std::cout<<event.circ_ps<<" "<<event.site<<" "<<event.circ_ns<<std::endl;
                #endif
                process_circle_event(event);
            }
        }
        else
        {
            if(arcs.size()==0)
            {
                Arc arc;
                
                arc.site=event.site;
                arc.event_ID=-1;
                
                arcs.push_back(arc);
            }
            else
            {
                process_site_event(event);
            }
        }
        
//        print(500,"test_v_"+std::to_string(it)+".png");
        
        ++it;
        #ifndef VORONOI_DEBUG
        dsp.set_end(it+queue.events.size());
        
        ++dsp;
        #endif
    }
    
    boundary_intersection(x_min,x_max,y_min,y_max);
    
    for(int i=0;i<N;i++)
        (*polys)[i].reorder_vertices(*v_arr);
}

void FortuneVoronoi::print(int Nx,std::string const &fname)
{
    int N=sites.size();
    
    int Ny=nearest_integer(Nx*(by_max-by_min)/(bx_max-bx_min));
    
    Bitmap img(Nx,Ny);
    
    for(int i=0;i<N;i++)
    {
        int Np=(*polys)[i].N;
                
        double x=sites[i].x;
        double y=sites[i].y;
        
        x=(x-bx_min)/(bx_max-bx_min);
        y=(y-by_min)/(by_max-by_min);
        
        x=(0.05+0.9*x)*Nx;
        y=(0.05+0.9*y)*Ny;
        
        img.draw_line(x-5,y,x+5,y,1,1,1);
        img.draw_line(x,y-5,x,y+5,1,1,1);
        
        for(int j=0;j<Np;j++)
        {
            double x1=(*v_arr)[(*polys)[i].vertex_index[j]].loc.x;
            double y1=(*v_arr)[(*polys)[i].vertex_index[j]].loc.y;
            
            double x2=(*v_arr)[(*polys)[i].vertex_index[(j+1)%Np]].loc.x;
            double y2=(*v_arr)[(*polys)[i].vertex_index[(j+1)%Np]].loc.y;
            
            x1=(0.05+0.9*(x1-bx_min)/(bx_max-bx_min))*Nx;
            x2=(0.05+0.9*(x2-bx_min)/(bx_max-bx_min))*Nx;
            y1=(0.05+0.9*(y1-by_min)/(by_max-by_min))*Ny;
            y2=(0.05+0.9*(y2-by_min)/(by_max-by_min))*Ny;
            
            img.draw_line(x1,y1,x2,y2,1,0,0);
        }
    }
    
    img.write(fname);
}

void FortuneVoronoi::print_parabolas(int Nx,std::string const &fname,double y_scan)
{
    int N=sites.size();
    
    int Ny=nearest_integer(Nx*(by_max-by_min)/(bx_max-bx_min));
    
    Bitmap img(Nx,Ny);
    
    for(int i=0;i<N;i++)
    {
        double xs=sites[i].x;
        double ys=sites[i].y;
        
        double x=(xs-bx_min)/(bx_max-bx_min);
        double y=(ys-by_min)/(by_max-by_min);
        
        x=x*Nx;
        y=y*Ny;
        
        img.draw_line(x-5,y,x+5,y,1,1,1);
        img.draw_line(x,y-5,x,y+5,1,1,1);
        
        if(y_scan<=ys) for(int m=0;m<Nx;m++)
        {
            double x_scan=bx_min+(bx_max-bx_min)*m/(Nx-1.0);
            double yp=((xs-x_scan)*(xs-x_scan)-y_scan*y_scan+ys*ys)/(2.0*(ys-y_scan));
            
            int n=static_cast<int>(Ny*(yp-by_min)/(by_max-by_min));
            
            if(n>=0 && n<Ny) img.set(m,n,1.0,0,0);
        }
    }
    
    img.draw_line(0,Ny*(y_scan-by_min)/(by_max-by_min),Nx,Ny*(y_scan-by_min)/(by_max-by_min),1,1,1);
    
    img.write(fname);
}

void FortuneVoronoi::process_circle_event(Event const &event)
{
    unsigned k=arcs.size();
    
    for(unsigned int i=0;i<k;i++)
    {
        if(arcs[i].event_ID==event.ID)
        {
            k=i;
            break;
        }
    }
    
    if(k<1 || k>=arcs.size())
    {
        std::cout<<"circle error"<<std::endl;
        std::exit(0);
    }
    
    int s1=arcs[k-1].site;
    int s2=arcs[k].site;
    int s3=arcs[k+1].site;
    
    (*polys)[s1].add_vertex(current_vertex);
    (*polys)[s2].add_vertex(current_vertex);
    (*polys)[s3].add_vertex(current_vertex);
    
    Vertex tmp_vertex;
    
    tmp_vertex.loc.x=event.circ_x;
    tmp_vertex.loc.y=event.circ_y;
    tmp_vertex.loc.z=0;
    
    v_arr->push_back(tmp_vertex);
    
    current_vertex++;
    
    queue.forget_event(arcs[k-1].event_ID);
    queue.forget_event(arcs[k+1].event_ID);
    
    arcs[k-1].event_ID=-1;
    arcs[k+1].event_ID=-1;
    
    // Updating breakpoints
    
    double x=parabola_intersection(sites[arcs[k-1].site],sites[arcs[k].site],
                                   event.y,points[k-1].pos_value);
    
    double x1=parabola_intersection(sites[arcs[k-1].site],sites[arcs[k+1].site],
                                    event.y,false);
    
    double x2=parabola_intersection(sites[arcs[k-1].site],sites[arcs[k+1].site],
                                    event.y,true);
    
    
    #ifdef VORONOI_DEBUG
    std::cout<<"points B ";
    for(std::size_t i=0;i<points.size();i++)
    {
        std::cout<<points[i].site_prev<<points[i].site_next<<" ";
    }
    std::cout<<std::endl;
    #endif
    
    points[k-1].site_prev=arcs[k-1].site;
    points[k-1].site_next=arcs[k+1].site;
    points[k-1].pos_value=(std::abs(x-x2)<=std::abs(x-x1));
    
    // New circle events
    
    #ifdef VORONOI_DEBUG
    std::cout<<"points C ";
    for(std::size_t i=0;i<points.size();i++)
    {
        std::cout<<points[i].site_prev<<points[i].site_next<<" ";
    }
    std::cout<<std::endl;
    #endif
    
    if(k>=2 && k+1<arcs.size())
        predict_circle_event(arcs[k-1],points[k-2],points[k-1],event.y,s1,s2,s3);
    
    if(k>=1 && k+2<arcs.size())
        predict_circle_event(arcs[k+1],points[k-1],points[k+1],event.y,s1,s2,s3);
    
    std::vector<Arc>::const_iterator it=arcs.begin()+k;
    arcs.erase(it);
    
    std::vector<Breakpoint>::const_iterator pit=points.begin()+k;
    points.erase(pit);
    
    #ifdef VORONOI_DEBUG
    std::cout<<"points D ";
    for(std::size_t i=0;i<points.size();i++)
    {
        std::cout<<points[i].site_prev<<points[i].site_next<<" ";
    }
    std::cout<<std::endl;
    #endif
}

double FortuneVoronoi::breakpoint_x(int i,double y)
{
    return parabola_intersection(sites[points[i].site_prev],
                                 sites[points[i].site_next],y,points[i].pos_value);
}

void FortuneVoronoi::process_site_event(Event const &event)
{
    unsigned int k=arcs.size()-1;
    
    // Determining which arc the site belongs
    // by looking at intervals
    
    double prev_x=bx_min,next_x;
    
    for(unsigned int i=0;i<=points.size();i++)
    {
        if(i<points.size()) next_x=std::min(bx_max,breakpoint_x(i,event.y));
        else next_x=bx_max;
        
        if(event.x<=next_x)
        {
            double s1=std::abs(event.x-prev_x);
            double s2=std::abs(event.x-next_x);
            double s=std::abs(next_x-prev_x);
            
            // Shift to avoid calculations errors
            
            if(s1<0.01*s) sites[event.site].x+=(1e-3+randp(0.01))*s;
            else if(s2<0.01*s) sites[event.site].x-=(1e-3+randp(0.01))*s;
            
            k=i;
            break;
        }
        else prev_x=std::max(bx_min,next_x);
    }
    
    #ifdef VORONOI_DEBUG
    std::cout<<"Target arc "<<k<<"\n";
    #endif
                    
    // Arc division
    
    queue.forget_event(arcs[k].event_ID);
    
    arcs[k].event_ID=-1;
    
    arcs.resize(arcs.size()+2);
    
    for(unsigned int i=arcs.size()-1;i>=k+2;i--)
        arcs[i]=arcs[i-2];
    
    arcs[k+1].site=event.site;
    arcs[k+1].event_ID=-1;
    
    arcs[k+2]=arcs[k];
    
    // Centering
    
    k++;
    
    // Breakpoints creation
    
    points.resize(points.size()+2);
    
    for(unsigned int i=points.size()-1;i>=k+1;i--)
        points[i]=points[i-2];
    
    points[k-1].site_prev=arcs[k-1].site;
    points[k-1].site_next=arcs[k].site;
    points[k-1].pos_value=false;
    
    points[k].site_prev=arcs[k].site;
    points[k].site_next=arcs[k-1].site;
    points[k].pos_value=true;
    
    // Circle events identification
    
    #ifdef VORONOI_DEBUG
        chk_var(k);
        chk_var(arcs.size()-1);
    #endif
    
    if(k>=2)
        predict_circle_event(arcs[k-1],points[k-2],points[k-1],event.y,-1,-1,-1);
    
    if(k+2<=arcs.size()-1)
        predict_circle_event(arcs[k+1],points[k],points[k+1],event.y,-1,-1,-1);
}

//void FortuneVoronoi::predict_circle_event(Arc &arc,Breakpoint const &BP1,Breakpoint const &BP2,double l)
//{
//    int s1=BP1.site_prev;
//    int s2=BP1.site_next;
//    int s3=BP2.site_next;
//    
//    Site &S1=sites[s1];
//    Site &S2=sites[s2];
//    Site &S3=sites[s3];
//    
//    if(closing_breakpoints(S1,S2,S3,l,BP1.pos_value,BP2.pos_value,0.0001*(bx_max-bx_min)))
//    {
//        // Circle center
//        
//        Vector3 P1,P2,v1,v2;
//        
//        double x1=S1.x, y1=S1.y;
//        double x2=S2.x, y2=S2.y;
//        double x3=S3.x, y3=S3.y;
//        
//        P1.x=0.5*(x1+x2);
//        P1.y=0.5*(y1+y2);
//        
//        P2.x=0.5*(x2+x3);
//        P2.y=0.5*(y2+y3);
//        
//        v1.x=-(y2-y1);
//        v1.y=x2-x1;
//        v1.normalize();
//        
//        v2.x=-(y3-y2);
//        v2.y=x3-x2;
//        v2.normalize();
//        
//        Vector3 vc=crossprod(v2,v1);
//        Vector3 pcv=crossprod(P1-P2,v2);
//        
//        double t=scalar_prod(pcv,vc)/vc.norm_sqr();
//        
//        Vector3 P=P1+t*v1;
//        
//        double r=(Vector3(x1,y1,0)-P).norm();
//        
//        // New circle event
//        
//        Event event;
//        
//        event.x=P.x;
//        event.y=P.y-r;
//        
//        event.circle_event=true;
//        event.circ_x=P.x;
//        event.circ_y=P.y;
//        
////        if(span<1e-8*(bx_max-bx_min) && event.y<l)
//        if(event.y<l)
//        {
//            event.circ_ps=s1;
//            event.site=s2;
//            event.circ_ns=s3;
//            
//            std::cout<<"Circ "<<event.circ_ps<<" "<<event.site<<" "<<event.circ_ns<<"  |  "<<(event.y-by_min)/(by_max-by_min)<<std::endl;
//            
//            arc.event_ID=queue.add_circle_event(event);
//        }
//    }
//}

void FortuneVoronoi::predict_circle_event(Arc &arc,Breakpoint const &BP1,Breakpoint const &BP2,double l,
                                          int gen_s1,int gen_s2,int gen_s3)
{
    int s1=BP1.site_prev;
    int s2=BP1.site_next;
    int s3=BP2.site_next;
    
    // Checking it doesn't come from the same circle event
        
    if(s1==gen_s1)
    {
             if(s2==gen_s2 && s3==gen_s3) return;
        else if(s2==gen_s3 && s3==gen_s2) return;
    }
    else if(s1==gen_s2)
    {
             if(s2==gen_s1 && s3==gen_s3) return;
        else if(s2==gen_s3 && s3==gen_s1) return;
    }
    else if(s1==gen_s3)
    {
             if(s2==gen_s2 && s3==gen_s1) return;
        else if(s2==gen_s1 && s3==gen_s2) return;
    }
    
    Site &S1=sites[s1];
    Site &S2=sites[s2];
    Site &S3=sites[s3];
    
    // Circle center
    
    Vector3 P1,P2,v1,v2;
    
    double x1=S1.x, y1=S1.y;
    double x2=S2.x, y2=S2.y;
    double x3=S3.x, y3=S3.y;
    
    P1.x=0.5*(x1+x2);
    P1.y=0.5*(y1+y2);
    
    P2.x=0.5*(x2+x3);
    P2.y=0.5*(y2+y3);
    
    v1.x=-(y2-y1);
    v1.y=x2-x1;
    v1.normalize();
    
    v2.x=-(y3-y2);
    v2.y=x3-x2;
    v2.normalize();
    
    Vector3 vc=crossprod(v2,v1);
    Vector3 pcv=crossprod(P1-P2,v2);
    
    double t=scalar_prod(pcv,vc)/vc.norm_sqr();
    
    Vector3 P=P1+t*v1;
    
    double r=(Vector3(x1,y1,0)-P).norm();
    
    // New circle event
    
    Event event;
    
    event.x=P.x;
    event.y=P.y-r;
    
    event.circle_event=true;
    event.circ_x=P.x;
    event.circ_y=P.y;
    
    double span=breakpoints_span(S1,S2,S3,event.y,BP1.pos_value,BP2.pos_value);
    double span_neg=breakpoints_span(S1,S2,S3,event.y,!BP1.pos_value,!BP2.pos_value);
    
    #ifdef VORONOI_DEBUG
//    chk_var("A");
//    chk_var(s1);
//    chk_var(s2);
//    chk_var(s3);
//    chk_var(x1);
//    chk_var(x2);
//    chk_var(x3);
//    chk_var(v1);
//    chk_var(v2);
//    chk_var(vc);
//    chk_var(r);
//    chk_var(l);
//    chk_var(event.y);
//    chk_var((event.y-by_min)/(by_max-by_min));
//    chk_var((l-by_min)/(by_max-by_min));
//    chk_var(l-event.y);
    chk_var(std::abs(span));
    chk_var(std::abs(span_neg));
//    
//    chk_var("B");
//    
//    if(std::isnan(event.y)) std::exit(0);
    #endif
    
    if(event.y<l && std::abs(span)<std::abs(span_neg))
    {
        event.circ_ps=s1;
        event.site=s2;
        event.circ_ns=s3;
        
        #ifdef VORONOI_DEBUG
        std::cout<<"Circ "<<event.circ_ps<<" "<<event.site<<" "<<event.circ_ns<<"  |  "<<(event.y-by_min)/(by_max-by_min)<<std::endl;
        #endif
        
        arc.event_ID=queue.add_circle_event(event);
    }
    
//    if(s3==4) std::exit(0);
}

//###############
//   Functions
//###############

void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,
                           double x_min,double x_max,double y_min,double y_max)
{
    P.resize(N);
    std::vector<Vector3> test(Nt);
    
    P[0].x=x_min+randp(x_max-x_min);
    P[0].y=y_min+randp(y_max-y_min);
    
    ProgTimeDisp dsp(N);
    
    for(int i=1;i<N;i++)
    {
        int jf=0;
        double max_r=0;
        
        for(int j=0;j<Nt;j++)
        {
            double min_r_site=std::numeric_limits<double>::max();
            
            double x=x_min+randp(x_max-x_min);
            double y=y_min+randp(y_max-y_min);
            
            test[j].x=x;
            test[j].y=y;
            
            for(int k=0;k<i;k++)
            {
                double r=std::pow(P[k].x-x,2.0)+std::pow(P[k].y-y,2.0);
                
                min_r_site=std::min(min_r_site,r);
            }
            
            double r_border=var_min(x-x_min,x_max-x,y-y_min,y_max-y);
            min_r_site=std::min(min_r_site,r_border*r_border);
            
            if(min_r_site>max_r)
            {
                max_r=min_r_site;
                jf=j;
            }
        }
        
        P[i].x=test[jf].x;
        P[i].y=test[jf].y;
        
        ++dsp;
    }
}

class PQuadTree
{
    public:
        bool leaf;
        unsigned int gen,max_gen,max_elements;
        double x_min,x_max,y_min,y_max;
        Vector3 A,B,C,D;
        
        class Triplet
        {
            public:
                int ID;
                double x,y;
                
                Triplet(int ID_,double x_,double y_)
                    :ID(ID_), x(x_), y(y_)
                {}
        };
        
        std::vector<Triplet> p_list;
        
        PQuadTree *children[4];
        
        PQuadTree()
            :leaf(true), gen(0)
        {
            for(int i=0;i<4;i++) children[i]=nullptr;
        }
        
        ~PQuadTree()
        {
            for(int i=0;i<4;i++)
                if(children[i]!=nullptr)
                    delete children[i];
        }
        
        void add_point(int ID,double x,double y)
        {
            if(leaf)
            {
                if(x>=x_min && x<=x_max && y>=y_min && y<=y_max)
                    p_list.push_back(Triplet(ID,x,y));
                
                if(p_list.size()>max_elements)
                {
                    leaf=false;
                    
                    for(int i=0;i<4;i++)
                    {
                        children[i]=new PQuadTree;
                        children[i]->gen=gen+1;
                    }
                                    
                    children[0]->set_limits(x_min,0.5*(x_min+x_max),y_min,0.5*(y_min+y_max),max_elements,max_gen);
                    children[1]->set_limits(0.5*(x_min+x_max),x_max,y_min,0.5*(y_min+y_max),max_elements,max_gen);
                    children[2]->set_limits(x_min,0.5*(x_min+x_max),0.5*(y_min+y_max),y_max,max_elements,max_gen);
                    children[3]->set_limits(0.5*(x_min+x_max),x_max,0.5*(y_min+y_max),y_max,max_elements,max_gen);
                    
                    for(int i=0;i<4;i++)
                    {
                        for(unsigned int j=0;j<p_list.size();j++)
                        {
                            Triplet &trp=p_list[j];
                            
                            children[i]->add_point(trp.ID,trp.x,trp.y);
                        }
                    }
                    
                    p_list.clear();
                }
            }
            else
            {
                for(int i=0;i<4;i++)
                    children[i]->add_point(ID,x,y);
            }
        }
        
        bool is_in(double x,double y)
        {
            return (x>=x_min && x<=x_max && y>=y_min && y<=y_max);
        }
        
        void potentially_closer(std::vector<int> &p_list_,double x,double y,double r)
        {
            p_list_.clear();
            
            potentially_closer_sub(p_list_,x,y,r);
        }
        
        bool potentially_closer_check(double x,double y,double r)
        {
            if(is_in(x,y) || 
               (x>=x_max && x-x_max<=r) || (x<=x_min && x_min-x<=r) ||
               (y>=y_max && y-y_max<=r) || (y<=y_min && y_min-y<=r) ) return true;
            
            return false;
        }
        
        void potentially_closer_sub(std::vector<int> &p_list_,double x,double y,double r)
        {
            if(potentially_closer_check(x,y,r))
            {
                if(leaf)
                {
                    for(unsigned int i=0;i<p_list.size();i++)
                        p_list_.push_back(p_list[i].ID);
                }
                else
                {
                    for(int i=0;i<4;i++)
                        children[i]->potentially_closer_sub(p_list_,x,y,r);
                }
            }
        }
        
        void set_limits(double x_min_,double x_max_,
                        double y_min_,double y_max_,
                        unsigned int max_elements_=10,unsigned int max_gen_=8)
        {
            x_min=x_min_;
            x_max=x_max_;
            y_min=y_min_;
            y_max=y_max_;
            
            A=Vector3(x_min,y_min,0);
            B=Vector3(x_max,y_min,0);
            C=Vector3(x_max,y_max,0);
            D=Vector3(x_min,y_max,0);
            
            max_elements=max_elements_;
            max_gen=max_gen_;
            
            p_list.reserve(max_elements);
        }
        
        void ray_check(std::vector<int> &p_list_,double x,double y)
        {
            p_list_.clear();
            
            ray_check_sub(p_list_,x,y);
        }
        
        void ray_check_sub(std::vector<int> &p_list_,double x,double y)
        {
            if(is_in(x,y))
            {
                if(leaf)
                {
                    for(unsigned int i=0;i<p_list.size();i++)
                        p_list_.push_back(p_list[i].ID);
                }
                else
                {
                    for(int i=0;i<4;i++)
                        children[i]->ray_check_sub(p_list_,x,y);
                }
            }
        }
};

void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,
                           double x_min,double x_max,double y_min,double y_max,int Na)
{
    // Acceleration Grid
    
    if(Na==0) Na=N;
    
    PQuadTree tree;
    tree.set_limits(x_min,x_max,y_min,y_max,Na);
    
    //
    
    P.resize(N);
    std::vector<Vector3> test(Nt);
    
    P[0].x=x_min+randp(x_max-x_min);
    P[0].y=y_min+randp(y_max-y_min);
    
    tree.add_point(0,P[0].x,P[0].y);
    
    std::vector<int> p_list;
    
    ProgTimeDisp dsp(N);
    
//    Timer t1,t2;
    
    for(int i=1;i<N;i++)
    {
        int jf=0;
        double max_r=0;
        
        for(int j=0;j<Nt;j++)
        {            
            double x=x_min+randp(x_max-x_min);
            double y=y_min+randp(y_max-y_min);
            
            test[j].x=x;
            test[j].y=y;
            
            double min_r_site=(P[0]-test[j]).norm();
            
            double r_border=var_min(x-x_min,x_max-x,y-y_min,y_max-y);
            min_r_site=std::min(min_r_site,r_border*2.0);
            
            // Initial Leaf check
            
//            t1.tic();
            
            tree.ray_check(p_list,x,y);
            
//            t1.toc();
            
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm());
            
            // Other leaves-check
            
//            t2.tic();
            
            tree.potentially_closer(p_list,x,y,min_r_site);
            
//            t2.toc();
            
//            chk_var(p_list.size());
            
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm());
            
            if(min_r_site>max_r)
            {
                max_r=min_r_site;
                jf=j;
            }
        }
        
        P[i]=test[jf];
        
        tree.add_point(i,P[i].x,P[i].y);
        
        ++dsp;
    }
    
//    std::cout<<t1()<<" "<<t2()<<std::endl;
}

void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,
                           double x_min,double x_max,double y_min,double y_max,int Na,
                           std::vector<Vector3> const &fixed_sites,double fixed_rad)
{
    if(N<=0) return;
    
    // Acceleration Grid
    
    if(Na==0) Na=N;
    
    PQuadTree tree,ftree;
    tree.set_limits(x_min,x_max,y_min,y_max,Na);
    ftree.set_limits(x_min,x_max,y_min,y_max,Na);
    
    //
    
    P.resize(N);
    std::vector<Vector3> test(Nt);
    
    bool resample=true;
    
    while(resample)
    {
        P[0].x=x_min+randp(x_max-x_min);
        P[0].y=y_min+randp(y_max-y_min);
        
        resample=false;
        
        for(unsigned int i=0;i<fixed_sites.size();i++)
        {
            if((P[0]-fixed_sites[i]).norm()<=fixed_rad)
            {
                resample=true;
                break;
            }
        }
    }
    
    tree.add_point(0,P[0].x,P[0].y);
    
    for(unsigned int i=0;i<fixed_sites.size();i++)
        ftree.add_point(i,fixed_sites[i].x,fixed_sites[i].y);
    
    std::vector<int> p_list;
    
    ProgTimeDisp dsp(N);
    
    for(int i=1;i<N;i++)
    {
        int jf=0;
        double max_r=0;
        
        for(int j=0;j<Nt;j++)
        {            
            double x,y;
            
            resample=true;
            
            while(resample)
            {
                x=x_min+randp(x_max-x_min);
                y=y_min+randp(y_max-y_min);
                
                resample=false;
                ftree.ray_check(p_list,x,y);
                
                for(unsigned int k=0;k<p_list.size();k++)
                {
                    if((Vector3(x,y,0)-fixed_sites[p_list[k]]).norm()<=fixed_rad)
                    {
                        resample=true;
                        break;
                    }
                }
                
                ftree.potentially_closer(p_list,x,y,fixed_rad);
                
                for(unsigned int k=0;k<p_list.size();k++)
                {
                    if((Vector3(x,y,0)-fixed_sites[p_list[k]]).norm()<=fixed_rad)
                    {
                        resample=true;
                        break;
                    }
                }
            }
            
            test[j].x=x;
            test[j].y=y;
            
            double min_r_site=(P[0]-test[j]).norm();
            
            double r_border=var_min(x-x_min,x_max-x,y-y_min,y_max-y);
            min_r_site=std::min(min_r_site,r_border*2.0);
            
            // Initial Leaf check
            
            tree.ray_check(p_list,x,y);
            
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm());
            
            // Other leaves-check
            
            tree.potentially_closer(p_list,x,y,min_r_site);
                        
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm());
            
            if(min_r_site>max_r)
            {
                max_r=min_r_site;
                jf=j;
            }
        }
        
        P[i]=test[jf];
        
        tree.add_point(i,P[i].x,P[i].y);
        
        ++dsp;
    }
}

void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,
                           double x_min,double x_max,double y_min,double y_max,int Na,
                           Grid2<double> const &density_map)
{
    // Acceleration Grid
    
    if(Na==0) Na=N;
    
    PQuadTree tree;
    tree.set_limits(x_min,x_max,y_min,y_max,Na);
    
    //
    
    P.resize(N);
    std::vector<Vector3> test(Nt);
    
    P[0].x=x_min+randp(x_max-x_min);
    P[0].y=y_min+randp(y_max-y_min);
    
    tree.add_point(0,P[0].x,P[0].y);
    
    std::vector<int> p_list;
    
    ProgTimeDisp dsp(N);
    
//    Timer t1,t2;
    
    for(int i=1;i<N;i++)
    {
        int jf=0;
        double max_r=0;
        
        for(int j=0;j<Nt;j++)
        {            
            double x=x_min+randp(x_max-x_min);
            double y=y_min+randp(y_max-y_min);
            
            test[j].x=x;
            test[j].y=y;
            
            double local_density=density_map.bilin_interp(x,y,x_min,x_max,y_min,y_max);
            
            double min_r_site=(P[0]-test[j]).norm()/local_density;
            
            double r_border=var_min(x-x_min,x_max-x,y-y_min,y_max-y)/local_density;
            min_r_site=std::min(min_r_site,r_border*2.0);
            
            // Initial Leaf check
            
            tree.ray_check(p_list,x,y);
            
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm()/local_density);
            
            // Other leaves-check
            
            tree.potentially_closer(p_list,x,y,min_r_site);
            
            for(unsigned int k=0;k<p_list.size();k++)
                min_r_site=std::min(min_r_site,(P[p_list[k]]-test[j]).norm()/local_density);
            
            if(min_r_site>max_r)
            {
                max_r=min_r_site;
                jf=j;
            }
        }
        
        P[i]=test[jf];
        
        tree.add_point(i,P[i].x,P[i].y);
        
        ++dsp;
    }
}

