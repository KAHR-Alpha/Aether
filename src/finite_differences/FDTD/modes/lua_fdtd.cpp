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

#include <fieldblock_holder.h>
#include <lua_fdtd.h>
#include <string_tools.h>

//####################
//     FDTD Mode
//####################

FDTD_Mode::FDTD_Mode()
    :FD_Mode(),
     Nt(5000), tapering(0),
     display_step(-1),
     time_type(TIME_FIXED), 
     time_mod(1.0),
     cc_step(500),
     cc_lmin(370e-9), cc_lmax(850e-9),
     cc_coeff(1e-3), cc_quant(500),
     cc_layout("nnb"),
     Nl(481), lambda_min(370e-9), lambda_max(850e-9),
     obl_phase_type(0), obl_phase_Nkp(1), obl_phase_skip(0),
     obl_phase_kp_ic(0), obl_phase_kp_fc(1.0),
     obl_phase_phi(0),
     obl_phase_amin(0), obl_phase_amax(0),
     obl_phase_lmin(500e-9), obl_phase_lmax(500e-9),
     obl_phase_cut_angle(Degree(75)),
     obl_phase_safe_angle(Degree(65))
{
    type=FDTD_CUSTOM;
}

void FDTD_Mode::add_sensor(Sensor_generator const &sens)
{
    sensors.push_back(sens);
}

void FDTD_Mode::add_source(Source_generator const &src)
{
    sources.push_back(src);
}

void FDTD_Mode::delete_sensor(unsigned int ID)
{
    if(ID>=sensors.size()) return;
    
    std::vector<Sensor_generator>::const_iterator it=sensors.begin()+ID;
    
    sensors.erase(it);
}

void FDTD_Mode::delete_source(unsigned int ID)
{
    if(ID>=sources.size()) return;
    
    std::vector<Source_generator>::const_iterator it=sources.begin()+ID;
    
    sources.erase(it);
}

void FDTD_Mode::finalize()
{
    unsigned int i;
    
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    for(i=0;i<sensors.size();i++)
        sensors[i].to_discrete(Dx,Dy,Dz);
    
    for(i=0;i<sources.size();i++)
        sources[i].to_discrete(Dx,Dy,Dz);
    
    if(type==FDTD_NORMAL || type==FDTD_OBLIQUE_ARS)
    {
        pml_xm=pml_xp=0;
        pml_ym=pml_yp=0;
        
        if(pml_zm<=0) pml_zm=25;
        if(pml_zp<=0) pml_zp=25;
    }
    
    if(Nx>1)
    {
        if(pml_xm)
        {
            pad_xm=std::max(pad_xm,5);
            
            for(i=0;i<sensors.size();i++)
                pad_xm=std::max(pad_xm,5-sensors[i].x1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_xm=std::max(pad_xm,50-sources[i].x1);
                else pad_xm=std::max(pad_xm,5-sources[i].x1);
            }
        }
        else pad_xm=0;
        
        if(pml_xp)
        {
            pad_xp=std::max(pad_xp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_xp=std::max(pad_xp,5+sensors[i].x2-Nx);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_xp=std::max(pad_xp,50+sources[i].x2-Nx);
                else pad_xp=std::max(pad_xp,5+sources[i].x2-Nx);
            }
        }
        else pad_xp=0;
    }
    
    if(Ny>1)
    {
        if(pml_ym)
        {
            pad_ym=std::max(pad_ym,5);
            
            for(i=0;i<sensors.size();i++)
                pad_ym=std::max(pad_ym,5-sensors[i].y1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_ym=std::max(pad_ym,50-sources[i].y1);
                else pad_ym=std::max(pad_ym,5-sources[i].y1);
            }
        }
        else pad_ym=0;
        
        if(pml_yp)
        {
            pad_yp=std::max(pad_yp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_yp=std::max(pad_yp,5+sensors[i].y2-Ny);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_yp=std::max(pad_yp,50+sources[i].y2-Ny);
                else pad_yp=std::max(pad_yp,5+sources[i].y2-Ny);
            }
        }
        else pad_yp=0;
    }
    
    if(Nz>1)
    {
        if(pml_zm)
        {
            pad_zm=std::max(pad_zm,5);
            
            for(i=0;i<sensors.size();i++)
                pad_zm=std::max(pad_zm,5-sensors[i].z1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_zm=std::max(pad_zm,50-sources[i].z1);
                else pad_zm=std::max(pad_zm,5-sources[i].z1);
            }
        }
        else pad_zm=0;
        
        if(pml_zp)
        {
            pad_zp=std::max(pad_zp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_zp=std::max(pad_zp,5+sensors[i].z2-Nz);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_zp=std::max(pad_zp,50+sources[i].z2-Nz);
                else pad_zp=std::max(pad_zp,5+sources[i].z2-Nz);
            }
        }
        else pad_zp=0;
    }
}

void FDTD_Mode::finalize_thight()
{
    if(type==FDTD_NORMAL || type==FDTD_OBLIQUE_ARS)
    {
        pml_xm=pml_xp=0;
        pml_ym=pml_yp=0;
        
        if(pml_zm<=0) pml_zm=25;
        if(pml_zp<=0) pml_zp=25;
    }
    
//    if(Nx>1)
    {
        if(pml_xm) pad_xm=5;
        else pad_xm=0;
        
        if(pml_xp) pad_xp=5;
        else pad_xp=0;
    }
    
//    if(Ny>1)
    {
        if(pml_ym) pad_ym=5;
        else pad_ym=0;
        
        if(pml_yp)pad_yp=5;
        else pad_yp=0;
    }
    
//    if(Nz>1)
    {
        if(pml_zm) pad_zm=5;
        else pad_zm=0;
        
        if(pml_zp) pad_zp=5;
        else pad_zp=0;
    }
    
    finalize();
}

void FDTD_Mode::reset()
{
    FD_Mode::reset();
    
    Nt=5000; tapering=0;
    display_step=-1;
    time_type=TIME_FIXED; 
    time_mod=1.0;
    cc_step=500;
    cc_lmin=370e-9; cc_lmax=850e-9;
    cc_coeff=1e-3; cc_quant=500;
    cc_layout="nnb";
    Nl=481; lambda_min=370e-9; lambda_max=850e-9;
    obl_phase_type=0; obl_phase_Nkp=1; obl_phase_skip=0;
    obl_phase_kp_ic=0; obl_phase_kp_fc=1.0;
    obl_phase_phi=0;
    obl_phase_amin=0; obl_phase_amax=0;
    obl_phase_lmin=500e-9; obl_phase_lmax=500e-9;
    obl_phase_cut_angle=Degree(75);
    obl_phase_safe_angle=Degree(65);

    sensors.clear();
    sources.clear();
}

void FDTD_Mode::set_auto_tsteps(int Nt_,int cc_step_,double cc_coeff_)
{
    Nt=Nt_;
    cc_step=cc_step_;
    cc_coeff=cc_coeff_;
    
    time_type=TIME_ENERGY;
}

void FDTD_Mode::set_auto_tsteps(int Nt_,int cc_step_,
                                double cc_lmin_,double cc_lmax_,
                                double cc_coeff_,int cc_quant_,
                                std::string const &cc_layout_)
{
    Nt=Nt_;
    cc_step=cc_step_;
    cc_lmin=cc_lmin_;
    cc_lmax=cc_lmax_;
    cc_coeff=cc_coeff_;
    cc_quant=cc_quant_;
    cc_layout=cc_layout_;
    
    time_type=TIME_FT;
}

void FDTD_Mode::process()
{
    finalize();
    
    if(type==FDTD_NORMAL)
    {
        FDTD_normal_incidence(*this);
    }
    else if(type==FDTD_OBLIQUE_ARS)
    {
        FDTD_oblique_biphase(*this);
    }
    else if(type==FDTD_SINGLE_PARTICLE)
    {
        FDTD_single_particle(*this);
    }
    else if(type==FDTD_CUSTOM)
    {
        mode_default_fdtd(*this);
    }
    else if(type==FDTD_LAB)
    {
        mode_fdtd_lab(*this);
    }
}

void FDTD_Mode::set_N_tsteps(int Nt_)
{
    Nt=Nt_;
    time_type=TIME_FIXED;
    
    std::cout<<"Setting the number of time steps to "<<Nt<<std::endl;
}

void FDTD_Mode::set_display_step(int N)
{
    display_step=N;
}

void FDTD_Mode::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

void FDTD_Mode::set_time_mod(double md) { time_mod=md; }

void FDTD_Mode::show() const
{
    FD_Mode::show();
    
    std::cout<<"FDTD Mode"<<std::endl;
    chk_msg_sc(Nt);
    chk_msg_sc(display_step);
    chk_msg_sc(time_type);
    chk_msg_sc(time_mod);
    chk_msg_sc(cc_step);
    chk_msg_sc(cc_lmin);
    chk_msg_sc(cc_lmax);
    chk_msg_sc(cc_coeff);
    chk_msg_sc(cc_quant);
    chk_msg_sc(Nl);
    chk_msg_sc(lambda_min);
    chk_msg_sc(lambda_max);
}

void FDTD_Mode_create_metatable(lua_State *L)
{
    create_obj_metatable(L,"metatable_fdtd");
    
    metatable_add_func(L,"auto_tsteps",FDTD_mode_set_auto_tsteps);
    metatable_add_func(L,"compute",FDTD_mode_compute);
    metatable_add_func(L,"display_step",FDTD_mode_set_display_step);
    lua_wrapper<1,FDTD_Mode,double>::bind(L,"Dx",&FDTD_Mode::set_discretization_x);
    lua_wrapper<2,FDTD_Mode,double>::bind(L,"Dxyz",&FDTD_Mode::set_discretization);
    lua_wrapper<3,FDTD_Mode,double>::bind(L,"Dy",&FDTD_Mode::set_discretization_y);
    lua_wrapper<4,FDTD_Mode,double>::bind(L,"Dz",&FDTD_Mode::set_discretization_z);
    metatable_add_func(L,"Lx",FD_mode_get_lx);
    metatable_add_func(L,"Ly",FD_mode_get_ly);
    metatable_add_func(L,"Lz",FD_mode_get_lz);
    metatable_add_func(L,"material",FD_mode_set_material);
    lua_wrapper<5,FDTD_Mode,int>::bind(L,"N_tsteps",&FDTD_Mode::set_N_tsteps);
    metatable_add_func(L,"Nx",FD_mode_get_nx);
    metatable_add_func(L,"Ny",FD_mode_get_ny);
    metatable_add_func(L,"Nz",FD_mode_get_nz);
    metatable_add_func(L,"output_directory",FD_mode_set_output_directory);
    lua_wrapper<6,FDTD_Mode,int,int,int,int,int,int>::bind(L,"padding",&FDTD_Mode::set_padding);
    lua_wrapper<7,FDTD_Mode,int,double,double,double>::bind(L,"pml_xm",&FDTD_Mode::set_pml_xm);
    lua_wrapper<8,FDTD_Mode,int,double,double,double>::bind(L,"pml_xp",&FDTD_Mode::set_pml_xp);
    lua_wrapper<9,FDTD_Mode,int,double,double,double>::bind(L,"pml_ym",&FDTD_Mode::set_pml_ym);
    lua_wrapper<10,FDTD_Mode,int,double,double,double>::bind(L,"pml_yp",&FDTD_Mode::set_pml_yp);
    lua_wrapper<11,FDTD_Mode,int,double,double,double>::bind(L,"pml_zm",&FDTD_Mode::set_pml_zm);
    lua_wrapper<12,FDTD_Mode,int,double,double,double>::bind(L,"pml_zp",&FDTD_Mode::set_pml_zp);
    metatable_add_func(L,"polarization",FD_mode_set_polarization);
    metatable_add_func(L,"prefix",FD_mode_set_prefix);
    metatable_add_func(L,"structure",FD_mode_set_structure);
    metatable_add_func(L,"tapering",FDTD_mode_set_tapering);
    metatable_add_func(L,"time_mod",FDTD_mode_set_time_mod);
    
    metatable_add_func(L,"cut_angle",FDTD_mode_obph_set_cut_angle);
    metatable_add_func(L,"kp_auto",FDTD_mode_obph_set_kp_auto);
    metatable_add_func(L,"kp_fixed_angle",FDTD_mode_obph_set_kp_fixed_angle);
    metatable_add_func(L,"kp_fixed_lambda",FDTD_mode_obph_set_kp_fixed_lambda);
    metatable_add_func(L,"kp_full",FDTD_mode_obph_set_kp_full);
    metatable_add_func(L,"kp_skip",FDTD_mode_obph_skip);
    metatable_add_func(L,"kp_target",FDTD_mode_obph_set_kp_target);
    metatable_add_func(L,"incident_plane",FDTD_mode_obph_set_phi);
    
    // Non-trivial functions
    
    metatable_add_func(L,"disable_fields",FD_mode_disable_fields);
    metatable_add_func(L,"register_sensor",FDTD_mode_register_sensor);
    metatable_add_func(L,"register_source",FDTD_mode_register_source);
    metatable_add_func(L,"spectrum",FDTD_mode_set_spectrum);
}

int FDTD_mode_compute(lua_State *L)
{
    FDTD_Mode *p_fdtd=lua_get_metapointer<FDTD_Mode>(L,1);
    
    p_fdtd->process();
    
    return 0;
}

int FDTD_mode_register_sensor(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    Sensor_generator **pp_sens=reinterpret_cast<Sensor_generator**>(lua_touserdata(L,2));
    
    (*pp_fdtd)->add_sensor(**pp_sens);
    
    return 1;
}

int FDTD_mode_register_source(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    Source_generator **pp_src=reinterpret_cast<Source_generator**>(lua_touserdata(L,2));
    
    (*pp_fdtd)->add_source(**pp_src);
    
    return 1;
}

int FDTD_mode_set_auto_tsteps(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    if(lua_gettop(L)==4)
    {
        int Nt=lua_tointeger(L,2);
        int step=lua_tointeger(L,3);
        double coeff=lua_tonumber(L,4);
        
        (*pp_fdtd)->set_auto_tsteps(Nt,step,coeff);
    }
    else
    {
        int Nt=lua_tointeger(L,2);
        int step=lua_tointeger(L,3);
        double lmin=lua_tonumber(L,4);
        double lmax=lua_tonumber(L,5);
        double coeff=lua_tonumber(L,6);
        int quant=lua_tointeger(L,7);
        std::cout<<"Setting the number of time steps to automatic mode with coefficient "<<coeff<<std::endl;
        
        std::string cc_layout="nnb";
        
        if(lua_gettop(L)==8) cc_layout=lua_tostring(L,8);
        
        (*pp_fdtd)->set_auto_tsteps(Nt,step,lmin,lmax,coeff,quant,cc_layout);
    }
    
    return 1;
}

int FDTD_mode_set_display_step(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    int display_step=lua_tointeger(L,2);
    std::cout<<"Setting the display step to "<<display_step<<std::endl;
    
    (*pp_fdtd)->set_display_step(display_step);
    
    return 1;
}

int FDTD_mode_set_spectrum(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double lambda_min=lua_tonumber(L,2);
    double lambda_max=lua_tonumber(L,3);
    
    if(lua_gettop(L)==3)
    {
        (*pp_fdtd)->set_spectrum(lambda_min,lambda_max);
    
        std::cout<<"Setting the spectrum between " <<add_unit_u(lambda_min)<<" and "<<add_unit_u(lambda_max)<<std::endl;
    }
    else if(lua_gettop(L)==4)
    {
        int Nl=lua_tointeger(L,4);
        (*pp_fdtd)->set_spectrum(lambda_min,lambda_max,Nl);
    
        std::cout<<"Setting the analysis to "<<Nl<<" points between "
                 <<add_unit_u(lambda_min)<<" and "<<add_unit_u(lambda_max)<<std::endl;
    }
    
    return 1;
}

int FDTD_mode_set_tapering(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double Ntap=lua_tointeger(L,2);
    
    std::cout<<"Setting the tapering to "<<Ntap<<" time steps"<<std::endl;
    
    (*pp_fdtd)->tapering=Ntap;
    
    return 1;
}

int FDTD_mode_set_time_mod(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double time_mod=lua_tonumber(L,2);
    
    std::cout<<"Setting the time modifier to "<<time_mod<<std::endl;
    
    (*pp_fdtd)->set_time_mod(time_mod);
    
    return 1;
}

//####################
//     FDTD Mode
//   Oblique Phase
//####################

int FDTD_mode_obph_set_cut_angle(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_cut_angle=Degree(lua_tonumber(L,2));
    fm.obl_phase_safe_angle=Degree(lua_tonumber(L,3));
    
    return 1;
}

int FDTD_mode_obph_set_kp_auto(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_AUTO;
    fm.obl_phase_amin=lua_tonumber(L,2)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,3)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_kp_fixed_angle(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FIXED_A;
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_lmin=lua_tonumber(L,3);
    fm.obl_phase_lmax=lua_tonumber(L,4);
    fm.obl_phase_amin=lua_tonumber(L,5)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,5)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_kp_fixed_lambda(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FIXED_L;
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_lmin=fm.obl_phase_lmax=lua_tonumber(L,3);
    fm.obl_phase_amin=lua_tonumber(L,4)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,5)*Pi/180.0;
        
    return 1;
}

int FDTD_mode_obph_set_kp_full(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FULL;
    
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_kp_ic=lua_tonumber(L,3);
    fm.obl_phase_kp_fc=lua_tonumber(L,4);
    
    return 1;
}

int FDTD_mode_obph_set_kp_target(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_TARGET;
    fm.obl_phase_Nkp=1;
    fm.obl_phase_lmin=fm.obl_phase_lmax=lua_tonumber(L,2);
    fm.obl_phase_amin=lua_tonumber(L,3)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,3)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_phi(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_phi=Degree(lua_tonumber(L,2));
    
    return 1;
}

int FDTD_mode_obph_skip(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_skip=lua_tointeger(L,2);
    
    return 1;
}

//##############################
//       Fieldblock Treat
//##############################

Fblock_treat_mode::Fblock_treat_mode()
    :baseline(1.0), fname(""),
     blender_output(false),
     surface_poynting_compute(false),
     apply_stencil(false)
{
}

int fblock_treat_mode_apply_stencil(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    Fblock_treat_mode &fb=*(*pp_fb);
    
    fb.apply_stencil=true;
    fb.stencil.set_script(lua_tostring(L,2));
    fb.stencil_index=lua_tointeger(L,3);
    fb.stencil_output=lua_tostring(L,4);
    
    return 0;
}

int fblock_treat_mode_integrate_field(lua_State *L)
{
    return 0;
}

int fblock_treat_mode_set_baseline(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    
    (*pp_fb)->baseline=lua_tonumber(L,2);
    
    return 0;
}

int fblock_treat_mode_set_file(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    
    std::string fname=lua_tostring(L,2);
    
    (*pp_fb)->fname=fname;
    
    return 0;
}

int substract_fieldblocks(lua_State *L)
{
    std::string fname_out=lua_tostring(L,1);
    std::string fname_1=lua_tostring(L,2);
    std::string fname_2=lua_tostring(L,3);
    
    FieldBlockHolder fholder_1,fholder_2;
    
    fholder_1.load(fname_1);
    fholder_2.load(fname_2);
    
    FieldBlockHolder fholder_out(fholder_1);
    
    int i,j,k;
    
    for(i=0;i<fholder_1.Nx;i++) for(j=0;j<fholder_1.Ny;j++) for(k=0;k<fholder_1.Nz;k++)
    {
        fholder_out.Ex(i,j,k)=fholder_1.Ex(i,j,k)-fholder_2.Ex(i,j,k);
        fholder_out.Ey(i,j,k)=fholder_1.Ey(i,j,k)-fholder_2.Ey(i,j,k);
        fholder_out.Ez(i,j,k)=fholder_1.Ez(i,j,k)-fholder_2.Ez(i,j,k);
        
        fholder_out.Hx(i,j,k)=fholder_1.Hx(i,j,k)-fholder_2.Hx(i,j,k);
        fholder_out.Hy(i,j,k)=fholder_1.Hy(i,j,k)-fholder_2.Hy(i,j,k);
        fholder_out.Hz(i,j,k)=fholder_1.Hz(i,j,k)-fholder_2.Hz(i,j,k);
    }
    
    fholder_out.save(fname_out);
    
    return 1;
}

//##############################
//       Fieldmap Treat
//##############################

Fmap_treat_mode::Fmap_treat_mode()
    :baseline(1.0), fname(""),
     scale_x(1), scale_y(1), scale_z(1)
{
}

int fmap_treat_mode_set_baseline(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
    (*pp_fm)->baseline=lua_tonumber(L,2);
    
    return 0;
}

int fmap_treat_mode_set_file(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
    std::string fname=lua_tostring(L,2);
    
    (*pp_fm)->fname=fname;
    
    return 0;
}

int fmap_treat_mode_set_scale(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
         if(lua_gettop(L)==2)
    {
        double tmp=lua_tointeger(L,2);
        
        (*pp_fm)->scale_x=tmp;
        (*pp_fm)->scale_y=tmp;
        (*pp_fm)->scale_z=tmp;
    }
    else if(lua_gettop(L)==4)
    {
        (*pp_fm)->scale_x=lua_tointeger(L,2);
        (*pp_fm)->scale_y=lua_tointeger(L,3);
        (*pp_fm)->scale_z=lua_tointeger(L,4);
    }
    
    return 0;
}
