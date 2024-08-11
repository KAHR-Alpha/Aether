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

void mode_default_fdtd(FDTD_Mode const &fdtd_mode,std::atomic<bool> *end_computation,ProgTimeDisp *dsp_,Bitmap *bitmap_)
{
    int t;
    
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
    
    double Dt=std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light)*0.99*fdtd_mode.time_mod;
    
    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",
              fdtd_mode.pml_xm,fdtd_mode.pml_xp,
              fdtd_mode.pml_ym,fdtd_mode.pml_yp,
              fdtd_mode.pml_zm,fdtd_mode.pml_zp,
              fdtd_mode.pad_xm,fdtd_mode.pad_xp,
              fdtd_mode.pad_ym,fdtd_mode.pad_yp,
              fdtd_mode.pad_zm,fdtd_mode.pad_zp);
    
    // PML
    
    fdtd.set_pml_xm(fdtd_mode.kappa_xm,fdtd_mode.sigma_xm,fdtd_mode.alpha_xm);
    fdtd.set_pml_xp(fdtd_mode.kappa_xp,fdtd_mode.sigma_xp,fdtd_mode.alpha_xp);
    fdtd.set_pml_ym(fdtd_mode.kappa_ym,fdtd_mode.sigma_ym,fdtd_mode.alpha_ym);
    fdtd.set_pml_yp(fdtd_mode.kappa_yp,fdtd_mode.sigma_yp,fdtd_mode.alpha_yp);
    fdtd.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
    fdtd.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
    
    fdtd.set_tapering(fdtd_mode.tapering);
    
    // Grid and materials
    
    fdtd.set_matsgrid(matsgrid);
    
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdtd_mode.materials_str.size();m++)
        fdtd.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
    #endif
    for(unsigned int m=0;m<fdtd_mode.materials.size();m++)
        fdtd.set_material(m,fdtd_mode.materials[m]);
    
    // Disabling fields
    
    fdtd.disable_fields(fdtd_mode.disable_fields);
    
    // Spectrum and incident field
    
    Plog::print("Computing\n");
    
    fdtd.bootstrap();
    
    std::vector<Sensor*> sensors;
    std::vector<Source*> sources;
    
    for(unsigned int i=0;i<fdtd_mode.sensors.size();i++)
        sensors.push_back(generate_fdtd_sensor(fdtd_mode.sensors[i], fdtd, fdtd_mode.directory()));
    
    for(unsigned int i=0;i<fdtd_mode.sources.size();i++)
        sources.push_back(generate_fdtd_source(fdtd_mode.sources[i],fdtd));
    
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
    
    // Main Loop
    
    for(t=0;t<Nt;t++)
    {
        // E-field
        fdtd.update_E();
        
        // E-field injection
        
        for(unsigned int i=0;i<sources.size();i++)
            sources[i]->inject_E(fdtd);
        
        // H-field
        fdtd.update_H();
        
        for(unsigned int i=0;i<sensors.size();i++)
            sensors[i]->feed(fdtd);
        
        // H-field injection
        
        for(unsigned int i=0;i<sources.size();i++)
            sources[i]->inject_H(fdtd);
        
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
    for(unsigned int i=0;i<sources.size();i++) delete sources[i];
}


//###############################

void testlin_1(int N,int Nt)
{
    int i,t;
    
    double *Ey=new double[N];
    double *Hz=new double[N];
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    for(i=0;i<N;i++)
    {
        Ey[i]=0;
        Hz[i]=0;
    }
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=1;i<N;i++)   Ey[i]=Ey[i]+C1*(Hz[i]-Hz[i-1]);
        for(i=0;i<N-1;i++) Hz[i]=Hz[i]-C2*(Ey[i+1]-Ey[i]);
    }
    
    b=clock.now();
    
    std::chrono::duration<double> time=b-a;
    Plog::print("Duration split: ", time.count(), "\n");
    
    delete[] Ey;
    delete[] Hz;
}

void testlin_1_3D(int Nx,int Ny,int Nz,int Nt)
{
    int i,j,k,t;
    
    Grid3<double> Ey(Nx,Ny,Nz,0),Hz(Nx,Ny,Nz,0);
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=0;i<Nx-1;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            Ey(i,j,k)=Ey(i,j,k)+C1*(Hz(i,j,k)-Hz(i-1,j,k));
        
        for(i=1;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            Hz(i,j,k)=Hz(i,j,k)-C2*(Ey(i+1,j,k)-Ey(i,j,k));
    }
    
    b=clock.now();
    
    std::chrono::duration<double> time=b-a;
    Plog::print("Duration split: ", time.count() ,"\n");
}

void testlin_2(int N,int Nt)
{
    int i,t;
    
    double *EM=new double[2*N];
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    for(i=0;i<N;i++)
    {
        EM[2*i+0]=0;
        EM[2*i+1]=0;
    }
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=1;i<N;i++)   EM[2*i+0]=EM[2*i+0]+C1*(EM[2*i+1]-EM[2*(i-1)+1]);
        for(i=0;i<N-1;i++) EM[2*i+1]=EM[2*i+1]-C2*(EM[2*(i+1)+0]-EM[2*i+0]);
    }
    
    b=clock.now();
    std::chrono::duration<double> time=b-a;
    
    Plog::print("Duration interlaced: ", time.count(), "\n");
    
    delete[] EM;
}

void testlin_2_3D(int Nx,int Ny,int Nz,int Nt)
{
    int i,j,k,t;
    
    Grid4<double> EM(2,Nx,Ny,Nz,0);
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=0;i<Nx-1;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            EM(0,i,j,k)=EM(0,i,j,k)+C1*(EM(1,i,j,k)-EM(1,i-1,j,k));
        
        for(i=1;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            EM(1,i,j,k)=EM(1,i,j,k)-C2*(EM(0,i+1,j,k)-EM(0,i,j,k));
    }
    
    b=clock.now();
    
    std::chrono::duration<double> time=b-a;
    Plog::print("Duration interlaced 1: ", time.count(), "\n");
}

class node
{
    public:
        double Ey,Hz;
};

void testlin_3(int N,int Nt)
{
    int i,t;
    
    node *nEM=new node[N];
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    for(i=0;i<N;i++)
    {
        nEM[i].Ey=0;
        nEM[i].Hz=0;
    }
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=1;i<N;i++)   nEM[i].Ey=nEM[i].Ey+C1*(nEM[i].Hz-nEM[i-1].Hz);
        for(i=0;i<N-1;i++) nEM[i].Hz=nEM[i].Hz-C2*(nEM[i+1].Ey-nEM[i].Ey);
    }
    
    b=clock.now();
    
    std::chrono::duration<double> time=b-a;
    
    Plog::print("Duration interlaced 2: ", time.count(), "\n");
    
    delete[] nEM;
}

void testlin_3_3D(int Nx,int Ny,int Nz,int Nt)
{
    int i,j,k,t;
    
    Grid3<node> nEM(Nx,Ny,Nz);
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        nEM(i,j,k).Ey=0;
        nEM(i,j,k).Hz=0;
    }
    
    double Dx=5e-9;
    double Dt=Dx/(std::sqrt(3.0)*c_light);
    
    double C1=Dt/(Dx*e0);
    double C2=Dt/(Dx*mu0);
    
    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point a,b;
    
    a=clock.now();
    
    for(t=0;t<Nt;t++)
    {
        for(i=0;i<Nx-1;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            nEM(i,j,k).Ey=nEM(i,j,k).Ey+C1*(nEM(i,j,k).Hz-nEM(i-1,j,k).Hz);
        
        for(i=1;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            nEM(i,j,k).Hz=nEM(i,j,k).Hz-C2*(nEM(i+1,j,k).Ey-nEM(i,j,k).Ey);
    }
    
    b=clock.now();
    
    std::chrono::duration<double> time=b-a;
    Plog::print("Duration interlaced 2: ", time.count(), "\n");
}

void testlin()
{
//    Plog::print("N: ", 100000, "\n");
//    Plog::print("Nt: ", 100000, "\n");
//    testlin_1(100000,100000);
//    testlin_2(100000,100000);
//    testlin_3(100000,100000);
//    
//    Plog::print("N: ", 1000000, "\n");
//    Plog::print("Nt: ", 10000, "\n");
//    testlin_1(1000000,10000);
//    testlin_2(1000000,10000);
//    testlin_3(1000000,10000);
//    
//    Plog::print("N: ", 10000000, "\n");
//    Plog::print("Nt: ", 1000, "\n");
//    testlin_1(10000000,1000);
//    testlin_2(10000000,1000);
//    testlin_3(10000000,1000);
//    
//    Plog::print("N: ", 100000000, "\n");
//    Plog::print("Nt: ", 100, "\n");
//    testlin_1(100000000,100);
//    testlin_2(100000000,100);
//    testlin_3(100000000,100);
    
    Plog::print("N: ", 50, "\n");
    Plog::print("Nt: ", 20000, "\n");
    testlin_1_3D(50,50,50,20000);
    testlin_2_3D(50,50,50,20000);
    testlin_3_3D(50,50,50,20000);
    
    Plog::print("N: ", 100, "\n");
    Plog::print("Nt: ", 2500, "\n");
    testlin_1_3D(100,100,100,2500);
    testlin_2_3D(100,100,100,2500);
    testlin_3_3D(100,100,100,2500);
    
    Plog::print("N: ", 500, "\n");
    Plog::print("Nt: ", 20, "\n");
    testlin_1_3D(500,500,500,20);
    testlin_2_3D(500,500,500,20);
    testlin_3_3D(500,500,500,20);
}

void mode_fdtd_lab(FDTD_Mode const &fdtd_mode,std::atomic<bool> *end_computation,ProgTimeDisp *dsp_)
{
//    testlin();
//    
//    std::exit(0);
//    
//    int k,l,t;
//    
//    int Nx=60;
//    int Ny=60;
//    int Nz=60;
//    int Nt=fdtd_mode.Nt;
//    
//    int Nx_aux=60;
//    int Ny_aux=60;
//    int Nz_aux=60;
//    
//    double Dx=fdtd_mode.Dx;
//    double Dy=fdtd_mode.Dy;
//    double Dz=fdtd_mode.Dz;
//    
//    double lx,ly,lz;
//    
//    fdtd_mode.structure.retrieve_nominal_size(lx,ly,lz);
//    fdtd_mode.compute_discretization(Nx,Ny,Nz,lx,lz,lz);
//    
//    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
//    fdtd_mode.structure.discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
//    
//    Grid3<unsigned int> aux_grid;
//    
//    if(fdtd_mode.structure_aux.set) // Using user auxiliary grid
//    {
//        double Dx_tmp=0,Dy_tmp=0,Dz_tmp=0;
//        
//        Grid3<unsigned int> aux_grid_tmp(Nx_aux,Ny_aux,Nz_aux,0);
//        fdtd_mode.structure_aux.discretize(Nx_aux,Ny_aux,Nz_aux,
//                                           Dx_tmp,Dy_tmp,Dz_tmp,aux_grid_tmp,false);
//        
//        if(Nx_aux!=1 || Ny_aux!=1)
//        {
//            if(Nx_aux!=1) Plog::print("Warning: Nx_aux (", Nx_aux, ") different from 1", "\n");
//            if(Ny_aux!=1) Plog::print("Warning: Ny_aux (", Ny_aux, ") different from 1", "\n");
//        }
//        if(Nz_aux!=Nz)
//        {
//            Plog::print("Error: Nz (", Nz, ") doesn't match Nz_aux (", Nz_aux, ")", "\n");
//            
//            return;
//        }
//        
//        aux_grid.init(1,1,Nz_aux,0);
//        
//        for(k=0;k<Nz_aux;k++)
//            aux_grid(0,0,k)=aux_grid_tmp(0,0,k);
//    }
//    else // Auto auxiliary grid
//    {
//        Nx_aux=Ny_aux=1;
//        Nz_aux=Nz;
//        
//        aux_grid.init(1,1,Nz_aux,0);
//        
//        for(k=0;k<Nz_aux;k++)
//            aux_grid(0,0,k)=matsgrid(0,0,k);
//    }
//    
//    std::string polar_mode=fdtd_mode.polarization;
//        
//    double Dt=std::min(std::min(Dx,Dy),Dz)/(std::sqrt(3.0)*c_light)*0.99*fdtd_mode.time_mod;
//    
//    FDTD fdtd(Nx,Ny,Nz,Nt,Dx,Dy,Dz,Dt,"CUSTOM",
//              fdtd_mode.pml_xm,fdtd_mode.pml_xp,
//              fdtd_mode.pml_ym,fdtd_mode.pml_yp,
//              fdtd_mode.pml_zm,fdtd_mode.pml_zp);
//    
//    // PML
//    
//    fdtd.set_pml_xm(fdtd_mode.kappa_xm,fdtd_mode.sigma_xm,fdtd_mode.alpha_xm);
//    fdtd.set_pml_xp(fdtd_mode.kappa_xp,fdtd_mode.sigma_xp,fdtd_mode.alpha_xp);
//    fdtd.set_pml_ym(fdtd_mode.kappa_ym,fdtd_mode.sigma_ym,fdtd_mode.alpha_ym);
//    fdtd.set_pml_yp(fdtd_mode.kappa_yp,fdtd_mode.sigma_yp,fdtd_mode.alpha_yp);
//    fdtd.set_pml_zm(fdtd_mode.kappa_zm,fdtd_mode.sigma_zm,fdtd_mode.alpha_zm);
//    fdtd.set_pml_zp(fdtd_mode.kappa_zp,fdtd_mode.sigma_zp,fdtd_mode.alpha_zp);
//    
//    fdtd.set_prefix(fdtd_mode.prefix);
//    
//    // Grid and materials
//    
//    fdtd.set_matsgrid(matsgrid);
//    
//    Nx=fdtd.Nx;
//    Ny=fdtd.Ny;
//    Nz=fdtd.Nz;
//    
//    int zs_s=fdtd.zs_s;
//    int zs_e=fdtd.zs_e;
//    
//    #ifdef OLDMAT
//    for(unsigned int m=0;m<fdtd_mode.materials_str.size();m++)
//    {
//        fdtd.set_material(fdtd_mode.materials_index[m],fdtd_mode.materials_str[m]);
//    }
//    #endif
//    
//    for(unsigned int m=0;m<fdtd_mode.materials.size();m++)
//        fdtd.set_material(m,fdtd_mode.materials[m]);
//    
//    // Spectrum and incident field
//    
//    int Nl=fdtd_mode.Nl;
//    double lambda_min=fdtd_mode.lambda_min;
//    double lambda_max=fdtd_mode.lambda_max;
//    
//    Grid1<double> Lambda(Nl,0);
//    Grid1<double> w(Nl,0);
//    
//    for(l=0;l<Nl;l++) Lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
//    for(l=0;l<Nl;l++) w[l]=2.0*Pi*c_light/Lambda[l];
//    
////    double pol=0;
////    
////    if(polar_mode=="TE") pol=0;
////    else if(polar_mode=="TM") pol=90;
////    else if(polar_mode=="mix") pol=fdtd_mode.polar_angle;
//    
//    double eps_sub=fdtd.mats[fdtd.matsgrid(0,0,zs_s)].ei;
//    double eps_sup=fdtd.mats[fdtd.matsgrid(0,0,zs_e)].ei;
//    double index_sub=std::sqrt(eps_sub);
//    double index_sup=std::sqrt(eps_sup);
//    
//    Plog::print("u ", eps_sup, "/", index_sup, " d ", eps_sub, "/", index_sub, "\n");
//    
//    Plog::print("Computing", "\n");
//    
//    fdtd.bootstrap();
//    
//    //Adding sensors
//    
//    std::vector<Sensor*> sensors;
//    std::vector<Source*> sources;
//    
//    for(unsigned int i=0;i<fdtd_mode.sensors.size();i++)
//        sensors.push_back(generate_fdtd_sensor(fdtd_mode.sensors[i],fdtd));
//    
//    for(unsigned int i=0;i<fdtd_mode.sources.size();i++)
//        sources.push_back(generate_fdtd_source(fdtd_mode.sources[i],fdtd));
//    
//    AFP_TFSF afp_src;
//    afp_src.link(fdtd);
//    
//    sources.push_back(&afp_src);
//    
//    //Completion check
//    
//    int time_type=fdtd_mode.time_type;
//    int cc_step=fdtd_mode.cc_step;
//    double cc_lmin=fdtd_mode.cc_lmin,
//           cc_lmax=fdtd_mode.cc_lmax,
//           cc_coeff=fdtd_mode.cc_coeff,
//           cc_quant=fdtd_mode.cc_quant;
//    std::string cc_layout=fdtd_mode.cc_layout;
//    
//    CompletionSensor *cpl_sensor=nullptr;
//    
//    if(time_type==TIME_FT)
//    {
//        cpl_sensor=new CompletionSensor(cc_lmin,cc_lmax,cc_coeff,cc_quant,cc_layout);
//        cpl_sensor->link(fdtd);
//        sensors.push_back(cpl_sensor);
//    }
//    
//    int N_disp=std::max(Nt/100,1);
//    if(fdtd_mode.display_step>0) N_disp=fdtd_mode.display_step;
//        
//    ProgTimeDisp dspt(Nt);
//    
//    for(t=0;t<Nt;t++)
//    {
//        // Fields update
//        
//        // E-field
//        fdtd.update_E();
//        plog, t, " ", fdtd.Ex(0,0,Nz/2), "\n");
//        // E-field injection
//        for(unsigned int i=0;i<sources.size();i++)
//            sources[i]->inject_E(fdtd);
//        
//        // H-field
//        fdtd.update_H();
//        
//        // H-field injection
//        for(unsigned int i=0;i<sources.size();i++)
//            sources[i]->inject_H(fdtd);
//        
//        for(unsigned int i=0;i<sensors.size();i++)
//            sensors[i]->feed(fdtd);
//        
//        if(t%N_disp==0)
//        {
//            int vmode=0;
//            fdtd.draw(t,vmode,Nx/2,Ny/2,Nz/2);
//            //fdtd_aux.draw(t,vmode,0,0,Nz/2);
//        }
//        
//        if(time_type==TIME_FT && t%cc_step==0)
//        {
//            if(cpl_sensor->completion_check()) break;
//            
//            int Nt_est=std::min(Nt,cpl_sensor->estimate());
//            dspt.set_end(Nt_est);
//        }
//        
//        if(end_computation!=nullptr && *end_computation) break;
//        
//        ++dspt;
//    }
//    
//    for(unsigned int i=0;i<sensors.size();i++) sensors[i]->treat();
//    
//    for(unsigned int i=0;i<sensors.size();i++) delete sensors[i];
}
