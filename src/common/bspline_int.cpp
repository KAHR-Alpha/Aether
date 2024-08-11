/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <bspline_int.h>
#include <logger.h>

extern std::ofstream plog;

bsp_grid::bsp_grid()
    :glevel(0)
{
    a_x=1; b_x=0;
    a_y=1; b_y=0;
    
    Gs=std::pow(2,glevel)+3;
    
    phi.init(Gs,Gs,0);
}

void bsp_grid::subcomp(Grid2<double> &data)
{
    int i,j,k,l;
    int i1,j1;
    
    int Nd=data.L1();
    
    Grid2<double> bsp_num(Gs,Gs,0);
    Grid2<double> bsp_den(Gs,Gs,0);
    Grid2<bool> update_chk(Gs,Gs,0);
    
    Grid1<double> B_s(4,0),B_t(4,0);
    Grid2<double> w_ab(4,4,0);
    double S_ab;
    double w_c,z_c,phi_c;
    
    bsp_num=0;
    bsp_den=0;
    update_chk=0;
    
    double x,y;
    double s,t;
    
    Grid2<double> phi_tmp(Gs,Gs,0);
    
    for(i=0;i<Nd;i++)
    {
        x=a_x*data(i,0)+b_x;
        y=a_y*data(i,1)+b_y;
        
        s=x/Dx;
        t=y/Dy;
        
        k=static_cast<int>(s);
        l=static_cast<int>(t);
        
        s-=k;
        t-=l;
        
        B_s[0]=(1.0-3.0*s+3.0*s*s-s*s*s)/6.0;
        B_s[1]=(3.0*s*s*s-6.0*s*s+4.0)/6.0;
        B_s[2]=(-3.0*s*s*s+3.0*s*s+3.0*s+1)/6.0;
        B_s[3]=s*s*s/6.0;
        
        B_t[0]=(1.0-3.0*t+3.0*t*t-t*t*t)/6.0;
        B_t[1]=(3.0*t*t*t-6.0*t*t+4.0)/6.0;
        B_t[2]=(-3.0*t*t*t+3.0*t*t+3.0*t+1)/6.0;
        B_t[3]=t*t*t/6.0;
        
        S_ab=0;
        
        for(i1=0;i1<4;i1++){ for(j1=0;j1<4;j1++)
        {
            w_ab(i1,j1)=B_s[i1]*B_t[j1];
            S_ab+=w_ab(i1,j1)*w_ab(i1,j1);
        }}
        
        z_c=data(i,2);
        
        for(i1=0;i1<4;i1++){ for(j1=0;j1<4;j1++)
        {
            w_c=w_ab(i1,j1);
            phi_c=w_c*z_c/S_ab;
            
            bsp_num(k+i1,l+j1)+=w_c*w_c*phi_c;
            bsp_den(k+i1,l+j1)+=w_c*w_c;
            
            update_chk(k+i1,l+j1)=1;
        }}
    }
    
    for(i=0;i<Gs;i++){ for(j=0;j<Gs;j++)
    {
        if(update_chk(i,j) && bsp_den(i,j)!=0) phi_tmp(i,j)=bsp_num(i,j)/bsp_den(i,j);
        else phi_tmp(i,j)=0;
        
        phi(i,j)+=phi_tmp(i,j);
    }}
}

void bsp_grid::compute(Grid2<double> &data)
{
    int i,j;
    
    Grid2<double> tmp_data=data;
    
    Plog::print("SPLINES\n");
    
    int Nd=data.L1();
    
    double min_x=data(0,0), max_x=data(0,0);
    double min_y=data(0,1), max_y=data(0,1);
    double min_z=data(0,2), max_z=data(0,2);
    
    for(i=0;i<Nd;i++)
    {
        min_x=std::min(min_x,data(i,0));
        max_x=std::max(max_x,data(i,0));
        
        min_y=std::min(min_y,data(i,1));
        max_y=std::max(max_y,data(i,1));
        
        min_z=std::min(min_z,data(i,2));
        max_z=std::max(max_z,data(i,2));
    }
    
    double eps=1e-6*(max_z-min_z);
    
    a_x=0.8/(max_x-min_x);
    b_x=0.1-a_x*min_x;
    
    a_y=0.8/(max_y-min_y);
    b_y=0.1-a_y*min_y;
    
    Dx=1.0/(std::pow(2,glevel)+0.0);
    Dy=1.0/(std::pow(2,glevel)+0.0);
    
    phi=0;
    
    for(i=0;i<7;i++)
    
    max_z=0;
    for(i=0;i<Nd;i++) max_z=std::max(max_z,std::abs(data(i,2)));
    
    while(max_z>=eps && glevel<12)
    {
        subcomp(tmp_data);
        refine();
                        
        for(j=0;j<Nd;j++)
        {
            tmp_data(j,2)=data(j,2)-evaluate(data(j,0),data(j,1));
        }
        
        max_z=0;
        for(i=0;i<Nd;i++) max_z=std::max(max_z,std::abs(tmp_data(i,2)));
    }
    
    Plog::print("Spline interpolation level ", glevel, " ", max_z, " ", eps, "\n");
}

double bsp_grid::evaluate(double x_i,double y_i)
{
    int i,j,k,l;
        
    double x,y,s,t;
    double s2,s3;
    double t2,t3;
    
    Grid1<double> B_s(4,0),B_t(4,0);
    
    x=a_x*x_i+b_x;
    y=a_y*y_i+b_y;
    
    s=x/Dx;
    t=y/Dy;
        
    k=static_cast<int>(s);
    l=static_cast<int>(t);
    
    s-=k;
    t-=l;
    
    s2=s*s;
    s3=s2*s;
    
    t2=t*t;
    t3=t2*t;
    
    B_s[0]=(1.0-3.0*s+3.0*s2-s3)/6.0;
    B_s[1]=(3.0*s3-6.0*s2+4.0)/6.0;
    B_s[2]=(-3.0*s3+3.0*s2+3.0*s+1.0)/6.0;
    B_s[3]=s3/6.0;
    
    B_t[0]=(1.0-3.0*t+3.0*t2-t3)/6.0;
    B_t[1]=(3.0*t3-6.0*t2+4.0)/6.0;
    B_t[2]=(-3.0*t3+3.0*t2+3.0*t+1.0)/6.0;
    B_t[3]=t3/6.0;
    
    double S=0;
    
    for(i=0;i<4;i++){ for(j=0;j<4;j++)
    {
        S+=B_s[i]*B_t[j]*phi(i+k,j+l);
    }}
    
    return S;
}

void bsp_grid::refine()
{
    int i,j;
    
    int Gs2=3+std::pow(2,glevel+1);
    
    Grid2<double> tmpgrid(Gs2,Gs2,0);
    
    int i2,j2;
    double bsp_po,bsp_pp,bsp_op,bsp_mp,bsp_mo;
    double bsp_mm,bsp_om,bsp_pm,bsp_oo;
    
    for(i=0;i<Gs-1;i++){ for(j=0;j<Gs-1;j++)
    {
        bsp_po=bsp_pp=bsp_op=bsp_mp=bsp_mo=bsp_mm=bsp_om=bsp_pm=bsp_oo=0;
        
        bsp_po=phi(i+1,j);
        bsp_pp=phi(i+1,j+1);
        bsp_op=phi(i,j+1);
        if(i>0) bsp_mp=phi(i-1,j+1);
        if(i>0) bsp_mo=phi(i-1,j);
        if(i>0 && j>0) bsp_mm=phi(i-1,j-1);
        if(j>0) bsp_om=phi(i,j-1);
        if(j>0) bsp_pm=phi(i+1,j-1);
        bsp_oo=phi(i,j);
        
        i2=2*(i-1)+1;
        j2=2*(j-1)+1;
        
        if(i2>=0 && j2>=0) tmpgrid(i2,j2)=(bsp_mm+bsp_mp+bsp_pm+bsp_pp+6.0*(bsp_mo+bsp_om+bsp_op+bsp_po)+36.0*bsp_oo)/64.0;
        if(i2>=0) tmpgrid(i2,j2+1)=(bsp_mo+bsp_mp+bsp_po+bsp_pp+6.0*(bsp_oo+bsp_op))/16.0;
        if(j2>=0) tmpgrid(i2+1,j2)=(bsp_om+bsp_op+bsp_pm+bsp_pp+6.0*(bsp_oo+bsp_po))/16.0;
        tmpgrid(i2+1,j2+1)=(bsp_oo+bsp_op+bsp_po+bsp_pp)/4.0;
    }}
    
    phi.init(Gs2,Gs2,0);
    
    for(i=0;i<Gs2;i++) for(j=0;j<Gs2;j++) phi(i,j)=tmpgrid(i,j);
        
    glevel+=1;
    Gs=std::pow(2,glevel)+3;
    Dx/=2.0;
    Dy/=2.0;
}

void bsp_grid::show(int Nx,int Ny,std::string fname)
{
    int i,j,k,l;
    int i1,j1;
        
    double x,y,s,t;
    double s2,s3;
    double t2,t3;
    
    Grid1<double> B_s(4,0),B_t(4,0);
    Grid2<double> imtmp(Nx,Ny,0);
    
    std::string fname2=fname;
    fname2.append("_mod");
    std::ofstream f_out(fname2.c_str(),std::ios::out|std::ios::trunc);
    
    ProgDisp dsp(Nx*Ny,"Plotting");
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        x=0.1+i*0.8/(Nx-1.0);
        y=0.1+j*0.8/(Ny-1.0);
        
        s=x/Dx;
        t=y/Dy;
        
        k=static_cast<int>(s);
        l=static_cast<int>(t);
        
        s-=k;
        t-=l;
        
        s2=s*s;
        s3=s2*s;
        
        t2=t*t;
        t3=t2*t;
        
        B_s[0]=(1.0-3.0*s+3.0*s2-s3)/6.0;
        B_s[1]=(3.0*s3-6.0*s2+4.0)/6.0;
        B_s[2]=(-3.0*s3+3.0*s2+3.0*s+1.0)/6.0;
        B_s[3]=s3/6.0;
        
        B_t[0]=(1.0-3.0*t+3.0*t2-t3)/6.0;
        B_t[1]=(3.0*t3-6.0*t2+4.0)/6.0;
        B_t[2]=(-3.0*t3+3.0*t2+3.0*t+1.0)/6.0;
        B_t[3]=t3/6.0;
        
        double S=0;
        
        for(i1=0;i1<4;i1++){ for(j1=0;j1<4;j1++)
        {
            S+=B_s[i1]*B_t[j1]*phi(i1+k,j1+l);
        }}
        
        imtmp(i,j)=S;
        
        if(j==static_cast<int>(Ny*0.6/0.7))
        {
            f_out<<S<<std::endl;
        }
        
        ++dsp;
    }}
    
    Plog::print("min/max ", imtmp.min(), " ", imtmp.max(), "\n");
        
    G2_to_degra(imtmp,fname);
}
