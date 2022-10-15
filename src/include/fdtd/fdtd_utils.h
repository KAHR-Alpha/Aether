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

#ifndef FDTD_UTILS_H
#define FDTD_UTILS_H

#include <mathUT.h>
#include <em_grid.h>

#include <fstream>
#include <iomanip>
#include <list>
#include <vector>

#include <thread_utils.h>

#include <phys_constants.h>

//#define TESTLAB
//#define MOV
#define PCRC2

void testlab();
void scinti();

template<typename T>
void grid_extend(Grid3<T> &G,int &Nx,int &Ny,int &Nz,
                 int Px1,int Px2,int Py1,int Py2,int Pz1,int Pz2)
{
    int i,j,k;
    
    int Nx2=Nx+Px1+Px2;
    int Ny2=Ny+Py1+Py2;
    int Nz2=Nz+Pz1+Pz2;
    
    Grid3<T> tmp_grid(Nx2,Ny2,Nz2,T(0));
    
    // Base
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        tmp_grid(i+Px1,j+Py1,k+Pz1)=G(i,j,k);
    }}}
    
    // X - Extend
    
    for(i=0;i<Px1;i++){ for(j=0;j<Ny2;j++){ for(k=0;k<Nz2;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(Px1,j,k);
    }}}
    for(i=Px1+Nx;i<Nx2;i++){ for(j=0;j<Ny2;j++){ for(k=0;k<Nz2;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(Px1+Nx-1,j,k);
    }}}
    
    // Y - Extend
    
    for(i=0;i<Nx2;i++){ for(j=0;j<Py1;j++){ for(k=0;k<Nz2;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(i,Py1,k);
    }}}
    for(i=0;i<Nx2;i++){ for(j=Py1+Ny;j<Ny2;j++){ for(k=0;k<Nz2;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(i,Py1+Ny-1,k);
    }}}
    
    // Z - Extend
    
    for(i=0;i<Nx2;i++){ for(j=0;j<Ny2;j++){ for(k=0;k<Pz1;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(i,j,Pz1);
    }}}
    for(i=0;i<Nx2;i++){ for(j=0;j<Ny2;j++){ for(k=Pz1+Nz;k<Nz2;k++)
    {
        tmp_grid(i,j,k)=tmp_grid(i,j,Pz1+Nz-1);
    }}}
    
    Nx=Nx2;
    Ny=Ny2;
    Nz=Nz2;
    
    G.init(Nx,Ny,Nz,0);
    G=tmp_grid;
}

enum
{
    M_NORMAL,
    M_OBLIQUE,
    M_OBLIQUE_PHASE,
    M_EXTRAC,
    M_PLANAR_GUIDED,
    M_CUSTOM,
    MAT_ATOM_LEVEL,
    MAT_CONST,
    MAT_CONST_IM,
    MAT_DIELEC,
    MAT_RC,
    MAT_PCRC2,
    MAT_ANI_DIAG_CONST,
    MAT_ANI_FULL_CONST,
    MAT_NAGRA_2LVL,
    MAT_GAIN_NAGRA,
    MAT_GAIN_SPE,
    MAT_UNDEFINED
};

class ChpIn
{
    public:
        int Nthreads;
        double lambda,omega,inc,pol,dir,k,kx,ky,kz,n_ind;
        double dz,startx,starty,startz;
        double Ex0,Ey0,Ez0,Hx0,Hy0,Hz0;
        double C1;
        
        int Nl;
        double dphas;
        Grid1<double> g_Lambda,g_w,g_ang;
        Grid1<double> g_k,g_kx,g_ky,g_kz;
        Grid1<double> g_Ex0,g_Ey0,g_Ez0;
        Grid1<double> g_Hx0,g_Hy0,g_Hz0;
        
        double pcw0,pcdw;
        Imdouble pcEx,pcEy,pcEz,pcHx,pcHy,pcHz;
        
        Grid1<double> pc_w,pc_kn,pc_sp;
        Grid1<Imdouble> pc_srt;
        
        ChpIn();
        
        double (ChpIn::*pEx)(double,double,double,double);
        double (ChpIn::*pEy)(double,double,double,double);
        double (ChpIn::*pEz)(double,double,double,double);
        double (ChpIn::*pHx)(double,double,double,double);
        double (ChpIn::*pHy)(double,double,double,double);
        double (ChpIn::*pHz)(double,double,double,double);
        
        double Ex(double,double,double,double);
        double Ey(double,double,double,double);
        double Ez(double,double,double,double);
        double Hx(double,double,double,double);
        double Hy(double,double,double,double);
        double Hz(double,double,double,double);
        
        double PulseEx(double,double,double,double);
        double PulseEy(double,double,double,double);
        double PulseEz(double,double,double,double);
        double PulseHx(double,double,double,double);
        double PulseHy(double,double,double,double);
        double PulseHz(double,double,double,double);
        
        double PhaseEx(double,double,double,double);
        double PhaseEy(double,double,double,double);
        double PhaseEz(double,double,double,double);
        double PhaseHx(double,double,double,double);
        double PhaseHy(double,double,double,double);
        double PhaseHz(double,double,double,double);
        
        double PhaseEx_D(double,double,double,double);
        double PhaseEy_D(double,double,double,double);
        double PhaseEz_D(double,double,double,double);
        double PhaseHx_D(double,double,double,double);
        double PhaseHy_D(double,double,double,double);
        double PhaseHz_D(double,double,double,double);
        
        void init_phase(double li,double lf,double kx,double pol,double phase,double n_ind);
        void phase_precomp_E(double z,double t);
        void phase_precomp_Ex(double z,double t);
        void phase_precomp_Ey(double z,double t);
        void phase_precomp_Ez(double z,double t);
        void phase_precomp_H(double z,double t);
        void phase_reset_E();
        void phase_reset_H();
        void init_pulse(double li,double lf,double inc,double pol,
                        double dir,double startx,double starty,double startz,double n_ind);
        void setfields();
        void show();
        
        class PPE_params
        {
            public:
                ChpIn *inst;
                int l1,l2;
                double z,t;
                Imdouble t_pcEx,t_pcEy,t_pcEz;
                
                void set(ChpIn *inst_i,int l1_i,int l2_i,double z_i,double t_i)
                {
                    inst=inst_i;
                    l1=l1_i; l2=l2_i;
                    z=z_i;   t=t_i;
                }
                
                void operator () ()
                {
                    inst->phase_precomp_E_aux(*this);
                }
        };
        
        class PPH_params
        {
            public:
                ChpIn *inst;
                int l1,l2;
                double z,t;
                Imdouble t_pcHx,t_pcHy,t_pcHz;
                
                void set(ChpIn *inst_i,int l1_i,int l2_i,double z_i,double t_i)
                {
                    inst=inst_i;
                    l1=l1_i; l2=l2_i;
                    z=z_i;   t=t_i;
                }
                
                void operator () ()
                {
                    inst->phase_precomp_H_aux(*this);
                }
        };
        
        void phase_precomp_E_aux(PPE_params &);
        void phase_precomp_H_aux(PPH_params &);
};

#endif // FDTD_UTILS_H
