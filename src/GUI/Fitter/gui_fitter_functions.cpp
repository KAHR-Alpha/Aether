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

#include <gui_fitter_functions.h>

extern const double Pi;

//#######################
//   Constant Function
//#######################

void f_const(std::vector<double> const &x,
             std::vector<double> const &params,
             std::vector<double> &y)
{
    for(unsigned int i=0;i<y.size();i++)
        y[i]=params[0];
}

void f_const_h_to_p(std::vector<FitterHandle> const &handles,
                    std::vector<double> &params)
{
    params[0]=handles[0].y;
}

void f_const_p_to_h(std::vector<double> const &params,
                    std::vector<FitterHandle> &handles)
{
    handles[0].y=params[0];
}

void f_const_base_h(double x_min,double x_max,double y_min,double y_max,
                    std::vector<FitterHandle> &handles)
{
    handles[0].x=0.5*(x_min+x_max);
    handles[0].y=0.5*(y_min+y_max);
}

//#################
//   Exponential
//#################

void f_exp(std::vector<double> const &x,
           std::vector<double> const &params,
           std::vector<double> &y)
{
    for(unsigned int i=0;i<x.size();i++)
        y[i]=params[0]*std::exp(params[1]*x[i]);
}

void f_exp_h_to_p(std::vector<FitterHandle> const &handles,
                  std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    const double &x2=handles[1].x;
    const double &y2=handles[1].y;
    
    if(y2*y1<=0) params[1]=0;
    else params[1]=1.0/(x2-x1)*std::log(y2/y1);
    
    params[0]=y1*std::exp(-params[1]*x1);
}

void f_exp_p_to_h(std::vector<double> const &params,
                  std::vector<FitterHandle> &handles)
{
    handles[0].y=params[0]*std::exp(params[1]*handles[0].x);
    handles[1].y=params[0]*std::exp(params[1]*handles[1].x);
}

void f_exp_base_h(double x_min,double x_max,double y_min,double y_max,
                  std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    double y_span=y_max-y_min;
    
    handles[0].x=x_min+0.2*x_span;
    handles[0].y=y_min+0.8*y_span;
    
    handles[1].x=x_min+0.8*x_span;
    handles[1].y=y_min+0.2*y_span;
}

//##############
//   Gaussian
//##############

void f_gaussian(std::vector<double> const &x,
                std::vector<double> const &params,
                std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=x[i]-params[2];
        
        y[i]=params[0]*std::exp(-params[1]*xs*xs);
    }
}

void f_gaussian_h_to_p(std::vector<FitterHandle> const &handles,
                       std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    const double &x2=handles[1].x;
    const double &y2=handles[1].y;
    
    params[0]=y1;
    params[2]=x1;
    
    if(y2*y1<=0) params[1]=0;
    else
    {
        double xs=x2-x1;
        params[1]=-1.0/(xs*xs)*std::log(y2/y1);
    }
}

void f_gaussian_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles)
{
    handles[0].x=params[2];
    handles[0].y=params[0];
    
    double xs=handles[1].x-params[2];
    handles[1].y=params[0]*std::exp(-params[1]*xs*xs);
}

void f_gaussian_base_h(double x_min,double x_max,double y_min,double y_max,
                  std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    
    handles[0].x=0.5*(x_max+x_min);
    handles[1].x=handles[0].x+0.25*x_span;
    
    if(y_max>0)
    {
        handles[0].y=0.8*y_max;
        handles[1].y=0.4*y_max;
    }
    else
    {
        handles[0].y=0.8*y_min;
        handles[1].y=0.4*y_min;
    }
}

//###################
//   Inverse Power
//###################

void f_inv_pow(std::vector<double> const &x,
               std::vector<double> const &params,
               std::vector<double> &y)
{
    for(unsigned int i=0;i<x.size();i++)
        y[i]=params[0]/std::pow(x[i],params[1]);
}

void f_inv_pow_h_to_p(std::vector<FitterHandle> const &handles,
                      std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    const double &x2=handles[1].x;
    const double &y2=handles[1].y;
    
    if(y2*y1<=0 || x2*x1<=0) params[1]=0;
    else params[1]=std::log(y2/y1)/std::log(x1/x2);
    
    params[0]=y1*std::pow(x1,params[1]);
}

void f_inv_pow_p_to_h(std::vector<double> const &params,
                      std::vector<FitterHandle> &handles)
{
    handles[0].y=params[0]/std::pow(handles[0].x,params[1]);
    handles[1].y=params[0]/std::pow(handles[1].x,params[1]);
}

void f_inv_pow_base_h(double x_min,double x_max,double y_min,double y_max,
                      std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    double y_span=y_max-y_min;
    
    if(x_max>0) x_span=std::min(x_max-x_min,x_max);
    if(y_max>0) y_span=std::min(y_max-y_min,y_max);
    
    if(x_max>0)
    {
        if(y_max>0)
        {
            handles[0].x=std::max(0.0,x_min)+0.2*x_span;
            handles[0].y=std::max(0.0,y_min)+0.8*y_span;
            
            handles[1].x=std::max(0.0,x_min)+0.8*x_span;
            handles[1].y=std::max(0.0,y_min)+0.2*y_span;
        }
        else
        {
            handles[0].x=std::max(0.0,x_min)+0.2*x_span;
            handles[0].y=y_min+0.2*y_span;
            
            handles[1].x=std::max(0.0,x_min)+0.8*x_span;
            handles[1].y=y_min+0.8*y_span;
        }
    }
    else
    {
        if(y_max>0)
        {
            handles[0].x=x_min+0.2*x_span;
            handles[0].y=std::max(0.0,y_min)+0.2*y_span;
            
            handles[1].x=x_min+0.8*x_span;
            handles[1].y=std::max(0.0,y_min)+0.8*y_span;
        }
        else
        {
            handles[0].x=x_min+0.2*x_span;
            handles[0].y=y_min+0.8*y_span;
            
            handles[1].x=x_min+0.8*x_span;
            handles[1].y=y_min+0.2*y_span;
        }
    }
}

//#################
//   Lorenztzian
//#################

void f_logistic(std::vector<double> const &x,
                std::vector<double> const &params,
                std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=x[i]-params[2];
        
        y[i]=params[0]/(1.0+std::exp(-params[1]*xs));
    }
}

void f_logistic_h_to_p(std::vector<FitterHandle> const &handles,
                       std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    const double &x2=handles[1].x;
    const double &y2=handles[1].y;
    
    params[0]=2.0*y1;
    params[2]=x1;
    
    if(y1*y2<=0 || std::abs(y2)>std::abs(2.0*y1)) params[1]=0;
    else
    {
        double xs=x2-x1;
        params[1]=-1.0/xs*std::log(2.0*y1/y2-1.0);
    }
}

void f_logistic_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles)
{
    handles[0].x=params[2];
    handles[0].y=params[0]/2.0;
    
    double xs=handles[1].x-params[2];
    handles[1].y=params[0]/(1.0+std::exp(-params[1]*xs));
}

void f_logistic_base_h(double x_min,double x_max,double y_min,double y_max,
                       std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    
    handles[0].x=x_min+0.5*x_span;
    handles[1].x=x_min+0.75*x_span;
    
    if(y_max>0)
    {
        handles[0].y=0.4*y_max;
        handles[1].y=0.6*y_max;
    }
    else
    {
        handles[0].y=0.4*y_min;
        handles[1].y=0.6*y_min;
    }
}

//#################
//   Lorenztzian
//#################

void f_lorentz(std::vector<double> const &x,
               std::vector<double> const &params,
               std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=x[i]-params[2];
        
        y[i]=params[0]/(1+params[1]*xs*xs);
    }
}

void f_lorentz_h_to_p(std::vector<FitterHandle> const &handles,
                      std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    const double &x2=handles[1].x;
    const double &y2=handles[1].y;
    
    params[0]=y1;
    params[2]=x1;
    
    if(y2*y1<=0 || std::abs(y2)>std::abs(y1)) params[1]=0;
    else
    {
        double xs=x2-x1;
        params[1]=(y1/y2-1)/(xs*xs);
    }
}

void f_lorentz_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles)
{
    handles[0].x=params[2];
    handles[0].y=params[0];
    
    double xs=handles[1].x-params[2];
    handles[1].y=params[0]/(1+params[1]*xs*xs);
}

void f_lorentz_base_h(double x_min,double x_max,double y_min,double y_max,
                      std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    
    handles[0].x=0.5*(x_max+x_min);
    handles[1].x=handles[0].x+0.25*x_span;
    
    if(y_max>0)
    {
        handles[0].y=0.8*y_max;
        handles[1].y=0.4*y_max;
    }
    else
    {
        handles[0].y=0.8*y_min;
        handles[1].y=0.4*y_min;
    }
}

//#############################
//   Lorenztzian Generalized
//#############################

void f_lorentz_gen(std::vector<double> const &x,
                   std::vector<double> const &params,
                   std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=std::abs(x[i]-params[2]);
        
        y[i]=params[0]/(1+params[1]*std::pow(xs,params[3]));
    }
}

void f_lorentz_gen_h_to_p(std::vector<FitterHandle> const &handles,
                          std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
    
    double x2=handles[1].x;
    double y2=handles[1].y;
    
    double x3=handles[2].x;
    double y3=handles[2].y;
    
    if(std::abs(x2-x1)>std::abs(x3-x1))
    {
        std::swap(x2,x3);
        std::swap(y2,y3);
    }
    
    params[0]=y1;
    params[2]=x1;
    
    using std::abs;
    
    if(abs(y2)>abs(y1) || abs(y3)>abs(y1) ||
       y1*y2<=0 || y1*y3<=0 || y2*y3<=0)
    {
        params[3]=0;
    }
    else
    {
        params[3]=std::log((y1/y2-1)/(y1/y3-1))/std::log(abs((x2-x1)/(x3-x1)));
    }
    
    params[1]=(y1/y3-1.0)/std::pow(abs(x3-x1),params[3]);
}

void f_lorentz_gen_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles)
{
    handles[0].x=params[2];
    handles[0].y=params[0];
    
    double xs;
    
    xs=std::abs(handles[1].x-params[2]);
    handles[1].y=params[0]/(1+params[1]*std::pow(xs,params[3]));
    
    xs=std::abs(handles[2].x-params[2]);
    handles[2].y=params[0]/(1+params[1]*std::pow(xs,params[3]));
}

void f_lorentz_gen_base_h(double x_min,double x_max,double y_min,double y_max,
                  std::vector<FitterHandle> &handles)
{
    double x_span=x_max-x_min;
    
    handles[0].x=0.5*(x_max+x_min);
    handles[1].x=handles[0].x+0.2*x_span;
    handles[2].x=handles[0].x+0.4*x_span;
    
    if(y_max>0)
    {
        handles[0].y=0.8*y_max;
        handles[1].y=0.4*y_max;
        handles[2].y=0.1*y_max;
    }
    else
    {
        handles[0].y=0.8*y_min;
        handles[1].y=0.4*y_min;
        handles[2].y=0.1*y_min;
    }
}

//########################
//   Planck - Frequency
//########################

void f_planck_freq(std::vector<double> const &x,
                   std::vector<double> const &params,
                   std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=x[i];
        
        if(xs<=0) y[i]=0;
        else y[i]=params[0]*xs*xs*xs/(std::exp(params[1]*xs)-1.0);
    }
}

double Planck_cste_f=2.821439372122079;

void f_planck_freq_h_to_p(std::vector<FitterHandle> const &handles,
                          std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
        
    params[0]=y1/(x1*x1*x1)*(std::exp(Planck_cste_f)-1.0);
    params[1]=Planck_cste_f/x1;
}

void f_planck_freq_p_to_h(std::vector<double> const &params,
                       std::vector<FitterHandle> &handles)
{
    handles[0].x=Planck_cste_f/params[1];
    
    double &x=handles[0].x;
    handles[0].y=params[0]*x*x*x/(std::exp(Planck_cste_f)-1.0);
}

void f_planck_freq_base_h(double x_min,double x_max,double y_min,double y_max,
                  std::vector<FitterHandle> &handles)
{
    handles[0].x=0.5*(x_max+std::max(0.0,x_min));
    
    if(y_max>0) handles[0].y=0.8*y_max;
    else handles[0].y=0.8*y_min; 
}

//#########################
//   Planck - Wavelength
//#########################

void f_planck_wvl(std::vector<double> const &x,
                  std::vector<double> const &params,
                  std::vector<double> &y)
{
    double xs;
    
    for(unsigned int i=0;i<x.size();i++)
    {
        xs=x[i];
        
        if(xs<=0) y[i]=0;
        else y[i]=params[0]/(xs*xs*xs*xs*xs)/(std::exp(params[1]/xs)-1.0);
    }
}

double Planck_cste_l=4.965114231744276;

void f_planck_wvl_h_to_p(std::vector<FitterHandle> const &handles,
                         std::vector<double> &params)
{
    const double &x1=handles[0].x;
    const double &y1=handles[0].y;
        
    params[0]=y1*(x1*x1*x1*x1*x1)*(std::exp(Planck_cste_l)-1.0);
    params[1]=Planck_cste_l*x1;
}

void f_planck_wvl_p_to_h(std::vector<double> const &params,
                         std::vector<FitterHandle> &handles)
{
    handles[0].x=params[1]/Planck_cste_l;
    
    double &x=handles[0].x;
    handles[0].y=params[0]/(x*x*x*x*x)/(std::exp(Planck_cste_l)-1.0);
}

void f_planck_wvl_base_h(double x_min,double x_max,double y_min,double y_max,
                         std::vector<FitterHandle> &handles)
{
    handles[0].x=0.5*(x_max+std::max(0.0,x_min));
    
    if(y_max>0) handles[0].y=0.8*y_max;
    else handles[0].y=0.8*y_min; 
}

//###################
//   Sine Function
//###################

void f_sine(std::vector<double> const &x,
            std::vector<double> const &params,
            std::vector<double> &y)
{
    for(unsigned int i=0;i<x.size();i++)
        y[i]=params[0]+params[1]*std::sin(params[2]*x[i]+params[3]);
}

void f_sine_h_to_p(std::vector<FitterHandle> const &handles,
                   std::vector<double> &params)
{
    const double &x0=handles[0].x;
    const double &y0=handles[0].y;
    
    const double &x1=handles[1].x;
    const double &y1=handles[1].y;
    
    double S0=-1;
    double S1=std::sin(-Pi/2.0+2.0*Pi*params[4]);
    
    params[1]=(y1-y0)/(S1-S0);
    params[0]=y0-params[1]*S0;
    params[2]=2.0*params[4]*Pi/(x1-x0);
    params[3]=-Pi/2.0-params[2]*x0;
}

void f_sine_p_to_h(std::vector<double> const &params,
                   std::vector<FitterHandle> &handles)
{
    handles[0].x=(-Pi/2.0-params[3])/params[2];
    handles[1].x=handles[0].x+params[4]*2.0*Pi/params[2];
    
    handles[0].y=params[0]+params[1]*std::sin(params[2]*handles[0].x+params[3]);
    handles[1].y=params[0]+params[1]*std::sin(params[2]*handles[1].x+params[3]);
}

void f_sine_base_h(double x_min,double x_max,double y_min,double y_max,
                   std::vector<FitterHandle> &handles)
{
    handles[0].x=x_min+0.1*(x_max-x_min);
    handles[0].y=y_min+0.1*(y_max-y_min);
    
    handles[1].x=x_max-0.1*(x_max-x_min);
    handles[1].y=y_max-0.1*(y_max-y_min);
}
