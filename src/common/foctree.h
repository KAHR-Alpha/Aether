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

#ifndef FOCTREE_H
#define FOCTREE_H

#include <geometry.h>
#include <mathUT.h>
#include <list>

//Child of the FOctree class, works as a big binary tree
class FOctree_node
{
    public:
        bool bottom_node;
        int level,max_level;
        double fx1,fx2,fy1,fy2,fz1,fz2;
        
        int Nf;
        std::list<int> flist;
        Grid1<int> gflist;
        
        FOctree_node *children[8];
        
        FOctree_node(int level,int max_level,
                     double fx1,double fx2,
                     double fy1,double fy2,
                     double fz1,double fz2);
        ~FOctree_node();
        
        template<class TVertex,class TFace>
        void add_faces(Grid1<TVertex> const &V_arr,Grid1<TFace> const &F_arr,std::list<int> const &b_flist)
        {
            int i;
            
            std::list<int>::const_iterator iterator;
            
            double xmin=0,xmax=0,ymin=0,ymax=0,zmin=0,zmax=0;
            
            bool in=false;
            
            using std::min;
            using std::max;
            
            double Dx=fx2-fx1;
            double Dy=fy2-fy1;
            double Dz=fz2-fz1;
            
            for(iterator=b_flist.begin();iterator!=b_flist.end();iterator++)
            {
                i=*iterator;
                
                Vector3 const &loc1=V_arr(F_arr(i).V1).loc;
                Vector3 const &loc2=V_arr(F_arr(i).V2).loc;
                Vector3 const &loc3=V_arr(F_arr(i).V3).loc;
                
                xmin=min(loc1.x,min(loc2.x,loc3.x));
                xmax=max(loc1.x,max(loc2.x,loc3.x));
                ymin=min(loc1.y,min(loc2.y,loc3.y));
                ymax=max(loc1.y,max(loc2.y,loc3.y));
                zmin=min(loc1.z,min(loc2.z,loc3.z));
                zmax=max(loc1.z,max(loc2.z,loc3.z));
                
                in=false;
                
                if(xmin>fx2+0.05*Dx || xmax<fx1-0.05*Dx ||
                   ymin>fy2+0.05*Dy || ymax<fy1-0.05*Dy ||
                   zmin>fz2+0.05*Dz || zmax<fz1-0.05*Dz) in=false;
                else in=true;
                
                if(in) flist.push_back(i);
            }
            
            if(flist.size()<10) bottom_node=true;
        }
        
        template<class TVertex,class TFace>
        void add_faces(std::vector<TVertex> const &V_arr,std::vector<TFace> const &F_arr,std::list<int> const &b_flist)
        {
            int i;
            
            std::list<int>::const_iterator iterator;
            
            double xmin=0,xmax=0,ymin=0,ymax=0,zmin=0,zmax=0;
            
            bool in=false;
            
            using std::min;
            using std::max;
            
            double Dx=fx2-fx1;
            double Dy=fy2-fy1;
            double Dz=fz2-fz1;
            
            for(iterator=b_flist.begin();iterator!=b_flist.end();iterator++)
            {
                i=*iterator;
                
                Vector3 const &loc1=V_arr[F_arr[i].V1].loc;
                Vector3 const &loc2=V_arr[F_arr[i].V2].loc;
                Vector3 const &loc3=V_arr[F_arr[i].V3].loc;
                
                xmin=min(loc1.x,min(loc2.x,loc3.x));
                xmax=max(loc1.x,max(loc2.x,loc3.x));
                ymin=min(loc1.y,min(loc2.y,loc3.y));
                ymax=max(loc1.y,max(loc2.y,loc3.y));
                zmin=min(loc1.z,min(loc2.z,loc3.z));
                zmax=max(loc1.z,max(loc2.z,loc3.z));
                
                in=false;
                
                if(xmin>fx2+0.05*Dx || xmax<fx1-0.05*Dx ||
                   ymin>fy2+0.05*Dy || ymax<fy1-0.05*Dy ||
                   zmin>fz2+0.05*Dz || zmax<fz1-0.05*Dz) in=false;
                else in=true;
                
                if(in) flist.push_back(i);
            }
            
            if(flist.size()<10) bottom_node=true;
        }
        
        template<class TVertex,class TFace>
        void breed(Grid1<TVertex> const &V_arr,Grid1<TFace> const &F_arr)
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
                    children[l]=new FOctree_node(level+1,max_level,
                                                 fx1+i*fxh,fxm+i*fxh,
                                                 fy1+j*fyh,fym+j*fyh,
                                                 fz1+k*fzh,fzm+k*fzh);
                                                 
                    children[l]->add_faces(V_arr,F_arr,flist);
                    children[l]->breed(V_arr,F_arr);
                    l++;
                }}}
                
                flist.clear();
            }
        }
        
        template<class TVertex,class TFace>
        void breed(std::vector<TVertex> const &V_arr,std::vector<TFace> const &F_arr)
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
                    children[l]=new FOctree_node(level+1,max_level,
                                                 fx1+i*fxh,fxm+i*fxh,
                                                 fy1+j*fyh,fym+j*fyh,
                                                 fz1+k*fzh,fzm+k*fzh);
                                                 
                    children[l]->add_faces(V_arr,F_arr,flist);
                    children[l]->breed(V_arr,F_arr);
                    l++;
                }}}
                
                flist.clear();
            }
        }
        
        void finalize();
        
        template<class TRay>
        void ray_check(TRay const &ray,std::list<int> &o_flist)
        {
            bool compute=false;
            
            if(!bottom_node) compute=true;
            if(Nf>0) compute=true;
            
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
                            //for(int i=0;i<Nf;i++) tr_check(gflist(i))=1;
                            for(int i=0;i<Nf;i++) o_flist.push_back(gflist[i]);
                        }
                        else
                        {
                            for(int i=0;i<8;i++) children[i]->ray_check(ray,o_flist);
                        }
                    }
                }
            }
        }
        
        template<class TRay>
        void ray_check(TRay const &ray,std::vector<int> &o_flist)
        {
            bool compute=false;
            
            if(!bottom_node) compute=true;
            if(Nf>0) compute=true;
            
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
                            //for(int i=0;i<Nf;i++) tr_check(gflist(i))=1;
                            for(int i=0;i<Nf;i++) o_flist.push_back(gflist[i]);
                        }
                        else
                        {
                            for(int i=0;i<8;i++) children[i]->ray_check(ray,o_flist);
                        }
                    }
                }
            }
        }
};

//Root of the Octrees dedicated to faces, got the top-level commands
class FOctree
{
    private:
        void finalize();
    public:
        int max_level;
        double fx1,fx2,fy1,fy2,fz1,fz2;
        FOctree_node *children[8];
        
        FOctree();
        FOctree(int max_level,
                double fx1,double fx2,
                double fy1,double fy2,
                double fz1,double fz2);
        ~FOctree();
        
        void clear_tree();
        
        template<class TVertex,class TFace>
        void generate_tree(Grid1<TVertex> const &V_arr,Grid1<TFace> const &F_arr)
        {
            int i,j,k,l;
            
            clear_tree();
            
            int Nf=F_arr.L1();
            std::list<int> tmp_list;
            for(i=0;i<Nf;i++) tmp_list.push_back(i);
            
            l=0;
            
            double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
            double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
            double fzh=(fz2-fz1)/2.0 , fzm=(fz2+fz1)/2.0;
            
            for(i=0;i<2;i++){ for(j=0;j<2;j++){ for(k=0;k<2;k++)
            {
                children[l]=new FOctree_node(1,max_level,
                                             fx1+i*fxh,fxm+i*fxh,
                                             fy1+j*fyh,fym+j*fyh,
                                             fz1+k*fzh,fzm+k*fzh);
                
                children[l]->add_faces(V_arr,F_arr,tmp_list);
                children[l]->breed(V_arr,F_arr);
                l++;
            }}}
            
            tmp_list.clear();
            finalize();
        }
        
        template<class TVertex,class TFace>
        void generate_tree(std::vector<TVertex> const &V_arr,std::vector<TFace> const &F_arr)
        {
            int i,j,k,l;
            
            clear_tree();
            
            int Nf=F_arr.size();
            std::list<int> tmp_list;
            for(i=0;i<Nf;i++) tmp_list.push_back(i);
            
            l=0;
            
            double fxh=(fx2-fx1)/2.0 , fxm=(fx2+fx1)/2.0;
            double fyh=(fy2-fy1)/2.0 , fym=(fy2+fy1)/2.0;
            double fzh=(fz2-fz1)/2.0 , fzm=(fz2+fz1)/2.0;
            
            for(i=0;i<2;i++){ for(j=0;j<2;j++){ for(k=0;k<2;k++)
            {
                children[l]=new FOctree_node(1,max_level,
                                             fx1+i*fxh,fxm+i*fxh,
                                             fy1+j*fyh,fym+j*fyh,
                                             fz1+k*fzh,fzm+k*fzh);
                
                children[l]->add_faces(V_arr,F_arr,tmp_list);
                children[l]->breed(V_arr,F_arr);
                l++;
            }}}
            
            tmp_list.clear();
            finalize();
        }
        
        template<class TRay>
        void ray_check(TRay const &ray,std::list<int> &flist)
        {
            //double Tbuf[6];
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
                    for(int i=0;i<8;i++) children[i]->ray_check(ray,flist);
                }
            }
        }
        
        template<class TRay>
        void ray_check(TRay const &ray,std::vector<int> &flist)
        {
            flist.clear();
            
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
                    for(int i=0;i<8;i++) children[i]->ray_check(ray,flist);
                }
            }
        }
        
        void set_params(int max_level,
                        double fx1,double fx2,
                        double fy1,double fy2,
                        double fz1,double fz2);
};

#endif // FOCTREE_H
