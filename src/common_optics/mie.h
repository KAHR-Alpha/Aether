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

#ifndef MIE_H
#define MIE_H

#include <mathUT.h>
#include <lua_interface.h>
#include <material.h>

class Mie
{
    private:
        int NRec;
        double lambda,radius,x_part;
        double index_ext;
        Imdouble index_in,m;
        bool recalc_necessary;
        
        std::vector<Imdouble> an,bn;
        std::vector<double> coeff_pi,coeff_tau;
        
        void compute_ab();
        void compute_pi_tau(double mu);
        void xm_recalc();
        
    public:
        double get_an_abs(int n);
        double get_bn_abs(int n);
        double get_Cabs();
        double get_Cext();
        double get_Cscatt();
        double get_radiation(double theta,double phi);
        double get_Qabs();
        double get_Qext();
        double get_Qscatt();
        
        void set_NRec(int NRec);
        void set_lambda(double lambda);
        void set_radius(double radius);
        void set_index(Imdouble index_in,double index_ext);
        void set_index_ext(double index_ext);
        void set_index_in(Imdouble index_in);
        
        Mie();
        ~Mie();
        
        friend double EField(double x,double y,double z,Mie &mie);
        friend void EField_complex(double x,double y,double z,Mie &mie,Imdouble &X,Imdouble &Y,Imdouble &Z);
};

double EField(double x,double y,double z,Mie &mie);
void EField_complex(double x,double y,double z,Mie &mie,Imdouble &X,Imdouble &Y,Imdouble &Z);

#endif // MIE_H
