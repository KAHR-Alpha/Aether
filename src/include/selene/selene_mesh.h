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

#ifndef SELENE_MESH_H
#define SELENE_MESH_H

#include <index_utils.h>
#include <mathUT.h>
#include <material.h>
#include <multilayers.h>
#include <mesh_tools.h>

namespace Sel
{
    
enum
{
    IRF_FRESNEL,
    IRF_FRESNEL_SCATT,
    IRF_GRATING,
    IRF_LAMBERT,
    IRF_MULTILAYER,
    IRF_NONE,
    IRF_PERF_ABS,
    IRF_PERF_ANTIREF,
    IRF_PERF_MIRROR,
    IRF_SCATT_ABS,
    IRF_SNELL_FILE,
    IRF_SNELL_SCATT_FILE,
    IRF_SNELL_SPLITTER,
    TANGENT_UNSET,
    TANGENT_FIXED,
    TANGENT_EXPAND,
    TANGENT_EXPAND_NEG,
    TANGENT_POLAR,
    TANGENT_POLAR_NEG,
    SENS_ABS,
    SENS_NONE,
    SENS_TRANSP,
};

//typedef Material Material;

class smp_order
{
    public:
        double angle;
        double probability;
};

class IRF
{
    public:
        int type;
        std::string name;
        
        std::string ref_fname;
        std::string tra_fname;
        
        double scatt_ref;
                
        // Grating
        
        int grat_Nth;
        int grat_Nphi;
        int grat_No,grat_No_r,grat_No_t;
        
        std::filesystem::path grat_ref_path,grat_tra_path;
        
        std::vector<double> grat_th_angle,grat_phi_angle;
        Grid3<double> grat_orders_prob;
        Grid3<Vector3> grat_orders_dir;
        
        // Multilayers
        
        Multilayer_TMM_UD ml_model;
        
        std::vector<double> ml_heights;
        std::vector<Material> ml_materials;
        
        // Splitter
        
        double splitting_factor;
        
        // General purpose arrays
        
        int Nl,Nth;
        Grid1<double> lambda_data,ang_th_data,
                      ref_data,ref_scatt_data,
                      tra_data,tra_scatt_data;
        
        Grid2<double> g2,g2_ref,g2_tra;
        Grid3<double> g3,g3_ref,g3_tra;
        
        IRF();
        IRF(IRF const &irf);
        
        [[deprecated]] void bootstrap();
        
        bool get_response(Vector3 &out_dir,Vector3 &out_polar,
                          Vector3 const &in_dir,Vector3 const &in_polar,
                          Vector3 const &Fnorm,Vector3 const &Ftangent,
                          double lambda,double n1,double n2);
        bool get_response_fresnel(Vector3 &out_dir,Vector3 &out_polar,
                                  Vector3 const &in_dir,Vector3 const &in_polar,
                                  Vector3 const &Fnorm,double lambda,double n1,double n2);
        bool get_response_grating(Vector3 const &in_dir,Vector3 &out_dir,
                                  Vector3 const &Fnorm,Vector3 const &Ftang,
                                  double lambda,double n1,double n2);
        bool get_response_multilayer(Vector3 const &in_dir,Vector3 &out_dir,
                                     Vector3 const &Fnorm,double lambda,double n1,double n2);
        bool get_response_perf_antiref(Vector3 const &in_dir,Vector3 &out_dir,
                                       Vector3 const &Fnorm,double lambda,double n1,double n2);
        bool get_response_snell_scatt_file(Vector3 const &in_dir,Vector3 &out_dir,
                                           Vector3 const &Fnorm,double lambda,double n1,double n2);
        bool get_response_snell_file(Vector3 const &in_dir,Vector3 &out_dir,
                                     Vector3 const &Fnorm,double lambda,double n1,double n2);
        bool get_response_snell_splitter(Vector3 const &in_dir,Vector3 &out_dir,
                                         Vector3 const &Fnorm,double lambda,double n1,double n2);
        
        bool near_normal(double n_scal);
        
        void operator = (IRF const &IRF);
        
        void set_type(int type);
        void set_type_grating(std::string ref_fname,std::string tra_fname);
        void set_type_fresnel();
        void set_type_multilayer();
        void set_type_scatt_abs(double ref);
        void set_type_snell_file(std::string fname);
        void set_type_snell_scatt_file(std::string fname);
        void set_type_snell_splitter(double splitting_factor);
};

class Vertex
{
    public:
        Vector3 loc,norm;
        
        Vertex();
        Vertex(Vector3 const &);
        Vertex(double,double,double);
        Vertex(Vertex const &);
        
        void operator = (double);
        void operator = (::Vertex const &);
        void operator = (Sel::Vertex const &);
        
        void set_loc(Vector3 const &V);
        void set_loc(double const &x,double const &y,double const &z);
        void set_norm(Vector3 const &V);
        void set_norm(double const &x,double const &y,double const &z);
        void show();
        void translate(Vector3 const &);
        void translate(double const &x,double const &y,double const &z);
};

class SelFace
{
    public:
        int V1,V2,V3;
        Material *up_mat,*down_mat;
        IRF *up_irf,*down_irf;
        Vector3 norm;
        std::vector<int> ngb;
        
        int tangent_up,tangent_down;
        Vector3 fixed_tangent_up,
                fixed_tangent_down;
        
        SelFace();
        SelFace(int,int,int);
        SelFace(SelFace const &);
        
        void operator = (Face const &);
        void operator = (SelFace const &);
        
        void comp_norm(std::vector<Sel::Vertex> const &V);
        bool degeneracy_chk(int msg_ID);
        Vector3 get_cmass(std::vector<Sel::Vertex> const &V);
        void get_edge(int edge,int &Va,int &Vb,int &Vc);
        bool is_connected(SelFace const &F);
        void set_ngb(int N1,int N2,int N3);
        void set_norm(double x,double y,double z);
        void set_norm(Vector3 const &N);
        void set_Vindex(int const &V1,int const &V2,int const &V3);
        void set_vertex_index(int const &V1,int const &V2,int const &V3);
        void show();
        bool update_ngb(SelFace const &F,int F_ID);
};

}

#endif // SELENE_MESH_H
