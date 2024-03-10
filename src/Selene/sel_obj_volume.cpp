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

#include <selene.h>

extern std::ofstream plog;

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel
{
    int nearest_2np1(double val)
    {
        int N=nearest_integer((val-1.0)/2.0);
        return 2*N+1;
    }
    
    
    void Object::set_box()
    {
        type=OBJ_BOX;
        
        box.finalize();
        NFc=F_arr.size();
    }
    
    
    void Object::set_box(double lx, double ly, double lz)
    {
        box.set_parameters(lx, ly, lz);
        
        set_box();
    }

//##############
//   Booleans
//##############

void Object::intersect_boolean(SelRay const &ray,std::vector<RayInter> &interlist,int face_last_intersect,bool first_forward)
{
    int i;
    bool in_obj_1=false,in_obj_2=false;
    
    // Getting all the intersections of both object
    
    bool_buffer_1.clear();
    bool_buffer_2.clear();
    
    if(face_last_intersect>=bool_obj_1->NFc)
    {
        face_last_intersect-=bool_obj_1->NFc;
        bool_obj_1->intersect(ray,bool_buffer_1,-1,false);
        bool_obj_2->intersect(ray,bool_buffer_2,face_last_intersect,false);
    }
    else
    {
        bool_obj_1->intersect(ray,bool_buffer_1,face_last_intersect,false);
        bool_obj_2->intersect(ray,bool_buffer_2,-1,false);
    }
    
    int N1=bool_buffer_1.size();
    int N2=bool_buffer_2.size();
    
    if(N1==0) return;
    if(boolean_type==INTERSECT && N2==0) return;
    
    //Merging intersections
    std::vector<RayInter> &full_buffer=bool_buffer_1;
    
    full_buffer.resize(N1+N2);
    
    for(i=0;i<N2;i++)
        full_buffer[i+N1]=bool_buffer_2[i];
    
    std::sort(full_buffer.begin(),full_buffer.end());
    
    in_obj_1=(N1%2!=0);
    in_obj_2=(N2%2!=0);
    
    RayInter inter_out;
    bool check_1=true,check_2=true; // Intersection
    
         if(boolean_type==EXCLUDE) check_2=false;
    else if(boolean_type==UNION) check_1=check_2=false;
    
    if(first_forward)
    {
        bool second_hit=false;
        
        for(i=0;i<N1+N2;i++)
        {
            if(full_buffer[i].object==0)
            {
                if(in_obj_2==check_2) { inter_out=full_buffer[i]; break; }
                in_obj_1=!in_obj_1;
            }
            else
            {
                if(in_obj_1==check_1) { inter_out=full_buffer[i]; second_hit=true; break; }
                in_obj_2=!in_obj_2;
            }
        }
        
        inter_out.object=obj_ID;
        if(second_hit) inter_out.face+=bool_obj_1->NFc;
        
        interlist.push_back(inter_out);
    }
    else
    {
        for(i=0;i<N1+N2;i++)
        {
            if(full_buffer[i].object==0)
            {
                if(in_obj_2==check_2)
                {
                    inter_out=full_buffer[i];
                    inter_out.object=obj_ID;
                    interlist.push_back(inter_out);
                }
                in_obj_1=!in_obj_1;
            }
            else
            {
                if(in_obj_1==check_1)
                {
                    inter_out=full_buffer[i];
                    inter_out.object=obj_ID;
                    inter_out.face+=bool_obj_1->NFc;
                    interlist.push_back(inter_out);
                }
                in_obj_2=!in_obj_2;
            }
        }
    }
}

Vector3 Object::normal_boolean(RayInter inter)
{
    int &face_inter=inter.face;
    
    if(face_inter<bool_obj_1->NFc) return bool_obj_1->face_normal(inter);
    else
    {
        face_inter-=bool_obj_1->NFc;
        return bool_obj_2->face_normal(inter);
    }
}

void Object::set_boolean(Object *bool_obj_1_,Object *bool_obj_2_,Boolean_Type type_)
{
    type=OBJ_BOOLEAN;
    boolean_type=type_;
    
    bool_obj_1=bool_obj_1_;
    bool_obj_2=bool_obj_2_;
    
    bool_obj_1->obj_ID=0;
    bool_obj_2->obj_ID=1;
    
    NFc=bool_obj_1->NFc+bool_obj_2->NFc;
}

    //##########
    //   Cone
    //##########
    
    void Object::set_cone_volume()
    {
        type=OBJ_VOL_CONE;
        
        cone.finalize();
        NFc=F_arr.size();
    }
    
    
    void Object::set_cone_volume(double radius,double length,double cut)
    {
        cone.set_parameters(radius, length, cut);
        
        set_cone_volume();
    }
    
    
    //##############
    //   Cylinder
    //##############

    void Object::set_cylinder_volume()
    {
        type=OBJ_VOL_CYLINDER;
        
        cylinder.finalize();
        NFc = F_arr.size();
    }
    
    
    void Object::set_cylinder_volume(double length, double radius, double cut)
    {
        cylinder.set_parameters(length, radius, cut);
        
        set_cylinder_volume();
    }
    
    
//##########
//   Lens
//##########

void Object::set_lens()
{
    type=OBJ_LENS;
    
    lens.finalize();
    NFc = F_arr.size();
}

void Object::set_lens(double thickness,double r_max,double r1,double r2)
{
    lens.set_parameters(thickness, r_max, r1, r2);
    
    set_lens();
}


//##########
//   Mesh
//##########

void Object::set_mesh()
{
    type=OBJ_MESH;
    
    mesh.finalize();
    NFc=F_arr.size();
}

void Object::set_mesh(std::vector<Sel::Vertex> const &V_arr_,std::vector<Sel::SelFace> const &F_arr_)
{
    mesh.set_mesh(V_arr_, F_arr_);
    
    set_mesh();
}

//############
//   Prism
//############

//############
//   Sphere
//############

void Object::set_sphere()
{
    type=OBJ_SPHERE;
    
    sphere.finalize();
    NFc = F_arr.size();
}


void Object::set_sphere(double radius, double slice)
{
    sphere.set_parameters(radius, slice);
    
    set_sphere();
}
}
