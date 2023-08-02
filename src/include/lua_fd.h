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

#ifndef LUA_FD_H_INCLUDED
#define LUA_FD_H_INCLUDED

#include <lua_base.h>
#include <material.h>
#include <sensors.h>
#include <sources.h>
#include <structure.h>

#include <atomic>
#include <filesystem>
#include <vector>

//####################
//      Sources
//####################

int create_source(lua_State *L);

int source_set_location(lua_State *L);
int source_set_location_real(lua_State *L);
int source_set_orientation(lua_State *L);
int source_set_spectrum(lua_State *L);

void Source_generator_create_metatable(lua_State *L);

//####################
//      Sensors
//####################

int create_sensor(lua_State *L);

int sensor_set_location(lua_State *L);
int sensor_set_location_real(lua_State *L);
//int sensor_set_arbitrary_spectrum(lua_State *L);

void Sensor_generator_create_metatable(lua_State *L);

//####################
//       FDTD
//####################

/*class fdtd_structure_generator
{
    public:
        bool set;
        std::filesystem::path script;
        std::vector<std::string> append_cmd,parameter_cmd;
        
        fdtd_structure_generator();
        
        void append(std::string);
//        [[deprecated]]
//        void discretize(int &Nx,int &Ny,int &Nz,
//                        double &Dx,double &Dy,double &Dz,
//                        Grid3<unsigned int> &matgrid,bool print=true) const;
        void discretize(Grid3<unsigned int> &matgrid,
                        int Nx,int Ny,int Nz,double Dx,double Dy,double Dz,
                        bool print=true) const;
        void parameter(std::string);
        void retrieve_nominal_size(double &lx,double &ly,double &lz) const;
};*/

class FD_Mode: public base_mode
{
    public:
        enum{ MODE_NONE=0 };
        
        int type;
        Structure *structure;
        std::string prefix;
        std::filesystem::path directory;
        double polar_angle;
        std::string polarization;
        
        double Dx,Dy,Dz;
        
        int pad_xm,pad_xp;
        int pad_ym,pad_yp;
        int pad_zm,pad_zp;
        
        bool periodic_x;
        bool periodic_y;
        bool periodic_z;
        
        //PML
        int pml_xm,pml_xp;
        int pml_ym,pml_yp;
        int pml_zm,pml_zp;
        
        double kappa_xm,kappa_xp;
        double kappa_ym,kappa_yp;
        double kappa_zm,kappa_zp;
        
        double sigma_xm,sigma_xp;
        double sigma_ym,sigma_yp;
        double sigma_zm,sigma_zp;
        
        double alpha_xm,alpha_xp;
        double alpha_ym,alpha_yp;
        double alpha_zm,alpha_zp;
        
        //Materials
        #ifdef OLDMAT
        std::vector<int> materials_index;
        std::vector<std::string> materials_str;
        #endif
        
        std::vector<Material> materials;
        
        std::vector<int> disable_fields;
        
        FD_Mode();
        
        double get_Lx();
        double get_Ly();
        double get_Lz();
        
        int get_Nx();
        int get_Ny();
        int get_Nz();
        
        virtual void recompute_padding();
        virtual void reset();
        virtual void show() const;
        
        void compute_discretization(int &Nx,int &Ny,int &Nz,
                                    double lx,double ly,double lz) const;
        
        void set_discretization(double D);
        void set_discretization_x(double Dx);
        void set_discretization_y(double Dy);
        void set_discretization_z(double Dz);
        
        void set_material(int mat_index,std::filesystem::path const &mat_file);
        void set_padding(int pad_xm,int pad_xp,int pad_ym,int pad_yp,int pad_zm,int pad_zp);
        
        void set_pml_xm(int N_pml,double kap,double sig,double alp);
        void set_pml_xp(int N_pml,double kap,double sig,double alp);
        
        void set_pml_ym(int N_pml,double kap,double sig,double alp);
        void set_pml_yp(int N_pml,double kap,double sig,double alp);
        
        void set_pml_zm(int N_pml,double kap,double sig,double alp);
        void set_pml_zp(int N_pml,double kap,double sig,double alp);
        
        void set_polarization(double);
        void set_polarization(std::string polar);
        void set_prefix(std::string name);
        
        void set_output_directory(std::string dir);
        void set_structure(Structure *structure);
        
        //##########
        
        virtual void process();
};

class FDFD_Mode: public FD_Mode
{
    public:
        enum{ FDFD=0 };
        
        int N_theta;
        AngleRad theta_min,theta_max;
        
        int N_phi;
        AngleRad phi_min,phi_max;
        
        //Spectrum
        int Nl;
        double lambda_min,lambda_max;
        
        std::string solver;
        
        bool output_diffraction,output_map;
        
        FDFD_Mode();
        
        void set_azimuth(AngleRad phi_min,AngleRad phi_max,int N_phi);
        void set_incidence(AngleRad theta_min,AngleRad theta_max,int N_th);
        void set_spectrum(double lambda_min,double lambda_max,int Nl);
        
        //##########
        
        void process();
};

class FDMS_Mode: public FD_Mode
{
    public:
        int Nl;
        double lambda_min,lambda_max;
        double lambda_target,nr_target,ni_target;
        std::string solver;
        
        FDMS_Mode();
        
        void set_solver(std::string solver);
        void set_spectrum(double lambda_min,double lambda_max,double Nl);
        void set_target(double lambda,double nr,double ni);
        
        void process();
};

class FDTD_Mode: public FD_Mode
{
    public:
        enum
        {
            FDTD_CUSTOM=0,
            FDTD_LAB,
            FDTD_NORMAL,
            FDTD_OBLIQUE_ARS,
            FDTD_SINGLE_PARTICLE
        };
    
        int Nt,tapering;
        int display_step;
        int time_type;
        double time_mod;
        int cc_step;
        double cc_lmin,cc_lmax;
        double cc_coeff;
        int cc_quant;
        std::string cc_layout;
        
        //Spectrum
        int Nl;
        double lambda_min,lambda_max;
        
        //Sensors
        std::vector<Sensor_generator> sensors;
        
        //Sources
        std::vector<Source_generator> sources;
        
        //Obl phase
        int obl_phase_type,obl_phase_Nkp,obl_phase_skip;
        double obl_phase_kp_ic,obl_phase_kp_fc;
        AngleRad obl_phase_phi;
        AngleRad obl_phase_amin,obl_phase_amax;
        double obl_phase_lmin,obl_phase_lmax;
        AngleRad obl_phase_cut_angle,obl_phase_safe_angle;
        
        FDTD_Mode();
        
        void add_sensor(Sensor_generator const &sens);
        void add_source(Source_generator const &src);
        void delete_sensor(unsigned int ID);
        void delete_source(unsigned int ID);
        void finalize();
        void finalize_thight();
        void reset();
        void set_analysis(double lambda_min,double lambda_max,int Nl);
        void set_auto_tsteps(int Nt,int cc_step,double cc_coeff);
        void set_auto_tsteps(int Nt,int cc_step,
                             double cc_lmin,double cc_lmax,
                             double cc_coeff,int cc_quant,
                             std::string const &cc_layout);
        void set_display_step(int N);
        void set_incidence(AngleRad theta,AngleRad phi);
        void set_N_tsteps(int Nt);
        void set_spectrum(double lambda_min,double lambda_max,int Nl=481);
        void set_structure(std::string s_name);
        void set_structure_aux(std::string s_name);
        void set_time_mod(double md);
        void show() const;
        
        //##########
        
        void process();
};

void FDTD_normal_incidence(FDTD_Mode const &fdtd_mode,
                           std::atomic<bool> *end_computation=nullptr,
                           ProgTimeDisp *dsp=nullptr,Bitmap *bitmap=nullptr);
void FDTD_oblique_biphase(FDTD_Mode const &fdtd_mode,
                          std::atomic<bool> *end_computation=nullptr,
                          ProgTimeDisp *dsp=nullptr);
void FDTD_oblique_phase(FDTD_Mode const &fdtd_mode,
                        std::atomic<bool> *end_computation=nullptr,
                        ProgTimeDisp *dsp=nullptr);
void FDTD_single_particle(FDTD_Mode const &fdtd_mode,
                          std::atomic<bool> *end_computation=nullptr,
                          ProgTimeDisp *dsp=nullptr,Bitmap *bitmap=nullptr);
void mode_default_fdtd(FDTD_Mode const &fdtd_mode,
                       std::atomic<bool> *end_computation=nullptr,
                       ProgTimeDisp *dsp=nullptr,Bitmap *bitmap=nullptr);
void mode_fdtd_lab(FDTD_Mode const &fdtd_mode,
                   std::atomic<bool> *end_computation=nullptr,
                   ProgTimeDisp *dsp=nullptr);
void mode_fdfd(FDFD_Mode const &fdtd_mode);

int FD_mode_disable_fields(lua_State *L);
int FD_mode_get_lx(lua_State *L);
int FD_mode_get_ly(lua_State *L);
int FD_mode_get_lz(lua_State *L);
int FD_mode_get_nx(lua_State *L);
int FD_mode_get_ny(lua_State *L);
int FD_mode_get_nz(lua_State *L);
int FD_mode_set_material(lua_State *L);
int FD_mode_set_output_directory(lua_State *L);
int FD_mode_set_polarization(lua_State *L);
int FD_mode_set_prefix(lua_State *L);
int FD_mode_set_structure(lua_State *L);

int FDFD_mode_set_azimuth(lua_State *L);
int FDFD_mode_set_incidence(lua_State *L);
int FDFD_mode_output_diffraction(lua_State *L);
int FDFD_mode_output_map(lua_State *L);
int FDFD_mode_set_solver(lua_State *L);
int FDFD_mode_set_spectrum(lua_State *L);

int FDMS_mode_get_mode(lua_State *L);

void FDTD_Mode_create_metatable(lua_State *L);
int FDTD_mode_register_sensor(lua_State *L);
int FDTD_mode_register_source(lua_State *L);
int FDTD_mode_set_auto_tsteps(lua_State *L);
int FDTD_mode_set_display_step(lua_State *L);
int FDTD_mode_set_spectrum(lua_State *L);
int FDTD_mode_set_tapering(lua_State *L);
int FDTD_mode_set_time_mod(lua_State *L);
int FDTD_mode_obph_set_cut_angle(lua_State *L);
int FDTD_mode_obph_set_kp_auto(lua_State *L);
int FDTD_mode_obph_set_kp_fixed_angle(lua_State *L);
int FDTD_mode_obph_set_kp_fixed_lambda(lua_State *L);
int FDTD_mode_obph_set_kp_full(lua_State *L);
int FDTD_mode_obph_set_kp_target(lua_State *L);
int FDTD_mode_obph_set_phi(lua_State *L);
int FDTD_mode_obph_skip(lua_State *L);

//##############################
//       Fieldblock Treat
//##############################

class Fblock_treat_mode: public base_mode
{
    public:
        double baseline;
        std::string fname;
        bool blender_output,surface_poynting_compute;
        
        class Extract
        {
            public:
                int dir,field,i1,i2,j1,j2,index;
                std::string fname;
                
                Extract()
                    :dir(0), field(0),
                     i1(-1), i2(-1),
                     j1(-1), j2(-1), index(0), fname("map") {}
                Extract(Extract const &E)
                    :dir(E.dir), field(E.field),
                     i1(E.i1), i2(E.i2),
                     j1(E.i1), j2(E.i2), index(E.index), fname(E.fname)
                {}
                void operator = (Extract const &E)
                {
                    dir=E.dir; field=E.field;
                    i1=E.i1; i2=E.i2;
                    j1=E.i1; j2=E.j2;
                    index=E.index; fname=E.fname;
                }
        };
        
        std::vector<Extract> map;
//        std::vector<int> extract_dir,
//                         extract_field,
//                         extract_index;
//        std::vector<std::string> extract_fname;
        
        bool apply_stencil;
        unsigned int stencil_index;
        std::string stencil_output;
        Structure stencil;
        
        Fblock_treat_mode();
        
        void add_map_extraction(std::string dir,std::string field,int index,std::string fname);
        void add_map_extraction_sub(int field,int dir,int index,std::string const &fname);
        void process();
};

int fblock_treat_mode_apply_stencil(lua_State *L);
int fblock_treat_mode_set_baseline(lua_State *L);
int fblock_treat_mode_extract_map(lua_State *L);
int fblock_treat_mode_set_file(lua_State *L);

int substract_fieldblocks(lua_State *L);

//##############################
//       Fieldmap Treat
//##############################

class Fmap_treat_mode: public base_mode
{
    public:
        double baseline;
        std::string fname;
        int scale_x,scale_y,scale_z;
        
        Fmap_treat_mode();
        
        void process();
};

int fmap_treat_mode_set_baseline(lua_State *L);
int fmap_treat_mode_set_file(lua_State *L);
int fmap_treat_mode_set_scale(lua_State *L);

//###################################
//       Preview fdtd structure
//###################################

class Preview_fdtd_structure_mode: public FD_Mode
{
    public:
//        fdtd_structure_generator structure;
        int N_ext;
        std::vector<unsigned int> ext_target;
        std::vector<std::string> ext_fname;
        
        Preview_fdtd_structure_mode();
        
        void process();
};

int preview_fdtd_structure_mode_extract_discretization(lua_State *L);
int preview_fdtd_structure_mode_set_file(lua_State *L);
int preview_fdtd_structure_mode_struct_append(lua_State *L);
int preview_fdtd_structure_mode_struct_parameter(lua_State *L);

#endif // LUA_FD_H_INCLUDED
