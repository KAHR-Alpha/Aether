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

#include <limits>

#include <optical_fibers.h>
#include <math_bessel.h>
#include <string_tools.h>

extern const double Pi;

OpticalFiber::OpticalFiber()
    :Ns(1001),
     r(300e-9),
     lambda(500e-9),
     n1(1.5), n2(1.1)
{
}

OpticalFiber::OpticalFiber(double r_)
    :Ns(1001),
     r(r_),
     lambda(500e-9),
     n1(1.5), n2(1.1)
{
}

void OpticalFiber::compute_ks(double beta,double &kappa,double &sigma)
{
    double beta_s=beta*beta;
    
    if(beta_s>=k1s) kappa=0;
    else kappa=std::sqrt(k1s-beta_s);
    
    if(beta_s<=k2s) sigma=0;
    else sigma=std::sqrt(beta_s-k2s);
}

void OpticalFiber::compute_modes(double lambda_,double n1_,double n2_,
                                 int first_order,int last_order,double stepping,
                                 std::vector<OpticalFiberMode> &modes)
{
    lambda=lambda_;
    n1=n1_;
    n2=n2_;
    Ns=static_cast<int>(std::pow(10.0,stepping))+1;
    
    k0=2.0*Pi/lambda; k0s=k0*k0;
    k1=n1*k0;         k1s=k1*k1;
    k2=n2*k0;         k2s=k2*k2;
    
    if(first_order==0) compute_modes_te(modes);
    if(first_order==0) compute_modes_tm(modes);
    if(last_order>0 && last_order>=first_order) compute_modes_tem(first_order,last_order,modes);
}


void OpticalFiber::compute_modes_te(std::vector<OpticalFiberMode> &modes)
{
    int i;
    double beta_c,beta_m,beta_p;
    double ec,ep,em;
    double kappa_c,kappa_m,kappa_p;
    double sigma_c,sigma_m,sigma_p;
    
    int sub_order=0;
    
    for(i=Ns-1;i>0;i--)
    {
        beta_p=k0*(n2+(n1-n2)*i/(Ns-1.0));
        beta_m=k0*(n2+(n1-n2)*(i-1)/(Ns-1.0));
        
        compute_ks(beta_p,kappa_p,sigma_p);
        compute_ks(beta_m,kappa_m,sigma_m);;
        
        ep=te_eqn(kappa_p,sigma_p);
        em=te_eqn(kappa_m,sigma_m);
        
        if(sgn(ep)!=sgn(em))
        {
            bool dicho=true;
            
            while(dicho)
            {
                beta_c=0.5*(beta_m+beta_p);
                compute_ks(beta_c,kappa_c,sigma_c);
                
                ec=te_eqn(kappa_c,sigma_c);
                
                if(sgn(ec)!=sgn(em))
                {
                    beta_p=beta_c;
                    ep=ec;
                }
                else
                { 
                    beta_m=beta_c;
                    em=ec;
                }
                
                if(std::abs(beta_m-beta_p)/k0<=1e-6)
                {
                    beta_c=0.5*(beta_m+beta_m);
                    dicho=false;
                }
            }
            
            
            if(is_in_b(beta_c,k2,k1))
            {
                modes.push_back(OpticalFiberMode(lambda,0,sub_order,TE,beta_c));
                sub_order++;
            }
        }
    }
}

void OpticalFiber::compute_modes_tm(std::vector<OpticalFiberMode> &modes)
{
    int i;
    double beta_c,beta_m,beta_p;
    double ec,ep,em;
    double kappa_c,kappa_m,kappa_p;
    double sigma_c,sigma_m,sigma_p;
    
    int sub_order=0;
    
    for(i=Ns-1;i>0;i--)
    {
        beta_p=k0*(n2+(n1-n2)*i/(Ns-1.0));
        beta_m=k0*(n2+(n1-n2)*(i-1)/(Ns-1.0));
        
        compute_ks(beta_p,kappa_p,sigma_p);
        compute_ks(beta_m,kappa_m,sigma_m);;
        
        ep=tm_eqn(kappa_p,sigma_p);
        em=tm_eqn(kappa_m,sigma_m);
        
        if(sgn(ep)!=sgn(em))
        {
            bool dicho=true;
            
            while(dicho)
            {
                beta_c=0.5*(beta_m+beta_p);
                compute_ks(beta_c,kappa_c,sigma_c);
                
                ec=tm_eqn(kappa_c,sigma_c);
                
                if(sgn(ec)!=sgn(em))
                {
                    beta_p=beta_c;
                    ep=ec;
                }
                else
                { 
                    beta_m=beta_c;
                    em=ec;
                }
                
                if(std::abs(beta_m-beta_p)/k0<=1e-6)
                {
                    beta_c=0.5*(beta_m+beta_m);
                    dicho=false;
                }
            }
            
            
            if(is_in_b(beta_c,k2,k1))
            {
                modes.push_back(OpticalFiberMode(lambda,0,sub_order,TM,beta_c));
                sub_order++;
            }
        }
    }
}

void OpticalFiber::compute_modes_tem(int first_order,int last_order,std::vector<OpticalFiberMode> &modes)
{
    int i,p;
    double beta_c,beta_m,beta_p;
    double ec,ep,em;
    double kappa_c,kappa_m,kappa_p;
    double sigma_c,sigma_m,sigma_p;
    
    for(p=std::max(1,first_order);p<=last_order;p++)
    {
        int sub_order=0;
        
        for(i=Ns-1;i>0;i--)
        {
            beta_p=k0*(n2+(n1-n2)*i/(Ns-1.0));
            beta_m=k0*(n2+(n1-n2)*(i-1)/(Ns-1.0));
            
            compute_ks(beta_p,kappa_p,sigma_p);
            compute_ks(beta_m,kappa_m,sigma_m);;
            
            ep=tem_eqn(beta_p,kappa_p,sigma_p,p);
            em=tem_eqn(beta_m,kappa_m,sigma_m,p);
            
            if(sgn(ep)!=sgn(em))
            {
                bool dicho=true;
                
                while(dicho)
                {
                    beta_c=0.5*(beta_m+beta_p);
                    compute_ks(beta_c,kappa_c,sigma_c);
                    
                    ec=tem_eqn(beta_c,kappa_c,sigma_c,p);
                    
                    if(sgn(ec)!=sgn(em))
                    {
                        beta_p=beta_c;
                        ep=ec;
                    }
                    else
                    { 
                        beta_m=beta_c;
                        em=ec;
                    }
                    
                    if(std::abs(beta_m-beta_p)/k0<=1e-6)
                    {
                        beta_c=0.5*(beta_m+beta_m);
                        dicho=false;
                    }
                }
                
                if(is_in_b(beta_c,k2,k1))
                {
                    modes.push_back(OpticalFiberMode(lambda,p,sub_order,TEM,beta_c));
                    sub_order++;
                }
            }
        }
    }
}

double OpticalFiber::te_eqn(double kappa,double sigma)
{
    return sigma*bessel_K_IM(0,sigma*r)*bessel_J_IM(1,kappa*r)
          +kappa*bessel_J_IM(0,kappa*r)*bessel_K_IM(1,sigma*r);
}

double OpticalFiber::tm_eqn(double kappa,double sigma)
{
    return sigma*n1*n1*bessel_K_IM(0,sigma*r)*bessel_J_IM(1,kappa*r)
          +kappa*n2*n2*bessel_J_IM(0,kappa*r)*bessel_K_IM(1,sigma*r);
}

double OpticalFiber::tem_eqn(double beta,double kappa,double sigma,int order)
{
    double dJ=0.5*(bessel_J_IM(order-1,kappa*r)-bessel_J_IM(order+1,kappa*r));
    double J=bessel_J_IM(order,kappa*r);
    
    double dK=0.5*(bessel_K_IM(order-1,sigma*r)+bessel_K_IM(order+1,sigma*r));
    double K=bessel_K_IM(order,sigma*r);
    
    double C1=order*beta/(2.0*Pi/lambda)/r*(kappa*kappa+sigma*sigma)*J;
    double C2=sigma*dJ+kappa*J*dK/K;
    double C3=sigma*n1*n1*dJ+kappa*n2*n2*J*dK/K;
    
    return C1*C1-kappa*kappa*sigma*sigma*C2*C3;
}

//double OpticalFiber::tem_eqn(double beta,double kappa,double sigma,int order)
//{
//    double dJ=0.5*(bessel_J_IM(order-1,kappa*r)-bessel_J_IM(order+1,kappa*r));
//    double J=bessel_J_IM(order,kappa*r);
//    
//    double dK=0.5*(bessel_K_IM(order-1,sigma*r)+bessel_K_IM(order+1,sigma*r));
//    double K=bessel_K_IM(order,sigma*r);
//    
//    double C1=order*beta*2.0*Pi/(r*lambda)*(1.0/kappa/kappa+1.0/sigma/sigma);
//    double C2=dJ/(kappa*J)+dK/(sigma*K);
//    double C3=n1*n1*dJ/(kappa*J)+n2*n2*dK/(sigma*K);
//    
//    return C1*C1-C2*C3;
//}

//###############

std::string optical_fiber_generate_report(std::vector<OpticalFiberMode> &modes)
{
    int curr_type=-1;
    unsigned int m,Nm=modes.size();
    
    if(Nm==0) return "";
    
    std::stringstream report;
    
    report<<add_unit_u(modes[0].lambda)<<" :"<<std::endl;
    
    for(m=0;m<Nm;m++)
    {
        if(modes[m].type!=curr_type)
        {
            curr_type=modes[m].type;
            
                 if(curr_type==TE) report<<"   TE"<<std::endl;
            else if(curr_type==TM) report<<"   TM"<<std::endl;
            else if(curr_type==TEM) report<<"   TEM"<<std::endl;
        }
        
        report<<"      "<<modes[m].order<<" "<<modes[m].sub_order<<" "<<modes[m].beta*modes[m].lambda/(2.0*Pi)<<std::endl;
    }
    
    report<<std::endl;
    
    return report.str();
}

int optical_fibers_max_TEM_order(std::vector<std::vector<OpticalFiberMode>> &modes)
{
    int max_order=-1;
    
    unsigned int i,l;
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            if(modes[l][i].type==TEM)
                max_order=std::max(max_order,modes[l][i].order);
        }
    }
    
    return max_order;
}

int optical_fibers_reorders_TE(std::vector<std::vector<OpticalFiberMode>> &modes,
                               std::vector<std::vector<double>> &modes_TE,
                               std::vector<std::vector<double>> &modes_TE_l)
{
    int max_order=-1;
    
    unsigned int i,l;
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            if(modes[l][i].type==TE)
                max_order=std::max(max_order,modes[l][i].sub_order);
        }
    }
    
    modes_TE.clear();
    modes_TE_l.clear();
    
    if(max_order<0) return -1;
    
    modes_TE.resize(max_order+1);
    modes_TE_l.resize(max_order+1);
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            OpticalFiberMode &mode=modes[l][i];
            
            if(mode.type==TE)
            {
                modes_TE[mode.sub_order].push_back(mode.beta*mode.lambda/(2.0*Pi));
                modes_TE_l[mode.sub_order].push_back(mode.lambda);
            }
        }
    }
    
    return max_order;
}

int optical_fibers_reorders_TM(std::vector<std::vector<OpticalFiberMode>> &modes,
                               std::vector<std::vector<double>> &modes_TM,
                               std::vector<std::vector<double>> &modes_TM_l)
{
    int max_order=-1;
    
    unsigned int i,l;
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            if(modes[l][i].type==TM)
                max_order=std::max(max_order,modes[l][i].sub_order);
        }
    }
    
    modes_TM.clear();
    modes_TM_l.clear();
    
    if(max_order<0) return -1;
    
    modes_TM.resize(max_order+1);
    modes_TM_l.resize(max_order+1);
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            OpticalFiberMode &mode=modes[l][i];
            
            if(mode.type==TM)
            {
                modes_TM[mode.sub_order].push_back(mode.beta*mode.lambda/(2.0*Pi));
                modes_TM_l[mode.sub_order].push_back(mode.lambda);
            }
        }
    }
    
    return max_order;
}

void optical_fibers_reorders_TEM(std::vector<std::vector<OpticalFiberMode>> &modes,
                                 std::vector<std::vector<double>> &modes_TEM,
                                 std::vector<std::vector<double>> &modes_TEM_l,
                                 std::vector<std::vector<int>> &modes_TEM_orders)
{
    modes_TEM.clear();
    modes_TEM_l.clear();
    modes_TEM_orders.clear();
    
    int min_order=std::numeric_limits<int>::max();
    int max_order=-1;
    
    unsigned int i,l,m,n;
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            if(modes[l][i].type==TEM)
            {
                max_order=std::max(max_order,modes[l][i].order);
                min_order=std::min(min_order,modes[l][i].order);
            }
        }
    }
    
    if(min_order>max_order) return;
    
    modes_TEM_orders.resize(max_order-min_order+1);
    
    for(m=0;m<=max_order-min_order;m++)
    {
        modes_TEM_orders[m].resize(2);
        modes_TEM_orders[m][0]=m+min_order;
        modes_TEM_orders[m][1]=-1;
    }
    
    for(l=0;l<modes.size();l++)
    {
        for(i=0;i<modes[l].size();i++)
        {
            if(modes[l][i].type==TEM)
            {
                for(m=0;m<=max_order-min_order;m++)
                {
                    if(modes[l][i].order==modes_TEM_orders[m][0])
                    {
                        modes_TEM_orders[m][1]=std::max(modes_TEM_orders[m][1],modes[l][i].sub_order);
                        break;
                    }
                }
            }
        }
    }
    
    int M_sum=0;
    for(m=0;m<=max_order-min_order;m++) M_sum+=modes_TEM_orders[m][1]+1;
    
    modes_TEM.resize(M_sum);
    modes_TEM_l.resize(M_sum);
    
    unsigned int p=0;
    
    for(m=0;m<=max_order-min_order;m++)
    {
        for(n=0;n<=modes_TEM_orders[m][1];n++)
        {
             for(l=0;l<modes.size();l++)
            {
                for(i=0;i<modes[l].size();i++)
                {
                    OpticalFiberMode &mode=modes[l][i];
                    
                    if(mode.type==TEM &&
                       mode.order==modes_TEM_orders[m][0] &&
                       mode.sub_order==n)
                    {
                        modes_TEM[p].push_back(mode.beta*mode.lambda/(2.0*Pi));
                        modes_TEM_l[p].push_back(mode.lambda);
                    }
                }
            }
            
            p++;
        }
    }
}
