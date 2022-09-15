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

#ifndef FIELDBLOCK_HOLDER_H_INCLUDED
#define FIELDBLOCK_HOLDER_H_INCLUDED

#include <mathUT.h>
#include <enum_constants.h>

class FieldBlockHolder
{
    public:
        int x1,y1,z1;
        int Nx,Ny,Nz;
        double Dx,Dy,Dz;
        double lambda,baseline;
        
        Grid3<unsigned int> mats;
        Grid3<Imdouble> Ex,Ey,Ez,Hx,Hy,Hz;
        
        FieldBlockHolder();
        FieldBlockHolder(FieldBlockHolder const &F);
        
        void extract_E_slice(Grid2<Imdouble> &Ex,Grid2<Imdouble> &Ey,Grid2<Imdouble> &Ez,int direction,int index);
        void extract_H_slice(Grid2<Imdouble> &Hx,Grid2<Imdouble> &Hy,Grid2<Imdouble> &Hz,int direction,int index);
        void extract_S_slice(Grid2<Imdouble> &Sx,Grid2<Imdouble> &Sy,Grid2<Imdouble> &Sz,int direction,int index);
        double get_E_abs(int i,int j,int k);
        double get_E_abs_max();
        double get_Ex_abs(int i,int j,int k);
        double get_Ex_abs_max();
        double get_Ey_abs(int i,int j,int k);
        double get_Ey_abs_max();
        double get_Ez_abs(int i,int j,int k);
        double get_Ez_abs_max();
        double integrate_poynting_box(int i1,int i2,int j1,int j2,int k1,int k2);
        double integrate_poynting_plane(int direction,int i1,int i2,int j1,int j2,int k1,int k2);
        bool load(std::string const &fname);
        bool save(std::string const &fname);
        void save_matlab(int direction,int location,int field,std::string fname);
        void set_baseline(double baseline);
        void undo_baseline();
};

void fmap_mats_name(Filename const &fname,Filename &fname_mats);
void fmap_mats_raw(Filename const &fname,Grid2<unsigned int> const &mats);
void fmap_names(Filename const &fname,int type,
                Filename &fname_x,Filename &fname_y,Filename &fname_z);
void fmap_raw(Filename const &fname,int type,
              Grid2<Imdouble> const &Gx,
              Grid2<Imdouble> const &Gy,
              Grid2<Imdouble> const &Gz,bool real=false);
void fmap_script(Filename const &fname,int type,bool real=false,double D1=1.0,double D2=1.0);

#endif // FIELDBLOCKHOLDER_H_INCLUDED
