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
    //   Lens
    //##########
    
    Lens::Lens(BoundingBox &bbox_,
               std::vector<Sel::SelFace> &F_arr_,
               std::vector<std::string> &face_name_arr_)
        :bbox(bbox_), F_arr(F_arr_), face_name_arr(face_name_arr_),
         ls_thickness(0.01), ls_r1(0.3), ls_r2(-0.3), ls_r_max_nominal(0.1)
    {
    }
    
    
    Vector3 Lens::anchor(int anchor) const
    {
        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(-ls_thickness/1.0,0,0);
            case 2: return Vector3(+ls_thickness/1.0,0,0);
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

        lens_geometry(A,B,ls_r_max,th_1,th_2,
                      ls_thickness,ls_r_max_nominal,ls_r1,ls_r2);

        ls_c1=Vector3(A,0,0);
        ls_c2=Vector3(B,0,0);

        ls_cth_1=std::cos(th_1);
        ls_cth_2=std::cos(th_2);

        if(ls_r1>=0) ls_N1=-unit_vec_x;
        else ls_N1=unit_vec_x;

        if(ls_r2>=0) ls_N2=-unit_vec_x;
        else ls_N2=unit_vec_x;

        double x1=A-ls_r1*ls_cth_1;
        double x2=B-ls_r2*ls_cth_2;

        ls_cyl_pos=Vector3(x1,0,0);
        ls_cyl_h=x2-x1;
        ls_cyl_N=unit_vec_x;

        int NFc=3;
        F_arr.resize(NFc);

        // Bounding box

        x1=std::min(ls_cyl_pos.x,ls_c1.x-ls_r1);
        x2=std::max(ls_cyl_pos.x+ls_cyl_h,ls_c2.x-ls_r2);

        bbox.xm=1.1*x1;
        bbox.xp=1.1*x2;

        bbox.ym=-1.1*ls_r_max;
        bbox.yp=+1.1*ls_r_max;

        bbox.zm=-1.1*ls_r_max;
        bbox.zp=+1.1*ls_r_max;
        
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

        if(!ray_inter_coupola(ray.start,ray.dir,ls_c1,ls_N1,ls_r1,ls_cth_1,hits[0],hits[1]))
        {
            hits[0]=-1;
            hits[1]=-1;
        }
        if(!ray_inter_coupola(ray.start,ray.dir,ls_c2,ls_N2,ls_r2,ls_cth_2,hits[2],hits[3]))
        {
            hits[2]=-1;
            hits[3]=-1;
        }
        if(!ray_inter_cylinder(ray.start,ray.dir,ls_cyl_pos,ls_cyl_N,ls_r_max,ls_cyl_h,hits[4],hits[5]))
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
            Fnorm=+sgn(ls_r1)*(Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-ls_c1);
        else if(inter.face==1)
            Fnorm=-sgn(ls_r2)*(Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-ls_c2);
        else
        {
            Fnorm=Vector3(inter.obj_x,inter.obj_y,inter.obj_z)-ls_cyl_pos;
            Fnorm=Fnorm.transverse(ls_cyl_N);

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
        ls_thickness=thickness_;
        ls_r1=r1_;
        ls_r2=r2_;
        ls_r_max_nominal=r_max_;
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
