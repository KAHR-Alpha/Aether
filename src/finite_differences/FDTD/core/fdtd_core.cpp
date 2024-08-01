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


extern const Imdouble Im;
extern std::ofstream plog;

#include <exception>

FDTD::FDTD(int Nx_,int Ny_,int Nz_,int Nt_,
           double Dx_,double Dy_,double Dz_,double Dt_,
           std::string smod,
           int pml_xm_,int pml_xp_,
           int pml_ym_,int pml_yp_,
           int pml_zm_,int pml_zp_)
    :tstep(0), Nx(Nx_), Ny(Ny_), Nz(Nz_), Nt(Nt_), Ntap(0), Nmat(1),
     Dx(Dx_), Dy(Dy_), Dz(Dz_), Dt(Dt_), fact(0),
     kx(0), ky(0),
     enable_Ex(true), enable_Ey(true), enable_Ez(true),
     enable_Hx(true), enable_Hy(true), enable_Hz(true),
     pml_xm(pml_xm_), pml_xp(pml_xp_),
     pml_ym(pml_ym_), pml_yp(pml_yp_),
     pml_zm(pml_zm_), pml_zp(pml_zp_),
     pml_kappa_xm(1.0), pml_kappa_xp(1.0),
     pml_kappa_ym(1.0), pml_kappa_yp(1.0),
     pml_kappa_zm(1.0), pml_kappa_zp(1.0),
     pml_sigma_xm(0), pml_sigma_xp(0),
     pml_sigma_ym(0), pml_sigma_yp(0),
     pml_sigma_zm(0), pml_sigma_zp(0),
     pml_alpha_xm(0), pml_alpha_xp(0),
     pml_alpha_ym(0), pml_alpha_yp(0),
     pml_alpha_zm(0), pml_alpha_zp(0),
     Nthreads(max_threads_number()),
     allow_run_E(false),
     alternator_E(Nthreads),
     threads_ready_E(Nthreads), threads_E(Nthreads),
     allow_run_H(false),
     alternator_H(Nthreads),
     threads_ready_H(Nthreads), threads_H(Nthreads)
{
    dt_D_comp=0;
    dt_B_comp=0;
    
    mode=M_NORMAL;
    if(smod=="NORM") mode=M_NORMAL;
    if(smod=="OBL") mode=M_OBLIQUE;
    if(smod=="OBL_PHASE") mode=M_OBLIQUE_PHASE;
    if(smod=="EXTRAC") mode=M_EXTRAC;
    if(smod=="PLANAR_GUIDED") mode=M_PLANAR_GUIDED;
    if(smod=="CUSTOM") mode=M_CUSTOM;
    
    Npad=0;
    
    Nx_s=Nx; xs_s=0; xs_e=Nx;
    Ny_s=Ny; ys_s=0; ys_e=Ny;
    Nz_s=Nz; zs_s=0; zs_e=Nz;
    
    if(mode==M_NORMAL || mode==M_EXTRAC || mode==M_OBLIQUE_PHASE || mode==M_PLANAR_GUIDED)
    {
        if(pml_xm || pml_xp)
        {
            int pad=5;
            if(mode==M_PLANAR_GUIDED) pad=10;
            
            Nx+=pml_xm+pml_xp+2*pad;
            xs_s=pml_xm+pad;
            xs_e=xs_s+Nx_s;
        }
        if(pml_ym || pml_yp)
        {
            Ny+=pml_ym+pml_yp+2*5;
            ys_s=pml_ym+5;
            ys_e=ys_s+Ny_s;
        }
        if(pml_zm || pml_zp)
        {
            int pad=5;
            if(mode==M_EXTRAC || mode==M_PLANAR_GUIDED) pad=10;
            
            Nz+=pml_zm+pml_zp+2*pad;
            zs_s=pml_zm+pad;
            zs_e=zs_s+Nz_s;
        }
    }
    
    if(mode==M_CUSTOM)
    {
        int pad=10;
        
        if(pml_xm || pml_xp)
        {
            Nx+=pml_xm+pml_xp+2*pad;
            xs_s=pml_xm+pad;
            xs_e=xs_s+Nx_s;
        }
        if(pml_ym || pml_yp)
        {
            Ny+=pml_ym+pml_yp+2*pad;
            ys_s=pml_ym+pad;
            ys_e=ys_s+Ny_s;
        }
        if(pml_zm || pml_zp)
        {
            Nz+=pml_zm+pml_zp+2*pad;
            zs_s=pml_zm+pad;
            zs_e=zs_s+Nz_s;
        }
    }
    
    if(pml_xm || pml_xp) Plog::print("PML-x enabled: ", pml_xm, " ", pml_xp, "\n");
    if(pml_ym || pml_yp) Plog::print("PML-y enabled: ", pml_ym, " ", pml_yp, "\n");
    if(pml_zm || pml_zp) Plog::print("PML-z enabled: ", pml_zm, " ", pml_zp, "\n");
    if(mode==M_OBLIQUE) Plog::print("Oblique incidence requested, extending grid\n");
    Plog::print("New size: (", Nx, ",", Ny, ",", Nz, ") replacing (", Nx_, ",", Ny_, ",", Nz_, ")\n\n");
    
    alloc_DEBH();
    #ifndef SEP_MATS
    matsgrid.init(Nx,Ny,Nz,0);
    #else
    matsgrid_x.init(Nx,Ny,Nz,0);
    matsgrid_y.init(Nx,Ny,Nz,0);
    matsgrid_z.init(Nx,Ny,Nz,0);
    #endif
    
    basic_differentials_compute();
    
    //set_pml(pml_x,pml_y,pml_z);
    
    // Threads launching
    
    for(int i=0;i<Nthreads;i++)
    {
        threads_ready_E[i]=false;
        threads_ready_H[i]=false;
        
        threads_E[i]=new std::thread(&FDTD::threaded_process_E,this,i);
        threads_H[i]=new std::thread(&FDTD::threaded_process_H,this,i);
        
        while(!threads_ready_E[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while(!threads_ready_H[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

FDTD::FDTD(int Nx_,int Ny_,int Nz_,int Nt_,
           double Dx_,double Dy_,double Dz_,double Dt_,
           std::string smod,
           int pml_xm_,int pml_xp_,
           int pml_ym_,int pml_yp_,
           int pml_zm_,int pml_zp_,
           int pad_xm_,int pad_xp_,
           int pad_ym_,int pad_yp_,
           int pad_zm_,int pad_zp_)
    :tstep(0), Nx(Nx_), Ny(Ny_), Nz(Nz_), Nt(Nt_), Ntap(0), Nmat(1),
     Dx(Dx_), Dy(Dy_), Dz(Dz_), Dt(Dt_), fact(0),
     kx(0), ky(0),
     enable_Ex(true), enable_Ey(true), enable_Ez(true),
     enable_Hx(true), enable_Hy(true), enable_Hz(true),
     pad_xm(pad_xm_), pad_xp(pad_xp_),
     pad_ym(pad_ym_), pad_yp(pad_yp_),
     pad_zm(pad_zm_), pad_zp(pad_zp_),
     pml_xm(pml_xm_), pml_xp(pml_xp_),
     pml_ym(pml_ym_), pml_yp(pml_yp_),
     pml_zm(pml_zm_), pml_zp(pml_zp_),
     pml_kappa_xm(1.0), pml_kappa_xp(1.0),
     pml_kappa_ym(1.0), pml_kappa_yp(1.0),
     pml_kappa_zm(1.0), pml_kappa_zp(1.0),
     pml_sigma_xm(0), pml_sigma_xp(0),
     pml_sigma_ym(0), pml_sigma_yp(0),
     pml_sigma_zm(0), pml_sigma_zp(0),
     pml_alpha_xm(0), pml_alpha_xp(0),
     pml_alpha_ym(0), pml_alpha_yp(0),
     pml_alpha_zm(0), pml_alpha_zp(0),
     Nthreads(max_threads_number()),
     allow_run_E(false),
     alternator_E(Nthreads),
     threads_ready_E(Nthreads), threads_E(Nthreads),
     allow_run_H(false),
     alternator_H(Nthreads),
     threads_ready_H(Nthreads), threads_H(Nthreads)
{    
    dt_D_comp=0;
    dt_B_comp=0;
    
    mode=M_NORMAL;
    if(smod=="NORM") mode=M_NORMAL;
    if(smod=="OBL") mode=M_OBLIQUE;
    if(smod=="OBL_PHASE") mode=M_OBLIQUE_PHASE;
    if(smod=="EXTRAC") mode=M_EXTRAC;
    if(smod=="PLANAR_GUIDED") mode=M_PLANAR_GUIDED;
    if(smod=="CUSTOM") mode=M_CUSTOM;
    
    Npad=0;
    
    Nx_s=Nx; xs_s=0; xs_e=Nx;
    Ny_s=Ny; ys_s=0; ys_e=Ny;
    Nz_s=Nz; zs_s=0; zs_e=Nz;
    
    if(mode==M_NORMAL ||
       mode==M_CUSTOM ||
       mode==M_PLANAR_GUIDED)
    {
        if(pml_xm || pml_xp)
        {
            pad_xm=std::max(5,pad_xm);
            pad_xp=std::max(5,pad_xp);
            
            if(mode==M_PLANAR_GUIDED)
            {
                pad_xm=std::min(10,pad_xm);
                pad_xp=std::min(10,pad_xp);
            }
        }
        if(pml_ym || pml_yp)
        {
            pad_ym=std::max(5,pad_ym);
            pad_yp=std::max(5,pad_yp);
        }
        if(pml_zm || pml_zp)
        {
            pad_zm=std::max(5,pad_zm);
            pad_zp=std::max(5,pad_zp);
            
            if(mode==M_PLANAR_GUIDED)
            {
                pad_zm=std::max(10,pad_zm);
                pad_zp=std::max(10,pad_zp);
            }
        }
    }
    
    Nx+=pml_xm+pad_xm+pad_xp+pml_xp;
    Ny+=pml_ym+pad_ym+pad_yp+pml_yp;
    Nz+=pml_zm+pad_zm+pad_zp+pml_zp;
    
    xs_s=pml_xm+pad_xm; xs_e=xs_s+Nx_s;
    ys_s=pml_ym+pad_ym; ys_e=ys_s+Ny_s;
    zs_s=pml_zm+pad_zm; zs_e=zs_s+Nz_s;
    
    if(pml_xm || pml_xp) std::cout<<"PML-x enabled: "<<pml_xm<<" "<<pml_xp<<std::endl;
    if(pml_ym || pml_yp) std::cout<<"PML-y enabled: "<<pml_ym<<" "<<pml_yp<<std::endl;
    if(pml_zm || pml_zp) std::cout<<"PML-z enabled: "<<pml_zm<<" "<<pml_zp<<std::endl;
    if(mode==M_OBLIQUE) std::cout<<"Oblique incidence requested, extending grid"<<std::endl;
    std::cout<<"New size: ("<<Nx<<","<<Ny<<","<<Nz<<") replacing ("<<Nx_<<","<<Ny_<<","<<Nz_<<")"<<std::endl<<std::endl;
    
    alloc_DEBH();
    #ifndef SEP_MATS
    matsgrid.init(Nx,Ny,Nz,0);
    #else
    matsgrid_x.init(Nx,Ny,Nz,0);
    matsgrid_y.init(Nx,Ny,Nz,0);
    matsgrid_z.init(Nx,Ny,Nz,0);
    #endif
    
    basic_differentials_compute();
    
    // Threads launching
    
    for(int i=0;i<Nthreads;i++)
    {
        threads_ready_E[i]=false;
        threads_ready_H[i]=false;
        
        threads_E[i]=new std::thread(&FDTD::threaded_process_E,this,i);
        threads_H[i]=new std::thread(&FDTD::threaded_process_H,this,i);
        
        while(!threads_ready_E[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while(!threads_ready_H[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

FDTD::~FDTD()
{
    allow_run_E=false;
    allow_run_H=false;
    
    alternator_E.signal_threads();
    alternator_H.signal_threads();
    
    for(int i=0;i<Nthreads;i++)
    {
        threads_E[i]->join();
        threads_H[i]->join();
        
        delete threads_E[i];
        delete threads_H[i];
    }
}

//void FDTD::advEx(int i1,int i2)
//{
//    int i,j,k;
//    int j1,k1;
//    int j2,k2;
//    
//    double kappa_y,inv_kappa_y;
//    double kappa_z,inv_kappa_z;
//    int M;
//    double C1,C2y,C2z;
//    
//    for(j=0;j<Ny;j++)
//    {
//        j2=j;
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(j==0) j1=Ny-1;
//            else j1=j-1;
//        }
//        else
//        {
//            if(j==0) j1=Ny;
//            else j1=j-1;
//        }
//        
//        if(pml_ym || pml_xp) kappa_y=kappa_y_E[j];
//        else kappa_y=1.0;
//        
//        inv_kappa_y=1.0/kappa_y;
//        
//        for(k=0;k<Nz;k++)
//        {
//            k2=k;
//            if(k==0) k1=Nz-1;
//            else k1=k-1;
//            
//            if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
//            else kappa_z=1.0;
//            
//            inv_kappa_z=1.0/kappa_z;
//            
//            for(i=i1;i<i2;i++) //0 - Nx
//            {
//                #ifndef SEP_MATS
//                M=matsgrid(i,j,k);
//                #else
//                M=matsgrid_x(i,j,k);
//                #endif
//                
//                mats[M].coeffsX(C1,C2y,C2z);
//                
//                Ex(i,j,k)=C1*Ex(i,j,k)+C2y*inv_kappa_y*(Hz(i,j2,k)-Hz(i,j1,k))
//                                      -C2z*inv_kappa_z*(Hy(i,j,k2)-Hy(i,j,k1));
//            }
//        }
//    }
//}

void FDTD::advEx(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int j1,k1;
    int j2,k2;
    
    double kappa_y,inv_kappa_y;
    double kappa_z,inv_kappa_z;
    int M;
    double C1,C2y,C2z;
    
    for(k=k1_;k<k2_;k++)
    {
        k2=k;
        if(k==0) k1=Nz-1;
        else k1=k-1;
        
        if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
        else kappa_z=1.0;
        
        inv_kappa_z=1.0/kappa_z;
        
        for(j=j1_;j<j2_;j++)
        {
            j2=j;
            if(mode!=M_OBLIQUE_PHASE)
            {
                if(j==0) j1=Ny-1;
                else j1=j-1;
            }
            else
            {
                if(j==0) j1=Ny;
                else j1=j-1;
            }
            
            if(pml_ym || pml_xp) kappa_y=kappa_y_E[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/kappa_y;
            
            for(i=i1_;i<i2_;i++) //0 - Nx
            {
                #ifndef SEP_MATS
                M=matsgrid(i,j,k);
                #else
                M=matsgrid_x(i,j,k);
                #endif
                
                mats[M].coeffsX(C1,C2y,C2z);
                
                Ex(i,j,k)=C1*Ex(i,j,k)+C2y*inv_kappa_y*(Hz(i,j2,k)-Hz(i,j1,k))
                                      -C2z*inv_kappa_z*(Hy(i,j,k2)-Hy(i,j,k1));
            }
        }
    }
}

//void FDTD::advEy(int j1,int j2)
//{
//    int i,j,k;
//    int i1,i2,k1,k2;
//    int M;
//    //double x,y,z,tb;
//    double kappa_x,inv_kappa_x;
//    double kappa_z,inv_kappa_z;
//    double C1,C2x,C2z;
//    
//    for(i=0;i<Nx;i++)
//    {
//        i2=i;
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(i==0) i1=Nx-1;
//            else i1=i-1;
//        }
//        else
//        {
//            if(i==0) i1=Nx;
//            else i1=i-1;
//        }
//        
//        if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
//        else kappa_x=1.0;
//        
//        inv_kappa_x=1.0/kappa_x;
//        
//        for(k=0;k<Nz;k++)
//        {
//            k2=k;
//            if(k==0) k1=Nz-1;
//            else k1=k-1;
//            
//            if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
//            else kappa_z=1.0;
//            
//            inv_kappa_z=1.0/kappa_z;
//            
//            for(j=j1;j<j2;j++) //0 - Ny
//            {
//                #ifndef SEP_MATS
//                M=matsgrid(i,j,k);
//                #else
//                M=matsgrid_y(i,j,k);
//                #endif
//                
//                mats[M].coeffsY(C1,C2x,C2z);
//                
//                Ey(i,j,k)=C1*Ey(i,j,k)+C2z*inv_kappa_z*(Hx(i,j,k2)-Hx(i,j,k1))
//                                      -C2x*inv_kappa_x*(Hz(i2,j,k)-Hz(i1,j,k));
//            }
//        }
//    }
//}

void FDTD::advEy(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int i1,i2,k1,k2;
    int M;
    //double x,y,z,tb;
    double kappa_x,inv_kappa_x;
    double kappa_z,inv_kappa_z;
    double C1,C2x,C2z;
    
    for(k=k1_;k<k2_;k++)
    {
        k2=k;
        if(k==0) k1=Nz-1;
        else k1=k-1;
        
        if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
        else kappa_z=1.0;
        
        inv_kappa_z=1.0/kappa_z;
            
        for(j=j1_;j<j2_;j++) //0 - Ny
        {
            for(i=i1_;i<i2_;i++)
            {
                i2=i;
                if(mode!=M_OBLIQUE_PHASE)
                {
                    if(i==0) i1=Nx-1;
                    else i1=i-1;
                }
                else
                {
                    if(i==0) i1=Nx;
                    else i1=i-1;
                }
                
                if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
                else kappa_x=1.0;
                
                inv_kappa_x=1.0/kappa_x;
                
                #ifndef SEP_MATS
                M=matsgrid(i,j,k);
                #else
                M=matsgrid_y(i,j,k);
                #endif
                
                mats[M].coeffsY(C1,C2x,C2z);
                
                Ey(i,j,k)=C1*Ey(i,j,k)+C2z*inv_kappa_z*(Hx(i,j,k2)-Hx(i,j,k1))
                                      -C2x*inv_kappa_x*(Hz(i2,j,k)-Hz(i1,j,k));
            }
        }
    }
}

//void FDTD::advEz(int k1,int k2)
//{
//    int i,j,k;
//    int i1,i2;
//    int j1,j2;
//    int M;
//    double kappa_x,inv_kappa_x;
//    double kappa_y,inv_kappa_y;
//    double C1,C2x,C2y;
//    
//    for(i=0;i<Nx;i++)
//    {
//        i2=i;
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(i==0) i1=Nx-1;
//            else i1=i-1;
//        }
//        else
//        {
//            if(i==0) i1=Nx;
//            else i1=i-1;
//        }
//        
//        if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
//        else kappa_x=1.0;
//        
//        inv_kappa_x=1.0/kappa_x;
//        
//        for(j=0;j<Ny;j++)
//        {
//            j2=j;
//            if(mode!=M_OBLIQUE_PHASE)
//            {
//                if(j==0) j1=Ny-1;
//                else j1=j-1;
//            }
//            else
//            {
//                if(j==0) j1=Ny;
//                else j1=j-1;
//            }
//            
//            if(pml_ym || pml_xp) kappa_y=kappa_y_E[j];
//            else kappa_y=1.0;
//            
//            inv_kappa_y=1.0/kappa_y;
//            
//            for(k=k1;k<k2;k++) //0 - Nz
//            {
//                #ifndef SEP_MATS
//                M=matsgrid(i,j,k);
//                #else
//                M=matsgrid_z(i,j,k);
//                #endif
//                
//                mats[M].coeffsZ(C1,C2x,C2y);
//                                
//                Ez(i,j,k)=C1*Ez(i,j,k)+C2x*inv_kappa_x*(Hy(i2,j,k)-Hy(i1,j,k))
//                                      -C2y*inv_kappa_y*(Hx(i,j2,k)-Hx(i,j1,k));
//            }
//        }
//    }
//}

void FDTD::advEz(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int i1,i2;
    int j1,j2;
    int M;
    double kappa_x,inv_kappa_x;
    double kappa_y,inv_kappa_y;
    double C1,C2x,C2y;
        
    for(k=k1_;k<k2_;k++) //0 - Nz
    {
        for(j=j1_;j<j2_;j++)
        {
            j2=j;
            if(mode!=M_OBLIQUE_PHASE)
            {
                if(j==0) j1=Ny-1;
                else j1=j-1;
            }
            else
            {
                if(j==0) j1=Ny;
                else j1=j-1;
            }
            
            if(pml_ym || pml_xp) kappa_y=kappa_y_E[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/kappa_y;
            
            for(i=i1_;i<i2_;i++)
            {
                i2=i;
                if(mode!=M_OBLIQUE_PHASE)
                {
                    if(i==0) i1=Nx-1;
                    else i1=i-1;
                }
                else
                {
                    if(i==0) i1=Nx;
                    else i1=i-1;
                }
                
                if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
                else kappa_x=1.0;
                
                inv_kappa_x=1.0/kappa_x;
            
                #ifndef SEP_MATS
                M=matsgrid(i,j,k);
                #else
                M=matsgrid_z(i,j,k);
                #endif
                
                mats[M].coeffsZ(C1,C2x,C2y);
                                
                Ez(i,j,k)=C1*Ez(i,j,k)+C2x*inv_kappa_x*(Hy(i2,j,k)-Hy(i1,j,k))
                                      -C2y*inv_kappa_y*(Hx(i,j2,k)-Hx(i,j1,k));
            }
        }
    }
}

//void FDTD::advHx(int i1,int i2)
//{
//    int i,j,k;
//    int j1,j2;
//    int k1,k2;
//    
//    double kappa_y,inv_kappa_y;
//    double kappa_z,inv_kappa_z;
//    
//    for(j=0;j<Ny;j++)
//    {
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(j==Ny-1) j2=0;
//            else j2=j+1;
//        }
//        else
//        {
//            if(j==Ny-1) j2=Ny;
//            else j2=j+1;
//        }
//        j1=j;
//        
//        if(pml_ym || pml_xp) kappa_y=kappa_y_H[j];
//        else kappa_y=1.0;
//        
//        inv_kappa_y=1.0/kappa_y;
//        
//        for(k=0;k<Nz;k++)
//        {
//            if(k==Nz-1) k2=0;
//            else k2=k+1;
//            k1=k;
//            
//            if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
//            else kappa_z=1.0;
//            
//            inv_kappa_z=1.0/kappa_z;
//            
//            for(i=i1;i<i2;i++) //0 - Nx
//            {
//                Hx(i,j,k)+=dtdmz*inv_kappa_z*(Ey(i,j,k2)-Ey(i,j,k1))
//                          -dtdmy*inv_kappa_y*(Ez(i,j2,k)-Ez(i,j1,k));
//            }
//        }
//    }
//}



void FDTD::advHx(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int j1,j2;
    int k1,k2;
    
    double kappa_y,inv_kappa_y;
    double kappa_z,inv_kappa_z;
        
    for(k=k1_;k<k2_;k++)
    {
        if(k==Nz-1) k2=0;
        else k2=k+1;
        k1=k;
        
        if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
        else kappa_z=1.0;
        
        inv_kappa_z=1.0/kappa_z;
    
        for(j=j1_;j<j2_;j++)
        {
            if(mode!=M_OBLIQUE_PHASE)
            {
                if(j==Ny-1) j2=0;
                else j2=j+1;
            }
            else
            {
                if(j==Ny-1) j2=Ny;
                else j2=j+1;
            }
            j1=j;
            
            if(pml_ym || pml_xp) kappa_y=kappa_y_H[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/kappa_y;
            
            for(i=i1_;i<i2_;i++) //0 - Nx
            {
                Hx(i,j,k)+=dtdmz*inv_kappa_z*(Ey(i,j,k2)-Ey(i,j,k1))
                          -dtdmy*inv_kappa_y*(Ez(i,j2,k)-Ez(i,j1,k));
            }
        }
    }
}

//void FDTD::advHy(int j1,int j2)
//{
//    int i,j,k;
//    int i1,i2;
//    int k1,k2;
//    
//    double kappa_x,inv_kappa_x;
//    double kappa_z,inv_kappa_z;
//        
//    for(i=0;i<Nx;i++)
//    {
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(i==Nx-1) i2=0;
//            else i2=i+1;
//        }
//        else
//        {
//            if(i==Nx-1) i2=Nx;
//            else i2=i+1;
//        }
//        i1=i;
//        
//        if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
//        else kappa_x=1.0;
//        
//        inv_kappa_x=1.0/kappa_x;
//        
//        for(k=0;k<Nz;k++)
//        {
//            if(k==Nz-1) k2=0;
//            else k2=k+1;
//            k1=k;
//            
//            if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
//            else kappa_z=1.0;
//            
//            inv_kappa_z=1.0/kappa_z;
//            
//            for(j=j1;j<j2;j++)
//            {
//                Hy(i,j,k)+=dtdmx*inv_kappa_x*(Ez(i2,j,k)-Ez(i1,j,k))
//                          -dtdmz*inv_kappa_z*(Ex(i,j,k2)-Ex(i,j,k1));
//            }
//        }
//    }
//}

void FDTD::advHy(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int i1,i2;
    int k1,k2;
    
    double kappa_x,inv_kappa_x;
    double kappa_z,inv_kappa_z;
        
    for(k=k1_;k<k2_;k++)
    {
        if(k==Nz-1) k2=0;
        else k2=k+1;
        k1=k;
        
        if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
        else kappa_z=1.0;
        
        inv_kappa_z=1.0/kappa_z;
            
        for(j=j1_;j<j2_;j++)
        {
            for(i=i1_;i<i2_;i++)
            {
                if(mode!=M_OBLIQUE_PHASE)
                {
                    if(i==Nx-1) i2=0;
                    else i2=i+1;
                }
                else
                {
                    if(i==Nx-1) i2=Nx;
                    else i2=i+1;
                }
                i1=i;
                
                if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
                else kappa_x=1.0;
                
                inv_kappa_x=1.0/kappa_x;
                
                Hy(i,j,k)+=dtdmx*inv_kappa_x*(Ez(i2,j,k)-Ez(i1,j,k))
                          -dtdmz*inv_kappa_z*(Ex(i,j,k2)-Ex(i,j,k1));
            }
        }
    }
}

//void FDTD::advHz(int k1,int k2)
//{
//    int i,j,k;
//    int i1,i2;
//    int j1,j2;
//    
//    double kappa_x,inv_kappa_x;
//    double kappa_y,inv_kappa_y;
//    
//    for(i=0;i<Nx;i++)
//    {
//        if(mode!=M_OBLIQUE_PHASE)
//        {
//            if(i==Nx-1) i2=0;
//            else i2=i+1;
//        }
//        else
//        {
//            if(i==Nx-1) i2=Nx;
//            else i2=i+1;
//        }
//        i1=i;
//        
//        if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
//        else kappa_x=1.0;
//        
//        inv_kappa_x=1.0/kappa_x;
//        
//        for(j=0;j<Ny;j++)
//        {
//            if(mode!=M_OBLIQUE_PHASE)
//            {
//                if(j==Ny-1) j2=0;
//                else j2=j+1;
//            }
//            else
//            {
//                if(j==Ny-1) j2=Ny;
//                else j2=j+1;
//            }
//            j1=j;
//            
//            if(pml_ym || pml_xp) kappa_y=kappa_y_H[j];
//            else kappa_y=1.0;
//            
//            inv_kappa_y=1.0/kappa_y;
//            
//            for(k=k1;k<k2;k++)
//            {
//                Hz(i,j,k)+=dtdmy*inv_kappa_y*(Ex(i,j2,k)-Ex(i,j1,k))
//                          -dtdmx*inv_kappa_x*(Ey(i2,j,k)-Ey(i1,j,k));
//            }
//        }
//    }
//}

void FDTD::advHz(int i1_,int i2_,int j1_,int j2_,int k1_,int k2_)
{
    int i,j,k;
    int i1,i2;
    int j1,j2;
    
    double kappa_x,inv_kappa_x;
    double kappa_y,inv_kappa_y;
            
    for(k=k1_;k<k2_;k++)
    {
        for(j=j1_;j<j2_;j++)
        {
            if(mode!=M_OBLIQUE_PHASE)
            {
                if(j==Ny-1) j2=0;
                else j2=j+1;
            }
            else
            {
                if(j==Ny-1) j2=Ny;
                else j2=j+1;
            }
            j1=j;
            
            if(pml_ym || pml_xp) kappa_y=kappa_y_H[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/kappa_y;
            
            for(i=i1_;i<i2_;i++)
            {
                if(mode!=M_OBLIQUE_PHASE)
                {
                    if(i==Nx-1) i2=0;
                    else i2=i+1;
                }
                else
                {
                    if(i==Nx-1) i2=Nx;
                    else i2=i+1;
                }
                i1=i;
                
                if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
                else kappa_x=1.0;
                
                inv_kappa_x=1.0/kappa_x;
                
                Hz(i,j,k)+=dtdmy*inv_kappa_y*(Ex(i,j2,k)-Ex(i,j1,k))
                          -dtdmx*inv_kappa_x*(Ey(i2,j,k)-Ey(i1,j,k));
            }
        }
    }
}

void FDTD::advMats_ante(int i1,int i2)
{
    int i,j,k;
    int M;
    
    for(k=0;k<Nz;k++)
    {
        for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++) //0 - Nx
            {
                M=matsgrid(i,j,k);
                
                mats[M].ante_compute(i,j,k,Ex,Ey,Ez);
            }
        }
    }
    
    if(tstep==0)
    {
        for(j=0;j<Ny;j++)
        {
            for(k=0;k<Nz;k++)
            {
                for(i=i1;i<i2;i++) //0 - Nx
                {
                    M=matsgrid(i,j,k);
                    
                    if(mats[M].m_type==MAT_NAGRA_2LVL)
                    {
//                        Psi(i,j,k,12)=1e26;
                    }
                    if(mats[M].m_type==MAT_GAIN_NAGRA)
                    {
//                        Psi(i,j,k,14)=1e19;
//                        Psi(i,j,k,13)=1.36e22;
                    }
                }
            }
        }
    }
}

void FDTD::advMats_simp(int i1,int i2)
{
    int i,j,k;
    int M;
    
    for(k=0;k<Nz;k++)
    {
        for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++) //0 - Nx
            {
                M=matsgrid(i,j,k);
                
                mats[M].apply_E(i,j,k,Ex,0);
                mats[M].apply_E(i,j,k,Ey,1);
                mats[M].apply_E(i,j,k,Ez,2);
            }
        }
    }
}

void FDTD::advMats_post(int i1,int i2)
{
    int i,j,k;
    int M;
    
    for(k=0;k<Nz;k++)
    {
        for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++) //0 - Nx
            {
                M=matsgrid(i,j,k);
                
                mats[M].post_compute(i,j,k,Ex,Ey,Ez);
            }
        }
    }
}

void FDTD::advMats_self(int i1,int i2)
{
    int i,j,k;
    int M;
    
    for(k=0;k<Nz;k++)
    {
        for(j=0;j<Ny;j++)
        {
            for(i=i1;i<i2;i++) //0 - Nx
            {
                M=matsgrid(i,j,k);
                
                mats[M].self_compute(i,j,k,Ex,Ey,Ez);
            }
        }
    }
}

void FDTD::alloc_DEBH()
{
    int aNx=Nx;
    int aNy=Ny;
    int aNz=Nz;
    
    if(mode==M_OBLIQUE_PHASE)
    {
        aNx+=1;
        aNy+=1;
    }
    
    Ex.init(aNx,aNy,aNz,0);
    Ey.init(aNx,aNy,aNz,0);
    Ez.init(aNx,aNy,aNz,0);
    Hx.init(aNx,aNy,aNz,0);
    Hy.init(aNx,aNy,aNz,0);
    Hz.init(aNx,aNy,aNz,0);
    
    bootstrap();
}

void FDTD::bootstrap()
{
    allocate_pml();
    pml_coeff_calc();
    
    int aNx=Nx;
    int aNy=Ny;
    int aNz=Nz;
    
    if(mode==M_OBLIQUE_PHASE)
    {
        aNx+=1;
        aNy+=1;
    }
    
    if(dt_D_comp==true)
    {
        dt_Dx.init(aNx,aNy,aNz,0);
        dt_Dy.init(aNx,aNy,aNz,0);
        dt_Dz.init(aNx,aNy,aNz,0);
    }
    else
    {
        dt_Dx.init(1,1,1,0);
        dt_Dy.init(1,1,1,0);
        dt_Dz.init(1,1,1,0);
    }
    
    if(dt_B_comp==true)
    {
        dt_Bx.init(aNx,aNy,aNz,0);
        dt_By.init(aNx,aNy,aNz,0);
        dt_Bz.init(aNx,aNy,aNz,0);
    }
    else
    {
        dt_Bx.init(1,1,1,0);
        dt_By.init(1,1,1,0);
        dt_Bz.init(1,1,1,0);
    }
}

void FDTD::basic_differentials_compute()
{
    dte=Dt/e0;
    dtm=Dt/mu0;
    
    dtdex=Dt/(Dx*e0);
    dtdey=Dt/(Dy*e0);
    dtdez=Dt/(Dz*e0);
    dtdmx=Dt/(Dx*mu0);
    dtdmy=Dt/(Dy*mu0);
    dtdmz=Dt/(Dz*mu0);
}

void FDTD::disable_fields(std::vector<int> const &fields)
{
    for(unsigned int i=0;i<fields.size();i++)
    {
             if(fields[i]==EX_FIELD) enable_Ex=false;
        else if(fields[i]==EY_FIELD) enable_Ey=false;
        else if(fields[i]==EZ_FIELD) enable_Ez=false;
        else if(fields[i]==HX_FIELD) enable_Hx=false;
        else if(fields[i]==HY_FIELD) enable_Hy=false;
        else if(fields[i]==HZ_FIELD) enable_Hz=false;
    }
}

void FDTD::update_E()
{
    if(tstep==0) pml_coeff_calc();
    
    std::unique_lock<std::mutex> lock(alternator_E.get_main_mutex());
    
    allow_run_E=true;
    
    // Materials Ante
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // E Field
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    // Materials Simp
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // Materials Post
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // Materials Self
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // PMLS
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    allow_run_E=false;
    
//    update_mats_ante();
//    
//    if(!dt_D_comp)
//    {
//        update_E_simp();
//        update_mats_simp();
//    }
//    else
//    {
//        update_E_ext();
//        update_mats_ext();
//    }
//    
//    update_mats_post();
//    
//    apply_pml_E();
}

void FDTD::update_E_ante()
{
    if(tstep==0) pml_coeff_calc();
    
    std::unique_lock<std::mutex> lock(alternator_E.get_main_mutex());
    
    allow_run_E=true;
    
    // Materials Ante
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
}

void FDTD::update_E_self()
{
    std::unique_lock<std::mutex> lock(alternator_E.get_main_mutex());
    
    // E Field
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
}

void FDTD::update_E_post()
{
    std::unique_lock<std::mutex> lock(alternator_E.get_main_mutex());
    
    // Materials Simp
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // Materials Post
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // Materials Self
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    // PMLS
    
    alternator_E.signal_threads();
    alternator_E.main_wait_threads(lock);
    
    allow_run_E=false;
}

void FDTD::update_E_ext()
{
//    int i;
//    
//    pthread_t tEx[Nthreads],tEy[Nthreads],tEz[Nthreads];
//    void* status;
//    
//    Tparam PEx[Nthreads];
//    for(i=0;i<Nthreads;i++) PEx[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&tEx[i],0,&TAdv_dt_Dx,&PEx[i]);
//    
//    //
//    
//    Tparam PEy[Nthreads];
//    for(i=0;i<Nthreads;i++) PEy[i].set(this,(i*Ny)/Nthreads,((i+1)*Ny)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&tEy[i],0,&TAdv_dt_Dy,&PEy[i]);
//    
//    //
//    
//    Tparam PEz[Nthreads];
//    for(i=0;i<Nthreads;i++) PEz[i].set(this,(i*Nz)/Nthreads,((i+1)*Nz)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&tEz[i],0,&TAdv_dt_Dz,&PEz[i]);
//    
//    for(i=0;i<Nthreads;i++) pthread_join(tEx[i],&status);
//    for(i=0;i<Nthreads;i++) pthread_join(tEy[i],&status);
//    for(i=0;i<Nthreads;i++) pthread_join(tEz[i],&status);
}

//void FDTD::update_H()
//{
//    if(!dt_B_comp)
//    {
//        update_H_simp();
//    }
//    else
//    {
//        //update_H_ext();
//    }
//    
//    apply_pml_H();
//    
//    tstep+=1;
//    fact=1.0-std::exp(-std::pow(tstep/99.0,2.0));
//}

void FDTD::update_H()
{
    std::unique_lock<std::mutex> lock(alternator_H.get_main_mutex());
    
    allow_run_H=true;
        
    // H Field
    
    alternator_H.signal_threads();
    alternator_H.main_wait_threads(lock);
    
    // PMLS
    
    alternator_H.signal_threads();
    alternator_H.main_wait_threads(lock);
    
    allow_run_H=false;
    
    tstep+=1;
}

void FDTD::update_H_ext()
{
}

void FDTD::update_mats_ante()
{
//    int i;
//    
//    //Ante compute
//    
//    pthread_t thr[Nthreads];
//    void* status;
//    
//    Tparam Pthr[Nthreads];
//    for(i=0;i<Nthreads;i++) Pthr[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&thr[i],0,&TAdvMats_ante,&Pthr[i]);
//        
//    for(i=0;i<Nthreads;i++) pthread_join(thr[i],&status);
}

void FDTD::update_mats_simp()
{
//    int i;
//    
//    //Applies mats in simple configuration
//    
//    pthread_t thr[Nthreads];
//    void* status;
//    
//    Tparam Pthr[Nthreads];
//    for(i=0;i<Nthreads;i++) Pthr[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&thr[i],0,&TAdvMats_simp,&Pthr[i]);
//        
//    for(i=0;i<Nthreads;i++) pthread_join(thr[i],&status);
}

void FDTD::update_mats_ext()
{
//    int i;
//    
//    //Applies mats in D configuration
//    
//    pthread_t thr[Nthreads];
//    void* status;
//    
//    Tparam Pthr[Nthreads];
//    for(i=0;i<Nthreads;i++) Pthr[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&thr[i],0,&TAdvMats_ext,&Pthr[i]);
//        
//    for(i=0;i<Nthreads;i++) pthread_join(thr[i],&status);
}

void FDTD::update_mats_post()
{
//    int i;
//    
//    //Post compute
//    
//    pthread_t thr1[Nthreads];
//    void* status;
//    
//    Tparam Pthr1[Nthreads];
//    for(i=0;i<Nthreads;i++) Pthr1[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&thr1[i],0,&TAdvMats_post,&Pthr1[i]);
//        
//    for(i=0;i<Nthreads;i++) pthread_join(thr1[i],&status);
//    
//    //Self computation
//    
//    pthread_t thr2[Nthreads];
//    
//    Tparam Pthr2[Nthreads];
//    for(i=0;i<Nthreads;i++) Pthr2[i].set(this,(i*Nx)/Nthreads,((i+1)*Nx)/Nthreads);
//    for(i=0;i<Nthreads;i++) pthread_create(&thr2[i],0,&TAdvMats_self,&Pthr2[i]);
//        
//    for(i=0;i<Nthreads;i++) pthread_join(thr2[i],&status);
}
