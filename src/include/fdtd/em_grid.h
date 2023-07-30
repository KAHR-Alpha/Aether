/*Copyright 2008-2021 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

template<class T>
class XGrid
{
    private:
        int N1,N2,N3;
        int N12,N13,N23,NT;
        T *data;
        
    public:
        XGrid()
        {
            N1=N2=N3=0;
            N12=0;
            NT=0;
            data=0;
        }
        
        XGrid(int in1,int in2,int in3)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid3"<<std::endl;
            #endif
        }
        
        XGrid(int in1,int in2,int in3,T const &tmp)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        XGrid(XGrid<T> const &G)
            :N1(G.N1), N2(G.N2), N3(G.N3),
             N12(G.N12), N13(G.N13), N23(G.N23),
             NT(G.NT)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        ~XGrid()
        {
            N1=N2=N3=0;
            N12=N13=N23=0;
            NT=0;
            delete[] data;
        }
        
        void bound_check(int ind1,int ind2,int ind3)
        {
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2
               || ind3<0 || ind3>=N3)
            {
                std::cout<<"G3 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl
                         <<ind3<<" "<<N3<<std::endl;
                std::cout<<"Press enter to continue"<<std::endl;
                std::cin.get();
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
        
        void operator = (XGrid<T> const&G)
        {
            if(N1==G.N1 && N2==G.N2 && N3==G.N3)
            {
                for(int i=0;i<NT;i++) data[i]=G.data[i];
            }
            else std::cerr<<"Invalid Grid3 operation"<<std::endl;
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

template<class T>
class YGrid
{
    private:
        int N1,N2,N3;
        int N12,N13,N23,NT;
        T *data;
        
    public:
        YGrid()
        {
            N1=N2=N3=0;
            N12=0;
            NT=0;
            data=0;
        }
        
        YGrid(int in1,int in2,int in3)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid3"<<std::endl;
            #endif
        }
        
        YGrid(int in1,int in2,int in3,T const &tmp)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        YGrid(YGrid<T> const &G)
            :N1(G.N1), N2(G.N2), N3(G.N3),
             N12(G.N12), N13(G.N13), N23(G.N23),
             NT(G.NT)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        ~YGrid()
        {
            N1=N2=N3=0;
            N12=N13=N23=0;
            NT=0;
            delete[] data;
        }
        
        void bound_check(int ind1,int ind2,int ind3)
        {
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2
               || ind3<0 || ind3>=N3)
            {
                std::cout<<"G3 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl
                         <<ind3<<" "<<N3<<std::endl;
                std::cout<<"Press enter to continue"<<std::endl;
                std::cin.get();
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
        
        void operator = (YGrid<T> const&G)
        {
            if(N1==G.N1 && N2==G.N2 && N3==G.N3)
            {
                for(int i=0;i<NT;i++) data[i]=G.data[i];
            }
            else std::cerr<<"Invalid Grid3 operation"<<std::endl;
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

template<class T>
class ZGrid
{
    private:
        int N1,N2,N3;
        int N12,N13,N23,NT;
        T *data;
        
    public:
        ZGrid()
        {
            N1=N2=N3=0;
            N12=0;
            NT=0;
            data=0;
        }
        
        ZGrid(int in1,int in2,int in3)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            
            #ifdef GRID_INIT_CHECK
                std::cout<<"Warning: uninitialized Grid3"<<std::endl;
            #endif
        }
        
        ZGrid(int in1,int in2,int in3,T const &tmp)
            :N1(in1), N2(in2), N3(in3),
             N12(N1*N2), N13(N1*N3), N23(N2*N3),
             NT(N1*N2*N3)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=tmp;
        }
        
        ZGrid(ZGrid<T> const &G)
            :N1(G.N1), N2(G.N2), N3(G.N3),
             N12(G.N12), N13(G.N13), N23(G.N23),
             NT(G.NT)
        {
            data=new T[NT];
            for(int i=0;i<NT;i++) data[i]=G.data[i];
        }
        
        ~ZGrid()
        {
            N1=N2=N3=0;
            N12=N13=N23=0;
            NT=0;
            delete[] data;
        }
        
        void bound_check(int ind1,int ind2,int ind3)
        {
            if(ind1<0 || ind1>=N1
               || ind2<0 || ind2>=N2
               || ind3<0 || ind3>=N3)
            {
                std::cout<<"G3 Error, out of bounds: "<<std::endl
                         <<ind1<<" "<<N1<<std::endl
                         <<ind2<<" "<<N2<<std::endl
                         <<ind3<<" "<<N3<<std::endl;
                std::cout<<"Press enter to continue"<<std::endl;
                std::cin.get();
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
        
        void operator = (ZGrid<T> const&G)
        {
            if(N1==G.N1 && N2==G.N2 && N3==G.N3)
            {
                for(int i=0;i<NT;i++) data[i]=G.data[i];
            }
            else std::cerr<<"Invalid Grid3 operation"<<std::endl;
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
