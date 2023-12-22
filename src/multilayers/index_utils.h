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

#ifndef INDEX_UTILS_H
#define INDEX_UTILS_H

#include <bitmap3.h>
#include <mathUT.h>
#include <phys_constants.h>
#include <iostream>
#include <fstream>
#include <vector>


extern const Imdouble Im;

void normD_index_calc();
void normC_index_calc(int Nref,double n1,double n3,double h,std::string polar_mode,std::string prefix);
void normC_index_calc_var(int Nref,double n1,double n3,double h,std::string polar_mode,std::string prefix);

void aniso_diag_slab_analyt(double lambda,double h,double ang,double n1,double n3,Grid1<Imdouble> &eps_r,Grid1<Imdouble> &mu_r,
                       Imdouble &r_TE,Imdouble &r_TM,Imdouble &t_TE,Imdouble &t_TM);

void index_fit(std::string data_fname,int Ndr,int Nlo,int Ncp);

void mode_ani_diag_slab(int Ncmd,Grid1<int> &cmd_arr,Grid1<std::stringstream> &coms);

class B_strat;

class iso_index
{
    public:
        double eps_r,eps_i;
        double mu_r,mu_i;
        double R;
        
        void eval(double lambda,
                  double h,
                  double hd,
                  double r,
                  double rD,
                  double t,
                  double tD,
                  B_strat &strat1,
                  B_strat &strat2);
        void evalC(double lambda,
                      double h,
                      Imdouble r,
                      Imdouble t,
                      B_strat &strat1);
        void mutate(iso_index const&);
        void randgen();
        void show();
        
        void operator = (double);
        void operator = (iso_index const&);
};

class Index
{
    public:
        Imdouble n2;
        double e_r,e_i,mu_r,mu_i;
        Imdouble mu,e;
        double R;
        
        static double n1,n3,h,lambda,ref,trans,ref_p,trans_p;
        static double range;
        static Imdouble rh;
        
        void calcfit()
        {
            Imdouble r12=(n1*mu-n2)/(n1*mu+n2);
            Imdouble r23b=(n2-n3*mu)/(n2+n3*mu)*std::exp(2.0*n2*rh);
            Imdouble t12=2.0*n1*mu/(n1*mu+n2);
            Imdouble t23=2.0*n2/(n2+n3*mu);
            
            Imdouble denom=1.0+r12*r23b;
            
            Imdouble r=(r12+r23b)/denom;
            Imdouble t=t12*t23*std::exp(n2*rh)/denom;
            double a=ref-std::abs(r);
            double b=trans-std::abs(t);
            double c=abs(ref_p-std::arg(r));
            if(c>Pi) c=2.0*Pi-c;
            double d=abs(trans_p-std::arg(t));
            if(d>Pi) d=2.0*Pi-d;
            
            //c=d=0;
            
            R=abs(a)+abs(b)+abs(c)+abs(d);
            if(std::imag(n2)<0)
            {
                R+=1000;
            }
            //if(std::real(n2)<0) R=1000;
        }
        
        void mutate(Index const &indi)
        {
            double c1,c2,c3,c4;
            
            c1=c2=c3=c4=0;
            c1=randp(-1,1);
            c2=randp(-1,1);
            c3=randp(-1,1);
            c4=randp(-1,1);
            
            int a=std::rand()%4;
            
            if(a==0)
            {
                /*e_r=indi.e_r+c1;
                e_i=indi.e_i+c2;*/
                n2=indi.n2+c1+c2*Im;
                mu_r=indi.mu_r+c3;
                mu_i=indi.mu_i+c4;
            }
            if(a==1)
            {
                /*e_r=indi.e_r+0.1*c1;
                e_i=indi.e_i+0.1*c2;*/
                n2=indi.n2+0.1*(c1+c2*Im);
                mu_r=indi.mu_r+0.1*c3;
                mu_i=indi.mu_i+0.1*c4;
            }
            if(a==2)
            {
                /*e_r=(1.0+c1/10.0)*indi.e_r;
                e_i=(1.0+c2/10.0)*indi.e_i;*/
                n2=(1.0+c1/10.0)*std::real(indi.n2)+(1.0+c2/10.0)*std::imag(indi.n2)*Im;
                mu_r=(1.0+c3/10.0)*indi.mu_r;
                mu_i=(1.0+c4/10.0)*indi.mu_i;
            }
            if(a==3)
            {
                /*e_r=(1.0+c1/5.0)*indi.e_r;
                e_i=(1.0+c2/5.0)*indi.e_i;*/
                n2=(1.0+c1/5.0)*std::real(indi.n2)+(1.0+c2/5.0)*std::imag(indi.n2)*Im;
                mu_r=(1.0+c3/5.0)*indi.mu_r;
                mu_i=(1.0+c4/5.0)*indi.mu_i;
            }
            
            //mu_r=1;
            //mu_i=0;
            
            /*e=e_r+e_i*Im;
            mu=mu_r+mu_i*Im;
            n2=std::sqrt(e*mu);*/
            
            mu=mu_r+mu_i*Im;
            e=n2*n2/mu;
            e_r=std::real(e);
            e_i=std::imag(e);
        }
        
        void randgen()
        {
            /*e_r=randp(-10.0,10.0);
            e_i=randp(-10.0,10.0);
            mu_r=randp(-10.0,10.0);
            mu_i=randp(-10.0,10.0);
            mu_r=1;
            mu_i=0;
            
            e=e_r+e_i*Im;
            mu=mu_r+mu_i*Im;
            n2=std::sqrt(e*mu);*/
            
            n2=randp(-10.0,10.0)+randp(-10.0,10.0)*Im;
            mu_r=randp(-10.0,10.0);
            mu_i=randp(-10.0,10.0);
            mu_r=1;
            mu_i=0;
            
            mu=mu_r+mu_i*Im;
            e=n2*n2/mu;
            e_r=std::real(e);
            e_i=std::imag(e);
        }
        
        static void setparams(double n1i,double n3i,double hi,double lambdai,double refi,double transi,double ref_pi,double trans_pi)
        {
            n1=n1i;
            n3=n3i;
            h=hi;
            lambda=lambdai;
            ref=refi;
            trans=transi;
            ref_p=ref_pi;
            trans_p=trans_pi;
            
            rh=2.0*Pi/lambda*h*Im;
        }
        
        void show()
        {
            std::cout<<n2<<" "<<R<<std::endl;
        }
};

#endif // INDEX_UTILS_H
