/*Copyright 2008-2022 - Lo?c Le Cunff

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

extern const double Pi;
extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{

//##############
//   Booleans
//##############

void Object::intersect_boolean(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    int i;
    bool in_obj_1=false,in_obj_2=false;
    
    // Getting all the intersections of both object
    
    bool_buffer_1.clear();
    bool_buffer_2.clear();
    
    if(face_last_intersect>=bool_obj_1->NFc)
    {
        face_last_intersect-=bool_obj_1->NFc;
        bool_obj_1->intersect(ray,bool_buffer_1,-1,false);
        bool_obj_2->intersect(ray,bool_buffer_2,face_last_intersect,false);
    }
    else
    {
        bool_obj_1->intersect(ray,bool_buffer_1,face_last_intersect,false);
        bool_obj_2->intersect(ray,bool_buffer_2,-1,false);
    }
    
    int N1=bool_buffer_1.size();
    int N2=bool_buffer_2.size();
    
    if(N1==0) return;
    if(boolean_type==INTERSECT && N2==0) return;
    
    //Merging intersections
    std::vector<RayInter> &full_buffer=bool_buffer_1;
    
    full_buffer.resize(N1+N2);
    
    for(i=0;i<N2;i++)
        full_buffer[i+N1]=bool_buffer_2[i];
    
    std::sort(full_buffer.begin(),full_buffer.end());
    
    in_obj_1=(N1%2!=0);
    in_obj_2=(N2%2!=0);
    
    RayInter inter_out;
    bool check_1=true,check_2=true; // Intersection
    
         if(boolean_type==EXCLUDE) check_2=false;
    else if(boolean_type==UNION) check_1=check_2=false;
    
    if(first_forward)
    {
        bool second_hit=false;
        
        for(i=0;i<N1+N2;i++)
        {
            if(full_buffer[i].object==0)
            {
                if(in_obj_2==check_2) { inter_out=full_buffer[i]; break; }
                in_obj_1=!in_obj_1;
            }
            else
            {
                if(in_obj_1==check_1) { inter_out=full_buffer[i]; second_hit=true; break; }
                in_obj_2=!in_obj_2;
            }
        }
        
        inter_out.object=obj_ID;
        if(second_hit) inter_out.face+=bool_obj_1->NFc;
        
        interlist.push_back(inter_out);
    }
    else
    {
        for(i=0;i<N1+N2;i++)
        {
            if(full_buffer[i].object==0)
            {
                if(in_obj_2==check_2)
                {
                    inter_out=full_buffer[i];
                    inter_out.object=obj_ID;
                    interlist.push_back(inter_out);
                }
                in_obj_1=!in_obj_1;
            }
            else
            {
                if(in_obj_1==check_1)
                {
                    inter_out=full_buffer[i];
                    inter_out.object=obj_ID;
                    inter_out.face+=bool_obj_1->NFc;
                    interlist.push_back(inter_out);
                }
                in_obj_2=!in_obj_2;
            }
        }
    }
}

Vector3 Object::normal_boolean(RayInter inter)
{
    int &face_inter=inter.face;
    
    if(face_inter<bool_obj_1->NFc) return bool_obj_1->face_normal(inter);
    else
    {
        face_inter-=bool_obj_1->NFc;
        return bool_obj_2->face_normal(inter);
    }
}

void Object::set_boolean(Object *bool_obj_1_,Object *bool_obj_2_,Boolean_Type type_)
{
    type=OBJ_BOOLEAN;
    boolean_type=type_;
    
    bool_obj_1=bool_obj_1_;
    bool_obj_2=bool_obj_2_;
    
    bool_obj_1->obj_ID=0;
    bool_obj_2->obj_ID=1;
    
    NFc=bool_obj_1->NFc+bool_obj_2->NFc;
}

//#########
//   Box
//#########

void Object::intersect_box(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
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
    
    bxm=-box_lx/2.0;
    bxp=+box_lx/2.0;
    
    bym=-box_ly/2.0;
    byp=+box_ly/2.0;
    
    bzm=-box_lz/2.0;
    bzp=+box_lz/2.0;
    
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

//##########
//   Cone
//##########

void Object::intersect_cone_volume(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,3> hits;
    std::array<int,3> face_labels={0,1,1};
    
    double l2=cone_l/2.0;
    
    if(!ray_inter_disk_x(ray.start,ray.dir,-l2,0,cone_r,hits[0])) hits[0]=-1;
    if(!ray_inter_cone_x(ray.start,ray.dir,-l2,cone_r,cone_l,hits[1],hits[2]))
    {
        hits[1]=-1;
        hits[2]=-1;
    }
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_cone_volume(RayInter const &inter)
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

void Object::set_cone_volume()
{
    type=OBJ_VOL_CONE;
    
    double a=std::atan(cone_r/cone_l);
    cone_ca=std::cos(a);
    cone_sa=std::sin(a);
    
    NFc=2;
    F_arr.resize(NFc);
    
    bxm=-cone_l/2.0;
    bxp=+cone_l/2.0;
    
    bym=-cone_r;
    byp=+cone_r;
    
    bzm=-cone_r;
    bzp=+cone_r;
}

void Object::set_cone_volume(double radius,double length,double cut)
{
    cone_r=radius;
    cone_l=length;
    cone_cut=cut;
    
    set_cone_volume();
}

Vector3 Object::cone_anchor(int anchor)
{
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(-cone_l/2.0,0,0);
        case 2: return Vector3(+cone_l/2.0,0,0);
        default: return Vector3(0);
    }
}

std::string Object::cone_anchor_name(int anchor)
{
    switch(anchor)
    {
        case 0: return "Center";
        case 1: return "Face";
        case 2: return "Tip";
        default: return "Center";
    }
}

//##############
//   Cylinder
//##############

void Object::intersect_cylinder_volume(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,4> hits;
    std::array<int,4> face_labels={0,1,2,2};
    
    double l2=cyl_l/2.0;
    
    if(!ray_inter_disk_x(ray.start,ray.dir,-l2,0,cyl_r,hits[0])) hits[0]=-1;
    if(!ray_inter_disk_x(ray.start,ray.dir,+l2,0,cyl_r,hits[1])) hits[1]=-1;
    if(!ray_inter_cylinder_x(ray.start,ray.dir,-l2,cyl_r,cyl_l,hits[2],hits[3]))
    {
        hits[2]=-1;
        hits[3]=-1;
    }
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_cylinder_volume(RayInter const &inter)
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

void Object::set_cylinder_volume()
{
    type=OBJ_VOL_CYLINDER;
    
    NFc=4;
    F_arr.resize(NFc);
    
    bxm=-cyl_l/2.0;
    bxp=+cyl_l/2.0;
    
    bym=-cyl_r;
    byp=+cyl_r;
    
    bzm=-cyl_r;
    bzp=+cyl_r;
}

void Object::set_cylinder_volume(double length,double radius,double cut)
{
    cyl_l=length;
    cyl_r=radius;
    cyl_cut=cut;
    
    set_cylinder_volume();
}

Vector3 Object::cylinder_anchor(int anchor)
{
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(-cyl_l/2.0,0,0);
        case 2: return Vector3(+cyl_l/2.0,0,0);
        default: return Vector3(0);
    }
}

std::string Object::cylinder_anchor_name(int anchor)
{
    switch(anchor)
    {
        case 0: return "Center";
        case 1: return "Face_XM";
        case 2: return "Face_XP";
        default: return "Center";
    }
}

void Object::xyz_to_uv_cylinder_volume(double &u,double &v,int face_,double x,double y,double z)
{
    switch(face_)
    {
        case 0:
            u=0.5+y/cyl_r/2.0; v=0.5+z/cyl_r/2.0;
            break;
        case 1:
            u=0.5+y/cyl_r/2.0; v=0.5+z/cyl_r/2.0;
            break;
        case 2:
            u=std::atan2(z,y)+Pi/2.0;
            if(u<0) u+=2.0*Pi;
            u/=2.0*Pi;
            v=0.5+x/cyl_l;
            break;
    }
}

void Object::default_N_uv_cylinder_volume(int &Nu,int &Nv,int face_)
{
    double delta=std::max(cyl_l,2.0*cyl_r)/64;
    
    switch(face_)
    {
        case 0:
            Nu=Nv=nearest_2np1(2.0*cyl_r/delta);
            break;
        case 1:
            Nu=Nv=nearest_2np1(2.0*cyl_r/delta);
            break;
        case 2:
            Nu=nearest_2np1(2.0*Pi*cyl_r/delta);
            Nv=nearest_2np1(cyl_l/delta);
            break;
    }
}

//##########
//   Lens
//##########

void Object::intersect_lens(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
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

Vector3 Object::normal_lens(RayInter const &inter)
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

void Object::set_lens()
{
    type=OBJ_LENS;
    
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
    
    NFc=3;
    F_arr.resize(NFc);
    
    compute_boundaries();
}

void Object::set_lens(double thickness,double r_max,double r1,double r2)
{
    ls_thickness=thickness;
    ls_r1=r1;
    ls_r2=r2;
    ls_r_max_nominal=r_max;
    
    set_lens();
}

Vector3 Object::lens_anchor(int anchor)
{
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(-ls_thickness/1.0,0,0);
        case 2: return Vector3(+ls_thickness/1.0,0,0);
        default: return Vector3(0);
    }
}

std::string Object::lens_anchor_name(int anchor)
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

void Object::xyz_to_uv_lens(double &u,double &v,int face_,double x,double y,double z)
{
    u=v=0;
}

void Object::default_N_uv_lens(int &Nu,int &Nv,int face_)
{
    Nu=Nv=1;
}

//##########
//   Mesh
//##########

void Object::add_mesh(std::vector<Sel::Vertex> const &V_arr_,std::vector<Sel::SelFace> const &F_arr_)
{
    if(type==OBJ_MESH)
    {
        int NVtx=V_arr.size();
        
        int aNVtx=V_arr_.size();
        int aNFc=F_arr_.size();
        
        V_arr.resize(NVtx+aNVtx);
        F_arr.resize(NFc+aNFc);
        
        for(int i=NVtx;i<NVtx+aNVtx;i++)
            V_arr[i]=V_arr_[i-NVtx];
        
        for(int i=NFc;i<NFc+aNFc;i++)
        {
            F_arr[i]=F_arr_[i-NFc];
            
            F_arr[i].V1+=NVtx;
            F_arr[i].V2+=NVtx;
            F_arr[i].V3+=NVtx;
        }
        
        NVtx+=aNVtx;
        NFc+=aNFc;
        
        compute_boundaries();
    }
}

void Object::auto_recalc_normals()
{
    if(type==OBJ_MESH)
    {
        int i;
        
        Vector3 O,V;
        
        for(i=0;i<NFc;i++)
        {
            F_arr[i].comp_norm(V_arr);
            
            O=F_arr[i].get_cmass(V_arr);
            V.rand_sph(0.1);
            V+=F_arr[i].norm;
            V.normalize();
            
            int N_inter_1=0;
                
            if(!has_octree)
            {
                N_inter_1=ray_N_inter(V_arr,F_arr,i,O, V);
    //            int N_inter_2=ray_N_inter(V_arr,F_arr,i,O,-V);
            }
            else
            {
                SelRay ray;
                std::list<int> flist;
                
                ray.set_dir(V);
                ray.set_start(O);
                
                octree.ray_check(ray,flist);
                N_inter_1=ray_N_inter_list(V_arr,F_arr,flist,i,O,V);
                
                if(N_inter_1==2)
                {
                    
                }
            }
            
            if(N_inter_1%2!=0) F_arr[i].norm=-F_arr[i].norm;
        }
    }
}

void Object::intersect_mesh(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    if(first_forward)
    {
        int face_hit=-1;
        double t_intersec,u,v;
        
        if(!has_octree)
        {
            ray_inter(V_arr,F_arr,face_last_intersect,
                      ray.start,ray.dir,
                      face_hit,t_intersec,u,v);
        }
        else
        {
            octree.ray_check(ray,octree_buffer);
            ray_inter_vector(V_arr,F_arr,octree_buffer,face_last_intersect,
                             ray.start,ray.dir,
                             face_hit,t_intersec,u,v);
        }
        
        if(face_hit>-1)
        {
            Vector3 V=ray.start+t_intersec*ray.dir;
            
            RayInter inter_out;
            
//            inter_out.does_intersect=true;
            inter_out.obj_x=V.x;
            inter_out.obj_y=V.y;
            inter_out.obj_z=V.z;
            inter_out.t=t_intersec;
            inter_out.face=face_hit;
            inter_out.object=obj_ID;
            
            interlist.push_back(inter_out);
        }
    }
    else
    {
        face_intersect_buffer.clear();
        
        if(!has_octree)
        {
            ray_inter(V_arr,F_arr,face_last_intersect,
                      ray.start,ray.dir,face_intersect_buffer);
        }
        else
        {
            octree.ray_check(ray,octree_buffer);
            ray_inter_vector(V_arr,F_arr,octree_buffer,face_last_intersect,
                             ray.start,ray.dir,face_intersect_buffer);
        }
        
        for(unsigned int i=0;i<face_intersect_buffer.size();i++)
        {
            RayFaceIntersect &inter=face_intersect_buffer[i];
            
            Vector3 V=ray.start+inter.t_intersect*ray.dir;
            
            RayInter inter_out;
            
//            inter_out.does_intersect=true;
            inter_out.obj_x=V.x;
            inter_out.obj_y=V.y;
            inter_out.obj_z=V.z;
            inter_out.t=inter.t_intersect;
            inter_out.face=inter.ftarget;
            inter_out.object=obj_ID;
            
            interlist.push_back(inter_out);
        }
    }
}

void Object::rescale_mesh(double scaling_factor_)
{
    scaling_factor=scaling_factor_;
    
    for(std::size_t i=0;i<V_arr.size();i++)
        V_arr[i].loc*=scaling_factor;
    
    scaled_mesh=true;
    
    compute_boundaries();
}

void Object::recalc_normals_z()
{
    if(type==OBJ_MESH)
    {
        int i;
        
        Vector3 O,V;
        
        for(i=0;i<NFc;i++)
        {
            F_arr[i].comp_norm(V_arr);
            
            if(F_arr[i].norm.z<0)
                F_arr[i].norm=-F_arr[i].norm;
        }
    }
}

void Object::set_mesh()
{
    type=OBJ_MESH;
    
    //NVtx=V_arr.size();
    NFc=F_arr.size();
    
    if(Fg_arr.size()==0) // Default group
        define_faces_group(0,0,NFc-1);
    
    compute_boundaries();
}

void Object::set_mesh(std::vector<Sel::Vertex> const &V_arr_,std::vector<Sel::SelFace> const &F_arr_)
{
    V_arr=V_arr_;
    F_arr=F_arr_;
    
    set_mesh();
}

Vertex& Object::vertex(int index) { return V_arr[index]; }

//############
//   Prism
//############

//############
//   Sphere
//############

void Object::intersect_sphere(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    std::array<double,3> hits;
    std::array<int,3> face_labels={0,0,1};
    
    if(!ray_inter_sphere(ray.start,ray.dir,sph_r,hits[0],hits[1]))
    {
        hits[0]=-1;
        hits[1]=-1;
    }
    
    Vector3 P=ray.start+hits[0]*ray.dir;
    if(P.x<sph_cut_x) hits[0]=-1;
    
    P=ray.start+hits[1]*ray.dir;
    if(P.x<sph_cut_x) hits[1]=-1;
    
    if(!ray_inter_plane_x(ray.start,ray.dir,sph_cut_x,hits[2])) hits[2]=-1;
    else
    {
        P=ray.start+hits[2]*ray.dir;
        
        if(P.y*P.y+P.z*P.z>=sph_cut_rad*sph_cut_rad) hits[2]=-1;
    }
    
    if(first_forward)
        push_first_forward(interlist,ray,obj_ID,hits,face_labels);
    else
        push_full_forward(interlist,ray,obj_ID,hits,face_labels);
}

Vector3 Object::normal_sphere(RayInter const &inter)
{
    Vector3 Fnorm;
    
    if(inter.face==0) Fnorm=Vector3(inter.obj_x,inter.obj_y,inter.obj_z);
    else Fnorm=-unit_vec_x;
    
    Fnorm.normalize();
    return Fnorm;
}

void Object::set_sphere()
{
    sph_cut_x=sph_r*(1.0-2.0*sph_cut);
    sph_cut_th=std::acos(1.0-2.0*sph_cut);
    sph_cut_rad=sph_r*std::sin(sph_cut_th);
    
    type=OBJ_SPHERE;
    
    NFc=2;
    F_arr.resize(NFc);
    
    bxm=-1.1*sph_r;
    bxp=+1.1*sph_r;
    
    bym=-1.1*sph_r;
    byp=+1.1*sph_r;
    
    bzm=-1.1*sph_r;
    bzp=+1.1*sph_r;
    
    face_name_arr.resize(NFc);
    face_name_arr[0]="Sphere";
    face_name_arr[1]="Slice";
}

void Object::set_sphere(double radius,double slice)
{
    sph_r=radius;
    sph_cut=slice;
    
    set_sphere();
}

Vector3 Object::sphere_anchor(int anchor)
{
    switch(anchor)
    {
        case 0: return Vector3(0);
        case 1: return Vector3(sph_cut_x,0,0);
        default: return Vector3(0);
    }
}

std::string Object::sphere_anchor_name(int anchor)
{
    switch(anchor)
    {
        case 0: return "Center";
        case 1: return "Slice_Center";
        default: return "Center";
    }
}

void Object::xyz_to_uv_sphere(double &u,double &v,int face_,double x,double y,double z)
{
    if(face_==0)
    {
        double phi=std::atan2(z,y);
        double th=std::atan2(std::sqrt(y*y+z*z),x);
        
        u=th*std::cos(phi)/sph_cut_th;
        v=th*std::sin(phi)/sph_cut_th;
    }
    else
    {
        u=y/sph_cut_rad;
        v=z/sph_cut_rad;
    }
    
    u=u/2.0+0.5;
    v=v/2.0+0.5;
}

void Object::default_N_uv_sphere(int &Nu,int &Nv,int face_)
{
    double delta=std::max(2.0*Pi*sph_r/64,2.0*sph_cut_rad/64);
    
    if(face_==0) Nu=Nv=nearest_2np1(2.0*Pi*sph_r/delta);
    else Nu=Nv=nearest_2np1(2.0*sph_cut_rad/delta);
}

}
