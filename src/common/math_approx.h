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

#ifndef MATH_APPROX_INCLUDED
#define MATH_APPROX_INCLUDED

#include <grid.h>
#include <logger.h>
#include <mathUT.h>
#include <Eigen/Eigen>

enum
{
    CSPLINE_NATURAL,    //null second derivative
    CSPLINE_NAK         //continous third derivative
};

template<typename Tx,typename Ty,int order=3>
class Bezier
{
    private:
        Grid1<double> bin_coeff,c_nt,c_t,coeff;
        Grid1<Tx> vertex_x;
        Grid1<Ty> vertex_y;
        
        void set_t(double t)
        {
            c_t[0]=c_nt[0]=1.0;
            double nt=1.0-t;
            
            for(int i=1;i<=order;i++)
            {
                c_t[i]=t*c_t[i-1];
                c_nt[i]=nt*c_nt[i-1];
            }
            
            for(int i=0;i<=order;i++)
                coeff[i]=bin_coeff[i]*c_t[i]*c_nt[order-i];
        }
        
    public:
        Bezier()
            :bin_coeff(order+1),
             c_nt(order+1,1),
             c_t(order+1,0),
             coeff(order+1,0),
             vertex_x(order+1,0),
             vertex_y(order+1,0)
        {
            for(int i=0;i<=order;i++) bin_coeff[i]=binomial(order,i);
        }
        
        Bezier(Grid1<Tx> &vertex_x_,
               Grid1<Ty> &vertex_y_)
            :bin_coeff(order+1),
             c_nt(order+1,1),
             c_t(order+1,0),
             coeff(order+1,0),
             vertex_x(order+1,0),
             vertex_y(order+1,0)
        {
            for(int i=0;i<=order;i++)
            {
                bin_coeff[i]=binomial(order,i);
                vertex_x[i]=vertex_x_[i];
                vertex_y[i]=vertex_y_[i];
            }
        }
        
        void eval(double t,Tx &x,Ty &y)
        {
            set_t(t);
            
            x=y=0;
            
            for(int i=0;i<=order;i++)
            {
                x+=coeff[i]*vertex_x[i];
                y+=coeff[i]*vertex_y[i];
            }
        }
        
        Tx eval_x(double t)
        {
            set_t(t);
            Tx x=0;
            
            for(int i=0;i<=order;i++) x+=coeff[i]*vertex_x[i];
            
            return x;
        }
        
        Ty eval_y(double t)
        {
            set_t(t);
            Ty y=0;
            
            for(int i=0;i<=order;i++) y+=coeff[i]*vertex_y[i];
            
            return y;
        }
        
        Tx xmax() { return vertex_x[order]; }
        Tx xmin() { return vertex_x[0]; }
        
        void set(Grid1<Tx> const &vertex_x_,
                 Grid1<Ty> const &vertex_y_)
        {
            for(int i=0;i<=order;i++)
            {
                vertex_x[i]=vertex_x_[i];
                vertex_y[i]=vertex_y_[i];
            }
        }
        
        double x_to_t(Tx const &x,double eps=1e-6)
        {
                 if(x<=vertex_x[0]) return 0;
            else if(x>=vertex_x[order]) return 1.0;
            
            double t_a=0,t_b=1.0,t_c=0.5;
            
//            Tx x_a=eval_x(t_a);
//            Tx x_b=eval_x(t_b);
//            Tx x_c=eval_x(t_c);
            Tx x_c=0;
            
            while(2.0*std::abs((t_b-t_a)/(t_a+t_b))>eps)
            {
                t_c=(t_a+t_b)/2.0;
                
//                x_a=eval_x(t_a);
//                x_b=eval_x(t_b);
                x_c=eval_x(t_c);
                
                if(x_c<=x) t_a=t_c;
                else t_b=t_c;
            }
            
            t_c=(t_a+t_b)/2.0;
            
            return t_c;
        }
};

class Cspline
{
    private:
        int Np,end_mode;
        double offset_x,offset_y;
        double scale_x,scale_y;
        std::vector<double> xp,yp;
        Grid2<double> coeffs;
        
        void calc_coeffs();
        void reorder();
        void rescale();
        
    public:
        Cspline();
        Cspline(Cspline const &spline);
        Cspline(std::vector<double> const &x,std::vector<double> const &y,int end_mode=CSPLINE_NATURAL);
        
        double eval(double const &x) const;
        std::size_t get_N() const;
        double get_x_base(std::size_t const &i) const;
        double get_y_base(std::size_t const &i) const;
        void init(std::vector<double> const &x,std::vector<double> const &y,int end_mode=CSPLINE_NATURAL);
        bool is_empty() const;
        void show() const;
        
        double operator () (double const &x) const;
        void operator = (Cspline const &spline);
        bool operator == (Cspline const &spline) const;
        
};

class Cspline_2fct
{
    private:
        Cspline x_spline,y_spline;
        
    public:
        Cspline_2fct(std::vector<double> const &t,
                     std::vector<double> const &x,
                     std::vector<double> const &y,
                     int end_mode=CSPLINE_NATURAL);
        
        void eval(double t,double &x,double &y);
};

template<typename Tx,typename Ty,int order=3>
class MultiBezier
{
    public:
        bool finalized;
        int Nb;
        Grid1<Bezier<Tx,Ty,order>> bezier_arr;
        Grid1<Tx> xmax,xmin;
        
        MultiBezier()
            :finalized(false), Nb(0)
        {
        }
        
        MultiBezier(int Nb_)
            :finalized(false), Nb(Nb_), bezier_arr(Nb)
        {
        }
        
        Ty eval(Tx const &x)
        {
            if(finalized)
            {
                int k=0;
                
                     if(x<=bezier_arr[0].xmin()) k=0;
                else if(x>=bezier_arr[Nb-1].xmax()) k=Nb-1;
                else
                {
                    for(int i=0;i<Nb;i++) 
                        if(x>=bezier_arr[i].xmin() && x<=bezier_arr[i].xmax())
                        {
                            k=i;
                            break;
                        }
                }
                
                double t=bezier_arr[k].x_to_t(x);
                return bezier_arr[k].eval_y(t);
            }
            else{ Plog::print(LogType::FATAL, "ERROR: unfinalized MultiBezier\n"); }
            
            return 0;
        }
        
        void finalize()
        {
            finalized=true;
            
            xmax.init(Nb);
            xmin.init(Nb);
            
            for(int i=0;i<Nb;i++)
            {
                xmax[i]=bezier_arr[i].xmax();
                xmin[i]=bezier_arr[i].xmin();
            }
        }
        
        void set(int Nb_)
        {
            Nb=Nb_;
            bezier_arr.init(Nb);
        }
        
        void set(int n,Grid1<Tx> const &vertex_x,
                 Grid1<Ty> const &vertex_y)
        {
            if(n>=0 && n<Nb)
                bezier_arr[n].set(vertex_x,vertex_y);
        }
};

template<typename T>
class VBezier
{
    private:
        int order;
        Grid1<double> bin_coeff;
        Grid1<double> c_t;
        Grid1<double> c_nt;
        Grid1<T> vertex;
        
    public:
        VBezier(int a)
            :order(a), bin_coeff(order+1), c_t(order+1), c_nt(order+1), vertex(order+1)
        {
            for(int i=0;i<=order;i++) bin_coeff(i)=binomial(order,i);
        }
        
        VBezier(int a,Grid1<T> &vertex_i)
            :order(a), bin_coeff(order+1), c_t(order+1), c_nt(order+1), vertex(order+1)
        {
            for(int i=0;i<=order;i++)
            {
                bin_coeff(i)=binomial(order,i);
                vertex(i)=vertex_i(i);
            }
        }
        
        T operator() (double t)
        {
            int i;
            
            T R=0;
            
            c_t(0)=c_nt(0)=1.0;
            double nt=1.0-t;
            
            for(i=1;i<=order;i++)
            {
                c_t(i)=t*c_t(i-1);
                c_nt(i)=nt*c_nt(i-1);
            }
            
            for(i=0;i<=order;i++)
            {
                R+=bin_coeff(i)*c_t(i)*c_nt(order-i)*vertex(i);
            }
            
            return R;
        }
};

template<typename T>
T interpolate_bilinear(T const &A,T const &B,T const &C,T const &D,double u,double v)
{
    return (1.0-v)*((1.0-u)*A+u*B)+v*((1.0-u)*D+u*C);
}

template<typename T>
T interpolate_linear(T const &A,T const &B,double u) { return A+(B-A)*u; }

template<typename T>
T interpolate_tri_UV(T const &A,T const &B,T const &C,double u,double v)
{
    return (1.0-u-v)*A+u*B+v*C;
}


#endif // MATH_APPROX_INCLUDED
