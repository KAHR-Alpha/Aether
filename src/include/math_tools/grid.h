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

#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include <cmath>
#include <cstdlib>
#include <iostream>

//#define D_BCHECK
//#define GRID_INIT_CHECK

//#########################
// CRITICAL: DO NOT TOUCH
//#########################

template<class T>
class Grid1
{
    private:
        int N1;
        T *data;
        
    public:
        Grid1()
            :N1(0), data(nullptr)
        {
        }
        
        Grid1(int in)
            :N1(in)
        {
            if(N1>0) data=new T[N1];
            else data=nullptr;
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid1"<<std::endl;
            #endif
        }
        
        Grid1(int in,T const& tmp)
            :N1(in)
        {
            if(N1>0)
            {
                data=new T[N1];
                for(int i=0;i<N1;i++) data[i]=tmp;
            }
            else data=nullptr;
        }
        
        Grid1(Grid1<T> const&G)
        {
            N1=G.N1;
            
            if(N1>0)
            {
                data=new T[N1];
                for(int i=0;i<N1;i++) data[i]=G.data[i];
            }
            else data=nullptr;
        }
        
        ~Grid1()
        {
            if(data!=nullptr) delete[] data;
        }
        
        void gridsort(int k=0)
        {
            T buf;
            if(k==0) k=N1;
            
            for(int i=0;i<k;i++)
            {
                for(int j=i+1;j<k;j++)
                {
                    if(data[j]<=data[i])
                    {
                        buf=data[i];
                        data[i]=data[j];
                        data[j]=buf;
                    }
                }
            }
        }
        
        void gridsort_L(Grid1<double> &prop,int k1=0,int k2=0)
        {
            T buf;
            double buf1;
            if(k2==0) k2=N1;
            
            for(int i=k1;i<k2;i++)
            {
                for(int j=i+1;j<k2;j++)
                {
                    if(prop(j)<=prop(i))
                    {
                        buf1=prop(i);
                        prop(i)=prop(j);
                        prop(j)=buf1;
                        
                        buf=data[i];
                        data[i]=data[j];
                        data[j]=buf;
                    }
                }
            }
        }
        
        void gridsort_L_r(Grid1<double> &prop,int k1=0,int k2=0)
        {
            T buf;
            double buf1;
            if(k2==0) k2=N1;
            
            for(int i=k1;i<k2;i++)
            {
                for(int j=i+1;j<k2;j++)
                {
                    if(prop(j)>=prop(i))
                    {
                        buf1=prop(i);
                        prop(i)=prop(j);
                        prop(j)=buf1;
                        
                        buf=data[i];
                        data[i]=data[j];
                        data[j]=buf;
                    }
                }
            }
        }
        
        void init(int in)
        {
            if(in>0)
            {
                N1=in;
                
                if(data!=nullptr) delete[] data;
                data=new T[N1];
                
                #ifdef GRID_INIT_CHECK
                    std::cout<<"Warning: uninitialized Grid1"<<std::endl;
                #endif
            }
            else
            {
                if(data!=nullptr) delete[] data;
                data=nullptr;
                N1=0;
            }
        }
        
        void init(int in,T const &tmp)
        {
            if(in>0)
            {
                N1=in;
                
                if(data!=nullptr) delete[] data;
                data=new T[N1];
                
                for(int i=0;i<N1;i++) data[i]=tmp;
            }
            else
            {
                if(data!=nullptr) delete[] data;
                data=nullptr;
                N1=0;
            }
        }
        
        void insert(int k,T const &ob)
        {
            int i;
            
            Grid1<T> tmpgrid=*this;
            init(N1+1);
            
            for(i=0;i<k;i++) (*this)[i]=tmpgrid[i];
            (*this)[k]=ob;
            for(i=k+1;i<N1;i++) (*this)[i]=tmpgrid[i-1];
        }
        
        int L1() const { return N1; }
        
        T lin_interp(int k,double u) const
        {
            return (1.0-u)*data[k]+u*data[k+1];
        }
        
        T lin_interp(double pos,double a,double b) const
        {
            double h=(b-a)/(N1-1.0);
            
            double u=(pos-a)/h;
            int i=static_cast<int>(u);
            
            if(i<0) return data[0];
            else if(i>=N1-1) return data[N1-1];
            else
            {
                u-=i;
                
                return (1.0-u)*data[i]+u*data[i+1];
            }
        }
        
        void linspace(double start,double end)
        {
            for(int i=0;i<N1;i++)
            {
                data[i]=start+(end-start)*i/(N1-1.0);
            }
        }
        
        //Locates one value in the grid, linear interpolation used
        //The result is so that (1.0-u)*arr[k]+u*arr[k+1] should not
        //segfault if arr is of the same size of this grid
        //Made for arrays containing monotonous values only
        void vector_locate_linear(double val,int &k,double &u)
        {
            if(val<=data[0]) 
            {
                k=0;
                u=0;
                return;
            }
            else if(val>=data[N1-1])
            {
                k=N1-2;
                u=1.0;
                return;
            }
            else
            {
                int i;
                for(i=0;i<N1-1;i++)
                    if(val>=data[i] && val<data[i+1]) { k=i; break; }
                
                u=(val-data[k])/(data[k+1]-data[k]);
                return;
            }
        }
        
        T max() const
        {
            T R=data[0];
            for(int i=0;i<N1;i++) R=std::max(R,data[i]);
            return R;
        }
        
        double mean() const
        {
            double r=0;
            for(int i=0;i<N1;i++) r+=data[i];
            return r/N1;
        }
        
        double mem_size()
        {
            return static_cast<double>(N1)*sizeof(T);
        }
        
        T min() const
        {
            T R=data[0];
            for(int i=0;i<N1;i++) R=std::min(R,data[i]);
            return R;
        }
        
        void normalize()
        {
            T ma=max();
            T mi=min();
            for(int i=0;i<N1;i++) data[i]=(data[i]-mi)/(ma-mi);
        }
        
        [[deprecated]]
        T& operator() (int ind)
        {
            #ifdef D_BCHECK
            if(ind<0 || ind>=N1)
            {
                std::cout<<"G1 Error, out of bounds: "<<ind<<" "<<N1<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind];
        }
        
        T& operator[] (int ind)
        {
            #ifdef D_BCHECK
            if(ind<0 || ind>=N1)
            {
                std::cout<<"G1 Error, out of bounds: "<<ind<<" "<<N1<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind];
        }
        
        [[deprecated]]
        T const & operator() (int ind) const
        {
            #ifdef D_BCHECK
            if(ind<0 || ind>=N1)
            {
                std::cout<<"G1 Error, out of bounds: "<<ind<<" "<<N1<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind];
        }
        
        T const & operator[] (int ind) const
        {
            #ifdef D_BCHECK
            if(ind<0 || ind>=N1)
            {
                std::cout<<"G1 Error, out of bounds: "<<ind<<" "<<N1<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind];
        }
        
        void operator = (T a)
        {
            for(int i=0;i<N1;i++) data[i]=a;
        }
        
        void operator = (Grid1<T> const&G)
        {
            if(N1!=G.N1)
            {
                delete[] data;
                data=new T[G.N1];
            }
            
            for(int i=0;i<N1;i++) data[i]=G.data[i];
        }
        
        bool operator == (Grid1<T> const &G)
        {
            if(N1!=G.N1) return false;
            
            for(int i=0;i<N1;i++)
            {
                if(data[i]!=G.data[i]) return false;
            }
            
            return true;
        }
        
        void push_back(T const &a)
        {
            if(N1>0)
            {
                resize(N1+1);
                (*this)[N1-1]=a;
            }
            else
            {
                N1=1;
                data=new T[1];
                data[0]=a;
            }
        }
        
        void push_back(Grid1<T> const &G)
        {
            int i,k=N1;
            
            resize(k+G.L1());
            for(i=k;i<N1;i++) data[i]=G(i-k);
        }
        
        void reset()
        {
            N1=0;
            if(data!=nullptr) delete[] data;
            data=nullptr;
        }
        
        void resize(int N)
        {
            if(data==nullptr)
            {
                data=new T[N];
                N1=N;
            }
            else
            {
                T *data_tmp=new T[N];
                
                for(int i=0;i<std::min(N,N1);i++)
                    data_tmp[i]=data[i];
                    
                delete[] data;
                
                data=data_tmp;
                N1=N;
            }
        }
        
        void show() const
        {
            for(int i=0;i<N1;i++) std::cout<<data[i]<<std::endl;
            std::cout<<std::endl;
        }
        
        double std_dev() const
        {
            double t_mean=mean();
            double mean2=0;
            
            for(int i=0;i<N1;i++)
            {
                mean2+=data[i]*data[i];
            }
            
            mean2/=N1;
            
            return std::sqrt(mean2-t_mean*t_mean);
        }
        
        T sum() const
        {
            T tmp=0;
            for(int i=0;i<N1;i++) tmp+=data[i];
            return tmp;
        }
        
        template<class T2>
        T2 sum() const
        {
            T2 tmp=0;
            for(int i=0;i<N1;i++) tmp+=data[i];
            return tmp;
        }
};

//#########################
// CRITICAL: DO NOT TOUCH
//#########################

template<class T>
class Grid2
{
    private:
        int N1,N2;
        int N12;
        T *data;
        
    public:
        Grid2()
            :N1(0), N2(0), N12(0),
             data(nullptr)
        {
        }
        
        Grid2(int in1,int in2)
            :N1(in1), N2(in2)
        {
            N12=N1*N2;
            
            data=new T[N12];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid2"<<std::endl;
            #endif
        }
        
        Grid2(int in1,int in2,T const &tmp)
            :N1(in1), N2(in2)
        {
            N12=N1*N2;
            
            data=new T[N12];
            for(int i=0;i<N12;i++) data[i]=tmp;
        }
        
        Grid2(Grid2<T> const&G)
        {
            N1=G.N1;
            N2=G.N2;
            N12=N1*N2;
            
            data=new T[N12];
            for(int i=0;i<N12;i++) data[i]=G.data[i];
        }
        
        ~Grid2()
        {
            if(data!=nullptr) delete[] data;
        }
        
        T bilin_interp(int i,int j,double u,double v) const
        {
            double um=1.0-u;
            double vm=1.0-v;
            
            T r=um*vm*(*this)(i,j);
            r+=u*vm*(*this)(i+1,j);
            r+=u*v*(*this)(i+1,j+1);
            r+=um*v*(*this)(i,j+1);
            
            return r;
        }
        
        T bilin_interp(double x,double y,double x_min,double x_max,double y_min,double y_max) const
        {
            double hx=(x_max-x_min)/(N1-1.0);
            double hy=(y_max-y_min)/(N2-1.0);
            
            double u=(x-x_min)/hx;
            double v=(y-y_min)/hy;
            int i=static_cast<int>(u);
            int j=static_cast<int>(v);
            
            u-=i;
            v-=j;
            
            if(i<0){ i=0; u=0; }
            else if(i>=N1-1) { i=N1-2; u=1.0; }
            
            if(j<0){ j=0; v=0; }
            else if(j>=N2-1) { j=N2-2; v=1.0; }
            
            return bilin_interp(i,j,u,v);
        }
        
        void init(int in1,int in2)
        {
            N1=in1;
            N2=in2;
            
            N12=N1*N2;
            
            if(data!=nullptr) delete[] data;
            data=new T[N12];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid2"<<std::endl;
            #endif
        }
        
        void init(int in1,int in2,T const &tmp)
        {
            N1=in1;
            N2=in2;
            
            N12=N1*N2;
            
            if(data!=nullptr) delete[] data;
            data=new T[N12];
            
            for(int i=0;i<N12;i++) data[i]=tmp;
        }
        
        int L1() const {return N1;}
        int L2() const {return N2;}
        
        T max() const
        {
            T R=data[0];
            for(int i=0;i<N12;i++) R=std::max(R,data[i]);
            return R;
        }
        
        T min() const
        {
            T R=data[0];
            for(int i=0;i<N12;i++) R=std::min(R,data[i]);
            return R;
        }
        
        double mem_size()
        {
            return static_cast<double>(N12)*sizeof(T);
        }
        
        void normalize()
        {
            T ma=max();
            T mi=min();
            for(int i=0;i<N12;i++) data[i]=(data[i]-mi)/(ma-mi);
        }
        
        void operator = (T a)
        {
            for(int i=0;i<N12;i++) data[i]=a;
        }
        
        void operator = (Grid2<T> const&G)
        {
            if(N1!=G.N1 || N2!=G.N2)
            {
                delete[] data;
                data=new T[G.N12];
                
                N1=G.N1;
                N2=G.N2;
                N12=G.N12;
            }
            
            for(int i=0;i<N12;i++) data[i]=G.data[i];
        }
        
        bool operator == (Grid2<T> const &G)
        {
            if(N1!=G.N1 || N2!=G.N2) return false;
            
            for(int i=0;i<N12;i++)
            {
                if(data[i]!=G.data[i]) return false;
            }
            
            return true;
        }
        
        T& operator() (int ind1,int ind2)
        {
            #ifdef D_BCHECK
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2)
            {
                std::cout<<"G2 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind1+ind2*N1];
        }
        
        T const & operator() (int ind1,int ind2) const
        {
            #ifdef D_BCHECK
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2)
            {
                std::cout<<"G2 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl;
                std::system("pause");
            }
            #endif
            
            return data[ind1+ind2*N1];
        }
        
        void reset()
        {
            N1=N2=0;
            N12=0;
            if(data!=nullptr) delete[] data;
            data=nullptr;
        }
        
        void show() const
        {
            for(int i=0;i<N1;i++)
            {
                for(int j=0;j<N2;j++) std::cout<<data[i+j*N1]<<" ";
                std::cout<<std::endl;
            }
            std::cout<<std::endl;
        }
};

//#########################
// CRITICAL: DO NOT TOUCH
//#########################

template<class T>
class Grid3
{
    private:
        int N1,N2,N3;
        int N12,N13,N23,NT;
        T *data;
        
    public:
        Grid3()
        {
            N1=N2=N3=0;
            N12=0;
            NT=0;
            data=0;
        }
        
        Grid3(int in1,int in2,int in3)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid3"<<std::endl;
            #endif
        }
        
        Grid3(int in1,int in2,int in3,T const &tmp)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        Grid3(Grid3<T> const &G)
            :N1(G.N1), N2(G.N2), N3(G.N3),
             N12(G.N12), N13(G.N13), N23(G.N23),
             NT(G.NT)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        ~Grid3()
        {
            N1=N2=N3=0;
            N12=N13=N23=0;
            NT=0;
            delete[] data;
        }
        
        T at(int ind1,int ind2,int ind3,T const &failvalue) const
        {
            if(ind1<0 || ind1>N1-1 ||
               ind2<0 || ind2>N2-1 ||
               ind3<0 || ind3>N3-1) return failvalue;
            else return (*this)(ind1,ind2,ind3);
        }
        
        void bound_check(int ind1,int ind2,int ind3) const
        {
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2
               || ind3<0 || ind3>=N3)
            {
                std::cout<<"G3 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl
                         <<ind3<<" "<<N3<<std::endl;
                std::system("pause");
            }
        }
        
        void init(int in1,int in2,int in3)
        {
            N1=in1;
            N2=in2;
            N3=in3;
            
            N12=N1*N2;
            N13=N1*N3;
            N23=N2*N3;
            
            NT=N1*N2*N3;
            
            if(data!=0) delete[] data;
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid3"<<std::endl;
            #endif
        }
        
        void init(int in1,int in2,int in3,T const &tmp)
        {
            N1=in1;
            N2=in2;
            N3=in3;
            
            N12=N1*N2;
            N13=N1*N3;
            N23=N2*N3;
            
            NT=N1*N2*N3;
            
            if(data!=0) delete[] data;
            data=new T[NT];
            
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        int L1() const { return N1; }
        int L2() const { return N2; }
        int L3() const { return N3; }
        
        T max() const
        {
            T R=data[0];
            for(int i=0;i<NT;i++) R=std::max(R,data[i]);
            return R;
        }
        
        T min() const
        {
            T R=data[0];
            for(int i=0;i<NT;i++) R=std::min(R,data[i]);
            return R;
        }
        
        double mem_size()
        {
            return static_cast<double>(NT)*sizeof(T);
        }
        
        void normalize()
        {
            T ma=max();
            T mi=min();
            for(int i=0;i<NT;i++) data[i]=(data[i]-mi)/(ma-mi);
        }
        
        void operator = (T tin)
        {
            for(int i=0;i<NT;i++) data[i]=tin;
        }
        
        void operator = (Grid3<T> const&G)
        {
            if(N1!=G.N1 && N2!=G.N2 && N3!=G.N3)
            {
                delete[] data;
                data=new T[G.NT];
            }
            
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        T& operator() (int ind1,int ind2,int ind3)
        {
            #ifdef D_BCHECK
                bound_check(ind1,ind2,ind3);
            #endif
            
            return data[ind1+ind2*N1+ind3*N12];
        }
        
        T const & operator() (int ind1,int ind2,int ind3) const
        {
            #ifdef D_BCHECK
                bound_check(ind1,ind2,ind3);
            #endif
            
            return data[ind1+ind2*N1+ind3*N12];
        }
};

//#########################
// CRITICAL: DO NOT TOUCH
//#########################

template<class T>
class Grid4
{
    private:
        int N1,N2,N3,N4;
        int N12,N13,NT;
        T *data;
        
    public:
        Grid4()
        {
            N1=N2=N3=N4=NT=0;
            N12=N13=0;
            data=0;
        }
        
        Grid4(int in1,int in2,int in3,int in4)
            :N1(in1), N2(in2), N3(in3), N4(in4)
        {
            NT=N1*N2*N3*N4;
            N12=N1*N2;
            N13=N1*N2*N3;
            
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid4"<<std::endl;
            #endif
        }
        
        Grid4(int in1,int in2,int in3,int in4,T const &tmp)
            :N1(in1), N2(in2), N3(in3), N4(in4)
        {
            NT=N1*N2*N3*N4;
            N12=N1*N2;
            N13=N1*N2*N3;
            
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        Grid4(Grid4<T> const &G)
        {
            N1=G.N1;
            N2=G.N2;
            N3=G.N3;
            N4=G.N4;
            N12=G.N12;
            N13=G.N13;
            NT=G.NT;
            
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        ~Grid4()
        {
            N1=N2=N3=N4=NT=0;
            N12=N13=0;
            delete[] data;
        }
        
        void init(int in1,int in2,int in3,int in4)
        {
            N1=in1;
            N2=in2;
            N3=in3;
            N4=in4;
            
            NT=N1*N2*N3*N4;
            N12=N1*N2;
            N13=N1*N2*N3;
            
            if(data!=0) delete[] data;
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid4"<<std::endl;
            #endif
        }
        
        void init(int in1,int in2,int in3,int in4,T const &tmp)
        {
            N1=in1;
            N2=in2;
            N3=in3;
            N4=in4;
            
            NT=N1*N2*N3*N4;
            N12=N1*N2;
            N13=N1*N2*N3;
            
            if(data!=0) delete[] data;
            data=new T[NT];
            
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        double mem_size()
        {
            return static_cast<double>(NT)*sizeof(T);
        }
        
        void operator = (T tin)
        {
            for(int i=0;i<NT;i++) data[i]=tin;
        }
        
        void operator = (Grid4<T> const&G)
        {
            if(N1==G.N1 && N2==G.N2 && N3==G.N3 && N4==G.N4)
            {
                for(int i=0;i<NT;i++) data[i]=G.data[i];
            }
            else std::cerr<<"Invalid Grid4 operation"<<std::endl;
        }
        
        //Partial index computations
                
        int get_ind(int const &ind1) const
        {
            return ind1;
        }
        
        int get_ind(int const &ind1,int const &ind2) const
        {
            return ind1+ind2*N1;
        }
        
        int get_ind(int const &ind1,int const &ind2,int const &ind3) const
        {
            return ind1+ind2*N1+ind3*N12;
        }
        
        int get_ind(int const &ind1,int const &ind2,int const &ind3,int const &ind4) const
        {
            return ind1+ind2*N1+ind3*N12+ind4*N13;
        }
        
        void index_check(int ind1,int ind2,int ind3,int ind4) const
        {
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2
               || ind3<0 || ind3>=N3
               || ind4<0 || ind4>=N4)
            {
                std::cout<<"G4 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl
                         <<ind3<<" "<<N3<<std::endl
                         <<ind4<<" "<<N4<<std::endl;
                std::system("pause");
            }
        }
        
        T& agt(int P_ind,int ind4)
        {
            return data[P_ind+ind4*N13];
        }
        
        T& operator() (int ind1,int ind2,int ind3,int ind4)
        {
            #ifdef D_BCHECK
                index_check(ind1,ind2,ind3,ind4);
            #endif
            
            return data[ind1+ind2*N1+ind3*N12+ind4*N13];
        }
        
        T const & agt(int P_ind,int ind4) const
        {
            return data[P_ind+ind4*N13];
        }
        
        T const & operator() (int ind1,int ind2,int ind3,int ind4) const
        {
            #ifdef D_BCHECK
                index_check(ind1,ind2,ind3,ind4);
            #endif
            
            return data[ind1+ind2*N1+ind3*N12+ind4*N13];
        }
};


#endif // GRID_H_INCLUDED
