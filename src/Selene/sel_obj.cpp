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
     max_ray_generation(200),
     NFc(0),
     box(bbox, F_arr, face_name_arr),
     cone(bbox, F_arr, face_name_arr),
     conic(bbox, F_arr, face_name_arr),
     cylinder(bbox, F_arr, face_name_arr),
     disk(bbox, F_arr, face_name_arr),
     lens(bbox, F_arr, face_name_arr),
     mesh(bbox, F_arr, face_name_arr),
     parabola(bbox, F_arr, face_name_arr),
     rectangle(bbox, F_arr, face_name_arr),
     sphere(bbox, F_arr, face_name_arr),
     sphere_patch(bbox, F_arr, face_name_arr),
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
    
    box.map_variables(variables_map);
    
    variables_map["conic_radius"]=&conic.R_factor;
    variables_map["conic_constant"]=&conic.K_factor;
    variables_map["conic_internal_radius"]=&conic.in_radius;
    variables_map["conic_external_radius"]=&conic.out_radius;
    
    variables_map["cylinder_radius"]=&cylinder.radius;
    variables_map["cylinder_length"]=&cylinder.length;
    variables_map["cylinder_cut_factor"]=&cylinder.cut_factor;
    
    variables_map["disk_radius"]=&disk.radius;
    variables_map["disk_internal_radius"]=&disk.in_radius;
    
    variables_map["lens_thickness"]=&lens.thickness;
    variables_map["lens_front_radius"]=&lens.radius_front;
    variables_map["lens_back_radius"]=&lens.radius_back;
    variables_map["lens_radius"]=&lens.max_outer_radius;
    
    variables_map["parabola_focal_lengths"]=&parabola.focal;
    variables_map["parabola_length"]=&parabola.length;
    variables_map["parabola_internal_radius"]=&parabola.inner_radius;
    
    variables_map["rectangle_length_y"]=&rectangle.ref_ly();
    variables_map["rectangle_length_z"]=&rectangle.ref_lz();
    
    sphere.map_variables(variables_map);
    sphere_patch.map_variables(variables_map);
    
    variables_map["spheroid_radius_x"]=&spd_rx;
    variables_map["spheroid_radius_y"]=&spd_ry;
    variables_map["spheroid_radius_z"]=&spd_rz;
    variables_map["spheroid_cut_factor"]=&spd_cut;
}


void Object::bootstrap(std::filesystem::path const &output_directory,double ray_power,int max_ray_bounces)
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
                sb_file<<"box "<<box.get_lx()<<" "<<box.get_ly()<<" "<<box.get_lz(); break;
            case OBJ_VOL_CONE:
                sb_file<<"cone "; break;
            case OBJ_CONIC:
                sb_file<<"conic_section "<<conic.R_factor<<" "<<conic.K_factor<<" "<<conic.in_radius<<" "<<conic.out_radius; break;
            case OBJ_VOL_CYLINDER:
                sb_file<<"cylinder "<<cylinder.length<<" "<<cylinder.radius<<" "<<cylinder.cut_factor; break;
            case OBJ_DISK:
                sb_file<<"disk "<<disk.radius<<" "<<disk.in_radius; break;
            case OBJ_LENS:
                sb_file<<"lens "<<lens.thickness<<" "<<lens.max_outer_radius<<" "<<lens.radius_front<<" "<<lens.radius_back; break;
            case OBJ_MESH:
                sb_file<<"mesh "<<mesh.get_mesh_path().generic_string(); break;
            case OBJ_RECTANGLE:
                sb_file<<"rectangle "<<rectangle.get_ly()<<" "<<rectangle.get_lz(); break;
            case OBJ_PARABOLA:
                sb_file<<"parabola "<<parabola.focal<<" "<<parabola.inner_radius<<" "<<parabola.length; break;
            case OBJ_SPHERE:
                sb_file<<"sphere "<<sphere.get_radius()<<" "<<sphere.get_cut_factor(); break;
            case OBJ_SPHERE_PATCH:
                sb_file<<"spherical_patch "<<sphere_patch.get_radius()<<" "<<sphere_patch.get_cut_factor(); break;
        }
        
        sb_file<<"\n";
        sb_file<<loc.x<<" "<<loc.y<<" "<<loc.z<<" "
               <<local_x.x<<" "<<local_x.y<<" "<<local_x.z<<" "
               <<local_y.x<<" "<<local_y.y<<" "<<local_y.z<<" "
               <<local_z.x<<" "<<local_z.y<<" "<<local_z.z<<" "
               <<bbox.xm<<" "<<bbox.xp<<" "
               <<bbox.ym<<" "<<bbox.yp<<" "
               <<bbox.zm<<" "<<bbox.zp<<" "
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
    
    max_ray_generation=max_ray_bounces;
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
        case OBJ_BOX: box.default_N_uv(Nu,Nv,face_); break;
        case OBJ_VOL_CONE: Nu=Nv=1; break;
        case OBJ_CONIC: conic.default_N_uv(Nu,Nv,face_); break;
        case OBJ_VOL_CYLINDER: cylinder.default_N_uv(Nu,Nv,face_); break;
        case OBJ_DISK: disk.default_N_uv(Nu,Nv,face_); break;
        case OBJ_LENS: lens.default_N_uv(Nu,Nv,face_); break;
        case OBJ_PARABOLA: parabola.default_N_uv(Nu,Nv,face_); break;
        case OBJ_MESH: Nu=Nv=1; break;
        case OBJ_RECTANGLE: rectangle.default_N_uv(Nu,Nv,face_); break;
        case OBJ_SPHERE: sphere.default_N_uv(Nu,Nv,face_); break;
        case OBJ_SPHERE_PATCH: sphere_patch.default_N_uv(Nu,Nv,face_); break;
        default:
            std::cout<<"Undefined UV coordinates for object of type "<<type<<"\n";
            std::exit(EXIT_FAILURE);
    }
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
    return mesh.faces_group(index);
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
        case OBJ_BOX: return box.normal(inter);
        case OBJ_VOL_CONE: return cone.normal(inter);
        case OBJ_CONIC: return conic.normal(inter);
        case OBJ_VOL_CYLINDER: return cylinder.normal(inter);
        case OBJ_DISK: return -unit_vec_x;
        case OBJ_LENS: return lens.normal(inter);
        case OBJ_PARABOLA: return parabola.normal(inter);
        case OBJ_MESH: return face(inter.face).norm;
        case OBJ_RECTANGLE: return -unit_vec_x;
        case OBJ_SPHERE: return sphere.normal(inter);
        case OBJ_SPHERE_PATCH: return sphere_patch.normal(inter);
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
        case OBJ_BOX: return box.tangent(inter,normal,up);
//        case OBJ_VOL_CONE: return tangent_cone_volume(inter);
//        case OBJ_VOL_CYLINDER: return tangent_cylinder_volume(inter);
//        case OBJ_LENS: return tangent_lens(inter);
//        case OBJ_PARABOLA: return tangent_parabola(inter);
//        case OBJ_MESH: return face(inter.face).norm;
        case OBJ_RECTANGLE: return rectangle.tangent(inter,normal,up);
    }
    
    return Vector3(0);
}

Vector3 Object::get_anchor(int anchor)
{
    switch(type)
    {
        case OBJ_BOX: return box.anchor(anchor);
        case OBJ_CONIC: return conic.anchor(anchor);
        case OBJ_LENS: return lens.anchor(anchor);
        case OBJ_PARABOLA: return parabola.anchor(anchor);
        case OBJ_VOL_CONE: return cone.anchor(anchor);
        case OBJ_VOL_CYLINDER: return cylinder.anchor(anchor);
        case OBJ_SPHERE: return sphere.anchor(anchor);
        case OBJ_SPHERE_PATCH: return sphere_patch.anchor(anchor);
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
        case OBJ_BOX: return box.anchor_name(anchor);
        case OBJ_CONIC: return conic.anchor_name(anchor);
        case OBJ_LENS: return lens.anchor_name(anchor);
        case OBJ_PARABOLA: return parabola.anchor_name(anchor);
        case OBJ_VOL_CONE: return cone.anchor_name(anchor);
        case OBJ_VOL_CYLINDER: return cylinder.anchor_name(anchor);
        case OBJ_SPHERE: return sphere.anchor_name(anchor);
        case OBJ_SPHERE_PATCH: return sphere_patch.anchor_name(anchor);
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
            anchor_name=box.anchor_name(anchor);
            break;
        case OBJ_CONIC:
            prefix="SEL_OBJ_CONIC_SECTION_";
            anchor_name=conic.anchor_name(anchor);
            break;
        case OBJ_LENS:
            prefix="SEL_OBJ_LENS_";
            anchor_name=lens.anchor_name(anchor);
            break;
        case OBJ_PARABOLA:
            prefix="SEL_PARABOLA_";
            anchor_name=parabola.anchor_name(anchor);
            break;
        case OBJ_VOL_CONE:
            prefix="SEL_OBJ_CONE_";
            anchor_name=cone.anchor_name(anchor);
            break;
        case OBJ_VOL_CYLINDER:
            prefix="SEL_OBJ_CYL_";
            anchor_name=cylinder.anchor_name(anchor);
            break;
        case OBJ_SPHERE:
            prefix="SEL_OBJ_SPHERE_";
            anchor_name=sphere.anchor_name(anchor);
            break;
        case OBJ_SPHERE_PATCH:
            prefix="SEL_OBJ_SPHERE_PATCH_";
            anchor_name=sphere_patch.anchor_name(anchor);
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
            box.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_VOL_CONE:
            cone.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_CONIC:
            conic.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_VOL_CYLINDER:
            cylinder.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_DISK:
            disk.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_LENS:
            lens.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_MESH:
            mesh.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_PARABOLA:
            parabola.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_RECTANGLE:
            rectangle.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_SPHERE:
            sphere.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
        case OBJ_SPHERE_PATCH:
            sphere_patch.intersect(interlist, ray, obj_ID, face_last_intersect, first_forward);
            break;
    }
}

bool Object::intersect_boundaries_box(SelRay const &ray)
{
    if(type!=OBJ_BOOLEAN)
    {
        double tmin=(bbox.xm-ray.start.x)*ray.inv_dir.x;
        double tmax=(bbox.xp-ray.start.x)*ray.inv_dir.x;
        double tymin=(bbox.ym-ray.start.y)*ray.inv_dir.y;
        double tymax=(bbox.yp-ray.start.y)*ray.inv_dir.y;
        
        if(tmin>tmax) std::swap(tmin,tmax);
        if(tymin>tymax) std::swap(tymin,tymax);
        
        if(tmin>tymax || tymin>tmax) return false;
        if(tymin>tmin) tmin=tymin;
        if(tymax<tmax) tmax=tymax;
        
        double tzmin=(bbox.zm-ray.start.z)*ray.inv_dir.z;
        double tzmax=(bbox.zp-ray.start.z)*ray.inv_dir.z;
        
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
    
    if(abs_ray==true || ray.generation>=max_ray_generation)
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


double* Object::reference_variable(std::string const &variable_name)
{
    if(variables_map.count(variable_name)==0) return nullptr;
    
    return variables_map[variable_name];
}


void Object::save_mesh_to_obj(std::string const &fname)
{
    mesh.save_mesh_to_obj(fname);
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
        
        mesh.set_group_default_in_irf(irf);
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
        
        mesh.set_group_default_in_mat(mat);
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
        
        mesh.set_group_default_irf(irf);
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
        
        mesh.set_group_default_out_irf(irf);
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
        
        mesh.set_group_default_out_mat(mat);
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
        case OBJ_BOX: box.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_VOL_CONE: u=v=0; break;
        case OBJ_VOL_CYLINDER: cylinder.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_DISK: disk.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_LENS: lens.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_PARABOLA: parabola.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_MESH: u=v=0; break;
        case OBJ_RECTANGLE: rectangle.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_SPHERE: sphere.xyz_to_uv(u,v,face_,x,y,z); break;
        case OBJ_SPHERE_PATCH: sphere_patch.xyz_to_uv(u,v,face_,x,y,z); break;
        default:
            std::cout<<"Undefined UV coordinates for object of type "<<type<<"\n";
            std::exit(EXIT_FAILURE);
    }
}

}
