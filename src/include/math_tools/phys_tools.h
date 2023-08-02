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

#ifndef PHYS_TOOLS_H
#define PHYS_TOOLS_H

#include <Eigen/Eigen>

#include <geometry.h>
#include <phys_constants.h>
#include <mathUT.h>

double conic(double x,double R,double K);
void conic_far_focus(double &x,double &y,double R,double K);
double conic_invert(double y,double R,double K);
double conic_midpoint(double R,double K);
void conic_near_focus(double &x,double &y,double R,double K);
void fresnel_rt_te_tm(AngleRad angle_inc,Imdouble n1,Imdouble n2,
                      Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm);
void fresnel_rt_te_tm_wn(double lambda,Imdouble n1,Imdouble n2,double kp,
                         Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm);
void fresnel_rt_te_tm_E_wn(double lambda,Imdouble n1,Imdouble n2,double kp,
                           Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm);
double gaussian_spectrum(double w,double lambda_min,double lambda_max,double amp);
double eV_to_m(double E);
double Hz_to_m(double f);
double inv_cm_to_m(double n);
double J_to_m(double E);
double m_to_eV(double lambda);
double m_to_inv_cm(double lambda);
double m_to_J(double lambda);
double m_to_Hz(double lambda);
double m_to_microns(double lambda);
double m_to_mm(double lambda);
double m_to_nm(double lambda);
double m_to_rad_Hz(double lambda) ;
double m_to_THz(double lambda);
double microns_to_m(double lambda);
double mm_to_m(double lambda);
double rad_Hz_to_m(double w);
double nm_to_m(double lambda);
double THz_to_m(double f);
void plane_wave(double lambda,double n,Vector3 const &dir,double polar,double x,double y,double z,double t,ImVector3 &E,ImVector3 &H);
void plane_wave(double lambda,double n,double th,double phi,AngleRad polar,double x,double y,double z,double t,ImVector3 &E,ImVector3 &H);
double planck_distribution_wavelength(double lambda,double T);
Imdouble spp_get_kspp(double w,Imdouble eps_1,Imdouble eps_2);

class Position
{
    public:
        double x,y,z;
        double a,b,c;
        
        Eigen::Matrix3d a_mat,b_mat,c_mat,f_mat;
            
        Position()
            :x(0), y(0), z(0),
             a(0), b(0), c(0)
        {
            update_rot_matrix();
        }
        
        void get_matrix(Vector3 &O,Vector3 &A,Vector3 &B,Vector3 &C)
        {
            O.x=x;
            O.y=y;
            O.z=z;
            
            A.x=f_mat(0,0);
            A.y=f_mat(0,1);
            A.z=f_mat(0,2);
            
            B.x=f_mat(1,0);
            B.y=f_mat(1,1);
            B.z=f_mat(1,2);
            
            C.x=f_mat(2,0);
            C.y=f_mat(2,1);
            C.z=f_mat(2,2);
        }
        
        void get_orientation(Vector3 &A,Vector3 &B,Vector3 &C)
        {
            A.x=f_mat(0,0);
            A.y=f_mat(0,1);
            A.z=f_mat(0,2);
            
            B.x=f_mat(1,0);
            B.y=f_mat(1,1);
            B.z=f_mat(1,2);
            
            C.x=f_mat(2,0);
            C.y=f_mat(2,1);
            C.z=f_mat(2,2);
        }
        
        void set_location(double x_,double y_,double z_)
        {
            x=x_;
            y=y_;
            z=z_;
        }
        
        void set_rotation(double a_,double b_,double c_)
        {
            a=a_;
            b=b_;
            c=c_;
            
            update_rot_matrix();
        }
        
        void reset()
        {
            x=y=z=a=b=c=0;
            
            update_rot_matrix();
        }
        
        void update_rot_matrix()
        {
            a_mat(0,0)= std::cos(a); a_mat(0,1)=std::sin(a);  a_mat(0,2)=0;
            a_mat(1,0)=-std::sin(a); a_mat(1,1)=std::cos(a);  a_mat(1,2)=0;
            a_mat(2,0)= 0;           a_mat(2,1)=0;            a_mat(2,2)=1;
            
            b_mat(0,0)= std::cos(b); b_mat(0,1)=0; b_mat(0,2)=std::sin(b);
            b_mat(1,0)=0;            b_mat(1,1)=1; b_mat(1,2)=0;
            b_mat(2,0)=-std::sin(b); b_mat(2,1)=0; b_mat(2,2)=std::cos(b);
            
            c_mat(0,0)=1; c_mat(0,1)= 0;           c_mat(0,2)=0;
            c_mat(1,0)=0; c_mat(1,1)= std::cos(c); c_mat(1,2)=std::sin(c);
            c_mat(2,0)=0; c_mat(2,1)=-std::sin(c); c_mat(2,2)=std::cos(c);
            
            f_mat=c_mat*(b_mat*a_mat);
        }
};

class SeedParticle
{
    public:
        bool stable;
        double x,y,r;
        double Fx,Fy;
        
        SeedParticle();
        
        void add_force(double x_,double y_,double r_);
        void advance(double x_max,double y_max);
        void clamp_force(double fmax);
        void set(double x_,double y_,double r_);
};

void random_packing(std::vector<SeedParticle> &particles,double x_max,double y_max,double r,int Npart,int seed);

#endif // PHYS_TOOLS_H
