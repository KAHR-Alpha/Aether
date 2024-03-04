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
    //##########
    //   Cone
    //##########
    
    Cone::Cone(BoundingBox &bbox_,
               std::vector<Sel::SelFace> &F_arr_,
               std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         radius(1e-2), length(4e-2), cut_factor(1.0)
    {
    }
    
    
    Vector3 Cone::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(-length/2.0,0,0);
            case 2: return Vector3(+length/2.0,0,0);
            default: return Vector3(0);
        }
    }
    
    
    std::string Cone::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Face";
            case 2: return "Tip";
            default: return "Center";
        }
    }
    
    
    /*void Cone::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        // Todo
    }*/
    
    
    void Cone::finalize()
    {
        double a=std::atan(radius/length);
        cone_ca=std::cos(a);
        cone_sa=std::sin(a);
        
        int NFc=2;
        F_arr.resize(NFc);
        
        bbox.xm=-length/2.0;
        bbox.xp=+length/2.0;
        
        bbox.ym=-radius;
        bbox.yp=+radius;
        
        bbox.zm=-radius;
        bbox.zp=+radius;
    }
    
    
    void Cone::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const
    {
        std::array<double,3> hits;
        std::array<int,3> face_labels={0,1,1};
        
        double l2=length/2.0;
        
        if(!ray_inter_disk_x(ray.start,ray.dir,-l2,0,radius,hits[0])) hits[0]=-1;
        if(!ray_inter_cone_x(ray.start,ray.dir,-l2,radius,length,hits[1],hits[2]))
        {
            hits[1]=-1;
            hits[2]=-1;
        }
        
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Cone::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        int const &face_inter=inter.face;
        
        if(face_inter==0) Fnorm=-unit_vec_x;
        else
        {
            Fnorm=Vector3(0,inter.obj_y,inter.obj_z);
            Fnorm.normalize();
            
            Fnorm=cone_ca*Fnorm+cone_sa*unit_vec_x;
        }
        
        Fnorm.normalize();
        return Fnorm;
    }
    
    
    void Cone::set_parameters(double radius_,
                              double length_,
                              double cut_)
    {
        radius = radius_;
        length = length_;
        cut_factor = cut_;
    }
    
    
    /*Vector3 Cone::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    /*void Cone::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        // Todo
    }*/
}
