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

#ifndef LUA_FDFD_H_INCLUDED
#define LUA_FDFD_H_INCLUDED

#include <lua_fd.h>

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
        
        void process() override;
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
        
        void process() override;
};

void mode_fdfd(FDFD_Mode const &fdtd_mode);

int FDFD_mode_set_azimuth(lua_State *L);
int FDFD_mode_set_incidence(lua_State *L);
int FDFD_mode_output_diffraction(lua_State *L);
int FDFD_mode_output_map(lua_State *L);
int FDFD_mode_set_solver(lua_State *L);
int FDFD_mode_set_spectrum(lua_State *L);

int FDMS_mode_get_mode(lua_State *L);

#endif
