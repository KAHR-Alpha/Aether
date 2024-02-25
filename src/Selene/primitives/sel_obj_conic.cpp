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
    //###########
    //   Conic
    //###########
    
    Conic::Conic(BoundingBox &bbox_,
                 std::vector<Sel::SelFace> &F_arr_,
                 std::vector<std::string> &face_name_arr_)
        :bbox(bbox_), F_arr(F_arr_), face_name_arr(face_name_arr_),
         R_factor(0.1), K_factor(0), in_radius(0), out_radius(0.1)
    {
    }
    
    
    Vector3 Conic::anchor(int anchor) const
    {
        double x,y;
        
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(R_factor,0,0);
            case 2:
                conic_near_focus(x,y,R_factor,K_factor);
                return Vector3(x,y,0);
            case 3:
                conic_far_focus(x,y,R_factor,K_factor);
                return Vector3(x,y,0);
            default: return Vector3(0);
        }
    }
    
    
    std::string Conic::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Vertex";
            case 1: return "Center";
            case 2: return "F1";
            case 3: return "F2";
            default: return "Vertex";
        }
    }
    
    
    void Conic::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu = Nv = 64;
    }
    
    
    void Conic::finalize()
    {
        int NFc=1;
        F_arr.resize(NFc);
        
        bbox.xm=-0.1*R_factor;
        
        if(K_factor>-1) bbox.xp=R_factor/(1.0+K_factor);
        else bbox.xp=conic_invert(out_radius,R_factor,K_factor);
        
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
    
    
    void Conic::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,2> hits;
        std::array<int,2> face_labels={0,0};
        
        ray_inter_conic_section_x(ray.start,ray.dir,
                                  R_factor,K_factor,in_radius,out_radius,
                                  hits[0],hits[1]);
                                  
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Conic::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        
        double x=inter.obj_x;
        double y=inter.obj_y;
        double z=inter.obj_z;
        //    double R2=y*y+z*z;
        
        Vector3 P(0,y,z);
        P.normalize();
        
        double factor=(R_factor-(1+K_factor)*x)/std::sqrt(2*R_factor*x-(1+K_factor)*x*x);
        
        Fnorm(factor,-P.y,-P.z);
        Fnorm.normalize();
        
        if(Fnorm.x>0) Fnorm=-Fnorm;
        
        return Fnorm;
    }
    
    
    /*void Conic::set_parameters(double lx_, double ly_, double lz_)
    {
        // Todo
    }*/
    
    
    /*Vector3 Conic::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        // Todo
    }*/
    
    
    /*void Conic::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        // Todo
    }*/
}
