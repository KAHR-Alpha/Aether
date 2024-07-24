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
#include <selene_primitives.h>

extern std::ofstream plog;

namespace Sel::Primitives
{
    //###############
    //   Rectangle
    //###############
    
    Rectangle::Rectangle(BoundingBox &bbox_,
                         std::vector<Sel::SelFace> &F_arr_,
                         std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         ly(0.1), lz(0.1)
    {
    }
    
    
    /*Vector3 Rectangle::anchor(int anchor) const
    {
        // Todo
    }*/
    
    
    /*std::string Rectangle::anchor_name(int anchor) const
    {
        // Todo
    }*/
    
    
    void Rectangle::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        double delta=std::max(ly, lz)/64.0;

        Nu=nearest_2np1(ly/delta);
        Nv=nearest_2np1(lz/delta);
    }
    
    
    void Rectangle::finalize()
    {
        int NFc=1;
        F_arr.resize(NFc);

        double span=std::max(ly, lz);

        bbox.xm=-0.05*span;
        bbox.xp=+0.05*span;

        bbox.ym=-1.1*ly/2.0;
        bbox.yp=+1.1*ly/2.0;

        bbox.zm=-1.1*lz/2.0;
        bbox.zp=+1.1*lz/2.0;
        
        // Todo
        /*face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    double Rectangle::get_ly() const { return ly; }
    double Rectangle::get_lz() const { return lz; }
    
    
    void Rectangle::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,1> hits;
        std::array<int,1> face_labels={0};

        if(!ray_inter_plane_x(ray.start,ray.dir,0,hits[0])) hits[0]=-1;
        else
        {
            Vector3 P=ray.start+hits[0]*ray.dir;

            if(std::abs(P.y)>ly/2.0 || std::abs(P.z)>lz/2.0)
                hits[0]=-1;
        }

        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    
    
    Vector3 Rectangle::normal(RayInter const &inter) const
    {
        return -unit_vec_x;
    }
    
    
    double& Rectangle::ref_ly() { return ly; }
    double& Rectangle::ref_lz() { return lz; }
    
    
    void Rectangle::set_parameters(double ly_, double lz_)
    {
        ly = ly_;
        lz = lz_;
    }
    
    
    Vector3 Rectangle::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        Sel::SelFace &face=F_arr[0];

        int tangent_type;
        Vector3 tangent;

        if(up)
        {
            tangent_type=face.tangent_up;
            tangent=face.fixed_tangent_up;
        }
        else
        {
            tangent_type=face.tangent_down;
            tangent=face.fixed_tangent_down;
        }

        if(tangent_type==TANGENT_UNSET) return Vector3(0);
        else if(tangent_type==TANGENT_FIXED)
        {
            tangent=tangent-normal*scalar_prod(normal,tangent);
            tangent.normalize();
        }
        else
        {
            switch(tangent_type)
            {
                // Todo
            }
        }

        return tangent;
    }
    
    
    void Rectangle::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        u=0.5+y/ly;
        v=0.5+z/lz;
    }
}
