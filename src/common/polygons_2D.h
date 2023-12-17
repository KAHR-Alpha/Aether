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

#ifndef POLYGONS_2D_H
#define POLYGONS_2D_H

#include <mesh_base.h>

class Polygon2D
{
    public:
        int N;
        double x_min,x_max,y_min,y_max,c_x,c_y;
        
        std::vector<int> vertex_index;
        
        void add_vertex(int vertex_index);
        void clear();
        double compute_area(std::vector<Vertex> const &v_arr);
        void compute_boundaries(std::vector<Vertex> const &v_arr);
        void forget_vertex(int vertex_index);
        Vector3 get_center(std::vector<Vertex> const &v_arr);
        bool is_inside(double x,double y,std::vector<Vertex> const &v_arr);
        void largest_in_circle(double &x,double &y,double &r,std::vector<Vertex> const &v_arr);
        void reorder_vertices(std::vector<Vertex> const &v_arr);
        void set_vertices(std::vector<int> const &v_index,std::vector<Vertex> const &v_arr);
};
    
class Poly2DEdge
{
    public:
        int V1,V2;
        std::vector<int> polygons;
        
        Poly2DEdge()
            :V1(-1), V2(-1)
        {}
        
        Poly2DEdge(int V1_,int V2_)
            :V1(V1_), V2(V2_)
        {}
        
        Poly2DEdge(Poly2DEdge const &E)
            :V1(E.V1), V2(E.V2), polygons(E.polygons)
        {}
        
        bool operator == (Poly2DEdge const &E) const
        {
            return (V1==E.V1 && V2==E.V2) || (V1==E.V2 && V2==E.V1);
        }
};

class Poly2DTree2
{
    public:
        bool leaf;
        int level,max_level;
        double fx1,fx2,fy1,fy2;
        
        unsigned int max_faces;
        
        std::vector<int> flist;
        
        Poly2DTree2 *children[4];
        
        Poly2DTree2()
            :leaf(true), level(0)
        {
            for(int i=0;i<4;i++)
                children[i]=nullptr;
        }
        
        Poly2DTree2(int max_level_)
            :leaf(true), level(0), max_level(max_level_)
        {
            for(int i=0;i<4;i++)
                children[i]=nullptr;
        }
        
        Poly2DTree2(int level_,int max_level_,int max_faces_,
                    double fx1_,double fx2_,
                    double fy1_,double fy2_)
            :leaf(true),
             level(level_), max_level(max_level_),
             fx1(fx1_), fx2(fx2_),
             fy1(fy1_), fy2(fy2_),
             max_faces(max_faces_)
        {
            for(int i=0;i<4;i++)
                children[i]=nullptr;
        }
        
        ~Poly2DTree2()
        {
            clear_tree();
        }
        
        void clear_tree()
        {
            for(int i=0;i<4;i++)
                if(children[i]!=nullptr)
                    delete children[i];
        }
        
        template<class TVertex>
        void add_polygons(std::vector<TVertex> const &V_arr,std::vector<Polygon2D> const &P_arr,std::vector<int> const &flist_)
        {
            double Dx=fx2-fx1;
            double Dy=fy2-fy1;
            
            for(unsigned int i=0;i<flist_.size();i++)
            {
                Polygon2D const &P=P_arr[flist_[i]];
                
                bool is_out=P.x_min>fx2+0.05*Dx || P.x_max<fx1-0.05*Dx ||
                            P.y_min>fy2+0.05*Dy || P.y_max<fy1-0.05*Dy;
                
                if(!is_out) flist.push_back(flist_[i]);
            }
            
            if(flist.size()>max_faces && level<max_level)
            {
                leaf=false;
                
                int i,j,l;
                
                double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
                double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
                
                l=0;
                
                for(i=0;i<2;i++){ for(j=0;j<2;j++)
                {
                    children[l]=new Poly2DTree2(level+1,max_level,max_faces,
                                                fx1+i*fxh,fxm+i*fxh,
                                                fy1+j*fyh,fym+j*fyh);
                    
                    children[l]->add_polygons(V_arr,P_arr,flist);
                    l++;
                }}
                
                flist.clear();
            }
        }
        
        template<class TVertex>
        void generate_tree(std::vector<TVertex> const &V_arr,std::vector<Polygon2D> const &P_arr,
                           unsigned int max_level_=8,unsigned int max_faces_=12)
        {
            max_level=max_level_;
            max_faces=max_faces_;
            
            clear_tree();
            
            // Boundaries
            
            fx1=fx2=P_arr[0].x_min;
            fy1=fy2=P_arr[0].y_min;
            
            for(unsigned int i=0;i<P_arr.size();i++)
            {
                fx1=std::min(fx1,P_arr[i].x_min);
                fx2=std::max(fx2,P_arr[i].x_max);
                
                fy1=std::min(fy1,P_arr[i].y_min);
                fy2=std::max(fy2,P_arr[i].y_max);
            }
            
            double span=fx2-fx1;
            fx1-=span/100.0; fx2+=span/100.0;
            
            span=fy2-fy1;
            fy1-=span/100.0; fy2+=span/100.0;
            
            chk_var(fx1);
            chk_var(fx2);
            chk_var(fy1);
            chk_var(fy2);
            
            // Generation
            
            std::vector<int> tmp_flist;
            
            for(unsigned int i=0;i<P_arr.size();i++)
                    tmp_flist.push_back(i);
                    
            add_polygons(V_arr,P_arr,tmp_flist);
        }
        
        void ray_check_sub(std::vector<int> &flist_,double x,double y)
        {
            if(x>=fx1 && x<=fx2 && y>=fy1 && y<=fy2)
            {
                if(leaf)
                {
                    for(unsigned int i=0;i<flist.size();i++)
                        flist_.push_back(flist[i]);
                }
                else
                {
                    for(int i=0;i<4;i++)
                        children[i]->ray_check_sub(flist_,x,y);
                }
            }
        }
        
        void ray_check(std::vector<int> &flist_,double x,double y)
        {
            flist_.clear();
            
            ray_check_sub(flist_,x,y);
        }
};

//Child of the Poly2DTree class, works as a big binary tree
class Poly2DTree_node
{
    public:
        bool bottom_node;
        int level,max_level;
        double fx1,fx2,fy1,fy2;
        
        int Nf;
        std::list<int> flist;
        std::vector<int> gflist;
        
        Poly2DTree_node *children[4];
        
        Poly2DTree_node(int level,int max_level,
                        double fx1,double fx2,
                        double fy1,double fy2);
        ~Poly2DTree_node();
                
        template<class TVertex>
        void add_polygons(std::vector<TVertex> const &V_arr,std::vector<Polygon2D> const &P_arr,std::list<int> const &b_flist)
        {
            int i;
            
            std::list<int>::const_iterator iterator;
            
            double xmin=0,xmax=0,ymin=0,ymax=0;
            
            bool in=false;
            
            using std::min;
            using std::max;
            
            double Dx=fx2-fx1;
            double Dy=fy2-fy1;
            
            for(iterator=b_flist.begin();iterator!=b_flist.end();iterator++)
            {
                i=*iterator;
                
                Vector3 const &loc=V_arr[P_arr[i].vertex_index[0]].loc;
                
                xmin=P_arr[i].x_min;
                xmax=P_arr[i].x_max;
                
                ymin=P_arr[i].y_min;
                ymax=P_arr[i].y_max;
                
                in=false;
                
                if(xmin>fx2+0.05*Dx || xmax<fx1-0.05*Dx ||
                   ymin>fy2+0.05*Dy || ymax<fy1-0.05*Dy) in=false;
                else in=true;
                
                if(in) flist.push_back(i);
            }
            
            if(flist.size()<10) bottom_node=true;
        }
                
        template<class TVertex>
        void breed(std::vector<TVertex> const &V_arr,std::vector<Polygon2D> const &P_arr)
        {
            if(bottom_node==false)
            {
                int i,j,k,l;
                
                l=0;
                
                double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
                double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
                
                for(i=0;i<2;i++){ for(j=0;j<2;j++)
                {
                    children[l]=new Poly2DTree_node(level+1,max_level,
                                                    fx1+i*fxh,fxm+i*fxh,
                                                    fy1+j*fyh,fym+j*fyh);
                                                 
                    children[l]->add_polygons(V_arr,P_arr,flist);
                    children[l]->breed(V_arr,P_arr);
                    l++;
                }}
                
                flist.clear();
            }
        }
        
        void finalize();
        
        void ray_check(std::vector<int> &o_flist,double x,double y);
};

//Root of the Octrees dedicated to faces, got the top-level commands
class Poly2DTree
{
    private:
        void finalize();
    public:
        int max_level;
        double fx1,fx2,fy1,fy2;
        Poly2DTree_node *children[4];
        
        Poly2DTree(int max_level=5);
        ~Poly2DTree();
        
        void clear_tree();
        
        template<class TVertex>
        void generate_tree(std::vector<TVertex> const &V_arr,std::vector<Polygon2D> const &P_arr)
        {
            int i,j,l;
            
            clear_tree();
            
            int Np=P_arr.size();
            
            std::list<int> tmp_list;
            for(i=0;i<Np;i++) tmp_list.push_back(i);
            
            // Boundaries
            
            fx1=fx2=P_arr[0].x_min;
            fy1=fy2=P_arr[0].y_min;
            
            for(unsigned int i=0;i<P_arr.size();i++)
            {
                fx1=std::min(fx1,P_arr[i].x_min);
                fx2=std::max(fx2,P_arr[i].x_max);
                
                fy1=std::min(fy1,P_arr[i].y_min);
                fy2=std::max(fy2,P_arr[i].y_max);
            }
            
            double span=fx2-fx1;
            fx1-=span/100.0; fx2+=span/100.0;
            
            span=fy2-fy1;
            fy1-=span/100.0; fy2+=span/100.0;
            
            chk_var(fx1);
            chk_var(fx2);
            chk_var(fy1);
            chk_var(fy2);
            
            double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
            double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
            
            l=0;
            
            for(i=0;i<2;i++){ for(j=0;j<2;j++)
            {
                children[l]=new Poly2DTree_node(1,max_level,
                                                fx1+i*fxh,fxm+i*fxh,
                                                fy1+j*fyh,fym+j*fyh);
                
                children[l]->add_polygons(V_arr,P_arr,tmp_list);
                children[l]->breed(V_arr,P_arr);
                l++;
            }}
            
            tmp_list.clear();
            finalize();
        }
        
        void ray_check(std::vector<int> &flist,double x,double y);
};

void edges_from_polygons(std::vector<Poly2DEdge> &edges,std::vector<Polygon2D> const &polys);
void polygon2D_array_print(std::vector<Polygon2D> const &p_arr,std::vector<Vertex> const &v_arr,int Nx,std::string const &fname);
void square_polyongs_grid(std::vector<Polygon2D> &polys,std::vector<Vertex> &v_arr,int Ng);

#endif // POLYGONS_2D_H
