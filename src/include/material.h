/*Copyright 2008-2023 - Loïc Le Cunff

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

enum class EffectiveModel
{
    BRUGGEMAN,
    LOOYENGA,
    MAXWELL_GARNETT,
    SUM,
    SUM_INV
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
        EffectiveModel effective_type;
        
        int maxwell_garnett_host;
        std::vector<Material*> eff_mats;
        std::vector<double> eff_weights;
        std::vector<Imdouble> eff_eps;
        
        std::string name,description; 
        std::filesystem::path script_path;
        
        Material();
        Material(Material const &mat);
        virtual ~Material();
        
        void add_spline_data(std::vector<double> const &lambda,
                             std::vector<double> const &data_r,
                             std::vector<double> const &data_i,
                             bool type_index);
        virtual void allocate_effective_materials(std::size_t Nm);
        bool fdtd_compatible();
        Imdouble get_eps(double w);
        std::string get_matlab(std::string const &fname) const;
        Imdouble get_n(double w);
        bool is_const() const;
        void operator = (Material const &mat);
        bool operator == (Material const &mat) const;
        bool operator != (Material const &mat) const;
        void reset();
        void set_const_eps(double eps);
        void set_const_n(double n);
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
                          
Imdouble effmodel_bruggeman(std::vector<Imdouble> const &eps,
                            std::vector<double> const &weights);
Imdouble effmodel_looyenga(std::vector<Imdouble> const &eps,
                           std::vector<double> const &weights);
Imdouble effmodel_maxwell_garnett(std::vector<Imdouble> const &eps,
                                  std::vector<double> const &weights,int host);
Imdouble effmodel_sum(std::vector<Imdouble> const &eps,
                      std::vector<double> const &weights);
Imdouble effmodel_inv_sum(std::vector<Imdouble> const &eps,
                          std::vector<double> const &weights);

#endif // MATERIAL_H
