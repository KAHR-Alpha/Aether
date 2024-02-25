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

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel::Primitives
{
    //##############
    //   Cylinder
    //##############
    
    Cylinder::Cylinder(BoundingBox &bbox_,
                       std::vector<Sel::SelFace> &F_arr_,
                       std::vector<std::string> &face_name_arr_)
        :bbox(bbox_), F_arr(F_arr_), face_name_arr(face_name_arr_),
         radius(1e-2), length(4e-2), cut_factor(1.0)
    {
    }
    
    
    Vector3 Cylinder::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(-length/2.0,0,0);
            case 2: return Vector3(+length/2.0,0,0);
            default: return Vector3(0);
        }
    }
    
    
    std::string Cylinder::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Face_XM";
            case 2: return "Face_XP";
            default: return "Center";
        }
    }
    
    
    void Cylinder::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        double delta=std::max(length,2.0*radius)/64;
        
        switch(face_)
        {
            case 0:
                Nu=Nv=nearest_2np1(2.0*radius/delta);
                break;
            case 1:
                Nu=Nv=nearest_2np1(2.0*radius/delta);
                break;
            case 2:
                Nu=nearest_2np1(2.0*Pi*radius/delta);
                Nv=nearest_2np1(length/delta);
                break;
        }
    }
    
    
    void Cylinder::finalize()
    {
        int NFc=4;
        F_arr.resize(NFc);
        
        bbox.xm=-length/2.0;
        bbox.xp=+length/2.0;
        
        bbox.ym=-radius;
        bbox.yp=+radius;
        
        bbox.zm=-radius;
        bbox.zp=+radius;
        
        // Todo
        /*face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    void Cylinder::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const
    {
        std::array<double,4> hits;
        std::array<int,4> face_labels={0,1,2,2};
        
        double l2=length/2.0;
        
        if(!ray_inter_disk_x(ray.start,ray.dir,-l2,0,radius,hits[0])) hits[0]=-1;
        if(!ray_inter_disk_x(ray.start,ray.dir,+l2,0,radius,hits[1])) hits[1]=-1;
        if(!ray_inter_cylinder_x(ray.start,ray.dir,-l2,radius,length,hits[2],hits[3]))
        {
            hits[2]=-1;
            hits[3]=-1;
        }
        
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Cylinder::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        
        if(inter.face==0) Fnorm=-unit_vec_x;
        else if(inter.face==1) Fnorm=+unit_vec_x;
        else
        {
            Fnorm=Vector3(0,inter.obj_y,inter.obj_z);
        }
        
        Fnorm.normalize();
        return Fnorm;
    }
    
    
    void Cylinder::set_parameters(double length_, double radius_, double cut_)
    {
        length = length_;
        radius = radius_;
        cut_factor = cut_;
    }
    
    
    /*Vector3 Cylinder::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    void Cylinder::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        switch(face_)
        {
            case 0:
                u=0.5+y/radius/2.0; v=0.5+z/radius/2.0;
                break;
            case 1:
                u=0.5+y/radius/2.0; v=0.5+z/radius/2.0;
                break;
            case 2:
                u=std::atan2(z,y)+Pi/2.0;
                if(u<0) u+=2.0*Pi;
                u/=2.0*Pi;
                v=0.5+x/length;
                break;
        }
    }
}
