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
#include <thread_utils.h>

extern const double Pi;
extern const Imdouble Im;
extern std::ofstream plog;

//###############
//     ChpIn
//###############

ChpIn::ChpIn()
    :Nthreads(4),
     lambda(500e-9),
     inc(0),
     pol(0),
     dir(0),
     n_ind(1.0)
{
    setfields();
}

double ChpIn::Ex(double x,double y,double z,double t){ return (this->*pEx)(x,y,z,t); }
double ChpIn::Ey(double x,double y,double z,double t){ return (this->*pEy)(x,y,z,t); }
double ChpIn::Ez(double x,double y,double z,double t){ return (this->*pEz)(x,y,z,t); }
double ChpIn::Hx(double x,double y,double z,double t){ return (this->*pHx)(x,y,z,t); }
double ChpIn::Hy(double x,double y,double z,double t){ return (this->*pHy)(x,y,z,t); }
double ChpIn::Hz(double x,double y,double z,double t){ return (this->*pHz)(x,y,z,t); }

double ChpIn::PulseEx(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Ex0*std::exp(-b*b)*std::cos(a);
}

double ChpIn::PulseEy(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Ey0*std::exp(-b*b)*std::cos(a);
}

double ChpIn::PulseEz(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Ez0*std::exp(-b*b)*std::cos(a);
}

double ChpIn::PulseHx(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Hx0*std::exp(-b*b)*std::cos(a);
}

double ChpIn::PulseHy(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Hy0*std::exp(-b*b)*std::cos(a);
}

double ChpIn::PulseHz(double x,double y,double z,double t)
{
    double p=kx*x+ky*y+kz*z;
    double a=p-omega*t;
    double b=(1.0/k*p-c_light*t/n_ind+C1)/dz;
    return Hz0*std::exp(-b*b)*std::cos(a);
}

///####################
///  Spectral Phase
///####################

double ChpIn::PhaseEx(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcEx);
}

double ChpIn::PhaseEy(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcEy);
}

double ChpIn::PhaseEz(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcEz);
}

double ChpIn::PhaseHx(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcHx);
}

double ChpIn::PhaseHy(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcHy);
}

double ChpIn::PhaseHz(double x,double y,double z,double t)
{
    return std::real(std::exp(kx*x*Im)*pcHz);
}

double ChpIn::PhaseEx_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcEx);
}

double ChpIn::PhaseEy_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcEy);
}

double ChpIn::PhaseEz_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcEz);
}

double ChpIn::PhaseHx_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcHx);
}

double ChpIn::PhaseHy_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcHy);
}

double ChpIn::PhaseHz_D(double x,double y,double z,double t)
{
    return std::imag(std::exp(kx*x*Im)*pcHz);
}

void ChpIn::init_phase(double li,double lf,double kxi,double poli,double phase_i,double n_ind_i)
{
    int l;
    
    n_ind=n_ind_i;
    
    double wi=2.0*Pi*c_light/li;
    double wf=2.0*Pi*c_light/lf;
    
    std::cout<<li<<std::endl;
    std::cout<<lf<<std::endl;
        
    pcw0=(wi+wf)/2.0;
    pcdw=std::abs(0.5*(wi-pcw0));
    pcdw=pcw0/2.0;
    
    kx=kxi;
    
    pol=poli*Pi/180.0;
    
    Nl=40000;
    
    pc_w.init(Nl,0);
    pc_kn.init(Nl,0);
    pc_sp.init(Nl,0);
    pc_srt.init(Nl,0);
    
    for(l=0;l<Nl;l++)
    {
        using std::pow;
        using std::exp;
        using std::sin;
        using std::sqrt;
        
        double w=3*pcw0*l/(Nl-1.0);
        double kn=w*n_ind/c_light;
        
        Imdouble asrt=kn*kn-kx*kx;
        
        double ang_damp=1.0;
        
        //double ang=(std::asin(kx/(w/c_light))/(Pi/2.0));
        double ang=std::abs(kx/kn);
        
        if(ang<1.0)
        {
            ang_damp=1.0-(3.0*ang*ang-2.0*ang*ang*ang);
        }
        else
        {
            ang_damp=0.0;
            ang=0.0;
        }
        
        pc_w[l]=w;
        pc_kn[l]=kn;
        pc_sp[l]=pow(sin(w/pcw0*Pi/2.0),2.0)*exp(-pow((w-pcw0)/pcdw,2.0))*ang_damp;
        pc_srt[l]=sqrt(asrt);
    }
    
    pc_sp.normalize();
    
    double spol=std::sin(pol);
    double cpol=std::cos(pol);
    
    Ex0=c_light/n_ind*spol/90.0;
    Ey0=cpol/90.0;
    Ez0=c_light/n_ind*spol*kx/90.0;
    Hx0=cpol/(mu0)/90.0;
    Hy0=-spol/(mu0*c_light)/90.0*n_ind;
    Hz0=cpol*kx/(mu0)/90.0;
    
    if(phase_i==0)
    {
        pEx=&ChpIn::PhaseEx;
        pEy=&ChpIn::PhaseEy;
        pEz=&ChpIn::PhaseEz;
        pHx=&ChpIn::PhaseHx;
        pHy=&ChpIn::PhaseHy;
        pHz=&ChpIn::PhaseHz;
        
        std::cout<<"EM field: Real part initialized"<<std::endl;
    }
    else
    {
        pEx=&ChpIn::PhaseEx_D;
        pEy=&ChpIn::PhaseEy_D;
        pEz=&ChpIn::PhaseEz_D;
        pHx=&ChpIn::PhaseHx_D;
        pHy=&ChpIn::PhaseHy_D;
        pHz=&ChpIn::PhaseHz_D;
        
        std::cout<<"EM field: Imaginary part initialized"<<std::endl;
    }
}

void ChpIn::phase_precomp_E_aux(PPE_params &params)
{
    int l;
    Imdouble zexp;
    
    int l1=params.l1;
    int l2=params.l2;
    double z=params.z;
    double t=params.t;
    
    Imdouble &t_pcEx=params.t_pcEx;
    Imdouble &t_pcEy=params.t_pcEy;
    Imdouble &t_pcEz=params.t_pcEz;
    
    t_pcEx=t_pcEy=t_pcEz=0;
    
    for(l=l1;l<l2;l++)
    {
        double const &w=pc_w[l];
        double const &sp=pc_sp[l];
        Imdouble const &srt=pc_srt[l];
        
        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
        
        t_pcEx+=srt/w*zexp;
        t_pcEy+=zexp;
        t_pcEz+=1.0/w*zexp;
    }
}

void ChpIn::phase_precomp_E(double z,double t)
{
//    int l;
//    Imdouble zexp;
//    
//    phase_reset_E();
//    
//    PPE_params tparams[Nthreads];
//    
//    tparams[0].set(this,1,Nl/Nthreads,z,t);
//    for(l=1;l<Nthreads;l++) tparams[l].set(this,(l*Nl)/Nthreads,((l+1)*Nl)/Nthreads,z,t);
//    
//    thread_exec<PPE_params>(tparams,Nthreads);
//        
//    for(l=0;l<Nthreads;l++)
//    {
//        pcEx+=tparams[l].t_pcEx;
//        pcEy+=tparams[l].t_pcEy;
//        pcEz+=tparams[l].t_pcEz;
//    }
//    
//    pcEx*=Ex0;
//    pcEy*=Ey0;
//    pcEz*=Ez0;
}

//void ChpIn::phase_precomp_E(double z,double t)
//{
//    int l;
//    Imdouble zexp;
//    
//    pcEx=pcEy=pcEz=0;
//    
//    for(l=1;l<Nl;l++)
//    {
//        double &w=pc_w[l];
//        double &sp=pc_sp[l];
//        Imdouble &srt=pc_srt[l];
//        
//        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
//        
//        pcEx+=srt/w*zexp;
//        pcEy+=zexp;
//        pcEz+=1.0/w*zexp;
//    }
//        
//    pcEx*=Ex0;
//    pcEy*=Ey0;
//    pcEz*=Ez0;
//}

void ChpIn::phase_precomp_Ex(double z,double t)
{
    int l;
    Imdouble zexp;
    
    pcEx=0;
    
    for(l=1;l<Nl;l++)
    {
        double &w=pc_w[l]; double &sp=pc_sp[l];
        Imdouble &srt=pc_srt[l];
        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
        pcEx+=srt/w*zexp;
    }
    
    pcEx*=Ex0;
}

void ChpIn::phase_precomp_Ey(double z,double t)
{
    int l;
    Imdouble zexp;
    
    pcEy=0;
    
    for(l=1;l<Nl;l++)
    {
        double &w=pc_w[l]; double &sp=pc_sp[l];
        Imdouble &srt=pc_srt[l];
        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
        pcEy+=zexp;
    }
    
    pcEy*=Ey0;
}


void ChpIn::phase_precomp_Ez(double z,double t)
{
    int l;
    Imdouble zexp;
    
    pcEz=0;
    
    for(l=1;l<Nl;l++)
    {
        double &w=pc_w[l]; double &sp=pc_sp[l];
        Imdouble &srt=pc_srt[l];
        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
        pcEz+=1.0/w*zexp;
    }
    
    pcEz*=Ez0;
}

void ChpIn::phase_precomp_H_aux(PPH_params &params)
{
    int l;
    Imdouble zexp;
    
    int l1=params.l1;
    int l2=params.l2;
    double z=params.z;
    double t=params.t;
    
    Imdouble &t_pcHx=params.t_pcHx;
    Imdouble &t_pcHy=params.t_pcHy;
    Imdouble &t_pcHz=params.t_pcHz;
    
    t_pcHx=t_pcHy=t_pcHz=0;
    
    for(l=l1;l<l2;l++)
    {
        double const &w=pc_w[l];
        double const &sp=pc_sp[l];
        Imdouble const &srt=pc_srt[l];
        
        zexp=std::exp(-(z*srt+w*t)*Im)*sp;
        
        t_pcHx+=srt/w*zexp;
        t_pcHy+=zexp;
        t_pcHz+=1.0/w*zexp;
    }
}

void ChpIn::phase_precomp_H(double z,double t)
{
//    int l;
//    Imdouble zexp;
//    
//    phase_reset_H();
//    
//    PPH_params tparams[Nthreads];
//    
//    tparams[0].set(this,1,Nl/Nthreads,z,t);
//    for(l=1;l<Nthreads;l++) tparams[l].set(this,(l*Nl)/Nthreads,((l+1)*Nl)/Nthreads,z,t);
//    
//    thread_exec<PPH_params>(tparams,Nthreads);
//        
//    for(l=0;l<Nthreads;l++)
//    {
//        pcHx+=tparams[l].t_pcHx;
//        pcHy+=tparams[l].t_pcHy;
//        pcHz+=tparams[l].t_pcHz;
//    }
//    
//    pcHx*=Hx0;
//    pcHy*=Hy0;
//    pcHz*=Hz0;
}

//void ChpIn::phase_precomp_H(double z,double t)
//{
//    int l;
//    Imdouble zexp;
//    
//    pcHx=pcHy=pcHz=0;
//    
//    for(l=1;l<Nl;l++)
//    {
//        double &w=pc_w[l];
//        double &sp=pc_sp[l];
//        Imdouble &srt=pc_srt[l];
//        
//        zexp=sp*std::exp(-(z*srt+w*t)*Im);
//        
//        pcHx+=srt/w*zexp;
//        pcHy+=zexp;
//        pcHz+=1.0/w*zexp;
//    }
//    
//    pcHx*=Hx0;
//    pcHy*=Hy0;
//    pcHz*=Hz0;
//}

void ChpIn::phase_reset_E()
{
    pcEx=pcEy=pcEz=0;
}

void ChpIn::phase_reset_H()
{
    pcHx=pcHy=pcHz=0;
}

void ChpIn::init_pulse(double li,double lf,double inci,double poli,double diri,
                       double startxi,double startyi,double startzi,double n_ind_i)
{
    double wi=2.0*Pi*c_light/li;
    double wf=2.0*Pi*c_light/lf;
    
    double w0=(wi+wf)/2.0;
    double l0=2.0*Pi*c_light/w0;
    double dw=std::abs(0.5*(wi-w0));
    
    lambda=l0;
    n_ind=n_ind_i;
    
    inc=inci*Pi/180.0;
    pol=poli*Pi/180.0;
    dir=diri*Pi/180.0;
    
    dz=2.0*c_light/dw/n_ind;
    
    startx=startxi;
    starty=startyi;
    startz=startzi;
    
    setfields();
    
    double bet=1e-4;
    
    C1=-1.0/k*(kx*startx+ky*starty+kz*startz)+dz*std::sqrt(-std::log(bet));
    
    pEx=&ChpIn::PulseEx;
    pEy=&ChpIn::PulseEy;
    pEz=&ChpIn::PulseEz;
    pHx=&ChpIn::PulseHx;
    pHy=&ChpIn::PulseHy;
    pHz=&ChpIn::PulseHz;
}

void ChpIn::setfields()
{
    k=2.0*Pi/lambda*n_ind;
    omega=2.0*Pi*c_light/lambda;
    
    double incr=Pi/2.0-inc;
    
    using std::cos;
    using std::sin;
    
    double Ca=cos(incr); double Sa=sin(incr);
    double Cb=cos(dir);  double Sb=sin(dir);
    double Cc=cos(pol);  double Sc=sin(pol);
    
    kx=k*(Ca*Cb);
    ky=k*(Ca*Sb);
    kz=k*(-Sa);
    
    double amp=1;
    
    Ex0=(Sa*Cb*Sc-Sb*Cc)*amp;
    Ey0=(Sa*Sb*Sc+Cb*Cc)*amp;
    Ez0=Ca*Sc*amp;
    
    double tmp=1.0/(c_light*mu0)*amp*n_ind;
    
    Hx0=tmp*(Sa*Cb*Cc+Sb*Sc);
    Hy0=tmp*(Sa*Sb*Cc-Cb*Sc);
    Hz0=tmp*(Ca*Cc);
}

void ChpIn::show()
{
    using std::cout;
    using std::endl;
    using std::pow;
    
    cout<<"kx: "<<kx/k<<endl<<"ky: "<<ky/k<<endl<<"kz: "<<kz/k<<endl;
    cout<<"Norm1: "<<pow(kx/k,2.0)+pow(ky/k,2.0)+pow(kz/k,2.0)<<endl;
    cout<<"Ex: "<<Ex0<<endl<<"Ey: "<<Ey0<<endl<<"Ez: "<<Ez0<<endl;
    cout<<"Norm2: "<<Ex0*Ex0+Ey0*Ey0+Ez0*Ez0<<endl;
    cout<<"Hx: "<<Hx0<<endl<<"Hy: "<<Hy0<<endl<<"Hz: "<<Hz0<<endl;
    cout<<"Norm3: "<<Hx0*Hx0+Hy0*Hy0+Hz0*Hz0<<endl;
}
