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

#include <lua_fdfd.h>

//####################
//     FDFD Mode
//####################

FDFD_Mode::FDFD_Mode()
    :FD_Mode(),
     N_theta(1), theta_min(0), theta_max(0),
     N_phi(1), phi_min(0), phi_max(0),
     Nl(49), lambda_min(370e-9), lambda_max(850e-9),
     solver("LU"),
     output_diffraction(false),
     output_map(false)
{
}

void FDFD_Mode::process()
{
    if(type==FDFD)
    {
        mode_fdfd(*this);
    }
}

void FDFD_Mode::set_azimuth(AngleRad phi_min_,AngleRad phi_max_,int N_phi_)
{
    phi_min=phi_min_;
    phi_max=phi_max_;
    N_phi=N_phi_;
}

void FDFD_Mode::set_incidence(AngleRad theta_min_,AngleRad theta_max_,int N_theta_)
{
    theta_min=theta_min_;
    theta_max=theta_max_;
    N_theta=N_theta_;
}

void FDFD_Mode::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

int FDFD_mode_set_azimuth(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    AngleRad phi_min=Degree(lua_tonumber(L,2));
    AngleRad phi_max=Degree(lua_tonumber(L,3));
    int N_phi=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_azimuth(phi_min,phi_max,N_phi);
    
    return 1;
}

int FDFD_mode_set_incidence(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    AngleRad theta_min=Degree(lua_tonumber(L,2));
    AngleRad theta_max=Degree(lua_tonumber(L,3));
    int N_theta=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_incidence(theta_min,theta_max,N_theta);
    
    return 1;
}

int FDFD_mode_output_diffraction(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::cout<<"Enabling diffracted orders computation"<<std::endl;
    
    (*pp_fdfd)->output_diffraction=true;
    
    return 1;
}

int FDFD_mode_output_map(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::cout<<"Enabling fieldmap"<<std::endl;
    
    (*pp_fdfd)->output_map=true;
    
    return 1;
}

int FDFD_mode_set_solver(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::string solver=lua_tostring(L,2);
    
    (*pp_fdfd)->solver=solver;
    
    return 1;
}

int FDFD_mode_set_spectrum(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    double lambda_min=lua_tonumber(L,2);
    double lambda_max=lua_tonumber(L,3);
    int Nl=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_spectrum(lambda_min,lambda_max,Nl);
    
    return 1;
}

//###################
//     FDMS_Mode
//###################

FDMS_Mode::FDMS_Mode()
    :FD_Mode(),
     Nl(481),
     lambda_min(370e-9), lambda_max(850e-9),
     lambda_target(500e-9), nr_target(1.0), ni_target(0.0),
     solver("default")
{
}

void FDMS_Mode::set_solver(std::string solver_)
{
    if(solver_=="bicgstab") solver="bicgstab";
}

void FDMS_Mode::set_spectrum(double lambda_min_,double lambda_max_,double Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

void FDMS_Mode::set_target(double lambda_target_,double nr_target_,double ni_target_)
{
    lambda_target=lambda_target_;
    nr_target=nr_target_;
    ni_target=ni_target_;
}
