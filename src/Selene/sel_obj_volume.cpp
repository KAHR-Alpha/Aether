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
    int nearest_2np1(double val)
    {
        int N=nearest_integer((val-1.0)/2.0);
        return 2*N+1;
    }
    
    
    void Object::set_box()
    {
        type=OBJ_BOX;
        
        box.finalize();
        NFc=F_arr.size();
    }
    
    
    void Object::set_box(double lx, double ly, double lz)
    {
        box.set_parameters(lx, ly, lz);
        
        set_box();
    }

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

    //##########
    //   Cone
    //##########
    
    void Object::set_cone_volume()
    {
        type=OBJ_VOL_CONE;
        
        cone.finalize();
        NFc=F_arr.size();
    }
    
    
    void Object::set_cone_volume(double radius,double length,double cut)
    {
        cone.set_parameters(radius, length, cut);
        
        set_cone_volume();
    }
    
    
    //##############
    //   Cylinder
    //##############

    void Object::set_cylinder_volume()
    {
        type=OBJ_VOL_CYLINDER;
        
        cylinder.finalize();
        NFc = F_arr.size();
    }
    
    
    void Object::set_cylinder_volume(double length, double radius, double cut)
    {
        cylinder.set_parameters(length, radius, cut);
        
        set_cylinder_volume();
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
    
    bbox.xm=-1.1*sph_r;
    bbox.xp=+1.1*sph_r;
    
    bbox.ym=-1.1*sph_r;
    bbox.yp=+1.1*sph_r;
    
    bbox.zm=-1.1*sph_r;
    bbox.zp=+1.1*sph_r;
    
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
