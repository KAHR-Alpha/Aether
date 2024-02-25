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
        
        double span_x() { return xp-xm; }
        double span_y() { return yp-ym; }
        double span_z() { return zp-zm; }
    };
    
    int nearest_2np1(double val);
}

namespace Sel::Primitives
{
    class Box
    {
        public:
            double lx,ly,lz;
            
            Box(BoundingBox &bbox,
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
            BoundingBox &bbox;
            std::vector<Sel::SelFace> &F_arr;
            std::vector<std::string> &face_name_arr;
    };

    class Cone
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
            BoundingBox &bbox;
            std::vector<Sel::SelFace> &F_arr;
            std::vector<std::string> &face_name_arr;
    };

    class Conic
    {
        public:
            double conic_R,conic_K;
            double conic_in_radius,conic_out_radius;

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
            BoundingBox &bbox;
            std::vector<Sel::SelFace> &F_arr;
            std::vector<std::string> &face_name_arr;
    };
}

#endif // SELENE_PRIMITIVES_H
