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

#ifndef SOURCES_H
#define SOURCES_H

#include <geometry.h>
#include <mathUT.h>
#include <phys_constants.h>
#include <planar_wgd.h>
#include <bitmap3.h>
#include <fdtd_core.h>

//######################
//   Source_generator
//######################

class Source_generator
{
    public:
        int type;
        
        int x1,x2,y1,y2,z1,z2;
        double x1r,x2r,y1r,y2r,z1r,z2r;
        
        bool location_real;
        
        int orientation,polarization;
        
        double lambda_min,lambda_max;
        double lambda_target,nr_target,ni_target;
        
        enum
        {
            SOURCE_GEN_AFP_TFSF=0,
            SOURCE_GEN_GUIDED_PLANAR,
            SOURCE_GEN_OSCILLATOR
        };
        
        Source_generator();
        Source_generator(Source_generator const &src);
        
        void operator = (Source_generator const &src);
        void to_discrete(double Dx,double Dy,double Dz);
        void set_guided_target(double lambda_target,double nr_target,double ni_target);
        void set_polarization(std::string polarization);
        void set_spectrum(double lambda_min,double lambda_max);
};

//############
//   Source
//############

class Source
{
    public:
        int Nx,Ny,Nz,Nt;
        int xs_s,xs_e,ys_s,ys_e,zs_s,zs_e;
        
        double Dx,Dy,Dz,Dt;
        
        int step;
        int type;
        int x1,x2,y1,y2,z1,z2;
        int span1,span2,span3;
        
        int Nl;
        double lambda_min,lambda_max;
        std::vector<double> lambda,w,Sp;
        
        // Threading
        
        bool process_threads;
        unsigned int Nthreads;
        ThreadsAlternator alternator;
        std::vector<std::thread*> threads;
        std::vector<bool> threads_ready;
        
        Source();
        Source(int x1,int x2,
               int y1,int y2,
               int z1,int z2);
        virtual ~Source();
        
        void inject_E(FDTD &fdtd);
        void inject_H(FDTD &fdtd);
        
        virtual void deep_inject_E(FDTD &fdtd);
        virtual void deep_inject_H(FDTD &fdtd);
        virtual void deep_link(FDTD const &fdtd);
        
        virtual void initialize();
        void link(FDTD const &fdtd);
        
        void expand_spectrum_gaussian(double pw_edge,double threshold,int Nl=0);
        void expand_spectrum_S(double factor,int Nl=0);
        void set_loc(int x1,int x2,int y1,int y2,int z1,int z2);
        virtual void set_spectrum(double lambda);
        virtual void set_spectrum(double lambda_min,double lambda_max);
        virtual void set_spectrum(Grid1<double> const &lambda);
        void set_type(int type);
};

class AFP_TFSF: public Source
{
    public:
        int Nmats;
        Grid1<Material> mats;
        Grid3<unsigned int> matsgrid;
        Grid2<Imdouble> Ex,Ey,Ez,Hx,Hy,Hz;
        
        AFP_TFSF();
        ~AFP_TFSF();
        
        void deep_inject_E(FDTD &fdtd);
        void deep_inject_H(FDTD &fdtd);
        void deep_link(FDTD const &fdtd);
        void initialize();
        void set_matsgrid(Grid3<unsigned int> const &G);
};

class Bloch_Monochromatic: public Source
{
    public:
        
};

class Bloch_Wideband: public Source
{
    public:
        int t_offset;
        double kx,ky,kxy,eps_inf;
        double polar;
        
        AngleRad safe_angle,cut_angle;
        
        Grid1<double> kn,kz,Sp;
        Grid1<Vector3> E_base,H_base;
        
        Bloch_Wideband(int x1,int x2,int y1,int y2,int z1,int z2,
                       double kx,double ky,AngleRad polar);
        ~Bloch_Wideband();
        
        void deep_link(FDTD const &fdtd);
        void get_E(Grid2<double> &Ex,
                   Grid2<double> &Ey,
                   Grid2<double> &Ez,
                   double z,double t);
        void initialize();
        void inject_E(FDTD &real_fdtd,FDTD &imag_fdtd);
        void inject_H(FDTD &real_fdtd,FDTD &imag_fdtd);
        void set_cut_angle(AngleRad const &safe_angle,AngleRad const &cut_angle);
};

class Electric_Dipole: public Source
{
    public:
        double xr,yr,zr;
        Vector3 dip_p;
        double dip_eps;
        
        Grid3<Vector3> ERsP,ERvP,HRvP;
        
        Electric_Dipole();
        ~Electric_Dipole();
        
        void deep_inject_E(FDTD &fdtd);
        void deep_inject_H(FDTD &fdtd);
        void deep_link(FDTD const &fdtd);
        void initialize();
};

class Gaussian_beam_2D: public Source
{
    public:
        
};

class Guided_planar: public Source
{
    public:
        int tshift,t_max;
        double scaling;
        
        int polar;
        double lambda_target,nr_target,ni_target;
        
        std::vector<FDTD_Material const *> mats;
        std::vector<unsigned int> matsgrid;
        std::vector<Imdouble> n_eff;
        
        Grid2<Imdouble> E_mode,H_mode;
        
        std::vector<Imdouble> precomp,precomp_E,precomp_H;
        
        Guided_planar(int type,
                      int x1,int x2,int y1,int y2,int z1,int z2,
                      int polar,double lambda_target,double nr_target,double ni_target);
        ~Guided_planar();
        
        void deep_inject_E(FDTD &fdtd);
        void deep_inject_H(FDTD &fdtd);
        void deep_link(FDTD const &fdtd);
        void initialize();
        void threaded_computation(unsigned int ID);
};

class Oscillator: public Source
{
    public:
        int tshift;
        double w0;
        double dw;
        
        Oscillator(int type,
                   int x1,int x2,
                   int y1,int y2,
                   int z1,int z2);
        ~Oscillator();
        
        void deep_inject_E(FDTD &fdtd);
        void initialize();
};

class MR_Oscillator: public Source
{
    public:
        int tshift;
        double w0;
        double dw;
        
        MR_Oscillator(int Nosc,
                      int x1,int x2,
                      int y1,int y2,
                      int z1,int z2,
                      std::string generator_fname);
        ~MR_Oscillator();
        
        void deep_inject_E(FDTD &fdtd);
        void initialize();
};

Source* generate_fdtd_source(Source_generator const &gen,FDTD const &fdtd);

#endif // SOURCES_H
