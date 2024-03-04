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

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

namespace Sel::Primitives
{
    //##########
    //   Mesh
    //##########
    
    Mesh::Mesh(BoundingBox &bbox_,
               std::vector<Sel::SelFace> &F_arr_,
               std::vector<std::string> &face_name_arr_)
        :Primitive(bbox_, F_arr_, face_name_arr_),
         scaled_mesh(false), scaling_factor(1.0), has_octree(false)
    {
    }
    
    
    void Mesh::add_mesh(std::vector<Sel::Vertex> const &V_arr_,
                        std::vector<Sel::SelFace> const &F_arr_)
    {
        int NVtx=V_arr.size();
        int NFc=F_arr.size();
        
        int aNVtx=V_arr_.size();
        int aNFc=F_arr_.size();
        
        V_arr.resize(NVtx+aNVtx);
        F_arr.resize(NFc+aNFc);
        
        for(int i=NVtx;i<NVtx+aNVtx;i++)
            V_arr[i]=V_arr_[i-NVtx];
            
        for(int i=NFc;i<NFc+aNFc;i++)
        {
            F_arr[i]=F_arr_[i-NFc];
            
            F_arr[i].V1+=NVtx;
            F_arr[i].V2+=NVtx;
            F_arr[i].V3+=NVtx;
        }
        
        compute_boundaries();
    }
    
    
    void Mesh::auto_recalc_normals()
    {
        int i;
        
        Vector3 O,V;
        
        for(i=0;i<F_arr.size();i++)
        {
            F_arr[i].comp_norm(V_arr);
            
            O=F_arr[i].get_cmass(V_arr);
            V.rand_sph(0.1);
            V+=F_arr[i].norm;
            V.normalize();
            
            int N_inter_1=0;
            
            if(!has_octree)
            {
                N_inter_1=ray_N_inter(V_arr,F_arr,i,O, V);
                //            int N_inter_2=ray_N_inter(V_arr,F_arr,i,O,-V);
            }
            else
            {
                SelRay ray;
                std::list<int> flist;
                
                ray.set_dir(V);
                ray.set_start(O);
                
                octree.ray_check(ray,flist);
                N_inter_1=ray_N_inter_list(V_arr,F_arr,flist,i,O,V);
                
                if(N_inter_1==2)
                {
                    
                }
            }
            
            if(N_inter_1%2!=0) F_arr[i].norm=-F_arr[i].norm;
        }
    }
    
    
    void Mesh::compute_boundaries()
    {
        int NFc = F_arr.size();
        
        if(V_arr.size()==0)
        {
            bbox.xm=bbox.xp=0;
            bbox.ym=bbox.yp=0;
            bbox.zm=bbox.zp=0;
            
            return;
        }
        
        bbox.xm=bbox.xp=V_arr[0].loc.x;
        bbox.ym=bbox.yp=V_arr[0].loc.y;
        bbox.zm=bbox.zp=V_arr[0].loc.z;
        
        for(std::size_t i=0;i<V_arr.size();i++)
        {
            bbox.xm=std::min(bbox.xm,V_arr[i].loc.x);
            bbox.ym=std::min(bbox.ym,V_arr[i].loc.y);
            bbox.zm=std::min(bbox.zm,V_arr[i].loc.z);

            bbox.xp=std::max(bbox.xp,V_arr[i].loc.x);
            bbox.yp=std::max(bbox.yp,V_arr[i].loc.y);
            bbox.zp=std::max(bbox.zp,V_arr[i].loc.z);
        }
        
        double spanx=bbox.xp-bbox.xm;
        double spany=bbox.yp-bbox.ym;
        double spanz=bbox.zp-bbox.zm;
        
        double span_max=var_max(spanx,
                                spany,
                                spanz);
                                
        spanx=std::max(spanx,0.1*span_max);
        spany=std::max(spany,0.1*span_max);
        spanz=std::max(spanz,0.1*span_max);
        
        bbox.xm-=0.05*spanx; bbox.xp+=0.05*spanx;
        bbox.ym-=0.05*spany; bbox.yp+=0.05*spany;
        bbox.zm-=0.05*spanz; bbox.zp+=0.05*spanz;
        
        if(NFc>12)
        {
            has_octree=true;
            octree.clear_tree();
            octree.set_params(8,bbox.xm,bbox.xp,bbox.ym,bbox.yp,bbox.zm,bbox.zp);
            octree.generate_tree(V_arr,F_arr);
        }
    }
    
    
    void Mesh::define_faces_group(int index,int start,int end)
    {
        if(index<0) return;
        
        if(static_cast<int>(Fg_arr.size())<=index)
        {
            Fg_arr.resize(index+1);
            Fg_start.resize(index+1);
            Fg_end.resize(index+1);
        }
        
        Fg_start[index]=start;
        Fg_end[index]=end;
    }
    
    
    SelFace& Mesh::faces_group(int index)
    {
        return Fg_arr[index];
    }
    
    
    void Mesh::finalize()
    {        
        compute_boundaries();
        
        if(Fg_arr.size()==0) // Default group
            define_faces_group(0,0,F_arr.size()-1);
    }
    
    
    std::vector<Sel::SelFace> const& Mesh::get_faces_array() const
    {
        return F_arr;
    }
    
    
    void Mesh::get_faces_groups(std::vector<Sel::SelFace> &faces_,
                                std::vector<int> &groups_starts_,
                                std::vector<int> &groups_ends_) const
    {
        faces_ = Fg_arr;
        groups_starts_ = Fg_start;
        groups_ends_ = Fg_end;
    }
    
    
    std::filesystem::path Mesh::get_mesh_path() const
    {
        return mesh_fname;
    }
    
    
    int Mesh::get_N_faces_groups() const
    {
        return Fg_arr.size();
    }
    
    
    double Mesh::get_scaling_factor() const
    {
        return scaling_factor;
    }
    
    
    bool Mesh::get_scaling_status() const
    {
        return scaled_mesh;
    }
    
    
    std::vector<Sel::Vertex> const& Mesh::get_vertex_array() const
    {
        return V_arr;
    }
    
    
    void Mesh::intersect(std::vector<RayInter> &interlist, SelRay const &ray, int obj_ID, int face_last_intersect, bool first_forward)
    {
        if(first_forward)
        {
            int face_hit=-1;
            double t_intersec,u,v;
            
            if(!has_octree)
            {
                ray_inter(V_arr,F_arr,face_last_intersect,
                          ray.start,ray.dir,
                          face_hit,t_intersec,u,v);
            }
            else
            {
                octree.ray_check(ray,octree_buffer);
                ray_inter_vector(V_arr,F_arr,octree_buffer,face_last_intersect,
                                 ray.start,ray.dir,
                                 face_hit,t_intersec,u,v);
            }
            
            if(face_hit>-1)
            {
                Vector3 V=ray.start+t_intersec*ray.dir;
                
                RayInter inter_out;

                //            inter_out.does_intersect=true;
                inter_out.obj_x=V.x;
                inter_out.obj_y=V.y;
                inter_out.obj_z=V.z;
                inter_out.t=t_intersec;
                inter_out.face=face_hit;
                inter_out.object=obj_ID;
                
                interlist.push_back(inter_out);
            }
        }
        else
        {
            face_intersect_buffer.clear();
            
            if(!has_octree)
            {
                ray_inter(V_arr,F_arr,face_last_intersect,
                          ray.start,ray.dir,face_intersect_buffer);
            }
            else
            {
                octree.ray_check(ray,octree_buffer);
                ray_inter_vector(V_arr,F_arr,octree_buffer,face_last_intersect,
                                 ray.start,ray.dir,face_intersect_buffer);
            }
            
            for(unsigned int i=0;i<face_intersect_buffer.size();i++)
            {
                RayFaceIntersect &inter=face_intersect_buffer[i];
                
                Vector3 V=ray.start+inter.t_intersect*ray.dir;
                
                RayInter inter_out;

                //            inter_out.does_intersect=true;
                inter_out.obj_x=V.x;
                inter_out.obj_y=V.y;
                inter_out.obj_z=V.z;
                inter_out.t=inter.t_intersect;
                inter_out.face=inter.ftarget;
                inter_out.object=obj_ID;
                
                interlist.push_back(inter_out);
            }
        }
    }
    
    
    void Mesh::propagate_faces_group(int index)
    {
        int start=std::max(0,Fg_start[index]);
        int end=std::min(Fg_end[index],static_cast<int>(F_arr.size()-1));
        chk_var(start);
        chk_var(end);
        chk_var(Fg_start.size());
        chk_var(Fg_end.size());
        for(int i=start;i<=end;i++)
        {
            F_arr[i].up_mat=Fg_arr[index].up_mat;
            F_arr[i].down_mat=Fg_arr[index].down_mat;
            
            F_arr[i].up_irf=Fg_arr[index].up_irf;
            F_arr[i].down_irf=Fg_arr[index].down_irf;
            
            F_arr[i].tangent_up=Fg_arr[index].tangent_up;
            F_arr[i].tangent_down=Fg_arr[index].tangent_down;
            
            F_arr[i].fixed_tangent_up=Fg_arr[index].fixed_tangent_up;
            F_arr[i].fixed_tangent_down=Fg_arr[index].fixed_tangent_down;
        }
    }
    
    
    void Mesh::propagate_faces_groups()
    {
        for(std::size_t i=0; i<Fg_arr.size(); i++)
            propagate_faces_group(i);
    }
    
    
    void Mesh::recalc_normals_z()
    {
        for(std::size_t i=0;i<F_arr.size();i++)
        {
            F_arr[i].comp_norm(V_arr);
            
            if(F_arr[i].norm.z<0)
                F_arr[i].norm=-F_arr[i].norm;
        }
    }
    
    
    void Mesh::rescale_mesh(double scaling_factor_)
    {
        scaling_factor=scaling_factor_;
        
        for(std::size_t i=0;i<V_arr.size();i++)
            V_arr[i].loc*=scaling_factor;
            
        scaled_mesh=true;
        
        compute_boundaries();
    }
    
    
    void Mesh::set_group_default_in_irf(IRF *irf)
    {
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].down_irf=irf;
    }
    
    
    void Mesh::set_group_default_in_mat(Material *mat)
    {
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].down_mat=mat;
    }
    
    
    void Mesh::set_group_default_irf(IRF *irf)
    {
        for(std::size_t i=0;i<Fg_arr.size();i++)
        {
            Fg_arr[i].down_irf=irf;
            Fg_arr[i].up_irf=irf;
        }
    }
    
    
    void Mesh::set_group_default_out_irf(IRF *irf)
    {
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].up_irf=irf;
    }
    
    
    void Mesh::set_group_default_out_mat(Material *mat)
    {
        for(std::size_t i=0;i<Fg_arr.size();i++)
            Fg_arr[i].up_mat=mat;
    }
    
    
    void Mesh::set_mesh_path(std::filesystem::path const &mesh_path_)
    {
        mesh_fname = mesh_path_;
    }
    
    
    void Mesh::set_scaling_factor(double scaling_factor_)
    {
        scaling_factor = scaling_factor_;
    }
    
    
    void Mesh::set_scaling_status(bool scaling_status_)
    {
        scaled_mesh = scaling_status_;
    }
    
    
    void Mesh::save_mesh_to_obj(std::string const &fname) const
    {
        obj_file_save(fname,V_arr,F_arr);
    }
    
    
    void Mesh::set_faces_groups(std::vector<Sel::SelFace> const &faces,
                                std::vector<int> const &groups_starts,
                                std::vector<int> const &groups_ends)
    {
        Fg_arr=faces;
        Fg_start=groups_starts;
        Fg_end=groups_ends;
    }
    
    
    void Mesh::set_mesh(std::vector<Sel::Vertex> const &V_arr_,
                        std::vector<Sel::SelFace> const &F_arr_)
    {
        V_arr=V_arr_;
        F_arr=F_arr_;
        
        finalize();
    }
    
    Vertex& Mesh::vertex(int index) { return V_arr[index]; }
}
