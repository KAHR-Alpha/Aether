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

#include <bitmap3.h>
#include <polygons_2D.h>

extern const double Pi;

//###############
//   Polygon2D
//###############

void Polygon2D::add_vertex(int vertex_index_)
{
    vertex_index.push_back(vertex_index_);
    N=vertex_index.size();
}

void Polygon2D::clear()
{
    vertex_index.clear();
    N=0;
}

#include <string_tools.h>

double Polygon2D::compute_area(std::vector<Vertex> const &v_arr)
{
    reorder_vertices(v_arr);
    
    Vector3 O(c_x,c_y);
    
//    chk_var(c_x);
//    chk_var(c_y);
    
    double S=0;
    
    for(int i=0;i<N;i++)
    {
        Vector3 V1=v_arr[vertex_index[i]].loc-O;
        Vector3 V2=v_arr[vertex_index[(i+1)%N]].loc-O;
        
        Vector3 C=crossprod(V1,V2);
        
//        chk_var(add_unit(V1.norm()));
        
        S+=C.norm()/2.0;
    }
    
//    std::system("pause");
    
    return S;
}

void Polygon2D::compute_boundaries(std::vector<Vertex> const &v_arr)
{
    c_x=c_y=0;
        
    x_min=x_max=v_arr[vertex_index[0]].loc.x;
    y_min=y_max=v_arr[vertex_index[0]].loc.y;
    
    for(int i=0;i<N;i++)
    {
        c_x+=v_arr[vertex_index[i]].loc.x;
        c_y+=v_arr[vertex_index[i]].loc.y;
        
        x_min=std::min(x_min,v_arr[vertex_index[i]].loc.x);
        x_max=std::max(x_max,v_arr[vertex_index[i]].loc.x);
        
        y_min=std::min(y_min,v_arr[vertex_index[i]].loc.y);
        y_max=std::max(y_max,v_arr[vertex_index[i]].loc.y);
    }
    
    c_x/=N;
    c_y/=N;
}

void Polygon2D::forget_vertex(int vertex_index_)
{
    std::vector<int>::const_iterator it=vertex_index.begin()+vertex_index_;
    vertex_index.erase(it);
    N--;
}

//bool Polygon2D::is_inside(double x,double y,std::vector<Vertex> const &V_arr)
//{
//    if(x<x_min || x>x_max || y<y_min || y>y_max) return false;
//    
//    double vx,vy;
//    
//    int k=0;
//    double xt,t;
//    
//    for(int i=0;i<N;i++)
//    {
//        int j=(i+1)%N;
//        
//        Vector3 const &p1=V_arr[vertex_index[i]].loc;
//        Vector3 const &p2=V_arr[vertex_index[j]].loc;
//        
//        vx=p2.x-p1.x;
//        vy=p2.y-p1.y;
//        
//        if(vy!=0)
//        {
//            t=(y-p1.y)/vy;
//            xt=p1.x+t*vx;
//            
//            if(xt>x && t>=0 && t<=1.0)
//                k++;
//        }
//    }
//        
//    if(k%2!=0) return true;
//    
//    return false;
//}

bool Polygon2D::is_inside(double x,double y,std::vector<Vertex> const &V_arr)
{
    if(x<x_min || x>x_max || y<y_min || y>y_max) return false;
    
    int k=0;    
        
    for(int i=0;i<N;i++)
    {
        int j=(i+1)%N;
        
        Vector3 const &p1=V_arr[vertex_index[i]].loc;
        Vector3 const &p2=V_arr[vertex_index[j]].loc;
        
        double x1=p1.x; double y1=p1.y;
        double x2=p2.x; double y2=p2.y;
        
        if(x1>x2)
        {
            std::swap(x1,x2);
            std::swap(y1,y2);
        }
        
        if(x>=x1 && x<=x2)
        {
            double u=(x-x1)/(x2-x1);
            double v=(1.0-u)*y1+u*y2;
            
            if(y>=v) k++;
        }
    }
        
    if(k%2!=0) return true;
    
    return false;
}

//bool Polygon2D::is_inside(double x,double y,std::vector<Vertex> const &V_arr)
//{
//    if(x<x_min || x>x_max || y<y_min || y>y_max) return false;
//    
//    Vector3 P,V,V12;
//    
//    P.x=x;
//    P.y=y;
//    
//    V.randnorm_s();
//    
//    int k=0;
//    double t1,t2;
//    
//    for(int i=0;i<N;i++)
//    {
//        int j=(i+1)%N;
//        
//        Vector3 const &p1=V_arr[vertex_index[i]].loc;
//        Vector3 const &p2=V_arr[vertex_index[j]].loc;
//        
//        V12=p2-p1;
//        
//        lines_intersect(t1,t2,P,V,p1,V12);
//        
//        if(t1>=0 && t2>=0 && t2<=1.0) k++;
//    }
//    
//    if(k%2!=0) return true;
//    
//    return false;
//}

Vector3 Polygon2D::get_center(std::vector<Vertex> const &v_arr)
{
    reorder_vertices(v_arr);
    
    return Vector3(c_x,c_y,0);
}

void Polygon2D::largest_in_circle(double &x,double &y,double &r,std::vector<Vertex> const &v_arr)
{
    int i,j,k,l;
    
    double r_max=0;
    
//    chk_var(N);
//    for(i=0;i<N;i++)
//        std::cout<<v_arr[vertex_index[i]].loc<<std::endl;
        
    
    for(i=0;i<N;i++)
    {
        Vector3 P1=v_arr[vertex_index[i]].loc;
        
        Vector3 v1=v_arr[vertex_index[(i+1)%N]].loc-P1;
        v1.normalize();
    
//        chk_var(P1);
//        chk_var(v1);
        
        for(j=i+1;j<N;j++)
        {
            Vector3 P2=v_arr[vertex_index[j]].loc;
            
            Vector3 v2=v_arr[vertex_index[(j+1)%N]].loc-P2;
            v2.normalize();
            
//            chk_var(P2);
//            chk_var(v2);
            
            Vector3 A=lines_intersect(P1,v1,P2,v2);
                        
            Vector3 v12=v2-v1;
            v12.normalize();
            
//            chk_var(v12);
//            std::system("pause");
            
            for(k=j+1;k<N;k++)
            {
                Vector3 P3=v_arr[vertex_index[k]].loc;
                
                Vector3 v3=v_arr[vertex_index[(k+1)%N]].loc-P3;
                v3.normalize();
                
                Vector3 B=lines_intersect(P2,v2,P3,v3);
                
                Vector3 v23=v3-v2;
                v23.normalize();
                
                //
                
                Vector3 X=lines_intersect(A,v12,B,v23);
                
                r=point_to_line_distance(X,A,B-A);
//                chk_var(r);
//                chk_var(X);
//                chk_var(is_inside(X.x,X.y,v_arr));
//                std::exit(0);
                
                if(is_inside(X.x,X.y,v_arr))
                {
                    bool valid_circle=true;
                    
                    for(l=0;l<N;l++)
                    {
                        if(l!=i && l!=j && l!=k)
                        {
                            if(r*0.999>=point_to_edge_distance(X,v_arr[vertex_index[l]].loc,
                                                           v_arr[vertex_index[(l+1)%N]].loc))
                            {
                                valid_circle=false;
                                break;
                            }
                        }
                    }
                        
                    if(valid_circle && r>=r_max)
                    {
                        r_max=r;
                        x=X.x;
                        y=X.y;
                    }
                }
            }
        }
    }
    
    r=r_max;
}

void Polygon2D::reorder_vertices(std::vector<Vertex> const &v_arr)
{
    int i,j;
    
    compute_boundaries(v_arr);
    
    for(i=0;i<N;i++)
    {
        int k=i;
        double min_a=std::atan2(v_arr[vertex_index[i]].loc.y-c_y,
                                v_arr[vertex_index[i]].loc.x-c_x);
        
        for(j=i+1;j<N;j++)
        {
            double tmp_a=std::atan2(v_arr[vertex_index[j]].loc.y-c_y,
                                    v_arr[vertex_index[j]].loc.x-c_x);
            
            if(tmp_a<=min_a)
            {
                min_a=tmp_a;
                k=j;
            }
        }
        
        if(k!=i) std::swap(vertex_index[i],vertex_index[k]);
    }
}

void Polygon2D::set_vertices(std::vector<int> const &v_index,std::vector<Vertex> const &v_arr)
{
    vertex_index=v_index;
    
    N=vertex_index.size();
    
    compute_boundaries(v_arr);
}

//#####################
//   Poly2DTree_node
//#####################

Poly2DTree_node::Poly2DTree_node(int level_,int max_level_,
                           double fx1_,double fx2_,
                           double fy1_,double fy2_)
    :bottom_node(false),
     level(level_),
     max_level(max_level_),
     fx1(fx1_), fx2(fx2_),
     fy1(fy1_), fy2(fy2_)
{
    if(level==max_level) bottom_node=true;
    
    for(int i=0;i<4;i++) children[i]=nullptr;
}

Poly2DTree_node::~Poly2DTree_node()
{
    for(int i=0;i<4;i++)
    {
        if(children[i]!=nullptr)
        {
            delete children[i];
            children[i]=nullptr;
        }
    }
}

void Poly2DTree_node::finalize()
{
    int i,j;
    
    if(bottom_node)
    {
        Nf=flist.size();
        gflist.resize(Nf,0);
        
        std::list<int>::const_iterator iterator;
        
        j=0;
        for(iterator=flist.begin();iterator!=flist.end();iterator++)
        {
            gflist[j]=*iterator;
            j++;
        }
        
        flist.clear();
    }
    else
    {
        for(i=0;i<4;i++) children[i]->finalize();
    }
}

void Poly2DTree_node::ray_check(std::vector<int> &o_flist,double x,double y)
{
    bool compute=false;
    
    if(!bottom_node) compute=true;
    if(Nf>0) compute=true;
    
    if(compute)
    {
        double Dx=(fx2-fx1)*0.05;
        double Dy=(fy2-fy1)*0.05;
        
        if(x>fx1-Dx && x<fx2+Dx &&
           y>fy1-Dy && y<fy2+Dy)
        {
            if(bottom_node)
            {
                for(int i=0;i<Nf;i++) o_flist.push_back(gflist[i]);
            }
            else
            {
                for(int i=0;i<4;i++) children[i]->ray_check(o_flist,x,y);
            }
        }
    }
}

//################
//################

Poly2DTree::Poly2DTree(int max_level_)
    :max_level(max_level_),
     fx1(-1), fx2(1),
     fy1(-1), fy2(1)
{
    for(int i=0;i<4;i++) children[i]=nullptr;
}

Poly2DTree::~Poly2DTree()
{
    clear_tree();
}

void Poly2DTree::clear_tree()
{
    for(int i=0;i<4;i++)
    {
        if(children[i]!=nullptr)
        {
            delete children[i];
            children[i]=nullptr;
        }
    }
}

void Poly2DTree::finalize()
{
    int i;
    
    for(i=0;i<4;i++) children[i]->finalize();
}

void Poly2DTree::ray_check(std::vector<int> &flist,double x,double y)
{
    flist.clear();
    
    if(x>=fx1 && x<=fx2 && y>=fy1 && y<=fy2)
    {
        for(int i=0;i<4;i++)
            children[i]->ray_check(flist,x,y);
    }
}

//

void edges_from_polygons(std::vector<Poly2DEdge> &edges,std::vector<Polygon2D> const &polys)
{
    int N_reserve=0;
    
    for(std::size_t i=0;i<polys.size();i++)
    {
        N_reserve+=polys[i].N;
    }
    
    edges.clear();
    edges.reserve(N_reserve/2);
    
    Poly2DEdge tmp_edge;
    
    for(std::size_t i=0;i<polys.size();i++)
    {
        Polygon2D const &P=polys[i];
        
        for(int j=0;j<P.N;j++)
        {
            bool found_edge=false;
            std::size_t found_index=0;
            
            tmp_edge.V1=P.vertex_index[j];
            tmp_edge.V2=P.vertex_index[(j+1)%P.N];
            
            found_index=vector_locate(found_edge,edges,tmp_edge);
            
            if(!found_edge)
            {
                found_index=edges.size();
                edges.push_back(tmp_edge);
            }
            
            edges[found_index].polygons.push_back(i);
        }
    }
}



void polygon2D_array_print(std::vector<Polygon2D> const &p_arr,std::vector<Vertex> const &v_arr,int Nx,std::string const &fname)
{
    double x_min,x_max,y_min,y_max;
    
    x_min=x_max=v_arr[0].loc.x;
    y_min=y_max=v_arr[0].loc.y;
    
    for(std::size_t i=0;i<v_arr.size();i++)
    {
        x_min=std::min(x_min,v_arr[i].loc.x);
        x_max=std::max(x_max,v_arr[i].loc.x);
        
        y_min=std::min(y_min,v_arr[i].loc.y);
        y_max=std::max(y_max,v_arr[i].loc.y);
    }
    
    int N=p_arr.size();
    
    int Ny=nearest_integer(Nx*(y_max-y_min)/(x_max-x_min));
    
    Bitmap img(Nx,Ny);
    
    for(int i=0;i<N;i++)
    {
        int Np=p_arr[i].N;
        
        for(int j=0;j<Np;j++)
        {
            double x1=v_arr[p_arr[i].vertex_index[j]].loc.x;
            double y1=v_arr[p_arr[i].vertex_index[j]].loc.y;
            
            double x2=v_arr[p_arr[i].vertex_index[(j+1)%Np]].loc.x;
            double y2=v_arr[p_arr[i].vertex_index[(j+1)%Np]].loc.y;
            
            x1=(0.05+0.9*(x1-x_min)/(x_max-x_min))*Nx;
            x2=(0.05+0.9*(x2-x_min)/(x_max-x_min))*Nx;
            y1=(0.05+0.9*(y1-y_min)/(y_max-y_min))*Ny;
            y2=(0.05+0.9*(y2-y_min)/(y_max-y_min))*Ny;
            
            img.draw_line(x1,y1,x2,y2,1,0,0);
        }
    }
    
    img.write(fname);
}

void square_polyongs_grid(std::vector<Polygon2D> &polys,std::vector<Vertex> &v_arr,int Ng)
{
    polys.resize(Ng*Ng);
    v_arr.resize((Ng+1)*(Ng+1));
    
    for(int i=0;i<Ng+1;i++)
    {
        for(int j=0;j<Ng+1;j++)
        {
            Vertex &V=v_arr[i+j*(Ng+1)];
            
            V.loc.x=i/static_cast<double>(Ng);
            V.loc.y=j/static_cast<double>(Ng);
        }
    }
    
    for(int i=0;i<Ng;i++)
    {
        for(int j=0;j<Ng;j++)
        {
            Polygon2D &P=polys[i+Ng*j];
            
            P.add_vertex(i+(Ng+1)*j);
            P.add_vertex(i+1+(Ng+1)*j);
            P.add_vertex(i+(Ng+1)*(j+1));
            P.add_vertex(i+1+(Ng+1)*(j+1));
            
            P.reorder_vertices(v_arr);
        }
    }
}
