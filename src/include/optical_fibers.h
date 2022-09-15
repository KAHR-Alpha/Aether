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

#ifndef OPTICAL_FIBERS_H_INCLUDED
#define OPTICAL_FIBERS_H_INCLUDED

#include <vector>

#include <mathUT.h>
#include <phys_constants.h>

class OpticalFiberMode
{
    public:
        int order,sub_order,type;
        double lambda,beta;
        
        OpticalFiberMode(double lambda_,int order_,int sub_order_,int type_,double beta_)
            :order(order_),
             sub_order(sub_order_),
             type(type_),
             lambda(lambda_),
             beta(beta_)
        {
        }
        
        OpticalFiberMode(OpticalFiberMode const &mode)
            :order(mode.order),
             sub_order(mode.sub_order),
             type(mode.type),
             lambda(mode.lambda),
             beta(mode.beta)
        {
        }
        
        void operator = (OpticalFiberMode const &mode)
        {
            order=mode.order;
            sub_order=mode.sub_order;
            type=mode.type;
            lambda=mode.lambda;
            beta=mode.beta;
        }
};

class OpticalFiber
{
    private:
        int Ns;
        double r,lambda,n1,n2;
        double k0,k1,k2;
        double k0s,k1s,k2s;
        
        void compute_ks(double beta,double &kappa,double &sigma);
        void compute_modes_te(std::vector<OpticalFiberMode> &modes);
        void compute_modes_tm(std::vector<OpticalFiberMode> &modes);
        void compute_modes_tem(int first_order,int last_order,std::vector<OpticalFiberMode> &modes);
        
        double te_eqn(double kappa,double sigma);
        double tm_eqn(double kappa,double sigma);
        double tem_eqn(double beta,double kappa,double sigma,int order);
        
    public:
        OpticalFiber();
        OpticalFiber(double r);
        
        void compute_modes(double lambda,double n1,double n2,
                           int first_order,int last_order,double stepping,
                           std::vector<OpticalFiberMode> &modes);
};

std::string optical_fiber_generate_report(std::vector<OpticalFiberMode> &modes);

int optical_fibers_max_TEM_order(std::vector<std::vector<OpticalFiberMode>> &modes);

int optical_fibers_reorders_TE(std::vector<std::vector<OpticalFiberMode>> &modes,
                               std::vector<std::vector<double>> &modes_TE,
                               std::vector<std::vector<double>> &modes_TE_l);

int optical_fibers_reorders_TM(std::vector<std::vector<OpticalFiberMode>> &modes,
                               std::vector<std::vector<double>> &modes_TM,
                               std::vector<std::vector<double>> &modes_TM_l);

void optical_fibers_reorders_TEM(std::vector<std::vector<OpticalFiberMode>> &modes,
                                 std::vector<std::vector<double>> &modes_TM,
                                 std::vector<std::vector<double>> &modes_TM_l,
                                 std::vector<std::vector<int>> &modes_TEM_orders);

#endif // OPTICAL_FIBERS_H_INCLUDED
