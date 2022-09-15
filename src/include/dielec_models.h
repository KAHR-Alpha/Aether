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

#ifndef DIELEC_MODELS_H_INCLUDED
#define DIELEC_MODELS_H_INCLUDED

#include <bitmap3.h>
#include <mathUT.h>
#include <phys_constants.h>
#include <iostream>
#include <fstream>
#include <vector>

class dielec_debye
{
    private:
        double ds,t0;
        
    public:
        dielec_debye();
        dielec_debye(dielec_debye const &D);
    
        Imdouble eval(double) const;
        std::string get_matlab(int ID) const;
        void get_time_exp(Imdouble &a,Imdouble &b);
        void set(double ds,double t0);
        void show();
        
        void operator = (dielec_debye const &D);
};

class dielec_drude
{
    private:
        double wd,wd2,g;
        
    public:
        dielec_drude();
        dielec_drude(dielec_drude const &D);
        
        Imdouble eval(double) const;
        std::string get_matlab(int ID) const;
        void get_time_exp(Imdouble &a,Imdouble &b);
        double get_sigma();
        void set(double wd,double g);
        void show();
        
        void operator = (dielec_drude const &D);
};

class dielec_lorentz
{
    private:
        double A,O,O2,G;
        
    public:
        dielec_lorentz();
        dielec_lorentz(dielec_lorentz const &D);
        
        Imdouble eval(double) const;
        std::string get_matlab(int ID) const;
        void get_time_exp(Imdouble &a,Imdouble &b);
        void set(double A,double O,double G);
        void show();
        
        void operator = (dielec_lorentz const &D);
};

class dielec_critpoint
{
    private:
        double A,O,P,G;
        
    public:
        dielec_critpoint();
        dielec_critpoint(dielec_critpoint const &D);
        
        Imdouble eval(double) const;
        std::string get_matlab(int ID) const;
        void get_time_exp(Imdouble &a,Imdouble &b);
        void set(double A,double O,double P,double G);
        void show();
        
        void operator = (dielec_critpoint const &D);
};

class Dielec_model
{
    public:
        Dielec_model();
        Dielec_model(Dielec_model const& D);
        ~Dielec_model();
        
        double ei;
        
        int Ndebye,Ndrude,Nlorentz,Ncp;
        
        Grid1<dielec_debye> debye_arr;
        Grid1<dielec_drude> drude_arr;
        Grid1<dielec_lorentz> lorentz_arr;
        Grid1<dielec_critpoint> cp_arr;
        
        void set_const(double ei);
        void add_debye(double ds,double t0);
        void add_drude(double wd,double g);
        void add_lorentz(double A,double O,double G);
        void add_critpoint(double A,double O,double P,double G);
        
        void clear();
        Imdouble eval(double) const; 
        double get_const();
        void get_time_exp(Grid1<Imdouble> &va,Grid1<Imdouble> &vb);
        std::string get_matlab() const;
        int get_N_models();
        double get_sigma();
        void show();
        void operator = (Dielec_model const &D);
};

#endif // DIELEC_MODELS_H_INCLUDED
