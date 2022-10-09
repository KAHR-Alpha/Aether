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

#include <fdtd_material.h>

extern const double Pi;
extern std::ofstream plog;

void FDTD_Material::RC_ante(int i,int j,int k,
                            Grid3<double> const &Ex,
                            Grid3<double> const &Ey,
                            Grid3<double> const &Ez)
{
    for(int p=0;p<Np;p++)
    {
        Imdouble &psi_loc_x=m_Psi_c(i-x1,j-y1,k-z1,3*p+0);
        Imdouble &psi_loc_y=m_Psi_c(i-x1,j-y1,k-z1,3*p+1);
        Imdouble &psi_loc_z=m_Psi_c(i-x1,j-y1,k-z1,3*p+2);
        
        psi_loc_x=Crec[p]*psi_loc_x+Dchi[p]*Ex(i,j,k);
        psi_loc_y=Crec[p]*psi_loc_y+Dchi[p]*Ey(i,j,k);
        psi_loc_z=Crec[p]*psi_loc_z+Dchi[p]*Ez(i,j,k);
    }
}

void FDTD_Material::RC_apply_E(int i,int j,int k,Grid3<double> &E,int dir)
{
    double Psisum=0;
        
    for(int p=0;p<Np;p++)
    {
        Imdouble &psi_loc=m_Psi_c(i-x1,j-y1,k-z1,3*p+dir);
        
        Psisum+=std::real(psi_loc);
    }
    
    E(i,j,k)+=C3*Psisum;
}

void FDTD_Material::RC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                           Grid3<double> const &Dx,
                           Grid3<double> const &Dy,
                           Grid3<double> const &Dz)
{
    double D_tmp=Dx(i,j,k);
    if(dir==1) D_tmp=Dy(i,j,k);
    else if(dir==2) D_tmp=Dz(i,j,k);
    
    E(i,j,k)+=C2*D_tmp;
    RC_apply_E(i,j,k,E,dir);
}

/*void FDTD_Material::dielec_load_script(ScriptHandler &spt)
{
    using std::stringstream;
    bool disp_mod=0;
    
    if(spt.has("eps_inf"))
    {
        double e_inf;
        stringstream strm; strm.str(spt.get_options());
        strm>>e_inf;
        dielec.set_const(e_inf);
    }
    while(spt.has("drude"))
    {
        double wd,g;
        stringstream strm; strm.str(spt.get_options());
        strm>>wd; strm>>g;
        dielec.add_drude(wd,g); disp_mod=1;
    }
    while(spt.has("lorentz"))
    {
        double A,O,G;
        stringstream strm; strm.str(spt.get_options());
        strm>>A; strm>>O; strm>>G;
        dielec.add_lorentz(A,O,G); disp_mod=1;
    }
    while(spt.has("crit_point"))
    {
        double A,O,P,G;
        stringstream strm; strm.str(spt.get_options());
        strm>>A; strm>>O; strm>>P; strm>>G;
        dielec.add_critpoint(A,O,P,G); disp_mod=1;
    }
    
    
    if(disp_mod)
    {
        if(spt.has("mode"))
        {
            std::string mode_str=spt.get_options();
            if(mode_str=="PCRC2") PCRC_dielec_treat();
        }
        else RC_dielec_treat();
    }
    else chk_msg("Invalid material file");
    
    show();
    
    std::ofstream file("mat_test",std::ios::out|std::ios::trunc);
    
    for(int l=0;l<1000;l++)
    {
        double lambda=380e-9+620e-9*l/999.0;
        
        double w=2.0*Pi*c_light/lambda;
        
        Imdouble eps=dielec.eval(w);
        file<<lambda*1e9<<" "<<std::real(eps)<<" "<<std::imag(eps)<<std::endl;
    }
}*/

void FDTD_Material::RC_dielec_treat()
{
    int i;
    
    int tNp=dielec.get_N_models();
    set_mem_depth(tNp,0,3*tNp);
    
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
        chi[i]=pa/pb*(exp(pb*Dt)-1.0);
        Dchi[i]=-pa/pb*pow(1.0-exp(pb*Dt),2.0);
    }
    
    RC_recalc();
}

void FDTD_Material::RC_recalc()
{
    m_type=MAT_RC;
    comp_simp=0;
    comp_ante=1;
    comp_post=0;
    comp_D=0;
    
    double chitmp=0;
    for(int i=0;i<Np;i++)
    {
        chitmp+=std::real(chi[i]);
    }
    
    std::cout<<"chitmp "<<chitmp<<std::endl;
    
    double e_eff=ei+chitmp+sig*Dt/e0;
    
    C1=ei/e_eff;
    C2=Dt/(e0*e_eff);
    C2x=Dt/(Dx*e0*e_eff);
    C2y=Dt/(Dy*e0*e_eff);
    C2z=Dt/(Dz*e0*e_eff);
    C3=1.0/e_eff;
    C4=Dt/(e0*e_eff);
}

