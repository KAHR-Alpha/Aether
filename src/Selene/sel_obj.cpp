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
#include <mesh_tools.h>
#include <ray_intersect.h>

extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{

//###########
//   Frame
//###########

void Frame::consolidate_position()
{
    Position position;
    
    position.set_rotation(in_A,in_B,in_C);
    position.get_orientation(local_x,local_y,local_z);
    
    if(rotation_frame!=nullptr)
    {
        rotation_frame->consolidate_position();
        
        Vector3 &X=rotation_frame->local_x;
        Vector3 &Y=rotation_frame->local_y;
        Vector3 &Z=rotation_frame->local_z;
        
        local_x=local_x.x*X+local_x.y*Y+local_x.z*Z;
        local_y=local_y.x*X+local_y.y*Y+local_y.z*Z;
        local_z=local_z.x*X+local_z.y*Y+local_z.z*Z;
    }
    
    Vector3 O(0);
    
    Vector3 V=get_anchor(origin_anchor);
    V=V.x*local_x+V.y*local_y+V.z*local_z;
    
    Vector3 D=in_displacement;
    
    if(translation_frame!=nullptr)
    {
        translation_frame->consolidate_position();
        
        D=D.x*translation_frame->local_x+
          D.y*translation_frame->local_y+
          D.z*translation_frame->local_z;
    }
    
    if(relative_origin!=nullptr)
    {
        relative_origin->consolidate_position();
        
        O=relative_origin->loc;
        Vector3 Vr=relative_origin->get_anchor(relative_anchor);
        
        O+=Vr.x*relative_origin->local_x+
           Vr.y*relative_origin->local_y+
           Vr.z*relative_origin->local_z;
    }
    
    loc=O+D-V;
}

//####################
//     Object
//####################

Object::Object()
    :obj_ID(-1),
     bxm(0), bxp(0),
     bym(0), byp(0),
     bzm(0), bzp(0),
     NFc(0),
     box_lx(0.1), box_ly(0.1), box_lz(0.1),
     cone_r(1e-2), cone_l(4e-2), cone_cut(1.0),
     conic_R(0.1), conic_K(0), conic_in_radius(0), conic_out_radius(0.1),
     cyl_r(1e-2), cyl_l(4e-2), cyl_cut(1.0),
     dsk_r(0.01), dsk_r_in(0),
     ls_thickness(0.01), ls_r1(0.3), ls_r2(-0.3), ls_r_max_nominal(0.1),
     scaled_mesh(false), scaling_factor(1.0), has_octree(false),
     pr_f(0.1), pr_in_radius(5e-3), pr_length(0.02),
     sph_r(0.05), sph_cut(1.0),
//     prism_length(5e-2), prism_height(2e-2), prism_a1(Pi/3.0), prism_a2(Pi/3.0), prism_width(1),
     cleanup_done(false),
     sensor_type(Sensor::NONE),
     sens_wavelength(false),
     sens_source(false),
     sens_path(false),
     sens_generation(false),
     sens_length(false),
     sens_phase(false),
     sens_ray_world_intersection(false),
     sens_ray_world_direction(false),
     sens_ray_world_polar(false),
     sens_ray_obj_intersection(false),
     sens_ray_obj_direction(false),
     sens_ray_obj_polar(false),
     sens_ray_obj_face(false)
{
    type=OBJ_UNSET;
    build_variables_map();
}

Object::~Object()
{
    if(!cleanup_done) cleanup();
}

void Object::build_variables_map()
{
    variables_map["x"]=&in_displacement.x;
    variables_map["y"]=&in_displacement.y;
    variables_map["z"]=&in_displacement.z;
    
    variables_map["angle_A"]=&in_A.val;
    variables_map["angle_B"]=&in_B.val;
    variables_map["angle_C"]=&in_C.val;
    
    variables_map["box_length_x"]=&box_lx;
    variables_map["box_length_y"]=&box_ly;
    variables_map["box_length_z"]=&box_lz;
    
    variables_map["conic_radius"]=&conic_R;
    variables_map["conic_constant"]=&conic_K;
    variables_map["conic_internal_radius"]=&conic_in_radius;
    variables_map["conic_external_radius"]=&conic_out_radius;
    
    variables_map["cylinder_radius"]=&cyl_r;
    variables_map["cylinder_length"]=&cyl_l;
    variables_map["cylinder_cut_factor"]=&cyl_cut;
    
    variables_map["disk_radius"]=&dsk_r;
    variables_map["disk_internal_radius"]=&dsk_r_in;
    
    variables_map["lens_thickness"]=&ls_thickness;
    variables_map["lens_front_radius"]=&ls_r1;
    variables_map["lens_back_radius"]=&ls_r2;
    variables_map["lens_radius"]=&ls_r_max_nominal;
    
    variables_map["parabola_focal_lengths"]=&pr_f;
    variables_map["parabola_length"]=&pr_length;
    variables_map["parabola_internal_radius"]=&pr_in_radius;
    
    variables_map["rectangle_length_y"]=&box_ly;
    variables_map["rectangle_length_z"]=&box_lz;
    
    variables_map["sphere_radius"]=&sph_r;
    variables_map["sphere_cut_factor"]=&sph_cut;
    
    variables_map["spheroid_radius_x"]=&spd_rx;
    variables_map["spheroid_radius_y"]=&spd_ry;
    variables_map["spheroid_radius_z"]=&spd_rz;
    variables_map["spheroid_cut_factor"]=&spd_cut;
}

void Object::bootstrap(std::filesystem::path const &output_directory,double ray_power)
{
//    std::cout<<"boot "<<this<<std::endl;
//    
//    chk_var(in_displacement);
//    chk_var(in_A.degree());
//    chk_var(in_B.degree());
//    chk_var(in_C.degree());
//    chk_var(loc);
    
    consolidate_position();
    
//    chk_var(local_x);
//    chk_var(local_y);
//    chk_var(local_z);
//    
//    chk_var(loc);
//    
//    std::cout<<"\n\n";
    
    if(sensor_type!=Sensor::NONE)
    {
        sb_fname=output_directory / (name+"_ray_sensor");
        
        if(sb_file.is_open()) sb_file.close();
        cleanup_done=false;
        
        sb_file.open(sb_fname,std::ios::out|std::ios::trunc|std::ios::binary);
        
        sb_Ncurr=sb_Ntot=0;
        
        sb_Nmax=0;
        
        switch(type)
        {
            case OBJ_BOOLEAN:
                sb_file<<"boolean "; break;
            case OBJ_BOX:
                sb_file<<"box "<<box_lx<<" "<<box_ly<<" "<<box_lz; break;
            case OBJ_VOL_CONE:
                sb_file<<"cone "; break;
            case OBJ_CONIC:
                sb_file<<"conic_section "<<conic_R<<" "<<conic_K<<" "<<conic_in_radius<<" "<<conic_out_radius; break;
            case OBJ_VOL_CYLINDER:
                sb_file<<"cylinder "<<cyl_l<<" "<<cyl_r<<" "<<cyl_cut; break;
            case OBJ_DISK:
                sb_file<<"disk "<<dsk_r<<" "<<dsk_r_in; break;
            case OBJ_LENS:
                sb_file<<"lens "<<ls_thickness<<" "<<ls_r_max_nominal<<" "<<ls_r1<<" "<<ls_r2; break;
            case OBJ_MESH:
                sb_file<<"mesh "<<mesh_fname; break;
            case OBJ_RECTANGLE:
                sb_file<<"rectangle "<<box_ly<<" "<<box_lz; break;
            case OBJ_PARABOLA:
                sb_file<<"parabola "<<pr_f<<" "<<pr_in_radius<<" "<<pr_length; break;
            case OBJ_SPHERE:
                sb_file<<"sphere "<<sph_r<<" "<<sph_cut; break;
            case OBJ_SPHERE_PATCH:
                sb_file<<"spherical_patch "<<sph_r<<" "<<sph_cut; break;
        }
        
        sb_file<<"\n";
        sb_file<<loc.x<<" "<<loc.y<<" "<<loc.z<<" "
               <<local_x.x<<" "<<local_x.y<<" "<<local_x.z<<" "
               <<local_y.x<<" "<<local_y.y<<" "<<local_y.z<<" "
               <<local_z.x<<" "<<local_z.y<<" "<<local_z.z<<" "
               <<bxm<<" "<<bxp<<" "
               <<bym<<" "<<byp<<" "
               <<bzm<<" "<<bzp<<" "
               <<ray_power<<"\n";
        
        if(sens_wavelength) { sb_Nmax+=sizeof(double); sb_file<<"wavelength "; }
        if(sens_source) { sb_Nmax+=sizeof(int); sb_file<<"source "; }
        if(sens_path) { sb_Nmax+=sizeof(int); sb_file<<"path "; }
        if(sens_generation) { sb_Nmax+=sizeof(int); sb_file<<"generation "; }
        if(sens_length) { sb_Nmax+=sizeof(double); sb_file<<"length "; }
        if(sens_phase) { sb_Nmax+=sizeof(double); sb_file<<"phase "; }
        if(sens_ray_world_intersection) { sb_Nmax+=3*sizeof(double); sb_file<<"world_intersection "; }
        if(sens_ray_world_direction) { sb_Nmax+=3*sizeof(double); sb_file<<"world_direction "; }
        if(sens_ray_world_polar) { sb_Nmax+=3*sizeof(double); sb_file<<"world_polarization "; }
        if(sens_ray_obj_intersection) { sb_Nmax+=3*sizeof(double); sb_file<<"obj_intersection "; }
        if(sens_ray_obj_direction) { sb_Nmax+=3*sizeof(double); sb_file<<"obj_direction "; }
        if(sens_ray_obj_polar) { sb_Nmax+=3*sizeof(double); sb_file<<"obj_polarization "; }
        if(sens_ray_obj_face) { sb_Nmax+=sizeof(int); sb_file<<"obj_face "; }
        
        sb_Nmax=static_cast<int>(150e6/sb_Nmax);
        
        if(sens_wavelength) sb_lambda.resize(sb_Nmax);
        if(sens_source) sb_source.resize(sb_Nmax);
        if(sens_path) sb_path.resize(sb_Nmax);
        if(sens_generation) sb_generation.resize(sb_Nmax);
        if(sens_length) sb_opl.resize(sb_Nmax);
        if(sens_phase) sb_phase.resize(sb_Nmax);
        if(sens_ray_world_intersection) sb_world_i.resize(sb_Nmax);
        if(sens_ray_world_direction) sb_world_d.resize(sb_Nmax);
        if(sens_ray_world_polar) sb_world_polar.resize(sb_Nmax);
        if(sens_ray_obj_intersection) sb_obj_i.resize(sb_Nmax);
        if(sens_ray_obj_direction) sb_obj_d.resize(sb_Nmax);
        if(sens_ray_obj_polar) sb_obj_polar.resize(sb_Nmax);
        if(sens_ray_obj_face) sb_face.resize(sb_Nmax);
    }
}

void Object::cleanup()
{
    if(sensor_type!=Sensor::NONE)
    {
        sens_buffer_dump();
        
        sb_file.close();
    }
    
    cleanup_done=true;
}

void Object::compute_boundaries()
{
         if(type==OBJ_LENS)
    {
        double x1=std::min(ls_cyl_pos.x,ls_c1.x-ls_r1);
        double x2=std::max(ls_cyl_pos.x+ls_cyl_h,ls_c2.x-ls_r2);
        
        bxm=1.1*x1;
        bxp=1.1*x2;
        
        bym=-1.1*ls_r_max;
        byp=+1.1*ls_r_max;
        
        bzm=-1.1*ls_r_max;
        bzp=+1.1*ls_r_max;
    }
    else if(type==OBJ_MESH)
    {
        if(V_arr.size()==0)
        {
            bxm=bxp=0;
            bym=byp=0;
            bzm=bzp=0;
            
            return;
        }
                
        bxm=bxp=V_arr[0].loc.x;
        bym=byp=V_arr[0].loc.y;
        bzm=bzp=V_arr[0].loc.z;
        
        for(std::size_t i=0;i<V_arr.size();i++)
        {
            bxm=std::min(bxm,V_arr[i].loc.x);
            bym=std::min(bym,V_arr[i].loc.y);
            bzm=std::min(bzm,V_arr[i].loc.z);
            
            bxp=std::max(bxp,V_arr[i].loc.x);
            byp=std::max(byp,V_arr[i].loc.y);
            bzp=std::max(bzp,V_arr[i].loc.z);
        }
        
        double spanx=bxp-bxm;
        double spany=byp-bym;
        double spanz=bzp-bzm;
        
        double span_max=var_max(spanx,spany,spanz);
        
        spanx=std::max(spanx,0.1*span_max);
        spany=std::max(spany,0.1*span_max);
        spanz=std::max(spanz,0.1*span_max);
        
        bxm-=0.05*spanx; bxp+=0.05*spanx;
        bym-=0.05*spany; byp+=0.05*spany;
        bzm-=0.05*spanz; bzp+=0.05*spanz;
        
        if(NFc>12)
        {
            has_octree=true;
            octree.clear_tree();
            octree.set_params(8,bxm,bxp,bym,byp,bzm,bzp);
            octree.generate_tree(V_arr,F_arr);
        }
    }
}

bool Object::contains(double x_,double y_,double z_)
{
    Vector3 D;
    D.rand_sph();
    
    SelRay ray;
    ray.set_start(Vector3(x_,y_,z_));
    ray.set_dir(D);
    
    std::vector<RayInter> inter_list;
    
    intersect(ray,inter_list,-1,false);
    
    if(inter_list.size()%2==0) return false;
    else return true;
}

void Object::default_N_uv(int &Nu,int &Nv,int face_)
{
    switch(type)
    {
        case OBJ_BOOLEAN: Nu=Nv=1; break;
        case OBJ_BOX: default_N_uv_box(Nu,Nv,face_); break;
        case OBJ_VOL_CONE: Nu=Nv=1; break;
        case OBJ_CONIC: default_N_uv_conic_section(Nu,Nv,face_); break;
        case OBJ_VOL_CYLINDER: default_N_uv_cylinder_volume(Nu,Nv,face_); break;
        case OBJ_DISK: default_N_uv_disk(Nu,Nv,face_); break;
        case OBJ_LENS: default_N_uv_lens(Nu,Nv,face_); break;
        case OBJ_PARABOLA: default_N_uv_parabola(Nu,Nv,face_); break;
        case OBJ_MESH: Nu=Nv=1; break;
        case OBJ_RECTANGLE: default_N_uv_rectangle(Nu,Nv,face_); break;
        case OBJ_SPHERE: default_N_uv_sphere(Nu,Nv,face_); break;
        case OBJ_SPHERE_PATCH: default_N_uv_spherical_patch(Nu,Nv,face_); break;
        default:
            std::cout<<"Undefined UV coordinates for object of type "<<type<<"\n";
            std::exit(EXIT_FAILURE);
    }
}

void Object::define_faces_group(int index,int start,int end)
{
    if(index<0) return;
    
    if(static_cast<int>(Fg_arr.size())<=index)
    {
        Fg_arr.resize(index+1);
        Fg_start.resize(index+1);
        Fg_end.resize(index+1);
    }
    
    Fg_start[index]=start;
    Fg_end[index]=end;
}

SelFace& Object::face(int index)
{
    if(type!=OBJ_BOOLEAN) return F_arr[index];
    else
    {
        int &Nf1=bool_obj_1->NFc;
        
        if(index<Nf1) return bool_obj_1->face(index);
        else return bool_obj_2->face(index-Nf1);
    }
}

SelFace& Object::faces_group(int index)
{
    return Fg_arr[index];
}

std::string Object::face_name(int index)
{
    if(index<static_cast<int>(face_name_arr.size())) return face_name_arr[index];
    else return "Face "+std::to_string(index);
}

Vector3 Object::face_normal(RayInter const &inter)
{
    switch(type)
    {
        case OBJ_BOOLEAN: return normal_boolean(inter);
        case OBJ_BOX: return normal_box(inter);
        case OBJ_VOL_CONE: return normal_cone_volume(inter);
        case OBJ_CONIC: return normal_conic_section(inter);
        case OBJ_VOL_CYLINDER: return normal_cylinder_volume(inter);
        case OBJ_DISK: return -unit_vec_x;
        case OBJ_LENS: return normal_lens(inter);
        case OBJ_PARABOLA: return normal_parabola(inter);
        case OBJ_MESH: return face(inter.face).norm;
        case OBJ_RECTANGLE: return -unit_vec_x;
        case OBJ_SPHERE: return normal_sphere(inter);
        case OBJ_SPHERE_PATCH: return normal_spherical_patch(inter);
        default:
            std::cout<<"Undefined normal for object of type "<<type<<"\n";
            std::exit(0);
    }
        
    return Vector3(0);
}

Vector3 Object::face_tangent(RayInter const &inter,
                             Vector3 const &normal, bool up)
{
    switch(type)
    {
//        case OBJ_BOOLEAN: return tangent_boolean(inter);
        case OBJ_BOX: return tangent_box(inter,normal,up);
//        case OBJ_VOL_CONE: return tangent_cone_volume(inter);
//        case OBJ_VOL_CYLINDER: return tangent_cylinder_volume(inter);
//        case OBJ_LENS: return tangent_lens(inter);
//        case OBJ_PARABOLA: return tangent_parabola(inter);
//        case OBJ_MESH: return face(inter.face).norm;
        case OBJ_RECTANGLE: return tangent_rectangle(inter,normal,up);
    }
    
    return Vector3(0);
}

Vector3 Object::get_anchor(int anchor)
{
    switch(type)
    {
        case OBJ_BOX: return box_anchor(anchor);
        case OBJ_CONIC: return conic_section_anchor(anchor);
        case OBJ_LENS: return lens_anchor(anchor);
        case OBJ_PARABOLA: return parabola_anchor(anchor);
        case OBJ_VOL_CONE: return cone_anchor(anchor);
        case OBJ_VOL_CYLINDER: return cylinder_anchor(anchor);
        case OBJ_SPHERE: return sphere_anchor(anchor);
        case OBJ_SPHERE_PATCH: return sphere_anchor(anchor);
        default: return Vector3(0);
    }
}

int Object::get_anchors_number()
{
    switch(type)
    {
        case OBJ_BOX: return 15;
        case OBJ_CONIC: return 4;
        case OBJ_LENS: return 7;
        case OBJ_PARABOLA: return 4;
        case OBJ_VOL_CYLINDER: return 3;
        case OBJ_VOL_CONE: return 3;
        case OBJ_SPHERE: return 2;
        case OBJ_SPHERE_PATCH: return 2;
        default: return 1;
    }
}

std::string Object::get_anchor_name(int anchor)
{
    switch(type)
    {
        case OBJ_BOX: return box_anchor_name(anchor);
        case OBJ_CONIC: return conic_section_anchor_name(anchor);
        case OBJ_LENS: return lens_anchor_name(anchor);
        case OBJ_PARABOLA: return parabola_anchor_name(anchor);
        case OBJ_VOL_CONE: return cone_anchor_name(anchor);
        case OBJ_VOL_CYLINDER: return cylinder_anchor_name(anchor);
        case OBJ_SPHERE: return sphere_anchor_name(anchor);
        case OBJ_SPHERE_PATCH: return sphere_anchor_name(anchor);
        default: return "Center";
    }
}

std::string Object::get_anchor_script_name(int anchor)
{
    std::string prefix="";
    std::string anchor_name="";
    
    switch(type)
    {
        case OBJ_BOX:
            prefix="SEL_OBJ_BOX_";
            anchor_name=box_anchor_name(anchor);
            break;
        case OBJ_CONIC:
            prefix="SEL_OBJ_CONIC_SECTION_";
            anchor_name=conic_section_anchor_name(anchor);
            break;
        case OBJ_LENS:
            prefix="SEL_OBJ_LENS_";
            anchor_name=lens_anchor_name(anchor);
            break;
        case OBJ_PARABOLA:
            prefix="SEL_PARABOLA_";
            anchor_name=parabola_anchor_name(anchor);
            break;
        case OBJ_VOL_CONE:
            prefix="SEL_OBJ_CONE_";
            anchor_name=cone_anchor_name(anchor);
            break;
        case OBJ_VOL_CYLINDER:
            prefix="SEL_OBJ_CYL_";
            anchor_name=cylinder_anchor_name(anchor);
            break;
        case OBJ_SPHERE:
            prefix="SEL_OBJ_SPHERE_";
            anchor_name=sphere_anchor_name(anchor);
            break;
        case OBJ_SPHERE_PATCH:
            prefix="SEL_OBJ_SPHERE_PATCH_";
            anchor_name=sphere_anchor_name(anchor);
            break;
        default:
            prefix="SEL_OBJ_";
            anchor_name="CENTER";
    }
    
    anchor_name=prefix+anchor_name;
    for(std::size_t i=0;i<anchor_name.size();i++)
        if(anchor_name[i]!='_') anchor_name[i]=std::toupper(anchor_name[i]);
    
    return anchor_name;
}

int Object::get_N_faces() { return NFc; }
int Object::get_N_faces_groups() { return Fg_arr.size(); }

std::filesystem::path Object::get_sensor_file_path() const
{
    return sb_fname;
}

std::string Object::get_type_name()
{
    switch(type)
    {
        case OBJ_BOOLEAN: return "boolean";
        case OBJ_BOX: return "box";
        case OBJ_VOL_CONE: return "cone";
        case OBJ_CONIC: return "conic_section";
        case OBJ_VOL_CYLINDER: return "cylinder";
        case OBJ_DISK: return "disk";
        case OBJ_LENS: return "lens";
        case OBJ_MESH: return "mesh";
        case OBJ_PARABOLA: return "parabola";
        case OBJ_RECTANGLE: return "rectangle";
        case OBJ_SPHERE: return "sphere";
        case OBJ_SPHERE_PATCH: return "spherical_patch";
    }
    
    return "unknown_object_type";
}

void Object::intersect(SelRay const &base_ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    SelRay ray;
    to_local_ray(ray,base_ray);
    
    if(!intersect_boundaries_box(ray)) return;
    
    switch(type)
    {
        case OBJ_BOOLEAN:
            intersect_boolean(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_BOX:
            intersect_box(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_VOL_CONE:
            intersect_cone_volume(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_CONIC:
            intersect_conic_section(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_VOL_CYLINDER:
            intersect_cylinder_volume(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_DISK:
            intersect_disk(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_LENS:
            intersect_lens(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_MESH:
            intersect_mesh(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_PARABOLA:
            intersect_parabola(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_RECTANGLE:
            intersect_rectangle(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_SPHERE:
            intersect_sphere(ray,interlist,face_last_intersect,first_forward);
            break;
        case OBJ_SPHERE_PATCH:
            intersect_spherical_patch(ray,interlist,face_last_intersect,first_forward);
            break;
    }
}

bool Object::intersect_boundaries_box(SelRay const &ray)
{
    if(type!=OBJ_BOOLEAN)
    {
        double tmin=(bxm-ray.start.x)*ray.inv_dir.x;
        double tmax=(bxp-ray.start.x)*ray.inv_dir.x;
        double tymin=(bym-ray.start.y)*ray.inv_dir.y;
        double tymax=(byp-ray.start.y)*ray.inv_dir.y;
        
        if(tmin>tmax) std::swap(tmin,tmax);
        if(tymin>tymax) std::swap(tymin,tymax);
        
        if(tmin>tymax || tymin>tmax) return false;
        if(tymin>tmin) tmin=tymin;
        if(tymax<tmax) tmax=tymax;
        
        double tzmin=(bzm-ray.start.z)*ray.inv_dir.z;
        double tzmax=(bzp-ray.start.z)*ray.inv_dir.z;
        
        if(tzmin>tzmax) std::swap(tzmin,tzmax);
        if(tmin>tzmax || tzmin>tmax) return false;
        if(tzmin>tmin) tmin=tzmin;
        if(tzmax<tmax) tmax=tzmax;
        
        if(tmax>0) return true;
        else return false;
    }
    else
    {
        if(boolean_type==Boolean_Type::EXCLUDE) return bool_obj_1->intersect_boundaries_box(ray);
        else
        {
            return bool_obj_1->intersect_boundaries_box(ray) || bool_obj_2->intersect_boundaries_box(ray);
        }
    }
}

void Object::process_intersection(RayPath &path)
{
    SelRay &ray=path.ray;
    RayInter &inter=path.intersection;
    
    Vector3 next_start_obj(inter.obj_x,inter.obj_y,inter.obj_z);
    Vector3 next_start=next_start_obj.x*local_x+
                       next_start_obj.y*local_y+
                       next_start_obj.z*local_z+loc;
    
    if(type==OBJ_BOOLEAN)
    {
        if(inter.face<bool_obj_1->NFc) next_start+=bool_obj_1->loc;
        else next_start+=bool_obj_2->loc;
    }
    
    ray.age+=std::real(ray.n_ind)*(next_start-ray.start).norm();
    int &face_inter=inter.face;
    
    if(sensor_type!=Sensor::NONE)
    {
        sens_buffer_add(ray,next_start,next_start_obj,face_inter);
        
        if(sensor_type==Sensor::ABS)
        {
            ray.prev_start=ray.start;
            ray.start=next_start;
            
            path.complete=true;
            return;
        }
    }
    
    Vector3 Fnorm,Ftang;
    SelFace &hit_face=face(face_inter);
    
    Fnorm=face_normal(path.intersection);
    
    Vector3 local_dir=to_local(ray.dir);
    Vector3 local_polar=to_local(ray.pol);
    
    double &lambda=ray.lambda;
    
    double n_scal=scalar_prod(local_dir,Fnorm);
    
    // Default assumption: coming from outside
    
    Material *n1_mat=hit_face.up_mat;
    Material *n2_mat=hit_face.down_mat;
    IRF *irf=hit_face.up_irf;
    Ftang=face_tangent(path.intersection,Fnorm,true);
    
    if(n_scal>=0) // If coming from inside
    {
        n1_mat=hit_face.down_mat;
        n2_mat=hit_face.up_mat;
        irf=hit_face.down_irf;
        Ftang=face_tangent(path.intersection,Fnorm,false);
    }
    
    double w=m_to_rad_Hz(lambda);
    
    Imdouble n1=n1_mat->get_n(w);
    Imdouble n2=n2_mat->get_n(w);
    
    Vector3 dir_out,polar_out;
    
    bool abs_ray=false;
    
    double beer_lambert_factor=std::exp(-4.0*Pi*std::imag(ray.n_ind)/ray.lambda);
    
    if(randp()>beer_lambert_factor) abs_ray=true;
    else
    {
        abs_ray=irf->get_response(dir_out,polar_out,
                                  local_dir,local_polar,
                                  Fnorm,Ftang,
                                  lambda,n1.real(),n2.real());
    
        // Reflected or transmitted determination
        
        double n_scal_resp=scalar_prod(dir_out,Fnorm);
    
        ray.n_ind=n1;
        
        if(n_scal*n_scal_resp>=0) // Transmitted
            ray.n_ind=n2;
        
        // Shifting the ray to the new starting point
        
        ray.prev_start=ray.start;
        ray.start=next_start;
    }
    
    if(abs_ray==true || ray.generation>=200)
    {
        path.complete=true;
        return;
    }
    
    ray.set_dir(to_global(dir_out));
    ray.set_pol(to_global(polar_out));
    
    // Reset job
    
    path.complete=false;
    path.does_intersect=false;
    
    path.obj_last_intersection_f=obj_ID;
    path.face_last_intersect=path.intersection.face;
    
    path.intersection.reset();
}

void Object::propagate_faces_group(int index)
{
    int start=std::max(0,Fg_start[index]);
    int end=std::min(Fg_end[index],static_cast<int>(F_arr.size()-1));
    chk_var(start);
    chk_var(end);
    chk_var(Fg_start.size());
    chk_var(Fg_end.size());
    for(int i=start;i<=end;i++)
    {
        F_arr[i].up_mat=Fg_arr[index].up_mat;
        F_arr[i].down_mat=Fg_arr[index].down_mat;
        
        F_arr[i].up_irf=Fg_arr[index].up_irf;
        F_arr[i].down_irf=Fg_arr[index].down_irf;
        
        F_arr[i].tangent_up=Fg_arr[index].tangent_up;
        F_arr[i].tangent_down=Fg_arr[index].tangent_down;
        
        F_arr[i].fixed_tangent_up=Fg_arr[index].fixed_tangent_up;
        F_arr[i].fixed_tangent_down=Fg_arr[index].fixed_tangent_down;
    }
}

double* Object::reference_variable(std::string const &variable_name)
{
    if(variables_map.count(variable_name)==0) return nullptr;
    
    return variables_map[variable_name];
}

void Object::save_mesh_to_obj(std::string const &fname)
{
    obj_file_save(fname,V_arr,F_arr);
}

void Object::sens_buffer_add(SelRay &ray,
                             Vector3 const &world_intersection,
                             Vector3 const &obj_intersection,
                             int hit_face)
{
    SelRay local_ray;
    to_local_ray(local_ray,ray);
    
    if(sb_Ncurr>=sb_Nmax) sens_buffer_dump();
    
    int i=sb_Ncurr;
    
    if(sens_wavelength) sb_lambda[i]=ray.lambda;
    if(sens_source) sb_source[i]=ray.source_ID;
    if(sens_path) sb_path[i]=ray.family;
    if(sens_generation) sb_generation[i]=ray.generation;
    if(sens_length) sb_opl[i]=ray.age;
    if(sens_phase) sb_phase[i]=ray.age/ray.lambda;
    if(sens_ray_world_intersection) sb_world_i[i]=world_intersection;
    if(sens_ray_world_direction) sb_world_d[i]=ray.dir;
    if(sens_ray_world_polar) sb_world_polar[i]=ray.pol;
    if(sens_ray_obj_intersection) sb_obj_i[i]=obj_intersection;
    if(sens_ray_obj_direction) sb_obj_d[i]=local_ray.dir;
    if(sens_ray_obj_polar) sb_obj_polar[i]=local_ray.pol;
    if(sens_ray_obj_face) sb_face[i]=hit_face;
    
    sb_Ncurr++;
    sb_Ntot++;
}

void Object::sens_buffer_dump()
{
    int i;
    
    std::stringstream strm;
    
    for(i=0;i<std::min(sb_Nmax,sb_Ncurr);i++)
    {
        strm<<"\n";
        if(sens_wavelength) strm<<sb_lambda[i]<<" ";
        if(sens_source) strm<<sb_source[i]<<" ";
        if(sens_path) strm<<sb_path[i]<<" ";
        if(sens_generation) strm<<sb_generation[i]<<" ";
        if(sens_length) strm<<sb_opl[i]<<" ";
        if(sens_phase) strm<<sb_phase[i]<<" ";
        if(sens_ray_world_intersection) strm<<sb_world_i[i].x<<" "<<sb_world_i[i].y<<" "<<sb_world_i[i].z<<" ";
        if(sens_ray_world_direction)    strm<<sb_world_d[i].x<<" "<<sb_world_d[i].y<<" "<<sb_world_d[i].z<<" ";
        if(sens_ray_world_polar)        strm<<sb_world_polar[i].x<<" "<<sb_world_polar[i].y<<" "<<sb_world_polar[i].z<<" ";
        if(sens_ray_obj_intersection)   strm<<sb_obj_i[i].x<<" "<<sb_obj_i[i].y<<" "<<sb_obj_i[i].z<<" ";
        if(sens_ray_obj_direction)      strm<<sb_obj_d[i].x<<" "<<sb_obj_d[i].y<<" "<<sb_obj_d[i].z<<" ";
        if(sens_ray_obj_polar)          strm<<sb_obj_polar[i].x<<" "<<sb_obj_polar[i].y<<" "<<sb_obj_polar[i].z<<" ";
        if(sens_ray_obj_face) strm<<sb_face[i]<<" ";
    }
    
    sb_file<<strm.str();
    
    sb_Ncurr=0;
}

void Object::set_default_in_irf(IRF *irf)
{
    if(type==OBJ_BOOLEAN)
    {
        if(boolean_type==Boolean_Type::EXCLUDE)
        {
            bool_obj_1->set_default_in_irf(irf);
            bool_obj_2->set_default_out_irf(irf);
        }
        else
        {
            bool_obj_1->set_default_in_irf(irf);
            bool_obj_2->set_default_in_irf(irf);
        }
    }
    else
    {
        for(int i=0;i<NFc;i++) F_arr[i].down_irf=irf;
        
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].down_irf=irf;
    }
}

void Object::set_default_in_mat(Material *mat)
{
    if(type==OBJ_BOOLEAN)
    {
        if(boolean_type==Boolean_Type::EXCLUDE)
        {
            bool_obj_1->set_default_in_mat(mat);
            bool_obj_2->set_default_out_mat(mat);
        }
        else
        {
            bool_obj_1->set_default_in_mat(mat);
            bool_obj_2->set_default_in_mat(mat);
        }
    }
    else
    {
        for(int i=0;i<NFc;i++) F_arr[i].down_mat=mat;
        
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].down_mat=mat;
    }
}

void Object::set_default_irf(IRF *irf)
{
    if(type==OBJ_BOOLEAN)
    {
        bool_obj_1->set_default_irf(irf);
        bool_obj_2->set_default_irf(irf);
    }
    else
    {
        for(int i=0;i<NFc;i++)
        {
            F_arr[i].down_irf=irf;
            F_arr[i].up_irf=irf;
        }
        
        for(std::size_t i=0;i<Fg_arr.size();i++)
        {
            Fg_arr[i].down_irf=irf;
            Fg_arr[i].up_irf=irf;
        }
    }
}

void Object::set_default_out_irf(IRF *irf)
{
    if(type==OBJ_BOOLEAN)
    {
        if(boolean_type==Boolean_Type::EXCLUDE)
        {
            bool_obj_1->set_default_out_irf(irf);
            bool_obj_2->set_default_in_irf(irf);
        }
        else
        {
            bool_obj_1->set_default_out_irf(irf);
            bool_obj_2->set_default_out_irf(irf);
        }
    }
    else
    {
        for(int i=0;i<NFc;i++) F_arr[i].up_irf=irf;
        
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].up_irf=irf;
    }
}

void Object::set_default_out_mat(Material *mat)
{
    if(type==OBJ_BOOLEAN)
    {
        if(boolean_type==Boolean_Type::EXCLUDE)
        {
            bool_obj_1->set_default_out_mat(mat);
            bool_obj_2->set_default_in_mat(mat);
        }
        else
        {
            bool_obj_1->set_default_out_mat(mat);
            bool_obj_2->set_default_out_mat(mat);
        }
    }
    else
    {
        for(int i=0;i<NFc;i++) F_arr[i].up_mat=mat;
        
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].up_mat=mat;
    }
}

void Object::set_sens_abs()
{
    sensor_type=Sensor::ABS;
}

void Object::set_sens_none()
{
    sensor_type=Sensor::NONE;
}

void Object::set_sens_transp()
{
    sensor_type=Sensor::TRANSP;
}


void Object::to_local_ray(SelRay &ray,SelRay const &base_ray)
{
    ray=base_ray;
    
    ray.start.x=scalar_prod(local_x,base_ray.start-loc);
    ray.start.y=scalar_prod(local_y,base_ray.start-loc);
    ray.start.z=scalar_prod(local_z,base_ray.start-loc);
    
    Vector3 local_dir;
    
    local_dir.x=scalar_prod(local_x,base_ray.dir);
    local_dir.y=scalar_prod(local_y,base_ray.dir);
    local_dir.z=scalar_prod(local_z,base_ray.dir);
    
    ray.set_dir(local_dir);
    
    ray.pol=to_local(base_ray.pol);
}


void Object::update_geometry()
{
    switch(type)
    {
        case OBJ_BOOLEAN: break;
        case OBJ_BOX: set_box(); break;
        case OBJ_CONIC: set_conic_section(); break;
        case OBJ_DISK: set_disk(); break;
        case OBJ_LENS: set_lens(); break;
        case OBJ_MESH: break;
        case OBJ_PARABOLA: set_parabola(); break;
        case OBJ_RECTANGLE: set_rectangle(); break;
        case OBJ_SPHERE: set_sphere(); break;
        case OBJ_SPHERE_PATCH: set_spherical_patch(); break;
        case OBJ_VOL_CONE: set_cone_volume(); break;
        case OBJ_VOL_CYLINDER: set_cylinder_volume(); break;
        default:
            std::cout<<"Geometry unaccounted for\n";
            std::exit(EXIT_FAILURE);
    }
}


void Object::xyz_to_uv(double &u,double &v,int face_,
                       double x,double y,double z)
{
    switch(type)
    {
        case OBJ_BOOLEAN: u=v=0; break;
        case OBJ_BOX: xyz_to_uv_box(u,v,face_,x,y,z); break;
        case OBJ_VOL_CONE: u=v=0; break;
        case OBJ_VOL_CYLINDER: xyz_to_uv_cylinder_volume(u,v,face_,x,y,z); break;
        case OBJ_DISK: xyz_to_uv_disk(u,v,face_,x,y,z); break;
        case OBJ_LENS: xyz_to_uv_lens(u,v,face_,x,y,z); break;
        case OBJ_PARABOLA: xyz_to_uv_parabola(u,v,face_,x,y,z); break;
        case OBJ_MESH: u=v=0; break;
        case OBJ_RECTANGLE: xyz_to_uv_rectangle(u,v,face_,x,y,z); break;
        case OBJ_SPHERE: xyz_to_uv_sphere(u,v,face_,x,y,z); break;
        case OBJ_SPHERE_PATCH: xyz_to_uv_spherical_patch(u,v,face_,x,y,z); break;
        default:
            std::cout<<"Undefined UV coordinates for object of type "<<type<<"\n";
            std::exit(EXIT_FAILURE);
    }
}

}
