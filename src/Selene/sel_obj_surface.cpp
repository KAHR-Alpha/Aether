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

#include <selene.h>

extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{
    //###################
    //   Conic Section
    //###################

    void Object::set_conic_section()
    {
        type = OBJ_CONIC;
        
        conic.finalize();
        NFc = F_arr.size();
    }

//##########
//   Disk
//##########

void Object::set_disk()
{
    type=OBJ_DISK;
    
    disk.finalize();
    NFc=F_arr.size();
}

void Object::set_disk(double radius_,double in_radius_)
{
    disk.radius=radius_;
    disk.in_radius=in_radius_;
    
    set_disk();
}





//##############
//   Parabola
//##############

void Object::set_parabola()
{
    type=OBJ_PARABOLA;
    
    parabola.finalize();
    NFc = F_arr.size();
}


void Object::set_parabola(double focal_length_,double in_radius_,double length_)
{
    parabola.focal=focal_length_;
    parabola.inner_radius=in_radius_;
    parabola.length=length_;
    
    set_parabola();
}


//###############
//   Rectangle
//###############

void Object::set_rectangle()
{
    type=OBJ_RECTANGLE;
    
    
}


void Object::set_rectangle(double ly, double lz)
{
    rectangle.set_parameters(ly, lz);
    set_rectangle();
}


//#####################
//   Spherical Patch
//#####################

void Object::set_spherical_patch()
{
    type=OBJ_SPHERE_PATCH;
    
    sphere_patch.finalize();
    NFc = F_arr.size();
}


void Object::set_spherical_patch(double radius,double slice)
{
    sphere_patch.set_parameters(radius, slice);
    
    set_spherical_patch();
}





}
