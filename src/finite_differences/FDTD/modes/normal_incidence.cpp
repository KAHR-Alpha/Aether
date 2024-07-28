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
#include <lua_fdtd.h>

extern const Imdouble Im;
extern std::ofstream plog;

void norm_octave_script(std::string prefix,double n1,double n2)
{
    std::ofstream file(prefix+"show_norm.m",std::ios::out|std::ios::trunc);
    
    using std::endl;
    
    std::filesystem::path fname(prefix);
    prefix=fname.stem().generic_string();
    
    file<<"function out="<<prefix<<"show_norm(varargin)"<<endl;
    file<<endl;
    file<<"D=load('"<<prefix+"spectdata_norm"<<"');"<<endl;
    file<<"L=D(:,1)*1e6;"<<endl;
    file<<"r=D(:,2);"<<endl;
    file<<"t=D(:,3);"<<endl;
    file<<endl;
    file<<"R=r.^2;"<<endl;
    file<<"T="<<n2<<"/"<<n1<<"*t.^2;"<<endl;
    file<<"A=1-R-T;"<<endl;
    file<<endl;
    file<<"figure(1)"<<endl;
    file<<"plot(L,r,L,t)"<<endl;
    file<<"axis([min(L) max(L) 0 1])"<<endl;
    file<<endl;
    file<<"figure(2)"<<endl;
    file<<"plot(L,R,L,T,L,A)"<<endl;
    file<<"legend('R','T','A')"<<endl;
    file<<"xlabel('Lambda (microns)')"<<endl;
    file<<"axis([min(L) max(L) 0 1])"<<endl;
    file<<endl;
    file<<"if nargin>0"<<endl;
    file<<"    out=[L,R,T,A];"<<endl;
    file<<"end"<<endl;
    file<<endl;
    file<<"end"<<endl;
}

void norm2_octave_script(std::string prefix,double n1,double n2)
{
    std::stringstream strm1,strm2;
    strm1<<prefix<<"show_norm2.m";
    strm2<<prefix<<"spectdata_norm2";
    
    std::ofstream file(strm1.str().c_str(),std::ios::out|std::ios::trunc);
    
    using std::endl;
    
    file<<"function out="<<prefix<<"show_norm2()"<<endl;
    file<<endl;
    file<<"close all"<<endl;
    file<<endl;
    file<<"D=load(\""<<strm2.str()<<"\");"<<endl;
    file<<"L=D(:,1)*1e6;"<<endl;
    file<<"rx=D(:,2);"<<endl;
    file<<"rxa=D(:,3)*180/pi;"<<endl;
    file<<"ry=D(:,4);"<<endl;
    file<<"rya=D(:,5)*180/pi;"<<endl;
    file<<"rz=D(:,6);"<<endl;
    file<<"rza=D(:,7)*180/pi;"<<endl;
    file<<endl;
    
    file<<"tx=D(:,8);"<<endl;
    file<<"txa=D(:,9)*180/pi;"<<endl;
    file<<"ty=D(:,10);"<<endl;
    file<<"tya=D(:,11)*180/pi;"<<endl;
    file<<"tz=D(:,12);"<<endl;
    file<<"tza=D(:,13)*180/pi;"<<endl;
    file<<endl;
    
    file<<"R=rx.^2+ry.^2+rz.^2;"<<endl;
    file<<"T="<<n2<<"/"<<n1<<"*(tx.^2+ty.^2+tz.^2);"<<endl;
    file<<"A=1-R-T;"<<endl;
    file<<endl;
    
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,rx)"<<endl;
    file<<"title(\"ref_x\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,rxa)"<<endl;
    file<<"title(\"phase ref_x\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,ry)"<<endl;
    file<<"title(\"ref_y\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,rya)"<<endl;
    file<<"title(\"phase ref_y\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,rz)"<<endl;
    file<<"title(\"ref_z\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,rza)"<<endl;
    file<<"title(\"phase ref_z\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,tx)"<<endl;
    file<<"title(\"trans_x\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,txa)"<<endl;
    file<<"title(\"phase trans_x\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,ty)"<<endl;
    file<<"title(\"trans_y\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,tya)"<<endl;
    file<<"title(\"phase trans_y\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"subplot(1,2,1)"<<endl;
    file<<"plot(L,tz)"<<endl;
    file<<"title(\"trans_z\")"<<endl;
    file<<"subplot(1,2,2)"<<endl;
    file<<"plot(L,tza)"<<endl;
    file<<"title(\"phase trans_z\")"<<endl;
    file<<endl;
    
    file<<"figure"<<endl;
    file<<"plot(L,R,L,T,L,A)"<<endl;
    file<<"legend('R','T','A')"<<endl;
    file<<"xlabel('Lambda (microns)')"<<endl;
    file<<"axis([min(L) max(L) 0 1])"<<endl;
    file<<endl<<"end";
}

void FDTD_normal_incidence(FDTD_Mode const &fdtd_mode,std::atomic<bool> *end_computation,ProgTimeDisp *dsp_,Bitmap *bitmap_)
{
    int i,j,l,t;
    
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
    
//    int pml_z=std::max(fdtd_mode.pml_zm,fdtd_mode.pml_zp);
//    
//    if(pml_z<=0) pml_z=25;
    
    int pad_zm=std::max(5,fdtd_mode.pad_zm);
    int pad_zp=std::max(5,fdtd_mode.pad_zp);
    
    double Dt=std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light)*0.99*fdtd_mode.time_mod;
    
    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"NORM",
              0,0,0,0,fdtd_mode.pml_zm,fdtd_mode.pml_zp,
              0,0,0,0,pad_zm,pad_zp);
    
    // PML
    
    fdtd.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    
    fdtd.set_tapering(fdtd_mode.tapering);
    
    // Grid and materials
    
    fdtd.set_matsgrid(matsgrid);
    
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    
    int zs_s=fdtd.zs_s;
    int zs_e=fdtd.zs_e;
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdtd_mode.materials_str.size();m++)
        fdtd.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
    #endif
    for(unsigned int m=0;m<fdtd_mode.materials.size();m++)
        fdtd.set_material(m,fdtd_mode.materials[m]);
    
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
    if(polar_mode=="TM") pol=90;
    
    double eps_sub=fdtd.mats[fdtd.matsgrid(0,0,zs_s)].ei;
    double eps_sup=fdtd.mats[fdtd.matsgrid(0,0,zs_e)].ei;
    double index_sub=std::sqrt(eps_sub);
    double index_sup=std::sqrt(eps_sup);
    
    std::cout<<"u "<<eps_sup<<"/"<<index_sup<<" d "<<eps_sub<<"/"<<index_sub<<std::endl;
    
    ChpIn inj_chp;
    inj_chp.init_pulse(lambda_min,lambda_max,0,pol,0,0,0,(zs_e-1)*Dz,index_sup);
    
    std::cout<<"Computing"<<std::endl;
    
    fdtd.bootstrap();
    
    Grid1<double> BRsensorX(Nt,0),BRsensorY(Nt,0),BRsensorZ(Nt,0);
    Grid1<double> RsensorX(Nt,0),RsensorY(Nt,0),RsensorZ(Nt,0);
    
    Grid1<double> BTsensorX(Nt,0),BTsensorY(Nt,0),BTsensorZ(Nt,0);
    Grid1<double> TsensorX(Nt,0),TsensorY(Nt,0),TsensorZ(Nt,0);
    
    //Adding sensors
    
    std::vector<Sensor*> sensors;
    
    for(unsigned int i=0;i<fdtd_mode.sensors.size();i++)
        sensors.push_back(generate_fdtd_sensor(fdtd_mode.sensors[i], fdtd, fdtd_mode.directory()));
    
    //Completion check
    
    int time_type=fdtd_mode.time_type;
    int cc_step=fdtd_mode.cc_step;
    double cc_lmin=fdtd_mode.cc_lmin,
           cc_lmax=fdtd_mode.cc_lmax,
           cc_coeff=fdtd_mode.cc_coeff,
           cc_quant=fdtd_mode.cc_quant;
    std::string cc_layout=fdtd_mode.cc_layout;
    
    chk_var(cc_step);
    chk_var(cc_lmin);
    chk_var(cc_lmax);
    chk_var(cc_coeff);
    chk_var(cc_quant);
    
    CompletionSensor *cpl_sensor=nullptr;
    
    if(time_type==TIME_FT)
    {
        cpl_sensor=new CompletionSensor(cc_lmin,cc_lmax,cc_coeff,cc_quant,cc_layout);
        cpl_sensor->link(fdtd, fdtd_mode.directory());
        sensors.push_back(cpl_sensor);
    }
    else if(time_type==TIME_ENERGY)
    {
        cpl_sensor=new CompletionSensor(cc_coeff);
        cpl_sensor->link(fdtd, fdtd_mode.directory());
        sensors.push_back(cpl_sensor);
    }
    
    double tb=0;
    
    // Real-time outputs
    
    ProgTimeDisp *dspt=nullptr;
    
    if(dsp_!=nullptr)
    {
        dspt=dsp_;
        dspt->reset(Nt);
    }
    else dspt=new ProgTimeDisp(Nt);
    
    Bitmap *bitmap=nullptr;
    
    int N_disp=std::max(Nt/100,1);
    if(fdtd_mode.display_step>0) N_disp=fdtd_mode.display_step;
    
    if(bitmap_!=nullptr)
    {
        bitmap=bitmap_;
        if(fdtd_mode.display_step>0) N_disp=fdtd_mode.display_step;
        else N_disp=20;
    }
    else bitmap=new Bitmap(512,512);
    
    for(t=0;t<Nt;t++)
    {
        // Fields update
        
        // E-field
        fdtd.update_E();
        
        // E-field injection
        
        tb=t*Dt;
        double inj_Hx=inj_chp.Hx(0,0,(zs_e-1+0.5)*Dz,tb);
        double inj_Hy=inj_chp.Hy(0,0,(zs_e-1+0.5)*Dz,tb);
        
        for(j=0;j<Ny;j++){ for(i=0;i<Nx;i++)
        {
            fdtd.Ex(i,j,zs_e-1)-=fdtd.dtdez*inj_Hy/eps_sup;
            fdtd.Ey(i,j,zs_e-1)+=fdtd.dtdez*inj_Hx/eps_sup;
        }}
        
        // H-field
        fdtd.update_H();
        
        // H-field injection
        
        tb=(t+0.5)*Dt;
        double inj_Ex=inj_chp.Ex(0,0,(zs_e-1)*Dz,tb);
        double inj_Ey=inj_chp.Ey(0,0,(zs_e-1)*Dz,tb);
        
        for(j=0;j<Ny;j++){ for(i=0;i<Nx;i++)
        {
            fdtd.Hx(i,j,zs_e-1)+=fdtd.dtdmz*inj_Ey;
            fdtd.Hy(i,j,zs_e-1)-=fdtd.dtdmz*inj_Ex;
        }}
        
        for(unsigned int i=0;i<sensors.size();i++)
            sensors[i]->feed(fdtd);
        
        if(t%N_disp==0)
        {
            int vmode=0;
            fdtd.draw(t,vmode,Nx/2,Ny/2,Nz/2,bitmap);
//            
            if(bitmap_==nullptr)
            {                
                std::stringstream K;
                K<<"render/render";
                K<<t;
                K<<".png";
                
                bitmap->write(K.str());
            }
        }
        
        tb=(t+0.5)*Dt; //calibration
        
        double Dx2=Dx/2.0;
        double Dy2=Dy/2.0;
        double Dz2=Dz/2.0;
        
        double tapering=1.0;
        if(t>=Nt-fdtd_mode.tapering) tapering=s_curve(t,Nt-1.0,Nt-fdtd_mode.tapering);
        
        for(j=0;j<Ny;j++)
        {
            double y=j*Dy;
            
            for(i=0;i<Nx;i++)
            {
                double x=i*Dx;
                
                double z=(zs_e-2)*Dz;
                
                BRsensorX[t]+=inj_chp.Ex(x+Dx2,y,z,tb)*tapering;
                BRsensorY[t]+=inj_chp.Ey(x,y+Dy2,z,tb)*tapering;
                BRsensorZ[t]+=inj_chp.Ez(x,y,z+Dz2,tb)*tapering;
                
                RsensorX[t]+=(fdtd.Ex(i,j,zs_e-2)-inj_chp.Ex(x+Dx2,y,z,tb))*tapering;
                RsensorY[t]+=(fdtd.Ey(i,j,zs_e-2)-inj_chp.Ey(x,y+Dy2,z,tb))*tapering;
                RsensorZ[t]+=(fdtd.Ez(i,j,zs_e-2)-inj_chp.Ez(x,y,z+Dz2,tb))*tapering;
                
                z=(zs_s)*Dz;
                
                BTsensorX[t]+=inj_chp.Ex(x+Dx2,y,z,tb)*tapering;
                BTsensorY[t]+=inj_chp.Ey(x,y+Dy2,z,tb)*tapering;
                BTsensorZ[t]+=inj_chp.Ez(x,y,z+Dz2,tb)*tapering;
                
                TsensorX[t]+=fdtd.Ex(i,j,zs_s)*tapering;
                TsensorY[t]+=fdtd.Ey(i,j,zs_s)*tapering;
                TsensorZ[t]+=fdtd.Ez(i,j,zs_s)*tapering;
            }
        }
        
        if(time_type!=TIME_FIXED && t%cc_step==0)
        {
            if(cpl_sensor->completion_check())
            {
                Nt=t;
                break;
            }
            
            int Nt_est=std::min(Nt,cpl_sensor->estimate());
            dspt->set_end(Nt_est);
        }
        
        ++(*dspt);
        
        if(end_computation!=nullptr && *end_computation)
        {
            Nt=t;
            break;
        }
    }
    
    std::cout<<"Simulation end"<<std::endl;
    
    if(dsp_==nullptr) delete dspt;
    
    Grid1<Imdouble> ref(Nl,0),ref1x(Nl,0),ref1y(Nl,0),ref1z(Nl,0),ref2x(Nl,0),ref2y(Nl,0),ref2z(Nl,0);
    Grid1<Imdouble> trans(Nl,0),trans1x(Nl,0),trans1y(Nl,0),trans1z(Nl,0),trans2x(Nl,0),trans2y(Nl,0),trans2z(Nl,0);
    
    double hsup,hsub,hstruc;
    fdtd.find_slab(zs_s,zs_e-1,hsub,hstruc,hsup);
    
    hsup-=Dz/2.0; //! Phase calibration, unknown reason
    hsub-=Dz/2.0; //! Phase calibration, unknown reason
    
    std::cout<<hsup<<" "<<hsub<<" "<<hstruc<<std::endl;
    
    ProgDisp dsp(Nl,"Fourier Transform");
    
    for(l=0;l<Nl;l++)
    {
        double k0=-w[l]/c_light;
        double k1=k0*index_sup;
        double k3=k0*index_sub;
        
        Imdouble tmp_sup=std::exp(2.0*k1*hsup*Im);
        Imdouble tmp_sub=std::exp((k3*hsub-k0*(hstruc+hsub))*Im);
        
        for(t=0;t<Nt;t++)
        {
            Imdouble tmp1=std::exp(t*Dt*w[l]*Im)/(Nt-1.0);
            Imdouble tmp2=tmp1*tmp_sup;
            Imdouble tmp3=tmp1*tmp_sub;
            
            ref1x[l]+=BRsensorX[t]*tmp1;
            ref1y[l]+=BRsensorY[t]*tmp1;
            ref1z[l]+=BRsensorZ[t]*tmp1;
            
            ref2x[l]+=RsensorX[t]*tmp2;
            ref2y[l]+=RsensorY[t]*tmp2;
            ref2z[l]+=RsensorZ[t]*tmp2;
            
            trans1x[l]+=BTsensorX[t]*tmp1;
            trans1y[l]+=BTsensorY[t]*tmp1;
            trans1z[l]+=BTsensorZ[t]*tmp1;
            
            trans2x[l]+=TsensorX[t]*tmp3;
            trans2y[l]+=TsensorY[t]*tmp3;
            trans2z[l]+=TsensorZ[t]*tmp3;
        }
        
        ++dsp;
    }
    
    std::filesystem::path fname1 = fdtd_mode.directory()/(fdtd_mode.prefix+"spectdata_norm");
    std::filesystem::path fname2 = fdtd_mode.directory()/(fdtd_mode.prefix+"spectdata_norm2");
    std::filesystem::path fname_power = fdtd_mode.directory()/(fdtd_mode.prefix+"power");
    
    std::ofstream file(fname1,std::ios::out|std::ios::trunc);
    std::ofstream file2(fname2,std::ios::out|std::ios::trunc);
//    std::ofstream file_p(fname_power,std::ios::out|std::ios::trunc);
    
    using std::abs;
    using std::sqrt;
    using std::norm;
    using std::arg;
    
    std::vector<Imdouble> base_TE_spectrum_ref(Nl,0);
    std::vector<Imdouble> base_TE_spectrum_trans(Nl,0);
    std::vector<Imdouble> base_TM_spectrum_ref(Nl,0);
    std::vector<Imdouble> base_TM_spectrum_trans(Nl,0);
    
    std::vector<double> lambda_v(Nl);
    
    for(l=0;l<Nl;l++)
    {
        lambda_v[l]=Lambda[l];
        
        file<<Lambda[l]<<" ";
//        file_p<<Lambda[l]<<" ";
        double a=sqrt(norm(ref2x[l])+norm(ref2y[l])+norm(ref2z[l]));
        double b=sqrt(norm(ref1x[l])+norm(ref1y[l])+norm(ref1z[l]));
        file<<a/b<<" ";
//        file_p<<a*a/b/b<<" ";
        a=sqrt(norm(trans2x[l])+norm(trans2y[l])+norm(trans2z[l]));
        b=sqrt(norm(trans1x[l])+norm(trans1y[l])+norm(trans1z[l]));
        file<<a/b<<std::endl;
//        file_p<<a*a/b/b*index_sub/index_sup<<std::endl;
        
        Imdouble ref0=ref1y[l];
        Imdouble trans0=trans1y[l];
        
        if(polar_mode=="TM")
        {
            ref0=ref1x[l];
            trans0=trans1x[l];
        }
        
        Imdouble refx=ref2x[l]/ref0;
        Imdouble refy=ref2y[l]/ref0;
//        Imdouble refz=ref2z[l]/ref0;
        
        double pow_rx=std::norm(refx);
        double pow_ry=std::norm(refy);
        
        Imdouble transx=trans2x[l]/trans0;
        Imdouble transy=trans2y[l]/trans0;
//        Imdouble transz=trans2z[l]/trans0;
        
        double pow_tx=std::norm(transx)*index_sub/index_sup;
        double pow_ty=std::norm(transy)*index_sub/index_sup;
        
        base_TE_spectrum_ref[l]=refy;
        base_TM_spectrum_ref[l]=refx;
        base_TE_spectrum_trans[l]=transy;
        base_TM_spectrum_trans[l]=transx;
        
        file2<<Lambda[l]<<" "
             <<abs(refx)<<" "
             <<arg(refx)<<" "
             <<pow_rx<<" "
             <<abs(refy)<<" "
             <<arg(refy)<<" "
             <<pow_ry<<" "
//             <<absrefz)<<" "
//             <<arg(refz)<<" "
             <<abs(transx)<<" "
             <<arg(transx)<<" "
             <<pow_tx<<" "
             <<abs(transy)<<" "
             <<arg(transy)<<" "
             <<pow_ty<<std::endl;
//             <<abs(transz)<<" "
//             <<arg(transz)<<std::endl;
    }
    
    file.close();
    file2.close();
    
    Spectrum TE_spectrum_ref(lambda_v,0,base_TE_spectrum_ref);
    Spectrum TM_spectrum_ref(lambda_v,0,base_TM_spectrum_ref);
    
    TE_spectrum_ref.set_polarizations(polar_mode,"TE");
    TE_spectrum_ref.set_type_ref();
    TM_spectrum_ref.set_polarizations(polar_mode,"TM");
    TM_spectrum_ref.set_type_ref();
    
    Spectrum TE_spectrum_trans(lambda_v,0,base_TE_spectrum_trans);
    Spectrum TM_spectrum_trans(lambda_v,0,base_TM_spectrum_trans);
    
    TE_spectrum_trans.set_polarizations(polar_mode,"TE");
    TE_spectrum_trans.set_type_trans();
    TM_spectrum_trans.set_polarizations(polar_mode,"TM");
    TM_spectrum_trans.set_type_trans();
    
    SpectrumCollec sp_collec;
    
    sp_collec.add_spectrum(TE_spectrum_ref);
    sp_collec.add_spectrum(TM_spectrum_ref);
    sp_collec.add_spectrum(TE_spectrum_trans);
    sp_collec.add_spectrum(TM_spectrum_trans);
    
    std::filesystem::path col_fname = fdtd_mode.directory()/(fdtd_mode.prefix+"norm_fullspec");
//    sp_collec.write(col_fname);
    
    norm_octave_script((fdtd_mode.directory()/fdtd_mode.prefix).generic_string(),index_sup,index_sub);
//    norm2_octave_script(fdtd.prefix,index_sup,index_sub);
    
    for(unsigned int i=0;i<sensors.size();i++) sensors[i]->treat();
    
    for(unsigned int i=0;i<sensors.size();i++) delete sensors[i];
}
