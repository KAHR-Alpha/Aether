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

#ifndef PLANAR_WGD_H
#define PLANAR_WGD_H

#include <vector>
#include <mathUT.h>
#include <phys_constants.h>

class Slab_guide
{
    public:
        double n1,n2,n3,h;
        
        double lambda;
        int n_TE,n_TM;
        std::vector<double> beta_TE,beta_TM;
        
        Slab_guide()
            :n1(1.0), n2(3.0), n3(2.0), h(300e-9),
             lambda(1e-6), n_TE(0), n_TM(0)
        {}
        
        Slab_guide(double h_,double n1_,double n2_,double n3_)
            :n1(n1_), n2(n2_), n3(n3_), h(h_),
             lambda(1e-6), n_TE(0), n_TM(0)
        {}
        
        double guide_eqn(double k0,double beta,std::string polar_mode);
        Imdouble guide_eqn_z(double k0,Imdouble beta,std::string polar_mode);
        void compute_guiding_char(double lambda);
        #ifdef FD_ARMA_MODES_SOLVER
        void compute_guiding_char_disc(double lambda,double Dz);
        #endif
        void show_guiding_char();
};

double guide_eqn(double k0,double beta,double n1,double n2,double n3,double h,std::string polar_mode);
bool is_guided_simple(double lambda,double &beta,Slab_guide const &gd,std::string polar_mode,int M_num);
double gd_mode_profile_simp(double lambda,double beta,Slab_guide const &gd,double z,std::string polar_mode,int compo=1);
double gd_mode_energy_simp(double lambda,double beta,Slab_guide const &gd,std::string polar_mode);
Imdouble gd_mode_profile(double lambda,double beta,Slab_guide const &gd,double z,std::string polar_mode,double phase,int compo=1);

void modes_dcpl(double lambda,
                Slab_guide &slb,
                Grid1<Imdouble> const &Ey,Grid1<double> const &z_Ey,
                Grid1<Imdouble> const &Hy,Grid1<double> const &z_Hy,
                Grid1<Imdouble> &coeff_TE,
                Grid1<Imdouble> &coeff_TM);

void modes_dcpl(double lambda,
                Slab_guide &slb,
                Grid1<Imdouble> const &Ey,Grid1<double> const &z_Ey,
                Grid1<Imdouble> const &Hy,Grid1<double> const &z_Hy,
                Grid1<Imdouble> &coeff_TE,
                Grid1<Imdouble> &coeff_TM,
                double &resi_TE,
                double &resi_TM);
                     

#endif // PLANAR_WGD_H
