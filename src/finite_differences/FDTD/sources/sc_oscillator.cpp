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

#include <enum_constants.h>
#include <phys_tools.h>
#include <sources.h>

extern const Imdouble Im;

Oscillator::Oscillator(int type_,
                       int x1_,int x2_,
                       int y1_,int y2_,
                       int z1_,int z2_)
    :Source(x1_,x2_,y1_,y2_,z1_,z2_)
{
    set_type(type_);
}

Oscillator::~Oscillator()
{
}

void Oscillator::deep_inject_E(FDTD &fdtd)
{
    int i,j,k;
    
    Imdouble im_val=std::exp(-w0*step*Dt*Im)*std::exp(-std::pow(dw/2.0*((step-tshift)*Dt),2.0));
    double val=std::real(im_val)*50;
    
//    if(step%10==0) Plog::print("val "<<val<<std::endl;
    
    if(type==NORMAL_X)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ex(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_Y)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ey(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_Z)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ez(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_UNDEF)
    {
    }
}

void Oscillator::initialize()
{
    if(type==NORMAL_XM) type=NORMAL_X;
    else if(type==NORMAL_YM) type=NORMAL_Y;
    else if(type==NORMAL_ZM) type=NORMAL_Z;
    
    double alp=std::sqrt(0.1);
    double bet=1e-7;
    
    double w1=m_to_rad_Hz(lambda_max);
    double w2=m_to_rad_Hz(lambda_min);
    
    w0=(w1+w2)/2.0;
    dw=std::abs(w2-w1)/(2.0*std::sqrt(-std::log(alp)));
    tshift=std::sqrt(-4.0/(dw*dw)*std::log(bet))/Dt;
}

/*MR_Oscillator::MR_Oscillator(int type_,
                             int x1_,int x2_,
                             int y1_,int y2_,
                             int z1_,int z2_)
{
    step=0;
    
    set_type(type_);
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
}

MR_Oscillator::~MR_Oscillator()
{
}

void MR_Oscillator::deep_inject_E(FDTD &fdtd)
{
    int i,j,k;
    
    Imdouble im_val=std::exp(-w0*step*Dt*Im)*std::exp(-std::pow(dw/2.0*((step-tshift)*Dt),2.0));
    double val=std::real(im_val)*50;
    
    if(type==NORMAL_X)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ex(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_Y)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ey(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_Z)
    {
        for(i=x1;i<x2;i++) for(j=y1;j<y2;j++)  for(k=z1;k<z2;k++) 
        {
            fdtd.Ez(i,j,k)+=val;
        }
    }
    else if(type==NORMAL_UNDEF)
    {
    }
}

void MR_Oscillator::initialize()
{
    if(type==NORMAL_XM) type=NORMAL_X;
    else if(type==NORMAL_YM) type=NORMAL_Y;
    else if(type==NORMAL_ZM) type=NORMAL_Z;
    
    double alp=0.1;
    double bet=1e-7;
    
    double w1=2.0*Pi*c_light/lambda_max;
    double w2=2.0*Pi*c_light/lambda_min;
    
    w0=(w1+w2)/2.0;
    dw=std::abs(w2-w1)/(2.0*std::sqrt(-std::log(alp)));
    tshift=std::sqrt(-4.0/(dw*dw)*std::log(bet))/Dt;
}*/
