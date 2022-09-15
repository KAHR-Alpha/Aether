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


#include <mathUT.h>
#include <string_tools.h>

#include <fstream>
#include <iomanip>
#include <mutex>
#include <thread>

extern const double Pi=4.0*std::atan(1.0);
extern const double Pio4=Pi/4.0;
extern const double dPi=2.0*Pi;
extern const Imdouble Im(0,1);

extern std::ofstream plog;

double exp_dec(double x)
{
    return std::exp(-x);
}

//double randp(double A)
//{
//    return A*std::rand()/(RAND_MAX-1.0);
//}
//
//double randp(double A,double B)
//{
//    return A+(B-A)*std::rand()/(RAND_MAX-1.0);
//}


std::mt19937 mte(std::time(0));
//std::mt19937 mte(1986);
std::normal_distribution<> rdn_gen(0,1);

unsigned long int randi()
{
    return mte();
}

double randp(double A)
{
    return A*mte()/(mte.max()-mte.min());
}

double randp(double A,std::mt19937 &gen)
{
    return A*gen()/(gen.max()-gen.min());
}

double randp(double A,double B)
{
    return A+(B-A)*mte()/(mte.max()-mte.min());
}

double randp_norm(double mean,double std_dev)
{
    return std_dev*rdn_gen(mte)+mean;
}

void seedp(int i)
{
    mte.seed(i);
}

//###############
//    Angle
//###############

Angle::Angle(double val_,bool degree_mode_)
    :degree_mode(degree_mode_), val(val_)
{
    if(degree_mode) val*=Pi/180.0;
}

Angle::Angle(Angle const &A)
    :degree_mode(A.degree_mode), val(A.val)
{
}

double Angle::degree() const { return val*180.0/Pi; }

void Angle::degree(double A) { val=A*Pi/180.0; }

void Angle::operator = (Angle const &A)
{
    degree_mode=A.degree_mode;
    val=A.val;
}

void Angle::operator = (double A)
{
    degree_mode=false;
    val=A;
}

Angle::operator double() const { return val; }

double Angle::radian() const { return val; }

//###############
//   Filename
//###############

Filename::Filename()
    :separator("/")
{
}

Filename::Filename(std::string const &fullpath_)
    :separator("/"),
     fullpath(fullpath_)
{
    breakdown();
}

Filename::Filename(Filename const &filename)
    :separator("/"),
     core(filename.core),
     extension(filename.extension),
     fullpath(filename.fullpath),
     path(filename.path)
{
}

void Filename::breakdown()
{
    check_separator();
    
    core.clear();
    extension.clear();
    path.clear();
    
    int pos1=-1;
    int pos2=fullpath.size();
    
    if(fullpath.rfind(separator)!=std::string::npos)
    {
        pos1=fullpath.rfind(separator);
        path=fullpath.substr(0,pos1+1);
    }
    
    if(fullpath.rfind(".")!=std::string::npos)
    {
        pos2=fullpath.rfind(".");
        extension=fullpath.substr(pos2);
    }
    
    core=fullpath.substr(pos1+1,pos2-(pos1+1));
}

void Filename::check_separator()
{
    if(fullpath.rfind("\\")!=std::string::npos) separator="\\";
    else separator="/";
}

void Filename::clear_core()
{
    core.clear();
    rebuild();
}

void Filename::clear_extension()
{
    extension.clear();
    rebuild();
}

void Filename::clear_path()
{
    path.clear();
    rebuild();
}

std::string Filename::get_core() const { return core; }
std::string Filename::get_core_ext() const
{
    std::string r(core);
    r.append(extension);
    return r;
}
std::string Filename::get_extension() const { return extension; }
std::string Filename::get_fullpath() const { return fullpath; }
std::string Filename::get_path() const { return path; }

void Filename::operator = (std::string const &fullpath_)
{
    fullpath=fullpath_;
    breakdown();
}

void Filename::operator = (Filename const &filename)
{
    core=filename.core;
    fullpath=filename.fullpath;
    extension=filename.extension;
    path=filename.path;
}

void Filename::rebuild()
{
    fullpath=path;
    fullpath.append(core);
    fullpath.append(extension);
    
    breakdown();
}

void Filename::set_core(std::string const &core_)
{
    core=core_;
    rebuild();
}

void Filename::set_extension(std::string const &extension_)
{
    extension=extension_;
    rebuild();
}

void Filename::set_fullpath(std::string const &fullpath_)
{
    fullpath=fullpath_;
    breakdown();
}

void Filename::set_path(std::string const &path_)
{
    path=path_;
    rebuild();
}

//###############
//   ProgDisp
//###############

ProgDisp::ProgDisp(int i)
    :Nstep(i), Nstep2(i-1.0)
{
    count=0;
    j=1;
    std::cout<<std::endl<<"0%   10   20   30   40   50   60   70   80   90   100%\n";
    std::cout<<"|----|----|----|----|----|----|----|----|----|----|"<<std::endl;
}

ProgDisp::ProgDisp(int i,std::string K)
    :Nstep(i), Nstep2(i-1.0)
{
    count=0;
    j=1;
    std::cout<<std::endl<<K<<std::endl<<
        "0%   10   20   30   40   50   60   70   80   90   100%\n";
    std::cout<<"|----|----|----|----|----|----|----|----|----|----|"<<std::endl;
}


void ProgDisp::operator ++()
{
    if(count/Nstep2<=j/51.0 && (count+1.0)/Nstep2>j/51.0)
    {
        std::cout<<"#"<<std::flush;
        j++;
    }
    
    count++;
    if(count==Nstep) std::cout<<std::endl;
}

void ProgDisp::reset(int i)
{
    Nstep=i;
    Nstep2=i-1.0;
    j=1;
    count=0;
    std::cout<<std::endl<<"0%   10   20   30   40   50   60   70   80   90   100%\n";
    std::cout<<"|----|----|----|----|----|----|----|----|----|----|"<<std::endl;
}

void ProgDisp::reset(int i,std::string K)
{
    Nstep=i;
    Nstep2=i-1.0;
    j=1;
    count=0;
    std::cout<<std::endl<<K<<std::endl<<
        "0%   10   20   30   40   50   60   70   80   90   100%\n";
    std::cout<<"|----|----|----|----|----|----|----|----|----|----|"<<std::endl;
}

//####################
//    ProgTimeDisp
//####################

ProgTimeDisp::ProgTimeDisp(int Nstep_,int skip_,std::string prf)
    :Nstep(Nstep_), count(0), skip(skip_), prefix(prf)
{
    start=clk.now();
    a=start;
    
    if(skip>=Nstep) skip=std::max(1,Nstep/100);
}

std::thread::id main_thread_id=std::this_thread::get_id();
std::mutex prog_disp_mutex;

void ProgTimeDisp::operator ++()
{
    std::unique_lock<std::mutex> lock(prog_disp_mutex);
    
    if(count%skip==0)
    {
        b=clk.now();
        
        std::chrono::duration<double> d_avg=b-a;
        std::chrono::duration<double> d_spent=b-start;
        
        double avg=d_avg.count()/static_cast<double>(skip);
        double spent=d_spent.count();
        double remain=avg*(Nstep-count);
        double tot=spent+remain;
        
        completion_percent=100.0*count/(Nstep-1.0);
        time_remaining=seconds_to_string(remain);
        time_spent=seconds_to_string(spent);
        time_total=seconds_to_string(tot);
        
        #ifndef GUI_ON
        int i;
        
        if(std::this_thread::get_id()!=main_thread_id)
            std::cout<<"thread "<<std::this_thread::get_id()<<" : ";
        
        if(prefix=="")
            std::cout<<completion_percent<<"% ";
        else
            std::cout<<prefix<<" "<<completion_percent<<"% ";
        
        std::cout<<time_spent<<" "
                 <<d_spent.count()/(count+1.0)<<" "<<avg<<" "
                 <<time_remaining<<" "
                 <<time_total<<" "
                 <<count<<" / "<<Nstep<<" ";
        
        int Nc=static_cast<int>(20.0*(count+0.5)/(Nstep-1.0));
        
        std::cout<<"[";
        for(i=0;i<Nc;i++) std::cout<<"#";
        for(i=0;i<20-Nc;i++) std::cout<<"-";
        std::cout<<"]"<<std::endl;
        #endif
        
        a=b;
    }
    
    count++;
}

void ProgTimeDisp::reset(int i,int j)
{
    Nstep=i;
    count=0;
    skip=j;
    
    if(skip>=Nstep) skip=std::max(1,Nstep/100);
    
    start=clk.now();
    a=start;
}

void ProgTimeDisp::set_end(int Nstep_) { Nstep=Nstep_; }

//###############
//###############

EvoS::EvoS(int Npi,int Nindi,int Nproi)
    :Np(Npi), Nind(Nindi), Npro(Nproi),
        neg(Np,true),
        pos(Np,true),
        coeff(Np,Nind,0),
        scl(Np,1.0),
        transl(Np,0),
        rating(Nind,0)
{
}

void EvoS::mutate(double m_coeff)
{
    int i,j,k;
    
    randgen(Npro,2*Npro);
    
    for(i=2*Npro;i<Nind;i++)
    {
        k=std::rand()%Npro;
        int a=std::rand()%4;
        
        for(j=0;j<Np;j++)
        {
            double c=randp(-1.0,1.0)*scl[j]*m_coeff;
            
            if(a==0) coeff(j,i)=coeff(j,k)+c;
            if(a==1) coeff(j,i)=coeff(j,k)+0.1*c;
            if(a==2) coeff(j,i)=coeff(j,k)*(1.0+c/(10.0*scl[j]));
            if(a==3) coeff(j,i)=coeff(j,k)*(1.0+c/(5.0*scl[j]));
        }
    }
}

void EvoS::randgen(double fact)
{
    for(int i=0;i<Np;i++)
    {
        for(int j=0;j<Nind;j++) coeff(i,j)=transl[i]+randp(-fact*scl[i]*neg[i],fact*scl[i]*pos[i]);
    }
}

void EvoS::randgen(int j1,int j2,double fact)
{
    for(int i=0;i<Np;i++)
    {
        for(int j=j1;j<j2;j++) coeff(i,j)=transl[i]+randp(-fact*scl[i]*neg[i],fact*scl[i]*pos[i]);
    }
}

void EvoS::randgen_spe(int k,double a,double b)
{
    for(int j=0;j<Nind;j++) coeff(k,j)=randp(a,b);
}

void EvoS::setsign(int i,bool n,bool p)
{
    neg[i]=n;
    pos[i]=p;
}

void EvoS::sort()
{
    int i,j,k;
    
    double tmp;
    
    for(i=0;i<Nind;i++)
    {
        for(j=i+1;j<Nind;j++)
        {
            if(rating[j]<rating[i])
            {
                tmp=rating[i];
                rating[i]=rating[j];
                rating[j]=tmp;
                
                for(k=0;k<Np;k++)
                {
                    tmp=coeff(k,i);
                    coeff(k,i)=coeff(k,j);
                    coeff(k,j)=tmp;
                }
            }
        }
    }
}

//###############
//###############

multi_cspline::multi_cspline(int N)
{
    if(N<1) N=1;
    
    Ns=N;
    if(Ns%2==0) Ns++;
    
    Np=Ns+1;

    xi.init(Np,0);
    yi.init(Np,0);
    di.init(Np,0);
    sa.init(Ns,0);
    sb.init(Ns,0);
    sc.init(Ns,0);
    sd.init(Ns,0);
}

void multi_cspline::auto_sct(Grid1<double> &x_data)
{
    int i,k;
    
    int Nd=x_data.L1();
    
    xi[0]=x_data[0];
    
    k=0;
    
    int Nd2=Nd-1;
    int Np1=static_cast<int>(Nd2/(Ns+0.0));
    int Np2=Np1+1;
    
    for(i=1;i<Np;i++)
    {
        if(Nd2%Np2!=0)
        {
            k+=Np1;
            Nd2-=Np1;
        }
        else
        {
            k+=Np2;
            Nd2-=Np2;
        }
        
        std::cout<<k<<std::endl;
        
        xi[i]=x_data[k];
    }
    
    set_boundaries(xi[0],xi[Np-1]);
}

void multi_cspline::auto_sct(Grid2<double> &data)
{
    int i,k;
    
    int Nd=data.L1();
    
    xi[0]=data(0,0);
    
    k=0;
    
    int Nd2=Nd-1;
    int Np1=static_cast<int>(Nd2/(Ns+0.0));
    int Np2=Np1+1;
    
    for(i=1;i<Np;i++)
    {
        if(Nd2%Np2!=0)
        {
            k+=Np1;
            Nd2-=Np1;
        }
        else
        {
            k+=Np2;
            Nd2-=Np2;
        }
        
        std::cout<<k<<std::endl;
        
        xi[i]=data(k,0);
    }
    
    set_boundaries(xi[0],xi[Np-1]);
}

double multi_cspline::deriv(double x)
{
    int i,j=0;
    
    if(x<=xs) j=0;
    else if(x>=xe) j=Ns-1;
    else
    {
        for(i=0;i<Ns;i++)
        {
            if(x>=xi[i] && x<xi[i+1])
            {
                return (3.0*sa[i]*x+2.0*sb[i])*x+sc[i];
            }
        }
    }
    
    return (3.0*sa[j]*x+2.0*sb[j])*x+sc[j];
}

double multi_cspline::eval(double x)
{
    int i,j=0;
    
    if(x<=xs) j=0;
    else if(x>=xe) j=Ns-1;
    else
    {
        for(i=0;i<Ns;i++)
        {
            if(x>=xi[i] && x<xi[i+1])
            {
                return ((sa[i]*x+sb[i])*x+sc[i])*x+sd[i];
            }
        }
    }
        
    return ((sa[j]*x+sb[j])*x+sc[j])*x+sd[j];
}

double multi_cspline::evalfit(Grid2<double> &data)
{
    int i,j,k;
    int Nd=data.L1();
    
    double R=0;
    double d;
    
    double x,y_c;
    
    j=0;
    
    for(i=0;i<Nd;i++)
    {
        x=data(i,0);
        
        if(x<=xs) j=0;
        else if(x>=xe) j=Ns-1;
        else
        {
            for(k=0;k<Ns;k++)
            {
                if(x>=xi[k] && x<xi[k+1])
                {
                    j=k;
                    break;
                }
            }
        }
        
        y_c=((sa[j]*x+sb[j])*x+sc[j])*x+sd[j];
        d=data(i,1)-y_c;
                        
        R+=d*d;
    }
                
    return R;
}

double multi_cspline::evalfit_sub(int k,Grid2<double> &data)
{
    int i;
    int Nd=data.L1();
    
    double R=0;
    double d,x,y_c;
                
    for(i=0;i<Nd;i++)
    {
        x=data(i,0);
        
        if(x>=xi[k] && x<xi[k+1])
        {
            y_c=((sa[k]*x+sb[k])*x+sc[k])*x+sd[k];
            d=data(i,1)-y_c;
            
            R+=d*d;
        }
    }
                
    return R;
}

double multi_cspline::fit(Grid2<double> &data)
{
    int k;
    
//    int Nd=data.L1();
//    set_boundaries(data(0,0),data(Nd-1,0));
//    
//    for(i=0;i<Np;i++) xi(i)=xs+(xe-xs)*i/(Np-1.0);
    
    auto_sct(data);
    
    for(k=0;k<Ns;k+=2)
    {
        fit_sub(k,data);
    }
    
    return evalfit(data);
}

void multi_cspline::fit_sub(int k,Grid2<double> &data)
{
    int i,j;
    
    int Nd=data.L1();
    for(i=0;i<Np;i++) xi[i]=xs+(xe-xs)*i/(Np-1.0);
                
    int Nind=100;
    int Ngen=1000;
    
    double y_max=data(0,1);
    double y_min=data(0,1);
    
    for(i=0;i<Nd;i++)
    {
        if(xi[k]<=data(i,0) && xi[k+1]>data(i,0))
        {
            y_max=data(i,1);
            y_min=data(i,1);
            break;
        }
    }
    
    for(i=0;i<Nd;i++)
    {
        if(xi[k]<=data(i,0) && xi[k+1]>data(i,0))
        {
            y_max=std::max(y_max,data(i,1));
            y_min=std::min(y_min,data(i,1));
        }
    }
    
    double y_span=y_max-y_min;
    double s_span=y_span/(xi[k+1]-xi[k]);
    
    EvoS evo(4,Nind,0.2*Nind);
    
    evo.scl[0]=y_span;
    evo.scl[1]=s_span;
    evo.scl[2]=y_span;
    evo.scl[3]=s_span;
    
    evo.randgen();
    evo.randgen_spe(0,y_min-y_span,y_max+y_span);
    evo.randgen_spe(2,y_min-y_span,y_max+y_span);
    
    for(i=0;i<Nind;i++)
    {
        yi[k]=evo.coeff(0,i);
        di[k]=evo.coeff(1,i);
        yi[k+1]=evo.coeff(2,i);
        di[k+1]=evo.coeff(3,i);
        
        recalc();
        evo.rating[i]=evalfit_sub(k,data);
    }
    evo.sort();
    
    for(i=0;i<Ngen;i++)
    {
        evo.mutate();
        
        for(j=0;j<Nind;j++)
        {
            yi[k]=evo.coeff(0,j);
            di[k]=evo.coeff(1,j);
            yi[k+1]=evo.coeff(2,j);
            di[k+1]=evo.coeff(3,j);
            
            recalc();
            evo.rating[j]=evalfit_sub(k,data);
        }
        evo.sort();
        
    }
    
    yi[k]=evo.coeff(0,0);
    di[k]=evo.coeff(1,0);
    yi[k+1]=evo.coeff(2,0);
    di[k+1]=evo.coeff(3,0);
    
    recalc();
}

int multi_cspline::get_Nseg()
{
    return Ns;
}

bool multi_cspline::in_seg(int k,double x)
{
    if(k<Ns-1)
    {
        if(xi[k]<=x && x<xi[k+1]) return 1;
    }
    else if(k==Ns-1)
    {
        if(xi[k]<=x && x<=xi[k+1]) return 1;
    }
    
    return 0;
}

void multi_cspline::insert_x(double x)
{
    int i;
    
    if(x<xs)
    {
        xi.insert(0,x);
        
        yi.insert(0,0);
        di.insert(0,0);
        sa.insert(0,0);
        sb.insert(0,0);
        sc.insert(0,0);
        sd.insert(0,0);
    }
    else if(x>xe)
    {
        xi.push_back(x);
        
        yi.push_back(0);
        di.push_back(0);
        sa.push_back(0);
        sb.push_back(0);
        sc.push_back(0);
        sd.push_back(0);
    }
    else
    {
        for(i=0;i<Ns;i++)
        {
            if(xi[i]<=x && x<xi[i+1])
            {
                xi.insert(i+1,x);
                
                yi.insert(i+1,0);
                di.insert(i+1,0);
                sa.insert(i+1,0);
                sb.insert(i+1,0);
                sc.insert(i+1,0);
                sd.insert(i+1,0);
                
                break;
            }
        }
    }
    
    Ns++;
    Np++;
    
    recalc();
}

void multi_cspline::recalc()
{
    int i;
    
    double D1;
    double D2;
    double D3;
    
    double x12;
    double x13;
                
    for(i=0;i<Ns;i++)
    {
        double &x1=xi[i];
        double &x2=xi[i+1];
        double &y1=yi[i];
        double &y2=yi[i+1];
        double &d1=di[i];
        double &d2=di[i+1];
        
        x12=x1*x1;
        x13=x1*x12;
        
        D1=x1-x2;
        D2=x12-x2*x2;
        D3=x13-x2*x2*x2;
        
        sa[i]=(2.0*D1*(y1-y2)-(D2-2.0*D1*x1)*(d1-d2)-2.0*D1*D1*d1)/(2.0*D1*(D3-3.0*D1*x12)-3.0*D2*(D2-2.0*D1*x1));
        sb[i]=1.0/(2.0*D1)*(d1-d2-3.0*sa[i]*D2);
        sc[i]=d1-3.0*sa[i]*x12-2*sb[i]*x1;
        sd[i]=y1-sa[i]*x13-sb[i]*x12-sc[i]*x1;
    }
}

void multi_cspline::seg_split(int k,int Nsp)
{
    int i;
    
    if(Nsp>1)
    {
        int Npt=Nsp-1;
        
        Grid1<double> txi(Npt,0);
        Grid1<double> tyi(Npt,0);
        Grid1<double> tdi(Npt,0);
                
        double h=(xi[k+1]-xi[k])/(Nsp+0.0);
        
        for(i=0;i<Npt;i++)
        {
            txi[i]=xi[k]+h*(1.0+i);
            if(Npt==1) txi[i]=(xi[k+1]+xi[k])/2.0;
            
            tyi[i]=eval(txi[i]);
            tdi[i]=deriv(txi[i]);
        }
        
        for(i=0;i<Npt;i++) insert_x(txi[i]);
        for(i=0;i<Npt;i++) yi[k+1+i]=tyi[i];
        for(i=0;i<Npt;i++) di[k+1+i]=tdi[i];
        
        recalc();
    }
}

void multi_cspline::set_boundaries(double a,double b)
{
    xs=a;
    xe=b;
    
    xi[0]=xs;
    xi[Np-1]=xe;
}

void multi_cspline::show()
{
    int i;
    
    for(i=0;i<Ns;i++)
    {
        std::cout<<xi[i]<<" "<<yi[i]<<" "<<di[i]<<" "
                    <<sa[i]<<" "<<sb[i]<<" "<<sc[i]<<" "<<sd[i]<<std::endl;
    }
    std::cout<<xi[Np-1]<<" "<<di[Np-1]<<std::endl<<std::endl;
}

//###############
//###############

void chk_msg(std::string k)
{
    std::cout<<k<<std::endl;
    std::system("pause");
}

void debug_msg(std::string k)
{
    #ifdef DEBUG_MSG_ON
    std::cout<<k<<std::endl;
    #endif
}

void degra(double val,double max,double &r,double &g,double &b)
{
    double R=max/5.0;
    
         if(val>=0 && val<R)
    {
        b=1.0;
        g=0.0;
        r=1.0-1.0*val/R;
    }
    else if(val>=R && val<2*R)
    {
         b=1.0;
         g=1.0*(val-R)/R;
         r=0.0;
    }
    else if(val>=2*R && val<3*R)
    {
         b=1.0-1.0*(val-2*R)/R;
         g=1.0;
         r=0.0;
    }
    else if(val>=3*R && val<4*R)
    {
         b=0.0;
         g=1.0;
         r=1.0*(val-3*R)/R;
    }
    else if(val>=4*R && val<5*R)
    {
         b=0.0;
         g=1.0-1.0*(val-4*R)/R;
         r=1.0;
    }
    else if(val>=max)
    {
         b=0.0;
         g=0.0;
         r=1.0;
    }
}

double factorial(int k)
{
    double r=1;
    for(int i=1;i<=k;i++) r*=i;
    return r;
}

double factorial_frac(int n,int k)
{
    double r=1;
    if(n>=k)
    {
        for(int i=k+1;i<=n;i++) r*=i;
        return r;
    }
    if(n<k)
    {
        for(int i=n+1;i<=k;i++) r*=i;
        return 1.0/r;
    }
    
    return 1.0;
}

void ascii_data_loader(std::string const &fname,std::vector<std::vector<double>> &data_holder,bool per_line)
{
    std::vector<std::string> strings_holder;
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    if(!file.is_open())
    {
        std::cout<<"Error: Could not open "<<fname<<std::endl;
        return;
    }
    
    while(!file.eof())
    {
        std::string str;
        std::getline(file,str,'\n');
        
        clean_data_string(str);
        if(str.size()!=0 && !contains_non_numerics(str))
        {
            strings_holder.push_back(str);
        }
    }
    
    file.close();
    
    int Nl=strings_holder.size();
    
    if(Nl>0)
    {
        int i,j;
        std::stringstream strm_init(strings_holder[0]);
        int Nc=0;
        double tmp;
        
        while(!strm_init.eof())
        {
            strm_init>>tmp;
            Nc++;
        }
        
        data_holder.resize(Nc);
        for(i=0;i<Nc;i++) data_holder[i].resize(Nl,0);
        
        for(i=0;i<Nl;i++)
        {
            std::stringstream strm(strings_holder[i]);
            
            for(j=0;j<Nc;j++)
            {
                strm>>data_holder[j][i];
            }
        }
    }
}

double binomial(int n,int k)
{
    return factorial_frac(n,k)/static_cast<double>(factorial(n-k));
}

double dichotomy(double a,double b,double (*f)(double),double eps)
{
    double c=a+b/2.0;
    
    double fa,fc;
    
    while(2.0*std::abs((b-a)/(a+b))>eps)
    {
        c=(a+b)/2.0;
        fa=(*f)(a);
//        fb=(*f)(b);
        fc=(*f)(c);
        
        if(fc/fa>0) a=c;
        else b=c;
    }
    
    return c;
}

int fcountlines(std::string k)
{
    std::ifstream file(k.c_str(),std::ios::in|std::ios::binary);
    std::string kb;
    
    int R=0;
    
    while(std::getline(file,kb))
    {
        R++;
    }
    file.close();
    
    return R;
    
}

Imdouble im_exp(double x) { return std::exp(x*Im); }
Imdouble im_exp(Imdouble const &x) { return std::exp(x*Im); }

std::string int_to_hex(int I,int s)
{
    int i;
    
    Grid1<unsigned char> tmp(s,0);
    
    for(i=0;i<s;i++)
    {
        tmp[i]=I%256;
        I=I/256;
    }
    
    std::stringstream K;
    
    for(i=0;i<s;i++) K<<tmp[i];
    
    return K.str();
}
bool is_prime(unsigned int N)
{
    for(unsigned int i=2;i<=N/2;i++) if(N%i==0) return false;
    return true;
}

bool Knk(int i,int j)
{
    if(i==j) return 1;
    return 0;
}

void linspace(std::vector<double> &v,double v_min,double v_max)
{
    unsigned int N=v.size();
    double v_span=v_max-v_min;
    
    for(unsigned int i=0;i<N;i++)
        v[i]=v_min+v_span*i/(N-1.0);
}

void lin_reg(std::vector<double> const &x,std::vector<double> const &y,double &a,double &b)
{
    unsigned int i;
    
    double sx=0,sy=0,
           sx2=0,sxy=0;
    
    unsigned int N=x.size();
    
    for(i=0;i<N;i++)
    {
        sx+=x[i];
        sy+=y[i];
        sx2+=x[i]*x[i];
        sxy+=x[i]*y[i];
    }
    
    double det=N*sx2-sx*sx;
    
    a=N*sxy-sx*sy;
    b=sx2*sy-sx*sxy;
    
    a/=det;
    b/=det;
}

void lin_reg_weight(std::vector<double> const &x,std::vector<double> const &y,
                    std::vector<double> const &w,double &a,double &b)
{
    unsigned int i;
    
    double sx=0,sy=0,sw=0,
           sx2=0,sxy=0;
    
    unsigned int N=x.size();
    
    for(i=0;i<N;i++)
    {
        sx+=w[i]*x[i];
        sy+=w[i]*y[i];
        sw+=w[i];
        sx2+=w[i]*x[i]*x[i];
        sxy+=w[i]*x[i]*y[i];
    }
    
    double det=sw*sx2-sx*sx;
    
    a=sw*sxy-sx*sy;
    b=sx2*sy-sx*sxy;
    
    a/=det;
    b/=det;
}

bool polyn2_real(double &x1,double &x2,double a,double b,double c)
{
    if(a==0)
    {
        if(b==0) return false;
        
        x1=x2=-c/b;
    }
    else
    {
        double delt=b*b-4.0*a*c;
        
        if(delt<0) return false;
        
        double srd=std::sqrt(delt);
        
        if(srd==b) x1=x2=-c/b;
        else
        {
            x1=(-b-srd)/(2.0*a);
            x2=(-b+srd)/(2.0*a);
        }
    }
    
    return true;
}

double polyn2_sol_neg(bool &sol_real,double a,double b,double c)
{
    double delt=4.0*a*c;
    
    if(delt<0)
    {
        sol_real=false;
        return 0;
    }
    
    sol_real=true;
    return (-b-std::sqrt(delt))/(2.0*a);
}

double polyn2_sol_pos(bool &sol_real,double a,double b,double c)
{
    double delt=4.0*a*c;
    
    if(delt<0)
    {
        sol_real=false;
        return 0;
    }
    
    sol_real=true;
    return (-b+std::sqrt(delt))/(2.0*a);
}

Imdouble polyn2_sol_im_neg(Imdouble a,Imdouble b,Imdouble c)
{
    Imdouble delt=4.0*a*c;
    return (-b-std::sqrt(delt))/(2.0*a);
}

Imdouble polyn2_sol_im_pos(Imdouble a,Imdouble b,Imdouble c)
{
    Imdouble delt=4.0*a*c;
    return (-b+std::sqrt(delt))/(2.0*a);
}

double restricted_vector_interp_linear(std::vector<double> const &x_data,
                                       std::vector<double> const &data,
                                       int N1,int N2,double x,bool monotonous)
{
    if(N2==N1) return data[N1];
    
    if(x<=x_data[N1]) return data[N1]+(data[N1+1]-data[N1])/(x_data[N1+1]-x_data[N1])*(x-x_data[N1]);
    else if(x>=x_data[N2]) return data[N2-1]+(data[N2]-data[N2-1])/(x_data[N2]-x_data[N2-1])*(x-x_data[N2-1]);
    else
    {
        int k=0;
        double u=0;
        
        if(monotonous)
        {
            u=(x-x_data[0])/(x_data[1]-x_data[0]);
            k=static_cast<int>(u);
            u-=k;
            
            if(k<N1) { k=N1; u=0; }
            else if(k>=N2) { k=N2-1; u=1.0; }
        }
        else vector_locate_linear(k,u,x_data,x);
        
        return (1.0-u)*data[k]+u*data[k+1];
    }
}

int nearest_integer(double x)
{
    if(x>=0) return static_cast<int>(x+0.5);
    else return static_cast<int>(x-0.5);
}

double nearest_integer_distance(double x)
{
    return x-nearest_integer(x);
}

int sgn(double x)
{
    if(x>=0) return 1;
    return -1;
}

double s_curve(double x,double xmin,double xmax)
{
    double u=(x-xmin)/(xmax-xmin);
    
    if(u<=0) return 0;
    else if(u>=1.0) return 1.0;
    else return u*u*(3.0-2.0*u);
}

double triangle_func(double x,double period,double y0,double y1,double phase)
{
    double xs=x/period+phase;
    xs-=std::floor(xs);
    
    if(xs<=0.5) return y0+2.0*xs*(y1-y0);
    else return y0+2.0*(y1-y0)*(1.0-xs);
}

double value_s_curve(double x,double xmin,double xmax,double value)
{
    return s_curve(x,xmin,xmax)-value;
}

double vector_interp_linear(std::vector<double> const &x_data,
                            std::vector<double> const &data,
                            double x,bool monotonous)
{
    int N=data.size();
    
    if(x<=x_data[0]) return data[0]+(data[1]-data[0])/(x_data[1]-x_data[0])*(x-x_data[0]);
    else if(x>=x_data[N-1]) return data[N-2]+(data[N-1]-data[N-2])/(x_data[N-1]-x_data[N-2])*(x-x_data[N-2]);
    else
    {
        int k=0;
        double u=0;
        
        if(monotonous)
        {
            u=(x-x_data[0])/(x_data[1]-x_data[0]);
            k=static_cast<int>(u);
            u-=k;
            
            if(k<0) { k=0; u=0; }
            else if(k>=N-1) { k=N-2; u=1.0; }
        }
        else vector_locate_linear(k,u,x_data,x);
        
        return (1.0-u)*data[k]+u*data[k+1];
    }
}

void vector_locate_linear(int &k,double &u,std::vector<double> const &data,double val)
{
    unsigned int N=data.size();
    
    if(val<=data[0]) 
    {
        k=0;
        u=0;
        return;
    }
    else if(val>=data[N-1])
    {
        k=N-2;
        u=1.0;
        return;
    }
    else
    {
        unsigned int i;
        for(i=0;i<N-1;i++)
            if(val>=data[i] && val<data[i+1]) { k=i; break; }
        
        u=(val-data[k])/(data[k+1]-data[k]);
        return;
    }
}

bool OutChk(double val,double l,double h,int Id)
{
    if(val<l || val>h) 
    {
        std::cout<<"Out of bound "<<Id<<" "<<val<<std::endl;
        return 1;
    }
    return 0;
}
