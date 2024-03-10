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

#ifndef SELENE_RAYS_H
#define SELENE_RAYS_H

#include <geometry.h>
#include <mathUT.h>

namespace Sel
{
    class SelRay
    {
        public:
            unsigned int source_ID,family,generation;
            double age;
            double lambda;
            double phase;
            Imdouble n_ind;

            Vector3 start,dir,inv_dir,pol,prev_start;

            SelRay();
            SelRay(SelRay const &ray);

            void operator = (SelRay const &ray);

            void set_dir(Vector3 const &dir);
            void set_pol(Vector3 const &pol);
            void set_start(Vector3 const &start);
    };

    class RayInter
    {
        public:
    //        bool does_intersect;
            double obj_x,obj_y,obj_z,t;
            int face,object;

            RayInter();
            RayInter(RayInter const &inter);

            void operator = (RayInter const &inter);
            bool operator < (RayInter const &inter);
            void reset();
    };

    class RayPath
    {
        public:
            bool complete;
            bool does_intersect;
            int face_last_intersect,obj_last_intersection_f;

            SelRay ray;
            RayInter intersection;

            RayPath();
            RayPath(RayPath const &path);

            void operator = (RayPath const &path);
    };
}

#endif // SELENE_RAYS_H
