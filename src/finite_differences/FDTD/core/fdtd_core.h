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

#ifndef FDTD_CORE_H_INCLUDED
#define FDTD_CORE_H_INCLUDED

#include <fdtd_material.h>
#include <fdtd_utils.h>
#include <logger.h>

//#ifndef NTHR
//    #define NTHR 4
//#endif

class FDTD
{
    public:
        //####################
        //  Core elements
        //####################
        
        int mode;
        int tstep,iteration;
        int Nx,Ny,Nz,Nt,Ntap;
        unsigned int Nmat;
        
        int Nshift;
        double Dx,Dy,Dz,Dt,lambda,lambda_m,lambda_p;
        double cl,dte,dtm,dtdex,dtdey,dtdez,dtdmx,dtdmy,dtdmz,fact;
        double inc_ang,kx,ky;
        
        int Nx_s,Ny_s,Nz_s;
        int xs_s,xs_e,ys_s,ys_e,zs_s,zs_e;
        
        int Npad;
        
        bool enable_Ex,enable_Ey,enable_Ez;
        bool enable_Hx,enable_Hy,enable_Hz;
        
        std::string polar_mode;
        
        //ChpIn chp;
        Grid1<FDTD_Material> mats;
        #ifndef SEP_MATS
        Grid3<unsigned int> matsgrid;
        #else
        Grid3<unsigned int> matsgrid_x;
        Grid3<unsigned int> matsgrid_y;
        Grid3<unsigned int> matsgrid_z;
        #endif
        Grid3<double> Ex,Ey,Ez,Hx,Hy,Hz;

//        XGrid<double> Ex,Hx;
//        YGrid<double> Ey,Hy;
//        ZGrid<double> Ez,Hz;

//        Grid3<double> Ey,Ez,Hx,Hy,Hz;
        
        bool dt_D_comp;
        bool dt_B_comp;
        Grid3<double> dt_Dx,dt_Dy,dt_Dz;
        Grid3<double> dt_Bx,dt_By,dt_Bz;
        
        int pad_xm,pad_xp;
        int pad_ym,pad_yp;
        int pad_zm,pad_zp;
        
        FDTD(int Nx,int Ny,int Nz,int Nt,
             double Dx,double Dy,double Dz,double Dt,
             std::string smod,
             int pml_xm,int pml_xp,
             int pml_ym,int pml_yp,
             int pml_zm,int pml_zp);
        
        FDTD(int Nx,int Ny,int Nz,int Nt,
             double Dx,double Dy,double Dz,double Dt,
             std::string smod,
             int pml_xm,int pml_xp,
             int pml_ym,int pml_yp,
             int pml_zm,int pml_zp,
             int pad_xm,int pad_xp,
             int pad_ym,int pad_yp,
             int pad_zm,int pad_zp);
        
        ~FDTD();
        
        void alloc_DEBH();
        void bootstrap();
        void basic_differentials_compute();
        void bufread(Grid3<double> &,int,std::string);
        void bufwrite(Grid3<double> &,int,std::string);
        double compute_poynting_box(int i1,int i2,int j1,int j2,int k1,int k2) const;
        double compute_poynting_X(int j1,int j2,int k1,int k2,int pos_x,int sgn=1) const;
        double compute_poynting_Y(int i1,int i2,int k1,int k2,int pos_y,int sgn=1) const;
        double compute_poynting_Z(int i1,int i2,int j1,int j2,int pos_z,int sgn=1) const;
        void draw(int,int,int,int,int);
        void draw(int,int,int,int,int,Bitmap *im);
        void disable_fields(std::vector<int> const &fields);
        void find_slab(int sub_ref,int sup_ref,double &hsub,double &hstruc,double &hsup);
        bool mats_in_grid(unsigned int ind);
        void report_size();
        void reset_fields();
//        void set_field_SP_phase(double,double,std::string,double,double); 
        #ifndef SEP_MATS
        void set_matsgrid(Grid3<unsigned int> &);
        #else
        void set_matsgrid(Grid3<unsigned int> &mat_x,Grid3<unsigned int> &mat_y,Grid3<unsigned int> &mat_z);
        #endif
        void set_material(unsigned int index,Material const &material);
        //void set_spectrum_dens(int);
        void set_tapering(int Ntap);
        
        //#########################
        //   Computation update
        //#########################
        
        void advEx(int i1,int i2,int j1,int j2,int k1,int k2);
        void advEy(int i1,int i2,int j1,int j2,int k1,int k2);
        void advEz(int i1,int i2,int j1,int j2,int k1,int k2);
        void advHx(int i1,int i2,int j1,int j2,int k1,int k2);
        void advHy(int i1,int i2,int j1,int j2,int k1,int k2);
        void advHz(int i1,int i2,int j1,int j2,int k1,int k2);
        
        void adv_dt_Dx(int,int);
        void adv_dt_Dy(int,int);
        void adv_dt_Dz(int,int);
        void adv_dt_Bx(int,int);
        void adv_dt_By(int,int);
        void adv_dt_Bz(int,int);
                
        void advMats_ante(int,int);
        void advMats_simp(int,int);
        void advMats_ext(int,int);
        void advMats_post(int,int);
        void advMats_self(int,int);
        
        void update_E();
        void update_H();
        
        void update_E_ante();
        void update_E_self();
        void update_E_post();
        
        void update_E_simp();
        void update_E_ext();
        void update_H_simp();
        void update_H_ext();
        
        void update_mats_ante();
        void update_mats_simp();
        void update_mats_ext();
        void update_mats_post();
        
        //###############
        //     PMLs
        //###############
        
        double pml_m,pml_ma,pml_kmax,pml_amax;
        double pml_smax_x,pml_smax_y,pml_smax_z_u,pml_smax_z_d;
        
        int pml_xm,pml_xp;
        int pml_ym,pml_yp;
        int pml_zm,pml_zp;
        
        double pml_kappa_xm,pml_kappa_xp;
        double pml_kappa_ym,pml_kappa_yp;
        double pml_kappa_zm,pml_kappa_zp;
        
        double pml_sigma_xm,pml_sigma_xp;
        double pml_sigma_ym,pml_sigma_yp;
        double pml_sigma_zm,pml_sigma_zp;
        
        double pml_alpha_xm,pml_alpha_xp;
        double pml_alpha_ym,pml_alpha_yp;
        double pml_alpha_zm,pml_alpha_zp;
        
        Grid3<double> PsiExy,PsiExz;
        Grid3<double> PsiEyx,PsiEyz;
        Grid3<double> PsiEzx,PsiEzy;
        Grid3<double> PsiHxy,PsiHxz;
        Grid3<double> PsiHyx,PsiHyz;
        Grid3<double> PsiHzx,PsiHzy;
        
        Grid1<double> kappa_x_E,kappa_y_E,kappa_z_E;
        Grid1<double> kappa_x_H,kappa_y_H,kappa_z_H;
        
        Grid1<double> b_x_E,b_y_E,b_z_E;
        Grid1<double> b_x_H,b_y_H,b_z_H;
        
        Grid1<double> c_x_E,c_y_E,c_z_E;
        Grid1<double> c_x_H,c_y_H,c_z_H;
        
        void allocate_pml();
        
        void app_pml_Ex(int,int);
        void app_pml_Ey(int,int);
        void app_pml_Ez(int,int);
        void app_pml_Hx(int,int);
        void app_pml_Hy(int,int);
        void app_pml_Hz(int,int);
        
        void app_pml_Ex();
        void app_pml_Ey();
        void app_pml_Ez();
        void app_pml_Hx();
        void app_pml_Hy();
        void app_pml_Hz();
        
        void pml_coeff_calc();
        void pml_x_coeffs(double ind,double &kappa_x,double &b_x,double &c_x);
        void pml_y_coeffs(double ind,double &kappa_y,double &b_y,double &c_y);
        void pml_z_coeffs(double ind,double &kappa_z,double &b_z,double &c_z);
        //void pml_params_override(double,double,double,double);
        
        double pml_sig_x(double sig_base);
        double pml_sig_y(double sig_base);
        double pml_sig_z(double sig_base);
        
        //void set_pml(int,int,int);
        void set_pml_xm(double kap,double sig,double alp);
        void set_pml_xp(double kap,double sig,double alp);
        void set_pml_ym(double kap,double sig,double alp);
        void set_pml_yp(double kap,double sig,double alp);
        void set_pml_zm(double kap,double sig,double alp);
        void set_pml_zp(double kap,double sig,double alp);
        void state_pml();
        
        //###############
        //   Threading
        //###############
        
        int Nthreads;
        
        bool allow_run_E;
        ThreadsAlternator alternator_E;
        std::vector<bool> threads_ready_E;
        std::vector<std::thread*> threads_E;
        void threaded_process_E(int ID);
        
        bool allow_run_H;
        ThreadsAlternator alternator_H;
        std::vector<bool> threads_ready_H;
        std::vector<std::thread*> threads_H;
        void threaded_process_H(int ID);
        
        //###############
        //  Utilities
        //###############
        
        double get_index(int i,int j,int k) const;
        double get_kx(double lambda) const;
        double get_ky(double lambda) const;
        void guess_waveguide(Grid1<int> &strat_N,Grid1<double> &strat_ind);
        
        void local_E(int i,int j,int k,double &Ex_o,double &Ey_o,double &Ez_o) const;
        double local_Ex(int i,int j,int k) const;
        double local_Ey(int i,int j,int k) const;
        double local_Ez(int i,int j,int k) const;
        
        void local_H(int i,int j,int k,double &Hx_o,double &Hy_o,double &Hz_o) const;
        double local_Hx(int i,int j,int k) const;
        double local_Hy(int i,int j,int k) const;
        double local_Hz(int i,int j,int k) const;
        
        double local_Px(int i,int j,int k) const;
        double local_Px_p(int i,int j,int k) const;
        double local_Py(int i,int j,int k) const;
        double local_Py_p(int i,int j,int k) const;
        double local_Pz(int i,int j,int k) const;
        double local_Pz_p(int i,int j,int k) const;
        
        void set_kx(double kx);
        void set_ky(double ky);
};

class Tparam
{
    public:
        FDTD *fdtd;
        int a,b;
        
        Tparam()
            :fdtd(0), a(0), b(0)
        {
        }
        
        Tparam(FDTD *fdtdi,int ai,int bi)
            :fdtd(fdtdi), a(ai), b(bi)
        {
//            show();
        }
        
        void set(FDTD *fdtdi,int ai,int bi)
        {
            fdtd=fdtdi;
            a=ai;
            b=bi;
        }
        
        void show()
        {
            Plog::print(a, " ", b, "\n");
        }
};

#endif // FDTD_CORE_H_INCLUDED
