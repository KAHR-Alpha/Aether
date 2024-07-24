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
    //#################
    //   Polynormial
    //#################
    
    Polynomial::Polynomial(BoundingBox &bbox_,
             std::vector<Sel::SelFace> &F_arr_,
             std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         inner_radius(0),
         outer_radius(50e-3),
         coeffs(1, 1),
         effective_coeffs(coeffs),
         normalized(true)
    {
    }
    
    
    Vector3 Polynomial::anchor(int anchor) const
    {
        return Vector3(0);
    }
    
    
    std::string Polynomial::anchor_name(int anchor) const
    {
        return "Center";
    }
    
    
    void Polynomial::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        Nu=Nv=64;
    }


    double Polynomial::eval(double x) const
    {
        double r=0;

        for(double const &c : effective_coeffs)
        {
            r=x*(c+r);
        }

        return r;
    }
    
    
    void Polynomial::finalize()
    {
        /*int NFc=6;
        F_arr.resize(NFc);
        
        bbox.xm=-lx/2.0;
        bbox.xp=+lx/2.0;
        
        bbox.ym=-ly/2.0;
        bbox.yp=+ly/2.0;
        
        bbox.zm=-lz/2.0;
        bbox.zp=+lz/2.0;
        
        face_name_arr.resize(NFc);
        face_name_arr[0]="Face XM";
        face_name_arr[1]="Face XP";
        face_name_arr[2]="Face YM";
        face_name_arr[3]="Face YP";
        face_name_arr[4]="Face ZM";
        face_name_arr[5]="Face ZP";*/
    }
    
    
    void Polynomial::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        /*std::array<double,6> hits;
        std::array<int,6> face_labels={0,1,2,3,4,5};
        
        double &t_x1=hits[0],&t_x2=hits[1];
        double &t_y1=hits[2],&t_y2=hits[3];
        double &t_z1=hits[4],&t_z2=hits[5];
        
        t_x1=(-lx/2.0-ray.start.x)*ray.inv_dir.x;
        t_x2=(+lx/2.0-ray.start.x)*ray.inv_dir.x;
        
        t_y1=(-ly/2.0-ray.start.y)*ray.inv_dir.y;
        t_y2=(+ly/2.0-ray.start.y)*ray.inv_dir.y;
        
        t_z1=(-lz/2.0-ray.start.z)*ray.inv_dir.z;
        t_z2=(+lz/2.0-ray.start.z)*ray.inv_dir.z;
        
        Vector3 V;
        
        V=ray.start+t_x1*ray.dir;
        if(V.y>ly/2.0 || V.y<-ly/2.0 ||
           V.z>lz/2.0 || V.z<-lz/2.0) t_x1=-1;
        V=ray.start+t_x2*ray.dir;
        if(V.y>ly/2.0 || V.y<-ly/2.0 ||
           V.z>lz/2.0 || V.z<-lz/2.0) t_x2=-1;
           
        V=ray.start+t_y1*ray.dir;
        if(V.x>lx/2.0 || V.x<-lx/2.0 ||
           V.z>lz/2.0 || V.z<-lz/2.0) t_y1=-1;
        V=ray.start+t_y2*ray.dir;
        if(V.x>lx/2.0 || V.x<-lx/2.0 ||
           V.z>lz/2.0 || V.z<-lz/2.0) t_y2=-1;
           
        V=ray.start+t_z1*ray.dir;
        if(V.x>lx/2.0 || V.x<-lx/2.0 ||
           V.y>ly/2.0 || V.y<-ly/2.0) t_z1=-1;
        V=ray.start+t_z2*ray.dir;
        if(V.x>lx/2.0 || V.x<-lx/2.0 ||
           V.y>ly/2.0 || V.y<-ly/2.0) t_z2=-1;
           
        if(first_forward)
            push_first_forward(interlist,ray,obj_ID,hits,face_labels);
        else
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);*/
    }
    

    void Polynomial::map_variables(std::map<std::string,double*> &variables_map)
    {
        /*variables_map["box_length_x"]=&lx;
        variables_map["box_length_y"]=&ly;
        variables_map["box_length_z"]=&lz;*/
    }

    
    Vector3 Polynomial::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        /*int const &face_inter=inter.face;
        
        if(face_inter==0) Fnorm=-unit_vec_x;
        else if(face_inter==1) Fnorm= unit_vec_x;
        else if(face_inter==2) Fnorm=-unit_vec_y;
        else if(face_inter==3) Fnorm= unit_vec_y;
        else if(face_inter==4) Fnorm=-unit_vec_z;
        else if(face_inter==5) Fnorm= unit_vec_z;*/
        
        return Fnorm;
    }


    void Polynomial::set_parameters(double outer_radius_,
                                    double inner_radius_,
                                    std::vector<double> const &coeffs_,
                                    bool normalized_)
    {
        outer_radius = outer_radius_;
        inner_radius = inner_radius_;
        coeffs = coeffs_;
        normalized = normalized_;

        effective_coeffs = coeffs;

        if(normalized)
        {
            for(std::size_t i=0; i<effective_coeffs.size(); i++)
            for(std::size_t j=i; j<effective_coeffs.size(); j++)
            {
                effective_coeffs[j] /= outer_radius;
            }
        }
    }

    
    Vector3 Polynomial::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        int const &face_inter=inter.face;
        
        Sel::SelFace const &face=F_arr[face_inter];
        
        int tangent_type;
        Vector3 tangent;
        
        // Todo
        
        return tangent;
    }
    
    
    void Polynomial::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        // Todo
    }
}
