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

#include <index_utils.h>
#include <phys_constants.h>
#include <cmath>
#include <complex>
#include <berreman_strat.h>

extern const double Pi;
extern const Imdouble Im;

extern std::ofstream plog;

//###############
//   B strat
//###############

B_strat::B_strat(int Nli,Imdouble sup_ind_i,Imdouble sub_ind_i)
    :N_layers(Nli), sup_ind(sup_ind_i), sub_ind(sub_ind_i),
        h_layer(N_layers,0),
        e11(N_layers,0), e22(N_layers,0), e33(N_layers,0),
        mu11(N_layers,0), mu22(N_layers,0), mu33(N_layers,0),
        F_m(4,4,0),
        Ft_m(4,4,0),
        P_m(4,4,N_layers,0),
        diagonal(N_layers,true),
        eps(3,3,N_layers,0),
        mu(3,3,N_layers,0)
{
}

void B_strat::get_D_matrix(int l,Imdouble dze,Eigen::Matrix<Imdouble,4,4> &D_mat)
{
    Imdouble &M11=eps(0,0,l); Imdouble &M12=eps(0,1,l); Imdouble &M13=eps(0,2,l);
    Imdouble &M21=eps(1,0,l); Imdouble &M22=eps(1,1,l); Imdouble &M23=eps(1,2,l);
    Imdouble &M31=eps(2,0,l); Imdouble &M32=eps(2,1,l); Imdouble &M33=eps(2,2,l);
        
    Imdouble M14=0,M15=0,M16=0;
    Imdouble M24=0,M25=0,M26=0;
    Imdouble M34=0,M35=0,M36=0;
    
    Imdouble &M44=mu(0,0,l); Imdouble &M45=mu(0,1,l); Imdouble &M46=mu(0,2,l);
    Imdouble &M54=mu(1,0,l); Imdouble &M55=mu(1,1,l); Imdouble &M56=mu(1,2,l);
    Imdouble &M64=mu(2,0,l); Imdouble &M65=mu(2,1,l); Imdouble &M66=mu(2,2,l);
    
    Imdouble M41=0,M42=0,M43=0;
    Imdouble M51=0,M52=0,M53=0;
    Imdouble M61=0,M62=0,M63=0;
    
    Imdouble inv_d=1.0/(M33*M66-M36*M63);
    
    Imdouble a31=(M61*M36-M31*M66)*inv_d;
    Imdouble a32=((M62-dze)*M36-M32*M66)*inv_d;
    Imdouble a34=(M64*M36-M34*M66)*inv_d;
    Imdouble a35=(M65*M36-(M35+dze)*M66)*inv_d;
    Imdouble a61=(M63*M31-M33*M61)*inv_d;
    Imdouble a62=(M63*M32-M33*(M62-dze))*inv_d;
    Imdouble a64=(M63*M34-M33*M64)*inv_d;
    Imdouble a65=(M63*(M35+dze)-M33*M65)*inv_d;
    
    Imdouble dze53=M53+dze;
    D_mat(0,0)=M51+dze53*a31+M56*a61;
    D_mat(0,1)=M55+dze53*a35+M56*a65;
    D_mat(0,2)=M52+dze53*a32+M56*a62;
    D_mat(0,3)=-(M54+dze53*a34+M56*a64);
    
    D_mat(1,0)=M11+M13*a31+M16*a61;
    D_mat(1,1)=M15+M13*a35+M16*a65;
    D_mat(1,2)=M12+M13*a32+M16*a62;
    D_mat(1,3)=-(M14+M13*a34+M16*a64);
    
    D_mat(2,0)=-(M41+M43*a31+M46*a61);
    D_mat(2,1)=-(M45+M43*a35+M46*a65);
    D_mat(2,2)=-(M42+M43*a32+M46*a62);
    D_mat(2,3)=M44+M43*a34+M46*a64;
    
    Imdouble dze26=M26-dze;
    D_mat(3,0)=M21+M23*a31+dze26*a61;
    D_mat(3,1)=M25+M23*a35+dze26*a65;
    D_mat(3,2)=M22+M23*a32+dze26*a62;
    D_mat(3,3)=M24+M23*a34+dze26*a64;
}

void B_strat::get_D_matrix_simp(int l,Imdouble dze,Eigen::Matrix<Imdouble,4,4> &D_mat)
{
    Imdouble &M11=eps(0,0,l); Imdouble &M12=eps(0,1,l); Imdouble &M13=eps(0,2,l);
    Imdouble &M21=eps(1,0,l); Imdouble &M22=eps(1,1,l); Imdouble &M23=eps(1,2,l);
    Imdouble &M31=eps(2,0,l); Imdouble &M32=eps(2,1,l); Imdouble &M33=eps(2,2,l);
            
    Imdouble &M44=mu(0,0,l); Imdouble &M45=mu(0,1,l); Imdouble &M46=mu(0,2,l);
    Imdouble &M54=mu(1,0,l); Imdouble &M55=mu(1,1,l); Imdouble &M56=mu(1,2,l);
    Imdouble &M64=mu(2,0,l); Imdouble &M65=mu(2,1,l); Imdouble &M66=mu(2,2,l);
            
    Imdouble inv_d=1.0/(M33*M66);
    
    Imdouble a31=(-M31*M66)*inv_d;
    Imdouble a32=(-M32*M66)*inv_d;
    Imdouble a35=(-(dze)*M66)*inv_d;
    Imdouble a62=(-M33*(-dze))*inv_d;
    Imdouble a64=(-M33*M64)*inv_d;
    Imdouble a65=(-M33*M65)*inv_d;
    
    Imdouble dze53=dze;
    D_mat(0,0)=dze53*a31;
    D_mat(0,1)=M55+dze53*a35+M56*a65;
    D_mat(0,2)=dze53*a32+M56*a62;
    D_mat(0,3)=-(M54+M56*a64);
    
    D_mat(1,0)=M11+M13*a31;
    D_mat(1,1)=M13*a35;
    D_mat(1,2)=M12+M13*a32;
    D_mat(1,3)=0;
    
    D_mat(2,0)=0;
    D_mat(2,1)=-(M45+M46*a65);
    D_mat(2,2)=-(M46*a62);
    D_mat(2,3)=M44+M46*a64;
    
    Imdouble dze26=-dze;
    D_mat(3,0)=M21+M23*a31;
    D_mat(3,1)=M23*a35+dze26*a65;
    D_mat(3,2)=M22+M23*a32+dze26*a62;
    D_mat(3,3)=dze26*a64;
}

void B_strat::compute(double lambda,double ang_rad,
                      Imdouble &r_EE,Imdouble &r_EM,Imdouble &t_EE,Imdouble &t_EM,
                      Imdouble &r_ME,Imdouble &r_MM,Imdouble &t_ME,Imdouble &t_MM)
{
    int i,j;
    
    Imdouble &n1=sup_ind;
    Imdouble &n3=sub_ind;
    
    double theta=ang_rad;
//    Imdouble theta_r=asin(sup_ind/sub_ind*std::sin(theta));
    
//    Imdouble e_sp=sup_ind*sup_ind*e0;
//    Imdouble e_sb=sub_ind*sub_ind*e0;
    
//    double mu_sp=mu0;
//    double mu_sb=mu0;
    
//    Imdouble p1_TE=sqrt(e_sp/mu_sp)*cos(theta);
//    Imdouble p3_TE=sqrt(e_sb/mu_sb)*cos(theta_r);
//    
//    Imdouble p1_TM=sqrt(mu_sp/e_sp)*cos(theta);
//    Imdouble p3_TM=sqrt(mu_sb/e_sb)*cos(theta_r);
    
    double w=2.0*Pi*c_light/lambda;
    Imdouble kx0=2.0*Pi*n1/lambda*std::sin(theta);
    Imdouble dze=kx0/w;
//    double dze2=dze*dze;
    
    Imdouble a,b,u,v;
    Imdouble x1,x2;
    
    Imdouble cx1,sx1,cx2,sx2;
    
//    for(l=0;l<N_layers;l++)
//    {
//        Eigen::Matrix<Imdouble,4,4> D_mat;
//        get_D_matrix(l,dze,D_mat);
//        
//    }
//    
//    for(l=0;l<N_layers;l++)
//    {
//        Eigen::Matrix<Imdouble,4,4> D_mat;
//        get_D_matrix_simp(l,dze,D_mat);
//    }
        
//    Eigen::Matrix<Imdouble,4,4> D_mat,F_mat;
    get_D_matrix(0,dze,D_mat);
    
    Imdouble d_mod=-h_layer[0]*w*Im;
    
    for(i=0;i<4;i++){ for(j=0;j<4;j++)
    {
        D_mat(i,j)*=d_mod;
    }}
    
    F_mat=D_mat.exp();
    
    
//    for(i=0;i<4;i++)
//    {
//        for(j=0;j<4;j++)
//        {
//            std::cout<<F_mat(i,j)<<" ";
//        }
//        std::cout<<std::endl;
//    }
    
    Imdouble eps1=sup_ind*sup_ind*e0;
    Imdouble eps3=sub_ind*sub_ind*e0;
    Imdouble mu1=mu0;
    Imdouble mu3=mu0;
    
    Imdouble k1=2.0*Pi*n1/lambda;
    Imdouble k3=2.0*Pi*n3/lambda;
    Imdouble k1z=std::sqrt(static_cast<Imdouble>(k1*k1-kx0*kx0));
    Imdouble k3z=std::sqrt(static_cast<Imdouble>(k3*k3-kx0*kx0));
    
    Imdouble p1x=eps1*w/k1z;
    Imdouble p1y=k1z/(mu1*w); /// - sign
    Imdouble p3x=eps3*w/k3z;
    Imdouble p3y=k3z/(mu3*w); /// - sign
    
    Imdouble &F11=F_mat(0,0); Imdouble &F12=F_mat(0,1); Imdouble &F13=F_mat(0,2); Imdouble &F14=F_mat(0,3);
    Imdouble &F21=F_mat(1,0); Imdouble &F22=F_mat(1,1); Imdouble &F23=F_mat(1,2); Imdouble &F24=F_mat(1,3);
    Imdouble &F31=F_mat(2,0); Imdouble &F32=F_mat(2,1); Imdouble &F33=F_mat(2,2); Imdouble &F34=F_mat(2,3);
    Imdouble &F41=F_mat(3,0); Imdouble &F42=F_mat(3,1); Imdouble &F43=F_mat(3,2); Imdouble &F44=F_mat(3,3);
    
    Imdouble g11=(F11+F12*p3x)/2.0;       Imdouble g12=(F13+F14*p3y)/2.0;
    Imdouble g21=(F21+F22*p3x)/(2.0*p1x); Imdouble g22=(F23+F24*p3y)/(2.0*p1x);
    Imdouble g31=(F31+F32*p3x)/2.0;       Imdouble g32=(F33+F34*p3y)/2.0;
    Imdouble g41=(F41+F42*p3x)/(2.0*p1y); Imdouble g42=(F43+F44*p3y)/(2.0*p1y);
    
    Imdouble D=(g11+g21)*(g32+g42)-(g21+g22)*(g31+g41);
    
    double kokz3=std::abs(k3/k3z); double kzok3=std::abs(k3z/k3);
    double kokz1=std::abs(k1/k1z); double kzok1=std::abs(k1z/k1);
    
//    std::cout<<p1y<<" "<<p3y<<std::endl;
    
    t_EE=(g11+g21)/D;
    t_EM=-(g12+g22)/D*kokz3;
    t_ME=-(g31+g41)/D*kzok1;
    t_MM=(g32+g42)/D*kzok1*kokz3;
    
    r_EE=(g31-g41)*kzok3*t_EM+(g32-g42)*t_EE;
    r_EM=((g11-g21)*kzok3*t_EM+(g12-g22)*t_EE)*kokz1;
    r_ME=(g31-g41)*kzok3*t_MM+(g32-g42)*t_ME;
    r_MM=((g11-g21)*kzok3*t_MM+(g12-g22)*t_ME)*kokz1;
}

void B_strat::compute(double lambda,AngleOld ang,Imdouble &r_TE,Imdouble &r_TM,Imdouble &t_TE,Imdouble &t_TM)
{
    int i,j,k,l;
    
    double theta=ang.radian();
    Imdouble theta_r=std::asin(sup_ind/sub_ind*std::sin(theta));
    
    Imdouble e_sp=sup_ind*sup_ind*e0;
    Imdouble e_sb=sub_ind*sub_ind*e0;
    
    double mu_sp=mu0;
    double mu_sb=mu0;
    
    Imdouble p1_TE=sqrt(e_sp/mu_sp)*cos(theta);
    Imdouble p3_TE=sqrt(e_sb/mu_sb)*cos(theta_r);
    
    Imdouble p1_TM=sqrt(mu_sp/e_sp)*cos(theta);
    Imdouble p3_TM=sqrt(mu_sb/e_sb)*cos(theta_r);
    
    double w=2.0*Pi*c_light/lambda;
    double kx0=2.0*Pi/lambda*std::sin(theta);
    double dze=kx0/w;
    double dze2=dze*dze;
    
    Imdouble a,b,u,v;
    Imdouble x1,x2;
    
    Imdouble cx1,sx1,cx2,sx2;
        
    for(l=0;l<N_layers;l++)
    {
        using std::sqrt;
        
        a=sqrt(mu22[l]-dze2/e33[l]);
        b=sqrt(e11[l]);
        u=sqrt(mu11[l]);
        v=sqrt(e22[l]-dze2/mu33[l]);
        
        x1=-w*h_layer[l]*a*b;
        x2=-w*h_layer[l]*u*v;
        
        using std::cos;
        using std::sin;
        
        cx1=cos(x1);
        sx1=sin(x1);
        cx2=cos(x2);
        sx2=sin(x2);
        
        P_m(0,0,l)=cx1;
        P_m(0,1,l)=a/b*sx1*Im;
        P_m(1,0,l)=b/a*sx1*Im;
        P_m(1,1,l)=cx1;
        
        P_m(2,2,l)=cx2;
        P_m(2,3,l)=u/v*sx2*Im;
        P_m(3,2,l)=v/u*sx2*Im;
        P_m(3,3,l)=cx2;
    }
    
    Imdouble S;
    
    for(l=0;l<N_layers;l++)
    {
        if(l==0)
        {
            for(i=0;i<4;i++) for(j=0;j<4;j++) F_m(i,j)=P_m(i,j,0);
        }
        else
        {
            Ft_m=F_m;
            
            for(i=0;i<4;i++){ for(j=0;j<4;j++)
            {
                S=0;
                
                for(k=0;k<4;k++) S+=Ft_m(i,k)*P_m(k,j,l);
                
                F_m(i,j)=S;
            }}
        }
    }
    
//    for(i=0;i<4;i++)
//    {
//        for(j=0;j<4;j++)
//        {
//            std::cout<<F_m(i,j)<<" ";
//        }
//        std::cout<<std::endl;
//    }
    
    Imdouble d=(p1_TE*(F_m(2,2)+p3_TE*F_m(2,3))+(F_m(3,2)+p3_TE*F_m(3,3)));
        
    r_TE=(p1_TE*(F_m(2,2)+p3_TE*F_m(2,3))-(F_m(3,2)+p3_TE*F_m(3,3)))/d;
    t_TE=2.0*p1_TE/d;
    
//    std::cout<<p1_TE<<" "<<p3_TE<<std::endl;
    
    d=((p3_TM*F_m(0,0)+F_m(0,1))+p1_TM*(p3_TM*F_m(1,0)+F_m(1,1)));
    
    r_TM=((p3_TM*F_m(0,0)+F_m(0,1))-p1_TM*(p3_TM*F_m(1,0)+F_m(1,1)))/d;
    t_TM=2.0*p3_TM/d*cos(theta)/cos(theta_r);
}

void B_strat::compute_abs(double lambda,double ang,double &r_TE,double &r_TM,double &t_TE,double &t_TM)
{
    Imdouble r_TEi=0,r_TMi=0,t_TEi=0,t_TMi=0;
    
    compute(lambda,ang,r_TEi,r_TMi,t_TEi,t_TMi);
        
    r_TE=std::abs(r_TEi);
    r_TM=std::abs(r_TMi);
    t_TE=std::abs(t_TEi);
    t_TM=std::abs(t_TMi);
}

void B_strat::set_aniso(int l,double h,Grid2<Imdouble> const &eps_i,Grid2<Imdouble> const &mu_i)
{
    int i,j;
    
    h_layer[l]=h;
    diagonal[l]=false;
    
    for(i=0;i<3;i++){ for(j=0;j<3;j++)
    {
        eps(i,j,l)=eps_i(i,j)*e0;
        mu(i,j,l)=mu_i(i,j)*mu0;
    }}
}

void B_strat::set_aniso_diag(int l,double h,
                             Imdouble e11_i,Imdouble e22_i,Imdouble e33_i,
                             Imdouble mu11_i,Imdouble mu22_i,Imdouble mu33_i)
{
    h_layer[l]=h;
    diagonal[l]=true;
    
    e11[l]=e11_i*e0;
    e22[l]=e22_i*e0;
    e33[l]=e33_i*e0;
    mu11[l]=mu11_i*mu0;
    mu22[l]=mu22_i*mu0;
    mu33[l]=mu33_i*mu0;
}

void B_strat::set_environment(Imdouble sup_ind_i,Imdouble sub_ind_i)
{
    sup_ind=sup_ind_i;
    sub_ind=sub_ind_i;
}

void B_strat::set_iso(int l,double h,Imdouble e_i,Imdouble mu_i)
{
    h_layer[l]=h;
    diagonal[l]=true;
    
    e11[l]=e_i*e0;
    e22[l]=e_i*e0;
    e33[l]=e_i*e0;
    mu11[l]=mu_i*mu0;
    mu22[l]=mu_i*mu0;
    mu33[l]=mu_i*mu0;
}
