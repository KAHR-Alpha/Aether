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

#include <sources.h>
#include <phys_constants.h>
#include <phys_tools.h>


extern const Imdouble Im;
extern std::ofstream plog;

Bloch_Wideband::Bloch_Wideband(int x1_,int x2_,int y1_,int y2_,int z1_,int z2_,
                               double kx_,double ky_,AngleRad polar_)
    :Source(x1_,x2_,y1_,y2_,z1_,z2_),
     t_offset(0),
     kx(kx_), ky(ky_),
     polar(polar_.radian()),
     safe_angle(Degree(75)), cut_angle(Degree(85))
{
    kxy=std::sqrt(kx*kx+ky*ky);
}

Bloch_Wideband::~Bloch_Wideband()
{
}

void Bloch_Wideband::deep_link(FDTD const &fdtd)
{
    eps_inf=fdtd.mats[fdtd.matsgrid(x1,y1,z2)].ei;
}

void Bloch_Wideband::get_E(Grid2<double> &Ex,
                           Grid2<double> &Ey,
                           Grid2<double> &Ez,
                           double z,double t)
{
    int i,j,l;
    
    Imdouble Ex_c=0,Ey_c=0,Ez_c=0;
    Imdouble Ex_inj,Ey_inj,Ez_inj;
    
    Imdouble im_arg;
    
    for(l=0;l<Nl;l++)
    {
        im_arg=std::exp(-(kz[l]*z+w[l]*(t+Dt*t_offset))*Im);
        
        Ex_c+=E_base[l].x*im_arg;
        Ey_c+=E_base[l].y*im_arg;
    }
    
    for(l=0;l<Nl;l++)
    {
        im_arg=std::exp(-(kz[l]*(z+Dz/2.0)+w[l]*(t+Dt*t_offset))*Im);
        
        Ez_c+=E_base[l].z*im_arg;
    }
    
    for(i=0;i<Nx;i++)
    {
        for(j=0;j<Ny;j++)
        {
            Ex_inj=Ex_c*std::exp(((i+0.5)*Dx*kx+j*Dy*ky)*Im);
            Ey_inj=Ey_c*std::exp((i*Dx*kx+(j+0.5)*Dy*ky)*Im);
            Ez_inj=Ez_c*std::exp((i*Dx*kx+j*Dy*ky)*Im);
            
            Ex(i,j)=Ex_inj.real();
            Ey(i,j)=Ey_inj.real();
            Ez(i,j)=Ez_inj.real();
        }
    }
}

void Bloch_Wideband::initialize()
{
    int l;
    
    double w_max=m_to_rad_Hz(lambda_min);
    double w_min=m_to_rad_Hz(lambda_max);
    
    double w_cut=kxy*c_light/(std::sqrt(eps_inf)*std::sin(cut_angle));
    double w_safe=kxy*c_light/(std::sqrt(eps_inf)*std::sin(safe_angle));
    
    double xs=dichotomy<double,double,double>(0,1,value_s_curve,0,1,std::sqrt(0.1),1e-6);
    
    w_min=std::max(w_min,w_safe);
    
    double w_med=0.5*(w_min+w_max);
    
    // w_cut --- [w_out=xs*(w_med-w_min+w_out)] | w_min ---- w_med ---- w_max
    
    double w_out=xs/(1.0-xs)*(w_med-w_min);
    
    double w1m,w1p,w2m,w2p;
    
    if(w_min-w_cut>w_out)
    {
        w1m=w_min-w_out;
        w1p=w_med;
        w2m=w_med;
        w2p=w_max+w_out;
    }
    else
    {
        w_out=w_min-w_cut;
        double w_tmp=w_out/xs;
        
        w1m=w_cut;
        w1p=w1m+w_tmp;
        w2p=w_max+w_out;
        w2m=w2p-w_tmp;
    }
    
    Nl=30001;
    
    w.resize(Nl);
    linspace(w,w1m,w2p);
    
    lambda.resize(Nl);
    kn.init(Nl);
    kz.init(Nl);
    Sp.init(Nl);
    E_base.init(Nl);
    H_base.init(Nl);
    
    double SEx=0,SEy=0,SEz=0;
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=2.0*Pi*c_light/w[l];
        kn[l]=std::sqrt(eps_inf)*w[l]/c_light;
        kz[l]=std::sqrt(kn[l]*kn[l]-kxy*kxy);
        
        Sp[l]=s_curve(w[l],w1m,w1p)*s_curve(w[l],w2p,w2m);
        
        Vector3 k_vec(kx,ky,-kz[l]),
                p_vec(-ky/kxy,kx/kxy,0);
        
        if(kx==0 && ky==0) p_vec=Vector3(0,1,0);
        
        Vector3 kvp;
        kvp.crossprod(k_vec,p_vec);
        
        Vector3 t_vec=kvp,kvt;
        
        t_vec.normalize();
        kvt.crossprod(k_vec,t_vec);
        
        E_base[l]=Sp[l]*(std::cos(polar)*p_vec+std::sin(polar)*t_vec);
        H_base[l]=Sp[l]/(w[l]*mu0)*(std::cos(polar)*kvp+std::sin(polar)*kvt);
        
        SEx+=E_base[l].x;
        SEy+=E_base[l].y;
        SEz+=E_base[l].z;
    }
    
    double Snorm=std::sqrt(SEx*SEx+SEy*SEy+SEz*SEz);
    
    for(l=0;l<Nl;l++)
    {
        Sp[l]/=Snorm;
        E_base[l]/=Snorm;
        H_base[l]/=Snorm;
    }
    
    Snorm=1.0;
    
    Imdouble im_arg;
    ImVector3 E;
    
    while(Snorm>=1e-4)
    {
        E=ImVector3(0,0,0);
        
        for(l=0;l<Nl;l++)
        {
            im_arg=std::exp(-(kz[l]*z2*Dz+w[l]*Dt*t_offset)*Im);
            E+=im_arg*E_base[l];
        }
        
        Snorm=E.norm();
        
        t_offset+=100;
    }
    
    t_offset=-2*t_offset;
    
//    for(int t=0;t<15000;t++)
//    {
//        E=ImVector3(0,0,0);
//        
//        for(l=0;l<Nl;l++)
//        {
////            im_arg=std::exp(-(kz[l]*z2*Dz+w[l]*(t-t_offset)*Dt)*Im);
//            im_arg=std::exp(-(w[l]*(t+0*t_offset)*Dt)*Im);
//            E+=im_arg*E_base[l];
//        }
//        
//        plog<<t<<" "<<E.x.real()<<" "<<E.y.real()<<" "<<E.z.real()<<std::endl;
//    }
//    
//    std::system("pause");
}

void Bloch_Wideband::inject_E(FDTD &real_fdtd,FDTD &imag_fdtd)
{
    int i,j,l;
    
    Imdouble Hx=0,Hy=0;
    Imdouble Hx_inj,Hy_inj;
    
    Imdouble im_arg;
    
    for(l=0;l<Nl;l++)
    {
        im_arg=std::exp(-(kz[l]*(z2-0.5)*Dz+w[l]*Dt*(step-0.5+t_offset))*Im);
        
        Hx+=H_base[l].x*im_arg;
        Hy+=H_base[l].y*im_arg;
    }
    
    double tmp1,tmp2,C2z;
    real_fdtd.mats[real_fdtd.matsgrid(0,0,z2)].coeffsX(tmp1,tmp2,C2z);
    
    for(i=0;i<Nx;i++)
    {
        for(j=0;j<Ny;j++)
        {
            Hx_inj=Hx*std::exp((i*Dx*kx+(j+0.5)*Dy*ky)*Im);
            Hy_inj=Hy*std::exp(((i+0.5)*Dx*kx+j*Dy*ky)*Im);
            
            real_fdtd.Ex(i,j,z2)-=C2z*Hy_inj.real();
            imag_fdtd.Ex(i,j,z2)-=C2z*Hy_inj.imag();
            
            real_fdtd.Ey(i,j,z2)+=C2z*Hx_inj.real();
            imag_fdtd.Ey(i,j,z2)+=C2z*Hx_inj.imag();
        }
    }
}

void Bloch_Wideband::inject_H(FDTD &real_fdtd,FDTD &imag_fdtd)
{
    int i,j,l;
    
    Imdouble Ex=0,Ey=0;
    Imdouble Ex_inj,Ey_inj;
    
    Imdouble im_arg;
    
    for(l=0;l<Nl;l++)
    {
        im_arg=std::exp(-(kz[l]*(z2)*Dz+w[l]*Dt*(step+t_offset))*Im);
        
        Ex+=E_base[l].x*im_arg;
        Ey+=E_base[l].y*im_arg;
    }
    
    for(i=0;i<Nx;i++)
    {
        for(j=0;j<Ny;j++)
        {
            Ex_inj=Ex*std::exp(((i+0.5)*Dx*kx+j*Dy*ky)*Im);
            Ey_inj=Ey*std::exp((i*Dx*kx+(j+0.5)*Dy*ky)*Im);
            
            real_fdtd.Hx(i,j,z2-1)+=real_fdtd.dtdmz*Ey_inj.real();
            imag_fdtd.Hx(i,j,z2-1)+=imag_fdtd.dtdmz*Ey_inj.imag();
            
            real_fdtd.Hy(i,j,z2-1)-=real_fdtd.dtdmz*Ex_inj.real();
            imag_fdtd.Hy(i,j,z2-1)-=imag_fdtd.dtdmz*Ex_inj.imag();
        }
    }
    
    step+=1;
}

void Bloch_Wideband::set_cut_angle(AngleRad const &safe_angle_,AngleRad const &cut_angle_)
{
    safe_angle=safe_angle_;
    cut_angle=cut_angle_;
    
    if(safe_angle.degree()>cut_angle.degree()-1.0) safe_angle.degree(cut_angle.degree()-1.0);
}
