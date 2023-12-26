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

#ifndef LUA_FDTD_H_INCLUDED
#define LUA_FDTD_H_INCLUDED

#include <lua_fd.h>
#include <sensors.h>
#include <sources.h>

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
                   
void FDTD_Mode_create_metatable(lua_State *L);
int FDTD_mode_compute(lua_State *L);
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

#endif
