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

#include <bitmap3.h>
#include <fdtd_core.h>
#include <filehdl.h>
#include <string_tools.h>


extern const Imdouble Im;
extern std::ofstream plog;

//###############
//     FDTD
//###############

std::filesystem::path FDTD::add_prefix(std::string const &str_)
{
    return directory/(prefix+str_);
}

//void FDTD::bufread(Grid3<double> &G,int Nmem,std::string fs)
//{
//    int j,k,t;
//    
//    std::cout<<"Read "<<fs<<std::endl;
//    std::ifstream file(fs.c_str(),std::ios::in|std::ios::binary);
//        
//    for(t=0;t<Nmem;t++)
//    {
//        for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//        {
//            file>>G(j,k,t);
//        }}
//    }
//    
//    file.close();
//}
//
//void FDTD::bufwrite(Grid3<double> &G,int Nmem,std::string fs)
//{
//    int j,k,t;
//    
//    std::cout<<"Write "<<fs<<std::endl;
//    std::ofstream file(fs.c_str(),std::ios::out|std::ios::trunc);
//    
//    for(t=0;t<Nmem;t++)
//    {
//        for(j=0;j<Ny;j++)
//        {
//            std::stringstream sbuf;
//            sbuf.precision(10);
//            for(k=0;k<Nz;k++)
//            {
//                sbuf<<G(j,k,t)<<" ";
//            }
//            
//            file<<sbuf.str();
//        }
//    }
//    
//    file.close();
//}

void FDTD::bufread(Grid3<double> &G,int Nmem,std::string fs)
{
    int j,k,t;
    
    std::cout<<"Read "<<fs<<std::endl;
    std::ifstream file(fs.c_str(),std::ios::in|std::ios::binary);
    
    double b;
    
    for(t=0;t<Nmem;t++)
    {
        for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
        {
            file.read(reinterpret_cast<char*>(&b),sizeof(double));
            G(j,k,t)=b;
        }}
    }
    
    file.close();
}

void FDTD::bufwrite(Grid3<double> &G,int Nmem,std::string fs)
{
    int j,k,t;
    
    std::cout<<"Write "<<fs<<std::endl;
    std::ofstream file(fs.c_str(),std::ios::out|std::ios::trunc|std::ios::binary);
    
    double b;
    
    for(t=0;t<Nmem;t++)
    {
        for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
        {
            b=G(j,k,t);
            file.write(reinterpret_cast<char*>(&b),sizeof(double));
            
        }}
    }
    
    file.close();
}

double FDTD::compute_poynting_box(int i1,int i2,int j1,int j2,int k1,int k2) const
{
    double R=0;
    
    R+=compute_poynting_X(j1,j2,k1,k2,i1,-1);
    R+=compute_poynting_X(j1,j2,k1,k2,i2,1);
    
    R+=compute_poynting_Y(i1,i2,k1,k2,j1,-1);
    R+=compute_poynting_Y(i1,i2,k1,k2,j2,1);
    
    R+=compute_poynting_Z(i1,i2,j1,j2,k1,-1);
    R+=compute_poynting_Z(i1,i2,j1,j2,k2,1);
    
    return R;
}

double FDTD::compute_poynting_X(int j1,int j2,int k1,int k2,int pos_x,int sgn) const
{
    int i,j,k;
        
    double R=0;
    
    i=pos_x;
    
    for(j=j1;j<j2;j++)
    {
        for(k=k1;k<k2;k++)
        {
            R+=local_Px(i,j,k);
        }
    }
    
    R*=sgn*Dy*Dz;
    return R;
}

double FDTD::compute_poynting_Y(int i1,int i2,int k1,int k2,int pos_y,int sgn) const
{
    int i,j,k;
    
    double R=0;
    
    j=pos_y;
    
    for(i=i1;i<i2;i++)
    {
        for(k=k1;k<k2;k++)
        {
            R+=local_Py(i,j,k);
        }
    }
    
    R*=sgn*Dx*Dz;
    return R;
}

double FDTD::compute_poynting_Z(int i1,int i2,int j1,int j2,int pos_z,int sgn) const
{
    int i,j,k;
    
    double R=0;
    
    k=pos_z;
    
    for(i=i1;i<i2;i++)
    {
        for(j=j1;j<j2;j++)
        {
            R+=local_Pz(i,j,k);
        }
    }
    
    R*=sgn*Dx*Dy;
    return R;
}

void FDTD::draw(int t,int vmode,int pos_x,int pos_y,int pos_z)
{
    int i,j,k;
    
    std::stringstream K;
    K<<"render/render";
    K<<t;
    K<<".png";
    
    using std::abs;
    using std::exp;
    using std::max;
    
    double max_Ex=1e-80,max_Ey=1e-80,max_Ez=1e-80,max_E=1e-80;
        
    int span1=Nx,span2=Nz;
    
    if(vmode==1) { span1=Ny; span2=Nz; }
    if(vmode==2) { span1=Nx; span2=Ny; }
    
    span1=2*span1+4;
    span2=3*span2+8;
    
    Bitmap im(span1,span2);
        
    if(vmode==0)
    {
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(Ex(i,pos_y,k)));
            max_Ey=max(max_Ey,abs(Ey(i,pos_y,k)));
            max_Ez=max(max_Ez,abs(Ez(i,pos_y,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            im.degra(i,k,1.0-exp(-1.0*abs(Ez(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k,1.0-exp(-1.0*abs(Ez(i,pos_y,k))/max_E),0,1.0);
            
            im.degra(i,k+Nz+4,1.0-exp(-1.0*abs(Ey(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k+Nz+4,1.0-exp(-1.0*abs(Ey(i,pos_y,k))/max_E),0,1.0);
            
            im.degra(i,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(i,pos_y,k))/max_E),0,1.0);
        }
        
        std::cout<<max_Ex<<std::endl;
        std::cout<<max_Ey<<std::endl;
        std::cout<<max_Ez<<std::endl;
    }
    if(vmode==1)
    {
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(Ex(pos_x,j,k)));
            max_Ey=max(max_Ey,abs(Ey(pos_x,j,k)));
            max_Ez=max(max_Ez,abs(Ez(pos_x,j,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            im.degra(j,k,1.0-exp(-1.0*abs(Ez(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k,1.0-exp(-1.0*abs(Ez(pos_x,j,k))/max_E),0,1.0);
            
            im.degra(j,k+Nz+4,1.0-exp(-1.0*abs(Ey(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k+Nz+4,1.0-exp(-1.0*abs(Ey(pos_x,j,k))/max_E),0,1.0);
            
            im.degra(j,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(pos_x,j,k))/max_E),0,1.0);
        }
        
        std::cout<<max_Ex<<std::endl;
        std::cout<<max_Ey<<std::endl;
        std::cout<<max_Ez<<std::endl;
    }
    if(vmode==2)
    {
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            max_Ex=max(max_Ex,abs(Ex(i,j,pos_z)));
            max_Ey=max(max_Ey,abs(Ey(i,j,pos_z)));
            max_Ez=max(max_Ez,abs(Ez(i,j,pos_z)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            im.degra(i,j,1.0-exp(-1.0*abs(Ez(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j,1.0-exp(-1.0*abs(Ez(i,j,pos_z))/max_E),0,1.0);
            
            im.degra(i,j+Ny+4,1.0-exp(-1.0*abs(Ey(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j+Ny+4,1.0-exp(-1.0*abs(Ey(i,j,pos_z))/max_E),0,1.0);
            
            im.degra(i,j+2*Ny+8,1.0-exp(-1.0*abs(Ex(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j+2*Ny+8,1.0-exp(-1.0*abs(Ex(i,j,pos_z))/max_E),0,1.0);
        }
        
        std::cout<<max_Ex<<std::endl;
        std::cout<<max_Ey<<std::endl;
        std::cout<<max_Ez<<std::endl;
    }
    
    im.write(K.str());
}

void FDTD::draw(int t,int vmode,int pos_x,int pos_y,int pos_z,Bitmap *im)
{
//    std::unique_lock<std::mutex> lock(im->get_mutex());
    int i,j,k;
    
    using std::abs;
    using std::exp;
    using std::max;
    
    double max_Ex=1e-80,max_Ey=1e-80,max_Ez=1e-80,max_E=1e-80;
        
    int span1=Nx,span2=Nz;
    
    if(vmode==1) { span1=Ny; span2=Nz; }
    if(vmode==2) { span1=Nx; span2=Ny; }
    
    span1=2*span1+4;
    span2=3*span2+8;
    
    im->set_size(span1,span2);
        
    if(vmode==0)
    {
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(Ex(i,pos_y,k)));
            max_Ey=max(max_Ey,abs(Ey(i,pos_y,k)));
            max_Ez=max(max_Ez,abs(Ez(i,pos_y,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            im->degra(i,k,1.0-exp(-1.0*abs(Ez(i,pos_y,k))),0,1.0);
            im->degra(i+Nx+4,k,1.0-exp(-1.0*abs(Ez(i,pos_y,k))/max_E),0,1.0);
            
            im->degra(i,k+Nz+4,1.0-exp(-1.0*abs(Ey(i,pos_y,k))),0,1.0);
            im->degra(i+Nx+4,k+Nz+4,1.0-exp(-1.0*abs(Ey(i,pos_y,k))/max_E),0,1.0);
            
            im->degra(i,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(i,pos_y,k))),0,1.0);
            im->degra(i+Nx+4,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(i,pos_y,k))/max_E),0,1.0);
        }
    }
    if(vmode==1)
    {
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(Ex(pos_x,j,k)));
            max_Ey=max(max_Ey,abs(Ey(pos_x,j,k)));
            max_Ez=max(max_Ez,abs(Ez(pos_x,j,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            im->degra(j,k,1.0-exp(-1.0*abs(Ez(pos_x,j,k))),0,1.0);
            im->degra(j+Ny+4,k,1.0-exp(-1.0*abs(Ez(pos_x,j,k))/max_E),0,1.0);
            
            im->degra(j,k+Nz+4,1.0-exp(-1.0*abs(Ey(pos_x,j,k))),0,1.0);
            im->degra(j+Ny+4,k+Nz+4,1.0-exp(-1.0*abs(Ey(pos_x,j,k))/max_E),0,1.0);
            
            im->degra(j,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(pos_x,j,k))),0,1.0);
            im->degra(j+Ny+4,k+2*Nz+8,1.0-exp(-1.0*abs(Ex(pos_x,j,k))/max_E),0,1.0);
        }
    }
    if(vmode==2)
    {
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            max_Ex=max(max_Ex,abs(Ex(i,j,pos_z)));
            max_Ey=max(max_Ey,abs(Ey(i,j,pos_z)));
            max_Ez=max(max_Ez,abs(Ez(i,j,pos_z)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            im->degra(i,j,1.0-exp(-1.0*abs(Ez(i,j,pos_z))),0,1.0);
            im->degra(i+Nx+4,j,1.0-exp(-1.0*abs(Ez(i,j,pos_z))/max_E),0,1.0);
            
            im->degra(i,j+Ny+4,1.0-exp(-1.0*abs(Ey(i,j,pos_z))),0,1.0);
            im->degra(i+Nx+4,j+Ny+4,1.0-exp(-1.0*abs(Ey(i,j,pos_z))/max_E),0,1.0);
            
            im->degra(i,j+2*Ny+8,1.0-exp(-1.0*abs(Ex(i,j,pos_z))),0,1.0);
            im->degra(i+Nx+4,j+2*Ny+8,1.0-exp(-1.0*abs(Ex(i,j,pos_z))/max_E),0,1.0);
        }
    }
}

void FDTD::find_slab(int sub_ref,int sup_ref,double &hsub,double &hstruc,double &hsup)
{
    int i,j,k;
    
    int Nsub=0,Nsup=0;
    
    int IDmoy=0,IDsup=0,IDsub=0;
    
    for(i=xs_s;i<xs_e;i++){ for(j=ys_s;j<ys_e;j++)
    {
        IDsup+=matsgrid(i,j,zs_e);
        IDsub+=matsgrid(i,j,zs_s);
    }}
    
    IDmoy=IDsub;
    k=sub_ref;
    
    while(IDmoy==IDsub && k<sup_ref)
    {
        IDmoy=0;
        
        for(i=xs_s;i<xs_e;i++) for(j=ys_s;j<ys_e;j++) IDmoy+=matsgrid(i,j,k+1);
        
        Nsub++;
        k++;
    }
    
    IDmoy=IDsup;
    k=sup_ref;
    
    while(IDmoy==IDsup && k>=sub_ref)
    {
        IDmoy=0;
        
        for(i=xs_s;i<xs_e;i++) for(j=ys_s;j<ys_e;j++) IDmoy+=matsgrid(i,j,k-1);
        
        Nsup++;
        k--;
    }
    
    hsup=(Nsup-1)*Dz;
    hsub=Nsub*Dz;
    hstruc=(Nz_s-Nsup-Nsub)*Dz;
        
    std::cout<<hsup<<" "<<hsub<<" "<<hstruc<<std::endl;
}

double FDTD::get_index(int i,int j,int k) const
{
    double eps=mats[matsgrid(i,j,k)].ei;
    return std::sqrt(eps);
}

double FDTD::get_kx(double lambda) const
{
    return kx;
}

double FDTD::get_ky(double lambda) const
{
    return ky;
}

void FDTD::guess_waveguide(Grid1<int> &strat_N,Grid1<double> &strat_ind)
{
    int k;
    
    strat_N.init(3,0);
    strat_ind.init(3,0);
    
    unsigned int MID_sup=matsgrid(0,0,Nz-1);
    unsigned int MID_gd=0;
    unsigned int MID_sub=matsgrid(0,0,0);
    
    int N_sup=0;
    int N_gd=0;
    int N_sub=0;
    
    for(k=0;k<Nz;k++)
    {
        unsigned int tmpID=matsgrid(0,0,k);
        
        if(tmpID!=MID_sup && tmpID!=MID_sub) MID_gd=tmpID;
        if(tmpID==MID_sup) N_sup++;
        if(tmpID==MID_sub) N_sub++;
    }
    
    N_gd=Nz-N_sup-N_sub;
    
    strat_N[0]=N_sub;
    strat_N[1]=N_gd;
    strat_N[2]=N_sup;
    
    using std::sqrt;
    
    strat_ind[0]=sqrt(mats[MID_sub].ei);
    strat_ind[1]=sqrt(mats[MID_gd].ei);
    strat_ind[2]=sqrt(mats[MID_sup].ei);
}

//Everything taken at the center of blocks instead of
//at the origin

void FDTD::local_E(int i,int j,int k,double &Ex_o,double &Ey_o,double &Ez_o) const
{
    Ex_o=local_Ex(i,j,k);
    Ey_o=local_Ey(i,j,k);
    Ez_o=local_Ez(i,j,k);
}

double FDTD::local_Ex(int i,int j,int k) const
{
    int jp=j+1; if(jp==Ny) jp=0;
    int kp=k+1; if(kp==Nz) kp=0;
    
    return (Ex(i,j,k)+Ex(i,jp,k)+Ex(i,j,kp)+Ex(i,jp,kp))/4.0;
}

double FDTD::local_Ey(int i,int j,int k) const
{
    int ip=i+1; if(ip==Nx) ip=0;
    int kp=k+1; if(kp==Nz) kp=0;
    
    return (Ey(i,j,k)+Ey(ip,j,k)+Ey(i,j,kp)+Ey(ip,j,kp))/4.0;
}

double FDTD::local_Ez(int i,int j,int k) const
{
    int ip=i+1; if(ip==Nx) ip=0;
    int jp=j+1; if(jp==Ny) jp=0;
    
    return (Ez(i,j,k)+Ez(ip,j,k)+Ez(i,jp,k)+Ez(ip,jp,k))/4.0;
}

void FDTD::local_H(int i,int j,int k,double &Hx_o,double &Hy_o,double &Hz_o) const
{
    Hx_o=local_Hx(i,j,k);
    Hy_o=local_Hy(i,j,k);
    Hz_o=local_Hz(i,j,k);
}

double FDTD::local_Hx(int i,int j,int k) const
{
    int ip=i+1; if(ip==Nx) ip=0;
    
    return (Hx(i,j,k)+Hx(ip,j,k))/2.0;
}

double FDTD::local_Hy(int i,int j,int k) const
{
    int jp=j+1; if(jp==Ny) jp=0;
    
    return (Hy(i,j,k)+Hy(i,jp,k))/2.0;
}

double FDTD::local_Hz(int i,int j,int k) const
{
    int kp=k+1; if(kp==Nz) kp=0;
    
    return (Hz(i,j,k)+Hz(i,j,kp))/2.0;
}

//Taken on face with normals being X, Y or Z

double FDTD::local_Px(int i,int j,int k) const
{
    int ip=i-1; if(i==0) ip=Nx-1;
    int jp=j+1; if(jp==Ny) jp=0;
    int kp=k+1; if(kp==Nz) kp=0;
    
    double tEy=(Ey(i,j,k)+Ey(i,j,kp))/2.0;
    double tEz=(Ez(i,j,k)+Ez(i,jp,k))/2.0;
    double tHy=(Hy(i,j,k)+Hy(i,jp,k)+Hy(ip,j,k)+Hy(ip,jp,k))/4.0;
    double tHz=(Hz(i,j,k)+Hz(i,j,kp)+Hz(ip,j,k)+Hz(ip,j,kp))/4.0;
    
    return tEy*tHz-tEz*tHy;
}

double FDTD::local_Px_p(int i,int j,int k) const
{
    return local_Px(i+1,j,k);
}

double FDTD::local_Py(int i,int j,int k) const
{
    int ip=i+1; if(ip==Nx) ip=0;
    int jp=j-1; if(j==0) jp=Ny-1;
    int kp=k+1; if(kp==Nz) kp=0;
    
    double tEx=(Ex(i,j,k)+Ex(i,j,kp))/2.0;
    double tEz=(Ez(i,j,k)+Ez(ip,j,k))/2.0;
    double tHx=(Hx(i,j,k)+Hx(ip,j,k)+Hx(i,jp,k)+Hx(ip,jp,k))/4.0;
    double tHz=(Hz(i,j,k)+Hz(i,j,kp)+Hz(i,jp,k)+Hz(i,jp,kp))/4.0;
    
    return tEz*tHx-tEx*tHz;
}

double FDTD::local_Py_p(int i,int j,int k) const
{
    return local_Py(i,j+1,k);
}

double FDTD::local_Pz(int i,int j,int k) const
{
    int ip=i+1; if(ip==Nx) ip=0;
    int jp=j+1; if(jp==Ny) jp=0;
    int kp=k-1; if(k==0) kp=Nz-1;
    
    double tEx=(Ex(i,j,k)+Ex(i,jp,k))/2.0;
    double tEy=(Ey(i,j,k)+Ey(ip,j,k))/2.0;
    double tHx=(Hx(i,j,k)+Hx(ip,j,k)+Hx(i,j,kp)+Hx(ip,j,kp))/4.0;
    double tHy=(Hy(i,j,k)+Hy(i,jp,k)+Hy(i,j,kp)+Hy(i,jp,kp))/4.0;
    
    return tEx*tHy-tEy*tHx;
}

double FDTD::local_Pz_p(int i,int j,int k) const
{
    return local_Pz(i,j,k+1);
}

void FDTD::set_kx(double kx_) { kx=kx_; }
void FDTD::set_ky(double ky_) { ky=ky_; }

#ifndef SEP_MATS
void FDTD::set_matsgrid(Grid3<unsigned int> &GMi)
#else
void FDTD::set_matsgrid(Grid3<unsigned int> &GMi_x,Grid3<unsigned int> &GMi_y,Grid3<unsigned int> &GMi_z)
#endif
{
    int i,j,k;
    
    if(mode==M_NORMAL || mode==M_EXTRAC || mode==M_OBLIQUE_PHASE || mode==M_PLANAR_GUIDED || mode==M_CUSTOM)
    {
        for(i=xs_s;i<xs_e;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            #ifndef SEP_MATS
            matsgrid(i,j,k)=GMi(i-xs_s,j-ys_s,k-zs_s);
            #else
            matsgrid_x(i,j,k)=GMi_x(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_y(i,j,k)=GMi_y(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_z(i,j,k)=GMi_z(i-xs_s,j-ys_s,k-zs_s);
            #endif
        }}}
        
        //#############
        //   Extend
        //#############
        
        for(i=0;i<xs_s;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            #ifndef SEP_MATS
            matsgrid(i,j,k)=matsgrid(xs_s,j,k);
            #else
            matsgrid_x(i,j,k)=GMi_x(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_y(i,j,k)=GMi_y(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_z(i,j,k)=GMi_z(i-xs_s,j-ys_s,k-zs_s);
            #endif
        }}}
        
        for(i=xs_e;i<Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            #ifndef SEP_MATS
            matsgrid(i,j,k)=matsgrid(xs_e-1,j,k);
            #else
            matsgrid_x(i,j,k)=GMi_x(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_y(i,j,k)=GMi_y(i-xs_s,j-ys_s,k-zs_s);
            matsgrid_y(i,j,k)=GMi_z(i-xs_s,j-ys_s,k-zs_s);
            #endif
        }}}
    }
    
    if(mode==M_OBLIQUE)
    {
        for(int l=0;l<Npad;l++)
        {
            for(i=xs_s+l*Nx;i<xs_s+(l+1)*Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
            {
                matsgrid(i,j,k)=GMi(i-xs_s-l*Nx,j-ys_s,k-zs_s);
            }}}
            
            for(i=xs_s+l*Nx;i<xs_s+(l+1)*Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
            {
                matsgrid(i,j,k)=GMi(i-xs_s-l*Nx,j-ys_s,k-zs_s);
            }}}
        }
        
        //###############
        //   Extend X
        //###############
        
//        for(i=0;i<xs_s;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
//        {
//            matsgrid(i,j,k)=matsgrid(xs_s,j,k);
//        }}}
//        
//        for(i=xs_e;i<xs_e+3;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
//        {
//            matsgrid(i,j,k)=matsgrid(i-xs_e+xs_s,j,k);
//        }}}
//        
//        for(i=xs_e+3;i<Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
//        {
//            matsgrid(i,j,k)=matsgrid(xs_e+2,j,k);
//        }}}

        for(i=0;i<xs_s;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            unsigned int it=matsgrid(xs_s,0,zs_e);
            unsigned int id=matsgrid(xs_s,0,zs_s);
            
            int l,m;
            
            bool c1=0;
            bool c2=0;
            
            for(l=xs_s;l<xs_e;l++){ for(m=ys_s;m<ys_e;m++)
            {
                if(matsgrid(l,m,k)==it) c1=1;
                if(matsgrid(l,m,k)==id) c2=1;
            }}
            
            if(c1) matsgrid(i,j,k)=it;
            else if(!c1 && !c2) matsgrid(i,j,k)=it;
            else if(c2) matsgrid(i,j,k)=id;
        }}}
        
        for(i=xs_e;i<xs_e+3;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            matsgrid(i,j,k)=matsgrid(i-xs_e+xs_s,j,k);
        }}}
        
        for(i=xs_e+3;i<Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
        {
            unsigned int it=matsgrid(xs_s,0,zs_e);
            unsigned int id=matsgrid(xs_s,0,zs_s);
            
            int l,m;
            
            bool c1=0;
            bool c2=0;
            
            for(l=xs_s;l<xs_e;l++){ for(m=ys_s;m<ys_e;m++)
            {
                if(matsgrid(l,m,k)==it) c1=1;
                if(matsgrid(l,m,k)==id) c2=1;
            }}
            
            if(c1) matsgrid(i,j,k)=it;
            else if(!c1 && !c2) matsgrid(i,j,k)=it;
            else if(c2) matsgrid(i,j,k)=id;
        }}}
        
        //for(i=xs_e;i<Nx;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
//        for(i=xs_e;i<xs_e+3;i++){ for(j=ys_s;j<ys_e;j++){ for(k=zs_s;k<zs_e;k++)
//        {
//            matsgrid(i,j,k)=matsgrid(i-xs_e+xs_s,j,k);
//        }}}
    }
    
    //###############
    //   Extend Y
    //###############
    
    for(i=0;i<Nx;i++){ for(j=0;j<ys_s;j++){ for(k=zs_s;k<zs_e;k++)
    {
        #ifndef SEP_MATS
        matsgrid(i,j,k)=matsgrid(i,ys_s,k);
        #else
        matsgrid_x(i,j,k)=matsgrid_x(i,ys_s,k);
        matsgrid_y(i,j,k)=matsgrid_y(i,ys_s,k);
        matsgrid_z(i,j,k)=matsgrid_z(i,ys_s,k);
        #endif
    }}}
    
    for(i=0;i<Nx;i++){ for(j=ys_e;j<Ny;j++){ for(k=zs_s;k<zs_e;k++)
    {
        #ifndef SEP_MATS
        matsgrid(i,j,k)=matsgrid(i,ys_e-1,k);
        #else
        matsgrid_x(i,j,k)=matsgrid_x(i,ys_e-1,k);
        matsgrid_y(i,j,k)=matsgrid_y(i,ys_e-1,k);
        matsgrid_z(i,j,k)=matsgrid_z(i,ys_e-1,k);
        #endif
    }}}
    
    //###############
    //   Extend Z
    //###############
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<zs_s;k++)
    {
        #ifndef SEP_MATS
        matsgrid(i,j,k)=matsgrid(i,j,zs_s);
        #else
        matsgrid_x(i,j,k)=matsgrid_x(i,j,zs_s);
        matsgrid_y(i,j,k)=matsgrid_y(i,j,zs_s);
        matsgrid_z(i,j,k)=matsgrid_z(i,j,zs_s);
        #endif
    }}}
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=zs_e;k<Nz;k++)
    {
        #ifndef SEP_MATS
        matsgrid(i,j,k)=matsgrid(i,j,zs_e-1);
        #else
        matsgrid_x(i,j,k)=matsgrid_x(i,j,zs_e-1);
        matsgrid_y(i,j,k)=matsgrid_y(i,j,zs_e-1);
        matsgrid_z(i,j,k)=matsgrid_z(i,j,zs_e-1);
        #endif
    }}}
    
    //###############
    //   Check
    //###############
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        Nmat=std::max(Nmat,matsgrid(i,j,k)+1);
    }}}
    
    mats.init(Nmat,FDTD_Material());
    std::cout<<"Number of materials: "<<Nmat<<std::endl;
    
    unsigned int imax=matsgrid(0,0,0);
    unsigned int imin=matsgrid(0,0,0);
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        imin=std::min(matsgrid(i,j,k),imin);
        imax=std::max(matsgrid(i,j,k),imax);
    }}}
    
    Bitmap tbmp(Nx,Ny);
    Grid2<double> tmp(Nx,Ny,0);
    Grid2<double> stmp(Nx,Ny,0);
    
    for(k=0;k<Nz;k++)
    {
        for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
        {
            tmp(i,j)=(matsgrid(i,j,k)-imin)/(imax-imin+0.0);
            stmp(i,j)+=tmp(i,j);
        }}
        
        tbmp.G2degraM(tmp,"grid2/grid",k,".png",0,1);
    }
    
    tbmp.G2degra(stmp,"grid2/ga.png");
}

bool FDTD::mats_in_grid(unsigned int ind)
{
    int i,j,k;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(matsgrid(i,j,k)==ind) return true;
    }}}
    
    return false;
}

void FDTD::set_material(unsigned int ind,Material const &material_)
{
    if(mats_in_grid(ind))
    {
        mats[ind].link_fdtd(Dx,Dy,Dz,Dt);
        mats[ind].link_grid(matsgrid,ind);
        mats[ind].set_base_mat(material_);
        
        if(mats[ind].needs_D_field()) dt_D_comp=true;
    }
}

void FDTD::report_size()
{
    double F_size=0,P_size=0,M_size=0;
    
    F_size+=Ex.mem_size();
    F_size+=Ey.mem_size();
    F_size+=Ez.mem_size();
    F_size+=Hx.mem_size();
    F_size+=Hy.mem_size();
    F_size+=Hz.mem_size();
    F_size+=dt_Dx.mem_size();
    F_size+=dt_Dy.mem_size();
    F_size+=dt_Dz.mem_size();
    F_size+=dt_Bx.mem_size();
    F_size+=dt_By.mem_size();
    F_size+=dt_Bz.mem_size();
    
    P_size+=PsiExy.mem_size(); P_size+=PsiExz.mem_size();
    P_size+=PsiEyx.mem_size(); P_size+=PsiEyz.mem_size();
    P_size+=PsiEzx.mem_size(); P_size+=PsiEzy.mem_size();
    P_size+=PsiHxy.mem_size(); P_size+=PsiHxz.mem_size();
    P_size+=PsiHyx.mem_size(); P_size+=PsiHyz.mem_size();
    P_size+=PsiHzx.mem_size(); P_size+=PsiHzy.mem_size();
    
    M_size+=matsgrid.mem_size();
    for(unsigned int i=0;i<Nmat;i++) M_size+=mats[i].report_size();
    
    std::cout<<"Fields size: "<<add_unit(F_size,"byte")<<std::endl;
    std::cout<<"PML size: "<<add_unit(P_size,"byte")<<std::endl;
    std::cout<<"Materials size: "<<add_unit(M_size,"byte")<<std::endl;
    std::cout<<"Total size: "<<add_unit(F_size+P_size+M_size,"byte")<<std::endl;
}

void FDTD::reset_fields()
{
    Ex=0; Ey=0; Ez=0; Hx=0; Hy=0; Hz=0;
    PsiExy=0; PsiExz=0;
    PsiEyx=0; PsiEyz=0;
    PsiEzx=0; PsiEzy=0;
    PsiHxy=0; PsiHxz=0;
    PsiHyx=0; PsiHyz=0;
    PsiHzx=0; PsiHzy=0;
//    Psi=0;
//    Psi_c=0;
    
    if(dt_D_comp)
    {
        dt_Dx=0;
        dt_Dy=0;
        dt_Dz=0;
    }
    if(dt_B_comp)
    {
        dt_Bx=0;
        dt_By=0;
        dt_Bz=0;
    }
    
    for(unsigned int i=0;i<Nmat;i++) mats[i].clean_fields();
}

double CutBelow(double a,double b)
{
    if(std::abs(a)>b) return a;
    else return 0;
}

void FDTD::set_directory(std::filesystem::path const &directory_)
{
    directory=directory_;
    
    if(!directory_.empty())
    {
        bool result=true;
        
        if(std::filesystem::exists(directory))
        {
            result=std::filesystem::is_directory(directory);
        }
        else result=std::filesystem::create_directories(directory);
        
        if(!result) directory=PathManager::tmp_path;
    }
}

//void FDTD::set_field_SP_phase(double li,double lf,std::string poli,double kx,double phase)
//{
//    double wi=2.0*Pi*cl/li;
//    double wf=2.0*Pi*cl/lf;
//    
//    double w0=(wi+wf)/2.0;
//    double l0=2.0*Pi*c_light/w0;
//    
//    chpind=zs_e-1;
//    lambda=l0;
//    
//    lambda_m=li;
//    lambda_p=lf;
//    
//    Nshift=0;
//    
//    double pol=0;
//    if(poli=="TE") pol=0;
//    if(poli=="TM") pol=90;
//    
//    chp.init_phase(li,lf,kx,pol,phase,1.0);
//    
//    polar_mode=poli;
//}

void FDTD::set_prefix(std::string prefix_)
{
    prefix=prefix_;
}

//void FDTD::set_spectrum_dens(int A)
//{
//    if(A>=200) spectrum_dens=A;
//}

void FDTD::set_tapering(int Ntap_)
{
    Ntap=Ntap_;
}

void FDTD::set_working_dir(std::string wdir)
{
    std::stringstream strm;
    strm<<wdir<<"/"<<prefix;
    set_prefix(strm.str());
}
