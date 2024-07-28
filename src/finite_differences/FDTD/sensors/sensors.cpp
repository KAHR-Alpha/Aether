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

#include <sensors.h>
#include <string_tools.h>

extern const Imdouble Im;


extern std::ofstream plog;

//######################
//   Sensor_generator
//######################

Sensor_generator::Sensor_generator()
    :type(Sensor_type::BOX_SPECTRAL_POYNTING), name("sensor"),
     x1(0), x2(0),
     y1(0), y2(0),
     z1(0), z2(0),
     x1r(0), x2r(0),
     y1r(0), y2r(0),
     z1r(0), z2r(0),
     location_real(true),
     orientation(NORMAL_Z),
     Nfx(50), Nfy(50),
     Nl(481), lambda_min(470e-9), lambda_max(850e-9),
     skip(1),
     disable_xm(false), disable_xp(false),
     disable_ym(false), disable_yp(false),
     disable_zm(false), disable_zp(false)
{
}

Sensor_generator::Sensor_generator(Sensor_generator const &sens)
    :type(sens.type), name(sens.name),
     x1(sens.x1), x2(sens.x2),
     y1(sens.y1), y2(sens.y2),
     z1(sens.z1), z2(sens.z2),
     x1r(sens.x1r), x2r(sens.x2r),
     y1r(sens.y1r), y2r(sens.y2r),
     z1r(sens.z1r), z2r(sens.z2r),
     location_real(sens.location_real),
     orientation(sens.orientation),
     Nl(sens.Nl), lambda_min(sens.lambda_min), lambda_max(sens.lambda_max),
     skip(sens.skip),
     disable_xm(sens.disable_xm), disable_xp(sens.disable_xp),
     disable_ym(sens.disable_ym), disable_yp(sens.disable_yp),
     disable_zm(sens.disable_zm), disable_zp(sens.disable_zp)
{
}

void Sensor_generator::disable_plane(std::string dir)
{
         if(is_x_neg(dir)) disable_xm=true;
    else if(is_x_pos(dir)) disable_xp=true;
    else if(is_y_neg(dir)) disable_ym=true;
    else if(is_y_pos(dir)) disable_yp=true;
    else if(is_z_neg(dir)) disable_zm=true;
    else if(is_z_pos(dir)) disable_zp=true;
}

void Sensor_generator::operator = (Sensor_generator const &sens)
{
    type=sens.type;
    name=sens.name;
    
    x1=sens.x1; x2=sens.x2;
    y1=sens.y1; y2=sens.y2;
    z1=sens.z1; z2=sens.z2;
    
    x1r=sens.x1r; x2r=sens.x2r;
    y1r=sens.y1r; y2r=sens.y2r;
    z1r=sens.z1r; z2r=sens.z2r;
    
    location_real=sens.location_real;
    orientation=sens.orientation;
    
    Nl=sens.Nl;
    lambda_min=sens.lambda_min;
    lambda_max=sens.lambda_max;
    
    skip=sens.skip;
    
    disable_xm=sens.disable_xm; disable_xp=sens.disable_xp;
    disable_ym=sens.disable_ym; disable_yp=sens.disable_yp;
    disable_zm=sens.disable_zm; disable_zp=sens.disable_zp;
}

void Sensor_generator::set_name(std::string name_)
{
    name=name_;
    std::cout<<"Setting the sensor name to "<<name<<std::endl;
}

void Sensor_generator::set_orientation(std::string orient_str)
{
         if(is_x_pos(orient_str)) orientation=NORMAL_X;
    else if(is_y_pos(orient_str)) orientation=NORMAL_Y;
    else if(is_z_pos(orient_str)) orientation=NORMAL_Z;
    else if(is_x_neg(orient_str)) orientation=NORMAL_XM;
    else if(is_y_neg(orient_str)) orientation=NORMAL_YM;
    else if(is_z_neg(orient_str)) orientation=NORMAL_ZM;
}

void Sensor_generator::set_resolution(int Nfx_,int Nfy_)
{
    Nfx=Nfx_;
    Nfy=Nfy_;
}

void Sensor_generator::set_skip(int skip_) { skip=skip_; }

void Sensor_generator::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
    std::cout<<"Setting the analysis spectrum between "
             <<add_unit_u(lambda_min)<<" and "<<add_unit_u(lambda_max)
             <<" with "<<Nl<<" points"<<std::endl;
}

void Sensor_generator::set_wavelength(double lambda_)
{
    Nl=1;
    lambda_min=lambda_max=lambda_;
    std::cout<<"Setting the analysis wavelength to "<<add_unit_u(lambda_min)<<std::endl;
}

void Sensor_generator::to_discrete(double Dx,double Dy,double Dz)
{
    if(location_real)
    {
        x1=nearest_integer(x1r/Dx);
        x2=nearest_integer(x2r/Dx);
        
        if(x2<=x1) x2=x1+1;
        
        y1=nearest_integer(y1r/Dy);
        y2=nearest_integer(y2r/Dy);
        
        if(y2<=y1) y2=y1+1;
        
        z1=nearest_integer(z1r/Dz);
        z2=nearest_integer(z2r/Dz);
        
        if(z2<=z1) z2=z1+1;
    }
}

//############
//   Sensor
//############

int Sensor::sensor_ID_next=0;

Sensor::Sensor()
    :step(0),
     silent(false),
     Ntap(0),
     tapering_E(1.0), tapering_H(1.0),
     reference_src(nullptr),
     name(""),
     disable_xm(false), disable_xp(false),
     disable_ym(false), disable_yp(false),
     disable_zm(false), disable_zp(false),
     process_threads(false),
     Nthreads(max_threads_number()),
     alternator(Nthreads),
     threads(Nthreads,nullptr), threads_ready(Nthreads,false)
{
    sensor_ID=sensor_ID_next;
    sensor_ID_next++;
}

Sensor::~Sensor()
{
}

void Sensor::initialize()
{
}

void Sensor::link(FDTD const &fdtd, std::filesystem::path const &workingDirectory)
{
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    Nt=fdtd.Nt;
    
    Ntap=fdtd.Ntap;
    
    Dx=fdtd.Dx;
    Dy=fdtd.Dy;
    Dz=fdtd.Dz;
    Dt=fdtd.Dt;
    
    xs_s=fdtd.xs_s; xs_e=fdtd.xs_e;
    ys_s=fdtd.ys_s; ys_e=fdtd.ys_e;
    zs_s=fdtd.zs_s; zs_e=fdtd.zs_e;
    
    x1+=xs_s; x2+=xs_s;
    x1=std::clamp(x1,0,Nx);
    x2=std::clamp(x2,0,Nx);
    
    y1+=ys_s; y2+=ys_s;
    y1=std::clamp(y1,0,Ny);
    y2=std::clamp(y2,0,Ny);
    
    z1+=zs_s; z2+=zs_s;
    z1=std::clamp(z1,0,Nz);
    z2=std::clamp(z2,0,Nz);
    
    directory = workingDirectory;
    Nthreads=fdtd.Nthreads;
    
    initialize();
}

void Sensor::feed(FDTD const &fdtd)
{
    deep_feed(fdtd);
    
    if(step>=Nt-Ntap)
    {
        tapering_E=s_curve(step,Nt-1.0,Nt-Ntap);
        tapering_H=s_curve(step+0.5,Nt-1.0,Nt-Ntap);
    }
        
    step+=1;
}

void Sensor::deep_feed(FDTD const &fdtd)
{
}

void Sensor::set_loc(int x1_,int x2_,
                     int y1_,int y2_,
                     int z1_,int z2_)
{
    x1=x1_; x2=x2_;
    y1=y1_; y2=y2_;
    z1=z1_; z2=z2_;
}

void Sensor::set_reference_source(Source *reference_src_)
{
    reference_src=reference_src_;
}

void Sensor::set_silent(bool silent_)
{
    silent=silent_;
}

void Sensor::set_spectrum(double lambda_)
{
    Nl=1;
    
    lambda.resize(1);
    lambda[0]=lambda_;
    
    sp_result.init(1,0);
    rsp_result.init(1,0);
}

void Sensor::set_spectrum(std::vector<double> const &lambda_)
{
    lambda=lambda_;
    Nl=lambda.size();
    
    sp_result.init(Nl,0);
    rsp_result.init(Nl,0);
}

void Sensor::set_spectrum(int Nl_,double lambda_min,double lambda_max)
{
    Nl=Nl_;
    
    lambda.resize(Nl);
    
    for(int i=0;i<Nl;i++)
        lambda[i]=lambda_min+(lambda_max-lambda_min)*i/(Nl-1.0);
    
    sp_result.init(Nl,0);
    rsp_result.init(Nl,0);
}

void Sensor::set_type(int type_)
{
    type=type_;
}

void Sensor::show_location()
{
    std::cout<<x1<<" "<<x2<<" "<<y1<<" "<<y2<<" "<<z1<<" "<<z2<<std::endl;
}

void Sensor::treat()
{
}

Sensor* generate_fdtd_sensor(Sensor_generator const &gen,
                             FDTD const &fdtd,
                             std::filesystem::path const &workingDirectory)
{
    Sensor *sens_out=0;
    
    if(gen.type==Sensor_type::BOX_POYNTING)
    {
        sens_out=new Box_Poynting(gen.x1,gen.x2,
                                  gen.y1,gen.y2,
                                  gen.z1,gen.z2);
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Sensor_type::BOX_SPECTRAL_POYNTING)
    {
        sens_out=new Box_Spect_Poynting(gen.x1,gen.x2,
                                        gen.y1,gen.y2,
                                        gen.z1,gen.z2);
        
        if(gen.disable_xm) sens_out->disable_xm=true;
        if(gen.disable_xp) sens_out->disable_xp=true;
        if(gen.disable_ym) sens_out->disable_ym=true;
        if(gen.disable_yp) sens_out->disable_yp=true;
        if(gen.disable_zm) sens_out->disable_zm=true;
        if(gen.disable_zp) sens_out->disable_zp=true;
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Sensor_type::DIFF_ORDERS)
    {
        sens_out=new DiffSensor(gen.orientation,
                                gen.x1,gen.x2,
                                gen.y1,gen.y2,
                                gen.z1,gen.z2);
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Sensor_type::FARFIELD)
    {
        sens_out=new FarFieldSensor(gen.x1,gen.x2,
                                    gen.y1,gen.y2,
                                    gen.z1,gen.z2,
                                    gen.Nfx,gen.Nfy);
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Sensor_type::FIELDBLOCK)
    {
        sens_out=new FieldBlock(gen.x1,gen.x2,
                                gen.y1,gen.y2,
                                gen.z1,gen.z2);
                                
        sens_out->set_spectrum(gen.lambda_min);
    }
    else if(gen.type==Sensor_type::FIELDMAP)
    {
        sens_out=new FieldMap(gen.orientation,
                              gen.x1,gen.x2,
                              gen.y1,gen.y2,
                              gen.z1,gen.z2);
        
        sens_out->set_spectrum(gen.lambda_min);
    }
    else if(gen.type==Sensor_type::FIELDMAP2)
    {
        sens_out=new FieldMap2(gen.orientation,
                              gen.x1,gen.x2,
                              gen.y1,gen.y2,
                              gen.z1,gen.z2);
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Sensor_type::FIELDPOINT)
    {
        sens_out=new FieldPoint(gen.x1,gen.y1,gen.z1);
        
        sens_out->set_spectrum(gen.lambda_min);
    }
    else if(gen.type==Sensor_type::MOVIE)
    {
        sens_out=new MovieSensor(gen.orientation,
                                 gen.x1,gen.x2,
                                 gen.y1,gen.y2,
                                 gen.z1,gen.z2,
                                 gen.skip);
    }
    else if(gen.type==Sensor_type::PLANAR_SPECTRAL_POYNTING)
    {
        sens_out=new Spect_Poynting(gen.orientation,
                                    gen.x1,gen.x2,
                                    gen.y1,gen.y2,
                                    gen.z1,gen.z2);
        
        sens_out->set_spectrum(gen.Nl,gen.lambda_min,gen.lambda_max);
    }
    else sens_out=new Sensor;
    
    sens_out->name=gen.name;
    sens_out->link(fdtd, workingDirectory);
    
    return sens_out;
}

//#######################
//   SensorFieldHolder
//#######################

SensorFieldHolder::SensorFieldHolder(int type_,
                                     int x1_,int x2_,
                                     int y1_,int y2_,
                                     int z1_,int z2_,
                                     bool interpolate_)
    :interpolate(interpolate_)
{
    step=0;
    
    set_type(type_);
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]=new std::thread(&SensorFieldHolder::threaded_computation,this,i);
        
        while(!threads_ready[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

SensorFieldHolder::~SensorFieldHolder()
{
    process_threads=false;
    alternator.signal_threads();
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]->join();
        delete threads[i];
    }
}

void SensorFieldHolder::FT_comp(int l1,int l2)
{
    int i,j,l;
    double w;
    Imdouble coeff_E,coeff_H;
    
    for(l=l1;l<l2;l++)
    {
        w=2.0*Pi*c_light/lambda[l];
        coeff_E=tapering_E*std::exp(w*step*Dt*Im);
        coeff_H=tapering_H*std::exp(w*(step+0.5)*Dt*Im);
        
        for(j=0;j<span2;j++){ for(i=0;i<span1;i++)
        {
            sp_Ex(i,j,l)+=coeff_E*t_Ex(i,j);
            sp_Ey(i,j,l)+=coeff_E*t_Ey(i,j);
            sp_Ez(i,j,l)+=coeff_E*t_Ez(i,j);
            
            sp_Hx(i,j,l)+=coeff_H*t_Hx(i,j);
            sp_Hy(i,j,l)+=coeff_H*t_Hy(i,j);
            sp_Hz(i,j,l)+=coeff_H*t_Hz(i,j);
        }}
    }
}

void SensorFieldHolder::deep_feed(FDTD const &fdtd)
{
    if(interpolate) update_t_interp(fdtd);
    else update_t(fdtd);
    
    std::unique_lock<std::mutex> lock(alternator.get_main_mutex());
    
    process_threads=true;
    
    alternator.signal_threads();
    alternator.main_wait_threads(lock);
    
    process_threads=false;
}

void SensorFieldHolder::initialize()
{
    if(type==NORMAL_X || type==NORMAL_XM)
    {
        span1=y2-y1;
        span2=z2-z1;
    }
    else if(type==NORMAL_Y || type==NORMAL_YM)
    {
        span1=x2-x1;
        span2=z2-z1;
    }
    else if(type==NORMAL_Z || type==NORMAL_ZM)
    {
        span1=x2-x1;
        span2=y2-y1;
    }
    
    t_Ex.init(span1,span2,0);
    t_Ey.init(span1,span2,0);
    t_Ez.init(span1,span2,0);
    t_Hx.init(span1,span2,0);
    t_Hy.init(span1,span2,0);
    t_Hz.init(span1,span2,0);
    
    sp_Ex.init(span1,span2,Nl,0);
    sp_Ey.init(span1,span2,Nl,0);
    sp_Ez.init(span1,span2,Nl,0);
    sp_Hx.init(span1,span2,Nl,0);
    sp_Hy.init(span1,span2,Nl,0);
    sp_Hz.init(span1,span2,Nl,0);
}

void SensorFieldHolder::link(FDTD const &fdtd, std::filesystem::path const &workingDirectory)
{
    Sensor::link(fdtd, workingDirectory);
}

void SensorFieldHolder::threaded_computation(unsigned int ID)
{
    std::unique_lock<std::mutex> lock(alternator.get_thread_mutex(ID));
    
    threads_ready[ID]=true;
    
    alternator.thread_wait_ok(ID,lock);
    
    while(process_threads)
    {
        if(Nl>int(Nthreads)) FT_comp((ID*Nl)/Nthreads,((ID+1)*Nl)/Nthreads);
        else if(int(ID)<Nl) FT_comp(ID,ID+1);
        
        alternator.signal_main(ID);
        alternator.thread_wait_ok(ID,lock);
    }
}

void SensorFieldHolder::treat()
{
}

void SensorFieldHolder::update_t(FDTD const &fdtd)
{
    int i,j,k;
    
    if(type==NORMAL_X || type==NORMAL_XM)
    {
        for(j=y1;j<y2;j++){ for(k=z1;k<z2;k++)
        {
            t_Ex(j-y1,k-z1)=fdtd.Ex(x1,j,k);
            t_Ey(j-y1,k-z1)=fdtd.Ey(x1,j,k);
            t_Ez(j-y1,k-z1)=fdtd.Ez(x1,j,k);
            
            t_Hx(j-y1,k-z1)=fdtd.Hx(x1,j,k);
            t_Hy(j-y1,k-z1)=fdtd.Hy(x1,j,k);
            t_Hz(j-y1,k-z1)=fdtd.Hz(x1,j,k);
        }}
    }
    else if(type==NORMAL_Y || type==NORMAL_YM)
    {
        for(i=x1;i<x2;i++){ for(k=z1;k<z2;k++)
        {
            t_Ex(i-x1,k-z1)=fdtd.Ex(i,y1,k);
            t_Ey(i-x1,k-z1)=fdtd.Ey(i,y1,k);
            t_Ez(i-x1,k-z1)=fdtd.Ez(i,y1,k);
            
            t_Hx(i-x1,k-z1)=fdtd.Hx(i,y1,k);
            t_Hy(i-x1,k-z1)=fdtd.Hy(i,y1,k);
            t_Hz(i-x1,k-z1)=fdtd.Hz(i,y1,k);
        }}
    }
    else if(type==NORMAL_Z || type==NORMAL_ZM)
    {
        for(i=x1;i<x2;i++){ for(j=y1;j<y2;j++)
        {
            t_Ex(i-x1,j-y1)=fdtd.Ex(i,j,z1);
            t_Ey(i-x1,j-y1)=fdtd.Ey(i,j,z1);
            t_Ez(i-x1,j-y1)=fdtd.Ez(i,j,z1);
            
            t_Hx(i-x1,j-y1)=fdtd.Hx(i,j,z1);
            t_Hy(i-x1,j-y1)=fdtd.Hy(i,j,z1);
            t_Hz(i-x1,j-y1)=fdtd.Hz(i,j,z1);
        }}
    }
}

void SensorFieldHolder::update_t_interp(FDTD const &fdtd)
{
    int i,j,k;
    
    if(type==NORMAL_X || type==NORMAL_XM)
    {
        for(j=y1;j<y2;j++){ for(k=z1;k<z2;k++)
        {
            t_Ex(j-y1,k-z1)=fdtd.local_Ex(x1,j,k);
            t_Ey(j-y1,k-z1)=fdtd.local_Ey(x1,j,k);
            t_Ez(j-y1,k-z1)=fdtd.local_Ez(x1,j,k);
            
            t_Hx(j-y1,k-z1)=fdtd.local_Hx(x1,j,k);
            t_Hy(j-y1,k-z1)=fdtd.local_Hy(x1,j,k);
            t_Hz(j-y1,k-z1)=fdtd.local_Hz(x1,j,k);
        }}
    }
    else if(type==NORMAL_Y || type==NORMAL_YM)
    {
        for(i=x1;i<x2;i++){ for(k=z1;k<z2;k++)
        {
            t_Ex(i-x1,k-z1)=fdtd.local_Ex(i,y1,k);
            t_Ey(i-x1,k-z1)=fdtd.local_Ey(i,y1,k);
            t_Ez(i-x1,k-z1)=fdtd.local_Ez(i,y1,k);
            
            t_Hx(i-x1,k-z1)=fdtd.local_Hx(i,y1,k);
            t_Hy(i-x1,k-z1)=fdtd.local_Hy(i,y1,k);
            t_Hz(i-x1,k-z1)=fdtd.local_Hz(i,y1,k);
        }}
    }
    else if(type==NORMAL_Z || type==NORMAL_ZM)
    {
        for(i=x1;i<x2;i++){ for(j=y1;j<y2;j++)
        {
            t_Ex(i-x1,j-y1)=fdtd.local_Ex(i,j,z1);
            t_Ey(i-x1,j-y1)=fdtd.local_Ey(i,j,z1);
            t_Ez(i-x1,j-y1)=fdtd.local_Ez(i,j,z1);
            
            t_Hx(i-x1,j-y1)=fdtd.local_Hx(i,j,z1);
            t_Hy(i-x1,j-y1)=fdtd.local_Hy(i,j,z1);
            t_Hz(i-x1,j-y1)=fdtd.local_Hz(i,j,z1);
        }}
    }
}
