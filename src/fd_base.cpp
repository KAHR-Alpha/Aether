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

#include <fd_base.h>
#include <lua_material.h>

extern const double Pi;
extern const Imdouble Im;

FD_Base::FD_Base()
    :Nx(60), Ny(60), Nz(60),
     Nxy(Nx*Ny), Nxyz(Nx*Ny*Nz),
     Nmats(0),
     Dx(5e-9), Dy(5e-9), Dz(5e-9),
     Nx_s(Nx), Ny_s(60), Nz_s(60),
     xs_s(0), xs_e(Nx),
     ys_s(0), ys_e(Ny),
     zs_s(0), zs_e(Nz),
     pad_xm(0), pad_xp(0),
     pad_ym(0), pad_yp(0),
     pad_zm(0), pad_zp(0),
     pml_x(false),
     pml_y(false),
     pml_z(false),
     pml_xm(0), pml_xp(0),
     pml_ym(0), pml_yp(0),
     pml_zm(0), pml_zp(0),
     pml_m(4.0), pml_ma(2.0),
     pml_kappa_xm(1), pml_kappa_xp(1),
     pml_kappa_ym(1), pml_kappa_yp(1),
     pml_kappa_zm(1), pml_kappa_zp(1),
     pml_sigma_xm(0), pml_sigma_xp(0),
     pml_sigma_ym(0), pml_sigma_yp(0),
     pml_sigma_zm(0), pml_sigma_zp(0),
     pml_alpha_xm(0), pml_alpha_xp(0),
     pml_alpha_ym(0), pml_alpha_yp(0),
     pml_alpha_zm(0), pml_alpha_zp(0),
     matsgrid(Nx,Ny,Nz,0)
{
}

void FD_Base::extend_grid()
{
    extend_grid_sub(matsgrid);
}

void FD_Base::extend_grid_sub(Grid3<unsigned int> &matsgrid)
{
    int i,j,k;
    
    // X
    
    for(i=0;i<xs_s;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
    {
        matsgrid(i,j,k)=matsgrid(xs_s,j,k);
    }}}
    
    for(i=xs_e;i<Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
    {
        matsgrid(i,j,k)=matsgrid(xs_e-1,j,k);
    }}}
    
    // Y
    
    for(i=0;i<Nx;i++){ for(j=0;j<ys_s;j++){ for(k=zs_s;k<zs_e;k++)
    {
        matsgrid(i,j,k)=matsgrid(i,ys_s,k);
    }}}
    
    for(i=0;i<Nx;i++){ for(j=ys_e;j<Ny;j++){ for(k=zs_s;k<zs_e;k++)
    {
        matsgrid(i,j,k)=matsgrid(i,ys_e-1,k);
    }}}
    
    // Z
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<zs_s;k++)
    {
        matsgrid(i,j,k)=matsgrid(i,j,zs_s);
    }}}
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=zs_e;k<Nz;k++)
    {
        matsgrid(i,j,k)=matsgrid(i,j,zs_e-1);
    }}}
}

Imdouble FD_Base::get_Dx(double ind,double w)
{
    Imdouble Dx_o=Dx;
    
    double x,kappa,sig,alp;
    Imdouble s;
    
    if(ind<pml_xm)
    {
        x=(pml_xm-ind)/static_cast<double>(pml_xm);
        
        kappa=1.0+(pml_kappa_xm-1.0)*std::pow(x,pml_m);
        
        sig=pml_sigma_xm*std::pow(x,pml_m);
        alp=pml_alpha_xm*std::pow(1.0-x,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dx_o*=s;
    }
    if(ind>Nx-pml_xp)
    {
        x=(ind-(Nx-pml_xp))/static_cast<double>(pml_xp);
        
        kappa=1.0+(pml_kappa_xp-1.0)*std::pow(x,pml_m);
        
        sig=pml_sigma_xp*std::pow(x,pml_m);
        alp=pml_alpha_xp*std::pow(1.0-x,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dx_o*=s;
    }
    
    return Dx_o;
}

Imdouble FD_Base::get_Dy(double ind,double w)
{
    Imdouble Dy_o=Dy;
    
    double y,kappa,sig,alp;
    Imdouble s;
    
    if(ind<pml_ym)
    {
        y=(pml_ym-ind)/static_cast<double>(pml_ym);
        
        kappa=1.0+(pml_kappa_ym-1.0)*std::pow(y,pml_m);
        
        sig=pml_sigma_ym*std::pow(y,pml_m);
        alp=pml_alpha_ym*std::pow(1.0-y,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dy_o*=s;
    }
    if(ind>Ny-pml_yp)
    {
        y=(ind-(Ny-pml_yp))/static_cast<double>(pml_yp);
        
        kappa=1.0+(pml_kappa_yp-1.0)*std::pow(y,pml_m);
        
        sig=pml_sigma_yp*std::pow(y,pml_m);
        alp=pml_alpha_yp*std::pow(1.0-y,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dy_o*=s;
    }
    
    return Dy_o;
}

Imdouble FD_Base::get_Dz(double ind,double w)
{
    Imdouble Dz_o=Dz;
    
    double z,kappa,sig,alp;
    Imdouble s;
    
    if(ind<pml_zm)
    {
        z=(pml_zm-ind)/static_cast<double>(pml_zm);
        
        kappa=1.0+(pml_kappa_zm-1.0)*std::pow(z,pml_m);
        
        sig=pml_sigma_zm*std::pow(z,pml_m);
        alp=pml_alpha_zm*std::pow(1.0-z,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dz_o*=s;
    }
    if(ind>Nz-pml_zp)
    {
        z=(ind-(Nz-pml_zp))/static_cast<double>(pml_zp);
        
        kappa=1.0+(pml_kappa_zp-1.0)*std::pow(z,pml_m);
        
        sig=pml_sigma_zp*std::pow(z,pml_m);
        alp=pml_alpha_zp*std::pow(1.0-z,pml_ma);
        
        s=kappa+sig/(alp-w*e0*Im);
        Dz_o*=s;
    }
    
    return Dz_o;
}

int FD_Base::index(int i,int j,int k)
{
    return i+j*Nx+k*Nxy;
}

int FD_Base::index_Ex(int k) { return index(0,0,k,0); }
int FD_Base::index_Ex(int i,int j,int k) { return index(i,j,k,0); }

int FD_Base::index_Ey(int k) { return index(0,0,k,1); }
int FD_Base::index_Ey(int i,int j,int k) { return index(i,j,k,1); }

int FD_Base::index_Ez(int k) { return index(0,0,k,2); }
int FD_Base::index_Ez(int i,int j,int k) { return index(i,j,k,2); }

int FD_Base::index_Hx(int k) { return index(0,0,k,3); }
int FD_Base::index_Hx(int i,int j,int k) { return index(i,j,k,3); }

int FD_Base::index_Hy(int k) { return index(0,0,k,4); }
int FD_Base::index_Hy(int i,int j,int k) { return index(i,j,k,4); }

int FD_Base::index_Hz(int k) { return index(0,0,k,5); }
int FD_Base::index_Hz(int i,int j,int k) { return index(i,j,k,5); }

void FD_Base::init_pml()
{
    pml_m=4;
    pml_ma=2;
    
    double pml_opti_x=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dx);
    double pml_opti_y=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dy);
    double pml_opti_z=0.8*(pml_m+1.0)/(std::sqrt(mu0/e0)*Dz);
    
    if(pml_xm>0 || pml_xp>0) pml_x=true;
    if(pml_ym>0 || pml_yp>0) pml_y=true;
    if(pml_zm>0 || pml_zp>0) pml_z=true;
    
    if(pml_xm>0 && pad_xm<=0) pad_xm=5;
    if(pml_xp>0 && pad_xp<=0) pad_xp=5;
    if(pml_ym>0 && pad_ym<=0) pad_ym=5;
    if(pml_yp>0 && pad_yp<=0) pad_yp=5;
    if(pml_zm>0 && pad_zm<=0) pad_zm=5;
    if(pml_zp>0 && pad_zp<=0) pad_zp=5;
    
    chk_var(pml_xm);
    chk_var(pml_xp);
    chk_var(pml_ym);
    chk_var(pml_yp);
    chk_var(pml_zm);
    chk_var(pml_zp);
    
    pml_sigma_xm*=pml_opti_x;
    pml_sigma_xp*=pml_opti_x;
    pml_sigma_ym*=pml_opti_y;
    pml_sigma_yp*=pml_opti_y;
    pml_sigma_zm*=pml_opti_z;
    pml_sigma_zp*=pml_opti_z;
}

void FD_Base::set_materials(Grid1<Material> const &M)
{
    int Nmats=M.L1();
    mats.init(Nmats);
    
    mats=M;
}

void FD_Base::set_material(unsigned int m_ID,Material const &material_)
{
    if(m_ID<Nmats) mats[m_ID]=material_;
}

void FD_Base::set_material(unsigned int m_ID,std::string fname)
{
    if(m_ID<Nmats)
    {
        lua_material::Loader loader;
        loader.load(&mats[m_ID],fname);
    }
}

void FD_Base::set_matsgrid(Grid3<unsigned int> const &G)
{
    int i,j,k;
    
    Nx_s=G.L1();
    Ny_s=G.L2();
    Nz_s=G.L3();
    
    init_pml();
    update_Nxyz();
    
    // Initial copy
    
    for(i=0;i<Nx_s;i++){ for(j=0;j<Ny_s;j++){ for(k=0;k<Nz_s;k++)
    {
        matsgrid(xs_s+i,j+ys_s,k+zs_s)=G(i,j,k);
    }}}
    
    extend_grid();
    
    Nmats=matsgrid.max()+1;
    mats.init(Nmats);
    
    chk_var(Nmats);
}

//void FD_Base::set_matsgrid_full(Grid3<unsigned int> const &G) { matsgrid=G; }

void FD_Base::set_padding(int xm,int xp,int ym,int yp,int zm,int zp)
{
    pad_xm=xm; pad_xp=xp;
    pad_ym=ym; pad_yp=yp;
    pad_zm=zm; pad_zp=zp;
}

void FD_Base::set_pml_xm(int N_pml,double kap,double sig,double alp)
{
    pml_xm=N_pml;
    pml_kappa_xm=kap;
    pml_sigma_xm=sig;
    pml_alpha_xm=alp;
}

void FD_Base::set_pml_xp(int N_pml,double kap,double sig,double alp)
{
    pml_xp=N_pml;
    pml_kappa_xp=kap;
    pml_sigma_xp=sig;
    pml_alpha_xp=alp;
}

void FD_Base::set_pml_ym(int N_pml,double kap,double sig,double alp)
{
    pml_ym=N_pml;
    pml_kappa_ym=kap;
    pml_sigma_ym=sig;
    pml_alpha_ym=alp;
}

void FD_Base::set_pml_yp(int N_pml,double kap,double sig,double alp)
{
    pml_yp=N_pml;
    pml_kappa_yp=kap;
    pml_sigma_yp=sig;
    pml_alpha_yp=alp;
}

void FD_Base::set_pml_zm(int N_pml,double kap,double sig,double alp)
{
    pml_zm=N_pml;
    pml_kappa_zm=kap;
    pml_sigma_zm=sig;
    pml_alpha_zm=alp;
}

void FD_Base::set_pml_zp(int N_pml,double kap,double sig,double alp)
{
    pml_zp=N_pml;
    pml_kappa_zp=kap;
    pml_sigma_zp=sig;
    pml_alpha_zp=alp;
}

void FD_Base::update_Nxyz()
{
    xs_s=pml_xm+pad_xm;
    ys_s=pml_ym+pad_ym;
    zs_s=pml_zm+pad_zm;
    
    xs_e=xs_s+Nx_s;
    ys_e=ys_s+Ny_s;
    zs_e=zs_s+Nz_s;
    
    Nx=xs_e+pad_xp+pml_xp;
    Ny=ys_e+pad_yp+pml_yp;
    Nz=zs_e+pad_zp+pml_zp;
    
    Nxy=Nx*Ny;
    Nxyz=Nx*Ny*Nz;
    
    matsgrid.init(Nx,Ny,Nz,0);
}
