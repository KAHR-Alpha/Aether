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

#include <fdtd_core.h>
#include <bitmap3.h>


extern const Imdouble Im;

extern std::ofstream plog;

void octave_obl_script(std::string prefix,std::string dfname)
{
    std::stringstream strm;
    strm<<prefix<<"show_obl.m";
    std::ofstream file(strm.str().c_str(),std::ios::out|std::ios::trunc);
    
    using std::endl;
    
    file<<"clear *"<<endl;
    file<<"close all"<<endl;
    file<<endl;
    file<<"a=load(\""<<dfname<<"\");"<<endl;
    file<<"L=a(:,1);"<<endl;
    file<<"r=a(:,2);"<<endl;
    file<<"t=a(:,3);"<<endl;
    file<<endl;
    file<<"plot(L,r,';r;',L,t,';t;')"<<endl;
    file<<"axis([min(L) max(L) 0 1])"<<endl;
    
    file.close();
}

void bufread(Grid3<double> &G,int Nmem,std::string fs)
{
    int j,k,t;
    int Ny=G.L1();
    int Nz=G.L2();
    
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

void bufwrite(Grid3<double> &G,int Nmem,std::string fs)
{
    int j,k,t;
    int Ny=G.L1();
    int Nz=G.L2();
    
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

/*void mode_oblique_rec(ScriptHandler &script)
{
    int i,j,k,l,t;
    int it;
    
    std::cout<<"blah"<<std::endl;
    
    double inc_ang_deg=0;
    double inc_ang_rad=0;
    
    std::cout<<inc_ang_rad<<std::endl;
    
    int Nx=60,Ny=60,Nz=60;
    int Nt=40000;
    
    std::string polar_mode="TE";
    
    double Dx=10e-9,Dy=10e-9,Dz=10e-9;
    
    double lambda_min,lambda_max;
    
    lambda_min=1000e-9;
    lambda_max=5000e-9;
    
    int Nl=1000;
    
    std::string prefix_str="";
    
    int N_it=15;
    
    Grid3<unsigned int> mattmp(Nx,Ny,Nz,0);
    
    int pmlx=12;
    int pmly=0;
    int pmlz=12;
    
    if(script.has("iterations")) script.get_options(N_it);
    if(script.has("set_dx")) script.get_options(Dx);
    if(script.has("set_dy")) script.get_options(Dy);
    if(script.has("set_dz")) script.get_options(Dz);
    //if(script.has("set_geometry")) setstruct_fscript(Nx,Ny,Nz,Dx,Dy,Dz,mattmp,script.get_options());
    if(script.has("set_polarization")) polar_mode=script.get_options();
    if(script.has("set_prefix")) prefix_str=script.get_options();
    if(script.has("set_pml"))
    {
        std::stringstream cmd_strm(script.get_options());
        cmd_strm>>pmlx;
        cmd_strm>>pmly;
        cmd_strm>>pmlz;
    }
    if(script.has("set_spectrum")) script_set_spectrum(script.get_options(),Nl,lambda_min,lambda_max);
    if(script.has("set_N_tsteps")) script.get_options(Nt);
    
    if(script.has("geo_subdivide")) script_geo_subdivide(script.get_options(),mattmp,Nx,Ny,Nz,Nt,Dx,Dy,Dz);
    
    /////////////////////////
    /////////////////////////
    /////////////////////////
    
    double Dt=0.999*std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light);
    
    //###############
    //    Angle
    //###############
    
    if(script.has("angle")) script.get_options(inc_ang_deg);
    
    std::cout<<"Oblique incidence"<<std::endl;
    std::cout<<"Requested angle: "<<inc_ang_deg<<std::endl;
    std::cout<<Nx*Dx<<std::endl;
    
    i=0;
    
    double tmp1,tmp2;
    bool c=1;
    
    int Nshift=0;
    
    while(c)
    {
        double a=i*Dt*c_light/(Nx*Dx);
        double b=(i+1)*Dt*c_light/(Nx*Dx);
        
        tmp1=180.0/Pi*std::asin(a);
        tmp2=180.0/Pi*std::asin(b);
        
        if(tmp1<=inc_ang_deg && tmp2>inc_ang_deg)
        {
            c=0;
            inc_ang_deg=tmp1;
            Nshift=i;
        }
        
        if(b>=1.0)
        {
            c=0;
            inc_ang_deg=tmp1;
            Nshift=i;
        }
        
        i++;
    }
    
    std::cout<<"Used angle: "<<inc_ang_deg<<std::endl;
    std::cout<<"Time shift: "<<Nshift<<" steps"<<std::endl;
    
    inc_ang_rad=Pi/180.0*inc_ang_deg;
    
    int Nmem=500;
    if(3*Nshift<Nmem) Nmem=3*Nshift;
    
    //###############
    //   FDTD init
    //###############
    
    grid_extend(mattmp,Nx,Ny,Nz,10,10,0,0,0,0);
    
    for(i=0;i<5;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        mattmp(Nx-10+i,j,k)=mattmp(10+i,j,k);
    }
        
    
    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",
              pmlx,pmlx,
              pmly,pmly,
              pmlz,pmlz);
    
    fdtd.set_prefix(prefix_str);
    fdtd.set_matsgrid(mattmp);
    
    fdtd.bootstrap();
    
    while(script.has("set_material"))
    {
        std::stringstream cmd_strm(script.get_options());
        std::string matname;
        int matnumber;
        
        cmd_strm>>matnumber;
        cmd_strm>>matname;
        
        fdtd.set_material(matnumber,matname);
    }
    
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    
    int xs_s=fdtd.xs_s+10;
    int xs_e=xs_s+fdtd.Nx_s-20;
    
    std::cout<<xs_s<<" "<<xs_e<<" "<<xs_e-xs_s<<std::endl;
    
    //################
    // Mem alloc
    //###############
    
    Grid3<double> F_bufEy(Ny,Nz,Nmem,0);
    Grid3<double> F_bufEz(Ny,Nz,Nmem,0);
    Grid3<double> F_bufHy(Ny,Nz,Nmem,0);
    Grid3<double> F_bufHz(Ny,Nz,Nmem,0);
    
    Grid3<double> P_bufEy(Ny,Nz,Nmem,0);
    Grid3<double> P_bufEz(Ny,Nz,Nmem,0);
    Grid3<double> P_bufHy(Ny,Nz,Nmem,0);
    Grid3<double> P_bufHz(Ny,Nz,Nmem,0);
    
    Grid3<double> N_bufEy_l(Ny,Nz,Nmem,0);
    Grid3<double> N_bufEz_l(Ny,Nz,Nmem,0);
    Grid3<double> N_bufHy_l(Ny,Nz,Nmem,0);
    Grid3<double> N_bufHz_l(Ny,Nz,Nmem,0);
    
    Grid3<double> N_bufEy_r(Ny,Nz,Nmem,0);
    Grid3<double> N_bufEz_r(Ny,Nz,Nmem,0);
    Grid3<double> N_bufHy_r(Ny,Nz,Nmem,0);
    Grid3<double> N_bufHz_r(Ny,Nz,Nmem,0);
    
    //###############
    // Buf files rdy
    //###############
    
    for(t=0;t<1.5*Nt;t++)
    {
        if((t+1)/Nmem!=t/Nmem)
        {
            std::stringstream fse2,fse3,fsh2,fsh3;
            fse2<<"buf/bufey"<<t/Nmem;
            fse3<<"buf/bufez"<<t/Nmem;
            fsh2<<"buf/bufhy"<<t/Nmem;
            fsh3<<"buf/bufhz"<<t/Nmem;
            
            std::ofstream file;
            
            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
        }
        if((t+1)/Nmem!=t/Nmem)
        {
            std::stringstream fse2,fse3,fsh2,fsh3;
            fse2<<"buf/bufeyl"<<t/Nmem;
            fse3<<"buf/bufezl"<<t/Nmem;
            fsh2<<"buf/bufhyl"<<t/Nmem;
            fsh3<<"buf/bufhzl"<<t/Nmem;
            
            std::ofstream file;
            
            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
            file.close();
        }
    }
    
    //###############
    //
    
    
    std::stringstream tfile_fname;
    tfile_fname<<"outfield_rec_"<<prefix_str;
    std::ofstream tfile(tfile_fname.str().c_str(),std::ios::out|std::ios::trunc);
    
    Grid2<Imdouble> BRsensorX(fdtd.Nx_s,Nl,0),BRsensorY(fdtd.Nx_s,Nl,0),BRsensorZ(fdtd.Nx_s,Nl,0);
    Grid2<Imdouble> RsensorX(fdtd.Nx_s,Nl,0),RsensorY(fdtd.Nx_s,Nl,0),RsensorZ(fdtd.Nx_s,Nl,0);
    
    Grid2<Imdouble> BTsensorX(fdtd.Nx_s,Nl,0),BTsensorY(fdtd.Nx_s,Nl,0),BTsensorZ(fdtd.Nx_s,Nl,0);
    Grid2<Imdouble> TsensorX(fdtd.Nx_s,Nl,0),TsensorY(fdtd.Nx_s,Nl,0),TsensorZ(fdtd.Nx_s,Nl,0);
    
    Grid1<double> Lambda(Nl,0);
    Grid1<double> w(Nl,0);
    
    for(l=0;l<Nl;l++) Lambda(l)=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
    for(l=0;l<Nl;l++) w(l)=2.0*Pi*c_light/Lambda(l);
    
    for(it=0;it<N_it;it++)
    {
        fdtd.tstep=0;
        //int iteration=it;
        
        fdtd.reset_fields();
        
        F_bufEy=0;
        F_bufEz=0;
        F_bufHy=0;
        F_bufHz=0;
        
        P_bufEy=0;
        P_bufEz=0;
        P_bufHy=0;
        P_bufHz=0;
        
        N_bufEy_l=0;
        N_bufEz_l=0;
        N_bufHy_l=0;
        N_bufHz_l=0;
        
        N_bufEy_r=0;
        N_bufEz_r=0;
        N_bufHy_r=0;
        N_bufHz_r=0;
        
        if(it>0)
        {
            std::stringstream fse2,fse3,fsh2,fsh3;
            fse2<<"buf/bufey"<<Nshift/Nmem;
            fse3<<"buf/bufez"<<Nshift/Nmem;
            fsh2<<"buf/bufhy"<<Nshift/Nmem;
            fsh3<<"buf/bufhz"<<Nshift/Nmem;
            
            bufread(F_bufEy,Nmem,fse2.str());
            bufread(F_bufEz,Nmem,fse3.str());
            bufread(F_bufHy,Nmem,fsh2.str());
            bufread(F_bufHz,Nmem,fsh3.str());
        }
        if(it>0)
        {
            std::stringstream fse2,fse3,fsh2,fsh3;
            fse2<<"buf/bufeyl"<<Nshift/Nmem;
            fse3<<"buf/bufezl"<<Nshift/Nmem;
            fsh2<<"buf/bufhyl"<<Nshift/Nmem;
            fsh3<<"buf/bufhzl"<<Nshift/Nmem;
            
            bufread(P_bufEy,Nmem,fse2.str());
            bufread(P_bufEz,Nmem,fse3.str());
            bufread(P_bufHy,Nmem,fsh2.str());
            bufread(P_bufHz,Nmem,fsh3.str());
        }
        
        int M,pk=0;
        
        double C1,C2x,C2y,C2z;
        
        double pol=0;
        if(polar_mode=="TM") pol=90;
        
        ChpIn inj_chp;
        inj_chp.init_pulse(lambda_min,lambda_max,inc_ang_deg,pol,0,0,0,(fdtd.zs_e-1)*Dz,1.0);
        
        ProgTimeDisp dsp(Nt);
        
        bool no_mod=true;
        
        for(t=0;t<Nt;t++)
        {
            //plog<<"tap1"<<std::endl;
            
            double tsum=0;
            
            for(k=fdtd.zs_s;k<fdtd.zs_e;k++)
            {
                tsum+=std::abs(fdtd.Ey(fdtd.xs_e,0,k));
            }
            
            tsum/=fdtd.Nz_s+0.0;
            tfile<<t<<" "<<std::log10(1e-4+tsum)<<std::endl;
            
            int np=t-Nshift-1;
            int nf=t+Nshift-1;
            
            fdtd.update_E();
            
            double tb=t*Dt;
            double z=(fdtd.zs_e-1+0.5)*Dz;
            
            for(i=xs_s;i<xs_e+3;i++)
            {
                double inj_Hx=inj_chp.Hx(i*Dx,0,z,tb);
                double inj_Hy=inj_chp.Hy((i+0.5)*Dx,0,z,tb);
                
                for(j=0;j<Ny;j++)
                {
                    fdtd.Ex(i,j,fdtd.zs_e-1)-=fdtd.dtdez*inj_Hy;
                    fdtd.Ey(i,j,fdtd.zs_e-1)+=fdtd.dtdez*inj_Hx;
                }
            }
            
            double amp_mod=1.0;
            
            if(no_mod==false)
            {
                if(t>=Nt-2000 && t<Nt-1000)
                {
                    double tmod=(t-Nt+2000)/1000.0;
                    amp_mod=1.0-tmod*tmod*(3.0-2.0*tmod);
                }
                if(t>=Nt-1000) amp_mod=0;
            }
            
            //plog<<"tap2"<<std::endl;
            
            if(np>=0)
            {
                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
                {
                    i=xs_e+3;
                    
                    M=fdtd.matsgrid(i,j,k);
                    
                    fdtd.mats(M).coeffsY(C1,C2x,C2z);
                    #ifdef FASTOBL
                    Ey(i,j,k)-=C2x*N_bufHz_l(j,k,(np)%Nmem);
                    #endif
                    #ifndef FASTOBL
                    fdtd.Ey(i,j,k)-=C2x*P_bufHz(j,k,(np)%Nmem)*amp_mod;
                    #endif
                    
                    fdtd.mats(M).coeffsZ(C1,C2x,C2y);
                    #ifdef FASTOBL
                    Ez(i,j,k)+=C2x*N_bufHy_l(j,k,(np)%Nmem);
                    #endif
                    #ifndef FASTOBL
                    fdtd.Ez(i,j,k)+=C2x*P_bufHy(j,k,(np)%Nmem)*amp_mod;
                    #endif
                    
                }}
            }
            
            //plog<<"tap3"<<std::endl;
            
            if(nf>=0)
            {
                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
                {
                    M=fdtd.matsgrid(xs_s,j,k);
                    
                    fdtd.mats(M).coeffsY(C1,C2x,C2z);
                    fdtd.Ey(xs_s,j,k)+=C2x*F_bufHz(j,k,(nf)%Nmem)*amp_mod;
                    
                    fdtd.mats(M).coeffsZ(C1,C2x,C2y);
                    fdtd.Ez(xs_s,j,k)-=C2x*F_bufHy(j,k,(nf)%Nmem)*amp_mod;
                }}
            }
            
            fdtd.update_H();
            
            tb=(t+0.5)*Dt;
            z=(fdtd.zs_e-1)*Dz;
            
            for(i=xs_s;i<xs_e+3;i++)
            {                
                double inj_Ex=inj_chp.Ex((i+0.5)*Dx,0,z,tb);
                double inj_Ey=inj_chp.Ey(i*Dx,0,z,tb);
                
                for(j=0;j<Ny;j++)
                {
                    fdtd.Hx(i,j,fdtd.zs_e-1)+=fdtd.dtdmz*inj_Ey;
                    fdtd.Hy(i,j,fdtd.zs_e-1)-=fdtd.dtdmz*inj_Ex;
                }
            }
            
            //plog<<"tap4"<<std::endl;
            
            //Cache loading - Past
            
            #ifndef FASTOBL
            if(np+1<Nt && (np+1)/Nmem!=np/Nmem && it>0)
            {
                std::stringstream fse2,fse3,fsh2,fsh3;
                fse2<<"buf/bufeyl"<<(np+1)/Nmem;
                fse3<<"buf/bufezl"<<(np+1)/Nmem;
                fsh2<<"buf/bufhyl"<<(np+1)/Nmem;
                fsh3<<"buf/bufhzl"<<(np+1)/Nmem;
                
                bufread(P_bufEy,Nmem,fse2.str());
                bufread(P_bufEz,Nmem,fse3.str());
                bufread(P_bufHy,Nmem,fsh2.str());
                bufread(P_bufHz,Nmem,fsh3.str());
                
                std::cout<<"TAC "<<(np+1)/Nmem<<std::endl;
            }
            #endif
                        
            if(np>=0)
            {
                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
                {
                    i=xs_e+3;
                    
                    #ifdef FASTOBL
                    Hy(i-1,j,k)+=dtdmx*N_bufEz_l(j,k,(np+1)%Nmem);  //np+1
                    Hz(i-1,j,k)-=dtdmx*N_bufEy_l(j,k,(np+1)%Nmem);  //np+1
                    #endif
                    #ifndef FASTOBL
                    fdtd.Hy(i-1,j,k)+=fdtd.dtdmx*P_bufEz(j,k,(np+1)%Nmem);  //np+1
                    fdtd.Hz(i-1,j,k)-=fdtd.dtdmx*P_bufEy(j,k,(np+1)%Nmem);  //np+1
                    #endif
                }}
            }
            
            //plog<<"tap5"<<std::endl;
            
            // Cache loading - Future
            
            if((nf+1)/Nmem!=nf/Nmem && nf+1<Nt)
            {
                std::stringstream fse2,fse3,fsh2,fsh3;
                fse2<<"buf/bufey"<<(nf+1)/Nmem;
                fse3<<"buf/bufez"<<(nf+1)/Nmem;
                fsh2<<"buf/bufhy"<<(nf+1)/Nmem;
                fsh3<<"buf/bufhz"<<(nf+1)/Nmem;
                
                bufread(F_bufEy,Nmem,fse2.str());
                bufread(F_bufEz,Nmem,fse3.str());
                bufread(F_bufHy,Nmem,fsh2.str());
                bufread(F_bufHz,Nmem,fsh3.str());
                
                std::cout<<"TOC "<<(nf+1)/Nmem<<std::endl;
            }
            
            if((nf+1)/Nmem!=nf/Nmem && nf+1>=Nt)
            {
                F_bufEy=0; F_bufEz=0; F_bufHy=0; F_bufHz=0;
            }
            
            // H update - Future
            
            if(nf>=0)
            {
                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
                {
                    fdtd.Hy(xs_s-1,j,k)-=fdtd.dtdmx*F_bufEz(j,k,(nf+1)%Nmem)*amp_mod;  //nf+1
                    fdtd.Hz(xs_s-1,j,k)+=fdtd.dtdmx*F_bufEy(j,k,(nf+1)%Nmem)*amp_mod;  //nf+1
                }}
            }
            
            //plog<<"tap6"<<std::endl;
            
            // Temporary mem storage
            
            for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
            {
                using std::abs;
                i=xs_s+3;
                
                N_bufEy_l(j,k,t%Nmem)=fdtd.Ey(i,j,k);
                N_bufEz_l(j,k,t%Nmem)=fdtd.Ez(i,j,k);
                N_bufHy_l(j,k,t%Nmem)=fdtd.Hy(i-1,j,k);
                N_bufHz_l(j,k,t%Nmem)=fdtd.Hz(i-1,j,k);
                
                i=xs_e;
                
                N_bufEy_r(j,k,t%Nmem)=fdtd.Ey(i,j,k);
                N_bufEz_r(j,k,t%Nmem)=fdtd.Ez(i,j,k);
                N_bufHy_r(j,k,t%Nmem)=fdtd.Hy(i-1,j,k);
                N_bufHz_r(j,k,t%Nmem)=fdtd.Hz(i-1,j,k);
            }}
            
            //Cache writing - Future
            
            if((t+1)/Nmem!=t/Nmem)
            {
                std::stringstream fse2,fse3,fsh2,fsh3;
                fse2<<"buf/bufey"<<t/Nmem;
                fse3<<"buf/bufez"<<t/Nmem;
                fsh2<<"buf/bufhy"<<t/Nmem;
                fsh3<<"buf/bufhz"<<t/Nmem;
                
                bufwrite(N_bufEy_r,Nmem,fse2.str());
                bufwrite(N_bufEz_r,Nmem,fse3.str());
                bufwrite(N_bufHy_r,Nmem,fsh2.str());
                bufwrite(N_bufHz_r,Nmem,fsh3.str());
                
                std::cout<<"TIC R"<<t/Nmem<<std::endl;
            }
            
            //Cache writing - Past
            
            #ifndef FASTOBL
            if((t+1)/Nmem!=t/Nmem)
            {
                std::stringstream fse2,fse3,fsh2,fsh3;
                fse2<<"buf/bufeyl"<<t/Nmem;
                fse3<<"buf/bufezl"<<t/Nmem;
                fsh2<<"buf/bufhyl"<<t/Nmem;
                fsh3<<"buf/bufhzl"<<t/Nmem;
                
                bufwrite(N_bufEy_l,Nmem,fse2.str());
                bufwrite(N_bufEz_l,Nmem,fse3.str());
                bufwrite(N_bufHy_l,Nmem,fsh2.str());
                bufwrite(N_bufHz_l,Nmem,fsh3.str());
                
                std::cout<<"TIC L"<<t/Nmem<<std::endl;
            }
            #endif
            
            if(t/static_cast<double>(Nt)<=pk/100.0 && (t+1.0)/Nt>pk/100.0)
            {
                fdtd.draw(t,0,Nx/2,Ny/2,Nz/2+4);
                
                pk++;
            }
            
            //plog<<"tap7"<<std::endl;
            
            
            if(it==N_it-1)
            {
                Imdouble tmp1;
                
                double tb=(t+0.5)*Dt; //calibration
                
                double Dx2=fdtd.Dx/2.0;
                double Dz2=fdtd.Dz/2.0;
                
                for(i=xs_s;i<xs_e;i++)
                {
                    double x=i*Dx;
                    
                    double BRSX=0,BRSY=0,BRSZ=0;
                    double RSX=0,RSY=0,RSZ=0;
                    double BTSX=0,BTSY=0,BTSZ=0;
                    double TSX=0,TSY=0,TSZ=0;
                                        
                    double z=(fdtd.zs_e-2)*Dz;
                    
                    double chxr=inj_chp.Ex(x+Dx2,0,z,tb);
                    double chyr=inj_chp.Ey(x,0,z,tb);
                    double chzr=inj_chp.Ez(x,0,z+Dz2,tb);
                                        
                    z=(fdtd.zs_s+2)*Dz;
                    
                    double chxt=inj_chp.Ex(x+Dx2,0,z,tb);
                    double chyt=inj_chp.Ey(x,0,z,tb);
                    double chzt=inj_chp.Ez(x,0,z+Dz2,tb);
                                        
                    for(j=0;j<Ny;j++)
                    {
                        BRSX+=chxr;
                        BRSY+=chyr;
                        BRSZ+=chzr;

                        RSX+=(fdtd.Ex(i,j,fdtd.zs_e-2)-chxr);
                        RSY+=(fdtd.Ey(i,j,fdtd.zs_e-2)-chyr);
                        RSZ+=(fdtd.Ez(i,j,fdtd.zs_e-2)-chzr);
                        
                        BTSX+=chxt;
                        BTSY+=chyt;
                        BTSZ+=chzt;
                        
                        TSX+=fdtd.Ex(i,j,fdtd.zs_s+2);
                        TSY+=fdtd.Ey(i,j,fdtd.zs_s+2);
                        TSZ+=fdtd.Ez(i,j,fdtd.zs_s+2);
                    }
                    
                    int i2=i-xs_s;
                    
                    for(l=0;l<Nl;l++)
                    {
                        tmp1=std::exp(tb*w(l)*Im)*Dt/(Ny+0.0);
                        
                        BRsensorX(i2,l)+=BRSX*tmp1;
                        BRsensorY(i2,l)+=BRSY*tmp1;
                        BRsensorZ(i2,l)+=BRSZ*tmp1;
                        
                        RsensorX(i2,l)+=RSX*tmp1;
                        RsensorY(i2,l)+=RSY*tmp1;
                        RsensorZ(i2,l)+=RSZ*tmp1;
                                                    
                        BTsensorX(i2,l)+=BTSX*tmp1;
                        BTsensorY(i2,l)+=BTSY*tmp1;
                        BTsensorZ(i2,l)+=BTSZ*tmp1;
                        
                        TsensorX(i2,l)+=TSX*tmp1;
                        TsensorY(i2,l)+=TSY*tmp1;
                        TsensorZ(i2,l)+=TSZ*tmp1;
                    }
                    
                }
            }
            
            
            ++dsp;
        }
    }
    
    Grid1<Imdouble> ref1x(Nl,0),ref1y(Nl,0),ref1z(Nl,0);
    Grid1<Imdouble> ref2x(Nl,0),ref2y(Nl,0),ref2z(Nl,0);
    Grid1<Imdouble> trans1x(Nl,0),trans1y(Nl,0),trans1z(Nl,0);
    Grid1<Imdouble> trans2x(Nl,0),trans2y(Nl,0),trans2z(Nl,0);
    
    Grid1<Imdouble> ref1TE(Nl,0),ref2TE(Nl,0);
    Grid1<Imdouble> ref1TM(Nl,0),ref2TM(Nl,0);
    Grid1<Imdouble> trans1TE(Nl,0),trans2TE(Nl,0);
    Grid1<Imdouble> trans1TM(Nl,0),trans2TM(Nl,0);
    
    Grid1<Imdouble> refa(Nl,0),refb(Nl,0),refc(Nl,0);
    
    ProgDisp dsp(Nl,"Fourier Transform");
        
    for(l=0;l<Nl;l++)
    {
        double kl=2.0*Pi/Lambda(l)*std::sin(inc_ang_rad);
        
        Imdouble tmp1;
        
        for(i=0;i<xs_e-xs_s;i++)
        {
            tmp1=std::exp(-i*Dx*kl*Im);
            
            ref1x(l)+=BRsensorX(i,l)*tmp1;
            ref1y(l)+=BRsensorY(i,l)*tmp1;
            ref1z(l)+=BRsensorZ(i,l)*tmp1;
            
            trans1x(l)+=BTsensorX(i,l)*tmp1;
            trans1y(l)+=BTsensorY(i,l)*tmp1;
            trans1z(l)+=BTsensorZ(i,l)*tmp1;
            
            ref2x(l)+=RsensorX(i,l)*tmp1;
            ref2y(l)+=RsensorY(i,l)*tmp1;
            ref2z(l)+=RsensorZ(i,l)*tmp1;
            
            trans2x(l)+=TsensorX(i,l)*tmp1;
            trans2y(l)+=TsensorY(i,l)*tmp1;
            trans2z(l)+=TsensorZ(i,l)*tmp1;
        }
        
        for(i=0;i<60;i++)
        {
            tmp1=std::exp(-i*Dx*kl*Im);
            
            refa(l)+=RsensorX(i,l)*tmp1;
        }
        
        ++dsp;
    }
    
    std::stringstream ang_fname;
    ang_fname<<prefix_str<<"refdata_obl_"<<inc_ang_deg;
    
    octave_obl_script(prefix_str,ang_fname.str());
    
    std::ofstream file(ang_fname.str().c_str(),std::ios::out|std::ios::trunc);
    
    using std::abs;
    using std::sqrt;
    using std::norm;
        
    std::cout<<inc_ang_deg<<" "<<Nt<<std::endl;
    
    for(l=0;l<Nl;l++)
    {
        file<<Lambda(l)<<" ";
        
//        if(polar_mode=="TE")
//        {
            double a=sqrt(norm(ref2x(l))+norm(ref2y(l))+norm(ref2z(l)));
            double b=sqrt(norm(ref1x(l))+norm(ref1y(l))+norm(ref1z(l)));
            file<<a/b<<" ";
            a=sqrt(norm(trans2x(l))+norm(trans2y(l))+norm(trans2z(l)));
            b=sqrt(norm(trans1x(l))+norm(trans1y(l))+norm(trans1z(l)));
            file<<a/b<<std::endl;
//        }
//        else if(polar_mode=="TM")
//        {
//            double a=abs(ref2TM(l));
//            double b=abs(ref1TM(l));
//            file<<a/b<<" ";
//            a=abs(trans2TM(l));
//            b=abs(trans1TM(l));
//            file<<a/b<<std::endl;
//        }
    }
}*/

//void FDTD::oblique_incidence(int vmode)
//{
//    int i,j,k,l,t;
//    
//    int Nmem=500;
//    if(3*Nshift<Nmem) Nmem=3*Nshift;
//    int it,Nit=9;
//    int np,nf;
//    
//    tshift=-1000*Dt;
//    
//    std::time_t a,b;
//        
//    Grid3<double> F_bufEy(Ny,Nz,Nmem,0);
//    Grid3<double> F_bufEz(Ny,Nz,Nmem,0);
//    Grid3<double> F_bufHy(Ny,Nz,Nmem,0);
//    Grid3<double> F_bufHz(Ny,Nz,Nmem,0);
//    
//    Grid3<double> P_bufEy(Ny,Nz,Nmem,0);
//    Grid3<double> P_bufEz(Ny,Nz,Nmem,0);
//    Grid3<double> P_bufHy(Ny,Nz,Nmem,0);
//    Grid3<double> P_bufHz(Ny,Nz,Nmem,0);
//    
//    Grid3<double> N_bufEy_l(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufEz_l(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufHy_l(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufHz_l(Ny,Nz,Nmem,0);
//    
//    Grid3<double> N_bufEy_r(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufEz_r(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufHy_r(Ny,Nz,Nmem,0);
//    Grid3<double> N_bufHz_r(Ny,Nz,Nmem,0);
//    
//    Grid2<double> imtmp(Nx,Nz,0);
//    Bitmap tmpo(Nx,Nz);
//    
//    Grid2<double> ImE;
//    
//    if(vmode==0) ImE.init(Nx,Nz,0);
//    if(vmode==1) ImE.init(Ny,Nz,0);
//    if(vmode==2) ImE.init(Nx,Ny,0);
//    
//    int Nl=600;
//    Nl=1000;
//    //Nl=200;
//    double cl=299792458.0;
//    
//    Grid1<double> Lambda(Nl,0);
//    Grid1<double> w(Nl,0);
//    
//    for(l=0;l<Nl;l++) Lambda(l)=lambda_m+(lambda_p-lambda_m)*l/(Nl-1.0);
//    for(l=0;l<Nl;l++) w(l)=2.0*Pi*cl/Lambda(l);
//    
//    int Nlb=10000;
//    Grid1<double> wb(Nlb,0);
//    
//    for(l=0;l<Nlb;l++) wb(l)=10.0*l/(Nlb-1.0)*2.0*Pi*cl/lambda_m;
//    
//    std::ofstream fEnOut("energy_out",std::ios::out|std::ios::trunc);
//    //double tEy,tEz,tHy,tHz;
//    
//    std::ofstream frefout("ref.txt",std::ios::out|std::ios::trunc);
//    double rE1,rE2,rE3;
//    
//    Grid2<double> imfull(Nx,Nt,0);
//    Grid2<double> imfull2(Nx,Nt,0);
//        
//    //###############
//    // Buf files rdy
//    //###############
//    
//    for(t=0;t<1.5*Nt;t++)
//    {
//        if((t+1)/Nmem!=t/Nmem)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufey"<<t/Nmem;
//            fse3<<"buf/bufez"<<t/Nmem;
//            fsh2<<"buf/bufhy"<<t/Nmem;
//            fsh3<<"buf/bufhz"<<t/Nmem;
//            
//            std::ofstream file;
//            
//            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//        }
//        if((t+1)/Nmem!=t/Nmem)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufeyl"<<t/Nmem;
//            fse3<<"buf/bufezl"<<t/Nmem;
//            fsh2<<"buf/bufhyl"<<t/Nmem;
//            fsh3<<"buf/bufhzl"<<t/Nmem;
//            
//            std::ofstream file;
//            
//            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//        }
//    }
//    
//    xo_s=xs_s;
//    xo_e=xs_e;
//    Nx_o=Nx_s;
//    
//    Grid2<Imdouble> BRsensorX(Nx_o,Nl,0),BRsensorY(Nx_o,Nl,0),BRsensorZ(Nx_o,Nl,0);
//    Grid2<Imdouble> RsensorX(Nx_o,Nl,0),RsensorY(Nx_o,Nl,0),RsensorZ(Nx_o,Nl,0);
//    
//    Grid2<Imdouble> BTsensorX(Nx_o,Nl,0),BTsensorY(Nx_o,Nl,0),BTsensorZ(Nx_o,Nl,0);
//    Grid2<Imdouble> TsensorX(Nx_o,Nl,0),TsensorY(Nx_o,Nl,0),TsensorZ(Nx_o,Nl,0);
//    
//    Grid2<Imdouble> BRHsensorY(Nx_o,Nl,0);
//    Grid2<Imdouble> RHsensorY(Nx_o,Nl,0);
//    
//    Grid2<Imdouble> BTHsensorY(Nx_o,Nl,0);
//    Grid2<Imdouble> THsensorY(Nx_o,Nl,0);
//    
//    Grid1<Imdouble> PRSX(Nlb,0),PRSY(Nlb,0),PRSZ(Nlb,0);
//    Grid1<Imdouble> PTSX(Nlb,0),PTSY(Nlb,0),PTSZ(Nlb,0);
//    
//    std::time(&a);
//    
//    for(it=0;it<Nit;it++)
//    {
//        tstep=0;
//        iteration=it;
//        
//        reset_fields();
//        
//        F_bufEy=0;
//        F_bufEz=0;
//        F_bufHy=0;
//        F_bufHz=0;
//        
//        P_bufEy=0;
//        P_bufEz=0;
//        P_bufHy=0;
//        P_bufHz=0;
//        
//        N_bufEy_l=0;
//        N_bufEz_l=0;
//        N_bufHy_l=0;
//        N_bufHz_l=0;
//        
//        N_bufEy_r=0;
//        N_bufEz_r=0;
//        N_bufHy_r=0;
//        N_bufHz_r=0;
//        
//        if(it>0)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufey"<<Nshift/Nmem;
//            fse3<<"buf/bufez"<<Nshift/Nmem;
//            fsh2<<"buf/bufhy"<<Nshift/Nmem;
//            fsh3<<"buf/bufhz"<<Nshift/Nmem;
//            
//            bufread(F_bufEy,Nmem,fse2.str());
//            bufread(F_bufEz,Nmem,fse3.str());
//            bufread(F_bufHy,Nmem,fsh2.str());
//            bufread(F_bufHz,Nmem,fsh3.str());
//        }
//        if(it>0)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufeyl"<<Nshift/Nmem;
//            fse3<<"buf/bufezl"<<Nshift/Nmem;
//            fsh2<<"buf/bufhyl"<<Nshift/Nmem;
//            fsh3<<"buf/bufhzl"<<Nshift/Nmem;
//            
//            bufread(P_bufEy,Nmem,fse2.str());
//            bufread(P_bufEz,Nmem,fse3.str());
//            bufread(P_bufHy,Nmem,fsh2.str());
//            bufread(P_bufHz,Nmem,fsh3.str());
//        }
//        
//        int M,pk=0;
//        
//        double C1,C2x,C2y,C2z;
//        
//        #ifdef MOV
//        int mov_i=0;
//        #endif
//        
//        std::ofstream tfile;
//        if(it==0) tfile.open("test",std::ios::out|std::ios::trunc);
//        
//        for(t=0;t<Nt;t++)
//        {
//            np=t-Nshift-1;
//            nf=t+Nshift-1;
//                            
//            update_E();
//            
//            if(np>=0)
//            {
//                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//                {
//                    i=xs_e+3;
//                    
//                    M=matsgrid(i,j,k);
//                    
//                    mats(M).coeffsY(C1,C2x,C2z);
//                    #ifdef FASTOBL
//                    Ey(i,j,k)-=C2x*N_bufHz_l(j,k,(np)%Nmem);
//                    #endif
//                    #ifndef FASTOBL
//                    Ey(i,j,k)-=C2x*P_bufHz(j,k,(np)%Nmem);
//                    #endif
//                    
//                    mats(M).coeffsZ(C1,C2x,C2y);
//                    #ifdef FASTOBL
//                    Ez(i,j,k)+=C2x*N_bufHy_l(j,k,(np)%Nmem);
//                    #endif
//                    #ifndef FASTOBL
//                    Ez(i,j,k)+=C2x*P_bufHy(j,k,(np)%Nmem);
//                    #endif
//                    
//                }}
//            }
//            
//            if(nf>=0)
//            {
//                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//                {
//                    M=matsgrid(xs_s,j,k);
//                    
//                    mats(M).coeffsY(C1,C2x,C2z);
//                    Ey(xs_s,j,k)+=C2x*F_bufHz(j,k,(nf)%Nmem);
//                    
//                    mats(M).coeffsZ(C1,C2x,C2y);
//                    Ez(xs_s,j,k)-=C2x*F_bufHy(j,k,(nf)%Nmem);
//                }}
//            }
//            
////            if(it>=Nit/2)
////            {
////                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
////                {
////                    Ex(xs_e+4,j,k)=0;
////                    Ey(xs_e+4,j,k)=0;
////                    Ez(xs_e+4,j,k)=0;
////                    
////                    Ex(xs_s-1,j,k)=0;
////                    Ey(xs_s-1,j,k)=0;
////                    Ez(xs_s-1,j,k)=0;
////                }}
////            }
//            
//            update_H();
//            
//            //Cache loading - Past
//            
//            #ifndef FASTOBL
//            if(np+1<Nt && (np+1)/Nmem!=np/Nmem && it>0)
//            {
//                std::stringstream fse2,fse3,fsh2,fsh3;
//                fse2<<"buf/bufeyl"<<(np+1)/Nmem;
//                fse3<<"buf/bufezl"<<(np+1)/Nmem;
//                fsh2<<"buf/bufhyl"<<(np+1)/Nmem;
//                fsh3<<"buf/bufhzl"<<(np+1)/Nmem;
//                
//                bufread(P_bufEy,Nmem,fse2.str());
//                bufread(P_bufEz,Nmem,fse3.str());
//                bufread(P_bufHy,Nmem,fsh2.str());
//                bufread(P_bufHz,Nmem,fsh3.str());
//                
//                std::cout<<"TAC "<<(np+1)/Nmem<<std::endl;
//            }
//            #endif
//            
//            if(np>=0)
//            {
//                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//                {
//                    i=xs_e+3;
//                    
//                    #ifdef FASTOBL
//                    Hy(i-1,j,k)+=dtdmx*N_bufEz_l(j,k,(np+1)%Nmem);  //np+1
//                    Hz(i-1,j,k)-=dtdmx*N_bufEy_l(j,k,(np+1)%Nmem);  //np+1
//                    #endif
//                    #ifndef FASTOBL
//                    Hy(i-1,j,k)+=dtdmx*P_bufEz(j,k,(np+1)%Nmem);  //np+1
//                    Hz(i-1,j,k)-=dtdmx*P_bufEy(j,k,(np+1)%Nmem);  //np+1
//                    #endif
//                }}
//            }
//            
//            // Cache loading - Future
//            
//            if((nf+1)/Nmem!=nf/Nmem && nf+1<Nt)
//            {
//                std::stringstream fse2,fse3,fsh2,fsh3;
//                fse2<<"buf/bufey"<<(nf+1)/Nmem;
//                fse3<<"buf/bufez"<<(nf+1)/Nmem;
//                fsh2<<"buf/bufhy"<<(nf+1)/Nmem;
//                fsh3<<"buf/bufhz"<<(nf+1)/Nmem;
//                
//                bufread(F_bufEy,Nmem,fse2.str());
//                bufread(F_bufEz,Nmem,fse3.str());
//                bufread(F_bufHy,Nmem,fsh2.str());
//                bufread(F_bufHz,Nmem,fsh3.str());
//                
//                std::cout<<"TOC "<<(nf+1)/Nmem<<std::endl;
//            }
//            
//            if((nf+1)/Nmem!=nf/Nmem && nf+1>=Nt)
//            {
//                F_bufEy=0; F_bufEz=0; F_bufHy=0; F_bufHz=0;
//            }
//            
//            // H update - Future
//            
//            if(nf>=0)
//            {
//                for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//                {
//                    Hy(xs_s-1,j,k)-=dtdmx*F_bufEz(j,k,(nf+1)%Nmem);  //nf+1
//                    Hz(xs_s-1,j,k)+=dtdmx*F_bufEy(j,k,(nf+1)%Nmem);  //nf+1
//                }}
//            }
//            
//            // Temporary mem storage
//            
//            for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
//            {
//                using std::abs;
//                i=xs_s+3;
//                
//                N_bufEy_l(j,k,t%Nmem)=Ey(i,j,k);
//                N_bufEz_l(j,k,t%Nmem)=Ez(i,j,k);
//                N_bufHy_l(j,k,t%Nmem)=Hy(i-1,j,k);
//                N_bufHz_l(j,k,t%Nmem)=Hz(i-1,j,k);
//                
//                i=xs_e;
//                
//                N_bufEy_r(j,k,t%Nmem)=Ey(i,j,k);
//                N_bufEz_r(j,k,t%Nmem)=Ez(i,j,k);
//                N_bufHy_r(j,k,t%Nmem)=Hy(i-1,j,k);
//                N_bufHz_r(j,k,t%Nmem)=Hz(i-1,j,k);
//            }}
//            
//            //Cache writing - Future
//            
//            if((t+1)/Nmem!=t/Nmem)
//            {
//                std::stringstream fse2,fse3,fsh2,fsh3;
//                fse2<<"buf/bufey"<<t/Nmem;
//                fse3<<"buf/bufez"<<t/Nmem;
//                fsh2<<"buf/bufhy"<<t/Nmem;
//                fsh3<<"buf/bufhz"<<t/Nmem;
//                
//                bufwrite(N_bufEy_r,Nmem,fse2.str());
//                bufwrite(N_bufEz_r,Nmem,fse3.str());
//                bufwrite(N_bufHy_r,Nmem,fsh2.str());
//                bufwrite(N_bufHz_r,Nmem,fsh3.str());
//                
//                std::cout<<"TIC R"<<t/Nmem<<std::endl;
//            }
//            
//            //Cache writing - Past
//            
//            #ifndef FASTOBL
//            if((t+1)/Nmem!=t/Nmem)
//            {
//                std::stringstream fse2,fse3,fsh2,fsh3;
//                fse2<<"buf/bufeyl"<<t/Nmem;
//                fse3<<"buf/bufezl"<<t/Nmem;
//                fsh2<<"buf/bufhyl"<<t/Nmem;
//                fsh3<<"buf/bufhzl"<<t/Nmem;
//                
//                bufwrite(N_bufEy_l,Nmem,fse2.str());
//                bufwrite(N_bufEz_l,Nmem,fse3.str());
//                bufwrite(N_bufHy_l,Nmem,fsh2.str());
//                bufwrite(N_bufHz_l,Nmem,fsh3.str());
//                
//                std::cout<<"TIC L"<<t/Nmem<<std::endl;
//            }
//            #endif
//            
//            std::time(&b);
//            
//            if(t%100==0)
//            {
//                double tb=(b-a)/(t+1.0+it*Nt)*(Nt-t+Nt*(Nit-1-it));
//                int t1=int(tb/3600.0);
//                int t2=int((tb-3600.0*t1)/60.0);
//                double t3=tb-3600.0*t1-60*t2;
//                
//                double tc=b-a;
//                int tc1=int(tc)/3600;
//                int tc2=(int(tc)-3600*tc1)/60;
//                double tc3=tc-3600*tc1-60*tc2;
//                std::cout<<(t+it*Nt)/(Nt*Nit-1.0)*100.0<<"% "<<tc1<<":"<<tc2<<":"<<tc3<<" "
//                            <<(b-a)/(t+1.0+it*Nt)<<" "<<t1<<":"<<t2<<":"<<t3<<" "<<t<<" / "<<Nt<<" "<<t+it*Nt<<" / "<<Nt*Nit-1<<std::endl;
//            }
//            
//            if(t/static_cast<double>(Nt)<=pk/100.0 && (t+1.0)/Nt>pk/100.0)
//            {
//                draw(t,vmode,Nx/2,Ny/2,Nz/2);
//                
//                double tb=(t+0.5)*Dt; //calibration
//                
//                //for(i=xs_s;i<xs_e+3;i++){ for(k=zs_s;k<zs_e;k++)
//                for(i=0;i<Nx;i++){ for(k=0;k<Nz;k++)
//                {
//                    double x=i*Dx;
//                    double z=k*Dz;
////                    imtmp(i,k)=std::abs(Ex(i,0,k)-chp.Ex(x+Dx/2.0,0,z,tb+tshift));
//                    imtmp(i,k)=std::abs(Ez(i,0,k)-chp.Ez(x,0,z+Dz/2.0,tb+tshift));
////                    imtmp(i,k)=std::abs(Ey(i,0,k)-chp.Ey(x,0,z,tb+tshift));
//                }}
//                
//                imtmp(0,0)=0.01;
//                
//                tmpo.G2degra(imtmp,"RenderW/rend",t,".bmp");
//                
//                pk++;
//            }
//            
//            if(it==Nit-1)
//            {
//                Imdouble tmp1;
//                
//                double tb=(t+0.5)*Dt; //calibration
//                
//                double Dx2=Dx/2.0;
//                double Dz2=Dz/2.0;
//                
//                rE1=rE2=rE3=0;
//                
//                for(i=xo_s;i<xo_e;i++)
//                {
//                    double x=i*Dx;
//                    
//                    double BRSX=0,BRSY=0,BRSZ=0;
//                    double RSX=0,RSY=0,RSZ=0;
//                    double BTSX=0,BTSY=0,BTSZ=0;
//                    double TSX=0,TSY=0,TSZ=0;
//                                        
//                    double z=(zs_e-2)*Dz;
//                    
//                    double chxr=chp.Ex(x+Dx2,0,z,tb+tshift);
//                    double chyr=chp.Ey(x,0,z,tb+tshift);
//                    double chzr=chp.Ez(x,0,z+Dz2,tb+tshift);
//                                        
//                    z=(pml_z+2)*Dz;
//                    
//                    double chxt=chp.Ex(x+Dx2,0,z,tb+tshift);
//                    double chyt=chp.Ey(x,0,z,tb+tshift);
//                    double chzt=chp.Ez(x,0,z+Dz2,tb+tshift);
//                                        
//                    for(j=0;j<Ny;j++)
//                    {
//                        BRSX+=chxr;
//                        BRSY+=chyr;
//                        BRSZ+=chzr;
//
//                        RSX+=(Ex(i,j,zs_e-2)-chxr);
//                        RSY+=(Ey(i,j,zs_e-2)-chyr);
//                        RSZ+=(Ez(i,j,zs_e-2)-chzr);
//                        
//                        BTSX+=chxt;
//                        BTSY+=chyt;
//                        BTSZ+=chzt;
//                        
//                        TSX+=Ex(i,j,pml_z+2);
//                        TSY+=Ey(i,j,pml_z+2);
//                        TSZ+=Ez(i,j,pml_z+2);
//                    }
//                    
//                    int i2=i-xo_s;
//                    
//                    for(l=0;l<Nl;l++)
//                    {
//                        tmp1=std::exp(tb*w(l)*Im)*Dt/(Ny+0.0);
//                        
//                        BRsensorX(i2,l)+=BRSX*tmp1;
//                        BRsensorY(i2,l)+=BRSY*tmp1;
//                        BRsensorZ(i2,l)+=BRSZ*tmp1;
//                        
//                        RsensorX(i2,l)+=RSX*tmp1;
//                        RsensorY(i2,l)+=RSY*tmp1;
//                        RsensorZ(i2,l)+=RSZ*tmp1;
//                                                    
//                        BTsensorX(i2,l)+=BTSX*tmp1;
//                        BTsensorY(i2,l)+=BTSY*tmp1;
//                        BTsensorZ(i2,l)+=BTSZ*tmp1;
//                        
//                        TsensorX(i2,l)+=TSX*tmp1;
//                        TsensorY(i2,l)+=TSY*tmp1;
//                        TsensorZ(i2,l)+=TSZ*tmp1;
//                    }
//                    
//                    rE1+=(RSX);
//                    rE2+=(RSY);
//                    rE3+=(RSZ);
//                    
//                }
//                
//                //###############
//                // Correction
//                //###############
//                
//                double RSX=0,RSY=0,RSZ=0;
//                double TSX=0,TSY=0,TSZ=0;
//                
//                i=xs_e+10;
//                
//                for(j=0;j<Ny;j++)
//                {
//                    RSX+=Ex(i,j,zs_e-2);
//                    RSY+=Ey(i,j,zs_e-2);
//                    RSZ+=Ez(i,j,zs_e-2);
//                    
//                    TSX+=Ex(i,j,pml_z+2);
//                    TSY+=Ey(i,j,pml_z+2);
//                    TSZ+=Ez(i,j,pml_z+2);
//                }
//                
//                //int i2=i-xo_s;
//                
//                for(l=0;l<Nlb;l++)
//                {
//                    tmp1=std::exp(tb*wb(l)*Im)*Dt/(Ny+0.0);
//                    
//                    PRSX(l)+=RSX*tmp1;
//                    PRSY(l)+=RSY*tmp1;
//                    PRSZ(l)+=RSZ*tmp1;
//                    
//                    PTSX(l)+=TSX*tmp1;
//                    PTSY(l)+=TSY*tmp1;
//                    PTSZ(l)+=TSZ*tmp1;
//                }
//                
//                frefout<<t<<" "<<rE1<<" "<<rE2<<" "<<rE3<<std::endl;
//            }
//            
//            i=xs_e+10;
//            
//            double Sxl=0,Sxr=0;
//            
//            Sxr=compute_poynting_X(ys_s,ys_e,zs_s,zs_e,i,1)/(Ny_s*Dy)/(Nz_s*Dz);
//            
//            i=xs_s-5;
//            
//            Sxl=compute_poynting_X(ys_s,ys_e,zs_s,zs_e,i,-1)/(Ny_s*Dy)/(Nz_s*Dz);
//            
//            if(t%10==0) fEnOut<<std::abs(Sxr)+std::abs(Sxl)<<std::endl;
//            
//            for(i=0;i<Nx;i++)
//            {
//                double tb=(t+0.5)*Dt; //calibration
//                
//                imfull(i,t)=1.0-std::exp(-10*std::abs(Ez(i,0,zs_e-2)));
//                imfull2(i,t)=1.0-std::exp(-10*std::abs(Ez(i,0,zs_e-2)
//                                    -chp.Ez(i*Dx,0,Dz*(zs_e-1.5),tb+tshift)*is_in_r(i,xs_s,xs_e+3)));
//                                    
////                imfull(i,t)=1.0-std::exp(-10*std::abs(Ey(i,0,zs_e-2)));
////                imfull2(i,t)=1.0-std::exp(-10*std::abs(Ey(i,0,zs_e-2)
////                                    -chp.Ey(i*Dx,0,Dz*(zs_e-2),tb+tshift)*is_in_r(i,xs_s,xs_e+3)));
//                                    
////                imfull(i,t)=Ey(i,0,zs_e-2);
////                imfull2(i,t)=Ey(i,0,zs_e-2)
////                                    -chp.Ey(i*Dx,0,Dz*(zs_e-2),tb+tshift)*is_in_r(i,xs_s,xs_e+3);
//            }
//        }
//        std::stringstream strmim;
//        strmim<<"full"<<it<<".bmp";
//        G2_to_degra(imfull,strmim.str());
//        
//        strmim.str("");
//        strmim<<"fulb"<<it<<".bmp";
//        G2_to_degra(imfull2,strmim.str());
//    }
//    
//    Grid2<double> imfull3(Nx,Nt,0);
//    
//    double Dw=wb(1);
//    
//    ProgDisp dsp2(Nt);
//    
//    Grid1<double> px1(Nt,0);
//    
//    for(t=0;t<Nt;t++)
//    {
//        double tau=-2.0*Dt*Nshift+(xs_e-(xs_s+3))*Dx/c_light;
//        //double x=Dx*(xo_s+i);
//        double x=Dx*(i);
//        double x1=Dx*(xs_e+10);
//        double xc=(x1-x)/c_light;
//        xc=0;
//        
//        Imdouble S=0;
//        
//        for(l=0;l<Nlb;l++)
//        {
//            //Imdouble tmp1=std::exp(-wb(l)*xc*Im)/(1.0+std::exp(-wb(l)*tau*Im));
//            Imdouble tmp1=std::exp(-wb(l)*(xc+t*Dt)*Im)/(1.0+std::exp(-wb(l)*tau*Im))*Dw;
//            
////            RsensorX(i,l)-=PRSX(l)*tmp1;
////            RsensorY(i,l)-=PRSY(l)*tmp1;
//            S+=PRSZ(l)*tmp1;
//            
////            TsensorX(i,l)-=PTSX(l)*tmp1;
////            TsensorY(i,l)-=PTSY(l)*tmp1;
////            TsensorZ(i,l)-=PTSZ(l)*tmp1;
//
//        }
//        
//        px1(t)=std::real(S)/Pi;
//        //plog<<px1(t)<<std::endl;
//        
//        ++dsp2;
//    }
//    
//    for(i=0;i<Nx;i++){ for(t=0;t<Nt;t++)
//    {
//        double x=Dx*(i);
//        double x1=Dx*(xs_e+10);
//        double xc=(x-x1)/c_light;
//        
//        double tb=t*Dt-xc;
//        
//        double u=tb/Dt;
//        int Ntb=static_cast<int>(u);
//        u-=Ntb;
//        
//        if(is_in_r(Ntb,0,Nt-1))
//        {
//            imfull3(i,t)=(1.0-u)*px1(Ntb)+u*px1(Ntb+1);
//        }
//        else if(Ntb>=Nt-1)
//        {
//            imfull3(i,t)=px1(Nt-1);
//        }
//        else
//        {
//            imfull3(i,t)=0;
//        }
//        
//        //imfull3(i,t)=1.0-std::exp(-10*std::abs(imfull3(i,t)));
//    }}
//    
//    G2_to_degra(imfull3,"reconst.bmp");
//    
//    Grid1<Imdouble> ref1x(Nl,0),ref1y(Nl,0),ref1z(Nl,0);
//    Grid1<Imdouble> ref2x(Nl,0),ref2y(Nl,0),ref2z(Nl,0);
//    Grid1<Imdouble> trans1x(Nl,0),trans1y(Nl,0),trans1z(Nl,0);
//    Grid1<Imdouble> trans2x(Nl,0),trans2y(Nl,0),trans2z(Nl,0);
//    
//    Grid1<Imdouble> ref1TE(Nl,0),ref2TE(Nl,0);
//    Grid1<Imdouble> ref1TM(Nl,0),ref2TM(Nl,0);
//    Grid1<Imdouble> trans1TE(Nl,0),trans2TE(Nl,0);
//    Grid1<Imdouble> trans1TM(Nl,0),trans2TM(Nl,0);
//    
//    Grid1<Imdouble> refa(Nl,0),refb(Nl,0),refc(Nl,0);
//    
//    ProgDisp dsp(Nl,"Fourier Transform");
//    
//    for(i=0;i<Nx_o;i++)
//    {
//        plog<<std::abs(RsensorX(i,Nl/5))<<std::endl;
//    }
//    
//    for(l=0;l<Nl;l++)
//    {
//        double kl=2.0*Pi/Lambda(l)*std::sin(inc_ang);
//        
//        Imdouble tmp1;
//        
//        for(i=0;i<Nx_o;i++)
//        {
//            tmp1=std::exp(-i*Dx*kl*Im);
//            
//            ref1x(l)+=BRsensorX(i,l)*tmp1;
//            ref1y(l)+=BRsensorY(i,l)*tmp1;
//            ref1z(l)+=BRsensorZ(i,l)*tmp1;
//            
//            trans1x(l)+=BTsensorX(i,l)*tmp1;
//            trans1y(l)+=BTsensorY(i,l)*tmp1;
//            trans1z(l)+=BTsensorZ(i,l)*tmp1;
//            
//            ref2x(l)+=RsensorX(i,l)*tmp1;
//            ref2y(l)+=RsensorY(i,l)*tmp1;
//            ref2z(l)+=RsensorZ(i,l)*tmp1;
//            
//            trans2x(l)+=TsensorX(i,l)*tmp1;
//            trans2y(l)+=TsensorY(i,l)*tmp1;
//            trans2z(l)+=TsensorZ(i,l)*tmp1;
//        }
//        
//        for(i=0;i<60;i++)
//        {
//            tmp1=std::exp(-i*Dx*kl*Im);
//            
//            refa(l)+=RsensorX(i,l)*tmp1;
//        }
//        
////        for(i=60;i<120;i++)
////        {
////            tmp1=std::exp(-i*Dx*kl*Im);
////            
////            refb(l)+=RsensorX(i,l)*tmp1;
////        }
////        
////        for(i=120;i<180;i++)
////        {
////            tmp1=std::exp(-i*Dx*kl*Im);
////            
////            refc(l)+=RsensorX(i,l)*tmp1;
////        }
//        
////        plog<<Lambda(l)<<" "<<std::abs(refa(l))<<" "<<std::abs(refb(l))<<" "<<std::abs(refc(l))<<std::endl;
//        
//        ++dsp;
//    }
//    
//    std::stringstream ang_fname;
//    ang_fname<<prefix<<"refdata_obl_"<<inc_ang*180.0/Pi;
//    
//    octave_obl_script(prefix,ang_fname.str());
//    
//    std::ofstream file(ang_fname.str().c_str(),std::ios::out|std::ios::trunc);
//    
//    using std::abs;
//    using std::sqrt;
//    using std::norm;
//        
//    std::cout<<inc_ang*180.0/Pi<<" "<<Nt<<std::endl;
//    
//    for(l=0;l<Nl;l++)
//    {
//        file<<Lambda(l)<<" ";
//        
////        if(polar_mode=="TE")
////        {
//            double a=sqrt(norm(ref2x(l))+norm(ref2y(l))+norm(ref2z(l)));
//            double b=sqrt(norm(ref1x(l))+norm(ref1y(l))+norm(ref1z(l)));
//            file<<a/b<<" ";
//            a=sqrt(norm(trans2x(l))+norm(trans2y(l))+norm(trans2z(l)));
//            b=sqrt(norm(trans1x(l))+norm(trans1y(l))+norm(trans1z(l)));
//            file<<a/b<<std::endl;
////        }
////        else if(polar_mode=="TM")
////        {
////            double a=abs(ref2TM(l));
////            double b=abs(ref1TM(l));
////            file<<a/b<<" ";
////            a=abs(trans2TM(l));
////            b=abs(trans1TM(l));
////            file<<a/b<<std::endl;
////        }
//    }
//    
//    file.close();
//    
//    for(t=0;t<Nt;t++)
//    {
//        if((t+1)/Nmem!=t/Nmem)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufey"<<t/Nmem;
//            fse3<<"buf/bufez"<<t/Nmem;
//            fsh2<<"buf/bufhy"<<t/Nmem;
//            fsh3<<"buf/bufhz"<<t/Nmem;
//            
//            std::ofstream file;
//            
//            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//        }
//        if((t+1)/Nmem!=t/Nmem)
//        {
//            std::stringstream fse2,fse3,fsh2,fsh3;
//            fse2<<"buf/bufeyl"<<t/Nmem;
//            fse3<<"buf/bufezl"<<t/Nmem;
//            fsh2<<"buf/bufhyl"<<t/Nmem;
//            fsh3<<"buf/bufhzl"<<t/Nmem;
//            
//            std::ofstream file;
//            
//            file.open(fse2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fse3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh2.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//            file.open(fsh3.str().c_str(),std::ios::out|std::ios::trunc);
//            file.close();
//        }
//    }
//}
