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

#ifndef SENSORS_H
#define SENSORS_H

#include <enum_constants.h>
#include <mathUT.h>
#include <phys_constants.h>
#include <planar_wgd.h>
#include <bitmap3.h>
#include <fdtd_core.h>

class Source;

//######################
//   Sensor_generator
//######################
        
enum class Sensor_type
{
    BOX_POYNTING=0,
    BOX_SPECTRAL_POYNTING,
    DIFF_ORDERS,
    FARFIELD,
    FIELDBLOCK,
    FIELDMAP,
    FIELDMAP2,
    FIELDPOINT,
    MOVIE,
    PLANAR_SPECTRAL_POYNTING,
    UNKNOWN
};

class Sensor_generator
{
    public:
        Sensor_type type;
        std::string name;
        
        int x1,x2,y1,y2,z1,z2;
        double x1r,x2r,y1r,y2r,z1r,z2r;
        
        bool location_real;
        
        int orientation;
        int Nfx,Nfy;
        
        int Nl;
        double lambda_min,lambda_max;
        
        int skip;
        
        bool disable_xm,disable_xp,
             disable_ym,disable_yp,
             disable_zm,disable_zp;
        
        Sensor_generator();
        Sensor_generator(Sensor_generator const &sens);
        
        void disable_plane(std::string dir);
        void operator = (Sensor_generator const &sens);
        void set_name(std::string name);
        void set_orientation(std::string orient_str);
        void set_resolution(int Nfx,int Nfy);
        void set_skip(int skip);
        void set_spectrum(double lambda_min,double lambda_max,int Nl);
        void set_wavelength(double lambda);
        void to_discrete(double Dx,double Dy,double Dz);
};

//############
//   Sensor   
//############

class Sensor
{
    public:
        int Nx,Ny,Nz,Nt;
        int xs_s,xs_e,ys_s,ys_e,zs_s,zs_e;
        
        double Dx,Dy,Dz,Dt;
        
        int step;
        bool silent;
        int type;
        int x1,x2,y1,y2,z1,z2;
        int span1,span2,span3;
        
        int Ntap;
        double tapering_E,tapering_H;
        
        int Nl;
        std::vector<double> lambda;
        Grid1<double> rsp_result;
        Grid1<Imdouble> sp_result;
        
        Source *reference_src;
        
        std::filesystem::path directory;
        std::string name;
        
        int sensor_ID;
        static int sensor_ID_next;
        
        bool disable_xm,disable_xp;
        bool disable_ym,disable_yp;
        bool disable_zm,disable_zp;
        
        // Threading
        
        bool process_threads;
        unsigned int Nthreads;
        ThreadsAlternator alternator;
        std::vector<std::thread*> threads;
        std::vector<bool> threads_ready;
        
        Sensor();
        virtual ~Sensor();
        
        void feed(FDTD const &fdtd);
        virtual void deep_feed(FDTD const &fdtd);
        virtual void initialize();
        virtual void link(FDTD const &fdtd);
        void set_reference_source(Source *reference_src);
        void set_loc(int x1,int x2,int y1,int y2,int z1,int z2);
        void set_silent(bool silent);
        virtual void set_spectrum(double lambda);
        virtual void set_spectrum(std::vector<double> const &lambda);
        //virtual void set_spectrum(Grid1<double> const &lambda);
        virtual void set_spectrum(int Nl,double lambda_min,double lambda_max);
        void set_type(int type);
        void show_location();
        
        virtual void treat();
};

class SensorFieldHolder: public Sensor
{
    public:
        bool interpolate;
        Grid2<double> t_Ex,t_Ey,t_Ez,t_Hx,t_Hy,t_Hz;
        Grid3<Imdouble> sp_Ex,sp_Ey,sp_Ez,sp_Hx,sp_Hy,sp_Hz;
        
        SensorFieldHolder(int type,
                          int x1,int x2,
                          int y1,int y2,
                          int z1,int z2,
                          bool interpolate);
        ~SensorFieldHolder();
        
        virtual void deep_feed(FDTD const &fdtd);
        void FT_comp(int l1,int l2);
        virtual void initialize();
        virtual void link(FDTD const &fdtd);
        void threaded_computation(unsigned int ID);
        virtual void treat();
        void update_t(FDTD const &fdtd);
        void update_t_interp(FDTD const &fdtd);
};

class Box_Poynting: public Sensor
{
    public:
        Box_Poynting(int x1,int x2,int y1,int y2,int z1,int z2);
        void deep_feed(FDTD const &fdtd);
};

class CompletionSensor: public Sensor
{
    public:
        bool FT_mode;
        int Np,N_avg;
        double lambda_min,lambda_max;
        double coeff,energy_last,energy_max;
        std::string layout;
        
        Grid1<int> i_loc,j_loc,k_loc;
        Grid1<double> lambda_loc,w_loc;
        Grid1<Imdouble> Ex_loc,Ey_loc,Ez_loc;
        Grid2<double> Ex_loc_r,Ex_loc_i,
                      Ey_loc_r,Ey_loc_i,
                      Ez_loc_r,Ez_loc_i;
        
        std::vector<int> est_step;
        std::vector<double> est_ratio;
        
        CompletionSensor(double coeff);
        CompletionSensor(double lambda_min,double lambda_max,double coeff,int Np,std::string const &layout);
        
        bool completion_check();
        void deep_feed(FDTD const &fdtd);
        int estimate();
        void initialize();
};

class DiffSensor: public SensorFieldHolder
{
    public:
        double n_index;
        Grid1<double> beta_x,beta_y;
        
        DiffSensor(int type,
                   int x1,int x2,
                   int y1,int y2,
                   int z1,int z2);
        ~DiffSensor();
        
        void link(FDTD const &fdtd);
        void treat();
};

void diffract_renorm(std::string const &diff_fname,std::string const &base_fname,std::string const &out_fname);
void diffract_renorm(std::string const &diff_fname,std::vector<std::string> const &base_fname,std::string const &out_fname);
std::string diffract_average_files(std::vector<std::string> const &fnames,std::string const &out_fname="");
std::string diffract_orders_normalize(std::string const &diff_fname,std::string out_fname="");
std::string diffract_power_normalize(std::string const &diff_fname,std::string const &base_fname,std::string out_fname="");

//class DiffSensor_FT: public Sensor
//{
//    public:
//        double ref_index;
//        Grid3<Imdouble> acc_Ex,acc_Ey,acc_Ez;
//        
//        DiffSensor_FT(int z1,double ref_index);
//        
//        void set_spectrum(double lambda);
//        void set_spectrum(Grid1<double> const &lambda);
//        
//        void deep_feed(Grid3<double> const &Ex,Grid3<double> const &Ey,Grid3<double> const &Ez,
//                       Grid3<double> const &Hx,Grid3<double> const &Hy,Grid3<double> const &Hz);
//        void treat();
//};

class FarFieldSensor: public SensorFieldHolder
{
    public:
        int Nfx,Nfy;
        double n_index;
        
        FarFieldSensor(int x1,int x2,
                       int y1,int y2,
                       int z1,int z2,
                       int Nfx,int Nfy);
        ~FarFieldSensor();
        
        void link(FDTD const &fdtd);
        void set_resolution(int Nfx,int Nfy);
        void treat();
};

class FieldBlock: public Sensor
{
    public:
        FDTD const *fdtd;
        Grid3<unsigned int> mats;
        Grid3<Imdouble> Ex,Ey,Ez,Hx,Hy,Hz;
        
        FieldBlock(int x1,int x2,int y1,int y2,int z1,int z2);
        
        ~FieldBlock();
        
        void deep_feed(FDTD const &fdtd);
        void FT_Ex(int i1,int i2,Imdouble const &tcoeff);
        void FT_Ey(int i1,int i2,Imdouble const &tcoeff);
        void FT_Ez(int i1,int i2,Imdouble const &tcoeff);
        void FT_Hx(int i1,int i2,Imdouble const &tcoeff);
        void FT_Hy(int i1,int i2,Imdouble const &tcoeff);
        void FT_Hz(int i1,int i2,Imdouble const &tcoeff);
        void initialize();
        void threaded_computation(unsigned int ID);
        void treat();
};

class FieldMap: public Sensor
{
    public:
        bool mag_map,cumulative;
        Grid2<unsigned int> mats;
        Grid2<Imdouble> acc_Ex,acc_Ey,acc_Ez;
        
        FDTD const *fdtd_source;
        
        FieldMap(int type,
                 int x1,int x2,
                 int y1,int y2,
                 int z1,int z2);
        
        ~FieldMap();
        
        void deep_feed(FDTD const &fdtd);
        void initialize();
        void FT_compute(int j1,int j2);
        void set_cumulative(bool c=true);
        void set_mag_map(bool c=true);
        void threaded_computation(unsigned int ID);
        void treat();
};

class FieldMap2: public SensorFieldHolder
{
    public:
        Grid2<unsigned int> mats;
        
        FieldMap2(int type,
                  int x1,int x2,
                  int y1,int y2,
                  int z1,int z2);
        ~FieldMap2();
        
        void link(FDTD const &fdtd);
        void treat();
};

class FieldPoint: public Sensor
{
    public:
        std::ofstream file;
        
        FieldPoint(int x1,int y1,int z1);
        
        ~FieldPoint();
        
        void deep_feed(FDTD const &fdtd);
        void initialize();
        void treat();
};

class MovieSensor: public Sensor
{
    private:
        bool cumulative;
        double exposure;
        int skip;
        
        Grid2<double> f_x,f_y,f_z;
        Bitmap image;
        
    public:
        MovieSensor(int type,
                    int x1,int x2,
                    int y1,int y2,
                    int z1,int z2,
                    int skip=1,
                    double exposure=1.0);
                    
        void deep_feed(FDTD const &fdtd);
                  
        void set_cumulative(bool c=true);
};

//P for planar
class ModesDcpl: public Sensor
{
    public:
        int span;
                        
        Grid2<Imdouble> sp_Ey,sp_Hy;
        
        ModesDcpl(int type,
                  int x1,int x2,
                  int y1,int y2,
                  int z1,int z2);
        ~ModesDcpl();
        
        void deep_feed(Grid3<double> const &Ex,Grid3<double> const &Ey,Grid3<double> const &Ez,
                       Grid3<double> const &Hx,Grid3<double> const &Hy,Grid3<double> const &Hz);
        void set_spectrum(std::vector<double> const &lambda);
        void treat(std::string,Slab_guide &slab,int ng_ref);
};

class Spect_Poynting_FFT: public Sensor
{
    public:
        int base_bit;
        int Naccu;
        int curr_acc;
        
        std::string fname;
        std::ofstream f_buf;
        
        Grid3<double> acc_Ex,acc_Ey,acc_Ez,acc_Hx,acc_Hy,acc_Hz;
        
        Spect_Poynting_FFT(int type,
                           int x1,int x2,
                           int y1,int y2,
                           int z1,int z2);
        ~Spect_Poynting_FFT();
        
        void deep_feed(FDTD const &fdtd);
        void initialize();
        void treat(std::string);
};

class Spect_Poynting: public SensorFieldHolder
{
    public:
        Spect_Poynting(int type,
                       int x1,int x2,
                       int y1,int y2,
                       int z1,int z2);
        ~Spect_Poynting();
        
        void treat();
};

class Box_Spect_Poynting: public Sensor
{
    public:
        Spect_Poynting xm,xp,ym,yp,zm,zp;
        
        Box_Spect_Poynting(int x1,int x2,
                           int y1,int y2,
                           int z1,int z2);
                           
        void deep_feed(FDTD const &fdtd);
        void link(FDTD const &fdtd);
        void treat();
};

Sensor* generate_fdtd_sensor(Sensor_generator const &gen,FDTD const &fdtd);

#endif // SENSORS_H
