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

#ifndef DATA_HDL_INCLUDED
#define DATA_HDL_INCLUDED

#include <vector>

#include <mathUT.h>
#include <phys_constants.h>

class SpAng
{
    public:
        double lambda;
        int NEE,NEM,NME,NMM;
        std::vector<Imdouble> d_EE,d_EM,d_ME,d_MM;
        std::vector<double> a_EE,a_EM,a_ME,a_MM;
        
        SpAng();
        SpAng(SpAng const &hld);
        
        void show();
        
        void operator = (SpAng const &hld);
};

class Spectrum
{
    public:
        unsigned int N;
        std::string source_pol,resp_pol;
        std::string type;                   //"ref" or "trans"
        std::vector<double> lambda;
        std::vector<Angle> ang;           //ang is supposed to be the angle of incidence
        std::vector<Imdouble> spect;
        
        Spectrum();
        Spectrum(int N);
        Spectrum(Spectrum const &sp);
        Spectrum(std::vector<double> const &lambda,double ang,std::vector<Imdouble> const &spect);
        Spectrum(std::vector<double> const &lambda,std::vector<Angle> const &ang,std::vector<Imdouble> const &spect);
        
        bool is_in(double lambda);
        bool is_in_f(double f);
        bool is_in_w(double w);
        
        void get(double lambda,double &ang,Imdouble &spect); 
        void get_f(double f,double &ang,Imdouble &spect);
        void get_w(double w,double &ang,Imdouble &spect);
        void get_polarizations(std::string &source_pol,std::string &resp_pol);
        
        double max_amp();
        
        void set_polarizations(std::string const &source_pol,std::string const &resp_pol);
        void set_type_ref();
        void set_type_trans();
        
        void operator = (Spectrum const &sp);
        
        friend std::ostream & operator<< (std::ostream &out,Spectrum const &sp);
        friend std::istream & operator>> (std::istream &in,Spectrum &sp);
};

class SpectrumCollec
{
    public:
        int N_EE,N_EM,N_MM,N_ME; //one for each kind of polarization excitation/response
        
        std::vector<Spectrum *> sp_EE,sp_EM,sp_MM,sp_ME;
        
        SpectrumCollec();
        
        void add_collec(SpectrumCollec const &sp);
        void add_spectrum(Spectrum const &sp);
        
        bool check_polrot();
        
        Spectrum get_spectrum_EE(int n);
        Spectrum get_spectrum_EM(int n);
        Spectrum get_spectrum_MM(int n);
        Spectrum get_spectrum_ME(int n);
        
        void extract_data(SpAng &data,std::string type,double ang_max);
        
        void read(std::string fname);
        void write(std::string fname);
        
        friend std::ostream & operator<< (std::ostream &out,SpectrumCollec const &sp_col);
        friend std::istream & operator>> (std::istream &in,SpectrumCollec &sp_col);
};

#endif // DATA_HDL_INCLUDED
