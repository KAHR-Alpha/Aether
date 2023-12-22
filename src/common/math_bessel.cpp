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

#include <math_bessel.h>



double bessel_J_kernel(double nu,double z,double t)
{
    return std::cos(z*std::sin(t)-nu*t);
}

double bessel_J_IM(int nu,double z,double precision)
{
    int i;
    
    precision=std::pow(10.0,-precision);
    
    int N=2*nu+3;
    double dt=Pi/(N-1.0);
    
    double I=-(bessel_J_kernel(nu,z,0)+bessel_J_kernel(nu,z,Pi))/2.0;
    for(i=0;i<N;i++) { I+=bessel_J_kernel(nu,z,i*dt); }
    I=dt*I;
    
    bool computation_running=true;
    
    int N_it=0;
    
    while(computation_running && N_it<=25)
    {
        double S=0;
        for(i=0;i<N-1;i++) { S+=bessel_J_kernel(nu,z,dt/2+i*dt); }
        double I2=I/2.0+dt/2.0*S;
        
        if(std::abs(I2-I)<=precision) computation_running=false;
        
        N=2*N-1;
        dt=Pi/(N-1.0);
        
        I=I2;
        N_it++;
    }
    
    return I/Pi;
}

double bessel_K_kernel(double nu,double z,double t)
{
    return std::cosh(t*nu)*std::exp(-z*std::cosh(t));
}

double bessel_K_IM(double nu,double z,double precision,double cut)
{
    int i;
    
    if(z<=0) return 1e300;
    
    precision=std::pow(10.0,-precision);
    cut=std::pow(10.0,-cut);
    
    double t_max=1.0;
    
    while(bessel_K_kernel(nu,z,t_max)>=cut*std::exp(-z))
    {
        t_max*=2.0;
    }
    
    int N=5;
    double dt=t_max/(N-1.0);
    
    double I=-(bessel_K_kernel(nu,z,0)+bessel_K_kernel(nu,z,t_max))/2.0;
    for(i=0;i<N;i++) { I+=bessel_K_kernel(nu,z,i*dt); }
    I=dt*I;
    
    bool computation_running=true;
    
    int N_it=0;
    
    while(computation_running && N_it<=20)
    {
        double S=0;
        for(i=0;i<N-1;i++) { S+=bessel_K_kernel(nu,z,dt/2+i*dt); }
        double I2=I/2.0+dt/2.0*S;
        
        if(std::abs((I2-I)/I)<=precision) computation_running=false;
        
        N=2*N-1;
        dt=t_max/(N-1.0);
        
        I=I2;
        N_it++;
    }
    
    return I;
}
