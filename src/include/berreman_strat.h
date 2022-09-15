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

#ifndef BERREMAN_STRAT_H
#define BERREMAN_STRAT_H

#include <bitmap3.h>
#include <mathUT.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <Eigen/Eigen>
#include <unsupported/Eigen/MatrixFunctions>

class B_strat
{
    public:
        int N_layers;
        Imdouble sup_ind;
        Imdouble sub_ind;
        
        Grid1<double> h_layer;
        Grid1<Imdouble> e11,e22,e33;
        Grid1<Imdouble> mu11,mu22,mu33;
        
        Grid2<Imdouble> F_m;
        Grid2<Imdouble> Ft_m;
        Grid3<Imdouble> P_m;
        
        Eigen::Matrix<Imdouble,4,4> D_mat,F_mat;
        
        Grid1<bool> diagonal;
        
        Grid3<Imdouble> eps;
        Grid3<Imdouble> mu;
        
        B_strat(int Nl,Imdouble sup_ind,Imdouble sub_ind);
        
        void compute(double lambda,Angle ang,Imdouble &r_TE,Imdouble &r_TM,
                                             Imdouble &t_TE,Imdouble &t_TM);
        
        void compute(double lambda,double ang_rad,
                     Imdouble &r_EE,Imdouble &r_EM,Imdouble &t_EE,Imdouble &t_EM,
                     Imdouble &r_ME,Imdouble &r_MM,Imdouble &t_ME,Imdouble &t_MM);
        
        void compute_abs(double lambda,double ang,double &r_TE,double &r_TM,
                                                  double &t_TE,double &t_TM);
        
        void get_D_matrix(int l,Imdouble dze,Eigen::Matrix<Imdouble,4,4> &D_mat);
        void get_D_matrix_simp(int l,Imdouble dze,Eigen::Matrix<Imdouble,4,4> &D_mat);
        
        void set_aniso(int,double,Grid2<Imdouble> const &eps,Grid2<Imdouble> const &mu);
        
        void set_aniso_diag(int,double,Imdouble eps_xx,
                                       Imdouble eps_yy,
                                       Imdouble eps_zz,
                                       Imdouble mu_xx,
                                       Imdouble mu_yy,
                                       Imdouble mu_zz);
        void set_environment(Imdouble sup_ind,Imdouble sub_ind);
        void set_iso(int l,double h,Imdouble eps,Imdouble mu);
};

#endif // BERREMAN_STRAT_H
