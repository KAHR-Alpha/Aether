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
    //#####################
    //   Spherical Patch
    //#####################
    
    Spherical_Patch::Spherical_Patch(BoundingBox &bbox_,
                                     std::vector<Sel::SelFace> &F_arr_,
                                     std::vector<std::string> &face_name_arr_)
        :bbox(bbox_), F_arr(F_arr_), face_name_arr(face_name_arr_),
         radius(0.05), cut_factor(1.0)
    {
    }
    
    
    Vector3 Spherical_Patch::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(cut_x,0,0);
            default: return Vector3(0);
        }
    }


    std::string Spherical_Patch::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Slice_Center";
            default: return "Center";
        }
    }
    
    
    void Spherical_Patch::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu=Nv=33;
    }
    
    
    void Spherical_Patch::finalize()
    {
        cut_x=radius*(1.0-2.0*cut_factor);
        cut_angle=std::acos(1.0-2.0*cut_factor);
        cut_radius=radius*std::sin(cut_angle);
        
        int NFc=1;
        F_arr.resize(NFc);
        
        bbox.xm=-1.1*radius;
        bbox.xp=+1.1*radius;
        
        bbox.ym=-1.1*radius;
        bbox.yp=+1.1*radius;
        
        bbox.zm=-1.1*radius;
        bbox.zp=+1.1*radius;
        
        face_name_arr.resize(NFc);
        face_name_arr[0]="Sphere";
    }
    
    
    double Spherical_Patch::get_cut_factor() const { return cut_factor; }
    double Spherical_Patch::get_radius() const { return radius; }
    
    
    void Spherical_Patch::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,2> hits;
        std::array<int,2> face_labels={0,0};
        
        if(!ray_inter_sphere(ray.start,ray.dir,radius,hits[0],hits[1]))
        {
            hits[0]=-1;
            hits[1]=-1;
        }
        
        Vector3 P=ray.start+hits[0]*ray.dir;
        if(P.x<cut_x) hits[0]=-1;
        
        P=ray.start+hits[1]*ray.dir;
        if(P.x<cut_x) hits[1]=-1;
        
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    void Spherical_Patch::map_variables(std::map<std::string,double*> &variables_map)
    {
        variables_map["sphere_patch_radius"]=&radius;
        variables_map["sphere_patch_cut_factor"]=&cut_factor;
    }
    
    
    Vector3 Spherical_Patch::normal(RayInter const &inter) const
    {
        Vector3 Fnorm=Vector3(inter.obj_x,inter.obj_y,inter.obj_z);
        
        Fnorm.normalize();
        
        return Fnorm;
    }
    
    
    double& Spherical_Patch::ref_cut_factor() { return cut_factor; }
    double& Spherical_Patch::ref_radius() { return radius; }
    
    
    void Spherical_Patch::set_parameters(double radius_, double cut_factor_)
    {
        radius = radius_;
        cut_factor = cut_factor_;
    }
    
    
    /*Vector3 Spherical_Patch::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    void Spherical_Patch::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        double phi=std::atan2(z,y);
        double th=std::atan2(std::sqrt(y*y+z*z),x);
        
        u=th*std::cos(phi)/cut_angle;
        v=th*std::sin(phi)/cut_angle;
        
        u=u/2.0+0.5;
        v=v/2.0+0.5;
    }
}
