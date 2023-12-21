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

#include <planar_wgd.h>
#include <string_tools.h>

#include <Eigen/Eigen>

#include <fstream>


extern const Imdouble Im;

extern std::ofstream plog;

void Slab_guide::compute_guiding_char(double lambda_i)
{
    lambda=lambda_i;
    n_TE=n_TM=0;
    
    beta_TE.clear();
    beta_TM.clear();
    
    double bet=0;
    
    while(is_guided_simple(lambda,bet,*this,"TE",n_TE))
    {
        n_TE+=1;
        beta_TE.push_back(bet);
    }
    while(is_guided_simple(lambda,bet,*this,"TM",n_TM))
    {
        n_TM+=1;
        beta_TM.push_back(bet);
    }
}

void shift_inv_pow(double s,Eigen::MatrixXd &mat,double &eig,Eigen::MatrixXd &eig_vec)
{
    int i,j;
    
    int N=mat.rows();
    
    Eigen::MatrixXd M2(mat);
    
    double tmp=1.0/std::sqrt(N+0.0);
    
    for(i=0;i<N;i++)
    {
        M2(i,i)-=s;
        eig_vec(i)=tmp;
    }
    
    Eigen::FullPivLU<Eigen::MatrixXd> M2_dec=M2.fullPivLu();
    
    int sign=1;
    double a;
    
    for(j=0;j<50;j++)
    {
        a=eig_vec(N/2);
        eig_vec=M2_dec.solve(eig_vec);
        
        if(eig_vec(N/2)/a<0) sign=-1;
        else sign=1;
        
        tmp=0;
        for(i=0;i<N;i++) tmp+=eig_vec(i)*eig_vec(i);
        tmp=std::sqrt(tmp);
        for(i=0;i<N;i++) eig_vec(i)/=tmp;
        
        eig=1.0/tmp*sign+s;
        std::cout<<a<<" "<<eig<<std::endl;
    }
}

#ifdef FD_ARMA_MODES_SOLVER
#include <armadillo>

void Slab_guide::compute_guiding_char_disc(double lambda_i,double Dz)
{
    int i,j;
    
    lambda=lambda_i;
    n_TE=n_TM=0;
    
    beta_TE.clear();
    beta_TM.clear();
    
    int Ns=static_cast<int>(h/Dz+0.5);
    int Ng=15*Ns;
    std::cout<<Ng<<std::endl;
    
    arma::SpMat<std::complex<double>> TE_mat(Ng,Ng),TM_mat(Ng,Ng);
    
    arma::SpMat<std::complex<double>> Bxz(Ng,Ng);
    arma::SpMat<std::complex<double>> Ayz(Ng,Ng);
    
    arma::SpMat<std::complex<double>> Py(Ng,Ng);
    arma::SpMat<std::complex<double>> Qx(Ng,Ng);
    arma::SpMat<std::complex<double>> Qz(Ng,Ng);
    
    arma::SpMat<std::complex<double>> inv_Qx(Ng,Ng);
    
    double w=2.0*Pi*c_light/lambda;
    
    std::cout<<n1<<" "<<n2<<" "<<n3<<std::endl;
    
    for(i=0;i<Ng;i++)
    {
        if(i<7*Ns) Py(i,i)=-w*n1*n1*e0*Im;
        else if(i>=7*Ns && i<8*Ns) Py(i,i)=-w*n2*n2*e0*Im;
        else Py(i,i)=-w*n3*n3*e0*Im;
        
        Qx(i,i)=w*mu0*Im;
        Qz(i,i)=w*mu0*Im;
        
        inv_Qx(i,i)=1.0/(w*mu0*Im);
        
        Bxz(i,i)=1.0/Dz;
        if(i>0) Bxz(i,i-1)=-1.0/Dz;
        
        if(i<Ng-1) Ayz(i,i+1)=1.0/Dz;
        Ayz(i,i)=-1.0/Dz;
    }
    
    double k0=w/c_light;
    
    TE_mat=Qz*(Bxz*inv_Qx*Ayz+Py);
    //TE_mat=(Bxz*inv_Qx*Ayz+Py)*Qz;
    
//    Qz.print();
    Py.print();
    
    arma::cx_vec eigval;
    arma::cx_mat eigvec;
    
//    arma::eigs_gen(eigval, TE_mat, 6,"lr");
    arma::eigs_gen(eigval, eigvec,TE_mat,20,"lr");
    
//    eigval.print();
    for(int k=0;k<20;k++) std::cout<<std::sqrt(eigval[k]/k0/k0)<<std::endl;
    
    for(i=0;i<Ng;i++)
    {
//        plog<<i*Dz<<" "<<std::real(eigvec(i,0))<<" "<<std::imag(eigvec(i,0))<<std::endl;
        plog<<i*Dz<<" ";
        for(int k=10;k<20;k++)
        {
            plog<<std::real(eigvec(i,k))<<" "<<std::imag(eigvec(i,k))<<" ";
        }
        
        plog<<std::endl;
    }
}
#endif

double Slab_guide::guide_eqn(double k0,double beta,std::string polar_mode)
{
    double bet2=beta*beta;
    double k02=k0*k0;
    
    double d1a=bet2-k02*n1*n1;
    double ga=k02*n2*n2-bet2;
    double d3a=bet2-k02*n3*n3;
    
    if(d1a<0) d1a=0;
    if(ga<0) ga=0;
    if(d3a<0) d3a=0;
    
    using std::sqrt;
    
    double d1=sqrt(d1a);
    double g=sqrt(ga);
    double d3=sqrt(d3a);
    
    double e1=n1*n1;
    double e2=n2*n2;
    double e3=n3*n3;
    
    double sgh=std::sin(g*h);
    double cgh=std::cos(g*h);
    
    if(polar_mode=="TE") return g*sgh-d1*cgh-d3*(cgh+d1/g*sgh);
    else if(polar_mode=="TM") return g/e2*sgh-d1/e1*cgh-d3/e3*(cgh+d1/e1*e2/g*sgh);
    else return 0;
}

Imdouble Slab_guide::guide_eqn_z(double k0,Imdouble beta,std::string polar_mode)
{
    Imdouble bet2=beta*beta;
    double k02=k0*k0;
    
    Imdouble d1a=bet2-k02*n1*n1;
    Imdouble ga=k02*n2*n2-bet2;
    Imdouble d3a=bet2-k02*n3*n3;
    
//    if(d1a<0) d1a=0;
//    if(ga<0) ga=0;
//    if(d3a<0) d3a=0;
    
    using std::sqrt;
    
    Imdouble d1=sqrt(d1a);
    Imdouble g=sqrt(ga);
    Imdouble d3=sqrt(d3a);
    
    double e1=n1*n1;
    double e2=n2*n2;
    double e3=n3*n3;
    
    Imdouble sgh=std::sin(g*h);
    Imdouble cgh=std::cos(g*h);
    return sgh;
    if(polar_mode=="TE") return g*sgh-d1*cgh-d3*(cgh+d1/g*sgh);
    else if(polar_mode=="TM") return g/e2*sgh-d1/e1*cgh-d3/e3*(cgh+d1/e1*e2/g*sgh);
    else return 0;
}

void Slab_guide::show_guiding_char()
{
    int i;
    double k0=2.0*Pi/lambda;
    
    std::ofstream file("modes.txt",std::ios::out|std::ios::app);
    
    std::cout<<"Wavelength: "<<add_unit_u(lambda)<<std::endl;
    std::cout<<"Superstrate: "<<n1<<std::endl;
    std::cout<<"Guide: "<<n2<<std::endl;
    std::cout<<"Substrate: "<<n3<<std::endl;
    std::cout<<"Thickness: "<<add_unit_u(h)<<std::endl<<std::endl;
    std::cout<<"TE:"<<std::endl;
    for(i=0;i<n_TE;i++)
    {
        std::cout<<"Mode: "<<i<<" n_eff: "<<beta_TE[i]/k0<<std::endl;
    }
    std::cout<<"TM:"<<std::endl;
    for(i=0;i<n_TM;i++)
    {
        std::cout<<"Mode: "<<i<<" n_eff: "<<beta_TM[i]/k0<<std::endl;
    }
    std::cout<<"_________________"<<std::endl<<std::endl;
    
    file<<"Wavelength: "<<add_unit_u(lambda)<<std::endl;
    file<<"Superstrate: "<<n1<<std::endl;
    file<<"Guide: "<<n2<<std::endl;
    file<<"Substrate: "<<n3<<std::endl;
    file<<"Thickness: "<<add_unit_u(h)<<std::endl<<std::endl;
    file<<"TE:"<<std::endl;
    for(i=0;i<n_TE;i++)
    {
        file<<"Mode: "<<i<<" n_eff: "<<beta_TE[i]/k0<<std::endl;
    }
    file<<"TM:"<<std::endl;
    for(i=0;i<n_TM;i++)
    {
        file<<"Mode: "<<i<<" n_eff: "<<beta_TM[i]/k0<<std::endl;
    }
    file<<"_________________"<<std::endl<<std::endl;
    
    file.close();
}

double guide_eqn(double k0,double beta,double n1,double n2,double n3,double h,std::string polar_mode)
{
    double bet2=beta*beta;
    double k02=k0*k0;
    
    double d1a=bet2-k02*n1*n1;
    double ga=k02*n2*n2-bet2;
    double d3a=bet2-k02*n3*n3;
    
    if(d1a<0) d1a=0;
    if(ga<0) ga=0;
    if(d3a<0) d3a=0;
    
    using std::sqrt;
    using std::cos;
    using std::sin;
    
    double d1=sqrt(d1a);
    double g=sqrt(ga);
    double d3=sqrt(d3a);
    
    double e1=n1*n1;
    double e2=n2*n2;
    double e3=n3*n3;
    
    if(polar_mode=="TE") return g*sin(g*h)-d1*cos(g*h)-d3*(cos(g*h)+d1/g*sin(g*h));
    else if(polar_mode=="TM") return g/e2*sin(g*h)-d1/e1*cos(g*h)-d3/e3*(cos(g*h)+d1/e1*e2/g*sin(g*h));
    else return 0;
}

bool is_guided_simple(double lambda,double &beta,Slab_guide const &gd,std::string polar_mode,int M_num)
{
    int i,j;
    
    double k0=2.0*Pi/lambda;
    
    double t_bet;
    double bet_max=gd.n2*k0;
    double bet_min=std::max(gd.n1,gd.n3)*k0;
    
    int Nb=10000;
    
    t_bet=bet_max;
    double bet_eq1=guide_eqn(k0,t_bet,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
    
    t_bet=bet_max+(bet_min-bet_max)*1.0/(Nb-1.0);
    double bet_eq2=guide_eqn(k0,t_bet,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
    
    j=0;
    
    for(i=0;i<Nb-1;i++)
    {
        if(bet_eq1*bet_eq2<=0)
        {
            double bet_a=bet_max+(bet_min-bet_max)*i/(Nb-1.0);
            double bet_b=bet_max+(bet_min-bet_max)*(i+1)/(Nb-1.0);
            double bet_c=0,fa,fc;
            
            while(2.0*std::abs((bet_b-bet_a)/(bet_a+bet_b))>1e-6)
            {
                bet_c=(bet_a+bet_b)/2.0;
                fa=guide_eqn(k0,bet_a,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
//                fb=guide_eqn(k0,bet_b,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
                fc=guide_eqn(k0,bet_c,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
                
                if(fc/fa>0) bet_a=bet_c;
                else bet_b=bet_c;
            }
            
            if(j==M_num)
            {
                beta=bet_c;
                return true;
            }
            else
            {
                j++;
            }
        }
        
        t_bet=bet_max+(bet_min-bet_max)*(i+2.0)/(Nb-1.0);
        bet_eq1=bet_eq2;
        bet_eq2=guide_eqn(k0,t_bet,gd.n1,gd.n2,gd.n3,gd.h,polar_mode);
    }
    
    return false;
}

double gd_mode_profile_simp(double lambda,double beta,Slab_guide const &gd,double z,std::string polar_mode,int compo)
{
    double k0=2.0*Pi/lambda;
    double w=k0*c_light;
    double k02=k0*k0;
    double bet2=beta*beta;
            
    double d1a=bet2-k02*gd.n1*gd.n1;
    double ga=k02*gd.n2*gd.n2-bet2;
    double d3a=bet2-k02*gd.n3*gd.n3;
    
    if(d1a<0) d1a=0;
    if(ga<0) ga=0;
    if(d3a<0) d3a=0;
    
    using std::sqrt;
    using std::cos;
    using std::sin;
    using std::exp;
    
    double d1=sqrt(d1a);
    double g=sqrt(ga);
    double d3=sqrt(d3a);
    
    double eps1=gd.n1*gd.n1;
    double eps2=gd.n2*gd.n2;
    double eps3=gd.n3*gd.n3;
    
    eps1*=e0;
    eps2*=e0;
    eps3*=e0;
    
    double Ex=0,Ey=0,Ez=0,Hx=0,Hy=0,Hz=0;
    
    if(z>0)
    {
        Ey=exp(-d1*z);
        
        Hy=exp(-d1*z);
        Ez=-beta/(eps1*w)*Hy;
    }
    else if(z<=0 && z>=-gd.h)
    {
        Ey=cos(g*z)-d1/g*sin(g*z);
        
        Hy=cos(g*z)-d1/(eps1)*(eps2)/g*sin(g*z);
        Ez=-beta/(eps2*w)*Hy;
    }
    else if(z<gd.h)
    {
        Ey=(cos(g*gd.h)+d1/g*sin(g*gd.h))*exp(d3*(z+gd.h));
        
        Hy=(cos(g*gd.h)+d1/(eps1)*(eps2)/g*sin(g*gd.h))*exp(d3*(z+gd.h));
        Ez=-beta/(eps3*w)*Hy;
    }
    
    Hz=beta/(mu0*w)*Ey;
        
    if(polar_mode=="TE")
    {
        if(compo==0) return Hx;
        else if(compo==1) return Ey;
        else if(compo==2) return Hz;
    }
    else if(polar_mode=="TM")
    {
        if(compo==0) return Ex;
        else if(compo==1) return Hy;
        else if(compo==2) return Ez;
    }
    
    return 0;
}

double gd_mode_energy_simp(double lambda,double beta,Slab_guide const &gd,std::string polar_mode)
{
    int k,N=100000;
    
    if(polar_mode=="TE")
    {
        double z,E,H;
        
        double z_min=-11.0*gd.h;
        double z_max=10.0*gd.h;
        
        double Dz=(z_max-z_min)/(N-1.0);
        
        double S=0;
        
        for(k=0;k<N;k++)
        {
            z=z_min+k*Dz;
            
            E=gd_mode_profile_simp(lambda,beta,gd,z,"TE",1);
            H=gd_mode_profile_simp(lambda,beta,gd,z,"TE",2);
            
            S+=E*H;
        }
        
        return S*Dz*0.5;
    }
    else if(polar_mode=="TM")
    {
        double z,E,H;
        
        double z_min=-11.0*gd.h;
        double z_max=10.0*gd.h;
        
        double Dz=(z_max-z_min)/(N-1.0);
        
        double S=0;
        
        for(k=0;k<N;k++)
        {
            z=z_min+k*Dz;
            
            E=gd_mode_profile_simp(lambda,beta,gd,z,"TM",2);
            H=gd_mode_profile_simp(lambda,beta,gd,z,"TM",1);
            
            S+=-E*H;
        }
        
        return S*Dz*0.5;
    }
    
    return 0;
}

Imdouble gd_mode_profile(double lambda,double beta,Slab_guide const &gd,double z,std::string polar_mode,double phase,int compo)
{
    double k0=2.0*Pi/lambda;
    double w=k0*c_light;
    double k02=k0*k0;
    double bet2=beta*beta;
            
    double d1a=bet2-k02*gd.n1*gd.n1;
    double ga=k02*gd.n2*gd.n2-bet2;
    double d3a=bet2-k02*gd.n3*gd.n3;
    
    if(d1a<0) d1a=0;
    if(ga<0) ga=0;
    if(d3a<0) d3a=0;
    
    using std::sqrt;
    using std::cos;
    using std::sin;
    using std::exp;
    
    double d1=sqrt(d1a);
    double g=sqrt(ga);
    double d3=sqrt(d3a);
    
    double eps1=gd.n1*gd.n1;
    double eps2=gd.n2*gd.n2;
    double eps3=gd.n3*gd.n3;
    
    eps1*=e0;
    eps2*=e0;
    eps3*=e0;
    
    Imdouble Ex=0,Hx=0;
    double Ey=0,Ez=0,Hy=0,Hz=0;
    
    if(z>0)
    {
        Hx=-Im*d1/(mu0*w)*exp(-d1*z);
        Ey=exp(-d1*z);
        
        Ex=Im*d1/(eps1*w)*exp(-d1*z);
        Hy=exp(-d1*z);
        Ez=-beta/(eps1*w)*Hy;
    }
    else if(z<=0 && z>=-gd.h)
    {
        Hx=-Im*g/(mu0*w)*(sin(g*z)+d1/g*cos(g*z));
        Ey=cos(g*z)-d1/g*sin(g*z);
        
        Ex=Im*g/(eps2*w)*(sin(g*z)+d1/g*eps2/eps1*cos(g*z));
        Hy=cos(g*z)-d1/(eps1)*(eps2)/g*sin(g*z);
        Ez=-beta/(eps2*w)*Hy;
    }
    else if(z<gd.h)
    {
        Hx=Im*d3/(mu0*w)*(cos(g*gd.h)+d1/g*sin(g*gd.h))*exp(d3*(z+gd.h));
        Ey=(cos(g*gd.h)+d1/g*sin(g*gd.h))*exp(d3*(z+gd.h));
        
        Ex=-Im*d3/(eps3*w)*(cos(g*gd.h)+d1/g*eps2/eps1*sin(g*gd.h))*exp(d3*(z+gd.h));
        Hy=(cos(g*gd.h)+d1/(eps1)*(eps2)/g*sin(g*gd.h))*exp(d3*(z+gd.h));
        Ez=-beta/(eps3*w)*Hy;
    }
    
    Hz=beta/(mu0*w)*Ey;
    
    Imdouble dephas=std::exp(Im*phase);
    
    if(polar_mode=="TE")
    {
        if(compo==0) return Hx*dephas;
        else if(compo==1) return Ey*dephas;
        else if(compo==2) return Hz*dephas;
    }
    else if(polar_mode=="TM")
    {
        if(compo==0) return Ex*dephas;
        else if(compo==1) return Hy*dephas;
        else if(compo==2) return Ez*dephas;
    }
    
    return 0;
}

void modes_dcpl(double lambda,
                 Slab_guide &slb,
                 Grid1<Imdouble> const &Ey,Grid1<double> const &z_Ey,
                 Grid1<Imdouble> const &Hy,Grid1<double> const &z_Hy,
                 Grid1<Imdouble> &coeff_TE,
                 Grid1<Imdouble> &coeff_TM)
{
    int i,j,k;
    
    slb.compute_guiding_char(lambda);
    
    // Decoupling TE
    
    int n_TE=slb.n_TE;
    
    if(n_TE>0) coeff_TE.init(n_TE);
    else coeff_TE.init(1,0);
    
    if(n_TE>0)
    {
        Eigen::Matrix<Imdouble,Eigen::Dynamic,1> fproj,r_coeff;
        Eigen::Matrix<Imdouble,Eigen::Dynamic,Eigen::Dynamic> proj_mat;
        
        fproj.resize(n_TE);
        r_coeff.resize(n_TE);
        proj_mat.resize(n_TE,n_TE);
        
        for(i=0;i<n_TE;i++)
        {
            fproj(i)=0;
            r_coeff(i)=0;
            for(j=0;j<n_TE;j++) proj_mat(i,j)=0;
        }
        
        Grid2<double> base_Ey(Ey.L1(),n_TE);
        
        int Nz=z_Ey.L1();
        
        for(i=0;i<n_TE;i++) for(k=0;k<Nz;k++)
            base_Ey(k,i)=gd_mode_profile_simp(lambda,slb.beta_TE[i],slb,z_Ey[k],"TE",1);
        
        for(i=0;i<n_TE;i++){ for(k=0;k<Nz;k++)
        {
            fproj(i)+=base_Ey(k,i)*Ey[k];
        }}
        
        for(i=0;i<n_TE;i++){ for(j=0;j<n_TE;j++)
        {
            for(k=0;k<Nz;k++)
            {
                proj_mat(i,j)+=base_Ey(k,i)*base_Ey(k,j);
            }
        }}
        
        r_coeff=proj_mat.fullPivLu().solve(fproj);
        
        for(i=0;i<n_TE;i++) coeff_TE[i]=r_coeff(i);
    }
    
    // Decoupling TM
    
    int n_TM=slb.n_TM;
    
    if(n_TM>0) coeff_TM.init(n_TM);
    else coeff_TM.init(1,0);
    
    if(n_TM>0)
    {
        Eigen::Matrix<Imdouble,Eigen::Dynamic,1> fproj,r_coeff;
        Eigen::Matrix<Imdouble,Eigen::Dynamic,Eigen::Dynamic> proj_mat;
        
        fproj.resize(n_TM);
        r_coeff.resize(n_TM);
        proj_mat.resize(n_TM,n_TM);
        
        for(i=0;i<n_TM;i++)
        {
            fproj(i)=0;
            r_coeff(i)=0;
            for(j=0;j<n_TM;j++) proj_mat(i,j)=0;
        }
        
        Grid2<double> base_Hy(Hy.L1(),n_TM);
        
        int Nz=z_Hy.L1();
        
        for(i=0;i<n_TM;i++) for(k=0;k<Nz;k++)
            base_Hy(k,i)=gd_mode_profile_simp(lambda,slb.beta_TM[i],slb,z_Hy[k],"TM",1);
        
        for(i=0;i<n_TM;i++){ for(k=0;k<Nz;k++)
        {
            fproj(i)+=base_Hy(k,i)*Hy[k];
        }}
        
        for(i=0;i<n_TM;i++){ for(j=0;j<n_TM;j++)
        {
            for(k=0;k<Nz;k++)
            {
                proj_mat(i,j)+=base_Hy(k,i)*base_Hy(k,j);
            }
        }}
        
        r_coeff=proj_mat.fullPivLu().solve(fproj);
        
        for(i=0;i<n_TM;i++) coeff_TM[i]=r_coeff(i);
    }
}

void modes_dcpl(double lambda,
                 Slab_guide &slb,
                 Grid1<Imdouble> const &Ey,Grid1<double> const &z_Ey,
                 Grid1<Imdouble> const &Hy,Grid1<double> const &z_Hy,
                 Grid1<Imdouble> &coeff_TE,
                 Grid1<Imdouble> &coeff_TM,
                 double &resi_TE,
                 double &resi_TM)
{
    int i,j,k;
    
    slb.compute_guiding_char(lambda);
    
    // Decoupling TE
    
    int n_TE=slb.n_TE;
    
    if(n_TE>0) coeff_TE.init(n_TE);
    else coeff_TE.init(1,0);
    
    resi_TE=resi_TM=0;
    
    double mfield=0;
        
    if(n_TE>0)
    {
        Eigen::Matrix<Imdouble,Eigen::Dynamic,1> fproj,r_coeff;
        Eigen::Matrix<Imdouble,Eigen::Dynamic,Eigen::Dynamic> proj_mat;
        
        fproj.resize(n_TE);
        r_coeff.resize(n_TE);
        proj_mat.resize(n_TE,n_TE);
        
        for(i=0;i<n_TE;i++)
        {
            fproj(i)=0;
            r_coeff(i)=0;
            for(j=0;j<n_TE;j++) proj_mat(i,j)=0;
        }
        
        Grid2<double> base_Ey(Ey.L1(),n_TE);
        
        int Nz=z_Ey.L1();
        
        for(k=0;k<Nz;k++)
            mfield=std::max(mfield,std::abs(Ey[k]));
        
        for(i=0;i<n_TE;i++) for(k=0;k<Nz;k++)
            base_Ey(k,i)=gd_mode_profile_simp(lambda,slb.beta_TE[i],slb,z_Ey[k],"TE",1);
        
        for(i=0;i<n_TE;i++){ for(k=0;k<Nz;k++)
        {
            fproj(i)+=base_Ey(k,i)*Ey[k];
        }}
        
        for(i=0;i<n_TE;i++){ for(j=0;j<n_TE;j++)
        {
            for(k=0;k<Nz;k++)
            {
                proj_mat(i,j)+=base_Ey(k,i)*base_Ey(k,j);
            }
        }}
        
        r_coeff=proj_mat.fullPivLu().solve(fproj);
        
        for(i=0;i<n_TE;i++) coeff_TE[i]=r_coeff(i);
        
        for(k=0;k<Nz;k++)
        {
            Imdouble tmp=Ey[k];
            
            for(i=0;i<n_TE;i++) tmp-=coeff_TE[i]*base_Ey(k,i);
            
            resi_TE+=std::abs(tmp);
        }
        
        resi_TE/=static_cast<double>(Nz);
    }
    
    // Decoupling TM
    
    int n_TM=slb.n_TM;
    
    if(n_TM>0) coeff_TM.init(n_TM);
    else coeff_TM.init(1,0);
    
    if(n_TM>0)
    {
        Eigen::Matrix<Imdouble,Eigen::Dynamic,1> fproj,r_coeff;
        Eigen::Matrix<Imdouble,Eigen::Dynamic,Eigen::Dynamic> proj_mat;
        
        fproj.resize(n_TM);
        r_coeff.resize(n_TM);
        proj_mat.resize(n_TM,n_TM);
        
        for(i=0;i<n_TM;i++)
        {
            fproj(i)=0;
            r_coeff(i)=0;
            for(j=0;j<n_TM;j++) proj_mat(i,j)=0;
        }
        
        Grid2<double> base_Hy(Hy.L1(),n_TM);
        
        int Nz=z_Hy.L1();
        
        for(k=0;k<Nz;k++)
            mfield=std::max(mfield,mu0*c_light*std::abs(Hy[k]));
        
        for(i=0;i<n_TM;i++) for(k=0;k<Nz;k++)
            base_Hy(k,i)=gd_mode_profile_simp(lambda,slb.beta_TM[i],slb,z_Hy[k],"TM",1);
        
        for(i=0;i<n_TM;i++){ for(k=0;k<Nz;k++)
        {
            fproj(i)+=base_Hy(k,i)*Hy[k];
        }}
        
        for(i=0;i<n_TM;i++){ for(j=0;j<n_TM;j++)
        {
            for(k=0;k<Nz;k++)
            {
                proj_mat(i,j)+=base_Hy(k,i)*base_Hy(k,j);
            }
        }}
        
        r_coeff=proj_mat.fullPivLu().solve(fproj);
        
        for(i=0;i<n_TM;i++) coeff_TM[i]=r_coeff(i);
        
        for(k=0;k<Nz;k++)
        {
            Imdouble tmp=Hy[k];
            
            for(i=0;i<n_TM;i++) tmp-=coeff_TM[i]*base_Hy(k,i);
            
            resi_TM+=std::abs(tmp);
        }
        
        resi_TM/=static_cast<double>(Nz);
    }
    
    resi_TE/=mfield;
    resi_TM*=mu0*c_light/mfield;
}
