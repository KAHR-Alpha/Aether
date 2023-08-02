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
#include <data_hdl.h>
#include <fdtd_core.h>
#include <lua_fd.h>
#include <sensors.h>


extern const Imdouble Im;

extern std::ofstream plog;

//####################
//   Single particle
//####################

void FDTD_single_particle(FDTD_Mode const &fdtd_mode,std::atomic<bool> *end_computation,ProgTimeDisp *dsp_,Bitmap *bitmap_)
{
    int i,j,k,l,t;
    
    int Nx=60;
    int Ny=60;
    int Nz=60;
    int Nt=fdtd_mode.Nt;
    
    int Nx_aux=60;
    int Ny_aux=60;
    int Nz_aux=60;
    
    null_function(Nx_aux);
    
    double Dx=fdtd_mode.Dx;
    double Dy=fdtd_mode.Dy;
    double Dz=fdtd_mode.Dz;
    
    double lx,ly,lz;
    
    fdtd_mode.structure->retrieve_nominal_size(lx,ly,lz);
    fdtd_mode.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
    fdtd_mode.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    Grid3<unsigned int> aux_grid;
    
    // Auto auxiliary grid
    
    Nx_aux=Ny_aux=1;
    Nz_aux=Nz;
    
    aux_grid.init(1,1,Nz_aux,0);
    
    for(k=0;k<Nz_aux;k++)
        aux_grid(0,0,k)=matsgrid(0,0,k);
    
    std::string polar_mode=fdtd_mode.polarization;
    
    int pml_x=std::max(fdtd_mode.pml_xm,fdtd_mode.pml_xp);
    int pml_y=std::max(fdtd_mode.pml_ym,fdtd_mode.pml_yp);
    int pml_z=std::max(fdtd_mode.pml_zm,fdtd_mode.pml_zp);
    
    bool periodic_x=false,periodic_y=false;
    
    if(pml_x<=0) periodic_x=true;
    if(pml_y<=0) periodic_y=true;
    if(pml_z<=0) pml_z=25;
    
    double Dt=std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light)*0.99*fdtd_mode.time_mod;
    
//    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",
//              fdtd_mode.pml_xm,fdtd_mode.pml_xp,
//              fdtd_mode.pml_ym,fdtd_mode.pml_yp,
//              fdtd_mode.pml_zm,fdtd_mode.pml_zp);
//    
//    FDTD fdtd_aux(1,1,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",0,0,0,0,
//                  fdtd_mode.pml_zm,fdtd_mode.pml_zp);
    
    int pad_xm=std::max(10,fdtd_mode.pad_xm);
    int pad_xp=std::max(10,fdtd_mode.pad_xp);
    int pad_ym=std::max(10,fdtd_mode.pad_ym);
    int pad_yp=std::max(10,fdtd_mode.pad_yp);
    int pad_zm=std::max(10,fdtd_mode.pad_zm);
    int pad_zp=std::max(10,fdtd_mode.pad_zp);
    
    if(pml_x<=0) pad_xm=pad_xp=0;
    if(pml_y<=0) pad_ym=pad_yp=0;
    if(pml_z<=0) pad_zm=pad_zp=0;
    
    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",
              fdtd_mode.pml_xm,fdtd_mode.pml_xp,
              fdtd_mode.pml_ym,fdtd_mode.pml_yp,
              fdtd_mode.pml_zm,fdtd_mode.pml_zp,
              pad_xm,pad_xp,pad_ym,pad_yp,pad_zm,pad_zp);
    
    FDTD fdtd_aux(1,1,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",0,0,0,0,
                  fdtd_mode.pml_zm,fdtd_mode.pml_zp,
                  0,0,0,0,pad_zm,pad_zp);
    
    // PML
    
    fdtd.set_pml_xm(fdtd_mode.kappa_xm,fdtd_mode.sigma_xm,fdtd_mode.alpha_xm);
    fdtd.set_pml_xp(fdtd_mode.kappa_xp,fdtd_mode.sigma_xp,fdtd_mode.alpha_xp);
    fdtd.set_pml_ym(fdtd_mode.kappa_ym,fdtd_mode.sigma_ym,fdtd_mode.alpha_ym);
    fdtd.set_pml_yp(fdtd_mode.kappa_yp,fdtd_mode.sigma_yp,fdtd_mode.alpha_yp);
    fdtd.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    
    fdtd_aux.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd_aux.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    
    fdtd.set_prefix(fdtd_mode.prefix);
    
    // Grid and materials
    
    fdtd.set_matsgrid(matsgrid);
    fdtd_aux.set_matsgrid(aux_grid);
    
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    
    int xs_s=fdtd.xs_s;
    int xs_e=fdtd.xs_e;
    int ys_s=fdtd.ys_s;
    int ys_e=fdtd.ys_e;
    int zs_s=fdtd.zs_s;
    int zs_e=fdtd.zs_e;
    
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    
    chk_var(fdtd.zs_s);
    chk_var(fdtd.zs_e);
    chk_var(fdtd.Nz);
    chk_var(fdtd_aux.zs_s);
    chk_var(fdtd_aux.zs_e);
    chk_var(fdtd_aux.Nz);
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdtd_mode.materials_str.size();m++)
    {
        fdtd.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
        fdtd_aux.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
    }
    #endif
    for(unsigned int m=0;m<fdtd_mode.materials.size();m++)
    {
        fdtd.set_material(m,fdtd_mode.materials[m]);
        fdtd_aux.set_material(m,fdtd_mode.materials[m]);
    }
    
    // Disabling fields
    
    fdtd.disable_fields(fdtd_mode.disable_fields);
    
    // Spectrum and incident field
    
    int Nl=fdtd_mode.Nl;
    double lambda_min=fdtd_mode.lambda_min;
    double lambda_max=fdtd_mode.lambda_max;
    
    Grid1<double> Lambda(Nl,0);
    Grid1<double> w(Nl,0);
    
    for(l=0;l<Nl;l++) Lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
    for(l=0;l<Nl;l++) w[l]=2.0*Pi*c_light/Lambda[l];
    
    double pol=0;
    
    if(polar_mode=="TE") pol=0;
    else if(polar_mode=="TM") pol=90;
    else if(polar_mode=="mix") pol=fdtd_mode.polar_angle;
    
    double eps_sub=fdtd.mats[fdtd.matsgrid(0,0,zs_s)].ei;
    double eps_sup=fdtd.mats[fdtd.matsgrid(0,0,zs_e)].ei;
    double index_sub=std::sqrt(eps_sub);
    double index_sup=std::sqrt(eps_sup);
    
    std::cout<<"u "<<eps_sup<<"/"<<index_sup<<" d "<<eps_sub<<"/"<<index_sub<<std::endl;
    
    ChpIn inj_chp;
    inj_chp.init_pulse(lambda_min,lambda_max,0,pol,0,0,0,(zs_e-1+5)*Dz,1.0);
    
    std::cout<<"Computing"<<std::endl;
    
    fdtd.bootstrap();
    
    //Adding sensors
    
    std::vector<Sensor*> sensors;
    
    for(unsigned int i=0;i<fdtd_mode.sensors.size();i++)
        sensors.push_back(generate_fdtd_sensor(fdtd_mode.sensors[i],fdtd));
    
    //Completion check
    
    int time_type=fdtd_mode.time_type;
    int cc_step=fdtd_mode.cc_step;
    double cc_lmin=fdtd_mode.cc_lmin,
           cc_lmax=fdtd_mode.cc_lmax,
           cc_coeff=fdtd_mode.cc_coeff,
           cc_quant=fdtd_mode.cc_quant;
    std::string cc_layout=fdtd_mode.cc_layout;
    
    CompletionSensor *cpl_sensor=nullptr;
    
    if(time_type==TIME_FT)
    {
        cpl_sensor=new CompletionSensor(cc_lmin,cc_lmax,cc_coeff,cc_quant,cc_layout);
        cpl_sensor->link(fdtd);
        sensors.push_back(cpl_sensor);
    }
    else if(time_type==TIME_ENERGY)
    {
        cpl_sensor=new CompletionSensor(cc_coeff);
        cpl_sensor->link(fdtd);
        sensors.push_back(cpl_sensor);
    }
    
    double tb=0;
    
    // Real-time outputs
    
    ProgTimeDisp *dspt=nullptr;
    Bitmap *bitmap=nullptr;
    
    if(dsp_!=nullptr)
    {
        dspt=dsp_;
        dspt->reset(Nt);
    }
    else dspt=new ProgTimeDisp(Nt);
    
    int N_disp=std::max(Nt/100,1);
    if(fdtd_mode.display_step>0) N_disp=fdtd_mode.display_step;
    
    if(bitmap_!=nullptr)
    {
        bitmap=bitmap_;
        if(fdtd_mode.display_step>0) N_disp=fdtd_mode.display_step;
        else N_disp=100;
    }
    else bitmap=new Bitmap(512,512);
    
    for(t=0;t<Nt;t++)
    {
        // Fields update
        
        // E-field
//        fdtd.update_E();
        fdtd.update_E_ante();
        fdtd.update_E_self();
//        fdtd_aux.update_E();
        fdtd_aux.update_E_ante();
        fdtd_aux.update_E_self();
        
        // E-field injection
        
        tb=t*Dt;
        double inj_Hx=inj_chp.Hx(0,0,(zs_e-1+5.5)*Dz,tb);
        double inj_Hy=inj_chp.Hy(0,0,(zs_e-1+5.5)*Dz,tb);
        
        fdtd_aux.Ex(0,0,zs_e-1+5)-=fdtd.dtdez*inj_Hy/eps_sup;
        fdtd_aux.Ey(0,0,zs_e-1+5)+=fdtd.dtdez*inj_Hx/eps_sup;
        
        int i1=xs_s; int i2=xs_e;
        int j1=ys_s; int j2=ys_e;
        int k1=zs_s; int k2=zs_e;
        
        double tmp1,tmp2;
        double C2x1,C2x2;
        double C2y1,C2y2;
        double C2z1,C2z2;
        
        // Z
        for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
        {
            fdtd.mats[fdtd.matsgrid(i,j,k1)].coeffsX(tmp1,tmp2,C2z1);
            fdtd.mats[fdtd.matsgrid(i,j,k2)].coeffsX(tmp1,tmp2,C2z2);
            
            fdtd.Ex(i,j,k1)+=C2z1*fdtd_aux.Hy(0,0,k1-1);
            fdtd.Ex(i,j,k2)-=C2z2*fdtd_aux.Hy(0,0,k2-1);
            
            fdtd.mats[fdtd.matsgrid(i,j,k1)].coeffsY(tmp1,tmp2,C2z1);
            fdtd.mats[fdtd.matsgrid(i,j,k2)].coeffsY(tmp1,tmp2,C2z2);
            
            fdtd.Ey(i,j,k1)-=C2z1*fdtd_aux.Hx(0,0,k1-1);
            fdtd.Ey(i,j,k2)+=C2z2*fdtd_aux.Hx(0,0,k2-1);
        }
        
        // Y
        if(!periodic_y) for(i=i1;i<i2;i++) for(k=k1;k<k2;k++)
        {
            fdtd.mats[fdtd.matsgrid(i,j1,k)].coeffsX(tmp1,C2y1,tmp2);
            fdtd.mats[fdtd.matsgrid(i,j2,k)].coeffsX(tmp1,C2y2,tmp2);
            
            fdtd.Ex(i,j1,k)-=C2y1*fdtd_aux.Hz(0,0,k);
            fdtd.Ex(i,j2,k)+=C2y2*fdtd_aux.Hz(0,0,k);
            
            fdtd.mats[fdtd.matsgrid(i,j1,k)].coeffsZ(tmp1,tmp2,C2y1);
            fdtd.mats[fdtd.matsgrid(i,j2,k)].coeffsZ(tmp1,tmp2,C2y2);
            
            fdtd.Ez(i,j1,k)+=C2y1*fdtd_aux.Hx(0,0,k);
            fdtd.Ez(i,j2,k)-=C2y2*fdtd_aux.Hx(0,0,k);
        }
        
        // X
        if(!periodic_x) for(k=k1;k<k2;k++) for(j=j1;j<j2;j++)
        {
            fdtd.mats[fdtd.matsgrid(i1,j,k)].coeffsY(tmp1,C2x1,tmp2);
            fdtd.mats[fdtd.matsgrid(i2,j,k)].coeffsY(tmp1,C2x2,tmp2);
            
            fdtd.Ey(i1,j,k)+=C2x1*fdtd_aux.Hz(0,0,k);
            fdtd.Ey(i2,j,k)-=C2x2*fdtd_aux.Hz(0,0,k);
            
            fdtd.mats[fdtd.matsgrid(i1,j,k)].coeffsZ(tmp1,C2x1,tmp2);
            fdtd.mats[fdtd.matsgrid(i2,j,k)].coeffsZ(tmp1,C2x2,tmp2);
            
            fdtd.Ez(i1,j,k)-=C2x1*fdtd_aux.Hy(0,0,k);
            fdtd.Ez(i2,j,k)+=C2x2*fdtd_aux.Hy(0,0,k);
        }
        
        fdtd.update_E_post();
        fdtd_aux.update_E_post();
        
        // H-field
        fdtd.update_H();
        fdtd_aux.update_H();
        
        // H-field injection
        
        tb=(t+0.5)*Dt;
        double inj_Ex=inj_chp.Ex(0,0,(zs_e-1+5)*Dz,tb);
        double inj_Ey=inj_chp.Ey(0,0,(zs_e-1+5)*Dz,tb);
        
        fdtd_aux.Hx(0,0,zs_e-1+5)+=fdtd.dtdmz*inj_Ey;
        fdtd_aux.Hy(0,0,zs_e-1+5)-=fdtd.dtdmz*inj_Ex;
        
        // Z
        for(i=i1;i<i2;i++) for(j=j1;j<j2;j++)
        {
            fdtd.Hx(i,j,k1-1)-=fdtd.dtdmz*fdtd_aux.Ey(0,0,k1);
            fdtd.Hx(i,j,k2-1)+=fdtd.dtdmz*fdtd_aux.Ey(0,0,k2);
            
            fdtd.Hy(i,j,k1-1)+=fdtd.dtdmz*fdtd_aux.Ex(0,0,k1);
            fdtd.Hy(i,j,k2-1)-=fdtd.dtdmz*fdtd_aux.Ex(0,0,k2);
        }
        
        // Y
        if(!periodic_y) for(i=i1;i<i2;i++) for(k=k1;k<k2;k++)
        {
            fdtd.Hx(i,j1-1,k)+=fdtd.dtdmy*fdtd_aux.Ez(0,0,k);
            fdtd.Hx(i,j2-1,k)-=fdtd.dtdmy*fdtd_aux.Ez(0,0,k);
            
            fdtd.Hz(i,j1-1,k)-=fdtd.dtdmy*fdtd_aux.Ex(0,0,k);
            fdtd.Hz(i,j2-1,k)+=fdtd.dtdmy*fdtd_aux.Ex(0,0,k);
        }
        
        // X
        if(!periodic_x) for(k=k1;k<k2;k++) for(j=j1;j<j2;j++)
        {
            fdtd.Hy(i1-1,j,k)-=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
            fdtd.Hy(i2-1,j,k)+=fdtd.dtdmx*fdtd_aux.Ez(0,0,k);
            
            fdtd.Hz(i1-1,j,k)+=fdtd.dtdmx*fdtd_aux.Ey(0,0,k);
            fdtd.Hz(i2-1,j,k)-=fdtd.dtdmx*fdtd_aux.Ey(0,0,k);
        }
        
        for(unsigned int i=0;i<sensors.size();i++)
            sensors[i]->feed(fdtd);
        
        if(t%N_disp==0)
        {
            int vmode=0;
            fdtd.draw(t,vmode,Nx/2,Ny/2,Nz/2,bitmap);
            
            if(bitmap_==nullptr)
            {                
                std::stringstream K;
                K<<"render/render";
                K<<t;
                K<<".png";
                
                bitmap->write(K.str());
            }
        }
        
        if(time_type!=TIME_FIXED && t%cc_step==0)
        {
            if(cpl_sensor->completion_check()) break;
            
            int Nt_est=std::min(Nt,cpl_sensor->estimate());
            dspt->set_end(Nt_est);
        }
        
        if(end_computation!=nullptr && *end_computation) break;
        
        ++(*dspt);
    }
    
    for(unsigned int i=0;i<sensors.size();i++) sensors[i]->treat();
    
    for(unsigned int i=0;i<sensors.size();i++) delete sensors[i];
}
