/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef OCTREE_H
#define OCTREE_H

#include <mathUT.h>

#include <list>
#include <vector>

//Child of the Octree class, works as a big binary tree
class Octree_node
{
    public:
        bool bottom_node;
        int level,max_level;
        double fx1,fx2,fy1,fy2,fz1,fz2;
        
        int N_elem;
        std::list<int> flist;
        std::vector<int> gflist;
        
        Octree_node *children[8];
        
        Octree_node(int level,int max_level,
                    double fx1,double fx2,
                    double fy1,double fy2,
                    double fz1,double fz2);
        ~Octree_node();
        
        template<class Element,class Rule>
        void add_elements(std::vector<Element> const &E_arr,
                          Rule const &rule,
                          std::list<int> const &b_flist)
        {
            for(int i : b_flist)
            {
                if(rule(E_arr[i], fx1, fx2, fy1, fy2, fz1, fz2)) flist.push_back(i);
            }
            
            if(flist.size()<10) bottom_node=true;
        }
        
        template<class Element,class Rule>
        void breed(std::vector<Element> const &E_arr,
                   Rule const &rule)
        {
            if(bottom_node==false)
            {
                int i,j,k,l;
                
                l=0;
                
                double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
                double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
                double fzh=(fz2-fz1)/2.0 , fzm=(fz2+fz1)/2.0;
                
                for(i=0;i<2;i++){ for(j=0;j<2;j++){ for(k=0;k<2;k++)
                {
                    children[l]=new Octree_node(level+1,max_level,
                                                fx1+i*fxh,fxm+i*fxh,
                                                fy1+j*fyh,fym+j*fyh,
                                                fz1+k*fzh,fzm+k*fzh);
                                                 
                    children[l]->add_elements(E_arr, rule, flist);
                    children[l]->breed(E_arr, rule);
                    l++;
                }}}
                
                flist.clear();
            }
        }
        
        void finalize();
        
        template<class Container>
        void point_check(Container& o_flist,
                         double x, double y, double z)
        {
            bool compute=false;
            
            if(!bottom_node) compute=true;
            if(N_elem > 0) compute=true;
            
            if((!bottom_node || N_elem > 0)
               && x >= fx1 && x <= fx2
               && y >= fy1 && y <= fy2
               && z >= fz1 && z <= fz2)
            {
                if(bottom_node)
                {
                    for(int i=0; i<N_elem; i++) o_flist.push_back(gflist[i]);
                }
                else
                {
                    for(int i=0;i<8;i++) children[i]->point_check(o_flist, x, y, z);
                }
            }
        }
        
        template<class TRay, class Container>
        void ray_check(Container& o_flist, TRay const &ray)
        {
            bool compute=false;
            
            if(!bottom_node) compute=true;
            if(N_elem > 0) compute=true;
            
            if(compute)
            {
                //double Tbuf[6];
                double x,y,z;
                
                double Dx=(fx2-fx1)*0.05;
                double Dy=(fy2-fy1)*0.05;
                double Dz=(fz2-fz1)*0.05;
                
                double tmin=(fx1-Dx-ray.start.x)*ray.inv_dir.x;
                double tmax=(fx2+Dx-ray.start.x)*ray.inv_dir.x;
                double tymin=(fy1-Dy-ray.start.y)*ray.inv_dir.y;
                double tymax=(fy2+Dy-ray.start.y)*ray.inv_dir.y;
                
                if(tmin>tmax) std::swap(tmin,tmax);
                if(tymin>tymax) std::swap(tymin,tymax);
                
                if(tmin>tymax || tymin>tmax) return;
                if(tymin>tmin) tmin=tymin;
                if(tymax<tmax) tmax=tymax;
                
                double tzmin=(fz1-Dz-ray.start.z)*ray.inv_dir.z;
                double tzmax=(fz2+Dz-ray.start.z)*ray.inv_dir.z;
                
                if(tzmin>tzmax) std::swap(tzmin,tzmax);
                if(tmin>tzmax || tzmin>tmax) return;
                if(tzmin>tmin) tmin=tzmin;
                if(tzmax<tmax) tmax=tzmax;
            
                if(tmax>0)
                {
                    double t_tmp=(tmin+tmax)/2.0;
                    x=ray.start.x+t_tmp*ray.dir.x;
                    y=ray.start.y+t_tmp*ray.dir.y;
                    z=ray.start.z+t_tmp*ray.dir.z;
                    
                    if(x>fx1-Dx && x<fx2+Dx &&
                       y>fy1-Dy && y<fy2+Dy &&
                       z>fz1-Dz && z<fz2+Dz)
                    {
                        if(bottom_node)
                        {
                            for(int i=0; i<N_elem; i++) o_flist.push_back(gflist[i]);
                        }
                        else
                        {
                            for(int i=0;i<8;i++) children[i]->ray_check(o_flist, ray);
                        }
                    }
                }
            }
        }
};

//Root of the Octrees, got the top-level commands
class Octree
{
    public:
        Octree();
        Octree(int max_level,
               double fx1,double fx2,
               double fy1,double fy2,
               double fz1,double fz2);
        ~Octree();
        
        void clear_tree();
        
        template<class Element, class Rule>
        void generate_tree(std::vector<Element> const &E_arr, Rule const &rule)
        {
            int i, j, k, l;

            clear_tree();

            int N = E_arr.size();
            std::list<int> tmp_list;
            for (i = 0; i < N; i++) tmp_list.push_back(i);

            l = 0;

            double fxh = (fx2-fx1)/2.0, fxm = (fx2+fx1)/2.0;
            double fyh = (fy2-fy1)/2.0, fym = (fy2+fy1)/2.0;
            double fzh = (fz2-fz1)/2.0, fzm = (fz2+fz1)/2.0;

            for(i=0; i<2; i++) for(j=0; j<2; j++) for(k=0; k<2; k++)
            {
                children[l] = new Octree_node(1, max_level,
                                              fx1+i*fxh, fxm+i*fxh,
                                              fy1+j*fyh, fym+j*fyh,
                                              fz1+k*fzh, fzm+k*fzh);

                children[l]->add_elements(E_arr, rule, tmp_list);
                children[l]->breed(E_arr, rule);
                l++;
            }

            tmp_list.clear();
            finalize();
        }
        
        template<class TRay, class Container>
        void ray_check(Container &out_list, TRay const &ray)
        {
            out_list.clear();
            
            double x,y,z;
            
            double tmin=(fx1-ray.start.x)*ray.inv_dir.x;
            double tmax=(fx2-ray.start.x)*ray.inv_dir.x;
            double tymin=(fy1-ray.start.y)*ray.inv_dir.y;
            double tymax=(fy2-ray.start.y)*ray.inv_dir.y;
            
            if(tmin>tmax) std::swap(tmin,tmax);
            if(tymin>tymax) std::swap(tymin,tymax);
            
            if(tmin>tymax || tymin>tmax) return;
            if(tymin>tmin) tmin=tymin;
            if(tymax<tmax) tmax=tymax;
            
            double tzmin=(fz1-ray.start.z)*ray.inv_dir.z;
            double tzmax=(fz2-ray.start.z)*ray.inv_dir.z;
            
            if(tzmin>tzmax) std::swap(tzmin,tzmax);
            if(tmin>tzmax || tzmin>tmax) return;
            if(tzmin>tmin) tmin=tzmin;
            if(tzmax<tmax) tmax=tzmax;
            
            if(tmax>0)
            {
                double t_tmp=(tmin+tmax)/2.0;
                x=ray.start.x+t_tmp*ray.dir.x;
                y=ray.start.y+t_tmp*ray.dir.y;
                z=ray.start.z+t_tmp*ray.dir.z;
                
                if(x>fx1 && x<fx2 && y>fy1 && y<fy2 && z>fz1 && z<fz2)
                {
                    for(int i=0;i<8;i++) children[i]->ray_check(out_list, ray);
                }
            }
        }
        
        template<class Container>
        void point_check(Container &out_list,
                         double x, double y, double z)
        {
            out_list.clear();
            
            if(   x>=fx1 && x<=fx2
               && y>=fy1 && y<=fy2
               && z>=fz1 && z<=fz2)
            {
                for(int i=0;i<8;i++) children[i]->point_check(out_list, x, y, z);
            }
        }
        
        void set_params(int max_level,
                        double fx1,double fx2,
                        double fy1,double fy2,
                        double fz1,double fz2);
        
    private:
        int max_level;
        double fx1, fx2, fy1, fy2, fz1, fz2;
        Octree_node* children[8];

        void finalize();
};

#endif // FOCTREE_H
