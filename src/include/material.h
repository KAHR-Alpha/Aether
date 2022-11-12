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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <dielec_models.h>
#include <lua_base.h>
#include <math_approx.h>

#include <filesystem>

enum
{
    MAT_EFF_BRUGGEMAN,
    MAT_EFF_MG1,
    MAT_EFF_MG2,
    MAT_EFF_LOYENGA,
    MAT_EFF_SUM,
    MAT_EFF_SUM_INV
};

class Material
{
    public:
        double eps_inf;
        
        double lambda_valid_min,
               lambda_valid_max;
        
        // Common dielectric models
        
        std::vector<DebyeModel> debye;
        std::vector<DrudeModel> drude;
        std::vector<LorentzModel> lorentz;
        std::vector<CritpointModel> critpoint;
        
        // Cauchy
        std::vector<std::vector<double>> cauchy_coeffs;
        
        // Sellmeier
        std::vector<double> sellmeier_B,sellmeier_C;
        
        // For file-based materials
        std::vector<std::vector<double>> spd_lambda,spd_r,spd_i; // Base data
        std::vector<char> spd_type_index;
        
        std::vector<Cspline> er_spline,ei_spline;
        
        // Effective Material
        
        bool is_effective_material;
        int effective_type;
        Material *eff_mat_1,*eff_mat_2;
        double eff_weight;
        
        std::string name,description;
        std::filesystem::path script_path;
        
        Material();
        Material(Material const &mat);
        Material(std::filesystem::path const &script_path);
        ~Material();
        
        void add_spline_data(std::vector<double> const &lambda,
                             std::vector<double> const &data_r,
                             std::vector<double> const &data_i,
                             bool type_index);
        bool fdtd_compatible();
        Imdouble get_eps(double w) const;
        std::string get_description() const;
        std::string get_matlab(std::string const &fname) const;
        Imdouble get_n(double w) const;
        bool is_const() const;
        void load_lua_script(std::filesystem::path const &script_name);
        void operator = (Material const &mat);
        bool operator == (Material const &mat) const;
        void reset();
        void set_const_eps(double eps);
        void set_const_n(double n);
        void set_effective_material(int effective_type,Material const &eff_mat_1,Material const &eff_mat_2);
        void write_lua_script();
};

int gen_absorbing_material(lua_State *L);
int gen_const_material(lua_State *L);
int gen_complex_material(lua_State *L);

Imdouble effmodel_bruggeman(Imdouble eps_1,Imdouble eps_2,
                            double weight_1,double weight_2);
Imdouble effmodel_looyenga(Imdouble eps_1,Imdouble eps_2,
                           double weight_1,double weight_2);
Imdouble effmodel_maxwell_garnett_1(Imdouble eps_1,Imdouble eps_2,
                                    double weight_1,double weight_2);
Imdouble effmodel_maxwell_garnett_2(Imdouble eps_1,Imdouble eps_2,
                                    double weight_1,double weight_2);
Imdouble effmodel_sum(Imdouble eps_1,Imdouble eps_2,
                      double weight_1,double weight_2);
Imdouble effmodel_sum_inv(Imdouble eps_1,Imdouble eps_2,
                          double weight_1,double weight_2);

#endif // MATERIAL_H
