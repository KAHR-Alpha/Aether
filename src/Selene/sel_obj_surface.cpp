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

#include <selene.h>

extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{
    
//###################
//   Conic Section
//###################

Vector3 Object::conic_section_anchor(int anchor)
{
    double x,y;
    
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(conic_R,0,0);
        case 2:
            conic_near_focus(x,y,conic_R,conic_K);
            return Vector3(x,y,0);
        case 3:
            conic_far_focus(x,y,conic_R,conic_K);
            return Vector3(x,y,0);
        default: return Vector3(0);
    }
}

std::string Object::conic_section_anchor_name(int anchor)
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

void Object::intersect_conic_section(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,2> hits;
    std::array<int,2> face_labels={0,0};
    
    ray_inter_conic_section_x(ray.start,ray.dir,
                              conic_R,conic_K,conic_in_radius,conic_out_radius,
                              hits[0],hits[1]);
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_conic_section(RayInter const &inter)
{
    Vector3 Fnorm;
    
    double x=inter.obj_x;
    double y=inter.obj_y;
    double z=inter.obj_z;
//    double R2=y*y+z*z;
    
    Vector3 P(0,y,z);
    P.normalize();
    
    double factor=(conic_R-(1+conic_K)*x)/std::sqrt(2*conic_R*x-(1+conic_K)*x*x);
    
    Fnorm(factor,-P.y,-P.z);
    Fnorm.normalize();
    
    if(Fnorm.x>0) Fnorm=-Fnorm;
    
    return Fnorm;
}

void Object::set_conic_section()
{
    type=OBJ_CONIC;
    
    NFc=1;
    F_arr.resize(NFc);
    
    bbox.xm=-0.1*conic_R;
    
    if(conic_K>-1) bbox.xp=conic_R/(1.0+conic_K);
    else bbox.xp=conic_invert(conic_out_radius,conic_R,conic_K);
      
    bbox.ym=-conic_out_radius;
    bbox.yp=+conic_out_radius;
    
    bbox.zm=-conic_out_radius;
    bbox.zp=+conic_out_radius;
}

//void Object::xyz_to_uv_parabola(double &u,double &v,int face_,double x,double y,double z)
//{
//    double phi=std::atan2(z,y);
//    double th=std::atan2(std::sqrt(y*y+z*z),x);
//    
//    u=th*std::cos(phi)/sph_cut_th;
//    v=th*std::sin(phi)/sph_cut_th;
//    
//    u=u/2.0+0.5;
//    v=v/2.0+0.5;
//}

void Object::default_N_uv_conic_section(int &Nu,int &Nv,int face_)
{
    Nu=Nv=64;
}

//##########
//   Disk
//##########

void Object::intersect_disk(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,1> hits;
    std::array<int,1> face_labels={0};
    
    if(!ray_inter_plane_x(ray.start,ray.dir,0,hits[0])) hits[0]=-1;
    else
    {
        Vector3 P=ray.start+hits[0]*ray.dir;
        
        double r2=P.y*P.y+P.z*P.z;
        
        if(r2<dsk_r_in*dsk_r_in) hits[0]=-1;
        else if(r2>dsk_r*dsk_r) hits[0]=-1;
    }
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_disk(RayInter const &inter)
{
    return -unit_vec_x;
}

void Object::set_disk()
{
    type=OBJ_DISK;
    
    NFc=1;
    F_arr.resize(NFc);
    
    bbox.xm=-0.05*dsk_r;
    bbox.xp=+0.05*dsk_r;
    
    bbox.ym=-1.1*dsk_r;
    bbox.yp=+1.1*dsk_r;
    
    bbox.zm=-1.1*dsk_r;
    bbox.zp=+1.1*dsk_r;
}

void Object::set_disk(double radius_,double in_radius_)
{
    dsk_r=radius_;
    dsk_r_in=in_radius_;
    
    set_disk();
}

void Object::xyz_to_uv_disk(double &u,double &v,int face_,double x,double y,double z)
{
    u=y/dsk_r;
    v=z/dsk_r;
    
    u=u/2.0+0.5;
    v=v/2.0+0.5;
}

void Object::default_N_uv_disk(int &Nu,int &Nv,int face_)
{
    Nu=Nv=64;
}

//##############
//   Parabola
//##############

Vector3 Object::parabola_anchor(int anchor)
{
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(pr_f,0,0);
        case 2: return Vector3(pr_in_radius*pr_in_radius/(4.0*pr_f),0,0);
        case 3: return Vector3(pr_length,0,0);
        default: return Vector3(0);
    }
}

std::string Object::parabola_anchor_name(int anchor)
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

void Object::intersect_parabola(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,2> hits;
    std::array<int,2> face_labels={0,0};
    
    ray_inter_parabola_x(ray.start,ray.dir,
                         pr_f,pr_in_radius,pr_length,
                         hits[0],hits[1]);
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_parabola(RayInter const &inter)
{
    Vector3 Fnorm;
    
    Fnorm(-1,0.5/pr_f*inter.obj_y,0.5/pr_f*inter.obj_z);
    Fnorm.normalize();
    
    return Fnorm;
}

void Object::set_parabola()
{
    type=OBJ_PARABOLA;
    
    NFc=1;
    F_arr.resize(NFc);
    
    bbox.xm=-0.1*pr_length;
    bbox.xp=1.01*pr_length;
    
    double out_radius=std::sqrt(4.0*pr_f*pr_length)*1.01;
    
    bbox.ym=-out_radius;
    bbox.yp=+out_radius;
    
    bbox.zm=-out_radius;
    bbox.zp=+out_radius;
}

void Object::set_parabola(double focal_length_,double in_radius_,double length_)
{
    pr_f=focal_length_;
    pr_in_radius=in_radius_;
    pr_length=length_;
    
    set_parabola();
}

void Object::xyz_to_uv_parabola(double &u,double &v,int face_,double x,double y,double z)
{
    double phi=std::atan2(z,y);
    double th=std::atan2(std::sqrt(y*y+z*z),x);
    
    u=th*std::cos(phi)/sph_cut_th;
    v=th*std::sin(phi)/sph_cut_th;
    
    u=u/2.0+0.5;
    v=v/2.0+0.5;
}

void Object::default_N_uv_parabola(int &Nu,int &Nv,int face_)
{
    Nu=Nv=64;
}

//###############
//   Rectangle
//###############

void Object::intersect_rectangle(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,1> hits;
    std::array<int,1> face_labels={0};
    
    if(!ray_inter_plane_x(ray.start,ray.dir,0,hits[0])) hits[0]=-1;
    else
    {
        Vector3 P=ray.start+hits[0]*ray.dir;
        
        if(std::abs(P.y)>box_ly/2.0 || std::abs(P.z)>box_lz/2.0)
            hits[0]=-1;
    }
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_rectangle(RayInter const &inter)
{
    return -unit_vec_x;
}

Vector3 Object::tangent_rectangle(RayInter const &inter,Vector3 const &normal,bool up)
{
    Sel::SelFace &face=F_arr[0];
    
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

void Object::set_rectangle()
{
    type=OBJ_RECTANGLE;
    
    NFc=1;
    F_arr.resize(NFc);
    
    double span=std::max(box_ly,box_lz);
    
    bbox.xm=-0.05*span;
    bbox.xp=+0.05*span;
    
    bbox.ym=-1.1*box_ly/2.0;
    bbox.yp=+1.1*box_ly/2.0;
    
    bbox.zm=-1.1*box_lz/2.0;
    bbox.zp=+1.1*box_lz/2.0;
}

void Object::set_rectangle(double ly_,double lz_)
{
    box_ly=ly_;
    box_lz=lz_;
    
    set_rectangle();
}

void Object::xyz_to_uv_rectangle(double &u,double &v,int face_,double x,double y,double z)
{
    u=0.5+y/box_ly;
    v=0.5+z/box_lz;
}

void Object::default_N_uv_rectangle(int &Nu,int &Nv,int face_)
{
    double delta=std::max(box_ly,box_lz)/64.0;
    
    Nu=nearest_2np1(box_ly/delta);
    Nv=nearest_2np1(box_lz/delta);
}

//#####################
//   Spherical Patch
//#####################

void Object::intersect_spherical_patch(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,2> hits;
    std::array<int,2> face_labels={0,0};
    
    if(!ray_inter_sphere(ray.start,ray.dir,sph_r,hits[0],hits[1]))
    {
        hits[0]=-1;
        hits[1]=-1;
    }
    
    Vector3 P=ray.start+hits[0]*ray.dir;
    if(P.x<sph_cut_x) hits[0]=-1;
    
    P=ray.start+hits[1]*ray.dir;
    if(P.x<sph_cut_x) hits[1]=-1;
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_spherical_patch(RayInter const &inter)
{
    Vector3 Fnorm=Vector3(inter.obj_x,inter.obj_y,inter.obj_z);
    
    Fnorm.normalize();
    
    return Fnorm;
}

void Object::set_spherical_patch()
{
    sph_cut_x=sph_r*(1.0-2.0*sph_cut);
    sph_cut_th=std::acos(1.0-2.0*sph_cut);
    sph_cut_rad=sph_r*std::sin(sph_cut_th);
    
    type=OBJ_SPHERE_PATCH;
    
    NFc=1;
    F_arr.resize(NFc);
    
    bbox.xm=-1.1*sph_r;
    bbox.xp=+1.1*sph_r;
    
    bbox.ym=-1.1*sph_r;
    bbox.yp=+1.1*sph_r;
    
    bbox.zm=-1.1*sph_r;
    bbox.zp=+1.1*sph_r;
    
    face_name_arr.resize(NFc);
    face_name_arr[0]="Sphere";
}

void Object::set_spherical_patch(double radius,double slice)
{
    sph_r=radius;
    sph_cut=slice;
    
    set_spherical_patch();
}

void Object::xyz_to_uv_spherical_patch(double &u,double &v,int face_,double x,double y,double z)
{
    double phi=std::atan2(z,y);
    double th=std::atan2(std::sqrt(y*y+z*z),x);
    
    u=th*std::cos(phi)/sph_cut_th;
    v=th*std::sin(phi)/sph_cut_th;
    
    u=u/2.0+0.5;
    v=v/2.0+0.5;
}

void Object::default_N_uv_spherical_patch(int &Nu,int &Nv,int face_)
{
    Nu=Nv=33;
}

}
