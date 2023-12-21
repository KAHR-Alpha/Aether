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
#include <fdfd.h>


extern const Imdouble Im;
extern std::ofstream plog;

AFP_TFSF::AFP_TFSF()
    :Nmats(0)
{
}

AFP_TFSF::~AFP_TFSF()
{
}

void AFP_TFSF::deep_inject_E(FDTD &fdtd)
{
    int i,j,l;
    
    double t=(step+0.5-500)*Dt;
    
    int i1=xs_s;
    int i2=xs_e;
    
    int j1=ys_s;
    int j2=ys_e;
    
    int k1=Nz/5;
    int k2=3*Nz/4;
    
    double inj_Hx=0;
    double inj_Hy=0;
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        
        double w=2.0*Pi*c_light/lambda;
        Imdouble coeff=gaussian_spectrum(w,lambda_min,lambda_max,0.001)*std::exp(-w*t*Im);
        
        inj_Hx+=std::real(Hx(k2-1,l)*coeff);
        inj_Hy+=std::real(Hy(k2-1,l)*coeff);
    }
    
    // Z
    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
    {
        fdtd.Ex(i,j,k2)-=fdtd.dtdex*inj_Hy
                         /fdtd.mats[fdtd.matsgrid(i,j,k2)].ei;
        
        fdtd.Ey(i,j,k2)+=fdtd.dtdex*inj_Hx
                         /fdtd.mats[fdtd.matsgrid(i,j,k2)].ei;
    }
    
    inj_Hx=inj_Hy=0;
    
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        
        double w=2.0*Pi*c_light/lambda;
        Imdouble coeff=gaussian_spectrum(w,lambda_min,lambda_max,0.001)*std::exp(-w*t*Im);
        
        inj_Hx+=std::real(Hx(k1-1,l)*coeff);
        inj_Hy+=std::real(Hy(k1-1,l)*coeff);
    }
    
    
    // Z
    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
    {
        fdtd.Ex(i,j,k1)+=fdtd.dtdex*inj_Hy
                         /fdtd.mats[fdtd.matsgrid(i,j,k1)].ei;
        
        fdtd.Ey(i,j,k1)-=fdtd.dtdex*inj_Hx
                         /fdtd.mats[fdtd.matsgrid(i,j,k1)].ei;
    }
    
//    double inj_Hx=inj_chp.Hx(0,0,(zs_e-1+5.5)*Dz,tb);
//    double inj_Hy=inj_chp.Hy(0,0,(zs_e-1+5.5)*Dz,tb);
    
//    // Z
//    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
//    {
//        fdtd.Ex(i,j,k1)+=fdtd.dtdex*fdtd_aux.Hy(0,0,k1-1)
//                         /fdtd.mats(fdtd.matsgrid(i,j,k1)).ei;
//        fdtd.Ex(i,j,k2)-=fdtd.dtdex*fdtd_aux.Hy(0,0,k2-1)
//                         /fdtd.mats(fdtd.matsgrid(i,j,k2)).ei;
//        
//        fdtd.Ey(i,j,k1)-=fdtd.dtdex*fdtd_aux.Hx(0,0,k1-1)
//                         /fdtd.mats(fdtd.matsgrid(i,j,k1)).ei;
//        fdtd.Ey(i,j,k2)+=fdtd.dtdex*fdtd_aux.Hx(0,0,k2-1)
//                         /fdtd.mats(fdtd.matsgrid(i,j,k2)).ei;
//    }

//    
//    // Y
//    for(i=i1;i<i2;i++) for(k=k1;k<k2;k++)
//    {
//        fdtd.Ex(i,j1,k)-=fdtd.dtdex*fdtd_aux.Hz(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i,j1,k)).ei;
//        fdtd.Ex(i,j2,k)+=fdtd.dtdex*fdtd_aux.Hz(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i,j2,k)).ei;
//        
//        fdtd.Ez(i,j1,k)+=fdtd.dtdex*fdtd_aux.Hx(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i,j1,k)).ei;
//        fdtd.Ez(i,j2,k)-=fdtd.dtdex*fdtd_aux.Hx(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i,j2,k)).ei;
//    }
//    
//    // X
//    for(k=k1;k<k2;k++) for(j=j1;j<j2;j++)
//    {
//        fdtd.Ey(i1,j,k)+=fdtd.dtdex*fdtd_aux.Hz(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i1,j,k)).ei;
//        fdtd.Ey(i2,j,k)-=fdtd.dtdex*fdtd_aux.Hz(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i2,j,k)).ei;
//        
//        fdtd.Ez(i1,j,k)-=fdtd.dtdex*fdtd_aux.Hy(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i1,j,k)).ei;
//        fdtd.Ez(i2,j,k)+=fdtd.dtdex*fdtd_aux.Hy(0,0,k)
//                         /fdtd.mats(fdtd.matsgrid(i2,j,k)).ei;
//    }
}

void AFP_TFSF::deep_inject_H(FDTD &fdtd)
{
    // H-field injection
    
    int i,j,l;
    
    double t=(step+1-500)*Dt;
    
    int i1=xs_s;
    int i2=xs_e;
    
    int j1=ys_s;
    int j2=ys_e;
    
    int k1=Nz/5;
    int k2=3*Nz/4;
    
    double inj_Ex=0;
    double inj_Ey=0;
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        
        double w=2.0*Pi*c_light/lambda;
        Imdouble coeff=gaussian_spectrum(w,lambda_min,lambda_max,0.001)*std::exp(-w*t*Im);
        
        inj_Ex+=std::real(Ex(k2,l)*coeff);
        inj_Ey+=std::real(Ey(k2,l)*coeff);
    }
    
    // Z
    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
    {
        fdtd.Hx(i,j,k2-1)+=fdtd.dtdmx*inj_Ey;
        fdtd.Hy(i,j,k2-1)-=fdtd.dtdmx*inj_Ex;
    }
    
    inj_Ex=inj_Ey=0;
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        
        double w=2.0*Pi*c_light/lambda;
        Imdouble coeff=gaussian_spectrum(w,lambda_min,lambda_max,0.0001)*std::exp(-w*t*Im);
        
        inj_Ex+=std::real(Ex(k1,l)*coeff);
        inj_Ey+=std::real(Ey(k1,l)*coeff);
    }
//    plog<<step<<" "<<inj_Ex<<std::endl;
    // Z
    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
    {
        fdtd.Hx(i,j,k1-1)-=fdtd.dtdmx*inj_Ey;
        fdtd.Hy(i,j,k1-1)+=fdtd.dtdmx*inj_Ex;
    }
    
//    tb=(t+0.5)*Dt;
//    double inj_Ex=inj_chp.Ex(0,0,(zs_e-1+5)*Dz,tb);
//    double inj_Ey=inj_chp.Ey(0,0,(zs_e-1+5)*Dz,tb);
    
//    // Z
//    for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
//    {
//        fdtd.Hx(i,j,k1-1)-=fdtd.dtdmx*fdtd_aux.Ey(0,0,k1);
//        fdtd.Hx(i,j,k2-1)+=fdtd.dtdmx*fdtd_aux.Ey(0,0,k2);
//        
//        fdtd.Hy(i,j,k1-1)+=fdtd.dtdmx*fdtd_aux.Ex(0,0,k1);
//        fdtd.Hy(i,j,k2-1)-=fdtd.dtdmx*fdtd_aux.Ex(0,0,k2);
//    }
//    
//    // Y
//    for(i=i1;i<i2;i++) for(k=k1;k<k2;k++)
//    {
//        fdtd.Hx(i,j1-1,k)+=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
//        fdtd.Hx(i,j2-1,k)-=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
//        
//        fdtd.Hz(i,j1-1,k)-=fdtd.dtdmx*fdtd_aux.Ex(0,0,k);
//        fdtd.Hz(i,j2-1,k)+=fdtd.dtdmx*fdtd_aux.Ex(0,0,k);
//    }
//    
//    // X
//    for(k=k1;k<k2;k++) for(j=j1;j<j2;j++)
//    {
//        fdtd.Hy(i1-1,j,k)-=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
//        fdtd.Hy(i2-1,j,k)+=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
//        
//        fdtd.Hz(i1-1,j,k)+=fdtd.dtdmx*fdtd_aux.Ey(0,0,k);
//        fdtd.Hz(i2-1,j,k)-=fdtd.dtdmx*fdtd_aux.Ey(0,0,k);
//    }
}

void AFP_TFSF::deep_link(FDTD const &fdtd)
{
    int i;
    
    Nmats=fdtd.mats.L1();
    mats.init(Nmats);
    
    for(i=0;i<Nmats;i++) mats[i]=fdtd.mats[i].base_mat;
    
    set_matsgrid(fdtd.matsgrid);
}

void AFP_TFSF::initialize()
{
    int k,l;
    
    FDFD fdfd(0,0,0);
    
//    fdfd.init(1,1,Nz,Dx,Dy,Dz);
    fdfd.set_materials(mats);
    fdfd.set_matsgrid(matsgrid);
    
    Nl=3000;
    lambda_min=400e-9;
    lambda_max=1000e-9;
    
    Ex.init(Nz,Nl);
    Ey.init(Nz,Nl);
    Ez.init(Nz,Nl);
    
    Hx.init(Nz,Nl);
    Hy.init(Nz,Nl);
    Hz.init(Nz,Nl);
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        
        fdfd.solve_prop_1D(lambda,Degree(0),0,1);
        
        for(k=0;k<Nz;k++)
        {
            Ex(k,l)=fdfd.get_Ex(0,0,k);
            Ey(k,l)=fdfd.get_Ey(0,0,k);
            Ez(k,l)=fdfd.get_Ez(0,0,k);
            
            Hx(k,l)=fdfd.get_Hx(0,0,k);
            Hy(k,l)=fdfd.get_Hy(0,0,k);
            Hz(k,l)=fdfd.get_Hz(0,0,k);
        }
    }
}

void AFP_TFSF::set_matsgrid(Grid3<unsigned int> const &G)
{
    int k;
    
    matsgrid.init(1,1,Nz,0);
    for(k=0;k<Nz;k++) matsgrid(0,0,k)=G(0,0,k);
}
