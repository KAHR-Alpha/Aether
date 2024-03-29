/*Copyright 2008-2022 - Lo�c Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef SELENE_H
#define SELENE_H

#include <list>

#include <filehdl.h>
#include <mathUT.h>
#include <math_optim.h>
#include <phys_tools.h>
#include <selene_mesh.h>
#include <selene_primitives.h>
#include <selene_rays.h>

namespace Sel
{
enum
{
    FRAME,
    EXTENT_CIRCLE,
    EXTENT_ELLIPSE,
    EXTENT_ELLIPSOID,
//    EXTENT_GAUSSIAN_CIRCLE,
//    EXTENT_GAUSSIAN_ELLIPSE,
//    EXTENT_GAUSSIAN_ELLIPSOID,
//    EXTENT_GAUSSIAN_SPHERE,
    EXTENT_POINT,
    EXTENT_RECTANGLE,
    EXTENT_SPHERE,
    OBJ_BOOLEAN,
    OBJ_BOX,
    OBJ_CONIC,
    OBJ_DISK,
    OBJ_LENS,
    OBJ_MESH,
    OBJ_PARABOLA,
    OBJ_PRISM,
    OBJ_VOL_CONE,
    OBJ_VOL_CYLINDER,
    OBJ_RECTANGLE,
    OBJ_SPHERE,
    OBJ_SPHERE_PATCH,
    OBJ_UNSET,
    POLAR_ALONG,
    POLAR_NOT,
    POLAR_UNSET,
    SRC_CONE,
    SRC_GAUSSIAN_BEAM,
    SRC_LAMBERTIAN,
    SRC_PERFECT_BEAM,
    SRC_POINT,
    SRC_POINT_PLANAR,
    SRC_USER_DEFINED,
    SPECTRUM_FLAT,
    SPECTRUM_FILE,
    SPECTRUM_PLANCK,
    SPECTRUM_MONO,
    SPECTRUM_POLY,
    SPECTRUM_POLYMONO
};

class Frame
{
    public:
        Vector3 loc;
        Vector3 local_x,local_y,local_z;
        
        std::string name;
        std::map<std::string,double*> variables_map;
        
        int type,
            origin_anchor,
            relative_anchor;
            
        Vector3 in_displacement;
        Angle<AngleStorage::DEGREE> in_A,in_B,in_C;
        
        Frame *relative_origin;
        Frame *translation_frame;
        Frame *rotation_frame;
        
        Frame()
            :loc(0,0,0),
             local_x(1,0,0),local_y(0,1,0),local_z(0,0,1),
             type(FRAME),
             origin_anchor(0),
             relative_anchor(0),
             in_displacement(0),
             in_A(0), in_B(0), in_C(0),
             relative_origin(nullptr),
             translation_frame(nullptr),
             rotation_frame(nullptr)
        {}
        
        virtual ~Frame()
        {
        }
        
        bool cyclic_check(Frame *frame)
        {
            if(frame==this) return true;
            
            if(relative_origin!=nullptr)
            {
                if(relative_origin==frame) return true;
                else if(relative_origin->cyclic_check(frame)) return true;
            }
            
            if(translation_frame!=nullptr)
            {
                if(translation_frame==frame) return true;
                else if(translation_frame->cyclic_check(frame)) return true;
            }
            
            if(rotation_frame!=nullptr)
            {
                if(rotation_frame==frame) return true;
                else if(rotation_frame->cyclic_check(frame)) return true;
            }
            
            return false;
        }
        
        bool cyclic_check()
        {
            std::vector<Frame*> backtrace;
            
            return cyclic_check(backtrace);
        }
        
        bool cyclic_check(std::vector<Frame*> const &backtrace_)
        {
            std::vector<Frame*> backtrace;
            
            backtrace=backtrace_; // Copy instead of accumulation to accomodate the three branches
            backtrace.push_back(this);
            
            for(int i=0;i<3;i++)
            {
                Frame *p;
                
                     if(i==0) p=relative_origin;
                else if(i==1) p=translation_frame;
                else if(i==2) p=rotation_frame;
                
                if(p!=nullptr)
                {
                    if(vector_contains(backtrace,p)) return false;
                    if(p->cyclic_check(backtrace)==false) return false;
                }
            }
            
            return true;
        }
        
        void consolidate_position();
        
        void forget_frame(Sel::Frame *frame)
        {
            if(relative_origin==frame)
            {
                relative_origin=nullptr;
                relative_anchor=0;
            }
            
            if(translation_frame==frame) translation_frame=nullptr;
            if(rotation_frame==frame) rotation_frame=nullptr;
        }
        
        virtual Vector3 get_anchor(int anchor) { return Vector3(0); }
        virtual int get_anchors_number() { return 0; }
        virtual std::string get_anchor_name(int anchor) { return ""; }
        virtual std::string get_anchor_script_name(int anchor) { return ""; }
        void set_origin(int anchor) { origin_anchor=anchor; }
        
        void set_displacement(double x,double y,double z)
        {
            in_displacement.x=x;
            in_displacement.y=y;
            in_displacement.z=z;
        }
        
        void set_relative_origin(Frame *origin,int anchor)
        {
            relative_origin=origin;
            relative_anchor=anchor;
        }
        
        void set_rotation(double A,double B,double C)
        {
            in_A.degree(A);
            in_B.degree(B);
            in_C.degree(C);
        }
        
        void set_rotation_frame(Frame *frame)
        {
            rotation_frame=frame;
        }
        
        void set_translation_frame(Frame *frame) { translation_frame=frame; }
        
        Vector3 to_global(Vector3 const &V)
        {
            return V.x*local_x+V.y*local_y+V.z*local_z;
        }
        
        Vector3 to_local(Vector3 const &V)
        {
            Vector3 out;
            
            out.x=scalar_prod(local_x,V);
            out.y=scalar_prod(local_y,V);
            out.z=scalar_prod(local_z,V);
            
            return out;
        }
};

class Light: public Frame
{
    public:
        int spectrum_type,extent;
        int polar_type;
        Vector3 polar_vector;
        
        // Cone
        
        double cone_angle;
        
        // Beam
        
        double beam_numerical_aperture,
               beam_waist_distance;
        
        // Discrete spectra
        
        double lambda_mono;
        std::vector<double> polymono_lambda,polymono_weight;
        double polymono_weight_sum;
        
        // Continuous spectrum
        
        int spectrum_shape;
        double lambda_min,lambda_max;
        double planck_temperature,planck_max;
        std::string spectrum_file;
        double file_max;
        std::vector<double> spf_x,spf_y;
        
        //
        
        double power;
        double extent_x,extent_y,extent_z,extent_d;
        Material *amb_mat;
        
        // User defined
        
        AsciiDataLoader ray_file;
        std::vector<double> user_ray_buffer;
        
        int NRays_sent;
        
        Light();
        
        void bootstrap();
        Vector3 compute_polarization(Vector3 const &local_ray_dir);
        double compute_wavelength();
        Vector3 get_anchor(int anchor);                 // switch
        std::string get_anchor_name(int anchor);        // switch
        std::string get_anchor_script_name(int anchor); // switch
        int get_anchors_number();                       // switch
        double get_power();
        void get_ray(SelRay &ray);
        void load_spectrum_file(std::string const &fname);
        void reset_ray_counter();
        void set_power(double power);
        void set_spectrum_file(std::string const &fname);
        void set_spectrum_flat(double lambda_min,double lambda_max);
        void set_spectrum_planck(double lambda_min,double lambda_max,double T);
        void set_spectrum_type(int type);
        void set_type(int type);
};

class Object: public Frame
{
    public:
        enum Boolean_Type{EXCLUDE,INTERSECT,UNION};
        
        int obj_ID;
        int max_ray_generation;
        
        BoundingBox bbox;
        
        int NFc;
        std::vector<Sel::SelFace> F_arr;
        std::vector<std::string> face_name_arr;
        
        Object();
        ~Object();
        
        void build_variables_map();
        void bootstrap(std::filesystem::path const &output_directory,double ray_power,int max_ray_bounces);   // switch
        void cleanup();
        bool contains(double x,double y,double z);
        void default_N_uv(int &Nu,int &Nv,int face);   // switch
        //void define_faces_group(int index,int start,int end);
        SelFace& face(int index);
        SelFace& faces_group(int index);
        std::string face_name(int index);
        Vector3 face_normal(RayInter const &inter);    // switch
        Vector3 face_tangent(RayInter const &inter,
                             Vector3 const &normal,bool up);    // switch
        Vector3 get_anchor(int anchor);                 // switch
        std::string get_anchor_name(int anchor);        // switch
        std::string get_anchor_script_name(int anchor); // switch
        int get_anchors_number();                       // switch
        int get_N_faces();
        //int get_N_faces_groups();
        std::filesystem::path get_sensor_file_path() const;
        std::string get_type_name();                    // switch
        void intersect(SelRay const &ray,std::vector<RayInter> &inter_list,int face_last_intersect=-1,bool first_forward=true); //switch
        bool intersect_boundaries_box(SelRay const &ray);
        void process_intersection(RayPath &path);
        //void propagate_faces_group(int index);
        void save_mesh_to_obj(std::string const &fname);
        double* reference_variable(std::string const &variable_name);
        void set_default_in_irf(IRF *irf);
        void set_default_in_mat(Material *mat);
        void set_default_irf(IRF *irf);
        void set_default_out_irf(IRF *irf);
        void set_default_out_mat(Material *mat);
        void set_sens_abs();
        void set_sens_none();
        void set_sens_transp();
        void to_local_ray(SelRay &ray,SelRay const &base_ray);
        void update_geometry();                         // switch
        void xyz_to_uv(double &u,double &v,int face,
                       double x,double y,double z);   // switch
        
        // Boolean Type
        
        Boolean_Type boolean_type;
        Object* bool_obj_1;
        Object* bool_obj_2;
        std::vector<RayInter> bool_buffer_1,bool_buffer_2;
        
        void intersect_boolean(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward);
        Vector3 normal_boolean(RayInter inter);
        Vector3 tangent_boolean(RayInter inter);
        void set_boolean(Object *bool_obj_1,Object *bool_obj_2,Boolean_Type type);
        void set_boolean_intersection(Object *bool_obj_1,Object *bool_obj_2);
        void set_boolean_union(Object *bool_obj_1,Object *bool_obj_2);
        
        // Box Type
        
        Primitives::Box box;
        
        void set_box();
        void set_box(double lx,double lyh,double lz);
        
        // Cone Type
        
        Primitives::Cone cone;
        
        void set_cone_volume();
        void set_cone_volume(double length,double radius,double cut);
                
        // Conic Section Type
        
        Primitives::Conic conic;
        
        void set_conic_section();
        
        // Cylinder Type
        
        Primitives::Cylinder cylinder;
        
        void set_cylinder_volume();
        void set_cylinder_volume(double length,double radius,double cut);
                
        // Disk Type
        
        Primitives::Disk disk;
        
        void set_disk();
        void set_disk(double radius,double in_radius);
        
        // Lens Functions
        
        Primitives::Lens lens;

        void set_lens();
        void set_lens(double thickness,double r_max,double r1,double r2);
        
        // Mesh Functions
        
        Primitives::Mesh mesh;
        
        void set_mesh();
        void set_mesh(std::vector<Sel::Vertex> const &V_arr,std::vector<Sel::SelFace> const &F_arr);
                
        // Parabola
        
        Primitives::Parabola parabola;
        
        void set_parabola();
        void set_parabola(double focal_length,double in_radius,double height);
        
        // Prism Functions
        
        void intersect_prism(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward);
        Vector3 normal_prism(RayInter const &inter);
        void set_prism(double r);
        
        // Rectangle Functions
        
        Primitives::Rectangle rectangle;

        void set_rectangle();
        void set_rectangle(double ly,double lz);
        
        // Sphere Functions
        
        Primitives::Sphere sphere;
        
        void set_sphere();
        void set_sphere(double r,double cut=1.0);

        // Spherical Patch Functions

        Primitives::Sphere sphere_patch;
        
        void set_spherical_patch();
        void set_spherical_patch(double r,double cut=1.0);
        
        // Spheroid Functions
        
        double spd_rx,spd_ry,spd_rz,spd_cut;
        
        void set_spheroid();
        void set_spheroid(double rx,double ry,double rz,double cut=1.0);
        
        Vector3 spheroid_anchor(int anchor);
        std::string spheroid_anchor_name(int anchor);
        void intersect_spheroid(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward);
        Vector3 normal_spheroid(RayInter const &inter);
        Vector3 tangent_spheroid(RayInter const &inter);
        
        // Sensor
        
        bool cleanup_done;
        Sensor sensor_type;
        
        bool sens_wavelength,
             sens_source,
             sens_path,
             sens_generation,
             sens_length,
             sens_phase,
             sens_ray_world_intersection,
             sens_ray_world_direction,
             sens_ray_world_polar,
             sens_ray_obj_intersection,
             sens_ray_obj_direction,
             sens_ray_obj_polar,
             sens_ray_obj_face;
        
        int sb_Nmax,sb_Ncurr,sb_Ntot;
        std::filesystem::path sb_fname;
        std::vector<int> sb_source,sb_path,sb_generation,sb_face;
        std::vector<double> sb_lambda,sb_opl,sb_phase;
        std::vector<Vector3> sb_world_i,sb_world_d,sb_world_polar,
                             sb_obj_i,sb_obj_d,sb_obj_polar;
        std::ofstream sb_file;
        
        void sens_buffer_add(SelRay &ray,
                             Vector3 const &world_intersection,
                             Vector3 const &obj_intersection,
                             int face_hit);
        void sens_buffer_dump();
};


enum
{
    RC_COLOR,
    RC_COUNTING,
    RC_POWER,
    SP_FULL,
    SP_MINMAX,
    SP_WINDOW
};


class RayCounter
{
    public:
        bool owner;
        Sel::Object *object;
        int N_faces,computation_type;
        
        // Spectral filter
        int spectral_mode;
        double lambda_min,lambda_max;
        
        // Data
        bool empty_sensor;
        AsciiDataLoader loader;
        std::filesystem::path sensor_fname;
        
        double ray_unit;
        std::vector<Grid2<double>> bins;
        std::vector<double> Du,Dv;
        std::vector<int> Nu,Nv;
        
        int lambda_column,
            source_column,
            path_column,
            generation_column,
            phase_column,
            obj_inter_column,
            obj_dir_column,
            obj_polar_column,
            face_column;
        
        bool has_lambda,
             has_source,
             has_path,
             has_generation,
             has_phase,
             has_polarization;
        
        std::vector<double> lambda,phase;
        std::vector<Vector3> obj_inter,obj_dir,obj_polarization;
        std::vector<int> source,path,generation,face;
        
        RayCounter();
        
        double compute_angular_spread();
        double compute_hit_count();
        double compute_spatial_spread();
        void set_sensor(Sel::Object *object);
        void set_sensor(std::filesystem::path const &sensor_file);
        void update();
        void update_from_file();
    
    private:
        void initialize();
        void reallocate();
};


enum class OptimGoal
{
    MAXIMIZE_HIT_COUNT,
    MINIMIZE_ANGULAR_SPREAD,
    MINIMIZE_SPATIAL_SPREAD,
    TARGET_HIT_COUNT
};


class OptimTarget: public ::OptimTarget
{
    public:
        Sel::Object *sensor;
        OptimGoal goal;
        double target_value;
        
        double evaluate() const override;
};


class Selene
{
    private:
        int Nobj;
        int Nlight;
        int render_number;
        std::vector<Object*> obj_arr;
        std::vector<Light*> light_arr;
        
        double ray_power;
        std::vector<int> light_N_rays;
        
        std::vector<RayInter> intersection_buffer;
        
        unsigned int Nr_bounces;
        unsigned int Nr_disp;
        unsigned int Nr_tot;
        unsigned int Nr_cast;
        unsigned int trace_calls;
        
        std::vector<unsigned int> light_Nr_disp;
        
        RayPath request_job();
        
        std::filesystem::path output_directory;
    public:
        
        Selene();
        ~Selene();
        
        //Ray fetcher
        
        unsigned int fetch_family,
                     N_fetched_families,
                     ray_family_counter,
                     fetched_source;
        std::vector<int> gen_ftc;
        std::vector<double> lambda_ftc,xs_ftc,ys_ftc,zs_ftc,
                                       xe_ftc,ye_ftc,ze_ftc;
        std::vector<bool> lost_ftc;
        
        void add_light(Light *src);
        void add_object(Object *obj);
        void fetch_ray(SelRay const &ray);
        void fetch_ray_lost(SelRay const &ray);
        void render();
        void render(int Nr_disp,int Nr_tot);
        void request_raytrace(RayPath &ray_path);
        void reset_fetcher();
        void set_max_ray_bounces(int Nr_bounces);
        void set_N_rays_disp(int Nr_disp);
        void set_N_rays_total(int Nr_tot);
        void set_output_directory(std::filesystem::path const &output_directory);
};

std::ostream& operator << (std::ostream &strm,RayPath const &ray_path);

void lens_geometry(double &A,double &B,double &r_max,double &th_1,double &th_2,
                   double thickness,double r_max_,double r1,double r2);
int nearest_2np1(double val);
                   
void generate_intersection(RayInter &inter,SelRay const &ray,double t,int face_hit,int obj_ID);

template<std::size_t N>
double array_min_pos(double threshold,std::array<double,N> vals)
{
    std::size_t i;
    
    double val_out=threshold;
    
    for(i=0;i<N;i++) if(vals[i]>threshold) { val_out=vals[i]; break; }
    for(i=0;i<N;i++) if(vals[i]>threshold) val_out=std::min(val_out,vals[i]);
    
    return val_out;
}

template<std::size_t N>
void push_first_forward(std::vector<RayInter> &interlist,SelRay const &ray,int obj_ID,
                        std::array<double,N> const &hits,std::array<int,N> const &face_labels)
{
    double t_min=array_min_pos<N>(1e-6,hits);
    
    for(std::size_t i=0;i<N;i++)
    {
        if(t_min==hits[i])
        {
            RayInter inter_out;
            generate_intersection(inter_out,ray,hits[i],face_labels[i],obj_ID);
            
            interlist.push_back(inter_out);
            
            return;
        }
    }
}

template<std::size_t N>
void push_full_forward(std::vector<RayInter> &interlist,SelRay const &ray,int obj_ID,
                       std::array<double,N> const &hits,std::array<int,N> const &face_labels)
{
    RayInter inter_out;
    
    for(std::size_t i=0;i<N;i++)
    {
        if(hits[i]>1e-6)
        {
            generate_intersection(inter_out,ray,hits[i],face_labels[i],obj_ID);
            
            interlist.push_back(inter_out);
        }
    }
}
}

#endif // SELENE_H
