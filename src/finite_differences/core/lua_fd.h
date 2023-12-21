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

#ifndef LUA_FD_H_INCLUDED
#define LUA_FD_H_INCLUDED

#include <lua_base.h>
#include <material.h>
#include <structure.h>

#include <atomic>
#include <filesystem>
#include <vector>

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

#endif // LUA_FD_H_INCLUDED
