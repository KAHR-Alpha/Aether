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
#include <fdtd_material.h>
#include <fdtd_utils.h>


extern const Imdouble Im;
extern std::ofstream plog;

//####################
//   FDTD_Material
//####################

FDTD_Material::FDTD_Material()
    :m_type(MAT_CONST),
        Np(0),
        Np_r(0),
        Np_c(0),
        Dx(1), Dy(1), Dz(1),
        Dt(1),
        C1(1),
        C2(0), C2x(0), C2y(0), C2z(0),
        C3(0),
        C4(0),
        ei(1),
        sig(0),
        comp_simp(1),
        comp_ante(0),
        comp_self(0),
        comp_post(0),
        comp_D(0)
{
    Crec.init(1,0);
    chi.init(1,0);
    chi_h.init(1,0);
    Dchi.init(1,0);
    
    x1=x2=y1=y2=z1=z2=0;
}

/*FDTD_Material::FDTD_Material(int m_typei,double Dxi,double Dyi,double Dzi,double Dti)
    :m_type(m_typei),
        Dx(Dxi),
        Dy(Dyi),
        Dz(Dzi),
        Dt(Dti),
        comp_simp(1),
        comp_ante(0),
        comp_self(0),
        comp_post(0)
{
    if(m_type==MAT_CONST)
    {
        Np=Np_r=Np_c=0;
        
        comp_simp=1;
        comp_ante=comp_self=comp_post=0;
    }
    if(m_type==MAT_RC)
    {
        Np=1;
        Np_r=0;
        Np_c=1;
        
        comp_ante=1;
        comp_simp=comp_self=comp_post=0;
    }
    if(m_type==MAT_PCRC2)
    {
        Np=1;
        Np_r=3;
        Np_c=1;
    }
    if(m_type==MAT_NAGRA_2LVL)
    {
        Np=Np_c=0;
        Np_r=13;
    }
    if(m_type==MAT_GAIN_NAGRA)
    {
        Np=Np_c=0;
        Np_r=15;
    }
    
    C1=1;
    C2x=Dt/(e0*Dx);
    C2y=Dt/(e0*Dy);
    C2z=Dt/(e0*Dz);
    C3=0;
    C4=Dt/e0;
    ei=1;
    sig=0;
    
    realloc();
}*/

void FDTD_Material::clean_fields()
{
    m_Psi=0;
    m_Psi_c=0;
}

void FDTD_Material::coeffsX(double &C1o,double &C2yo,double &C2zo)
{
    C1o=C1;
    C2yo=C2y;
    C2zo=C2z;
}

void FDTD_Material::coeffsY(double &C1o,double &C2xo,double &C2zo)
{
    C1o=C1;
    C2xo=C2x;
    C2zo=C2z;
}

void FDTD_Material::coeffsZ(double &C1o,double &C2xo,double &C2yo)
{
    C1o=C1;
    C2xo=C2x;
    C2yo=C2y;
}

Imdouble FDTD_Material::get_n(double w)
{
    return base_mat.get_n(w);
}

void FDTD_Material::compute_close(int i,int j,int k,int dir,
                                  Grid3<double> &E,
                                  Grid4<double> &Psi,
                                  Grid4<Imdouble> &Psi_c)
{
    if(m_type==MAT_CONST) return;
    else if(m_type==MAT_RC) return;
    else if(m_type==MAT_PCRC2)
    {
        for(int p=0;p<Np;p++) Psi_c(i,j,k,3*p+dir)=Crec[p]*Psi_c(i,j,k,3*p+dir)+Dchi[p]*Psi(i,j,k,dir);
    }
}

void FDTD_Material::ante_compute(int i,int j,int k,
                                 Grid3<double> const &Ex,
                                 Grid3<double> const &Ey,
                                 Grid3<double> const &Ez)
{
    if(comp_ante==0) return;
    
    if(m_type==MAT_RC) RC_ante(i,j,k,Ex,Ey,Ez);
    else if(m_type==MAT_PCRC2) PCRC_ante(i,j,k,Ex,Ey,Ez);
    else if(m_type==MAT_ATOM_LEVEL) AL_ante(i,j,k,Ex,Ey,Ez);
}

void FDTD_Material::post_compute(int i,int j,int k,
                                 Grid3<double> const &Ex,
                                 Grid3<double> const &Ey,
                                 Grid3<double> const &Ez)
{
    if(comp_post==0) return;
    
    if(m_type==MAT_PCRC2) PCRC_post(i,j,k,Ex,Ey,Ez);
    else if(m_type==MAT_ATOM_LEVEL) AL_post(i,j,k,Ex,Ey,Ez);
}

void FDTD_Material::self_compute(int i,int j,int k,
                                 Grid3<double> const &Ex,
                                 Grid3<double> const &Ey,
                                 Grid3<double> const &Ez)
{
    if(comp_self==0) return;
}

void FDTD_Material::apply_E(int i,int j,int k,Grid3<double> &E,int dir)
{
    if(comp_simp==1) return;
    
    if(m_type==MAT_RC) RC_apply_E(i,j,k,E,dir);
    else if(m_type==MAT_PCRC2) PCRC_apply_E(i,j,k,E,dir);
    else if(m_type==MAT_ATOM_LEVEL) AL_apply_E(i,j,k,E,dir);
}


void FDTD_Material::apply_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                              Grid3<double> const &Dx,
                              Grid3<double> const &Dy,
                              Grid3<double> const &Dz)
{
    if(m_type==MAT_CONST) const_D2E(i,j,k,E,dir,Dx,Dy,Dz);
    else if(m_type==MAT_RC) RC_D2E(i,j,k,E,dir,Dx,Dy,Dz);
    else if(m_type==MAT_PCRC2) PCRC_D2E(i,j,k,E,dir,Dx,Dy,Dz);
    else if(m_type==MAT_ANI_DIAG_CONST) ani_DC_D2E(i,j,k,E,dir,Dx,Dy,Dz);
}

double FDTD_Material::compute_open(int i,int j,int k,int dir,
                                   Grid3<double> &E,
                                   Grid4<double> &Psi,
                                   Grid4<Imdouble> &Psi_c)
{
    if(m_type==MAT_CONST) return 0;
    else if(m_type==MAT_RC)
    {
        double Psisum=0;
        
        for(int p=0;p<Np;p++)
        {
            Psi_c(i,j,k,3*p+dir)=Crec[p]*Psi_c(i,j,k,3*p+dir)+Dchi[p]*E(i,j,k);
            Psisum+=std::real(Psi_c(i,j,k,3*p+dir));
        }
        
        return Psisum;
    }
    else if(m_type==MAT_PCRC2)
    {
        double Psisum=0;
        
        for(int p=0;p<Np;p++) Psisum+=std::real(Psi_c(i,j,k,3*p+dir));
        Psi(i,j,k,dir)=E(i,j,k);
        
        return Psisum;
    }
    else if(m_type==MAT_NAGRA_2LVL)
    {
        int P_index=3*(dir+1);
        double &Pnp=Psi(i,j,k,P_index);
        double &Pn=Psi(i,j,k,P_index+1);
        double &Pm=Psi(i,j,k,P_index+2);
        double &DN=Psi(i,j,k,12);
        double &En=E(i,j,k);
        
        Pm=Pn;
        Pn=Pnp;
        Pnp=n2l_P0*(n2l_P1*DN*En+n2l_P2*Pn+n2l_P3*Pm);
        return Pnp-Pn;
    }
    else if(m_type==MAT_GAIN_NAGRA)
    {
        int P_index=3*(dir+1);
        double &Pnp=Psi(i,j,k,P_index);
        double &Pn=Psi(i,j,k,P_index+1);
        double &Pm=Psi(i,j,k,P_index+2);
        double &En=E(i,j,k);
        
        double DN=Psi(i,j,k,12)-Psi(i,j,k,13);
        
//        static int step=0;
//        
//        if(i==0 && j==0 && k==100 && dir==0)
//        {
//            plog<<step<<" "<<DN<<std::endl;
//            step++;
//        }
        
        Pm=Pn;
        Pn=Pnp;
        Pnp=n4l_P0*(n4l_P1*DN*En+n4l_P2*Pn+n4l_P3*Pm);
        return Pnp-Pn;
    }
    
    return 0;
}

void FDTD_Material::compute_self(int i,int j,int k,
                                 Grid3<double> &Ex,
                                 Grid3<double> &Ey,
                                 Grid3<double> &Ez,
                                 Grid4<double> &Psi,
                                 Grid4<Imdouble> &Psi_c)
{
    if(m_type==MAT_CONST) return;
    else if(m_type==MAT_RC) return;
    else if(m_type==MAT_PCRC2) return;
    else if(m_type==MAT_NAGRA_2LVL)
    {
        double &Ex_p=Ex(i,j,k); double &tEx=Psi(i,j,k,0);
        double &Ey_p=Ey(i,j,k); double &tEy=Psi(i,j,k,1);
        double &Ez_p=Ez(i,j,k); double &tEz=Psi(i,j,k,2);
        double &Pnpx=Psi(i,j,k,3); double &Pnx=Psi(i,j,k,4);
        double &Pnpy=Psi(i,j,k,6); double &Pny=Psi(i,j,k,7);
        double &Pnpz=Psi(i,j,k,9); double &Pnz=Psi(i,j,k,10);
        double &DN=Psi(i,j,k,12);
        
        DN=n2l_N0*(DN*n2l_N1+n2l_N2+n2l_N3*((Ex_p+tEx)*(Pnpx-Pnx)
                                            +(Ey_p+tEy)*(Pnpy-Pny)
                                            +(Ez_p+tEz)*(Pnpz-Pnz)));
    }
    else if(m_type==MAT_GAIN_NAGRA)
    {
        double &Ex_p=Ex(i,j,k); double &tEx=Psi(i,j,k,0);
        double &Ey_p=Ey(i,j,k); double &tEy=Psi(i,j,k,1);
        double &Ez_p=Ez(i,j,k); double &tEz=Psi(i,j,k,2);
        double &Pnpx=Psi(i,j,k,3); double &Pnx=Psi(i,j,k,4);
        double &Pnpy=Psi(i,j,k,6); double &Pny=Psi(i,j,k,7);
        double &Pnpz=Psi(i,j,k,9); double &Pnz=Psi(i,j,k,10);
        double &cN1=Psi(i,j,k,12);
        double &cN2=Psi(i,j,k,13);
        double &cN3=Psi(i,j,k,14);
        
        double tN2=cN2;
        double tN3=cN3;
        
        double pscal=(Ex_p+tEx)*(Pnpx-Pnx)+(Ey_p+tEy)*(Pnpy-Pny)+(Ez_p+tEz)*(Pnpz-Pnz);
        
        static int step=0;
        
        if(i==0 && j==0 && k==100)
        {
//            plog<<step<<" "<<cN1<<" "<<cN2<<" "<<cN3<<std::endl;
            plog<<step<<" "<<std::log10(cN1)<<" "<<std::log10(cN2)<<" "<<std::log10(cN3)<<std::endl;
//            plog<<step<<" "<<cN1<<" "<<cN2<<" "<<cN3<<std::endl;
//            plog<<step<<" "<<(Pnpx-Pnx)/e0<<std::endl;
//            plog<<step<<" "<<n4l_N10*n4l_N14*pscal<<" "<<n4l_N20*n4l_N23*pscal<<std::endl;
            step++;
        }
        
        cN3=n4l_N30*(tN3*n4l_N31+n4l_Wp);
        cN2=n4l_N20*(tN2*n4l_N21+n4l_N22*(cN3+tN3)+n4l_N23*pscal);
        cN1=n4l_N10*(cN1*n4l_N11+n4l_N12*(cN3+tN3)+n4l_N13*(cN2+tN2)+n4l_N14*pscal);
    }
}

/*void FDTD_Material::init(int m_typei,double Dxi,double Dyi,double Dzi,double Dti)
{
    m_type=m_typei;
    Dx=Dxi;
    Dy=Dyi;
    Dz=Dzi;
    Dt=Dti;
    
    if(m_type==MAT_CONST)
    {
        Np=Np_r=Np_c=0;
    }
    if(m_type==MAT_RC)
    {
        Np=1;
        Np_r=0;
        Np_c=1;
    }
    if(m_type==MAT_PCRC2)
    {
        Np=1;
        Np_r=3;
        Np_c=1;
    }
    if(m_type==MAT_NAGRA_2LVL)
    {
        Np=Np_c=0;
        Np_r=13;
    }
    if(m_type==MAT_GAIN_NAGRA)
    {
        Np=Np_c=0;
        Np_r=15;
    }
    
    C1=1;
    C2x=Dt/(e0*Dx);
    C2y=Dt/(e0*Dy);
    C2z=Dt/(e0*Dz);
    C3=0;
    C4=Dt/e0;
    ei=1;
    sig=0;
    
    realloc();
}*/

bool FDTD_Material::needs_D_field()
{
    return comp_D;
}

void FDTD_Material::link_fdtd(double Dx_i,double Dy_i,double Dz_i,double Dt_i)
{
    Dx=Dx_i;
    Dy=Dy_i;
    Dz=Dz_i;
    Dt=Dt_i;
}

void FDTD_Material::link_grid(Grid3<unsigned int> const &mat_grid,unsigned int ID)
{
    int i,j,k;
    
    Nx=mat_grid.L1();
    Ny=mat_grid.L2();
    Nz=mat_grid.L3();
    
    Grid1<bool> x_chk(Nx,false);
    Grid1<bool> y_chk(Ny,false);
    Grid1<bool> z_chk(Nz,false);
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(mat_grid(i,j,k)==ID)
        {
            x_chk[i]=y_chk[j]=z_chk[k]=true;
        }
    }}}
    
    x1=0; x2=Nx-1;
    y1=0; y2=Ny-1;
    z1=0; z2=Nz-1;
    
    i=0; while(!x_chk[i]) i++; x1=i;
    i=Nx-1; while(!x_chk[i]) i--; x2=i;
    
    j=0; while(!y_chk[j]) j++; y1=j;
    j=Ny-1; while(!y_chk[j]) j--; y2=j;
    
    k=0; while(!z_chk[k]) k++; z1=k;
    k=Nz-1; while(!z_chk[k]) k--; z2=k;
    
    x_span=x2+1-x1;
    y_span=y2+1-y1;
    z_span=z2+1-z1;
    
    if(x_span<1 || y_span<1 || z_span<1)
    {
        mat_present.init(1,1,1,false);
        return;
    }
    
    mat_present.init(x_span,y_span,z_span,false);
    
    for(i=0;i<x_span;i++){ for(j=0;j<y_span;j++){ for(k=0;k<z_span;k++)
    {
        if(mat_grid(i+x1,j+y1,k+z1)==ID) mat_present(i,j,k)=true;
    }}}
}

void FDTD_Material::set_base_mat(Material const &material_)
{
    base_mat=material_;
    
    if(base_mat.is_const()) set_const(base_mat.eps_inf);
    else if(base_mat.fdtd_compatible())
    {
        dielec.eps_inf=base_mat.eps_inf;
        
        dielec.debye_arr=base_mat.debye;
        dielec.drude_arr=base_mat.drude;
        dielec.lorentz_arr=base_mat.lorentz;
        dielec.cp_arr=base_mat.critpoint;
        
//        if(base_mat.pcrc2) PCRC_dielec_treat();
//        else RC_dielec_treat();
        
        RC_dielec_treat();
    }
}

void FDTD_Material::set_mem_depth(int Np_i,int Np_ri,int Np_ci)
{
    Np=Np_i;
    Np_r=Np_ri;
    Np_c=Np_ci;
    
    realloc();
}

void FDTD_Material::realloc()
{
    if(Np==0)
    {
        Crec.init(1,0);
        chi.init(1,0);
        chi_h.init(1,0);
        Dchi.init(1,0);
    }
    else
    {
        std::cout<<Np<<std::endl;
        
        Crec.init(Np,0);
        chi.init(Np,0);
        chi_h.init(Np,0);
        Dchi.init(Np,0);
    }
    
    if(Np_r>0) m_Psi.init(x_span,y_span,z_span,Np_r,0);
    if(Np_c>0) m_Psi_c.init(x_span,y_span,z_span,Np_c,0);
}

double FDTD_Material::report_size()
{
    return m_Psi.mem_size()+m_Psi_c.mem_size();
}

// Legacy - to be removed

/*void FDTD_Material::recalc()
{
    if(m_type==MAT_CONST) const_recalc();
    else if(m_type==MAT_RC)
    {
        double chitmp=0;
        for(int i=0;i<Np;i++)
        {
            chitmp+=std::real(chi[i]);
        }
        
        std::cout<<"chitmp "<<chitmp<<std::endl;
        
        C1=ei/(ei+chitmp+sig*Dt/e0);
        C2x=Dt/(Dx*e0*(ei+chitmp+sig*Dt/e0));
        C2y=Dt/(Dy*e0*(ei+chitmp+sig*Dt/e0));
        C2z=Dt/(Dz*e0*(ei+chitmp+sig*Dt/e0));
        C3=1.0/(ei+chitmp+sig*Dt/e0);
        C4=Dt/(e0*(ei+chitmp+sig*Dt/e0));
    }
    else if(m_type==MAT_PCRC2)
    {
        double chitmp1=0;
        double chitmp2=0;
        for(int i=0;i<Np;i++)
        {
            chitmp1+=std::real(chi_h[i]);
            chitmp2+=std::real(chi_h[i]-chi[i]);
        }
        
        std::cout<<"chitmp1 "<<chitmp1<<std::endl;
        
        C1=(ei+chitmp2-sig*Dt/(2.0*e0))/(ei+chitmp1+sig*Dt/(2.0*e0));
        C2x=Dt/(Dx*e0*(ei+chitmp1+sig*Dt/(2.0*e0)));
        C2y=Dt/(Dy*e0*(ei+chitmp1+sig*Dt/(2.0*e0)));
        C2z=Dt/(Dz*e0*(ei+chitmp1+sig*Dt/(2.0*e0)));
        C3=1.0/(ei+chitmp1+sig*Dt/(2.0*e0));
        C4=Dt/(e0*(ei+chitmp1+sig*Dt/(2.0*e0)));
    }
    else if(m_type==MAT_NAGRA_2LVL)
    {
        C1=1.0;
        C2x=Dt/(e0*Dx);
        C2y=Dt/(e0*Dy);
        C2z=Dt/(e0*Dz);
        C3=-1.0/e0;
        C4=Dt/e0;
    }
    else if(m_type==MAT_GAIN_NAGRA)
    {
        C1=1.0;
        C2x=Dt/(e0*ei*Dx);
        C2y=Dt/(e0*ei*Dy);
        C2z=Dt/(e0*ei*Dz);
        C3=-1.0/(e0*ei);
        C4=Dt/(e0*ei);
    }
}*/

double FDTD_Material::pml_coeff()
{
    return C4;
}

// Legacy - to be removed

/*void FDTD_Material::set_const_i(Imdouble eps,double lambda)
{
    double e_r=std::real(eps);
    double e_i=std::imag(eps);
    
    double w=2.0*Pi*c_light/lambda;
    
    if(std::abs(e_i)<1e-6)
    {
        set_const(e_r);
    }
    else if(e_r>0)
    {
        double A=e_r;
        double B=e_i;
        
        double ei_p=A/2.0;
        double es_p=A+2*B*B/A;
        double t0_p=2*B/A/w;
        
        std::cout<<"Debye "<<ei_p<<" "<<es_p<<" "<<t0_p<<std::endl;
        
        setdebye(ei_p,es_p,t0_p);
    }
    else
    {
        double ei_p=std::abs(e_r)+1.0;
        double A=e_r-ei_p;
        double g=-e_i*w/A;
        double wd=std::sqrt(-w*w/A*(e_i*e_i+A*A));
        
        std::cout<<"Drude "<<ei_p<<" "<<g<<" "<<wd<<std::endl;
        
        setdrude(ei_p,wd,g);
    }
}


void FDTD_Material::setdebye(double ei_in,double es,double t0)
{
    //Debye
    //chi(t)=(es-ei)/t0*exp(-t/t0)
    
    Np=1;
    Np_r=0;
    if(m_type==MAT_PCRC2) Np_r=3;
    Np_c=3;
    
    realloc();
    
    ei=ei_in;
    sig=0;
    
    if(m_type==MAT_RC)
    {
        Crec(0)=std::exp(-Dt/t0);
        chi(0)=(es-ei)*(1.0-Crec(0));
        Dchi(0)=(es-ei)*std::pow(1.0-Crec(0),2.0);
    }
    else if(m_type==MAT_PCRC2)
    {
        double a=(es-ei)/t0;
        double b=-1.0/t0;
        
        using std::exp;
        
        Crec(0)=exp(b*Dt);
        chi(0)=a/b*exp(b*Dt/2.0)*(exp(b*Dt)-1.0);
        chi_h(0)=a/b*(exp(b*Dt/2.0)-1.0);
        Dchi(0)=-a/b*(exp(b*Dt)-1.0)*(exp(b*Dt)-1.0)*exp(b*Dt/2.0);
        
        if(t0==0)
        {
            Crec(0)=chi(0)=chi_h(0)=Dchi(0)=0;
        }
    }
    
    recalc();
}

void FDTD_Material::setnagra2lvl(double wa,double Dwa,double t21,double DN0,double kappa)
{
    m_type=MAT_NAGRA_2LVL;
    
    Np=Np_c=0;
    Np_r=13;
    
    n2l_wa=wa;
    n2l_Dwa=Dwa;
    n2l_T21=t21;
    n2l_DN0=DN0;
    n2l_kappa=kappa;
    
    n2l_P0=(2.0*Dt*Dt)/(2.0+Dwa*Dt);
    n2l_P1=kappa;
    n2l_P2=2.0/(Dt*Dt)-wa*wa;
    n2l_P3=Dwa/(2.0*Dt)-1.0/(Dt*Dt);
    
    n2l_N0=2.0*t21*Dt/(2.0*t21+Dt);
    n2l_N1=1.0/Dt-1.0/(2.0*t21);
    n2l_N2=DN0/t21;
    n2l_N3=-1.0/(Dt*hbar*wa);
        
    recalc();
}

void FDTD_Material::setnagra4lvl(double wa,double Dwa,double kappa,double Wp,double ei_in)
{
    m_type=MAT_GAIN_NAGRA;
    
    ei=ei_in;
        
    n4l_wa=wa;
    n4l_Dwa=Dwa;
    n4l_kappa=kappa;
    n4l_Wp=Wp;
    
    double t32=0.99e-10;
    double t31=1e-6;
    double t21=1.35e-7;
    double t20=1.0;
    double t10=1e-9;
        
    double t3=1.0/(1.0/t32+1.0/t31);
    double t2=1.0/(1.0/t21+1.0/t20);
    double t1=1.0/(1.0/t20+1.0/t10);
    
    std::cout<<t1<<" "<<t2<<" "<<t3<<std::endl;
    std::cout<<wa<<" "<<Dwa<<" "<<kappa<<" "<<Wp<<std::endl;
    
    n4l_P0=(2.0*Dt*Dt)/(2.0+Dwa*Dt);
    n4l_P1=kappa;
    n4l_P2=2.0/(Dt*Dt)-wa*wa;
    n4l_P3=Dwa/(2.0*Dt)-1.0/(Dt*Dt);
    
    n4l_N30=2.0*t3*Dt/(2.0*t3+Dt);
    n4l_N31=1.0/Dt-1.0/(2.0*t3);
    
    n4l_N20=2.0*t2*Dt/(2.0*t2+Dt);
    n4l_N21=1.0/Dt-1.0/(2.0*t2);
    n4l_N22=1.0/(2.0*t32);
    n4l_N23=1.0/(hbar*wa*Dt);
    
    n4l_N10=2.0*t1*Dt/(2.0*t1+Dt);
    n4l_N11=1.0/Dt-1.0/(2.0*t1);
    n4l_N12=1.0/(2.0*t31);
    n4l_N13=1.0/(2.0*t21);
    n4l_N14=-1.0/(hbar*wa*Dt);
        
    recalc();
    
    show();
}*/

void FDTD_Material::show()
{
    using std::cout;
    using std::endl;
    
    cout<<endl<<"# poles: "<<Np<<endl;
    cout<<"Dt: "<<Dt<<endl;
    cout<<"Delta: "<<Dx<<" "<<Dy<<" "<<Dz<<endl;
    cout<<"C1: "<<C1<<endl;
    cout<<"C2: "<<C2x<<" "<<C2y<<" "<<C2z<<endl;
    cout<<"C3: "<<C3<<endl;
    cout<<"Crec: ";
    for(int i=0;i<Np;i++) std::cout<<Crec[i]<<" ";
    cout<<endl;
    cout<<"ei: "<<ei<<endl;
    cout<<"sigma: "<<sig<<endl;
    
    cout<<endl<<"Span x: "<<x1<<" "<<x2
              <<" Span y: "<<y1<<" "<<y2
              <<" Span z: "<<z1<<" "<<z2<<endl;
    cout<<comp_simp<<" "<<comp_ante<<" "<<comp_post<<" "<<comp_self<<" "<<comp_D<<endl<<endl;
}

void FDTD_Material::operator = (double a)
{
    m_type=MAT_CONST;
    Np=0;
    Np_r=0;
    Np_c=0;
    
    Dt=1;
    Dx=Dy=Dz=1;
    C1=1;
    C2x=C2y=C2z=0;
    C3=0;
    C4=0;
    ei=1;
    sig=0;
    
    realloc();
}

void FDTD_Material::operator = (FDTD_Material const & Mi)
{
    Np=Mi.Np;
    Np_r=Mi.Np_r;
    Np_c=Mi.Np_c;
    
    Dt=Mi.Dt;
    Dx=Mi.Dx;
    Dy=Mi.Dy;
    Dz=Mi.Dz;
    C1=Mi.C1;
    C2x=Mi.C2x;
    C2y=Mi.C2y;
    C2z=Mi.C2z;
    C3=Mi.C3;
    C4=Mi.C4;
    
    ei=Mi.ei;
    sig=Mi.sig;
    
    realloc();
    
    Crec=Mi.Crec;
    chi=Mi.chi;
    chi_h=Mi.chi_h;
    Dchi=Mi.Dchi;
}
