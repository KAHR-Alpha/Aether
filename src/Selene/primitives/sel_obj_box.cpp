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

extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{
    //#########
    //   Box
    //#########

    void Object::intersect_box(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward)
    {
        std::array<double,6> hits;
        std::array<int,6> face_labels={0,1,2,3,4,5};

        double &t_x1=hits[0],&t_x2=hits[1];
        double &t_y1=hits[2],&t_y2=hits[3];
        double &t_z1=hits[4],&t_z2=hits[5];

        t_x1=(-box_lx/2.0-ray.start.x)*ray.inv_dir.x;
        t_x2=(+box_lx/2.0-ray.start.x)*ray.inv_dir.x;
        
        t_y1=(-box_ly/2.0-ray.start.y)*ray.inv_dir.y;
        t_y2=(+box_ly/2.0-ray.start.y)*ray.inv_dir.y;
        
        t_z1=(-box_lz/2.0-ray.start.z)*ray.inv_dir.z;
        t_z2=(+box_lz/2.0-ray.start.z)*ray.inv_dir.z;

        Vector3 V;

        V=ray.start+t_x1*ray.dir;
        if(V.y>box_ly/2.0 || V.y<-box_ly/2.0 ||
           V.z>box_lz/2.0 || V.z<-box_lz/2.0) t_x1=-1;
        V=ray.start+t_x2*ray.dir;
        if(V.y>box_ly/2.0 || V.y<-box_ly/2.0 ||
           V.z>box_lz/2.0 || V.z<-box_lz/2.0) t_x2=-1;

        V=ray.start+t_y1*ray.dir;
        if(V.x>box_lx/2.0 || V.x<-box_lx/2.0 ||
           V.z>box_lz/2.0 || V.z<-box_lz/2.0) t_y1=-1;
        V=ray.start+t_y2*ray.dir;
        if(V.x>box_lx/2.0 || V.x<-box_lx/2.0 ||
           V.z>box_lz/2.0 || V.z<-box_lz/2.0) t_y2=-1;

        V=ray.start+t_z1*ray.dir;
        if(V.x>box_lx/2.0 || V.x<-box_lx/2.0 ||
           V.y>box_ly/2.0 || V.y<-box_ly/2.0) t_z1=-1;
        V=ray.start+t_z2*ray.dir;
        if(V.x>box_lx/2.0 || V.x<-box_lx/2.0 ||
           V.y>box_ly/2.0 || V.y<-box_ly/2.0) t_z2=-1;

        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }

    Vector3 Object::normal_box(RayInter const &inter)
    {
        Vector3 Fnorm;
        int const &face_inter=inter.face;

             if(face_inter==0) Fnorm=-unit_vec_x;
        else if(face_inter==1) Fnorm= unit_vec_x;
        else if(face_inter==2) Fnorm=-unit_vec_y;
        else if(face_inter==3) Fnorm= unit_vec_y;
        else if(face_inter==4) Fnorm=-unit_vec_z;
        else if(face_inter==5) Fnorm= unit_vec_z;
        
        return Fnorm;
    }

    Vector3 Object::tangent_box(RayInter const &inter,Vector3 const &normal,bool up)
    {
        int const &face_inter=inter.face;

        Sel::SelFace &face=F_arr[face_inter];

        int tangent_type;
        Vector3 tangent;

        if(up)
        {
            tangent_type=face.tangent_up;
            tangent=face.fixed_tangent_up;
        }
        else
        {
            tangent_type=face.tangent_down;
            tangent=face.fixed_tangent_down;
        }

             if(tangent_type==TANGENT_UNSET) return Vector3(0);
        else if(tangent_type==TANGENT_FIXED)
        {
            tangent=tangent-normal*scalar_prod(normal,tangent);
            tangent.normalize();
        }
        else
        {
            switch(tangent_type)
            {
            }
        }

        return tangent;
    }

    void Object::set_box()
    {
        type=OBJ_BOX;

        NFc=6;
        F_arr.resize(NFc);

        bbox.xm=-box_lx/2.0;
        bbox.xp=+box_lx/2.0;

        bbox.ym=-box_ly/2.0;
        bbox.yp=+box_ly/2.0;

        bbox.zm=-box_lz/2.0;
        bbox.zp=+box_lz/2.0;

        face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";
    }

    void Object::set_box(double lx_,double ly_,double lz_)
    {
        box_lx=lx_;
        box_ly=ly_;
        box_lz=lz_;

        set_box();
    }

    Vector3 Object::box_anchor(int anchor)
    {
        double x=box_lx/2.0;
        double y=box_ly/2.0;
        double z=box_lz/2.0;

        switch(anchor)
        {
            case 0: return Vector3(0);
            case 1: return Vector3(-x,0,0);
            case 2: return Vector3(+x,0,0);
            case 3: return Vector3(0,-y,0);
            case 4: return Vector3(0,+y,0);
            case 5: return Vector3(0,0,-z);
            case 6: return Vector3(0,0,+z);
            case 7: return Vector3(-x,-y,-z);
            case 8: return Vector3(+x,-y,-z);
            case 9: return Vector3(-x,+y,-z);
            case 10: return Vector3(+x,+y,-z);
            case 11: return Vector3(-x,-y,+z);
            case 12: return Vector3(+x,-y,+z);
            case 13: return Vector3(-x,+y,+z);
            case 14: return Vector3(+x,+y,+z);
            default: return Vector3(0);
        }
    }

    std::string Object::box_anchor_name(int anchor)
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Face_XM";
            case 2: return "Face_XP";
            case 3: return "Face_YM";
            case 4: return "Face_YP";
            case 5: return "Face_ZM";
            case 6: return "Face_ZP";
            case 7: return "Corner_XM_YM_ZM";
            case 8: return "Corner_XP_YM_ZM";
            case 9: return "Corner_XM_YP_ZM";
            case 10: return "Corner_XP_YP_ZM";
            case 11: return "Corner_XM_YM_ZP";
            case 12: return "Corner_XP_YM_ZP";
            case 13: return "Corner_XM_YP_ZP";
            case 14: return "Corner_XP_YP_ZP";
            default: return "Center";
        }
    }

    void Object::xyz_to_uv_box(double &u,double &v,int face_,double x,double y,double z)
    {
        switch(face_)
        {
            case 0:
                u=y/box_ly; v=z/box_lz;
                break;
            case 1:
                u=y/box_ly; v=z/box_lz;
                break;
            case 2:
                u=x/box_lx; v=z/box_lz;
                break;
            case 3:
                u=x/box_lx; v=z/box_lz;
                break;
            case 4:
                u=x/box_lx; v=y/box_ly;
                break;
            case 5:
                u=x/box_lx; v=y/box_ly;
                break;
        }

        u+=0.5;
        v+=0.5;
    }

    int nearest_2np1(double val)
    {
        int N=nearest_integer((val-1.0)/2.0);
        return 2*N+1;
    }

    void Object::default_N_uv_box(int &Nu,int &Nv,int face_)
    {
        double delta=var_max(box_lx,box_ly,box_lz)/64.0;

        int Nx=nearest_2np1(box_lx/delta);
        int Ny=nearest_2np1(box_ly/delta);
        int Nz=nearest_2np1(box_lz/delta);

        switch(face_)
        {
            case 0:
                Nu=Ny; Nv=Nz; break;
            case 1:
                Nu=Ny; Nv=Nz; break;
            case 2:
                Nu=Nx; Nv=Nz; break;
            case 3:
                Nu=Nx; Nv=Nz; break;
            case 4:
                Nu=Nx; Nv=Ny; break;
            case 5:
                Nu=Nx; Nv=Ny; break;
        }
    }
}
