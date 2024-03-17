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
    //##############
    //   Parabola
    //##############
    
    Parabola::Parabola(BoundingBox &bbox_,
                       std::vector<Sel::SelFace> &F_arr_,
                       std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         focal(0.1), inner_radius(5e-3), length(0.02)
    {
    }
    
    
    Vector3 Parabola::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(focal,0,0);
            case 2: return Vector3(inner_radius*inner_radius/(4.0*focal),0,0);
            case 3: return Vector3(length,0,0);
            default: return Vector3(0);
        }
    }
    
    
    std::string Parabola::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Focus";
            case 2: return "In_center";
            case 3: return "Out_center";
            default: return "Center";
        }
    }
    
    
    void Parabola::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu=Nv=64;
    }
    
    
    void Parabola::finalize()
    {
        int NFc=1;
        F_arr.resize(NFc);
        
        bbox.xm=-0.1*length;
        bbox.xp=1.01*length;
        
        double out_radius=std::sqrt(4.0*focal*length)*1.01;
        
        bbox.ym=-out_radius;
        bbox.yp=+out_radius;
        
        bbox.zm=-out_radius;
        bbox.zp=+out_radius;
        
        // Todo
        /*face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    void Parabola::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,2> hits;
        std::array<int,2> face_labels={0,0};
        
        ray_inter_parabola_x(ray.start,ray.dir,
                             focal,inner_radius,length,
                             hits[0],hits[1]);
                             
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Parabola::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        
        Fnorm(-1,0.5/focal*inter.obj_y,0.5/focal*inter.obj_z);
        Fnorm.normalize();
        
        return Fnorm;
    }
    


    /*Vector3 Parabola::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    void Parabola::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        // Todo
        /*double phi=std::atan2(z,y);
        double th=std::atan2(std::sqrt(y*y+z*z),x);
        
        u=th*std::cos(phi)/sph_cut_th;
        v=th*std::sin(phi)/sph_cut_th;
        
        u=u/2.0+0.5;
        v=v/2.0+0.5;*/
        
        u=v=0;
    }
}
