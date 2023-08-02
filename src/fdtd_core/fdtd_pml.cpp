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

#include <fdtd_core.h>
#include <bitmap3.h>
#include <thread_utils.h>


extern const Imdouble Im;
extern std::ofstream plog;

#define OLDPML

void FDTD::allocate_pml()
{
    int tNx,tNy,tNz;
    // PML X
    
    tNx=tNy=tNz=1;
    if(pml_xm!=0 || pml_xp!=0) { tNx=pml_xm+pml_xp; tNy=Ny; tNz=Nz; }
    
    PsiEyx.init(tNx,tNy,tNz,0);
    PsiEzx.init(tNx,tNy,tNz,0);
    PsiHyx.init(tNx,tNy,tNz,0);
    PsiHzx.init(tNx,tNy,tNz,0);
    
    kappa_x_E.init(Nx,1.0);
    kappa_x_H.init(Nx,1.0);
    
    b_x_E.init(Nx,0);
    b_x_H.init(Nx,0);
    
    c_x_E.init(Nx,0);
    c_x_H.init(Nx,0);
            
    // PML Y
    
    tNx=tNy=tNz=1;
    if(pml_ym!=0 || pml_yp!=0) { tNx=Nx; tNy=pml_ym+pml_yp; tNz=Nz; }
    
    PsiExy.init(tNx,tNy,tNz,0);
    PsiEzy.init(tNx,tNy,tNz,0);
    PsiHxy.init(tNx,tNy,tNz,0);
    PsiHzy.init(tNx,tNy,tNz,0);
    
    kappa_y_E.init(Ny,1.0);
    kappa_y_H.init(Ny,1.0);
    
    b_y_E.init(Ny,0);
    b_y_H.init(Ny,0);
    
    c_y_E.init(Ny,0);
    c_y_H.init(Ny,0);
        
    // PML Z
    
    tNx=tNy=tNz=1;
    if(pml_zm!=0 || pml_zp!=0) { tNx=Nx; tNy=Ny; tNz=pml_zm+pml_zp; }
    
    PsiExz.init(tNx,tNy,tNz,0);
    PsiEyz.init(tNx,tNy,tNz,0);
    PsiHxz.init(tNx,tNy,tNz,0);
    PsiHyz.init(tNx,tNy,tNz,0);
    
    kappa_z_E.init(Nz,1.0);
    kappa_z_H.init(Nz,1.0);
    
    b_z_E.init(Nz,0);
    b_z_H.init(Nz,0);
    
    c_z_E.init(Nz,0);
    c_z_H.init(Nz,0);
}

/*void FDTD::set_pml(int pml_xi,int pml_yi,int pml_zi)
{
    pml_x=pml_xi;
    pml_y=pml_yi;
    pml_z=pml_zi;
    
    int p_min;
    
    int tNx,tNy,tNz;
    
    // PML X
    
    tNx=tNy=tNz=1;
    if(pml_x) { tNx=Nx; tNy=Ny; tNz=Nz; }
    
    #ifdef OLDPML
    PsiEyx.init(tNx,tNy,tNz,0);
    PsiEzx.init(tNx,tNy,tNz,0);
    PsiHyx.init(tNx,tNy,tNz,0);
    PsiHzx.init(tNx,tNy,tNz,0);
    #endif
    
    kappa_x_E.init(tNx,1.0);
    kappa_x_H.init(tNx,1.0);
    
    //#ifdef OLDPML
    b_x_E.init(tNx,0);
    b_x_H.init(tNx,0);
    
    c_x_E.init(tNx,0);
    c_x_H.init(tNx,0);
    //#endif
    
    p_min=1; if(pml_x) p_min=2*pml_x;
    
    #ifndef OLDPML
    PsiEyx.init(p_min,tNy,tNz,0);
    PsiEzx.init(p_min,tNy,tNz,0);
    PsiHyx.init(p_min,tNy,tNz,0);
    PsiHzx.init(p_min,tNy,tNz,0);
    
    b_x_Ey.init(p_min,Ny,Nz,0); b_x_Ez.init(p_min,Ny,Nz,0);
    b_x_Hy.init(p_min,Ny,Nz,0); b_x_Hz.init(p_min,Ny,Nz,0);
    
    c_x_Ey.init(p_min,Ny,Nz,0); c_x_Ez.init(p_min,Ny,Nz,0);
    c_x_Hy.init(p_min,Ny,Nz,0); c_x_Hz.init(p_min,Ny,Nz,0);
    #endif
    
    // PML Y
    
    tNx=tNy=tNz=1;
    if(pml_y) { tNx=Nx; tNy=Ny; tNz=Nz; }
    
    #ifdef OLDPML
    PsiExy.init(tNx,tNy,tNz,0);
    PsiEzy.init(tNx,tNy,tNz,0);
    PsiHxy.init(tNx,tNy,tNz,0);
    PsiHzy.init(tNx,tNy,tNz,0);
    #endif
    
    kappa_y_E.init(tNy,1.0);
    kappa_y_H.init(tNy,1.0);
    
    #ifdef OLDPML
    b_y_E.init(tNy,0);
    b_y_H.init(tNy,0);
    
    c_y_E.init(tNy,0);
    c_y_H.init(tNy,0);
    #endif
    
    p_min=1; if(pml_y) p_min=2*pml_y;
    
    #ifndef OLDPML
    PsiExy.init(tNx,p_min,tNz,0);
    PsiEzy.init(tNx,p_min,tNz,0);
    PsiHxy.init(tNx,p_min,tNz,0);
    PsiHzy.init(tNx,p_min,tNz,0);
    
    b_y_Ex.init(Nx,p_min,Nz,0); b_y_Ez.init(Nx,p_min,Nz,0);
    b_y_Hx.init(Nx,p_min,Nz,0); b_y_Hz.init(Nx,p_min,Nz,0);
    
    c_y_Ex.init(Nx,p_min,Nz,0); c_y_Ez.init(Nx,p_min,Nz,0);
    c_y_Hx.init(Nx,p_min,Nz,0); c_y_Hz.init(Nx,p_min,Nz,0);
    #endif
    
    // PML Z
    
    tNx=tNy=tNz=1;
    if(pml_z) { tNx=Nx; tNy=Ny; tNz=Nz; }
    
    #ifdef OLDPML
    PsiExz.init(tNx,tNy,tNz,0);
    PsiEyz.init(tNx,tNy,tNz,0);
    PsiHxz.init(tNx,tNy,tNz,0);
    PsiHyz.init(tNx,tNy,tNz,0);
    #endif
    
    kappa_z_E.init(tNz,1.0);
    kappa_z_H.init(tNz,1.0);
    
    #ifdef OLDPML
    b_z_E.init(tNz,0);
    b_z_H.init(tNz,0);
    
    c_z_E.init(tNz,0);
    c_z_H.init(tNz,0);
    #endif
    
    p_min=1; if(pml_z) p_min=2*pml_z;
    
    #ifndef OLDPML
    PsiExz.init(tNx,tNy,p_min,0);
    PsiEyz.init(tNx,tNy,p_min,0);
    PsiHxz.init(tNx,tNy,p_min,0);
    PsiHyz.init(tNx,tNy,p_min,0);
    #endif
    
    b_z_Ex.init(Nx,Ny,p_min,0); b_z_Ey.init(Nx,Ny,p_min,0);
    b_z_Hx.init(Nx,Ny,p_min,0); b_z_Hy.init(Nx,Ny,p_min,0);
    
    c_z_Ex.init(Nx,Ny,p_min,0); c_z_Ey.init(Nx,Ny,p_min,0);
    c_z_Hx.init(Nx,Ny,p_min,0); c_z_Hy.init(Nx,Ny,p_min,0);
    
    pml_m=4;
    pml_ma=2;
    pml_smax_x=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dx);
    pml_smax_y=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dy);
    pml_smax_z_u=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz);
    pml_smax_z_d=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz);
    pml_kmax=15; /////////////////
    if(mode==M_OBLIQUE)
    {
//        pml_smax_x*=0.2;
//        pml_kmax=10;
    }
    pml_amax=0.2;
    
//    pml_coeff_calc();
}*/

/*void FDTD::set_pml(int pml_xi,int pml_yi,int pml_zi)
{
    pml_x=pml_xi;
    pml_y=pml_yi;
    pml_z=pml_zi;
        
    int tNx,tNy,tNz;
    
    // PML X
    
    tNx=tNy=tNz=1;
    if(pml_x) { tNx=2*pml_x; tNy=Ny; tNz=Nz; }
    
    PsiEyx.init(tNx,tNy,tNz,0);
    PsiEzx.init(tNx,tNy,tNz,0);
    PsiHyx.init(tNx,tNy,tNz,0);
    PsiHzx.init(tNx,tNy,tNz,0);
    
    kappa_x_E.init(Nx,1.0);
    kappa_x_H.init(Nx,1.0);
    
    b_x_E.init(Nx,0);
    b_x_H.init(Nx,0);
    
    c_x_E.init(Nx,0);
    c_x_H.init(Nx,0);
            
    // PML Y
    
    tNx=tNy=tNz=1;
    if(pml_y) { tNx=Nx; tNy=2*pml_y; tNz=Nz; }
    
    PsiExy.init(tNx,tNy,tNz,0);
    PsiEzy.init(tNx,tNy,tNz,0);
    PsiHxy.init(tNx,tNy,tNz,0);
    PsiHzy.init(tNx,tNy,tNz,0);
    
    kappa_y_E.init(Ny,1.0);
    kappa_y_H.init(Ny,1.0);
    
    b_y_E.init(Ny,0);
    b_y_H.init(Ny,0);
    
    c_y_E.init(Ny,0);
    c_y_H.init(Ny,0);
        
    // PML Z
    
    tNx=tNy=tNz=1;
    if(pml_z) { tNx=Nx; tNy=Ny; tNz=2*pml_z; }
    
    PsiExz.init(tNx,tNy,tNz,0);
    PsiEyz.init(tNx,tNy,tNz,0);
    PsiHxz.init(tNx,tNy,tNz,0);
    PsiHyz.init(tNx,tNy,tNz,0);
    
    kappa_z_E.init(Nz,1.0);
    kappa_z_H.init(Nz,1.0);
    
    b_z_E.init(Nz,0);
    b_z_H.init(Nz,0);
    
    c_z_E.init(Nz,0);
    c_z_H.init(Nz,0);
            
    pml_m=4;
    pml_ma=2;
    pml_smax_x=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dx);
    pml_smax_y=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dy);
    pml_smax_z_u=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz);
    pml_smax_z_d=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz);
    pml_kmax=15;
    pml_amax=0.2;
    
//    pml_coeff_calc();
}*/

void FDTD::pml_coeff_calc()
{
    int i,j,k;
    
    pml_m=4;
    pml_ma=2;
    
    kappa_x_E=1.0; kappa_y_E=1.0; kappa_z_E=1.0;
    kappa_x_H=1.0; kappa_y_H=1.0; kappa_z_H=1.0;
    b_x_E=0; b_y_E=0; b_z_E=0;
    c_x_E=0; c_y_E=0; c_z_E=0;
    b_x_H=0; b_y_H=0; b_z_H=0;
    c_x_H=0; c_y_H=0; c_z_H=0;
    
    if(pml_xm || pml_xp)
    {
        for(i=0;i<Nx;i++)
        {
            pml_x_coeffs(i,kappa_x_E[i],b_x_E[i],c_x_E[i]);
            pml_x_coeffs(i+0.5,kappa_x_H[i],b_x_H[i],c_x_H[i]);
        }
    }
    if(pml_ym || pml_yp)
    {
        for(j=0;j<Ny;j++)
        {
            pml_y_coeffs(j,kappa_y_E[j],b_y_E[j],c_y_E[j]);
            pml_y_coeffs(j+0.5,kappa_y_H[j],b_y_H[j],c_y_H[j]);
        }
    }
    if(pml_zm || pml_zp)
    {
        for(k=0;k<Nz;k++)
        {
            pml_z_coeffs(k,kappa_z_E[k],b_z_E[k],c_z_E[k]);
            pml_z_coeffs(k+0.5,kappa_z_H[k],b_z_H[k],c_z_H[k]);
        }
    }
}

void FDTD::pml_x_coeffs(double ind,double &kappa_x,double &b_x,double &c_x)
{
    kappa_x=1.0;
    
    if(ind<pml_xm)
    {
        double x=(pml_xm-ind)/static_cast<double>(pml_xm);
        
        kappa_x=1.0+(pml_kappa_xm-1.0)*std::pow(x,pml_m);
        
        double sig=pml_sigma_xm*std::pow(x,pml_m);
        double alp=pml_alpha_xm*std::pow(1.0-x,pml_ma);
        
        b_x=std::exp(-(sig/kappa_x+alp)*Dt/e0);
        c_x=sig/(sig*kappa_x+kappa_x*kappa_x*alp)*(b_x-1.0);
    }
    if(ind>Nx-pml_xp)
    {
        double x=(ind-(Nx-pml_xp))/static_cast<double>(pml_xp);
        
        kappa_x=1.0+(pml_kappa_xp-1.0)*std::pow(x,pml_m);
        
        double sig=pml_sigma_xp*std::pow(x,pml_m);
        double alp=pml_alpha_xp*std::pow(1.0-x,pml_ma);
        
        b_x=std::exp(-(sig/kappa_x+alp)*Dt/e0);
        c_x=sig/(sig*kappa_x+kappa_x*kappa_x*alp)*(b_x-1.0);
    }
}

void FDTD::pml_y_coeffs(double ind,double &kappa_y,double &b_y,double &c_y)
{
    kappa_y=1.0;
    
    if(ind<pml_ym)
    {
        double y=(pml_ym-ind)/static_cast<double>(pml_ym);
        
        kappa_y=1.0+(pml_kappa_ym-1.0)*std::pow(y,pml_m);
        
        double sig=pml_sigma_ym*std::pow(y,pml_m);
        double alp=pml_alpha_ym*std::pow(1.0-y,pml_ma);
        
        b_y=std::exp(-(sig/kappa_y+alp)*Dt/e0);
        c_y=sig/(sig*kappa_y+kappa_y*kappa_y*alp)*(b_y-1.0);
    }
    if(ind>Ny-pml_yp)
    {
        double y=(ind-(Ny-pml_yp))/static_cast<double>(pml_yp);
        
        kappa_y=1.0+(pml_kappa_yp-1.0)*std::pow(y,pml_m);
        
        double sig=pml_sigma_yp*std::pow(y,pml_m);
        double alp=pml_alpha_yp*std::pow(1.0-y,pml_ma);
        
        b_y=std::exp(-(sig/kappa_y+alp)*Dt/e0);
        c_y=sig/(sig*kappa_y+kappa_y*kappa_y*alp)*(b_y-1.0);
    }
}

void FDTD::pml_z_coeffs(double ind,double &kappa_z,double &b_z,double &c_z)
{
    kappa_z=1.0;
    
    if(ind<pml_zm)
    {
        double z=(pml_zm-ind)/static_cast<double>(pml_zm);
        
        kappa_z=1.0+(pml_kappa_zm-1.0)*std::pow(z,pml_m);
        
        double sig=pml_sigma_zm*std::pow(z,pml_m);
        double alp=pml_alpha_zm*std::pow(1.0-z,pml_ma);
        
        b_z=std::exp(-(sig/kappa_z+alp)*Dt/e0);
        c_z=sig/(sig*kappa_z+kappa_z*kappa_z*alp)*(b_z-1.0);
    }
    if(ind>Nz-pml_zp)
    {
        double z=(ind-(Nz-pml_zp))/static_cast<double>(pml_zp);
        
        kappa_z=1.0+(pml_kappa_zp-1.0)*std::pow(z,pml_m);
        
        double sig=pml_sigma_zp*std::pow(z,pml_m);
        double alp=pml_alpha_zp*std::pow(1.0-z,pml_ma);
        
        b_z=std::exp(-(sig/kappa_z+alp)*Dt/e0);
        c_z=sig/(sig*kappa_z+kappa_z*kappa_z*alp)*(b_z-1.0);
    }
}

/*void FDTD::pml_params_override(double kmax,double amax,double smax_coeff1,double smax_coeff2)
{
    pml_kmax=kmax;
    pml_amax=amax;
    pml_smax_z_u=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz)*smax_coeff1;
    pml_smax_z_d=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz)*smax_coeff2;
    
    //pml_coeff_calc();
}*/

double FDTD::pml_sig_x(double sig_base) { return 0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dx)*sig_base; }
double FDTD::pml_sig_y(double sig_base) { return 0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dy)*sig_base; }
double FDTD::pml_sig_z(double sig_base) { return 0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz)*sig_base; }

void FDTD::state_pml()
{
    std::cout<<"pml x: "<<pml_xm<<" "<<pml_xp<<std::endl;
    std::cout<<"pml y: "<<pml_ym<<" "<<pml_yp<<std::endl;
    std::cout<<"pml z: "<<pml_zm<<" "<<pml_zp<<std::endl;
    std::cout<<pml_smax_x<<" "<<pml_smax_y<<" "<<pml_smax_z_u<<" "<<pml_smax_z_d<<std::endl;
}

void FDTD::set_pml_xm(double kap,double sig,double alp)
{
    pml_kappa_xm=kap;
    pml_sigma_xm=pml_sig_x(sig);
    pml_alpha_xm=alp;
}

void FDTD::set_pml_xp(double kap,double sig,double alp)
{
    pml_kappa_xp=kap;
    pml_sigma_xp=pml_sig_x(sig);
    pml_alpha_xp=alp;
}

void FDTD::set_pml_ym(double kap,double sig,double alp)
{
    pml_kappa_ym=kap;
    pml_sigma_ym=pml_sig_y(sig);
    pml_alpha_ym=alp;
}

void FDTD::set_pml_yp(double kap,double sig,double alp)
{
    pml_kappa_yp=kap;
    pml_sigma_yp=pml_sig_y(sig);
    pml_alpha_yp=alp;
}

void FDTD::set_pml_zm(double kap,double sig,double alp)
{
    pml_kappa_zm=kap;
    pml_sigma_zm=pml_sig_z(sig);
    pml_alpha_zm=alp;
}

void FDTD::set_pml_zp(double kap,double sig,double alp)
{
    pml_kappa_zp=kap;
    pml_sigma_zp=pml_sig_z(sig);
    pml_alpha_zp=alp;
}

//###############
//###############

//#############################

void FDTD::app_pml_Ex(int i1,int i2)
{
    int i,j,k,n;
    
    int M;
    double C4;
    
    double inv_Dy=1.0/Dy;
    double inv_Dz=1.0/Dz;
    
    if(pml_ym || pml_yp)
    {
        for(i=i1;i<i2;i++){ for(k=0;k<Nz;k++)
        {
            for(j=1;j<pml_ym;j++)
            {
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiExy(i,j,k)=b_y_E[j]*PsiExy(i,j,k)+c_y_E[j]*inv_Dy*(Hz(i,j,k)-Hz(i,j-1,k));
                Ex(i,j,k)+=C4*PsiExy(i,j,k);
            }
            
            for(j=Ny-pml_yp+1;j<Ny;j++)
            {
                n=j-(Ny-pml_yp)+pml_ym;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiExy(i,n,k)=b_y_E[j]*PsiExy(i,n,k)+c_y_E[j]*inv_Dy*(Hz(i,j,k)-Hz(i,j-1,k));
                Ex(i,j,k)+=C4*PsiExy(i,n,k);
            }
        }}
        
        for(i=i1;i<i2;i++){ for(k=0;k<Nz;k++)
        {
            Ex(i,0,k)=0;
        }}
    }
    
    if(pml_zm || pml_zp)
    {
        for(k=1;k<pml_zm;k++)
        {
            for(j=0;j<Ny;j++){ for(i=i1;i<i2;i++)
            {
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiExz(i,j,k)=b_z_E[k]*PsiExz(i,j,k)+c_z_E[k]*inv_Dz*(Hy(i,j,k)-Hy(i,j,k-1));
                Ex(i,j,k)-=C4*PsiExz(i,j,k);
            }}
        }
        
        for(k=Nz-pml_zp+1;k<Nz;k++)
        {
            for(j=0;j<Ny;j++){ for(i=i1;i<i2;i++)
            {
                n=k-(Nz-pml_zp)+pml_zm;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiExz(i,j,n)=b_z_E[k]*PsiExz(i,j,n)+c_z_E[k]*inv_Dz*(Hy(i,j,k)-Hy(i,j,k-1));
                Ex(i,j,k)-=C4*PsiExz(i,j,n);
            }}
        }
        
        //Bottom PEC
        
        for(i=i1;i<i2;i++){ for(j=0;j<Ny;j++)
        {
            Ex(i,j,0)=0;
        }}
    }
}

void FDTD::app_pml_Ey(int j1,int j2)
{
    int i,j,k,n;
    int M;
    double C4;
    
    double inv_Dx=1.0/Dx;
    double inv_Dz=1.0/Dz;
    
    if(pml_xm || pml_xp)
    {
        for(j=j1;j<j2;j++){ for(k=0;k<Nz;k++)
        {
            for(i=1;i<pml_xm;i++)
            {
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEyx(i,j,k)=b_x_E[i]*PsiEyx(i,j,k)+c_x_E[i]*inv_Dx*(Hz(i,j,k)-Hz(i-1,j,k));
                Ey(i,j,k)-=C4*PsiEyx(i,j,k);
            }
            
            for(i=Nx-pml_xp+1;i<Nx;i++)
            {
                n=i-(Nx-pml_xp)+pml_xm;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                                
                PsiEyx(n,j,k)=b_x_E[i]*PsiEyx(n,j,k)+c_x_E[i]*inv_Dx*(Hz(i,j,k)-Hz(i-1,j,k));
                Ey(i,j,k)-=C4*PsiEyx(n,j,k);
            }
        }}
        
        //Bottom PEC
        
        for(j=j1;j<j2;j++){ for(k=0;k<Nz;k++)
        {
            Ey(0,j,k)=0;
        }}
    }
    
    if(pml_zm || pml_zp)
    {
        for(k=1;k<pml_zm;k++)
        {
            for(j=j1;j<j2;j++){ for(i=0;i<Nx;i++)
            {
            M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEyz(i,j,k)=b_z_E[k]*PsiEyz(i,j,k)+c_z_E[k]*inv_Dz*(Hx(i,j,k)-Hx(i,j,k-1));
                Ey(i,j,k)+=C4*PsiEyz(i,j,k);
            }
        }}
        
        for(k=Nz-pml_zp+1;k<Nz;k++)
        {
            for(j=j1;j<j2;j++){ for(i=0;i<Nx;i++)
            {
                n=k-(Nz-pml_zp)+pml_zm;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEyz(i,j,n)=b_z_E[k]*PsiEyz(i,j,n)+c_z_E[k]*inv_Dz*(Hx(i,j,k)-Hx(i,j,k-1));
                Ey(i,j,k)+=C4*PsiEyz(i,j,n);
            }
        }}
        
        //Bottom PEC
        
        for(i=0;i<Nx;i++){ for(j=j1;j<j2;j++)
        {
            Ey(i,j,0)=0;
        }}
    }
}

void FDTD::app_pml_Ez(int k1,int k2)
{
    int i,j,k,n;
    int M;
    double C4;
    
    double inv_Dx=1.0/Dx;
    double inv_Dy=1.0/Dy;
    
    if(pml_xm || pml_xp)
    {
        for(j=0;j<Ny;j++){ for(k=k1;k<k2;k++)
        {
            for(i=1;i<pml_xm;i++)
            {
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEzx(i,j,k)=b_x_E[i]*PsiEzx(i,j,k)+c_x_E[i]*inv_Dx*(Hy(i,j,k)-Hy(i-1,j,k));
                Ez(i,j,k)+=C4*PsiEzx(i,j,k);
            }
            
            for(i=Nx-pml_xp+1;i<Nx;i++)
            {
                n=i-(Nx-pml_xp)+pml_xm;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEzx(n,j,k)=b_x_E[i]*PsiEzx(n,j,k)+c_x_E[i]*inv_Dx*(Hy(i,j,k)-Hy(i-1,j,k));
                Ez(i,j,k)+=C4*PsiEzx(n,j,k);
            }
        }}
        
        //Bottom PEC
        
        for(j=0;j<Ny;j++){ for(k=k1;k<k2;k++)
        {
            Ez(0,j,k)=0;
        }}
    }
    
    if(pml_ym || pml_yp)
    {
        for(i=0;i<Nx;i++){ for(k=k1;k<k2;k++)
        {
            for(j=1;j<pml_ym;j++)
            {
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEzy(i,j,k)=b_y_E[j]*PsiEzy(i,j,k)+c_y_E[j]*inv_Dy*(Hx(i,j,k)-Hx(i,j-1,k));
                Ez(i,j,k)-=C4*PsiEzy(i,j,k);
            }
            
            for(j=Ny-pml_yp+1;j<Ny;j++)
            {
                n=j-(Ny-pml_yp)+pml_ym;
                
                M=matsgrid(i,j,k);
                C4=mats[M].pml_coeff();
                
                PsiEzy(i,n,k)=b_y_E[j]*PsiEzy(i,n,k)+c_y_E[j]*inv_Dy*(Hx(i,j,k)-Hx(i,j-1,k));
                Ez(i,j,k)-=C4*PsiEzy(i,n,k);
            }
        }}
        
        //Bottom PEC
        
        for(i=0;i<Nx;i++){ for(k=k1;k<k2;k++)
        {
            Ez(i,0,k)=0;
        }}
    }
}

void FDTD::app_pml_Hx(int i1,int i2)
{
    int i,j,k,n;
    
    double inv_Dy=1.0/Dy;
    double inv_Dz=1.0/Dz;
    
    if(pml_ym || pml_yp)
    {
        //Bottom layer
        
        for(i=i1;i<i2;i++){ for(k=0;k<Nz;k++)
        {
            for(j=0;j<pml_ym;j++)
            {
                PsiHxy(i,j,k)=b_y_H[j]*PsiHxy(i,j,k)+c_y_H[j]*inv_Dy*(Ez(i,j+1,k)-Ez(i,j,k));
                Hx(i,j,k)-=dtm*PsiHxy(i,j,k);
            }
            
            for(j=Ny-pml_yp;j<Ny-1;j++)
            {
                n=j-(Ny-pml_yp)+pml_ym;
                
                PsiHxy(i,n,k)=b_y_H[j]*PsiHxy(i,n,k)+c_y_H[j]*inv_Dy*(Ez(i,j+1,k)-Ez(i,j,k));
                Hx(i,j,k)-=dtm*PsiHxy(i,n,k);
            }
        }}
        
        //Top PEC
        
        for(i=i1;i<i2;i++){ for(k=0;k<Nz;k++)
        {
            Hx(i,Ny-1,k)=0;
        }}
    }
    
    if(pml_zm || pml_zp)
    {
        for(k=0;k<pml_zm;k++){ for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++)
            {
                PsiHxz(i,j,k)=b_z_H[k]*PsiHxz(i,j,k)+c_z_H[k]*inv_Dz*(Ey(i,j,k+1)-Ey(i,j,k));
                Hx(i,j,k)+=dtm*PsiHxz(i,j,k);
            }
        }}
            
        for(k=Nz-pml_zp;k<Nz-1;k++){ for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++)
            {
                n=k-(Nz-pml_zp)+pml_zm;
                
                PsiHxz(i,j,n)=b_z_H[k]*PsiHxz(i,j,n)+c_z_H[k]*inv_Dz*(Ey(i,j,k+1)-Ey(i,j,k));
                Hx(i,j,k)+=dtm*PsiHxz(i,j,n);
            }
        }}
        
        //Top PEC
        
        for(i=i1;i<i2;i++){ for(j=0;j<Ny;j++)
        {
            Hx(i,j,Nz-1)=Hy(i,j,Nz-1)=0;
        }}
    }
}

void FDTD::app_pml_Hy(int j1,int j2)
{
    int i,j,k,n;
    
    double inv_Dx=1.0/Dx;
    double inv_Dz=1.0/Dz;
    
    if(pml_xm || pml_xp)
    {
        for(j=j1;j<j2;j++){ for(k=0;k<Nz;k++)
        {
            for(i=0;i<pml_xm;i++)
            {
                PsiHyx(i,j,k)=b_x_H[i]*PsiHyx(i,j,k)+c_x_H[i]*inv_Dx*(Ez(i+1,j,k)-Ez(i,j,k));
                Hy(i,j,k)+=dtm*PsiHyx(i,j,k);
            }
            
            for(i=Nx-pml_xp;i<Nx-1;i++)
            {
                n=i-(Nx-pml_xp)+pml_xm;
                
                PsiHyx(n,j,k)=b_x_H[i]*PsiHyx(n,j,k)+c_x_H[i]*inv_Dx*(Ez(i+1,j,k)-Ez(i,j,k));
                Hy(i,j,k)+=dtm*PsiHyx(n,j,k);
            }
        }}
        
        //Top PEC
        
        for(j=j1;j<j2;j++){ for(k=0;k<Nz;k++)
        {
            Hy(Nx-1,j,k)=0;
        }}
    }
    
    if(pml_zm || pml_zp)
    {
        for(k=0;k<pml_zm;k++)
        {
            for(j=j1;j<j2;j++){ for(i=0;i<Nx;i++)
            {
                PsiHyz(i,j,k)=b_z_H[k]*PsiHyz(i,j,k)+c_z_H[k]*inv_Dz*(Ex(i,j,k+1)-Ex(i,j,k));
                Hy(i,j,k)-=dtm*PsiHyz(i,j,k);
            }}
        }
        
        for(k=Nz-pml_zp;k<Nz-1;k++)
        {
            for(j=j1;j<j2;j++){ for(i=0;i<Nx;i++)
            {
                n=k-(Nz-pml_zp)+pml_zm;
                
                PsiHyz(i,j,n)=b_z_H[k]*PsiHyz(i,j,n)+c_z_H[k]*inv_Dz*(Ex(i,j,k+1)-Ex(i,j,k));
                Hy(i,j,k)-=dtm*PsiHyz(i,j,n);
            }}
        }
        
        //Top PEC
        
        for(i=0;i<Nx;i++){ for(j=j1;j<j2;j++)
        {
            Hy(i,j,Nz-1)=0;
        }}
    }
}

void FDTD::app_pml_Hz(int k1,int k2)
{
    int i,j,k,n;
    
    double inv_Dx=1.0/Dx;
    double inv_Dy=1.0/Dy;
    
    if(pml_xm || pml_xp)
    {
        for(j=0;j<Ny;j++){ for(k=k1;k<k2;k++)
        {
            for(i=0;i<pml_xm;i++)
            {
                PsiHzx(i,j,k)=b_x_H[i]*PsiHzx(i,j,k)+c_x_H[i]*inv_Dx*(Ey(i+1,j,k)-Ey(i,j,k));
                Hz(i,j,k)-=dtm*PsiHzx(i,j,k);
            }
            
            for(i=Nx-pml_xp;i<Nx-1;i++)
            {
                n=i-(Nx-pml_xp)+pml_xm;
                
                PsiHzx(n,j,k)=b_x_H[i]*PsiHzx(n,j,k)+c_x_H[i]*inv_Dx*(Ey(i+1,j,k)-Ey(i,j,k));
                Hz(i,j,k)-=dtm*PsiHzx(n,j,k);
            }
        }}
        
        //Top PEC
        
        for(j=0;j<Ny;j++){ for(k=k1;k<k2;k++)
        {
            Hz(Nx-1,j,k)=0;
        }}
    }
    
    if(pml_ym || pml_yp)
    {
        for(i=0;i<Nx;i++){ for(k=k1;k<k2;k++)
        {
            for(j=0;j<pml_ym;j++)
            {
                PsiHzy(i,j,k)=b_y_H[j]*PsiHzy(i,j,k)+c_y_H[j]*inv_Dy*(Ex(i,j+1,k)-Ex(i,j,k));
                Hz(i,j,k)+=dtm*PsiHzy(i,j,k);
            }
            
            for(j=Ny-pml_yp;j<Ny-1;j++)
            {
                n=j-(Ny-pml_yp)+pml_ym;
                
                PsiHzy(i,n,k)=b_y_H[j]*PsiHzy(i,n,k)+c_y_H[j]*inv_Dy*(Ex(i,j+1,k)-Ex(i,j,k));
                Hz(i,j,k)+=dtm*PsiHzy(i,n,k);
            }
        }}
        
        //Top PEC
        
        for(i=0;i<Nx;i++){ for(k=k1;k<k2;k++)
        {
            Hz(i,Ny-1,k)=0;
        }}
    }
}

