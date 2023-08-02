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

#ifndef MULTILAYERS_H_INCLUDED
#define MULTILAYERS_H_INCLUDED

#include <material.h>
#include <mathUT.h>

#include <Eigen/Eigen>

class Multilayer
{
    public:
        int N_layers;
        double lambda,k0,kp;
        AngleRad angle;
        double sup_ind;
        Imdouble sub_ind;
        
        std::vector<double> h_layer;
        std::vector<Imdouble> index_layer;
        
        std::vector<Eigen::Matrix<Imdouble,2,2>> F_mat_TE,F_mat_TM,
                                                 G_mat_TE,G_mat_TM,
                                                 W_mat;
        
        Multilayer(int Nl);
        
        void compute(Imdouble &r_TE,Imdouble &r_TM,
                     Imdouble &t_TE,Imdouble &t_TM);
        void compute_abs(double &r_TE,double &r_TM,
                         double &t_TE,double &t_TM);
        void compute_power(double &R_TE,double &T_TE,double &A_TE,
                           double &R_TM,double &T_TM,double &A_TM);
        void recompute_FG();
        void recompute_W();
        void set_angle(AngleRad const &ang);
        void set_environment(double sup_ind,Imdouble sub_ind);
        void set_lambda(double lambda);
        void set_layer(int l,double h,Imdouble n);
};

class MLFieldHolder
{
    public:
        std::vector<std::vector<double>> z_data,E_data,H_data;
        
        std::vector<std::vector<Imdouble>> Ex_data,Ey_data,Ez_data;
        std::vector<std::vector<double>> Ex_data_abs,Ey_data_abs,Ez_data_abs;
        std::vector<std::vector<double>> Ex_data_real,Ey_data_real,Ez_data_real;
        std::vector<std::vector<double>> Ex_data_imag,Ey_data_imag,Ez_data_imag;
        
        std::vector<std::vector<Imdouble>> Hx_data,Hy_data,Hz_data;
        std::vector<std::vector<double>> Hx_data_abs,Hy_data_abs,Hz_data_abs;
        std::vector<std::vector<double>> Hx_data_real,Hy_data_real,Hz_data_real;
        std::vector<std::vector<double>> Hx_data_imag,Hy_data_imag,Hz_data_imag;
        
        void complex_split()
        {
            unsigned int l,k;
            
            for(l=0;l<z_data.size();l++)
            {
                for(k=0;k<z_data[l].size();k++)
                {
                    Ex_data_abs[l][k]=std::abs(Ex_data[l][k]);
                    Ex_data_real[l][k]=std::real(Ex_data[l][k]);
                    Ex_data_imag[l][k]=std::imag(Ex_data[l][k]);
                    
                    Ey_data_abs[l][k]=std::abs(Ey_data[l][k]);
                    Ey_data_real[l][k]=std::real(Ey_data[l][k]);
                    Ey_data_imag[l][k]=std::imag(Ey_data[l][k]);
                    
                    Ez_data_abs[l][k]=std::abs(Ez_data[l][k]);
                    Ez_data_real[l][k]=std::real(Ez_data[l][k]);
                    Ez_data_imag[l][k]=std::imag(Ez_data[l][k]);
                    
                    Hx_data_abs[l][k]=std::abs(Hx_data[l][k]);
                    Hx_data_real[l][k]=std::real(Hx_data[l][k]);
                    Hx_data_imag[l][k]=std::imag(Hx_data[l][k]);
                    
                    Hy_data_abs[l][k]=std::abs(Hy_data[l][k]);
                    Hy_data_real[l][k]=std::real(Hy_data[l][k]);
                    Hy_data_imag[l][k]=std::imag(Hy_data[l][k]);
                    
                    Hz_data_abs[l][k]=std::abs(Hz_data[l][k]);
                    Hz_data_real[l][k]=std::real(Hz_data[l][k]);
                    Hz_data_imag[l][k]=std::imag(Hz_data[l][k]);
                }
            }
        }
        
        double& E(int l,int k) { return E_data[l][k]; }
        Imdouble& Ex(int l,int k) { return Ex_data[l][k]; }
        Imdouble& Ey(int l,int k) { return Ey_data[l][k]; }
        Imdouble& Ez(int l,int k) { return Ez_data[l][k]; }
        
        std::vector<double>& get_E(int l) { return E_data[l]; }
        std::vector<Imdouble>& get_Ex(int l) { return Ex_data[l]; }
        std::vector<Imdouble>& get_Ey(int l) { return Ey_data[l]; }
        std::vector<Imdouble>& get_Ez(int l) { return Ez_data[l]; }
        std::vector<double>& get_Ex_abs(int l) { return Ex_data_abs[l]; }
        std::vector<double>& get_Ey_abs(int l) { return Ey_data_abs[l]; }
        std::vector<double>& get_Ez_abs(int l) { return Ez_data_abs[l]; }
        std::vector<double>& get_Ex_real(int l) { return Ex_data_real[l]; }
        std::vector<double>& get_Ey_real(int l) { return Ey_data_real[l]; }
        std::vector<double>& get_Ez_real(int l) { return Ez_data_real[l]; }
        std::vector<double>& get_Ex_imag(int l) { return Ex_data_imag[l]; }
        std::vector<double>& get_Ey_imag(int l) { return Ey_data_imag[l]; }
        std::vector<double>& get_Ez_imag(int l) { return Ez_data_imag[l]; }
        
        std::vector<double>& get_H(int l) { return H_data[l]; }
        std::vector<Imdouble>& get_Hx(int l) { return Hx_data[l]; }
        std::vector<Imdouble>& get_Hy(int l) { return Hy_data[l]; }
        std::vector<Imdouble>& get_Hz(int l) { return Hz_data[l]; }
        std::vector<double>& get_Hx_abs(int l) { return Hx_data_abs[l]; }
        std::vector<double>& get_Hy_abs(int l) { return Hy_data_abs[l]; }
        std::vector<double>& get_Hz_abs(int l) { return Hz_data_abs[l]; }
        std::vector<double>& get_Hx_real(int l) { return Hx_data_real[l]; }
        std::vector<double>& get_Hy_real(int l) { return Hy_data_real[l]; }
        std::vector<double>& get_Hz_real(int l) { return Hz_data_real[l]; }
        std::vector<double>& get_Hx_imag(int l) { return Hx_data_imag[l]; }
        std::vector<double>& get_Hy_imag(int l) { return Hy_data_imag[l]; }
        std::vector<double>& get_Hz_imag(int l) { return Hz_data_imag[l]; }
        
        std::vector<double>& get_z(int l) { return z_data[l]; }
        
        double& H(int l,int k) { return H_data[l][k]; }
        Imdouble& Hx(int l,int k) { return Hx_data[l][k]; }
        Imdouble& Hy(int l,int k) { return Hy_data[l][k]; }
        Imdouble& Hz(int l,int k) { return Hz_data[l][k]; }
        
        void invert_z()
        {
            unsigned int l,k;
                        
            for(l=0;l<z_data.size();l++)
            {
                for(k=0;k<z_data[l].size();k++)
                {
                    z_data[l][k]=-z_data[l][k];
                    Ex_data[l][k]=-Ex_data[l][k];
                    Hx_data[l][k]=-Hx_data[l][k];
                }
            }
        }
        
        int N_layers() { return z_data.size(); }
        int Nz(int l) { return z_data[l].size(); }
        
        void normalize()
        {
            unsigned int l,k;
            
            double E_max=0;
            
            for(l=0;l<z_data.size();l++)
            {
                for(k=0;k<z_data[l].size();k++)
                    E_max=std::max(E_max,E_data[l][k]);
            }
            
            for(l=0;l<z_data.size();l++)
            {
                for(k=0;k<z_data[l].size();k++)
                {
                    E_data[l][k]/=E_max;
                    Ex_data[l][k]/=E_max;
                    Ey_data[l][k]/=E_max;
                    Ez_data[l][k]/=E_max;
                    
                    H_data[l][k]/=E_max;
                    Hx_data[l][k]/=E_max;
                    Hy_data[l][k]/=E_max;
                    Hz_data[l][k]/=E_max;
                }
            }
        }
        
        void set_E(int l,int k,double E_) { E_data[l][k]=E_; }
        void set_Ex(int l,int k,Imdouble Ex_) { Ex_data[l][k]=Ex_; }
        void set_Ey(int l,int k,Imdouble Ey_) { Ey_data[l][k]=Ey_; }
        void set_Ez(int l,int k,Imdouble Ez_) { Ez_data[l][k]=Ez_; }
        
        void set_H(int l,int k,double H_) { H_data[l][k]=H_; }
        void set_Hx(int l,int k,Imdouble Hx_) { Hx_data[l][k]=Hx_; }
        void set_Hy(int l,int k,Imdouble Hy_) { Hy_data[l][k]=Hy_; }
        void set_Hz(int l,int k,Imdouble Hz_) { Hz_data[l][k]=Hz_; }
        
        void set_N_layers(int N)
        {
            z_data.resize(N);
            E_data.resize(N);
            
            Ex_data.resize(N);
            Ey_data.resize(N);
            Ez_data.resize(N);
            
            Ex_data_abs.resize(N);
            Ey_data_abs.resize(N);
            Ez_data_abs.resize(N);
            
            Ex_data_real.resize(N);
            Ey_data_real.resize(N);
            Ez_data_real.resize(N);
            
            Ex_data_imag.resize(N);
            Ey_data_imag.resize(N);
            Ez_data_imag.resize(N);
            
            H_data.resize(N);
            
            Hx_data.resize(N);
            Hy_data.resize(N);
            Hz_data.resize(N);
            
            Hx_data_abs.resize(N);
            Hy_data_abs.resize(N);
            Hz_data_abs.resize(N);
            
            Hx_data_real.resize(N);
            Hy_data_real.resize(N);
            Hz_data_real.resize(N);
            
            Hx_data_imag.resize(N);
            Hy_data_imag.resize(N);
            Hz_data_imag.resize(N);
        }
        
        void set_Nz(int l,int N)
        {
            z_data[l].resize(N);
            
            E_data[l].resize(N);
            
            Ex_data[l].resize(N);
            Ey_data[l].resize(N);
            Ez_data[l].resize(N);
            
            Ex_data_abs[l].resize(N);
            Ey_data_abs[l].resize(N);
            Ez_data_abs[l].resize(N);
            
            Ex_data_real[l].resize(N);
            Ey_data_real[l].resize(N);
            Ez_data_real[l].resize(N);
            
            Ex_data_imag[l].resize(N);
            Ey_data_imag[l].resize(N);
            Ez_data_imag[l].resize(N);
            
            H_data[l].resize(N);
            
            Hx_data[l].resize(N);
            Hy_data[l].resize(N);
            Hz_data[l].resize(N);
            
            Hx_data_abs[l].resize(N);
            Hy_data_abs[l].resize(N);
            Hz_data_abs[l].resize(N);
            
            Hx_data_real[l].resize(N);
            Hy_data_real[l].resize(N);
            Hz_data_real[l].resize(N);
            
            Hx_data_imag[l].resize(N);
            Hy_data_imag[l].resize(N);
            Hz_data_imag[l].resize(N);
            
            for(int k=0;k<N;k++)
            {
                z_data[l][k]=0;
                
                E_data[l][k]=0;
                
                Ex_data[l][k]=0;
                Ey_data[l][k]=0;
                Ez_data[l][k]=0;
                
                H_data[l][k]=0;
                
                Hx_data[l][k]=0;
                Hy_data[l][k]=0;
                Hz_data[l][k]=0;
            }
        }
        
        void set_z(int l,int k,double z_) { z_data[l][k]=z_; }
        double& z(int l,int k) { return z_data[l][k]; }
};

class Multilayer_TMM
{
    public:
        int N_layers;
        double lambda,k0;
        AngleRad angle;
        Imdouble kp;
        Imdouble sup_ind;
        Imdouble sub_ind;
        Material sup_mat,sub_mat;
        
        std::vector<double> h_layer;
        std::vector<Imdouble> index_layer;
        std::vector<Material> material_layer;
        
        std::vector<Eigen::Matrix<Imdouble,2,2>> L_mat_TE,L_mat_TM;
        
        Multilayer_TMM();
        Multilayer_TMM(int Nl);
        Multilayer_TMM(Multilayer_TMM const &ml);
        
        void compute(Imdouble &r_TE,Imdouble &r_TM,
                     Imdouble &t_TE,Imdouble &t_TM);
        void compute_abs(double &r_TE,double &r_TM,
                         double &t_TE,double &t_TM);
        Imdouble compute_chara_TE(Imdouble const &n_eff);
        Imdouble compute_chara_TM(Imdouble const &n_eff);
        void compute_mode_TE(Imdouble const &n_eff,MLFieldHolder &holder,bool auto_z=true);
        void compute_mode_TM(Imdouble const &n_eff,MLFieldHolder &holder,bool auto_z=true);
        void compute_power(double &R_TE,double &T_TE,double &A_TE,
                           double &R_TM,double &T_TM,double &A_TM);
        void compute_TE_dispersion(double lambda_target,Imdouble n_eff_target,
                                   std::vector<double> const &lambda_disp,std::vector<Imdouble> &n_eff,
                                   double span_r,double span_i,double limit,int max_fail);
        void compute_TM_dispersion(double lambda_target,Imdouble n_eff_target,
                                   std::vector<double> const &lambda_disp,std::vector<Imdouble> &n_eff,
                                   double span_r,double span_i,double limit,int max_fail);
        Imdouble nearest_zero_chara_TE_MC(Imdouble const &n_eff_guess,
                                          double span_r,double span_i,
                                          double limit,int max_fail);
        Imdouble nearest_zero_chara_TM_MC(Imdouble const &n_eff_guess,
                                          double span_r,double span_i,
                                          double limit,int max_fail);
        void operator = (Multilayer_TMM const &ml);
        void recompute_L();
        void set_angle(AngleRad const &ang);
        void set_environment(Imdouble sup_ind,Imdouble sub_ind);
        void set_environment(Material const &sup_mat,Material const &sub_mat);
        void set_lambda(double lambda);
        void set_lambda_full(double lambda);
        void set_layer(int l,double h,Imdouble n);
        void set_layer(int l,double h,Material const &mat);
        void set_N_layers(int N_layers);
};

class Multilayer_TMM_UD
{
    public:
        int N_layers;
        double lambda,k0;
        AngleRad angle;
        Imdouble kp;
        Imdouble sup_ind;
        Imdouble sub_ind;
        
        std::vector<double> h_layer;
        std::vector<Imdouble> index_layer;
        
        std::vector<Eigen::Matrix<Imdouble,2,2>> L_mat_TE,L_mat_TM;
        
        Multilayer_TMM_UD();
        Multilayer_TMM_UD(int Nl);
        Multilayer_TMM_UD(Multilayer_TMM_UD const &ml);
        
        void compute(Imdouble &r_TE,Imdouble &r_TM,
                     Imdouble &t_TE,Imdouble &t_TM);
        void compute_abs(double &r_TE,double &r_TM,
                         double &t_TE,double &t_TM);
        void compute_power(double &R_TE,double &T_TE,double &A_TE,
                           double &R_TM,double &T_TM,double &A_TM);
        void operator = (Multilayer_TMM_UD const &ml);
        void recompute_L();
        void set_angle(AngleRad const &ang);
        void set_environment(Imdouble sup_ind,Imdouble sub_ind);
        void set_lambda(double lambda);
        void set_layer(int l,double h,Imdouble n);
        void set_N_layers(int N_layers);
};

#endif // MULTILAYERS_H_INCLUDED
