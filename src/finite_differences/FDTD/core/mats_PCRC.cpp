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

#include <fdtd_material.h>
#include <fdtd_utils.h>
#include <logger.h>

void FDTD_Material::PCRC_ante(int i,int j,int k,
                              Grid3<double> const &Ex,
                              Grid3<double> const &Ey,
                              Grid3<double> const &Ez)
{
    double &psi_loc_x=m_Psi(i-x1,j-y1,k-z1,0);
    double &psi_loc_y=m_Psi(i-x1,j-y1,k-z1,1);
    double &psi_loc_z=m_Psi(i-x1,j-y1,k-z1,2);
        
    psi_loc_x=Ex(i,j,k);
    psi_loc_y=Ey(i,j,k);
    psi_loc_z=Ez(i,j,k);
}

void FDTD_Material::PCRC_post(int i,int j,int k,
                              Grid3<double> const &Ex,
                              Grid3<double> const &Ey,
                              Grid3<double> const &Ez)
{
    for(int p=0;p<Np;p++)
    {
        Imdouble &Pclx=m_Psi_c(i-x1,j-y1,k-z1,3*p+0);
        Imdouble &Pcly=m_Psi_c(i-x1,j-y1,k-z1,3*p+1);
        Imdouble &Pclz=m_Psi_c(i-x1,j-y1,k-z1,3*p+2);
        
        double &Plx=m_Psi(i-x1,j-y1,k-z1,0);
        double &Ply=m_Psi(i-x1,j-y1,k-z1,1);
        double &Plz=m_Psi(i-x1,j-y1,k-z1,2);
        
        Pclx=Crec[p]*Pclx+Dchi[p]*Plx;
        Pcly=Crec[p]*Pcly+Dchi[p]*Ply;
        Pclz=Crec[p]*Pclz+Dchi[p]*Plz;
    }
}

void FDTD_Material::PCRC_apply_E(int i,int j,int k,Grid3<double> &E,int dir)
{
    double Psisum=0;
        
    for(int p=0;p<Np;p++)
    {
        Imdouble &psi_loc=m_Psi_c(i-x1,j-y1,k-z1,3*p+dir);
        
        Psisum+=std::real(psi_loc);
    }
    
    E(i,j,k)+=C3*Psisum;
}

void FDTD_Material::PCRC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                             Grid3<double> const &Dx,
                             Grid3<double> const &Dy,
                             Grid3<double> const &Dz)
{
    double D_tmp=Dx(i,j,k);
    if(dir==1) D_tmp=Dy(i,j,k);
    else if(dir==2) D_tmp=Dz(i,j,k);
    
    E(i,j,k)+=C2*D_tmp;
    PCRC_apply_E(i,j,k,E,dir);
}

void FDTD_Material::PCRC_dielec_treat()
{
    int i;
    
    int tNp=dielec.get_N_models();
    set_mem_depth(tNp,3,3*tNp);
    
    ei=dielec.get_const();
    sig=dielec.get_sigma();
    
    Grid1<Imdouble> va,vb;
    
    dielec.get_time_exp(va,vb);
    
    va.show();
    vb.show();
    
    for(i=0;i<Np;i++)
    {
        Imdouble &pa=va[i];
        Imdouble &pb=vb[i];
        
        using std::exp;
        using std::pow;
                
        Crec[i]=exp(pb*Dt);
        chi[i]=pa/pb*exp(pb*Dt/2.0)*(exp(pb*Dt)-1.0);
        chi_h[i]=pa/pb*(exp(pb*Dt/2.0)-1.0);
        Dchi[i]=-pa/pb*exp(pb*Dt/2.0)*pow(1.0-exp(pb*Dt),2.0);
    }
    
    PCRC_recalc();
}

void FDTD_Material::PCRC_recalc()
{
    double chitmp1=0;
    double chitmp2=0;
    
    m_type=MAT_PCRC2;
    comp_simp=0;
    comp_ante=1;
    comp_post=1;
    comp_D=0;
    
    for(int i=0;i<Np;i++)
    {
        chitmp1+=std::real(chi_h[i]);
        chitmp2+=std::real(chi_h[i]-chi[i]);
    }
    
    Plog::print("chitmp1 ", chitmp1, " chitmp2 ", chitmp2, "\n");
    
    double e_eff=ei+chitmp1+sig*Dt/(2.0*e0);
    
    C1=(ei+chitmp2-sig*Dt/(2.0*e0))/e_eff;
    C2=Dt/(e0*e_eff);
    C2x=Dt/(Dx*e0*e_eff);
    C2y=Dt/(Dy*e0*e_eff);
    C2z=Dt/(Dz*e0*e_eff);
    C3=1.0/e_eff;
    C4=Dt/(e0*e_eff);
}
