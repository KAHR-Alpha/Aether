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

#include <selene.h>
#include <selene_primitives.h>

extern std::ofstream plog;

namespace Sel::Primitives
{
    //##########
    //   Disk
    //##########
    
    Disk::Disk(BoundingBox &bbox_,
               std::vector<Sel::SelFace> &F_arr_,
               std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         radius(0.01), in_radius(0)
    {
    }
    
    
    /*Vector3 Disk::anchor(int anchor) const
    {
        // Todo
    }*/
    
    
    /*std::string Disk::anchor_name(int anchor) const
    {
        // Todo
    }*/
    
    
    void Disk::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu=Nv=64;
    }
    
    
    void Disk::finalize()
    {
        int NFc=1;
        F_arr.resize(NFc);
        
        bbox.xm=-0.05*radius;
        bbox.xp=+0.05*radius;
        
        bbox.ym=-1.1*radius;
        bbox.yp=+1.1*radius;
        
        bbox.zm=-1.1*radius;
        bbox.zp=+1.1*radius;
        
        // Todo
        /*
        face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    void Disk::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,1> hits;
        std::array<int,1> face_labels={0};
        
        if(!ray_inter_plane_x(ray.start,ray.dir,0,hits[0])) hits[0]=-1;
        else
        {
            Vector3 P=ray.start+hits[0]*ray.dir;
            
            double r2=P.y*P.y+P.z*P.z;
            
            if(r2<in_radius*in_radius) hits[0]=-1;
            else if(r2>radius*radius) hits[0]=-1;
        }
        
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Disk::normal(RayInter const &inter) const
    {
        return -unit_vec_x;
    }
    
    
    /*void Box::set_parameters(double lx_, double ly_, double lz_)
    {
        // Todo
    }*/
    
    
    /*Vector3 Disk::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    void Disk::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        u=y/radius;
        v=z/radius;
        
        u=u/2.0+0.5;
        v=v/2.0+0.5;
    }
}
