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

#include <vector>

#include <fd_base.h>
#include <lua_fdfd.h>
#include <grid.h>
#include <mathUT.h>
#include <material.h>
#include <phys_tools.h>

#include <Eigen/SparseCore>
#include <Eigen/SparseLU>
#include <Eigen/SparseQR>
#include <Eigen/IterativeLinearSolvers>
//#include <Eigen/SuperLUSupport>
#include <unsupported/Eigen/src/IterativeSolvers/Scaling.h>

#ifdef FD_ARMA_MODES_SOLVER
#include <armadillo>
#endif

class FDFD_mode;

enum
{
    SRC_NONE,
    SRC_PLANE_Z,
    SRC_CBOX,
    SOLVE_LU,
    SOLVE_BiCGSTAB
};

class Slice
{
    public:
        bool absorbed,solved;
        
        int ID,btm_ID,top_ID;
        Slice *btm,*top;
        
        Eigen::SparseMatrix<Imdouble> A_mat,B_mat,C_mat;
        
        Slice();
        
        void absorb_backward(Slice *slc);
        void absorb_forward(Slice *slc);
        void set_bottom(Slice *slc,int slc_ID);
        void set_ID(int ID_);
        void set_top(Slice *slc,int slc_ID);
        void show();
};

class FDFD: public FD_Base
{
    private:
        
        double lambda;
        AngleRad inc_theta,inc_phi;
        double kx,ky;
        
        // Injection
        
        int inj_type;
        int inj_xm,inj_xp;
        int inj_ym,inj_yp;
        int inj_zm,inj_zp;
        Eigen::SparseVector<Imdouble> inj_F_src;
        
        Eigen::VectorXcd F;
        Eigen::SparseMatrix<Imdouble> D_mat,M_mat;
        
        int solver_type;
        
        int slc_Ex(int i,int j);
        int slc_Ey(int i,int j);
        int slc_Ez(int i,int j);
        int slc_Hx(int i,int j);
        int slc_Hy(int i,int j);
        int slc_Hz(int i,int j);
        
        void update_Nxyz();
    public:
        FDFD(double Dx,double Dy,double Dz);
        
        void draw(int vmode,int pos_x,int pos_y,int pos_z,std::string name_mod="");
        void get_injection_cbox(int xm,int xp,int ym,int yp,int zm,int zp,
                               bool x_on,bool y_on,bool z_on,
                               Eigen::SparseVector<Imdouble> &F_src);
//        void init(int Nx,int Ny,int Nz,double Dx,double Dy,double Dz);
        
        Imdouble get_Ex(int i,int j,int k);
        Imdouble get_Ey(int i,int j,int k);
        Imdouble get_Ez(int i,int j,int k);
        
        Imdouble get_Hx(int i,int j,int k);
        Imdouble get_Hy(int i,int j,int k);
        Imdouble get_Hz(int i,int j,int k);
        
        void interp(double &x,double &y,double &z,
                    double offset_x,double offset_y,double offset_z,
                    int &i1,int &i2,
                    int &j1,int &j2,
                    int &k1,int &k2);
        
        Imdouble interp_Ex(double x,double y,double z);
        Imdouble interp_Ey(double x,double y,double z);
        Imdouble interp_Ez(double x,double y,double z);
        
        Imdouble interp_Hx(double x,double y,double z);
        Imdouble interp_Hy(double x,double y,double z);
        Imdouble interp_Hz(double x,double y,double z);
        
        void set_injection_cbox(int xm,int xp,int ym,int yp,int zm,int zp,
                               Eigen::SparseVector<Imdouble> &F_src);
        void set_injection_plane_z(int k);
        
        void solve_prop_1D(double lambda,AngleRad theta,AngleRad phi,AngleRad polar);
        void solve_prop_2D(double lambda,AngleRad theta,AngleRad phi,AngleRad polar);
        void solve_prop_3D(double lambda,AngleRad theta,AngleRad phi,AngleRad polar);
        
        void solve_prop_3D_SAM(double lambda,AngleRad theta,AngleRad phi,AngleRad polar);
        
        friend void fdfd_single_particle(FDFD_Mode const &fdtd_mode);
        friend void fdfd_periodic(FDFD_Mode const &fdtd_mode);
        friend void mode_fdfd(FDFD_Mode const &fdfd_mode);
        friend void mode_3D(FDFD_Mode const &fdfd_mode);
};

class FDMS: public FD_Base
{
    public:
        std::string solver;
        
        FDMS(double Dx,double Dy,double Dz);
        
        void draw(int vmode,int pos_x,int pos_y,int pos_z,std::string name_mod="");
        
        void solve_modes_1D(double lambda,int polar,
                            Imdouble const &n_guess,Imdouble &n_mode,
                            Eigen::VectorXcd &E_out,
                            Eigen::VectorXcd &H_out);
        
        void solve_modes_2D(double lambda,
                            Imdouble const &n_guess,Imdouble &n_mode,
                            Grid2<Imdouble> &Ex_out,Grid2<Imdouble> &Ey_out,Grid2<Imdouble> &Ez_out,
                            Grid2<Imdouble> &Hx_out,Grid2<Imdouble> &Hy_out,Grid2<Imdouble> &Hz_out);
};


void solve_LU(Eigen::SparseMatrix<Imdouble> const &A,
              Eigen::VectorXcd &x,
              Eigen::VectorXcd const &b);

void solve_BiCGSTAB(Eigen::SparseMatrix<Imdouble> const &A,
                    Eigen::VectorXcd &x,
                    Eigen::VectorXcd const &b,
                    Eigen::VectorXcd const &guess);

Imdouble inverse_power_iteration(Eigen::SparseMatrix<Imdouble> const &A,
                                 Imdouble guess,Eigen::VectorXcd &V,
                                 double conv=1e-5,int max_it=100);
Imdouble inverse_power_iteration_BICGSTAB(Eigen::SparseMatrix<Imdouble> const &A,
                                          Imdouble guess,Eigen::VectorXcd &V,
                                          double conv=1e-5,int max_it=100);
