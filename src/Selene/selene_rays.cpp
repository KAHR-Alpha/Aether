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

#include <selene_rays.h>

namespace Sel
{
    //####################
    //     SelRay
    //####################

    SelRay::SelRay()
        :source_ID(0), family(0), generation(0), age(0),
         lambda(500e-9), phase(0), n_ind(1.0),
         start(0,0,0), dir(0,0,1), inv_dir(1e100,1e100,1), pol(0,1,0), prev_start(0,0,0)
    {
    }

    SelRay::SelRay(SelRay const &ray)
        :source_ID(ray.source_ID), family(ray.family), generation(ray.generation), age(ray.age),
         lambda(ray.lambda), phase(ray.phase), n_ind(ray.n_ind),
         start(ray.start), dir(ray.dir), inv_dir(ray.inv_dir), pol(ray.pol), prev_start(ray.prev_start)
    {
    }

    void SelRay::operator = (SelRay const &ray)
    {
        source_ID=ray.source_ID;
        family=ray.family;
        generation=ray.generation;
        age=ray.age;

        lambda=ray.lambda;
        phase=ray.phase;
        n_ind=ray.n_ind;

        start=ray.start;
        dir=ray.dir;
        inv_dir=ray.inv_dir;
        pol=ray.pol;
        prev_start=ray.prev_start;
    }

    void SelRay::set_dir(Vector3 const &dir_)
    {
        dir=dir_;
        inv_dir.x=1.0/dir.x;
        inv_dir.y=1.0/dir.y;
        inv_dir.z=1.0/dir.z;
    }

    void SelRay::set_pol(Vector3 const &pol_)
    {
        pol=pol_;
    }

    void SelRay::set_start(Vector3 const &start_)
    {
        start=start_;
    }

    //####################
    //     RayInter
    //####################

    RayInter::RayInter()
        ://does_intersect(false),
         obj_x(0), obj_y(0), obj_z(0), t(0),
         face(-1), object(-1)
    {
    }

    RayInter::RayInter(RayInter const &inter)
        ://does_intersect(inter.does_intersect),
         obj_x(inter.obj_x),
         obj_y(inter.obj_y),
         obj_z(inter.obj_z),
         t(inter.t),
         face(inter.face),
         object(inter.object)
    {
    }

    void RayInter::operator = (RayInter const &inter)
    {
    //    does_intersect=inter.does_intersect;
        obj_x=inter.obj_x;
        obj_y=inter.obj_y;
        obj_z=inter.obj_z;
        t=inter.t;
        face=inter.face;
        object=inter.object;
    }

    bool RayInter::operator < (RayInter const &inter)
    {
        if(t==inter.t) return object<inter.object;

        return t<inter.t;
    }

    void RayInter::reset()
    {
    //    does_intersect=false;

        obj_x=0;
        obj_y=0;
        obj_z=0;
        t=0;

        face=-1;
        object=-1;
    }

    //####################
    //     RayPath
    //####################

    RayPath::RayPath()
        :complete(false),
         does_intersect(false),
         face_last_intersect(-1), obj_last_intersection_f(-1)
    {
    }

    RayPath::RayPath(RayPath const &path)
        :complete(path.complete),
         does_intersect(path.does_intersect),
         face_last_intersect(path.face_last_intersect),
         obj_last_intersection_f(path.obj_last_intersection_f),
         ray(path.ray),
         intersection(path.intersection)
    {
    }

    void RayPath::operator = (RayPath const &path)
    {
        complete=path.complete;
        does_intersect=path.does_intersect;
        face_last_intersect=path.face_last_intersect;
        obj_last_intersection_f=path.obj_last_intersection_f;
        ray=path.ray;
        intersection=path.intersection;
    }
}
