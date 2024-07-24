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
    //   Lens
    //##########
    
    Lens::Lens(BoundingBox &bbox_,
               std::vector<Sel::SelFace> &F_arr_,
               std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         thickness(0.01), radius_front(0.3), radius_back(-0.3), max_outer_radius(0.1)
    {
    }
    
    
    Vector3 Lens::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(-thickness/1.0,0,0);
            case 2: return Vector3(+thickness/1.0,0,0);
            default: return Vector3(0);
        }
    }
    
    
    std::string Lens::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Front";
            case 2: return "Rear";
            case 3: return "F1";
            case 4: return "F2";
            case 5: return "P1";
            case 6: return "P2";
            default: return "Center";
        }
    }
    
    
    void Lens::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu=Nv=1;
    }
    
    
    void Lens::finalize()
    {
        double A,B,th_1,th_2;
        
        lens_geometry(A,B,outer_radius,th_1,th_2,
                      thickness,max_outer_radius,radius_front,radius_back);
                      
        center_front=Vector3(A,0,0);
        center_back=Vector3(B,0,0);
        
        cth_front=std::cos(th_1);
        cth_back=std::cos(th_2);
        
        if(radius_front>=0) normal_front=-unit_vec_x;
        else normal_front=unit_vec_x;
        
        if(radius_back>=0) normal_back=-unit_vec_x;
        else normal_back=unit_vec_x;
        
        double x1=A-radius_front*cth_front;
        double x2=B-radius_back*cth_back;
        
        cylinder_origin=Vector3(x1,0,0);
        cylinder_length=x2-x1;
        cylinder_direction=unit_vec_x;
        
        int NFc=3;
        F_arr.resize(NFc);
        
        // Bounding box
        
        x1=std::min(cylinder_origin.x,center_front.x-radius_front);
        x2=std::max(cylinder_origin.x+cylinder_length,center_back.x-radius_back);
        
        bbox.xm=1.1*x1;
        bbox.xp=1.1*x2;
        
        bbox.ym=-1.1*outer_radius;
        bbox.yp=+1.1*outer_radius;
        
        bbox.zm=-1.1*outer_radius;
        bbox.zp=+1.1*outer_radius;
        
        // Todo
        /*face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    void Lens::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const
    {
        std::array<double,6> hits;
        std::array<int,6> face_labels={0,0,1,1,2,2};
        
        RayInter inter_out;
        
        if(!ray_inter_coupola(ray.start,ray.dir,center_front,normal_front,radius_front,cth_front,hits[0],hits[1]))
        {
            hits[0]=-1;
            hits[1]=-1;
        }
        if(!ray_inter_coupola(ray.start,ray.dir,center_back,normal_back,radius_back,cth_back,hits[2],hits[3]))
        {
            hits[2]=-1;
            hits[3]=-1;
        }
        if(!ray_inter_cylinder(ray.start,ray.dir,cylinder_origin,cylinder_direction,outer_radius,cylinder_length,hits[4],hits[5]))
        {
            hits[4]=-1;
            hits[5]=-1;
        }
        
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Lens::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        
        if(inter.face==0)
            Fnorm=+sgn(radius_front)*(Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-center_front);
        else if(inter.face==1)
            Fnorm=-sgn(radius_back)*(Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-center_back);
        else
        {
            Fnorm=Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-cylinder_origin;
            Fnorm=Fnorm.transverse(cylinder_direction);
            
            Fnorm=Vector3(0,inter.obj_y,inter.obj_z);
        }
        
        Fnorm.normalize();
        return Fnorm;
    }
    
    
    void Lens::set_parameters(double thickness_,
                              double r_max_,
                              double r1_,
                              double r2_)
    {
        thickness=thickness_;
        radius_front=r1_;
        radius_back=r2_;
        max_outer_radius=r_max_;
    }
    
    
    /*Vector3 Lens::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    void Lens::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        u=v=0; // Todo
    }
}
