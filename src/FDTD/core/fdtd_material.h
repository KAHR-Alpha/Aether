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

#ifndef FDTD_MATERIAL_H_INCLUDED
#define FDTD_MATERIAL_H_INCLUDED

#include <material.h>


/*

0 Ex
1 Ey
2 Ez
3 Px n+1
4 Px n
5 Px n-1
6 Py n+1
7 Py n
8 Py n-1
9 Pz n+1
10 Pz n
11 Pz n-1
12 DN
*/

class FDTD_Material
{
    public:
        Material base_mat;
        
        int m_type;
        int Np,Np_r,Np_c;
        double Dx,Dy,Dz,Dt;
        double C1,C2,C2x,C2y,C2z,C3,C4;
        double ei,sig;
        
        bool comp_simp;
        bool comp_ante;
        bool comp_self;
        bool comp_post;
        bool comp_D;
        
        int Nx,Ny,Nz;
        int x_span,y_span,z_span;
        int x1,x2,y1,y2,z1,z2;
        
        Grid3<bool> mat_present;
        
        Grid4<double> m_Psi;
        Grid4<Imdouble> m_Psi_c;
        
        //NAGRA - 2LVL
        
        double n2l_wa,n2l_Dwa,n2l_T21,n2l_DN0,n2l_kappa;
        double n2l_P0,n2l_P1,n2l_P2,n2l_P3,n2l_N0,n2l_N1,n2l_N2,n2l_N3;
        
        //NAGRA - 4LVL
        
        double n4l_wa,n4l_Dwa,n4l_kappa,n4l_Wp;
        double n4l_P0,n4l_P1,n4l_P2,n4l_P3;
        double n4l_N30,n4l_N31,
               n4l_N20,n4l_N21,n4l_N22,n4l_N23,
               n4l_N10,n4l_N11,n4l_N12,n4l_N13,n4l_N14;
        
        FDTD_Material();
        //FDTD_Material(int m_type,double Dx,double Dy,double Dz,double Dt);
        
        void clean_fields();
        void coeffsX(double &C1o,double &C2yo,double &C2zo);
        void coeffsY(double &C1o,double &C2xo,double &C2zo);
        void coeffsZ(double &C1o,double &C2xo,double &C2yo);
        Imdouble get_n(double w);
        //void init(int m_type,double Dx,double Dy,double Dz,double Dt);
        double pml_coeff();
        void compute_close(int i,int j,int k,int dir,
                           Grid3<double> &E,
                           Grid4<double> &Psi,
                           Grid4<Imdouble> &Psi_c);
        double compute_open(int i,int j,int k,int dir,
                            Grid3<double> &E,
                            Grid4<double> &Psi,
                            Grid4<Imdouble> &Psi_c);
        void compute_self(int i,int j,int k,
                          Grid3<double> &Ex,
                          Grid3<double> &Ey,
                          Grid3<double> &Ez,
                          Grid4<double> &Psi,
                          Grid4<Imdouble> &Psi_c);
        
        void set_base_mat(Material const &material);
        void set_mem_depth(int Np,int Np_r,int Np_c);
        void realloc();
        double report_size();
        //void recalc();
        void show();
        
        bool needs_D_field();
        void link_fdtd(double Dx,double Dy,double Dz,double Dt);
        void link_grid(Grid3<unsigned int> const &mat_grid,unsigned int ID);
        
        void operator = (double);
        void operator = (FDTD_Material const&);
        
        void ante_compute(int i,int j,int k,
                          Grid3<double> const &Ex,
                          Grid3<double> const &Ey,
                          Grid3<double> const &Ez);
        void post_compute(int i,int j,int k,
                          Grid3<double> const &Ex,
                          Grid3<double> const &Ey,
                          Grid3<double> const &Ez);
        void self_compute(int i,int j,int k,
                          Grid3<double> const &Ex,
                          Grid3<double> const &Ey,
                          Grid3<double> const &Ez);
        
        void apply_E(int i,int j,int k,Grid3<double> &E,int dir);
        void apply_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                       Grid3<double> const &Dx,
                       Grid3<double> const &Dy,
                       Grid3<double> const &Dz);
        
        //###############
        // Dielec Model
        //###############
                        
        //RC - PCRC2
        
        Grid1<Imdouble> Crec,Dchi,chi,chi_h;
        
        Dielec_model dielec;
        
        //###############
        //    Const
        //###############
        
        void set_const(double);
        //void set_const_i(Imdouble,double);
        void const_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                       Grid3<double> const &Dx,
                       Grid3<double> const &Dy,
                       Grid3<double> const &Dz);
        void const_recalc();
        
        //####################
        //   Ani Diag Const
        //####################
        
        double ADC_ex,ADC_ey,ADC_ez;
        
        void set_ani_DC(double eps_x,double eps_y,double eps_z);
        void ani_DC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                        Grid3<double> const &Dx,
                        Grid3<double> const &Dy,
                        Grid3<double> const &Dz);
        void ani_DC_recalc();
        
        //###############
        //   Ani Const
        //###############
        
        //########
        //   RC
        //########
        
        /*void setdebye(double,double,double);
        void setdrude(double,double,double);
        void setlorentz(double,double,double,double);
        void setdrudelorentz(double,double,double,double,double,double);
        void setdrude2lorentz(double,double,double,double,double,double,double,double,double);
        void setdrude2cp(double,double,double,double,double,double,double,double,double,double,double);*/
        
        void RC_ante(int i,int j,int k,
                     Grid3<double> const &Ex,
                     Grid3<double> const &Ey,
                     Grid3<double> const &Ez);
        void RC_apply_E(int i,int j,int k,Grid3<double> &E,int dir);
        void RC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                    Grid3<double> const &Dx,
                    Grid3<double> const &Dy,
                    Grid3<double> const &Dz);
        
        void RC_dielec_treat();
        void RC_recalc();
        
        //##########
        //   PCRC
        //##########
        
        void PCRC_ante(int i,int j,int k,
                       Grid3<double> const &Ex,
                       Grid3<double> const &Ey,
                       Grid3<double> const &Ez);
        void PCRC_post(int i,int j,int k,
                       Grid3<double> const &Ex,
                       Grid3<double> const &Ey,
                       Grid3<double> const &Ez);
        void PCRC_apply_E(int i,int j,int k,Grid3<double> &E,int dir);
        
        void PCRC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                      Grid3<double> const &Dx,
                      Grid3<double> const &Dy,
                      Grid3<double> const &Dz);
                      
        void PCRC_dielec_treat();
        void PCRC_recalc();
        
        //###############
        //   Levels
        //###############
        
        int N_levels;
        int N_stim_trans;
        double AL_pop;
        
        Grid4<long double> pop_matrix;
        Grid4<long double> pop_matrix_np;
        
        //Population equations
        
        Grid2<double> levels_matrix;
        Grid1<double> pump_matrix;
        Grid1<double> lve_base_coeff;
        Grid1<double> lve_self_coeff;
        
        //Stimulated transitions
        
        Grid1<int> stim_lv_up,stim_lv_dn;
        Grid1<double> stim_wlgth;
        
        Grid1<double> pol_C1,pol_C2,pol_C3;
        
        Grid4<double> pol_field_np;
        Grid4<double> pol_field_n;
        Grid4<double> pol_field_nm;
        
        Grid3<double> Ex_n,Ey_n,Ez_n;
        
        void atom_lev_alloc_mem();
        void atom_lev_enable();
        //void setnagra2lvl(double wa,double Dwa,double t21,double DN0,double kappa);
        //void setnagra4lvl(double wa,double Dwa,double kappa,double Wp,double ei_in);
        
        void atom_lev_precompute();
        
        void AL_ante(int i,int j,int k,
                     Grid3<double> const &Ex,
                     Grid3<double> const &Ey,
                     Grid3<double> const &Ez);
        void AL_post(int i,int j,int k,
                     Grid3<double> const &Ex,
                     Grid3<double> const &Ey,
                     Grid3<double> const &Ez);
        void AL_apply_E(int i,int j,int k,Grid3<double> &E,int dir);
        
};

#endif // FDTD_MATERIAL_H_INCLUDED
