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
    //#########
    //   Box
    //#########
    
    Box::Box(BoundingBox &bbox_,
             std::vector<Sel::SelFace> &F_arr_,
             std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         lx(0.1), ly(0.1), lz(0.1)
    {
    }
    
    
    Vector3 Box::anchor(int anchor) const
    {
        double x=lx/2.0;
        double y=ly/2.0;
        double z=lz/2.0;
        
        switch(anchor)
        {
            case 0:  return Vector3(0);
            case 1:  return Vector3(-x,0,0);
            case 2:  return Vector3(+x,0,0);
            case 3:  return Vector3(0,-y,0);
            case 4:  return Vector3(0,+y,0);
            case 5:  return Vector3(0,0,-z);
            case 6:  return Vector3(0,0,+z);
            case 7:  return Vector3(-x,-y,-z);
            case 8:  return Vector3(+x,-y,-z);
            case 9:  return Vector3(-x,+y,-z);
            case 10: return Vector3(+x,+y,-z);
            case 11: return Vector3(-x,-y,+z);
            case 12: return Vector3(+x,-y,+z);
            case 13: return Vector3(-x,+y,+z);
            case 14: return Vector3(+x,+y,+z);
            default: return Vector3(0);
        }
    }
    
    
    std::string Box::anchor_name(int anchor) const
    {
        switch(anchor)
        {
            case 0: return "Center";
            case 1: return "Face_XM";
            case 2: return "Face_XP";
            case 3: return "Face_YM";
            case 4: return "Face_YP";
            case 5: return "Face_ZM";
            case 6: return "Face_ZP";
            case 7: return "Corner_XM_YM_ZM";
            case 8: return "Corner_XP_YM_ZM";
            case 9: return "Corner_XM_YP_ZM";
            case 10: return "Corner_XP_YP_ZM";
            case 11: return "Corner_XM_YM_ZP";
            case 12: return "Corner_XP_YM_ZP";
            case 13: return "Corner_XM_YP_ZP";
            case 14: return "Corner_XP_YP_ZP";
            default: return "Center";
        }
    }
    
    
    void Box::default_N_uv(int &Nu,int &Nv,int face_) const
    {
        double delta=var_max(lx,ly,lz)/64.0;
        
        int Nx=nearest_2np1(lx/delta);
        int Ny=nearest_2np1(ly/delta);
        int Nz=nearest_2np1(lz/delta);
        
        switch(face_)
        {
            case 0:
                Nu=Ny; Nv=Nz; break;
            case 1:
                Nu=Ny; Nv=Nz; break;
            case 2:
                Nu=Nx; Nv=Nz; break;
            case 3:
                Nu=Nx; Nv=Nz; break;
            case 4:
                Nu=Nx; Nv=Ny; break;
            case 5:
                Nu=Nx; Nv=Ny; break;
        }
    }
    
    
    void Box::finalize()
    {
        int NFc=6;
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
        face_name_arr[5]="Face ZP";
    }


    double Box::get_lx() const { return lx; }
    double Box::get_ly() const { return ly; }
    double Box::get_lz() const { return lz; }
    
    
    void Box::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward) const
    {
        std::array<double,6> hits;
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
            push_full_forward(interlist,ray,obj_ID,hits,face_labels);
    }
    

    void Box::map_variables(std::map<std::string,double*> &variables_map)
    {
        variables_map["box_length_x"]=&lx;
        variables_map["box_length_y"]=&ly;
        variables_map["box_length_z"]=&lz;
    }

    
    Vector3 Box::normal(RayInter const &inter) const
    {
        Vector3 Fnorm;
        int const &face_inter=inter.face;
        
        if(face_inter==0) Fnorm=-unit_vec_x;
        else if(face_inter==1) Fnorm= unit_vec_x;
        else if(face_inter==2) Fnorm=-unit_vec_y;
        else if(face_inter==3) Fnorm= unit_vec_y;
        else if(face_inter==4) Fnorm=-unit_vec_z;
        else if(face_inter==5) Fnorm= unit_vec_z;
        
        return Fnorm;
    }


    double& Box::ref_lx() { return lx; }
    double& Box::ref_ly() { return ly; }
    double& Box::ref_lz() { return lz; }
    
    
    void Box::set_parameters(double lx_, double ly_, double lz_)
    {
        lx = lx_;
        ly = ly_;
        lz = lz_;
    }
    
    
    Vector3 Box::tangent(RayInter const &inter,Vector3 const &normal,bool up) const
    {
        int const &face_inter=inter.face;
        
        Sel::SelFace const &face=F_arr[face_inter];
        
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
            }
        }
        
        return tangent;
    }
    
    
    void Box::xyz_to_uv(double &u,double &v,int face_,double x,double y,double z) const
    {
        switch(face_)
        {
            case 0:
                u=y/ly; v=z/lz;
                break;
            case 1:
                u=y/ly; v=z/lz;
                break;
            case 2:
                u=x/lx; v=z/lz;
                break;
            case 3:
                u=x/lx; v=z/lz;
                break;
            case 4:
                u=x/lx; v=y/ly;
                break;
            case 5:
                u=x/lx; v=y/ly;
                break;
        }
        
        u+=0.5;
        v+=0.5;
    }
}
