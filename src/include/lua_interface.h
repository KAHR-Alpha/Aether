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

#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include <vector>

#include <enum_constants.h>
#include <mathUT.h>
#include <lua_base.h>
#include <string_tools.h>

//###################################
//       Dielec planar waveguide
//###################################

class Dielec_pwgd_mode: public base_mode
{
    public:
        double n1,n2,n3;
        double lambda,h;
        
        Dielec_pwgd_mode();
        
        void process();
};

int dielec_pwgd_mode_set_guide_index(lua_State *L);
int dielec_pwgd_mode_set_lambda(lua_State *L);
int dielec_pwgd_mode_set_sub_index(lua_State *L);
int dielec_pwgd_mode_set_sup_index(lua_State *L);
int dielec_pwgd_mode_set_thickness(lua_State *L);

//####################
//    Index fit
//####################

class Index_fit_mode: public base_mode
{
    public:
        std::string fname;
        std::string output;
        
        int Ndrude,Nlorentz,Ncp;
        
        Index_fit_mode();
        
        void process();
};

int index_fit_mode_set_file(lua_State *L);
int index_fit_mode_set_name(lua_State *L);
int index_fit_mode_set_Ndrude(lua_State *L);
int index_fit_mode_set_Nlorentz(lua_State *L);
int index_fit_mode_set_Ncp(lua_State *L);

//###############
//   Materials
//###############

int lua_create_material(lua_State *L);
int lua_material_set_index(lua_State *L);
int lua_material_set_script(lua_State *L);
int lua_material_set_name(lua_State *L);

//###############
//     Mie
//###############

class Mie_mode: public base_mode
{
    public:
        double radius,env_index;
        std::string part_material;
        
        std::vector<int> extract_type,extract_Nl;
        std::vector<double> extract_lmin,extract_lmax;
        std::vector<std::string> extract_fname;
        void process();
};

template<int type>
int mie_mode_get_cq(lua_State *L)
{
    Mie_mode **pp_mie=reinterpret_cast<Mie_mode**>(lua_touserdata(L,1));
    
    std::string fname=lua_tostring(L,2);
    double lmin=lua_tonumber(L,3);
    double lmax=lua_tonumber(L,4);
    int Nl=lua_tointeger(L,5);
    
    (*pp_mie)->extract_type.push_back(type);
    (*pp_mie)->extract_fname.push_back(fname);
    (*pp_mie)->extract_Nl.push_back(Nl);
    (*pp_mie)->extract_lmin.push_back(lmin);
    (*pp_mie)->extract_lmax.push_back(lmax);
    
    std::cout<<"Extracting the ";
         if(type==MIE_CABS) std::cout<<"absorption cross-section";
    else if(type==MIE_CEXT) std::cout<<"extinction cross-section";
    else if(type==MIE_CSCATT) std::cout<<"scattering cross-section";
    else if(type==MIE_FULL_CQ) std::cout<<"cross-sections and efficiencies";
    else if(type==MIE_QABS) std::cout<<"absorption efficiency";
    else if(type==MIE_QEXT) std::cout<<"extinction efficiency";
    else if(type==MIE_QSCATT) std::cout<<"scattering efficiency";
    std::cout<<" from "<<add_unit_u(lmin)<<" to "<<add_unit_u(lmax)<<" for "<<Nl<<" points, to "<<fname<<std::endl;
    
    return 1;
}

int mie_mode_set_env_index(lua_State *L);
int mie_mode_set_material(lua_State *L);
int mie_mode_set_radius(lua_State *L);

//###########
//   Flush
//###########

class Flush_mode: public base_mode
{
    public:
        bool interruption_type() { return true; }
        void process()
        {
        }
};

//####################
//       Pause
//####################

class Pause_mode: public base_mode
{
    public:
        bool interruption_type() { return true; }
        void process()
        {
            std::cout<<"Pausing program"<<std::endl;
            std::cout<<"Press Enter to resume..."<<std::endl;
            std::cin.get();
        }
};

//####################
//       Quit
//####################

class Quit_mode: public base_mode
{
    public:
        bool interruption_type() { return true; }
        void process()
        {
            std::cout<<"Ending program"<<std::endl;
            std::exit(0);
        }
};

//####################
//      Sleep
//####################

class Sleep_mode: public base_mode
{
    public:
        void process()
        {
            std::cout<<"Making the computer sleep"<<std::endl;
            #ifdef WIN7
                std::system("shutdown /h");
            #endif
        }
};

#ifdef PRIV_MODE
class Testlab_mode: public base_mode
{
    public:
        void process();
};
#endif

//####################
//####################

#include <thread_utils.h>

class mode_register: public ThreadsPool
{
    public:
        std::vector<base_mode*> mdv;
        
        void reg(base_mode *mode)
        {
            if(Nthr>1)
            {
                if(mode->interruption_type())
                {
                    if(mdv.size()>0) run_m(&base_mode::process,mdv.back());
                    
                    join_threads();
                    mode->process();
                }
                else
                {
                    if(mdv.size()>0) run_m(&base_mode::process,mdv.back());
                }
                
                mdv.push_back(mode);
            }
            else
            {
                if(mdv.size()>0) mdv.back()->process();
                
                mdv.push_back(mode);
            }
        }
        
        void process()
        {
            if(Nthr>1)
            {
                if(mdv.size()>0) run_m(&base_mode::process,mdv.back());
                
                join_threads();
            }
            else if(mdv.size()>0) mdv.back()->process();
        }
};

#endif // LUA_INTERFACE_H
