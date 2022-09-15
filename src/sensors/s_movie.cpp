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

#include <sensors.h>

MovieSensor::MovieSensor(int type_,
                         int x1_,int x2_,
                         int y1_,int y2_,
                         int z1_,int z2_,
                         int skip_,
                         double expo_)
    :cumulative(false),
     exposure(expo_),
     skip(skip_)
{
    set_type(type_);
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    
    if(type==NORMAL_X){ span1=y2-y1; span2=z2-z1; }
    if(type==NORMAL_Y){ span1=x2-x1; span2=z2-z1; }
    if(type==NORMAL_Z){ span1=x2-x1; span2=y2-y1; }
    
    image.set_size(span1,span2);
        
    f_x.init(span1,span2,0);
    f_y.init(span1,span2,0);
    f_z.init(span1,span2,0);
}

void MovieSensor::deep_feed(FDTD const &fdtd)
{
    Grid3<double> const &Ex=fdtd.Ex;
    Grid3<double> const &Ey=fdtd.Ey;
    Grid3<double> const &Ez=fdtd.Ez;
    
    if(step%skip==0)
    {
        int i,j,k;
        double tmp_x,tmp_y,tmp_z,tmp;
        
        if(cumulative)
        {
            if(type==NORMAL_X)
            {
                int span_c=x2-x1;
                
                for(j=0;j<span1;j++){ for(k=0;k<span2;k++)
                {
                    tmp_x=tmp_y=tmp_z=0;
                    
                    for(i=0;i<span_c;i++)
                    {
                        tmp_x+=Ex(x1+i,y1+j,z1+k);
                        tmp_y+=Ey(x1+i,y1+j,z1+k);
                        tmp_z+=Ez(x1+i,y1+j,z1+k);
                    }
                    
                    tmp_x/=static_cast<double>(span_c);
                    tmp_y/=static_cast<double>(span_c);
                    tmp_z/=static_cast<double>(span_c);
                    
                    f_x(j,k)=tmp_x;
                    f_y(j,k)=tmp_y;
                    f_z(j,k)=tmp_z;
                }}
            }
            else if(type==NORMAL_Y)
            {
                j=0;
                
                for(i=0;i<span1;i++){ for(k=0;k<span2;k++)
                {
                    f_x(i,k)=Ex(x1+i,y1+j,z1+k);
                    f_y(i,k)=Ey(x1+i,y1+j,z1+k);
                    f_z(i,k)=Ez(x1+i,y1+j,z1+k);
                }}
            }
            else if(type==NORMAL_Z)
            {
                k=0;
                
                for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
                {
                    f_x(i,j)=Ex(x1+i,y1+j,z1+k);
                    f_y(i,j)=Ey(x1+i,y1+j,z1+k);
                    f_z(i,j)=Ez(x1+i,y1+j,z1+k);
                }}
            }
        }
        else    // Non cumulative
        {
            if(type==NORMAL_X)
            {
                i=0;
                
                for(j=0;j<span1;j++){ for(k=0;k<span2;k++)
                {
                    f_x(j,k)=Ex(x1+i,y1+j,z1+k);
                    f_y(j,k)=Ey(x1+i,y1+j,z1+k);
                    f_z(j,k)=Ez(x1+i,y1+j,z1+k);
                }}
            }
            else if(type==NORMAL_Y)
            {
                j=0;
                
                for(i=0;i<span1;i++){ for(k=0;k<span2;k++)
                {
                    f_x(i,k)=Ex(x1+i,y1+j,z1+k);
                    f_y(i,k)=Ey(x1+i,y1+j,z1+k);
                    f_z(i,k)=Ez(x1+i,y1+j,z1+k);
                }}
            }
            else if(type==NORMAL_Z)
            {
                k=0;
                
                for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
                {
                    f_x(i,j)=Ex(x1+i,y1+j,z1+k);
                    f_y(i,j)=Ey(x1+i,y1+j,z1+k);
                    f_z(i,j)=Ez(x1+i,y1+j,z1+k);
                }}
            }
        }
        
        using std::exp;
        using std::abs;
        
        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
        {
            tmp_x=1.0-exp(-exposure*abs(f_x(i,j)));
            image.degra(i,j,tmp_x,0,1.0);
        }}
        
        image.write(name+"_"+std::to_string(step/skip)+"_Ex.png");
        
        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
        {
            tmp_y=1.0-exp(-exposure*abs(f_y(i,j)));
            image.degra(i,j,tmp_y,0,1.0);
        }}
        
        image.write(name+"_"+std::to_string(step/skip)+"_Ey.png");
        
        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
        {
            tmp_z=1.0-exp(-exposure*abs(f_z(i,j)));
            image.degra(i,j,tmp_z,0,1.0);
        }}
        
        image.write(name+"_"+std::to_string(step/skip)+"_Ez.png");
        
        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
        {
            tmp_x=std::abs(f_x(i,j));
            tmp_y=std::abs(f_y(i,j));
            tmp_z=std::abs(f_z(i,j));
            tmp=std::sqrt(tmp_x*tmp_x+tmp_y*tmp_y+tmp_z*tmp_z);
            
            tmp=1.0-exp(-exposure*tmp);
            
            image.degra(i,j,tmp,0,1.0);
        }}
        
        
        image.write(name+"_"+std::to_string(step/skip)+"_E.png");
    }
}
