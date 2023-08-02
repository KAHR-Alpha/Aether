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

#include <bitmap3.h>
#include <phys_tools.h>
#include <sensors.h>


extern const Imdouble Im;
extern std::ofstream plog;

//####################
//   FarFieldSensor
//####################

FarFieldSensor::FarFieldSensor(int x1_,int x2_,
                               int y1_,int y2_,
                               int z1_,int z2_,
                               int Nfx_,int Nfy_)
    :SensorFieldHolder(NORMAL_Z,x1_,x2_,y1_,y2_,z1_,z2_,true),
     Nfx(Nfx_), Nfy(Nfy_), n_index(1.0)
{
}

FarFieldSensor::~FarFieldSensor()
{
}

void FarFieldSensor::link(FDTD const &fdtd)
{
    SensorFieldHolder::link(fdtd);
    
    n_index=fdtd.get_index(x1,y1,z1);
}

void FarFieldSensor::set_resolution(int Nfx_,int Nfy_)
{
    Nfx=Nfx_;
    Nfy=Nfy_;
}

void FarFieldSensor::treat()
{
    int i,j,l,p,q;
    
    Imdouble ax=0,
             ay=0,
             az=0;
    
    std::string fname=name;
    fname.append("_farfield");
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    std::vector<Imdouble> precomp_x(x2-x1),precomp_y(y2-y1);
    
    ProgDisp dsp(Nl*(2*Nfx+1)*(2*Nfy+1),"Far Field Sensor");
    
    for(l=0;l<Nl;l++)
    {
        double k0=2.0*Pi/lambda[l];
        double kn=k0*n_index;
        
        file<<lambda[l]<<" "<<Nfx<<" "<<Nfy;
        
        for(i=-Nfx;i<=Nfx;i++)
        {
            double kx=i*kn/(Nfx+0.0);
            if(Nfx==0) kx=0;
            
            for(p=0;p<x2-x1;p++) precomp_x[p]=std::exp(-(p-(x2-x1-1)/2.0)*kx*Im);
            
            for(j=-Nfy;j<=Nfy;j++)
            {
                double ky=j*kn/(Nfy+0.0);
                if(Nfy==0) ky=0;
                
                if(kx*kx+ky*ky<=kn*kn)
                {
                    for(q=0;q<y2-y1;q++) precomp_y[q]=std::exp(-(q-(y2-y1-1)/2.0)*ky*Im);
                    
                    ax=ay=az=0;
                    
                    for(q=0;q<y2-y1;q++) for(p=0;p<x2-x1;p++)
                    {
                        Imdouble coeff=precomp_x[p]*precomp_y[q];
                        
                        ax+=t_Ex(p,q)*coeff;
                        ay+=t_Ey(p,q)*coeff;
                        az+=t_Ez(p,q)*coeff;
                    }
                    
                    double pw=std::abs(ax)*std::abs(ax)+
                              std::abs(ay)*std::abs(ay)+
                              std::abs(az)*std::abs(az);
                    
                    pw*=Dx*Dy*n_index/(2.0*mu0*c_light);
                    
                    file<<" "<<pw;
                }
                else file<<" "<<0;
                
                ++dsp;
            }
        }
        
        if(l!=Nl) file<<std::endl;
    }
}
