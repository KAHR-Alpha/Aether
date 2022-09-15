/*Copyright 2008-2021 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef LUA_MULTILAYER_H_INCLUDED
#define LUA_MULTILAYER_H_INCLUDED

#include <lua_base.h>
#include <multilayers.h>

//#########################
//       Multilayer
//#########################

class Multilayer_Berreman_mode: public base_mode
{
    public:
        int Nl;
        double lambda_min,lambda_max;
        Angle angle;
        double index_sub,index_sup;
        std::string output;
        std::vector<double> layer_h;
        std::vector<std::string> layer_mat;
        
        Multilayer_Berreman_mode();
        
        void process();
};

int multilayer_berr_mode_add_layer(lua_State *L);
int multilayer_berr_mode_set_angle(lua_State *L);
int multilayer_berr_mode_set_output(lua_State *L);
int multilayer_berr_mode_set_spectrum(lua_State *L);
int multilayer_berr_mode_set_sub_index(lua_State *L);
int multilayer_berr_mode_set_sup_index(lua_State *L);

//#########################
//       Multilayer
//#########################

class Multilayer_mode: public base_mode
{
    public:
        int Nl;
        double lambda_min,lambda_max;
        Angle angle;
        double index_sub,index_sup;
        std::string output;
        std::vector<double> layer_h;
        std::vector<std::string> layer_mat;
        
        Multilayer_mode();
        
        void process();
};

int multilayer_mode_add_layer(lua_State *L);
int multilayer_mode_set_angle(lua_State *L);
int multilayer_mode_set_output(lua_State *L);
int multilayer_mode_set_spectrum(lua_State *L);
int multilayer_mode_set_sub_index(lua_State *L);
int multilayer_mode_set_sup_index(lua_State *L);

//#########################
//       Multilayer TMM
//#########################

class Multilayer_TMM_mode: public base_mode
{
    public:
        enum{MODE_NONE,MODE_ANGLE,MODE_GUIDED};
        
        int mode,Nl;
        double lambda_min,lambda_max;
        double lambda_guess,nr_guess,ni_guess;
        Angle angle;
        std::string mat_sup_str,
                    mat_sub_str,
                    output,
                    polar;
        std::vector<double> layer_h;
        std::vector<std::string> layer_mat;
        
        Multilayer_TMM_mode();
        
        void add_layer(double h,std::string mat);
        void compute_angle(double angle);
        void compute_guided(std::string polar,
                            double lambda_guess,
                            double nr_guess,
                            double ni_guess);
        void set_output(std::string output);
        void set_spectrum(double lambda_min,double lambda_max,int Nl);
        void set_substrate(std::string mat);
        void set_superstrate(std::string mat);
        
        void process();
};

void create_metatables_multilayers(lua_State *L);

#endif // LUA_MULTILAYER_H_INCLUDED
