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


#ifndef MATHUT_H
#define MATHUT_H

#include <cmath>
#include <complex>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <grid.h>

#define TEST_MODE
#define DEBUG_MSG_ON

class Vector2;
class Vector3;

double exp_dec(double x);
double binomial(int,int);
double factorial(int);
double factorial_frac(int,int);

typedef std::complex<double> Imdouble;
typedef std::complex<long double> Imdouble_l;

unsigned long int randi();
double randp(double A=1);
double randp(double A,std::mt19937 &gen);
double randp(double A,double B);
double randp_norm(double mean,double std_dev);
void seedp(int i);

class Angle
{
    public:
        bool degree_mode;
        double val;
        
        Angle(double val=0,bool degree_mode=false);
        Angle(Angle const &A);
        
        double degree() const;
        void degree(double A);
        void operator = (Angle const &A);
        void operator = (double A);
        operator double () const;
        double radian() const;
};

class [[deprecated]] Filename
{
    private:
        std::string separator;
        std::string core,extension,fullpath,path;
        
        void breakdown();
        void check_separator();
        void rebuild();
        
    public:
        Filename();
        Filename(std::string const &fullpath);
        Filename(Filename const &filename);
        
        void clear_core();
        void clear_extension();
        void clear_path();
        
        std::string get_core() const;
        std::string get_core_ext() const;
        std::string get_extension() const;
        std::string get_fullpath() const;
        std::string get_path() const;
        
        void operator = (std::string const &fullpath);
        void operator = (Filename const &filename);
        
        void set_core(std::string const &core);
        void set_extension(std::string const &extension);
        void set_fullpath(std::string const &fullpath);
        void set_path(std::string const &path);
};

#define Degree(x) Angle(x,true)
#define Radian(x) Angle(x)

// for displaying a progress bar
class ProgDisp
{
    public:
        int Nstep,count,j;
        double Nstep2;
        
        ProgDisp(int);
        ProgDisp(int,std::string);
        
        void operator ++ ();
        void reset(int);
        void reset(int,std::string);

};

class ProgTimeDisp
{
    public:
        int Nstep,count,skip;
        double completion_percent;
        std::string prefix;
        std::string time_spent,time_remaining,time_total;
        std::chrono::high_resolution_clock clk;
        std::chrono::high_resolution_clock::time_point a,b,start;
        
        ProgTimeDisp(int Nstep,int skip=100,std::string prefix="");
        
        void operator ++ ();
        void reset(int Nstep,int skip=100);
        void set_end(int Nstep);
};

class Timer
{
    public:
        std::chrono::high_resolution_clock clk;
        std::chrono::high_resolution_clock::time_point a,b;
        std::chrono::duration<double> d;
        
        void tic() { a=clk.now(); }
        void toc()
        {
            b=clk.now();
            d+=b-a;
        }
        
        double operator() ()
        {
            return d.count();
        }
};

template<class T>
class CSphere
{
    public:
        int Np;
        Grid3<T> data;
        
        CSphere(int Npi)
            :Np(Npi), data(Np,Np,6)
        {
        }
        
        T& operator () (int i,int j,int f)
        {
            return data(i,j,f);
        }
        
        
};

template<double T(double)>
class LU_table
{
    public:
        int Nx;
        double xs,xe,sx,spanx;
        Grid1<double> table;
        
        LU_table(double xs_i,double xe_i,int N_i)
            :Nx(N_i),
            xs(xs_i),
            xe(xe_i),
            table(Nx)
        {
            int i;
            
            spanx=xe-xs;
            sx=spanx/(Nx-1.0);
            
            for(i=0;i<Nx;i++)
            {
                table(i)=T(xs+i*sx);
            }
        }
        
        double cyclic(double x)
        {
            int j;
            j=static_cast<int>((x-xs)/spanx);
            if(x<0) j-=1;
            x-=j*spanx;
            
            return table.lin_interp(x,xs,xe);
        }
        
        double exact(double x)
        {
            return T(x);
        }
        
        double operator() (double x)
        {
            if(x<xs) return table(0);
            if(x>=xe) return table(Nx-1);
            
            double u=(x-xs)/sx;
            int i=static_cast<int>(u);
            u-=i;
            return table(i)*(1.0-u)+table(i+1)*u;
        }
};

template<typename T>
class LU_table_nl
{
    public:
        int Np;
        Grid1<double> x;
        Grid1<T> y;
        
        LU_table_nl(int Np_i)
            :Np(Np_i)
        {
            x.init(Np);
            y.init(Np);
        }
        
        LU_table_nl(Grid1<double> &x_in,Grid1<T> &y_in)
        {
            Np=x_in.L1();
            
            for(int i=0;i<Np;i++)
            {
                x(i)=x_in(i);
                y(i)=y_in(i);
            }
        }
        
        double operator () (double x_query)
        {
            int i;
            double u;
            
            if(x_query<x(0)) return y(0);
            else if(x_query>=x(Np-1)) return y(Np-1);
            else
            {
                for(i=0;i<Np-1;i++)
                {
                    if(x_query<x(i+1) && x_query>=x(i))
                    {
                        u=(x_query-x(i))/(x(i+1)-x(i));
                        
                        return (1.0-u)*y(i)+u*y(i+1);
                    }
                }
            }
            
            return 0;
        }
        
        void set(int i,double xi,const T &yi)
        {
            x(i)=xi;
            y(i)=yi;
        }
};

//###############
//###############

class EvoS
{
    private:
        int Np;
        int Nind;
        int Npro;
        
        Grid1<bool> neg;
        Grid1<bool> pos;
        
    public:
        Grid2<double> coeff;
        Grid1<double> scl;
        Grid1<double> transl;
        Grid1<double> rating;
        
        EvoS(int Np,int Nind,int Npro);
        
        void setsign(int,bool,bool);
        void mutate(double m_coeff=1.0);
        void randgen(double fact=1.0);
        void randgen(int j1,int j2,double fact=1.0);
        void randgen_spe(int,double,double);
        void sort();
};

template<typename T>
class Averager
{
    private:
        int N,N_max;
        Grid1<T> data;
        T avg;
        
        void average_calc()
        {
            avg=0;
            for(int i=0;i<N;i++) avg+=data(i);
            avg/=static_cast<double>(N);
        }
        
        void push(T const &val)
        {
            N=std::min(N_max,N+1);
            
            for(int i=0;i<N-1;i++) data(i)=data(i+1);
            data(N-1)=val;
        }
        
    public:
        Averager(int N_)
            :N(0), N_max(N_), data(N_max,0), avg(0)
        {
            for(int i=0;i<N_max;i++) data[i]=0;
        }
        
        T average() { return avg; }
        
        bool convergence(double epsilon)
        {
                 if(dispersion()/std::abs(avg)<=epsilon) return true;
            else if(avg==0) return true;
            
            return false;
        }
        
        double dispersion()
        {
            T sum2=0;
            for(int i=0;i<N;i++) sum2+=data[i]*data[i];
            sum2/=static_cast<double>(N);
            
            return std::sqrt(static_cast<double>(sum2-avg*avg));
        }
        
        T max()
        {
            T r=data(0);
            for(int i=1;i<N;i++) r=std::max(r,data(i));
            return r;
        }
        
        T min()
        {
            T r=data(0);
            for(int i=1;i<N;i++) r=std::min(r,data(i));
            return r;
        }
        
        T operator()() { return avg; }
        
        T operator()(T const &val)
        {
            push(val);
            
            average_calc();
            
            return avg;
        }
        
        T span() { return max()-min(); }
};

class multi_cspline
{
    public:
        int Ns,Np;
        
        double xs,xe;
        double Sxw;
        
        Grid1<double> xi,yi,di;
        Grid1<double> sa,sb,sc,sd;
        
        multi_cspline(int N);
        
        void auto_sct(Grid1<double> &x_data);
        void auto_sct(Grid2<double> &data);
        double deriv(double x);
        double eval(double x);
        double evalfit(Grid2<double> &data);
        double evalfit_sub(int k,Grid2<double> &data);
        double fit(Grid2<double> &data);
        void fit_sub(int,Grid2<double> &data);
        int get_Nseg();
        bool in_seg(int,double);
        void insert_x(double x);
        void recalc();
        void seg_split(int,int);
        void set_boundaries(double a,double b);
        void show();
};

class poly_line
{
    public:
        Grid1<double> x_v;
        Grid1<double> y_v;
        Grid1<int> iv1;
        Grid1<int> iv2;
};

//###############
//###############

template<typename T>
double adaptive_trapeze_integral(int &N,T &t,double x_min,double x_max,double basic_precision=4.0,int max_it=20)
{
    if(x_min==x_max) { N=1; return 0; }
    
    int i;
            
    bool stable=false;
    
    double precision=std::pow(10.0,-basic_precision);
    
    N=5;
    double dx=(x_max-x_min)/(N-1.0);
    
    double I=-(t(x_min)+t(x_max))/2.0;
    for(i=0;i<N;i++) { I+=t(x_min+i*dx); }
    I=dx*I;
    
    int N_it=0;
    
    while(!stable && N_it<=max_it)
    {
        double S=0;
        for(i=0;i<N-1;i++) { S+=t(x_min+dx/2.0+i*dx); }
        double I2=I/2.0+dx/2.0*S;
        
        if(std::abs((I2-I)/I)<=precision) stable=true;
        
        N=2*N-1;
        dx=(x_max-x_min)/(N-1.0);
        
        I=I2;
        N_it++;
    }
    
    return I;
}

template<typename... f_params>
double dichotomy(double a,double b,double (*f)(double,f_params...),f_params... params,double eps)
{
    double c=(a+b)/2.0;
    
    double fa,fc;
    
    while(2.0*std::abs((b-a)/(a+b))>eps)
    {
        c=(a+b)/2.0;
        fa=(*f)(a,params...);
        fc=(*f)(c,params...);
        
        if(fc/fa>0) a=c;
        else b=c;
    }
    
    return c;
}

template<typename T>
bool find_in_vector(std::vector<T> const &V,T const &val)
{
    for(unsigned int i=0;i<V.size();i++)
        if(V[i]==val) return true;
    
    return false;
}

template<typename T>
bool is_in(T x,T a,T b)
{
    if(x>=a && x<=b) return 1;
    return 0;
}

template<typename T>
bool is_in_b(T x,T a,T b)
{
    if(x>a && x<b) return 1;
    return 0;
}

template<typename T>
bool is_in_l(T x,T a,T b)
{
    if(x>a && x<=b) return 1;
    return 0;
}

template<typename T>
bool is_in_r(T x,T a,T b)
{
    if(x>=a && x<b) return 1;
    return 0;
}

template<typename T>
T modulus(T val,T modulus_value)
{
    while(val<0) val+=modulus_value;
    while(val>=modulus_value) val-=modulus_value;
    
    return val;
}

template<typename T>
T neg_filter(T A)
{
    if(A<=0) return A;
    return 0;
}

template<typename T>
void null_function(T A)
{
}

template<typename T>
T pos_filter(T A)
{
    if(A>=0) return A;
    return 0;
}

template<typename T> // Variadic Min
T var_min(T a,T b)
{
    return std::min(a,b);
}

template<typename T,typename... Varg>
T var_min(T a,Varg... b)
{
    return std::min(a,var_min(b...));
}

template<typename T> // Variadic Positive Min Above Threshold
T var_min_pos(double threshold,T a,T b)
{
    if(a<threshold || b<threshold) return std::max(a,b);
    else return std::min(a,b);
}

template<typename T,typename... Varg>
T var_min_pos(double threshold,T a,Varg... b)
{
    return var_min_pos(threshold,a,var_min_pos(threshold,b...));
}

template<typename T> // Variadic Max
T var_max(T a,T b)
{
    return std::max(a,b);
}

template<typename T,typename... Varg>
T var_max(T a,Varg... b)
{
    return std::max(a,var_max(b...));
}

template<typename T>
std::complex<T> asin(std::complex<T> Z)
{
    T X=std::real(Z);
    T Y=std::imag(Z);
    
    T Y2=Y*Y;
    T sr1=std::sqrt((X+1)*(X+1)+Y2);
    T sr2=std::sqrt((X-1)*(X-1)+Y2);
    
    T a=(sr1+sr2)/2.0;
    T b=(sr1-sr2)/2.0;
    
    static std::complex<T> tim(0,1);
    
    std::complex<T> R=std::asin(b)+std::log(a+std::sqrt(a*a-1))*tim;
    
    if(Y>=0) return R;
    else return std::conj(R);
}

//###############
//###############

#define chk_dbg() std::cout<<__FILE__<<" "<<__LINE__<<std::endl;
#define chk_msg_s(N) chk_msg(#N,N)
#define chk_msg_sc(N) chk_msg_c(#N,N)
#define chk_var(N) chk_var_f(#N,N)

void ascii_data_loader(std::string const &fname,std::vector<std::vector<double>> &data_holder,bool per_line=false);

void chk_msg(std::string k);
template<typename T> void chk_msg(T const &k)
{
    std::cout<<"Check: "<<k<<std::endl;
    std::system("pause");
}
template<typename T> void chk_msg(std::string ks,T const &k)
{
    std::cout<<"Check "<<ks<<": "<<k<<std::endl;
    std::system("pause");
}
template<typename T> void chk_msg_c(std::string ks,T const &k)
{
    std::cout<<"Check "<<ks<<": "<<k<<std::endl;
}
template<typename T> void msg_n(std::string ks,T const &k)
{
    std::cout<<ks<<": "<<k<<std::endl;
}
template<typename T> void chk_var_f(std::string ks,T const &k)
{
    #ifdef DEV_INFO
    std::cout<<ks<<": "<<k<<std::endl<<std::flush;
    #endif
}

void debug_msg(std::string k);
void degra(double val,double max,double &r,double &g,double &b);
double dichotomy(double,double,double (*f)(double),double eps=1e-6);
int fcountlines(std::string k);
Imdouble im_exp(double x);
Imdouble im_exp(Imdouble const &x);
std::string int_to_hex(int I,int s);
bool is_prime(unsigned int N);
bool Knk(int i,int j);
void lin_reg(std::vector<double> const &x,std::vector<double> const &y,double &a,double &b);
void lin_reg_weight(std::vector<double> const &x,std::vector<double> const &y,
                    std::vector<double> const &w,double &a,double &b);
void linspace(std::vector<double> &v,double v_min,double v_max);
bool polyn2_real(double &x1,double &x2,double a,double b,double c);
double polyn2_sol_neg(bool &sol_real,double a,double b,double c);
double polyn2_sol_pos(bool &sol_real,double a,double b,double c);
Imdouble polyn2_sol_neg(Imdouble a,Imdouble b,Imdouble c);
Imdouble polyn2_sol_pos(Imdouble a,Imdouble b,Imdouble c);
double s_curve(double x,double xmin,double xmax);
int sgn(double x);
int nearest_integer(double x);
double nearest_integer_distance(double x);
double restricted_vector_interp_linear(std::vector<double> const &x_data,
                                       std::vector<double> const &data,
                                       int N1,int N2,double x,bool monotonous);
double triangle_func(double x,double period,double y0,double y1,double phase=0);
double value_s_curve(double x,double xmin,double xmax,double value);

template<typename T>
bool vector_contains(std::vector<T> const &V,T const &P)
{
    for(std::size_t i=0;i<V.size();i++) if(V[i]==P) return true;
    
    return false;
}

template<typename T>
T vector_min(std::vector<T> const &V)
{
    T R=V[0];
    
    for(std::size_t i=1;i<V.size();i++)
        R=std::min(R,V[i]);
    
    return R;
}

template<typename T>
T vector_max(std::vector<T> const &V)
{
    T R=V[0];
    
    for(std::size_t i=1;i<V.size();i++)
        R=std::max(R,V[i]);
    
    return R;
}

double vector_interp_linear(std::vector<double> const &x_data,
                            std::vector<double> const &data,
                            double x,bool monotonous=false);
                            
template<typename T>
std::size_t vector_locate(std::vector<T> const &V,T const &P,std::size_t start=0)
{
    for(std::size_t i=start;i<V.size();i++) if(V[i]==P) return i;
    
    return 0;
}
                            
template<typename T>
std::size_t vector_locate(bool &found,std::vector<T> const &V,T const &P,std::size_t start=0)
{
    found=false;
    
    for(std::size_t i=start;i<V.size();i++)
        if(V[i]==P)
        {
            found=true;
            return i;
        }
    
    return 0;
}
                           
void vector_locate_linear(int &k,double &u,std::vector<double> const &data,double val);

template<typename T>
void vector_push_single(std::vector<T> &V,T const &P)
{
    for(std::size_t i=0;i<V.size();i++) if(V[i]==P) return;
    
    V.push_back(P);
}

bool OutChk(double,double,double,int);

#endif
