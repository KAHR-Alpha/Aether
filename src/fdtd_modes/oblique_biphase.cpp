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
#include <sources.h>


extern const Imdouble Im;

extern std::ofstream plog;

//####################
//   Oblique phase
//####################

void FDTD_oblique_biphase_comp(FDTD_Mode const &fdtd_mode,double kx_in,double ky_in,
                               Spectrum &TE_spectrum_ref_out,Spectrum &TM_spectrum_ref_out,
                               Spectrum &TE_spectrum_trans_out,Spectrum &TM_spectrum_trans_out,int sim_index)
{
    int i,j,k,l,t;
    
    int Nx=60;
    int Ny=60;
    int Nz=60;
    int Nt=fdtd_mode.Nt;
    
    double Dx=fdtd_mode.Dx;
    double Dy=fdtd_mode.Dy;
    double Dz=fdtd_mode.Dz;
    
    double lx,ly,lz;
    
    fdtd_mode.structure->retrieve_nominal_size(lx,ly,lz);
    fdtd_mode.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
    fdtd_mode.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    std::string polar_mode=fdtd_mode.polarization;
    
    double Dt=std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light)*0.99*fdtd_mode.time_mod;
    
    /////////////////////////
    
    FDTD fdtd_r(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"OBL_PHASE",
                0,0,0,0,fdtd_mode.pml_zm,fdtd_mode.pml_zp,
                0,0,0,0,std::max(5,fdtd_mode.pad_zm),std::max(5,fdtd_mode.pad_zp));
    FDTD fdtd_i(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"OBL_PHASE",
                0,0,0,0,fdtd_mode.pml_zm,fdtd_mode.pml_zp,
                0,0,0,0,std::max(5,fdtd_mode.pad_zm),std::max(5,fdtd_mode.pad_zp));
    
    // PML
    
    fdtd_r.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd_r.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    fdtd_i.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd_i.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    
    std::string new_prefix_str=fdtd_mode.prefix;
    
    fdtd_r.set_prefix(new_prefix_str);
    fdtd_i.set_prefix(new_prefix_str);
    
    fdtd_r.set_tapering(fdtd_mode.tapering);
    fdtd_i.set_tapering(fdtd_mode.tapering);
    
    fdtd_r.set_matsgrid(matsgrid);
    fdtd_i.set_matsgrid(matsgrid);
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdtd_mode.materials_str.size();m++)
    {
        fdtd_r.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
        fdtd_i.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
    }
    #endif
    for(unsigned int m=0;m<fdtd_mode.materials.size();m++)
    {
        fdtd_r.set_material(m,fdtd_mode.materials[m]);
        fdtd_i.set_material(m,fdtd_mode.materials[m]);
    }
    
    /////////////////////////
    
    fdtd_r.disable_fields(fdtd_mode.disable_fields);
    fdtd_i.disable_fields(fdtd_mode.disable_fields);
    
    //Spectrum
    
    int Nl=fdtd_mode.Nl;
    double lambda_min=fdtd_mode.lambda_min;
    double lambda_max=fdtd_mode.lambda_max;
    
    fdtd_r.bootstrap();
    fdtd_i.bootstrap();
    
    ///###############
    ///  Sim Start
    ///###############
    
    Nx=fdtd_r.Nx;
    Ny=fdtd_r.Ny;
    Nz=fdtd_r.Nz;
    
    int zs_s=fdtd_r.zs_s;
    int zs_e=fdtd_r.zs_e;
    
    Imdouble i_E,i_H;
    
    Grid1<Imdouble> mkx(Nx,0),mkx_s(Nx,0);
    
    Grid2<double> inc_Ex(Nx,Ny,0),inc_Ey(Nx,Ny,0),inc_Ez(Nx,Ny,0);
    Grid2<Imdouble> mk_x(Nx,Ny,0),mk_y(Nx,Ny,0),mk_z(Nx,Ny,0);
        
    double eps_sub=fdtd_r.mats[fdtd_r.matsgrid(0,0,zs_s)].ei;
    double eps_sup=fdtd_r.mats[fdtd_r.matsgrid(0,0,zs_e)].ei;
    double index_sub=std::sqrt(eps_sub);
    double index_sup=std::sqrt(eps_sup);
    
    double kx=kx_in*index_sup;
    double ky=ky_in*index_sup;
    double kxy=std::sqrt(kx*kx+ky*ky);
    
    std::cout<<"u "<<eps_sup<<"/"<<index_sup<<" d "<<eps_sub<<"/"<<index_sub<<std::endl;
    
    AngleOld pol=Degree(0);
    if(polar_mode=="TM") pol=Degree(90);
    
    Bloch_Wideband inc_field(0,Nx,0,Ny,0,fdtd_r.Nz_s,kx,ky,pol);
        
    inc_field.set_spectrum(fdtd_mode.lambda_min,fdtd_mode.lambda_max);
    
    inc_field.link(fdtd_r);
    
    Imdouble dephasE_x=std::exp(kx*Nx*Dx*Im);
    Imdouble dephasE_y=std::exp(ky*Ny*Dy*Im);
    
    Imdouble dephasH_x=std::exp(-kx*Nx*Dx*Im);
    Imdouble dephasH_y=std::exp(-ky*Ny*Dy*Im);
    
    fdtd_r.set_kx(kx); fdtd_r.set_ky(ky);
    fdtd_i.set_kx(kx); fdtd_i.set_ky(ky);
    
    for(i=0;i<Nx;i++)
    {
        mkx[i]=std::exp(-kx*i*Dx*Im);
        mkx_s[i]=std::exp(-kx*(i+0.5)*Dx*Im);
    }
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        mk_x(i,j)=std::exp(-(kx*(i+0.5)*Dx+ky*j*Dy)*Im);
        mk_y(i,j)=std::exp(-(kx*i*Dx+ky*(j+0.5)*Dy)*Im);
        mk_z(i,j)=std::exp(-(kx*i*Dx+ky*j*Dy)*Im);
    }}
    
    int pk=0;
    int vmode=0;
    
    Grid1<Imdouble> BRsensorX(Nt,0),BRsensorY(Nt,0),BRsensorZ(Nt,0);
    Grid1<Imdouble> RsensorX(Nt,0),RsensorY(Nt,0),RsensorZ(Nt,0);
    
    Grid1<Imdouble> BTsensorX(Nt,0),BTsensorY(Nt,0),BTsensorZ(Nt,0);
    Grid1<Imdouble> TsensorX(Nt,0),TsensorY(Nt,0),TsensorZ(Nt,0);
    
    fdtd_r.reset_fields();
    fdtd_i.reset_fields();
    
    fdtd_r.tstep=0;
    fdtd_i.tstep=0;
    
    //Adding sensors
    
    std::vector<Sensor*> sensors;
    
    for(unsigned int i=0;i<fdtd_mode.sensors.size();i++)
        sensors.push_back(generate_fdtd_sensor(fdtd_mode.sensors[i],fdtd_r));
    
    std::stringstream sim_name;
    sim_name<<"_"<<sim_index;
    
    for(unsigned int i=0;i<sensors.size();i++)
        sensors[i]->name.append(sim_name.str());
        
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
        cpl_sensor->link(fdtd_r);
        sensors.push_back(cpl_sensor);
    }
    
    ProgTimeDisp dspt(Nt);
    
    for(t=0;t<Nt;t++)
    {
        // Bloch Conditions - H
        
        for(k=0;k<Nz;k++)
        {
            for(i=0;i<Nx;i++)
            {
                i_H=fdtd_r.Hx(i,Ny-1,k)+fdtd_i.Hx(i,Ny-1,k)*Im;
                
                fdtd_r.Hx(i,Ny,k)=std::real(i_H*dephasH_y);
                fdtd_i.Hx(i,Ny,k)=std::imag(i_H*dephasH_y);
                
                i_H=fdtd_r.Hz(i,Ny-1,k)+fdtd_i.Hz(i,Ny-1,k)*Im;
                
                fdtd_r.Hz(i,Ny,k)=std::real(i_H*dephasH_y);
                fdtd_i.Hz(i,Ny,k)=std::imag(i_H*dephasH_y);
            }
            
            for(j=0;j<Ny;j++)
            {
                i_H=fdtd_r.Hy(Nx-1,j,k)+fdtd_i.Hy(Nx-1,j,k)*Im;
                
                fdtd_r.Hy(Nx,j,k)=std::real(i_H*dephasH_x);
                fdtd_i.Hy(Nx,j,k)=std::imag(i_H*dephasH_x);
                
                i_H=fdtd_r.Hz(Nx-1,j,k)+fdtd_i.Hz(Nx-1,j,k)*Im;
                
                fdtd_r.Hz(Nx,j,k)=std::real(i_H*dephasH_x);
                fdtd_i.Hz(Nx,j,k)=std::imag(i_H*dephasH_x);
            }
        }
        
        // E-field computation
        
        fdtd_r.update_E();
        fdtd_i.update_E();
    
        // E-field injection
        
        inc_field.inject_E(fdtd_r,fdtd_i);
        
        //Bloch Conditions - E
        
        for(k=0;k<Nz;k++)
        {
            for(i=0;i<Nx;i++)
            {
                i_E=fdtd_r.Ex(i,0,k)+fdtd_i.Ex(i,0,k)*Im;
                
                fdtd_r.Ex(i,Ny,k)=std::real(i_E*dephasE_y);
                fdtd_i.Ex(i,Ny,k)=std::imag(i_E*dephasE_y);
                
                i_E=fdtd_r.Ez(i,0,k)+fdtd_i.Ez(i,0,k)*Im;
                
                fdtd_r.Ez(i,Ny,k)=std::real(i_E*dephasE_y);
                fdtd_i.Ez(i,Ny,k)=std::imag(i_E*dephasE_y);
            }
            
            for(j=0;j<Ny;j++)
            {
                i_E=fdtd_r.Ey(0,j,k)+fdtd_i.Ey(0,j,k)*Im;
                
                fdtd_r.Ey(Nx,j,k)=std::real(i_E*dephasE_x);
                fdtd_i.Ey(Nx,j,k)=std::imag(i_E*dephasE_x);
                
                i_E=fdtd_r.Ez(0,j,k)+fdtd_i.Ez(0,j,k)*Im;
                
                fdtd_r.Ez(Nx,j,k)=std::real(i_E*dephasE_x);
                fdtd_i.Ez(Nx,j,k)=std::imag(i_E*dephasE_x);
            }
        }
        
        // H-field update
        
        fdtd_r.update_H();
        fdtd_i.update_H();
        
        // H-field injection
        
        inc_field.inject_H(fdtd_r,fdtd_i);
        
        for(unsigned int i=0;i<sensors.size();i++)
            sensors[i]->feed(fdtd_r);
        
        if(t/static_cast<double>(Nt)<=pk/100.0 && (t+1.0)/Nt>pk/100.0)
        {
            fdtd_r.draw(t,vmode,Nx/2,Ny/2,Nz/2);
            
            pk++;
        }
        
        //###############
        //   Fourier
        //###############
        
        double tb=(t+0.5)*Dt; //calibration
        
        double z=zs_s*Dz;
        
        BRsensorX[t]=0; BRsensorY[t]=0; BRsensorZ[t]=0;
        RsensorX[t]=0;  RsensorY[t]=0;  RsensorZ[t]=0;
        BTsensorX[t]=0; BTsensorY[t]=0; BTsensorZ[t]=0;
        TsensorX[t]=0;  TsensorY[t]=0;  TsensorZ[t]=0;
        
        inc_field.get_E(inc_Ex,inc_Ey,inc_Ez,z,tb);
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            BRsensorX[t]+=inc_Ex(i,j)*mk_x(i,j);
            BRsensorY[t]+=inc_Ey(i,j)*mk_y(i,j);
            BRsensorZ[t]+=inc_Ez(i,j)*mk_z(i,j);
            
            RsensorX[t]+=fdtd_r.Ex(i,j,zs_e+2)*mk_x(i,j);
            RsensorY[t]+=fdtd_r.Ey(i,j,zs_e+2)*mk_y(i,j);
            RsensorZ[t]+=0.5*(fdtd_r.Ez(i,j,zs_e+1)+fdtd_r.Ez(i,j,zs_e+2))*mk_z(i,j);
            
            BTsensorX[t]+=inc_Ex(i,j)*mk_x(i,j);
            BTsensorY[t]+=inc_Ey(i,j)*mk_y(i,j);
            BTsensorZ[t]+=inc_Ez(i,j)*mk_z(i,j);
            
            TsensorX[t]+=fdtd_r.Ex(i,j,zs_s)*mk_x(i,j);
            TsensorY[t]+=fdtd_r.Ey(i,j,zs_s)*mk_y(i,j);
            TsensorZ[t]+=0.5*(fdtd_r.Ez(i,j,zs_s)+fdtd_r.Ez(i,j,zs_s-1))*mk_z(i,j);
        }
                
        if(time_type==TIME_FT && t%cc_step==0)
        {
            if(cpl_sensor->completion_check()) break;
            
            int Nt_est=std::min(Nt,cpl_sensor->estimate());
            dspt.set_end(Nt_est);
        }
        
        ++dspt;
    }
    
    // Spectrum resizing
    
    double lambda_limit_max=lambda_max;
    if(kx!=0 || ky!=0) lambda_limit_max=2*Pi*std::sin(89.0*Pi/180.0)*index_sup/kxy;
    
    if(lambda_limit_max<lambda_max)
    {
        double D_lambda=(lambda_max-lambda_min)/(Nl-1.0);
        
        Nl=static_cast<int>((lambda_limit_max-lambda_min)/D_lambda);
        if(Nl<2) Nl=2;
        
        lambda_max=lambda_min+(Nl-1.0)*D_lambda;
    }
    
    // Basic data analysis
    
    Grid1<Imdouble> BRSpX(Nl,0),BRSpY(Nl,0),BRSpZ(Nl,0);
    Grid1<Imdouble> RSpX(Nl,0),RSpY(Nl,0),RSpZ(Nl,0);
    
    Grid1<Imdouble> BTSpX(Nl,0),BTSpY(Nl,0),BTSpZ(Nl,0);
    Grid1<Imdouble> TSpX(Nl,0),TSpY(Nl,0),TSpZ(Nl,0);
    
    double hsup,hsub,hstruc;
    fdtd_r.find_slab(zs_s,zs_e+2,hsub,hstruc,hsup);
    
    ProgDisp dsp(Nl,"Fourier Transform");
    
    std::vector<double> lambda(Nl,0);
    std::vector<AngleOld> ang_arr(Nl);
    
    std::vector<Imdouble> base_TE_spectrum_ref(Nl,0);
    std::vector<Imdouble> base_TE_spectrum_trans(Nl,0);
    std::vector<Imdouble> base_TM_spectrum_ref(Nl,0);
    std::vector<Imdouble> base_TM_spectrum_trans(Nl,0);
    
    std::stringstream fang_name;
    fang_name<<fdtd_mode.prefix<<"angdata_"<<sim_index;
    std::ofstream fang(fang_name.str(),std::ios::out|std::ios::trunc);
        
    AngleOld inc_ang,
          ref_ang,
          phi(std::atan2(ky,kx));
    
    phi=std::atan2(ky,kx);
    
    double cp=std::cos(phi);
    double sp=std::sin(phi);
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        double w=2.0*Pi*c_light/lambda[l];
        double kn=2.0*Pi*index_sup/lambda[l];
        
        inc_ang=std::asin(kxy/kn);
        ref_ang=std::asin(index_sup/index_sub*std::sin(inc_ang));
        
        ang_arr[l]=inc_ang;
        
        fang<<lambda[l]<<" "<<inc_ang.degree()<<" "<<phi.degree()<<std::endl;
        
        double s_k0=-w/c_light;
        double s_k1=s_k0*index_sup;
        double s_k3=s_k0*index_sub;
        
        using std::cos;
        
        for(t=0;t<Nt;t++)
        {
            Imdouble coeff=std::exp(w*t*Dt*Im);
            
            BRSpX[l]+=BRsensorX[t]*coeff;
            BRSpY[l]+=BRsensorY[t]*coeff;
            BRSpZ[l]+=BRsensorZ[t]*coeff;
            
            RSpX[l]+=RsensorX[t]*coeff;
            RSpY[l]+=RsensorY[t]*coeff;
            RSpZ[l]+=RsensorZ[t]*coeff;
            
            BTSpX[l]+=BTsensorX[t]*coeff;
            BTSpY[l]+=BTsensorY[t]*coeff;
            BTSpZ[l]+=BTsensorZ[t]*coeff;
            
            TSpX[l]+=TsensorX[t]*coeff;
            TSpY[l]+=TsensorY[t]*coeff;
            TSpZ[l]+=TsensorZ[t]*coeff;
        }
        
        Imdouble tmp_x,tmp_y;
        
        tmp_x=BRSpX[l]; tmp_y=BRSpY[l]; 
        BRSpX[l]=cp*tmp_x+sp*tmp_y; BRSpY[l]=-sp*tmp_x+cp*tmp_y;
        
        tmp_x=RSpX[l]; tmp_y=RSpY[l]; 
        RSpX[l]=cp*tmp_x+sp*tmp_y; RSpY[l]=-sp*tmp_x+cp*tmp_y;
        
        tmp_x=BTSpX[l]; tmp_y=BTSpY[l]; 
        BTSpX[l]=cp*tmp_x+sp*tmp_y; BTSpY[l]=-sp*tmp_x+cp*tmp_y;
        
        tmp_x=TSpX[l]; tmp_y=TSpY[l]; 
        TSpX[l]=cp*tmp_x+sp*tmp_y; TSpY[l]=-sp*tmp_x+cp*tmp_y;
        
        using std::abs;
        using std::arg;
        using std::sqrt;
        using std::norm;
        using std::exp;
        
        double s_k1z=s_k1*std::cos(inc_ang);
        Imdouble tmp_sup=im_exp((2.0*hsup+2*Dz)*s_k1z);
        Imdouble tmp_sub=im_exp(s_k3*(hsub-Dz/2.0)*cos(ref_ang)+s_k0*(hsup+Dz/2.0)*cos(inc_ang));
        
        Imdouble Rbase_spectrum=BRSpY[l];
        Imdouble Tbase_spectrum=BTSpY[l];
        if(polar_mode=="TM")
        {
            Rbase_spectrum=BRSpX[l];
            Tbase_spectrum=BTSpX[l];
        }
        
        double base_norm_ref=abs(BRSpY[l]);
        double base_norm_trans=abs(BTSpY[l]);
        
        if(polar_mode=="TM")
        {
            base_norm_ref=sqrt(norm(BRSpX[l])+norm(BRSpZ[l]));
            base_norm_trans=sqrt(norm(BTSpX[l])+norm(BTSpZ[l]));
        }
        
        double TE_norm_ref=abs(RSpY[l]);
        double TM_norm_ref=sqrt(norm(RSpX[l])+norm(RSpZ[l]));
        double TE_norm_trans=abs(TSpY[l]);
        double TM_norm_trans=sqrt(norm(TSpX[l])+norm(TSpZ[l]));
        
        double TE_arg_ref=arg(RSpY[l]/Rbase_spectrum*tmp_sup);
        double TM_arg_ref=arg(RSpX[l]/Rbase_spectrum*tmp_sup);
        double TE_arg_trans=arg(TSpY[l]/Tbase_spectrum*tmp_sub);
        double TM_arg_trans=arg(TSpX[l]/Tbase_spectrum*tmp_sub);
        
        base_TE_spectrum_ref[l]=TE_norm_ref/base_norm_ref*exp(TE_arg_ref*Im);
        base_TM_spectrum_ref[l]=TM_norm_ref/base_norm_ref*exp(TM_arg_ref*Im);
        base_TE_spectrum_trans[l]=TE_norm_trans/base_norm_trans*exp(TE_arg_trans*Im);
        base_TM_spectrum_trans[l]=TM_norm_trans/base_norm_trans*exp(TM_arg_trans*Im);
        
        ++dsp;
    }
    
    Spectrum TE_spectrum_ref(lambda,ang_arr,base_TE_spectrum_ref);
    Spectrum TM_spectrum_ref(lambda,ang_arr,base_TM_spectrum_ref);
    
    TE_spectrum_ref.set_polarizations(polar_mode,"TE");
    TE_spectrum_ref.set_type_ref();
    TM_spectrum_ref.set_polarizations(polar_mode,"TM");
    TM_spectrum_ref.set_type_ref();
    
    Spectrum TE_spectrum_trans(lambda,ang_arr,base_TE_spectrum_trans);
    Spectrum TM_spectrum_trans(lambda,ang_arr,base_TM_spectrum_trans);
    
    TE_spectrum_trans.set_polarizations(polar_mode,"TE");
    TE_spectrum_trans.set_type_trans();
    TM_spectrum_trans.set_polarizations(polar_mode,"TM");
    TM_spectrum_trans.set_type_trans();
    
    TE_spectrum_ref_out=TE_spectrum_ref;
    TM_spectrum_ref_out=TM_spectrum_ref;
    TE_spectrum_trans_out=TE_spectrum_trans;
    TM_spectrum_trans_out=TM_spectrum_trans;
    
    for(unsigned int i=0;i<sensors.size();i++) sensors[i]->treat();
    
    for(unsigned int i=0;i<sensors.size();i++) delete sensors[i];
}

void FDTD_oblique_biphase_get_kp(FDTD_Mode const &fdtd_mode,std::vector<double> &kp_arr)
{
    int i,m;
    int Nkp;
    
    if(fdtd_mode.obl_phase_type==OBL_PHASE_KP_AUTO)
    {
        Nkp=1;
        
        double ang_min=fdtd_mode.obl_phase_amin.radian();
        double ang_max=fdtd_mode.obl_phase_amax.radian();
        
        double tfsin=std::sin(ang_min)/std::sin(ang_max);
        double tmpl=fdtd_mode.lambda_max*tfsin;
        
        while(tmpl>fdtd_mode.lambda_min) //lambda_n=lambda_0 (sin th_min/sin th_max)^n
        {
            tmpl=tmpl*tfsin;
            Nkp++;
        }
        
        kp_arr.resize(Nkp);
        
        for(m=0;m<Nkp;m++) // kp_n=kp_0(sin th_max/sin th_min)^n
        {
            kp_arr[m]=2.0*Pi/fdtd_mode.lambda_max*std::sin(ang_max);
            
            for(i=0;i<m;i++) kp_arr[m]*=std::sin(ang_max)/std::sin(ang_min);
        }
    }
    else if(fdtd_mode.obl_phase_type==OBL_PHASE_KP_FULL)
    {
        Nkp=fdtd_mode.obl_phase_Nkp;
        kp_arr.resize(Nkp);
        
        double kp_ic=fdtd_mode.obl_phase_kp_ic;
        double kp_fc=fdtd_mode.obl_phase_kp_fc;
        
        double lambda_min=fdtd_mode.lambda_min;
        double lambda_max=fdtd_mode.lambda_max;
        
        kp_fc=std::clamp(kp_fc,0.0,0.95);
        kp_ic=std::clamp(kp_ic,0.0,kp_fc);
        
        for(m=0;m<Nkp;m++)
        {
            double m_ct=m/(Nkp-1.0);
            if(Nkp<=1) m_ct=1.0;
            
            double m_c=0.99*Pi/2.0*(kp_ic+(kp_fc-kp_ic)*m_ct);
            double m_lambda=lambda_min+std::cos(m_c)*(lambda_max-lambda_min);
            double m_ang=Pi/2.0*std::sin(m_c);
            
            kp_arr[m]=2.0*Pi/m_lambda*std::sin(m_ang);
        }
    }
    else if(fdtd_mode.obl_phase_type==OBL_PHASE_KP_FIXED_A)
    {
        Nkp=fdtd_mode.obl_phase_Nkp;
        kp_arr.resize(Nkp);
        
        double lambda_min=fdtd_mode.obl_phase_lmin;
        double lambda_max=fdtd_mode.obl_phase_lmax;
        double angle=fdtd_mode.obl_phase_amin.radian();
        
        for(m=0;m<Nkp;m++)
        {
            double lambda=lambda_max+(lambda_min-lambda_max)*m/(Nkp-1.0);
            kp_arr[m]=2.0*Pi/lambda*std::sin(angle);
        }
    }
    else if(fdtd_mode.obl_phase_type==OBL_PHASE_KP_FIXED_L)
    {
        Nkp=fdtd_mode.obl_phase_Nkp;
        kp_arr.resize(Nkp);
        
        double lambda=fdtd_mode.obl_phase_lmin;
        double ang_min=fdtd_mode.obl_phase_amin.radian();
        double ang_max=fdtd_mode.obl_phase_amax.radian();
        
        for(i=0;i<Nkp;i++)
        {
            double t_ang=ang_min+(ang_max-ang_min)*i/(Nkp-1.0);
            kp_arr[i]=2.0*Pi/lambda*std::sin(t_ang);
        }
    }
    else if(fdtd_mode.obl_phase_type==OBL_PHASE_KP_TARGET)
    {
        Nkp=1;
        kp_arr.resize(Nkp);
        
        kp_arr[0]=2.0*Pi/fdtd_mode.obl_phase_lmin*std::sin(fdtd_mode.obl_phase_amin.radian());
    }
}

void FDTD_oblique_biphase(FDTD_Mode const &fdtd_mode,std::atomic<bool> *end_computation,ProgTimeDisp *dsp_)
{
    int l,m;
    
    /////////////////////////
    
    std::vector<double> kp_arr;
    
    FDTD_oblique_biphase_get_kp(fdtd_mode,kp_arr);
        
    SpectrumCollec sp_collec;
        
    for(m=0;m<static_cast<int>(kp_arr.size());m++)
    {
        double kp=kp_arr[m];
        
        chk_var(kp);
        
        double kx=kp*std::cos(fdtd_mode.obl_phase_phi);
        double ky=kp*std::sin(fdtd_mode.obl_phase_phi);
        
        Spectrum TE_spectrum_ref,TE_spectrum_trans;
        Spectrum TM_spectrum_ref,TM_spectrum_trans;
        
        FDTD_oblique_biphase_comp(fdtd_mode,kx,ky,
                                  TE_spectrum_ref,TM_spectrum_ref,
                                  TE_spectrum_trans,TM_spectrum_trans,m);
        
        int Nl=TE_spectrum_ref.N;
        
        std::stringstream fname;
        fname<<fdtd_mode.prefix<<"obl_"<<m;
        
        std::ofstream file(fname.str(),std::ios::out|std::ios::trunc);
        
        for(l=0;l<Nl;l++)
        {
            using std::abs;
            using std::arg;
            
            double lambda=TE_spectrum_ref.lambda[l];
            double ang=TE_spectrum_ref.ang[l].degree();
            
            file<<lambda<<" "
                <<ang<<" "
                <<abs(TE_spectrum_ref.spect[l])<<" "<<arg(TE_spectrum_ref.spect[l])<<" "
                <<abs(TM_spectrum_ref.spect[l])<<" "<<arg(TM_spectrum_ref.spect[l])<<" "
                <<abs(TE_spectrum_trans.spect[l])<<" "<<arg(TE_spectrum_trans.spect[l])<<" "
                <<abs(TM_spectrum_trans.spect[l])<<" "<<arg(TM_spectrum_trans.spect[l])<<std::endl;
        }
        
        file.close();
    }
    
    std::string col_fname=fdtd_mode.prefix;
    col_fname.append("obl_fullspec");
    
    sp_collec.write(col_fname);
}
