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

#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H


#include <enum_constants.h>
#include <logger.h>
#include <lua_base.h>
#include <mathUT.h>
#include <string_tools.h>

#include <vector>

//###################################
//       Dielec planar waveguide
//###################################

class Dielec_pwgd_mode: public base_mode
{
    public:
        double n1,n2,n3;
        double lambda,h;
        
        Dielec_pwgd_mode();
        
        void process() override;
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
        
        void process() override;
};

int index_fit_mode_set_file(lua_State *L);
int index_fit_mode_set_name(lua_State *L);
int index_fit_mode_set_Ndrude(lua_State *L);
int index_fit_mode_set_Nlorentz(lua_State *L);
int index_fit_mode_set_Ncp(lua_State *L);

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
        void process() override;
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
    
    Plog::print("Extracting the ");
         if(type==MIE_CABS) Plog::print("absorption cross-section");
    else if(type==MIE_CEXT) Plog::print("extinction cross-section");
    else if(type==MIE_CSCATT) Plog::print("scattering cross-section");
    else if(type==MIE_FULL_CQ) Plog::print("cross-sections and efficiencies");
    else if(type==MIE_QABS) Plog::print("absorption efficiency");
    else if(type==MIE_QEXT) Plog::print("extinction efficiency");
    else if(type==MIE_QSCATT) Plog::print("scattering efficiency");
    Plog::print(" from ", add_unit_u(lmin), " to ", add_unit_u(lmax), " for ", Nl, " points, to ", fname, "\n");
    
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
            Plog::print("Pausing program", "\n");
            Plog::print("Press Enter to resume...", "\n");
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
            Plog::print("Ending program", "\n");
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
            Plog::print("Making the computer sleep", "\n");
            #ifdef WIN7
                std::system("shutdown /h");
            #endif
        }
};


#endif // LUA_INTERFACE_H
