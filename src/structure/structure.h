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

#ifndef STRUCTURE_H_INCLUDED
#define STRUCTURE_H_INCLUDED

#include <lua_base.h>
#include <mesh_base.h>

#include <Eigen/Eigen>

#include <filesystem>
#include <vector>

class Structure;

class Structure_OP
{
    public:
        double x1,x2,y1,y2,z1,z2,r;
        int mat_index,stack_ID;
        Structure *parent;
        
        Structure_OP(double x1,double x2,
                     double y1,double y2,
                     double z1,double z2,int mat_index);
        virtual ~Structure_OP();
        
        virtual int index(double x,double y,double z);
};

class Add_Block: public Structure_OP
{
    public:
        Add_Block(double x1,double x2,
                  double y1,double y2,
                  double z1,double z2,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Coating: public Structure_OP
{
    public:
        double thickness,delta;
        Vector3 dir;
        int origin_mat;
        
        Add_Coating(AngleRad const &theta,AngleRad const &phi,
                    double thickness,double delta,int origin_mat,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Cone: public Structure_OP
{
    public:
        Vector3 base_vec;
        double length;
        
        Add_Cone(double x1,double y1,double z1,
                 double x2,double y2,double z2,
                 double r,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Cylinder: public Structure_OP
{
    public:
        Vector3 base_vec;
        double length;
        
        Add_Cylinder(double x1,double y1,double z1,
                     double x2,double y2,double z2,
                     double r,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Ellipsoid: public Structure_OP
{
    public:
        double rx,ry,rz;
        Add_Ellipsoid(double x1,double y1,double z1,
                      double rx,double ry,double rz,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Layer: public Structure_OP
{
    public:
        int type;
        
        Add_Layer(int type,double z1,double z2,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Lua_Def: public Structure_OP
{
    public:
        lua_State *L;
        std::string fname;
        std::vector<lua_tools::lua_type*> parameters;
        
        Add_Lua_Def(lua_State *L,
                    std::string const &fname,
                    std::vector<lua_tools::lua_type*> const &parameters,
                    int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Mesh: public Structure_OP
{
    public:
        double scale;
        std::filesystem::path fname;
        
        std::vector<Vertex> V_arr;
        std::vector<Face> F_arr;
        
        double x_min,x_max;
        double y_min,y_max;
        double z_min,z_max;
        
        Add_Mesh(double x1,double y1,double z1,double scale,
                 std::filesystem::path fname,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Sin_Layer: public Structure_OP
{
    public:
        double a1,a2;
        double px,phi_x;
        double py,phi_y;
        
        Add_Sin_Layer(double z1,double a1,double z2,double a2,
                      double px,double phi_x,double py,double phi_y,
                      int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Sphere: public Structure_OP
{
    public:
        Add_Sphere(double x1,double y1,double z1,double r,int mat_index);
        
        int index(double x,double y,double z);
};

class Add_Vect_Block: public Structure_OP
{
    public:
        double xO,yO,zO;
        double xA,yA,zA;
        double xB,yB,zB;
        double xC,yC,zC;
        
        Eigen::Matrix3d vectmat;
        
        double x_min,x_max;
        double y_min,y_max;
        double z_min,z_max;
        
        Add_Vect_Block(double xO,double yO,double zO,
                       double xA,double yA,double zA,
                       double xB,double yB,double zB,
                       double xC,double yC,double zC,int mat_index);
        
        void get_abc(double &a,double &b,double &c,
                     double x,double y,double z);
        virtual int index(double x,double y,double z);
};

class Add_Vect_Tri: public Add_Vect_Block
{
    public:
        double alp;
        
        Add_Vect_Tri(double xO,double yO,double zO,
                     double xA,double yA,double zA,
                     double xB,double yB,double zB,
                     double xC,double yC,double zC,
                     double alp,int mat_index);
        
        int index(double x,double y,double z);
};

class Structure
{
    public:
        int default_material;
        double lx,ly,lz;
        bool flip_x,flip_y,flip_z;
        int periodic_x,periodic_y,periodic_z;
        
        lua_State *L;
        std::filesystem::path script;
        
        std::vector<double> parameter_value;
        std::vector<std::string> parameter_name;
        
        std::vector<Structure_OP*> operations;
        
        Structure();
        Structure(std::filesystem::path const &script);
        ~Structure();
        
        void add_operation(Structure_OP *operation);
        void discretize(Grid3<unsigned int> &matgrid,
                        int Nx,int Ny,int Nz,double Dx,double Dy,double Dz);
        void finalize();
        int index(double x,double y,double z);
        int index(double x,double y,double z,int restrict_level);
        void print(std::filesystem::path const &path_,double Dx,double Dy,double Dz);
        void set_parameter(std::string const &parameter,double value);
        void retrieve_nominal_size(double &lx,double &ly,double &lz) const;
        void voxelize(double Dx,double Dy,double Dz);
};

#endif // STRUCTURE_H_INCLUDED
