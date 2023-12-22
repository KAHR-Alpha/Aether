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

#ifndef FD_BASE_H_INCLUDED
#define FD_BASE_H_INCLUDED

#include <grid.h>
#include <material.h>

class FD_Base
{
    public:
        int Nx,Ny,Nz;
        int Nxy,Nxyz;
        unsigned int Nmats;
        double Dx,Dy,Dz;
        
        int Nx_s,Ny_s,Nz_s;
        int xs_s,xs_e,ys_s,ys_e,zs_s,zs_e;
        
        int pad_xm,pad_xp;
        int pad_ym,pad_yp;
        int pad_zm,pad_zp;
        
        // PML
        
        bool pml_x,pml_y,pml_z;
        int pml_xm,pml_xp;
        int pml_ym,pml_yp;
        int pml_zm,pml_zp;
        double pml_m,pml_ma;
        
        double pml_kappa_xm,pml_kappa_xp;
        double pml_kappa_ym,pml_kappa_yp;
        double pml_kappa_zm,pml_kappa_zp;
        
        double pml_sigma_xm,pml_sigma_xp;
        double pml_sigma_ym,pml_sigma_yp;
        double pml_sigma_zm,pml_sigma_zp;
        
        double pml_alpha_xm,pml_alpha_xp;
        double pml_alpha_ym,pml_alpha_yp;
        double pml_alpha_zm,pml_alpha_zp;
        
        // Materials
        
        Grid1<Material> mats;
        Grid3<unsigned int> matsgrid;
        
        FD_Base();
        
        void extend_grid();
        void extend_grid_sub(Grid3<unsigned int> &matsgrid);
        
        Imdouble get_Dx(double ind,double w);
        Imdouble get_Dy(double ind,double w);
        Imdouble get_Dz(double ind,double w);
        
        int index(int i,int j,int k);
        
        template<int N=6>
        int index(int i,int j,int k,int f)
        {
            return f+N*(i+j*Nx+k*Nxy);
        }
        
        int index_Ex(int k);
        int index_Ex(int i,int j,int k);
        int index_Ey(int k);
        int index_Ey(int i,int j,int k);
        int index_Ez(int k);
        int index_Ez(int i,int j,int k);
        
        int index_Hx(int k);
        int index_Hx(int i,int j,int k);
        int index_Hy(int k);
        int index_Hy(int i,int j,int k);
        int index_Hz(int k);
        int index_Hz(int i,int j,int k);
        
        void init_pml();
        
        void set_materials(Grid1<Material> const &M);
        void set_material(unsigned int m_ID,Material const &material);
        void set_material(unsigned int m_ID,std::string fname);
        void set_matsgrid(Grid3<unsigned int> const &G);
//        void set_matsgrid_full(Grid3<unsigned int> const &G);
        void set_padding(int xm,int xp,int ym,int yp,int zm,int zp);
        
        void set_pml_xm(int N_pml,double kap,double sig,double alp);
        void set_pml_xp(int N_pml,double kap,double sig,double alp);
        
        void set_pml_ym(int N_pml,double kap,double sig,double alp);
        void set_pml_yp(int N_pml,double kap,double sig,double alp);
        
        void set_pml_zm(int N_pml,double kap,double sig,double alp);
        void set_pml_zp(int N_pml,double kap,double sig,double alp);
        
        virtual void update_Nxyz();
};

#endif // FD_BASE_H_INCLUDED
