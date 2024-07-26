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

#include <iostream>
#include <fstream>

#include <bitmap3.h>
#include <fieldblock_holder.h>
#include <sensors.h>
#include <string_tools.h>
#include <thread_utils.h>


extern const Imdouble Im;
extern std::ofstream plog;

FieldBlock::FieldBlock(int x1_,int x2_,
                       int y1_,int y2_,
                       int z1_,int z2_)
{
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]=new std::thread(&FieldBlock::threaded_computation,this,i);
        
        while(!threads_ready[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

FieldBlock::~FieldBlock()
{
    process_threads=false;
    alternator.signal_threads();
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]->join();
        delete threads[i];
    }
}

template<double (FDTD::*T)(int,int,int) const>
class thr_FB
{
    public:
        int start_x,start_y,start_z;
        int span_x,span_y,span_z;
        Imdouble tcoeff;
        Grid3<Imdouble> *EH;
        FDTD const *fdtd;
        
        void operator () ()
        {
            int i,j,k;
            
            Grid3<Imdouble> &EHr=*EH;
            
            for(i=0;i<span_x;i++) for(j=0;j<span_y;j++) for(k=0;k<span_z;k++)
            {
                EHr(i,j,k)+=(fdtd->*T)(i+start_x,j+start_y,k+start_z)*tcoeff;
            }
        }
        
        void set(int start_xi,int start_yi,int start_zi,
                 int span_xi,int span_yi,int span_zi,
                 Imdouble tcoeffi,Grid3<Imdouble> &EHi,FDTD const &fdtdi)
        {
            start_x=start_xi; start_y=start_yi; start_z=start_zi;
            span_x=span_xi; span_y=span_yi; span_z=span_zi;
            tcoeff=tcoeffi;
            EH=&EHi;
            fdtd=&fdtdi;
        }
};

void FieldBlock::deep_feed(FDTD const &fdtd_)
{
    fdtd=&fdtd_;
    
    std::unique_lock<std::mutex> lock(alternator.get_main_mutex());
    
    process_threads=true;
    
    alternator.signal_threads();
    alternator.main_wait_threads(lock);
    
    process_threads=false;
    
    if(step==0)
    {    
        int i,j,k;
        
        for(k=0;k<span3;k++){ for(j=0;j<span2;j++){ for(i=0;i<span1;i++)
        {
            mats(i,j,k)=fdtd->matsgrid(x1+i,y1+j,z1+k);
        }}}
    }
}

void FieldBlock::FT_Ex(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Ex(i,j,k)+=fdtd->local_Ex(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::FT_Ey(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Ey(i,j,k)+=fdtd->local_Ey(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::FT_Ez(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Ez(i,j,k)+=fdtd->local_Ez(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::FT_Hx(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Hx(i,j,k)+=fdtd->local_Hx(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::FT_Hy(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Hy(i,j,k)+=fdtd->local_Hy(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::FT_Hz(int i1,int i2,Imdouble const &tcoeff)
{
    int i,j,k;
    
    for(i=i1;i<i2;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        Hz(i,j,k)+=fdtd->local_Hz(i+x1,j+y1,k+z1)*tcoeff;
    }
}

void FieldBlock::initialize()
{
    span1=x2-x1;
    span2=y2-y1;
    span3=z2-z1;
    
    mats.init(span1,span2,span3,0);
    
    Ex.init(span1,span2,span3,0);
    Ey.init(span1,span2,span3,0);
    Ez.init(span1,span2,span3,0);
    Hx.init(span1,span2,span3,0);
    Hy.init(span1,span2,span3,0);
    Hz.init(span1,span2,span3,0);
}

void FieldBlock::threaded_computation(unsigned int ID)
{
    std::unique_lock<std::mutex> lock(alternator.get_thread_mutex(ID));
    
    threads_ready[ID]=true;
    
    alternator.thread_wait_ok(ID,lock);
    
    double w=2.0*Pi*c_light/lambda[0];
        
    while(process_threads)
    {
        Imdouble tcoeff=std::exp(w*step*Dt*Im);
        Imdouble tcoeff2=std::exp(w*(step+0.5)*Dt*Im);
        
        if(span1>static_cast<int>(Nthreads))
        {
            FT_Ex((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff);
            FT_Ey((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff);
            FT_Ez((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff);
            
            FT_Hx((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff2);
            FT_Hy((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff2);
            FT_Hz((ID*span1)/Nthreads,((ID+1)*span1)/Nthreads,tcoeff2);
        }
        else if(ID<Nthreads)
        {
            FT_Ex(ID,ID+1,tcoeff);
            FT_Ey(ID,ID+1,tcoeff);
            FT_Ez(ID,ID+1,tcoeff);
            
            FT_Hx(ID,ID+1,tcoeff2);
            FT_Hy(ID,ID+1,tcoeff2);
            FT_Hz(ID,ID+1,tcoeff2);
        }
        
        alternator.signal_main(ID);
        alternator.thread_wait_ok(ID,lock);
    }
}

void FieldBlock::treat()
{
    int i,j,k;
    
    std::string fname;
    fname=name;
    fname.append(".afblock");
    
    std::ofstream file(directory/fname,std::ios::out|std::ios::trunc|std::ios::binary);
    
    file.write(reinterpret_cast<char*>(&lambda[0]),sizeof(double));
    file.write(reinterpret_cast<char*>(&x1),sizeof(int));
    file.write(reinterpret_cast<char*>(&span1),sizeof(int));
    file.write(reinterpret_cast<char*>(&y1),sizeof(int));
    file.write(reinterpret_cast<char*>(&span2),sizeof(int));
    file.write(reinterpret_cast<char*>(&z1),sizeof(int));
    file.write(reinterpret_cast<char*>(&span3),sizeof(int));
    file.write(reinterpret_cast<char*>(&Dx),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dy),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dz),sizeof(double));
    
    double p_r,p_i;
    
    for(i=0;i<span1;i++) for(j=0;j<span2;j++) for(k=0;k<span3;k++)
    {
        p_r=std::real(Ex(i,j,k)); p_i=std::imag(Ex(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Ey(i,j,k)); p_i=std::imag(Ey(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Ez(i,j,k)); p_i=std::imag(Ez(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hx(i,j,k)); p_i=std::imag(Hx(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hy(i,j,k)); p_i=std::imag(Hy(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hz(i,j,k)); p_i=std::imag(Hz(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        file.write(reinterpret_cast<char*>(&mats(i,j,k)),sizeof(unsigned int));
    }
    
    file.close();
}

//###############
//   Fieldmap
//###############

FieldMap::FieldMap(int type_,
                   int x1_,int x2_,
                   int y1_,int y2_,
                   int z1_,int z2_)
    :mag_map(false),
     cumulative(false),
     fdtd_source(nullptr)
{
    set_type(type_);
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]=new std::thread(&FieldMap::threaded_computation,this,i);
        
        while(!threads_ready[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

FieldMap::~FieldMap()
{
    process_threads=false;
    alternator.signal_threads();
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]->join();
        delete threads[i];
    }
}

void FieldMap::deep_feed(FDTD const &fdtd)
{
    fdtd_source=&fdtd;
    
    std::unique_lock<std::mutex> lock(alternator.get_main_mutex());
    
    process_threads=true;
    
    alternator.signal_threads();
    alternator.main_wait_threads(lock);
    
    process_threads=false;
    
    if(step==0)
    {    
        int i,j;
        int a,b,c;
        
        for(j=0;j<span2;j++){ for(i=0;i<span1;i++)
        {
            a=b=c=0;
            
            if(type==NORMAL_X){ b=i; c=j; }
            else if(type==NORMAL_Y){ a=i; c=j; }
            else if(type==NORMAL_Z){ a=i; b=j; }
            
            mats(i,j)=fdtd.matsgrid(x1+a,y1+b,z1+c);
        }}
    }
}

void FieldMap::FT_compute(int j1,int j2)
{
    int i,j,k;
    int a,b,c;
    
    double lambda_b=lambda[0];
    double w=2.0*Pi*c_light/lambda_b;
    Imdouble tf_coeff=std::exp(w*step*Dt*Im);
    
    if(cumulative)
    {
        double Sx=0,Sy=0,Sz=0;
        
        int span3=0;
        if(type==NORMAL_X) span3=x2-x1;
        else if(type==NORMAL_Y) span3=y2-y1;
        else if(type==NORMAL_Z) span3=z2-z1;
                
        for(i=0;i<span1;i++){ for(j=j1;j<j2;j++)
        {
            Sx=Sy=Sz=0;
            a=b=c=0;
            
            for(k=0;k<span3;k++)
            {
                if(type==NORMAL_X){ a=k; b=i; c=j; }
                else if(type==NORMAL_Y){ a=i; b=k; c=j; }
                else if(type==NORMAL_Z){ a=i; b=j; c=k; }
                
                if(!mag_map)
                {
                    Sx+=fdtd_source->local_Ex(x1+a,y1+b,z1+c);
                    Sy+=fdtd_source->local_Ey(x1+a,y1+b,z1+c);
                    Sz+=fdtd_source->local_Ez(x1+a,y1+b,z1+c);
                }
                else
                {
                    Sx+=fdtd_source->local_Hx(x1+a,y1+b,z1+c);
                    Sy+=fdtd_source->local_Hy(x1+a,y1+b,z1+c);
                    Sz+=fdtd_source->local_Hz(x1+a,y1+b,z1+c);
                }
            }
            
            acc_Ex(i,j)+=tf_coeff*Sx;
            acc_Ey(i,j)+=tf_coeff*Sy;
            acc_Ez(i,j)+=tf_coeff*Sz;
        }}
    }
    else
    {
        for(j=j1;j<j2;j++){ for(i=0;i<span1;i++)
        {
            a=b=c=0;
            
            if(type==NORMAL_X){ b=i; c=j; }
            else if(type==NORMAL_Y){ a=i; c=j; }
            else if(type==NORMAL_Z){ a=i; b=j; }
            
            if(!mag_map)
            {
                acc_Ex(i,j)+=tf_coeff*fdtd_source->local_Ex(x1+a,y1+b,z1+c);
                acc_Ey(i,j)+=tf_coeff*fdtd_source->local_Ey(x1+a,y1+b,z1+c);
                acc_Ez(i,j)+=tf_coeff*fdtd_source->local_Ez(x1+a,y1+b,z1+c);
            }
            else
            {
                acc_Ex(i,j)+=tf_coeff*fdtd_source->local_Hx(x1+a,y1+b,z1+c);
                acc_Ey(i,j)+=tf_coeff*fdtd_source->local_Hy(x1+a,y1+b,z1+c);
                acc_Ez(i,j)+=tf_coeff*fdtd_source->local_Hz(x1+a,y1+b,z1+c);
            }
        }}
    }
}

void FieldMap::initialize()
{
    if(type==NORMAL_XM) type=NORMAL_X;
    else if(type==NORMAL_YM) type=NORMAL_Y;
    else if(type==NORMAL_ZM) type=NORMAL_Z;
    
    if(type==NORMAL_X){ span1=y2-y1; span2=z2-z1; }
    if(type==NORMAL_Y){ span1=x2-x1; span2=z2-z1; }
    if(type==NORMAL_Z){ span1=x2-x1; span2=y2-y1; }
    
    mats.init(span1,span2,0);
    acc_Ex.init(span1,span2,0);
    acc_Ey.init(span1,span2,0);
    acc_Ez.init(span1,span2,0);
}

void FieldMap::set_cumulative(bool c) { cumulative=c; }
void FieldMap::set_mag_map(bool c) { mag_map=c; }

void FieldMap::threaded_computation(unsigned int ID)
{
    std::unique_lock<std::mutex> lock(alternator.get_thread_mutex(ID));
    
    threads_ready[ID]=true;
    
    alternator.thread_wait_ok(ID,lock);
    
    while(process_threads)
    {
        if(span2>int(Nthreads)) FT_compute((ID*span2)/Nthreads,((ID+1)*span2)/Nthreads);
        else if(int(ID)<span2) FT_compute(ID,ID+1);
        
        alternator.signal_main(ID);
        alternator.thread_wait_ok(ID,lock);
    }
}

void FieldMap::treat()
{
    int i,j;
    
    Grid2<double> map(span1,span2,0);
    Grid2<double> mapx(span1,span2,0);
    Grid2<double> mapy(span1,span2,0);
    Grid2<double> mapz(span1,span2,0);
    
    std::ofstream file(directory/(name+"_fieldmap"),std::ios::out|std::ios::trunc|std::ios::binary);
    
    file.write(reinterpret_cast<char*>(&lambda[0]),sizeof(double));
    file.write(reinterpret_cast<char*>(&type),sizeof(int));
    
         if(type==NORMAL_X) file.write(reinterpret_cast<char*>(&y1),sizeof(int));
    else if(type==NORMAL_Y) file.write(reinterpret_cast<char*>(&x1),sizeof(int));
    else if(type==NORMAL_Z) file.write(reinterpret_cast<char*>(&x1),sizeof(int));
    
    file.write(reinterpret_cast<char*>(&span1),sizeof(int));
    
         if(type==NORMAL_X) file.write(reinterpret_cast<char*>(&z1),sizeof(int));
    else if(type==NORMAL_Y) file.write(reinterpret_cast<char*>(&z1),sizeof(int));
    else if(type==NORMAL_Z) file.write(reinterpret_cast<char*>(&y1),sizeof(int));
    
    file.write(reinterpret_cast<char*>(&span2),sizeof(int));
    file.write(reinterpret_cast<char*>(&Dx),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dy),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dz),sizeof(double));
    
    double p_r,p_i;
    
    for(i=0;i<span1;i++) for(j=0;j<span2;j++)
    {
        p_r=std::real(acc_Ex(i,j)); p_i=std::imag(acc_Ex(i,j));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(acc_Ey(i,j)); p_i=std::imag(acc_Ey(i,j));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(acc_Ez(i,j)); p_i=std::imag(acc_Ez(i,j));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
    }
    
    file.close();
    
    fmap_script((directory/name).generic_string(),E_FIELD);
    fmap_raw((directory/name).generic_string(),E_FIELD,acc_Ex,acc_Ey,acc_Ez);
    fmap_mats_raw((directory/name).generic_string(),mats);
    
    for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
    {
        using std::abs;
        using std::real;
        using std::imag;
        using std::sqrt;
        using std::norm;
        
        map(i,j)=sqrt(norm(acc_Ex(i,j))+norm(acc_Ey(i,j))+norm(acc_Ez(i,j)));
        mapx(i,j)=abs(acc_Ex(i,j));
        mapy(i,j)=abs(acc_Ey(i,j));
        mapz(i,j)=abs(acc_Ez(i,j));
    }}
    
    // Normalizations
    
    using std::abs;
    
    double c_norm=0;
    int c_norm_N=0;
    
    for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
    {
        c_norm+=mapx(i,j)*mapx(i,j); c_norm_N++;
        c_norm+=mapy(i,j)*mapy(i,j); c_norm_N++;
        c_norm+=mapz(i,j)*mapz(i,j); c_norm_N++;
    }}
        
    c_norm=std::sqrt(c_norm/c_norm_N);
    
    for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
    {
        map(i,j)=1.0-std::exp(-map(i,j)/c_norm/3.0);
        mapx(i,j)=1.0-std::exp(-mapx(i,j)/c_norm/3.0);
        mapy(i,j)=1.0-std::exp(-mapy(i,j)/c_norm/3.0);
        mapz(i,j)=1.0-std::exp(-mapz(i,j)/c_norm/3.0);
    }}
    
    // Writing
    
    Bitmap fmap(2*span1+4,4*span2+12);
    
    fmap.set_full(0,0,0);
    
    std::string fname;
    
    for(i=0;i<span1;i++){ for(j=0;j<span2;j++)
    {
        fmap.degra(i,j+3*span2+12,mapx(i,j),0,1);
        fmap.degra(i,j+2*span2+8,mapy(i,j),0,1);
        fmap.degra(i,j+span2+4,mapz(i,j),0,1);
        fmap.degra(i,j,map(i,j),0,1);
        
        fmap.degra_circ(i+span1+4,j+3*span2+12,std::arg(acc_Ex(i,j)),-Pi,Pi);
        fmap.degra_circ(i+span1+4,j+2*span2+8,std::arg(acc_Ey(i,j)),-Pi,Pi);
        fmap.degra_circ(i+span1+4,j+span2+4,std::arg(acc_Ez(i,j)),-Pi,Pi);
    }}
    
    fname=name; fname.append("_fieldmap.png");
    fmap.write(fname);
}

//###############
//   FieldMap2
//###############

FieldMap2::FieldMap2(int type_,
                       int x1_,int x2_,
                       int y1_,int y2_,
                       int z1_,int z2_)
    :SensorFieldHolder(type_,x1_,x2_,y1_,y2_,z1_,z2_,true)
{
}

FieldMap2::~FieldMap2()
{
    
}

void FieldMap2::link(FDTD const &fdtd, std::filesystem::path const &workingDirectory)
{
    SensorFieldHolder::link(fdtd, workingDirectory);
    
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
    
    mats.init(span1,span2);
    
    for(int i=0;i<span1;i++) for(int j=0;j<span2;j++)
    {
//             if(type==NORMAL_X || type==NORMAL_XM) mats(i,j)=fdtd.matsgrid(x1,i,j);
//        else if(type==NORMAL_Y || type==NORMAL_YM) mats(i,j)=fdtd.matsgrid(i,y1,j);
//        else if(type==NORMAL_Z || type==NORMAL_ZM) mats(i,j)=fdtd.matsgrid(i,j,z1);
    }
}
        
void FieldMap2::treat()
{
    std::ofstream file;
    
    file.open(directory/(name+"_fieldmap"),std::ios::out|std::ios::trunc|std::ios::binary);
    
    file<<0<<" "<<type<<" 1 1 1 1 1 1 1\n";
    for(int i=0;i<Nl;i++)
    {
        file<<lambda[i];
        if(i+1!=Nl) file<<" ";
    }
    file<<"\n";
    
    if(type!=NORMAL_X && type!=NORMAL_XM)
    {
        for(int i=x1;i<x2;i++)
        {
            file<<(xs_s+i)*Dx;
            if(i+1!=x2) file<<" ";
        }
        file<<"\n";
    }
    if(type!=NORMAL_Y && type!=NORMAL_YM)
    {
        for(int j=y1;j<y2;j++)
        {
            file<<(ys_s+j)*Dy;
            if(j+1!=y2) file<<" ";
        }
        file<<"\n";
    }
    if(type!=NORMAL_Z || type!=NORMAL_ZM)
    {
        for(int k=z1;k<z2;k++)
        {
            file<<(zs_s+k)*Dz;
            if(k+1!=z2) file<<" ";
        }
        file<<"\n";
    }
    
    std::vector<Grid3<Imdouble>*> fields(6);
    fields[0]=&sp_Ex; fields[1]=&sp_Ey; fields[2]=&sp_Ez;
    fields[3]=&sp_Hx; fields[4]=&sp_Hy; fields[5]=&sp_Hz;
    
    for(int j=0;j<span2;j++)
    {
        for(int i=0;i<span1;i++)
        {
            file<<0;
            if(i+1!=span1) file<<" ";
        }
        file<<"\n";
    }
    
    for(int l=0;l<Nl;l++)
    {
        for(int k=0;k<6;k++)
        {
            Grid3<Imdouble> &field=*(fields[k]);
            for(int j=0;j<span2;j++)
            {
                for(int i=0;i<span1;i++)
                {
                    file<<field(i,j,l).real()<<" "<<field(i,j,l).imag();
                    if(i+1!=span1) file<<" ";
                }
                file<<"\n";
            }
        }
    }
}

//###############
//   FieldPoint
//###############

FieldPoint::FieldPoint(int x1_,int y1_,int z1_)
{
    set_loc(x1_,x1_+1,y1_,y1_+1,z1_,z1_+1);
}

FieldPoint::~FieldPoint()
{
}

void FieldPoint::deep_feed(FDTD const &fdtd)
{
    double Ex=fdtd.local_Ex(x1,y1,z1);
    double Ey=fdtd.local_Ey(x1,y1,z1);
    double Ez=fdtd.local_Ez(x1,y1,z1);
    double E=std::sqrt(Ex*Ex+Ey*Ey+Ez*Ez);
    
    file<<step*Dt<<" "<<E<<" "<<Ex<<" "<<Ey<<" "<<Ez<<std::endl;
}

void FieldPoint::initialize()
{
    std::string fname(name);
    fname.append("_fieldpoint");
    
    file.open(directory/fname,std::ios::out|std::ios::trunc);
}

void FieldPoint::treat()
{
    file.close();
}
