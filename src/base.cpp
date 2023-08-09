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

#include <aniso_retriever.h>
#include <berreman_strat.h>
#include <bitmap3.h>
#include <bspline_int.h>
#include <data_hdl.h>
#include <fdtd_core.h>
#include <index_utils.h>
#include <lua_fd.h>
#include <lua_interface.h>
#include <lua_material.h>
#include <lua_multilayers.h>
#include <lua_optim.h>
#include <lua_selene.h>
#include <lua_structure.h>
#include <mathUT.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>

#ifdef QUENCHING_MODULE
    #include <quenching.h>
#endif

extern const Imdouble Im;
extern std::ofstream plog;

using std::cos;
using std::sin;
using std::exp;
using std::pow;
using std::abs;

int set_concurrent_computations(lua_State *L)
{
    lua_getglobal(L,"mode_register");
    mode_register *mrg=reinterpret_cast<mode_register*>(lua_touserdata(L,-1));
    
    int N=lua_tointeger(L,1);
    
    mrg->resize_pool(N);
    
    return 1;
}

int set_rng_seed(lua_State *L)
{
    seedp(lua_tointeger(L,1));
    
    return 0;
}

int mode_choice(lua_State *L)
{
    std::string mode=lua_tostring(L,1);
//    std::cout<<mode<<std::endl;
    
    lua_getglobal(L,"mode_register");
    mode_register *mrg=reinterpret_cast<mode_register*>(lua_touserdata(L,-1));
    
    base_mode *p_mode=nullptr;
    
    if(mode=="fdtd" ||
       mode=="fdtd_lab" ||
       mode=="fdtd_normal" ||
       mode=="fdtd_oblique_ARS" ||
       mode=="fdtd_planar_guided" ||
       mode=="fdtd_planar_guided_2D_ext" ||
       mode=="fdtd_single_particle")
    {
        FDTD_Mode *p_fdtd=lua_allocate_metapointer<FDTD_Mode>(L,"metatable_fdtd");
        
        FDTD_Mode &fdtd=*p_fdtd;
        
        if(mode=="fdtd") fdtd.type=FDTD_Mode::FDTD_CUSTOM;
        else if(mode=="fdtd_lab") fdtd.type=FDTD_Mode::FDTD_LAB;
        else if(mode=="fdtd_normal") fdtd.type=FDTD_Mode::FDTD_NORMAL;
        else if(mode=="fdtd_oblique_ARS") fdtd.type=FDTD_Mode::FDTD_OBLIQUE_ARS;
//        else if(mode=="fdtd_planar_guided") fdtd.type="planar_guided";
//        else if(mode=="fdtd_planar_guided_2D_ext") fdtd.type="planar_guided_2D_ext";
        else if(mode=="fdtd_single_particle") fdtd.type=FDTD_Mode::FDTD_SINGLE_PARTICLE;
        
        p_mode=&fdtd;
    }
    else if(mode=="fdfd")
    {
        FDFD_Mode *p_fdfd=lua_allocate_metapointer<FDFD_Mode>(L,"metatable_fdfd");
        
        if(mode=="fdfd") p_fdfd->type=FDFD_Mode::FDFD;
        
        p_mode=p_fdfd;
    }
    else if(mode=="fd_modes") p_mode=lua_allocate_metapointer<FDMS_Mode>(L,"metatable_fd_modes");
    else if(mode=="dielec_planar_waveguide") p_mode=lua_allocate_metapointer<Dielec_pwgd_mode>(L,"metatable_dielec_planar_waveguide");
    else if(mode=="fieldblock_treat") p_mode=lua_allocate_metapointer<Fblock_treat_mode>(L,"metatable_fieldblock_treat");
    else if(mode=="fieldmap_treat") p_mode=lua_allocate_metapointer<Fmap_treat_mode>(L,"metatable_fieldmap_treat");
    else if(mode=="flush") p_mode=new Flush_mode;
    else if(mode=="index_fit") p_mode=lua_allocate_metapointer<Index_fit_mode>(L,"metatable_index_fit");
    else if(mode=="mie") p_mode=lua_allocate_metapointer<Mie_mode>(L,"metatable_mie");
    else if(mode=="multilayer") p_mode=lua_allocate_metapointer<Multilayer_mode>(L,"metatable_multilayer");
    else if(mode=="multilayer_berreman") p_mode=lua_allocate_metapointer<Multilayer_Berreman_mode>(L,"metatable_multilayer_berreman");
    else if(mode=="multilayer_tmm") p_mode=lua_allocate_metapointer<Multilayer_TMM_mode>(L,"metatable_multilayer_tmm");
    else if(mode=="pause") p_mode=new Pause_mode;
    else if(mode=="quit") p_mode=new Quit_mode;
    else if(mode=="selene") p_mode=lua_allocate_metapointer<Selene_Mode>(L,"metatable_selene");
    else if(mode=="sleep") p_mode=new Sleep_mode;
    #ifdef PRIV_MODE
    else if(mode=="testlab") p_mode=new Testlab_mode;
    #endif
    else
    {
        std::cout<<"Unrecognized mode "<<mode<<std::endl;
        std::cout<<"Press Enter to continue..."<<std::endl;
        std::cin.get();
    }
    
    if(p_mode!=nullptr) mrg->reg(p_mode);
    
    return 1;
}

#ifndef GUI_ON
    int main(int n_args,char **argv)
#else
    void mode_lua()
#endif
{
    PathManager::initialize();
    plog.open(PathManager::to_temporary_path("log.txt"),std::ios::out|std::ios::trunc);
    
    std::string script_fname="script.lua";
    
    #ifdef GUI_ON
    int n_args=0;
    char **argv=nullptr;
    #endif
    
    if(n_args>1)
    {
        std::vector<std::string> args(n_args);
        for(int i=0;i<n_args;i++) args[i]=argv[i];
        
        int curr_arg=1;
        
        while(curr_arg<n_args)
        {
            if(args[curr_arg]=="-f") // Specified file
            {
                if(curr_arg+1<n_args) { curr_arg++; script_fname=args[curr_arg]; }
                else
                {
                    std::cerr << "Missing file argument for '-f'."<<std::endl;
                    std::cerr << "Try '"<<args[0]<<" --help' for more information."<<std::endl;
                    std::exit(EXIT_FAILURE);
                }
            }
            else if(args[curr_arg]=="-h" || args[curr_arg]=="--help") // Help requested
            {
                std::cout << "Usage: " << args[0] << " [-f SCRIPT_NAME]" << std::endl;
                std::cout << "If SCRIPT_NAME is not specified, Aether will attempt to run a file named 'script.lua' in the current working directory."<<std::endl;
                std::exit(EXIT_SUCCESS);
            }
            else // Unrecognized argument
            {
                std::cerr << "Invalid option '"<<args[curr_arg]<<"'."<<std::endl;
                std::cerr << "Try '"<<args[0]<<" --help' for more information."<<std::endl;
                std::exit(EXIT_FAILURE);
            }
            curr_arg++;
        }
    }
    
    std::filesystem::path script_fname_path=script_fname;
    
    if(!std::filesystem::is_regular_file(script_fname_path))
    {
        std::cerr<<script_fname_path.generic_string()<<" is not a file or could not be located at "
                 <<std::filesystem::absolute(script_fname_path).generic_string()<<"\nAborting...\n";
        std::exit(EXIT_FAILURE);
    }
    
    script_fname_path=script_fname_path.parent_path();
    
    mode_register mreg;
    Lua_memory_register lua_mem_reg;
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&mreg));
    lua_setglobal(L,"mode_register");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&lua_mem_reg));
    lua_setglobal(L,"lua_mem_register");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&script_fname_path));
    lua_setglobal(L,"lua_caller_path");
    
    lua_register(L,"MODE",mode_choice);
    lua_register(L,"complex_material",gen_complex_material);
    lua_register(L,"concurrent_computations",set_concurrent_computations);
    lua_register(L,"const_material",gen_const_material);
    lua_register(L,"create_sensor",create_sensor);
    lua_register(L,"create_source",create_source);
    lua_register(L,"empty_structure",LuaUI::gen_empty_structure);
    lua_register(L,"multilayer",LuaUI::gen_multilayer);
    lua_register(L,"rng_seed",set_rng_seed);
    lua_register(L,"simple_substrate",LuaUI::gen_simple_substrate);
    lua_register(L,"substract_fieldblocks",substract_fieldblocks);
    lua_register(L,"slab",LuaUI::gen_slab);
    
    lua_register(L,"nearest_integer",nearest_integer);
    lua_register(L,"trapz",lua_tools::lua_adaptive_trapeze_integral);
        
    //###############
    
    lua_material::Loader mat_loader;
    mat_loader.create_metatable(L);
    
    create_obj_metatable(L,"metatable_fdfd");
    
    metatable_add_func(L,"azimuth",FDFD_mode_set_azimuth);
    metatable_add_func(L,"incidence",FDFD_mode_set_incidence);
    metatable_add_func(L,"material",FD_mode_set_material);
    lua_wrapper<1,FDFD_Mode,int,int,int,int,int,int>::bind(L,"padding",&FDFD_Mode::set_padding);
    lua_wrapper<2,FDFD_Mode,int,double,double,double>::bind(L,"pml_xm",&FDFD_Mode::set_pml_xm);
    lua_wrapper<3,FDFD_Mode,int,double,double,double>::bind(L,"pml_xp",&FDFD_Mode::set_pml_xp);
    lua_wrapper<4,FDFD_Mode,int,double,double,double>::bind(L,"pml_ym",&FDFD_Mode::set_pml_ym);
    lua_wrapper<5,FDFD_Mode,int,double,double,double>::bind(L,"pml_yp",&FDFD_Mode::set_pml_yp);
    lua_wrapper<6,FDFD_Mode,int,double,double,double>::bind(L,"pml_zm",&FDFD_Mode::set_pml_zm);
    lua_wrapper<7,FDFD_Mode,int,double,double,double>::bind(L,"pml_zp",&FDFD_Mode::set_pml_zp);
    lua_wrapper<8,FDFD_Mode,double>::bind(L,"Dx",&FDFD_Mode::set_discretization_x);
    lua_wrapper<9,FDFD_Mode,double>::bind(L,"Dxyz",&FDFD_Mode::set_discretization);
    lua_wrapper<10,FDFD_Mode,double>::bind(L,"Dy",&FDFD_Mode::set_discretization_y);
    lua_wrapper<11,FDFD_Mode,double>::bind(L,"Dz",&FDFD_Mode::set_discretization_z);
    metatable_add_func(L,"output_diffraction",FDFD_mode_output_diffraction);
    metatable_add_func(L,"output_map",FDFD_mode_output_map);
    metatable_add_func(L,"polarization",FD_mode_set_polarization);
    metatable_add_func(L,"prefix",FD_mode_set_prefix);
    metatable_add_func(L,"solver",FDFD_mode_set_solver);
    metatable_add_func(L,"spectrum",FDFD_mode_set_spectrum);
    metatable_add_func(L,"structure",FD_mode_set_structure);
    
    create_obj_metatable(L,"metatable_fd_modes");
    
    metatable_add_func(L,"material",FD_mode_set_material);
    lua_wrapper<1,FDMS_Mode,int,int,int,int,int,int>::bind(L,"padding",&FDMS_Mode::set_padding);
    lua_wrapper<2,FDMS_Mode,int,double,double,double>::bind(L,"pml_xm",&FDMS_Mode::set_pml_xm);
    lua_wrapper<3,FDMS_Mode,int,double,double,double>::bind(L,"pml_xp",&FDMS_Mode::set_pml_xp);
    lua_wrapper<4,FDMS_Mode,int,double,double,double>::bind(L,"pml_ym",&FDMS_Mode::set_pml_ym);
    lua_wrapper<5,FDMS_Mode,int,double,double,double>::bind(L,"pml_yp",&FDMS_Mode::set_pml_yp);
    lua_wrapper<6,FDMS_Mode,int,double,double,double>::bind(L,"pml_zm",&FDMS_Mode::set_pml_zm);
    lua_wrapper<7,FDMS_Mode,int,double,double,double>::bind(L,"pml_zp",&FDMS_Mode::set_pml_zp);
    metatable_add_func(L,"prefix",FD_mode_set_prefix);
    lua_wrapper<8,FDMS_Mode,std::string>::bind(L,"solver",&FDMS_Mode::set_solver);
    lua_wrapper<9,FDMS_Mode,double,double,double>::bind(L,"spectrum",&FDMS_Mode::set_spectrum);
    lua_wrapper<10,FDMS_Mode,double,double,double>::bind(L,"target",&FDMS_Mode::set_target);
    metatable_add_func(L,"structure",FD_mode_set_structure);
    
    FDTD_Mode_create_metatable(L);
    Sensor_generator_create_metatable(L);
    Source_generator_create_metatable(L);
    
    //################
    //   Structures
    //################
    
    lua_register(L,"Structure",LuaUI::allocate_structure);
    LuaUI::create_structure_metatable(L);
    
    //###############
    
    create_obj_metatable(L,"metatable_dielec_planar_waveguide");
    
    metatable_add_func(L,"guide",dielec_pwgd_mode_set_guide_index);
    metatable_add_func(L,"lambda",dielec_pwgd_mode_set_lambda);
    metatable_add_func(L,"substrate",dielec_pwgd_mode_set_sub_index);
    metatable_add_func(L,"superstrate",dielec_pwgd_mode_set_sup_index);
    metatable_add_func(L,"thickness",dielec_pwgd_mode_set_thickness);
    
    create_obj_metatable(L,"metatable_fieldmap_treat");
    
    metatable_add_func(L,"baseline",fmap_treat_mode_set_baseline);
    metatable_add_func(L,"file",fmap_treat_mode_set_file);
    metatable_add_func(L,"scale",fmap_treat_mode_set_scale);
    
    create_obj_metatable(L,"metatable_fieldblock_treat");
    
    metatable_add_func(L,"apply_stencil",fblock_treat_mode_apply_stencil);
    metatable_add_func(L,"baseline",fblock_treat_mode_set_baseline);
    lua_wrapper<0,Fblock_treat_mode,std::string,std::string,int,std::string>
        ::bind(L,"extract_map",&Fblock_treat_mode::add_map_extraction);
    metatable_add_func(L,"file",fblock_treat_mode_set_file);
    
    create_obj_metatable(L,"metatable_index_fit");
    
    metatable_add_func(L,"file",index_fit_mode_set_file);
    metatable_add_func(L,"name",index_fit_mode_set_name);
    metatable_add_func(L,"N_drude",index_fit_mode_set_Ndrude);
    metatable_add_func(L,"N_lorentz",index_fit_mode_set_Nlorentz);
    metatable_add_func(L,"N_cp",index_fit_mode_set_Ncp);
    
    create_obj_metatable(L,"metatable_mie");
    
    metatable_add_func(L,"cabs",mie_mode_get_cq<MIE_CABS>);
    metatable_add_func(L,"cext",mie_mode_get_cq<MIE_CEXT>);
    metatable_add_func(L,"cscatt",mie_mode_get_cq<MIE_CSCATT>);
    metatable_add_func(L,"environment_index",mie_mode_set_env_index);
    metatable_add_func(L,"full_cq",mie_mode_get_cq<MIE_FULL_CQ>);
    metatable_add_func(L,"material",mie_mode_set_material);
    metatable_add_func(L,"radius",mie_mode_set_radius);
    metatable_add_func(L,"qabs",mie_mode_get_cq<MIE_QABS>);
    metatable_add_func(L,"qext",mie_mode_get_cq<MIE_QEXT>);
    metatable_add_func(L,"qscatt",mie_mode_get_cq<MIE_QSCATT>);
    
    create_metatables_multilayers(L);
    
    //############
    //   Selene
    //############
    
    Sel::IRF fresnel_IRF,
             perfect_abs_IRF,
             perfect_antiref_IRF,
             perfect_mirror_IRF;
    
    fresnel_IRF.set_type_fresnel();
    perfect_abs_IRF.set_type(Sel::IRF_PERF_ABS);
    perfect_antiref_IRF.set_type(Sel::IRF_PERF_ANTIREF);
    perfect_mirror_IRF.set_type(Sel::IRF_PERF_MIRROR);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&fresnel_IRF));
    lua_setglobal(L,"SEL_IRF_FRESNEL");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&perfect_abs_IRF));
    lua_setglobal(L,"SEL_IRF_PERFECT_ABSORBER");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&perfect_antiref_IRF));
    lua_setglobal(L,"SEL_IRF_PERFECT_ANTIREFLECTOR");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&perfect_mirror_IRF));
    lua_setglobal(L,"SEL_IRF_PERFECT_MIRROR");
    
    LuaUI::Selene_create_allocation_functions(L);
    LuaUI::Selene_create_base_metatable(L);
    LuaUI::Selene_create_light_metatable(L);
    LuaUI::Selene_create_object_metatable(L);
    LuaUI::selene_create_target_metatable(L);
    
    //#########################
    //   Optimization Engine
    //#########################
    
    LuaUI::create_optimization_metatable(L);
    
    //
    
    if(luaL_loadfile(L,script_fname.c_str()) || docall(L, 0, 0))
    {
        std::cerr << lua_tostring(L, -1) << std::endl;
        #ifndef GUI_ON
        std::exit(EXIT_FAILURE);
        #endif
    }
    
    lua_close(L);
    
    mreg.process();
}
