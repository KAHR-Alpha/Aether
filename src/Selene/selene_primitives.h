/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef SELENE_PRIMITIVES_H
#define SELENE_PRIMITIVES_H

#include <geometry.h>
#include <selene_rays.h>

namespace Sel
{
    struct BoundingBox
    {
        double xm=0;
        double xp=0;
        
        double ym=0;
        double yp=0;
        
        double zm=0;
        double zp=0;
        
        double span_x() const { return xp-xm; }
        double span_y() const { return yp-ym; }
        double span_z() const { return zp-zm; }
    };
    
    int nearest_2np1(double val);
}

namespace Sel::Primitives
{
    class Primitive
    {
        public:
            BoundingBox &bbox;
            std::vector<Sel::SelFace> &F_arr;
            std::vector<std::string> &face_name_arr;
            
            Primitive(BoundingBox &bbox_,
                      std::vector<Sel::SelFace> &F_arr_,
                      std::vector<std::string> &face_name_arr_)
                :bbox(bbox_), F_arr(F_arr_), face_name_arr(face_name_arr_)
            {
            }
    };
    
    class Box: public Primitive
    {
        public:
            Box(BoundingBox &bbox,
                std::vector<Sel::SelFace> &F_arr,
                std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            double get_lx() const;
            double get_ly() const;
            double get_lz() const;
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            void map_variables(std::map<std::string,double*> &variables_map);
            Vector3 normal(RayInter const &inter) const;
            double& ref_lx();
            double& ref_ly();
            double& ref_lz();
            void set_parameters(double lx,
                                double ly,
                                double lz);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
            double lx,ly,lz;
    };

    class Cone: public Primitive
    {
        public:
            double radius,length,cut_factor;
            double cone_ca,cone_sa;
            
            Cone(BoundingBox &bbox,
                 std::vector<Sel::SelFace> &F_arr,
                 std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double radius,
                                double length,
                                double cut);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
    };

    class Conic: public Primitive
    {
        public:
            double R_factor,K_factor;
            double in_radius,out_radius;

            Conic(BoundingBox &bbox,
                  std::vector<Sel::SelFace> &F_arr,
                  std::vector<std::string> &face_name_arr);

            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double lx,
                                double ly,
                                double lz);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;

        private:
    };

    class Cylinder: public Primitive
    {
        public:
            double radius,length,cut_factor;

            Cylinder(BoundingBox &bbox,
                     std::vector<Sel::SelFace> &F_arr,
                     std::vector<std::string> &face_name_arr);

            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double length, double radius, double cut);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;

        private:
    };

    class Disk: public Primitive
    {
        public:
            double radius,in_radius;

            Disk(BoundingBox &bbox,
                 std::vector<Sel::SelFace> &F_arr,
                 std::vector<std::string> &face_name_arr);

            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double lx,
                                double ly,
                                double lz);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;

        private:
    };

    class Lens: public Primitive
    {
        public:
            double thickness,
                   radius_front,
                   radius_back,
                   max_outer_radius;
        
            Lens(BoundingBox &bbox,
                 std::vector<Sel::SelFace> &F_arr,
                 std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double thickness,
                                double r_max,
                                double r1,
                                double r2);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
            double cth_front,
                   cth_back,
                   cylinder_length,
                   outer_radius;

            Vector3 center_front,
                    center_back,
                    normal_front,
                    normal_back,
                    cylinder_origin,
                    cylinder_direction;
    };

    class Mesh: public Primitive
    {
        public:
            Mesh(BoundingBox &bbox,
                 std::vector<Sel::SelFace> &F_arr,
                 std::vector<std::string> &face_name_arr);
                
            void add_mesh(std::vector<Sel::Vertex> const &V_arr,
                          std::vector<Sel::SelFace> const &F_arr);
            void auto_recalc_normals();
            void compute_boundaries();
            void define_faces_group(int index,int start,int end);
            SelFace& faces_group(int index);
            void finalize();
            std::vector<Sel::SelFace> const& get_faces_array() const;
            void get_faces_groups(std::vector<Sel::SelFace> &faces,
                                  std::vector<int> &groups_starts,
                                  std::vector<int> &groups_ends) const;
            std::filesystem::path get_mesh_path() const;
            int get_N_faces_groups() const;
            double get_scaling_factor() const;
            bool get_scaling_status() const;
            std::vector<Sel::Vertex> const& get_vertex_array() const;
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward);
            void propagate_faces_group(int index);
            void propagate_faces_groups();
            void recalc_normals_z();
            void rescale_mesh(double scaling_factor);
            void save_mesh_to_obj(std::string const &fname) const;
            void set_group_default_in_irf(IRF *irf);
            void set_group_default_in_mat(Material *mat);
            void set_group_default_irf(IRF *irf);
            void set_group_default_out_irf(IRF *irf);
            void set_group_default_out_mat(Material *mat);
            void set_faces_groups(std::vector<Sel::SelFace> const &faces,
                                  std::vector<int> const &groups_starts,
                                  std::vector<int> const &group_sends);
            void set_mesh(std::vector<Sel::Vertex> const &V_arr,
                          std::vector<Sel::SelFace> const &F_arr);
            void set_mesh_path(std::filesystem::path const &path);
            void set_scaling_factor(double scaling_factor);
            void set_scaling_status(bool scaling_status);
            Vertex& vertex(int index);
                           
        private:
            bool scaled_mesh;
            double scaling_factor;
            std::vector<Sel::SelFace> Fg_arr;
            std::vector<int> Fg_start,Fg_end;
            std::vector<Sel::Vertex> V_arr;
            std::filesystem::path mesh_fname;

            bool has_octree;
            FOctree octree;
            std::vector<int> octree_buffer;
            std::vector<RayFaceIntersect> face_intersect_buffer;
    };

    class Parabola: public Primitive
    {
        public:
            double focal,inner_radius,length;
            
            Parabola(BoundingBox &bbox,
                     std::vector<Sel::SelFace> &F_arr,
                     std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double lx,
                                double ly,
                                double lz);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
    };

    class Rectangle: public Primitive
    {
        public:
            double ly,lz;
            
            Rectangle(BoundingBox &bbox,
                      std::vector<Sel::SelFace> &F_arr,
                      std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            Vector3 normal(RayInter const &inter) const;
            void set_parameters(double ly,
                                double lz);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
    };

    class Sphere: public Primitive
    {
        public:
            Sphere(BoundingBox &bbox,
                   std::vector<Sel::SelFace> &F_arr,
                   std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            double get_cut_factor() const;
            double get_radius() const;
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            void map_variables(std::map<std::string,double*> &variables_map);
            Vector3 normal(RayInter const &inter) const;
            double& ref_cut_factor();
            double& ref_radius();
            void set_parameters(double radius, double cut_factor);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
            double radius,cut_factor;
            double cut_x,
                   cut_angle,
                   cut_radius;
    };

    class Spherical_Patch: public Primitive
    {
        public:
            Spherical_Patch(BoundingBox &bbox,
                            std::vector<Sel::SelFace> &F_arr,
                            std::vector<std::string> &face_name_arr);
                
            Vector3 anchor(int anchor) const;
            std::string anchor_name(int anchor) const;
            void default_N_uv(int &Nu, int &Nv, int face) const;
            void finalize();
            double get_cut_factor() const;
            double get_radius() const;
            void intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect,bool first_forward) const;
            void map_variables(std::map<std::string,double*> &variables_map);
            Vector3 normal(RayInter const &inter) const;
            double& ref_cut_factor();
            double& ref_radius();
            void set_parameters(double radius, double cut_factor);
            Vector3 tangent(RayInter const &inter,
                            Vector3 const &normal,
                            bool up) const;
            void xyz_to_uv(double &u, double &v, int face,
                           double x, double y, double z) const;
                           
        private:
            double radius,cut_factor;
            double cut_x,
                   cut_angle,
                   cut_radius;
    };
}

#endif // SELENE_PRIMITIVES_H
