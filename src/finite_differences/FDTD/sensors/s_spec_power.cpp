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

#include <filehdl.h>
#include <sensors.h>
#include <thread_utils.h>

#include <fftw3.h>

#include <iostream>
#include <cstdint>


extern const Imdouble Im;
extern std::ofstream plog;

//###################################
// Planar Spectral Poynting sensor 
//###################################

Spect_Poynting::Spect_Poynting(int type_,
                               int x1_,int x2_,
                               int y1_,int y2_,
                               int z1_,int z2_)
    :SensorFieldHolder(type_,x1_,x2_,y1_,y2_,z1_,z2_,true)
{
}

Spect_Poynting::~Spect_Poynting()
{
}

void Spect_Poynting::treat()
{
    int i,j,l;
    
    using std::real;
    using std::conj;
    
    double Sx,Sy,Sz;
    
    std::string fname_out=name;
    fname_out.append("_pspdft");
    
    std::ofstream file;
    if(!silent) file.open(directory/fname_out,std::ios::out|std::ios::trunc);
    
    ProgDisp dsp(Nl,"Computing Poynting sensor results");
    
    for(l=0;l<Nl;l++)
    {
        Sx=Sy=Sz=0;
        
//        double w=2.0*Pi*c_light/lambda[l];
//        Imdouble coeff=std::exp(w*Dt/2.0*Im)/2.0;
//        
//        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
//        {
//            sp_Hx(i,j,l)*=coeff;
//            sp_Hy(i,j,l)*=coeff;
//            sp_Hz(i,j,l)*=coeff;
//        }}
        
        for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
        {
            Sx+=real(sp_Ey(i,j,l)*conj(sp_Hz(i,j,l))-sp_Ez(i,j,l)*conj(sp_Hy(i,j,l)));
            Sy+=real(sp_Ez(i,j,l)*conj(sp_Hx(i,j,l))-sp_Ex(i,j,l)*conj(sp_Hz(i,j,l)));
            Sz+=real(sp_Ex(i,j,l)*conj(sp_Hy(i,j,l))-sp_Ey(i,j,l)*conj(sp_Hx(i,j,l)));
        }}
        
        double area=span1*span2;
        
        if(type==NORMAL_X || type==NORMAL_XM)
        {
            Sx*=Dy*Dz;
            Sy*=Dy*Dz;
            Sz*=Dy*Dz;
            area*=Dy*Dz;
        }
        else if(type==NORMAL_Y || type==NORMAL_YM)
        {
            Sx*=Dx*Dz;
            Sy*=Dx*Dz;
            Sz*=Dx*Dz;
            area*=Dx*Dz;
        }
        else if(type==NORMAL_Z || type==NORMAL_ZM)
        {
            Sx*=Dx*Dy;
            Sy*=Dx*Dy;
            Sz*=Dx*Dy;
            area*=Dx*Dy;
        }
        
        if(type==NORMAL_X) rsp_result[l]=Sx;
        else if(type==NORMAL_Y) rsp_result[l]=Sy;
        else if(type==NORMAL_Z) rsp_result[l]=Sz;
        else if(type==NORMAL_XM) rsp_result[l]=-Sx;
        else if(type==NORMAL_YM) rsp_result[l]=-Sy;
        else if(type==NORMAL_ZM) rsp_result[l]=-Sz;
        
        if(!silent) file<<std::setprecision(15)<<lambda[l]<<" "<<rsp_result[l]<<" "<<rsp_result[l]/area<<std::endl;
        
        ++dsp;
    }
    
    if(!silent) file.close();
}

//##############################
// Box Spectral Poynting sensor 
//##############################

Box_Spect_Poynting::Box_Spect_Poynting(int x1,int x2,
                                       int y1,int y2,
                                       int z1,int z2)
    :xm(NORMAL_XM,x1,x1+1,y1,y2,z1,z2),
     xp(NORMAL_X ,x2-1,x2,y1,y2,z1,z2),
     ym(NORMAL_YM,x1,x2,y1,y1+1,z1,z2),
     yp(NORMAL_Y ,x1,x2,y2-1,y2,z1,z2),
     zm(NORMAL_ZM,x1,x2,y1,y2,z1,z1+1),
     zp(NORMAL_Z ,x1,x2,y1,y2,z2-1,z2)
{
}

void Box_Spect_Poynting::deep_feed(FDTD const &fdtd)
{
    if(!disable_xm) xm.feed(fdtd);
    if(!disable_xp) xp.feed(fdtd);
    if(!disable_ym) ym.feed(fdtd);
    if(!disable_yp) yp.feed(fdtd);
    if(!disable_zm) zm.feed(fdtd);
    if(!disable_zp) zp.feed(fdtd);
}

void Box_Spect_Poynting::link(FDTD const &fdtd)
{
    
    if(!disable_xm) xm.set_spectrum(lambda);
    if(!disable_xp) xp.set_spectrum(lambda);
    if(!disable_ym) ym.set_spectrum(lambda);
    if(!disable_yp) yp.set_spectrum(lambda);
    if(!disable_zm) zm.set_spectrum(lambda);
    if(!disable_zp) zp.set_spectrum(lambda);
    
    chk_var(lambda.size());
    
    if(!disable_xm) xm.link(fdtd);
    if(!disable_xp) xp.link(fdtd);
    if(!disable_ym) ym.link(fdtd);
    if(!disable_yp) yp.link(fdtd);
    if(!disable_zm) zm.link(fdtd);
    if(!disable_zp) zp.link(fdtd);
    
    if(!disable_xm) xm.set_silent(true);
    if(!disable_xp) xp.set_silent(true);
    if(!disable_ym) ym.set_silent(true);
    if(!disable_yp) yp.set_silent(true);
    if(!disable_zm) zm.set_silent(true);
    if(!disable_zp) zp.set_silent(true);
    
    Sensor::link(fdtd);
}

void Box_Spect_Poynting::treat()
{
    int l;
    
    if(!disable_xm) xm.treat();
    if(!disable_xp) xp.treat();
    if(!disable_ym) ym.treat();
    if(!disable_yp) yp.treat();
    if(!disable_zm) zm.treat();
    if(!disable_zp) zp.treat();
    
    for(l=0;l<Nl;l++) rsp_result[l]=0;
    
    std::ofstream file,file_m;
    if(!silent)
    {
        file.open(directory/(name+"_bspdft"),std::ios::out|std::ios::trunc);
        file_m.open(directory/(name+".m"),std::ios::out|std::ios::trunc);
    }
    
    chk_var(Nl);
    chk_var("A");
            
    for(l=0;l<Nl;l++)
    {
        if(!disable_xm) rsp_result[l]+=xm.rsp_result[l];
        if(!disable_xp) rsp_result[l]+=xp.rsp_result[l];
        if(!disable_ym) rsp_result[l]+=ym.rsp_result[l];
        if(!disable_yp) rsp_result[l]+=yp.rsp_result[l];
        if(!disable_zm) rsp_result[l]+=zm.rsp_result[l];
        if(!disable_zp) rsp_result[l]+=zp.rsp_result[l];
        
        std::cout<<xm.rsp_result[l]<<std::endl;
        
        if(!silent)
        {
            file<<std::setprecision(15)<<lambda[l]<<" "<<rsp_result[l];
            if(!disable_xm) file<<std::setprecision(15)<<" "<<xm.rsp_result[l];
            if(!disable_xp) file<<std::setprecision(15)<<" "<<xp.rsp_result[l];
            if(!disable_ym) file<<std::setprecision(15)<<" "<<ym.rsp_result[l];
            if(!disable_yp) file<<std::setprecision(15)<<" "<<yp.rsp_result[l];
            if(!disable_zm) file<<std::setprecision(15)<<" "<<zm.rsp_result[l];
            if(!disable_zp) file<<std::setprecision(15)<<" "<<zp.rsp_result[l];
            file<<std::endl;
        }
    }
    
    if(!silent)
    {
        std::filesystem::path core(name);
        file_m<<"function [L,tot]="<<core.stem()<<"\n";
        file_m<<"   data=dlmread('"<<core.stem()<<"_bspdft');\n";
        file_m<<"   L=data(:,1);\n";
        file_m<<"   tot=data(:,2);\n";
        file_m<<"end";
    }
}

//##############################
//  Planar FFT Poynting sensor
//##############################

Spect_Poynting_FFT::Spect_Poynting_FFT(int type_,
                               int x1_,int x2_,
                               int y1_,int y2_,
                               int z1_,int z2_)
    :curr_acc(0)
{
    step=0;
    base_bit=6*sizeof(double);
    
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    set_type(type_);
}

Spect_Poynting_FFT::~Spect_Poynting_FFT()
{
    f_buf.open(fname.c_str(),std::ios::out|std::ios::trunc);
    f_buf<<"Old sensor file";
    f_buf.close();
}

void Spect_Poynting_FFT::deep_feed(FDTD const &fdtd)
{
    int i,j,k;
    
    if(type==NORMAL_X || type==NORMAL_XM)
    {
        for(j=y1;j<y2;j++){ for(k=z1;k<z2;k++)
        {
            acc_Ex(j-y1,k-z1,curr_acc)=fdtd.local_Ex(x1,j,k);
            acc_Ey(j-y1,k-z1,curr_acc)=fdtd.local_Ey(x1,j,k);
            acc_Ez(j-y1,k-z1,curr_acc)=fdtd.local_Ez(x1,j,k);
            
            acc_Hx(j-y1,k-z1,curr_acc)=fdtd.local_Hx(x1,j,k);
            acc_Hy(j-y1,k-z1,curr_acc)=fdtd.local_Hy(x1,j,k);
            acc_Hz(j-y1,k-z1,curr_acc)=fdtd.local_Hz(x1,j,k);
        }}
    }
    else if(type==NORMAL_Y || type==NORMAL_YM)
    {
        for(i=x1;i<x2;i++){ for(k=z1;k<z2;k++)
        {
            acc_Ex(i-x1,k-z1,curr_acc)=fdtd.local_Ex(i,y1,k);
            acc_Ey(i-x1,k-z1,curr_acc)=fdtd.local_Ey(i,y1,k);
            acc_Ez(i-x1,k-z1,curr_acc)=fdtd.local_Ez(i,y1,k);
            
            acc_Hx(i-x1,k-z1,curr_acc)=fdtd.local_Hx(i,y1,k);
            acc_Hy(i-x1,k-z1,curr_acc)=fdtd.local_Hy(i,y1,k);
            acc_Hz(i-x1,k-z1,curr_acc)=fdtd.local_Hz(i,y1,k);
        }}
    }
    else if(type==NORMAL_Z || type==NORMAL_ZM)
    {
        for(i=x1;i<x2;i++){ for(j=y1;j<y2;j++)
        {
            acc_Ex(i-x1,j-y1,curr_acc)=fdtd.local_Ex(i,j,z1);
            acc_Ey(i-x1,j-y1,curr_acc)=fdtd.local_Ey(i,j,z1);
            acc_Ez(i-x1,j-y1,curr_acc)=fdtd.local_Ez(i,j,z1);
            
            acc_Hx(i-x1,j-y1,curr_acc)=fdtd.local_Hx(i,j,z1);
            acc_Hy(i-x1,j-y1,curr_acc)=fdtd.local_Hy(i,j,z1);
            acc_Hz(i-x1,j-y1,curr_acc)=fdtd.local_Hz(i,j,z1);
        }}
    }
    
    curr_acc+=1;
    if(curr_acc==Naccu)
    {
        for(j=0;j<span2;j++) 
        {
            for(k=0;k<Naccu;k++)
            {
                for(i=0;i<span1;i++)
                {
                    f_buf.write(reinterpret_cast<char*>(&acc_Ex(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Ey(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Ez(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hx(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hy(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hz(i,j,k)),sizeof(double));
                }
            }
        }
        
        curr_acc=0;
    }
}

void Spect_Poynting_FFT::initialize()
{
    if(type==NORMAL_X || type==NORMAL_XM){ span1=y2-y1; span2=z2-z1; }
    if(type==NORMAL_Y || type==NORMAL_YM){ span1=x2-x1; span2=z2-z1; }
    if(type==NORMAL_Z || type==NORMAL_ZM){ span1=x2-x1; span2=y2-y1; }
    
    Naccu=static_cast<int>(500e6/(8.0*6.0*span1*span2));
    if(Naccu<1) Naccu=1;
    
    acc_Ex.init(span1,span2,Naccu,0);
    acc_Ey.init(span1,span2,Naccu,0);
    acc_Ez.init(span1,span2,Naccu,0);
    
    acc_Hx.init(span1,span2,Naccu,0);
    acc_Hy.init(span1,span2,Naccu,0);
    acc_Hz.init(span1,span2,Naccu,0);
    
    std::filesystem::path fname=PathManager::to_temporary_path("P_Poynt_sensor_" + std::to_string(sensor_ID));
    
    f_buf.open(fname,std::ios::out|std::ios::trunc|std::ios::binary);
}

void Spect_Poynting_FFT::treat(std::string fname_out)
{
    int i,j,k,t,l;
    
    k=0;
    
    int acc_pad=0;
    
    if(curr_acc!=0)
    {
        acc_pad=Naccu-curr_acc;
        
        for(j=0;j<span2;j++)
        {
            for(k=0;k<curr_acc;k++)
            {  
                for(i=0;i<span1;i++)
                {
                    f_buf.write(reinterpret_cast<char*>(&acc_Ex(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Ey(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Ez(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hx(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hy(i,j,k)),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&acc_Hz(i,j,k)),sizeof(double));
                }
            }
            
            // padding the file to align chunks properly
            
            for(k=0;k<acc_pad;k++)
            {
                double tmp=0;
                
                for(i=0;i<span1;i++)
                {
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                    f_buf.write(reinterpret_cast<char*>(&tmp),sizeof(double));
                }
            }
        }
    }
    
    f_buf.close();
    
    Nt=step;
    
    //#########################
    //   FFTW initialization
    //#########################
    
    int Nfftp=static_cast<int>(std::log(Nt+0.0)/std::log(2.0)+3);
    int Nfft=1; for(i=0;i<Nfftp;i++) Nfft*=2;
    
    fftw_complex *signal;
    fftw_plan p;
    
    signal=(fftw_complex*)fftw_malloc(Nfft*sizeof(fftw_complex));
    p=fftw_plan_dft_1d(Nfft,signal,signal,FFTW_BACKWARD,FFTW_MEASURE);
    
    Grid1<Imdouble> fft_result(Nfft,0);
    Grid1<double> fft_freq(Nfft,0);
    //!!!!!!!!!!!!!!!!!!!!
    
    Nl=Nfft;
    lambda.resize(Nl);
    for(int i=0;i<Nl;i++) lambda[i]=0;
    
    sp_result.init(Nl,0);
    
    for(l=0;l<Nl;l++)
    {
        double f=l/(Nfft*Dt);
        fft_freq[l]=f;
        
        if(l==0) f=0.5/(Nfft*Dt);
        lambda[l]=c_light/f;
    }
    
    Grid2<double> tr_Ex(span1,Nt,0),tr_Ey(span1,Nt,0),tr_Ez(span1,Nt,0);
    Grid2<double> tr_Hx(span1,Nt,0),tr_Hy(span1,Nt,0),tr_Hz(span1,Nt,0);
        
    Grid1<Imdouble> sp_Ex(Nl,0),sp_Ey(Nl,0),sp_Ez(Nl,0);
    Grid1<Imdouble> sp_Hx(Nl,0),sp_Hy(Nl,0),sp_Hz(Nl,0);
    
    Grid1<Imdouble> sp_S(Nl,0),sp_S_TE(Nl,0),sp_S_TM(Nl,0);
    Grid1<Imdouble> sp_r_TE(Nl,0), sp_r_TM(Nl,0);
    
    std::ifstream f_in(fname.c_str(),std::ios::in|std::ios::binary);
    
    using std::real;
    using std::conj;
    
    Imdouble tmp_coeff;
    Imdouble tmp_arg;
    
    sp_result=Imdouble(0);
    
    //###############
    //   Treatment
    //###############
    
    std::cout<<"Computing Poynting sensor results"<<std::endl;
    ProgTimeDisp dsp(span1*span2);
    
    
    
    for(j=0;j<span2;j++) 
    {
        int Nchunk=0;
        
        for(t=0;t<Nt;t++)
        {
            if(t%Naccu==0)
            {
                std::cout<<f_in.tellg()<<std::endl;
                
                //Necessary step to avoid overflows
                
                std::ofstream::pos_type p;
                
                std::ofstream::pos_type pt_j=j;
                std::ofstream::pos_type pt_span1=span1;
                std::ofstream::pos_type pt_span2=span2;
                std::ofstream::pos_type pt_Naccu=Naccu;
                std::ofstream::pos_type pt_Nchunk=Nchunk;
                std::ofstream::pos_type pt_base_bit=base_bit;
                
                p=(pt_j*pt_Naccu*pt_span1
                   +pt_Nchunk*pt_Naccu*pt_span1*pt_span2)*pt_base_bit;
                
                f_in.seekg(p);
                
                std::cout<<f_in.tellg()<<" "<<f_in.good()<<" "<<sizeof(long)<<std::endl;
                Nchunk+=1;
            }
            
            for(i=0;i<span1;i++)
            {
                f_in.read(reinterpret_cast<char*>(&tr_Ex(i,t)),sizeof(double));
                f_in.read(reinterpret_cast<char*>(&tr_Ey(i,t)),sizeof(double));
                f_in.read(reinterpret_cast<char*>(&tr_Ez(i,t)),sizeof(double));
                f_in.read(reinterpret_cast<char*>(&tr_Hx(i,t)),sizeof(double));
                f_in.read(reinterpret_cast<char*>(&tr_Hy(i,t)),sizeof(double));
                f_in.read(reinterpret_cast<char*>(&tr_Hz(i,t)),sizeof(double));
            }
            
            if(f_in.bad()) std::cout<<"bleh"<<std::endl;
            if(f_in.eof()) std::cout<<"blah"<<std::endl;
        }
        
        for(i=0;i<span1;i++)
        {
            sp_Ex=Imdouble(0); sp_Ey=Imdouble(0); sp_Ez=Imdouble(0);
            sp_Hx=Imdouble(0); sp_Hy=Imdouble(0); sp_Hz=Imdouble(0);
            
            for(k=0;k<6;k++)
            {
                Grid2<double> *pTr; Grid1<Imdouble> *pSp;
                
                if(k==0)      { pTr=&tr_Ex; pSp=&sp_Ex; }
                else if(k==1) { pTr=&tr_Ey; pSp=&sp_Ey; }
                else if(k==2) { pTr=&tr_Ez; pSp=&sp_Ez; }
                else if(k==3) { pTr=&tr_Hx; pSp=&sp_Hx; }
                else if(k==4) { pTr=&tr_Hy; pSp=&sp_Hy; }
                else if(k==5) { pTr=&tr_Hz; pSp=&sp_Hz; }
                
                for(t=0;t<Nfft;t++) { signal[t][0]=signal[t][1]=0; }
                for(t=0;t<Nt;t++) signal[t][0]=(*pTr)(i,t);
                
                fftw_execute(p);
                
                for(t=0;t<Nfft;t++)
                {
                    fft_result[t]=signal[t][0]+signal[t][1]*Im;
                    
                    if(k==3 || k==4 || k==5)
                    {
                        fft_result[l]*=std::exp(-Pi*Dt*fft_freq[l]*Im);
                    }
                }
                
                for(l=0;l<Nl;l++) (*pSp)[l]=fft_result[l];
            }
            
            if(type==NORMAL_X)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=real(sp_Ey[l]*conj(sp_Hz[l])-sp_Ez[l]*conj(sp_Hy[l]));
            }
            if(type==NORMAL_XM)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=-real(sp_Ey[l]*conj(sp_Hz[l])-sp_Ez[l]*conj(sp_Hy[l]));
            }
            else if(type==NORMAL_Y)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=real(sp_Ez[l]*conj(sp_Hx[l])-sp_Ex[l]*conj(sp_Hz[l]));
            }
            else if(type==NORMAL_YM)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=-real(sp_Ez[l]*conj(sp_Hx[l])-sp_Ex[l]*conj(sp_Hz[l]));
            }
            else if(type==NORMAL_Z)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=real(sp_Ex[l]*conj(sp_Hy[l])-sp_Ey[l]*conj(sp_Hx[l]));
            }
            else if(type==NORMAL_ZM)
            {
                for(l=0;l<Nl;l++)
                    sp_S[l]=-real(sp_Ex[l]*conj(sp_Hy[l])-sp_Ey[l]*conj(sp_Hx[l]));
            }
            
            for(l=0;l<Nl;l++) sp_result[l]+=sp_S[l];
            
            ++dsp;
        }
    }
    
    //####################
    //   FFTW Cleanup
    //####################
    
    fftw_destroy_plan(p);
    fftw_free(signal);
    
    //####################
    //  Result writing
    //####################
        
    if(type==NORMAL_X || type==NORMAL_XM)      for(l=0;l<Nl;l++) sp_result[l]*=Dy*Dz;
    else if(type==NORMAL_Y || type==NORMAL_YM) for(l=0;l<Nl;l++) sp_result[l]*=Dx*Dz;
    else if(type==NORMAL_Z || type==NORMAL_ZM) for(l=0;l<Nl;l++) sp_result[l]*=Dx*Dy;
    
    std::ofstream file(fname_out.c_str(),std::ios::out|std::ios::trunc);
    
    for(l=0;l<Nl;l++) file<<lambda[l]<<" "<<std::real(sp_result[l])<<std::endl;
}
