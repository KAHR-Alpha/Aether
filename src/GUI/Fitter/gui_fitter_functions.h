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

#ifndef GUI_FITTER_FUNCTIONS_H_INCLUDED
#define GUI_FITTER_FUNCTIONS_H_INCLUDED

#include <gui_fitter.h>

enum
{
    F_CONST,
    F_EXP,
    F_GAUSSIAN,
    F_INV_POW,
    F_LOGISTIC,
    F_LORENTZIAN,
    F_LORENTZIAN_GEN,
    F_PLANCK_FREQ,
    F_PLANCK_WVL,
    F_SINE
};

void f_const(std::vector<double> const &x,
             std::vector<double> const &params,
             std::vector<double> &y);
void f_const_h_to_p(std::vector<FitterHandle> const &handles,
                    std::vector<double> &params);
void f_const_p_to_h(std::vector<double> const &params,
                    std::vector<FitterHandle> &handles);
void f_const_base_h(double x_min,double x_max,double y_min,double y_max,
                    std::vector<FitterHandle> &handles);

void f_exp(std::vector<double> const &x,
           std::vector<double> const &params,
           std::vector<double> &y);
void f_exp_h_to_p(std::vector<FitterHandle> const &handles,
                  std::vector<double> &params);
void f_exp_p_to_h(std::vector<double> const &params,
                  std::vector<FitterHandle> &handles);
void f_exp_base_h(double x_min,double x_max,double y_min,double y_max,
                  std::vector<FitterHandle> &handles);

void f_gaussian(std::vector<double> const &x,
                std::vector<double> const &params,
                std::vector<double> &y);
void f_gaussian_h_to_p(std::vector<FitterHandle> const &handles,
                       std::vector<double> &params);
void f_gaussian_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles);
void f_gaussian_base_h(double x_min,double x_max,double y_min,double y_max,
                       std::vector<FitterHandle> &handles);

void f_inv_pow(std::vector<double> const &x,
               std::vector<double> const &params,
               std::vector<double> &y);
void f_inv_pow_h_to_p(std::vector<FitterHandle> const &handles,
                      std::vector<double> &params);
void f_inv_pow_p_to_h(std::vector<double> const &params,
                      std::vector<FitterHandle> &handles);
void f_inv_pow_base_h(double x_min,double x_max,double y_min,double y_max,
                      std::vector<FitterHandle> &handles);

void f_logistic(std::vector<double> const &x,
                std::vector<double> const &params,
                std::vector<double> &y);
void f_logistic_h_to_p(std::vector<FitterHandle> const &handles,
                       std::vector<double> &params);
void f_logistic_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles);
void f_logistic_base_h(double x_min,double x_max,double y_min,double y_max,
                       std::vector<FitterHandle> &handles);

void f_lorentz(std::vector<double> const &x,
               std::vector<double> const &params,
               std::vector<double> &y);
void f_lorentz_h_to_p(std::vector<FitterHandle> const &handles,
                      std::vector<double> &params);
void f_lorentz_p_to_h(std::vector<double> const &params,
                      std::vector<FitterHandle> &handles);
void f_lorentz_base_h(double x_min,double x_max,double y_min,double y_max,
                      std::vector<FitterHandle> &handles);

void f_lorentz_gen(std::vector<double> const &x,
                   std::vector<double> const &params,
                   std::vector<double> &y);
void f_lorentz_gen_h_to_p(std::vector<FitterHandle> const &handles,
                          std::vector<double> &params);
void f_lorentz_gen_p_to_h(std::vector<double> const &params,
                          std::vector<FitterHandle> &handles);
void f_lorentz_gen_base_h(double x_min,double x_max,double y_min,double y_max,
                          std::vector<FitterHandle> &handles);

void f_planck_freq(std::vector<double> const &x,
                   std::vector<double> const &params,
                   std::vector<double> &y);
void f_planck_freq_h_to_p(std::vector<FitterHandle> const &handles,
                          std::vector<double> &params);
void f_planck_freq_p_to_h(std::vector<double> const &params,
                          std::vector<FitterHandle> &handles);
void f_planck_freq_base_h(double x_min,double x_max,double y_min,double y_max,
                          std::vector<FitterHandle> &handles);

void f_planck_wvl(std::vector<double> const &x,
                  std::vector<double> const &params,
                  std::vector<double> &y);
void f_planck_wvl_h_to_p(std::vector<FitterHandle> const &handles,
                         std::vector<double> &params);
void f_planck_wvl_p_to_h(std::vector<double> const &params,
                         std::vector<FitterHandle> &handles);
void f_planck_wvl_base_h(double x_min,double x_max,double y_min,double y_max,
                         std::vector<FitterHandle> &handles);

void f_sine(std::vector<double> const &x,
            std::vector<double> const &params,
            std::vector<double> &y);
void f_sine_h_to_p(std::vector<FitterHandle> const &handles,
                   std::vector<double> &params);
void f_sine_p_to_h(std::vector<double> const &params,
                   std::vector<FitterHandle> &handles);
void f_sine_base_h(double x_min,double x_max,double y_min,double y_max,
                   std::vector<FitterHandle> &handles);
                  
#endif // GUI_FITTER_FUNCTIONS_H_INCLUDED
