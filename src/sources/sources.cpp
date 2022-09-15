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

#include <enum_constants.h>
#include <phys_tools.h>
#include <sources.h>

extern const Imdouble Im;
extern const double Pi;

extern std::ofstream plog;

//######################
//   Source_generator
//######################

Source_generator::Source_generator()
    :type(-1),
     x1(0), x2(0),
     y1(0), y2(0),
     z1(0), z2(0),
     x1r(0), x2r(0),
     y1r(0), y2r(0),
     z1r(0), z2r(0),
     location_real(true),
     orientation(NORMAL_Z),
     polarization(TE),
     lambda_min(370e-9), lambda_max(850e-9),
     lambda_target(500e-9), nr_target(3.0), ni_target(0.0)
{
}

Source_generator::Source_generator(Source_generator const &src)
    :type(src.type),
     x1(src.x1), x2(src.x2),
     y1(src.y1), y2(src.y2),
     z1(src.z1), z2(src.z2),
     x1r(src.x1r), x2r(src.x2r),
     y1r(src.y1r), y2r(src.y2r),
     z1r(src.z1r), z2r(src.z2r),
     location_real(src.location_real),
     orientation(src.orientation),
     polarization(src.polarization),
     lambda_min(src.lambda_min), lambda_max(src.lambda_max),
     lambda_target(src.lambda_target), nr_target(src.nr_target), ni_target(src.ni_target)
{
}

void Source_generator::operator = (Source_generator const &src)
{
    type=src.type;
    
    x1=src.x1; x2=src.x2;
    y1=src.y1; y2=src.y2;
    z1=src.z1; z2=src.z2;
    
    x1r=src.x1r; x2r=src.x2r;
    y1r=src.y1r; y2r=src.y2r;
    z1r=src.z1r; z2r=src.z2r;
    
    location_real=src.location_real;
    orientation=src.orientation;
    polarization=src.polarization;
    
    lambda_min=src.lambda_min;
    lambda_max=src.lambda_max;
    
    lambda_target=src.lambda_target;
    nr_target=src.nr_target;
    ni_target=src.ni_target;
}

void Source_generator::to_discrete(double Dx,double Dy,double Dz)
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

void Source_generator::set_guided_target(double lambda_target_,double nr_target_,double ni_target_)
{
    lambda_target=lambda_target_;
    nr_target=nr_target_;
    ni_target=ni_target_;
}

void Source_generator::set_polarization(std::string polarization_)
{
    polarization=TE;
    
    std::string tmp[4]={"TM","Tm","tM","tm"};
    
    for(int i=0;i<4;i++)
    {
        if(polarization_==tmp[i]) polarization=TM;
    }
}

void Source_generator::set_spectrum(double lambda_min_,double lambda_max_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
}

//############
//   Source
//############

Source::Source()
    :step(0),
     process_threads(false),
     Nthreads(max_threads_number()),
     alternator(Nthreads),
     threads(Nthreads,nullptr), threads_ready(Nthreads,false)
{
}

Source::Source(int x1_,int x2_,
               int y1_,int y2_,
               int z1_,int z2_)
    :step(0),
     x1(x1_), x2(x2_),
     y1(y1_), y2(y2_),
     z1(z1_), z2(z2_),
     process_threads(false),
     Nthreads(max_threads_number()),
     alternator(Nthreads),
     threads(Nthreads,nullptr), threads_ready(Nthreads,false)
{
}

Source::~Source()
{
}

void Source::deep_inject_E(FDTD &fdtd)
{
}

void Source::deep_inject_H(FDTD &fdtd)
{
}

void Source::deep_link(FDTD const &fdtd)
{
}

void Source::expand_spectrum_gaussian(double pw_edge,double threshold,int Nl_)
{
    if(Nl_!=0) Nl=Nl_;
    
    lambda.resize(Nl);
    w.resize(Nl);
    Sp.resize(Nl);
    
    double w1=m_to_rad_Hz(lambda_max);
    double w2=m_to_rad_Hz(lambda_min);
    
    double w0=0.5*(w1+w2);
    double wh=w2-w0;
    
    chk_var(w1);
    chk_var(w2);
    chk_var(w0);
    chk_var(wh);
    
    double dw=(w2-w0)/std::sqrt(-std::log(pw_edge));
    
    chk_var(dw);
    
    double w_th=w0+dw*std::sqrt(-std::log(threshold));
    
    w2=w0+w_th;
    w1=w0-w_th;
    
    chk_var(w_th);
    chk_var(w1);
    chk_var(w2);
    
    std::exit(0);
}

void Source::expand_spectrum_S(double factor,int Nl_)
{
    if(Nl_!=0) Nl=Nl_;
    
    lambda.resize(Nl);
    w.resize(Nl);
    Sp.resize(Nl);
    
    double w1=m_to_rad_Hz(lambda_max);
    double w2=m_to_rad_Hz(lambda_min);
    
    double w0=0.5*(w1+w2);
    double wh=w2-w0;
    
    w1=std::max(0.0,w0-wh*(1.0+factor));
    w2=w0+wh*(1.0+factor);
    
    w0=0.5*(w1+w2);
    
    linspace(w,w1,w2);
    
    for(int l=0;l<Nl;l++)
    {
        lambda[l]=rad_Hz_to_m(w[l]);
        Sp[l]=s_curve(w[l],w1,w0)*s_curve(w[l],w2,w0);
    }
}

void Source::initialize()
{
}

void Source::inject_E(FDTD &fdtd)
{
    deep_inject_E(fdtd);
}

void Source::inject_H(FDTD &fdtd)
{
    deep_inject_H(fdtd);
    
    step+=1;
}

void Source::link(FDTD const &fdtd)
{
    Nx=fdtd.Nx;
    Ny=fdtd.Ny;
    Nz=fdtd.Nz;
    Nt=fdtd.Nt;
    
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
    
    deep_link(fdtd);
    
    initialize();
}

void Source::set_loc(int x1_,int x2_,
                     int y1_,int y2_,
                     int z1_,int z2_)
{
    x1=x1_; x2=x2_;
    y1=y1_; y2=y2_;
    z1=z1_; z2=z2_;
}

void Source::set_spectrum(double lambda_)
{
    Nl=1;
    
    lambda.resize(1,0);
    lambda[0]=lambda_;
}

void Source::set_spectrum(double lambda_min_,double lambda_max_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
}

void Source::set_spectrum(Grid1<double> const &lambda_)
{
    Nl=lambda_.L1();
    
    lambda.resize(Nl,0);
    
    for(int i=0;i<Nl;i++)
        lambda[i]=lambda_[i];
}

void Source::set_type(int type_)
{
    type=type_;
}

Source* generate_fdtd_source(Source_generator const &gen,FDTD const &fdtd)
{
    Source *src_out=0;
    
    if(gen.type==Source_generator::SOURCE_GEN_OSCILLATOR)
    {
        src_out=new Oscillator(gen.orientation,
                               gen.x1,gen.x2,
                               gen.y1,gen.y2,
                               gen.z1,gen.z2);
                              
        src_out->set_spectrum(gen.lambda_min,gen.lambda_max);
    }
    else if(gen.type==Source_generator::SOURCE_GEN_GUIDED_PLANAR)
    {
        src_out=new Guided_planar(gen.orientation,
                                  gen.x1,gen.x2,gen.y1,gen.y2,gen.z1,gen.z2,
                                  gen.polarization,gen.lambda_target,gen.nr_target,gen.ni_target);
        
        src_out->set_spectrum(gen.lambda_min,gen.lambda_max);
    }
    
    src_out->link(fdtd);
    
    return src_out;
}
