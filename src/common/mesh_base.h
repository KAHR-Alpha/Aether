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

#ifndef MESH_BASE_H_INCLUDED
#define MESH_BASE_H_INCLUDED

#include <mathUT.h>
#include <foctree.h>
#include <ray_intersect.h>

class Vertex
{
    public:
        Vector3 loc,norm;
        
        Vertex();
        Vertex(Vector3 const &V);
        Vertex(double x,double y,double z);
        Vertex(Vertex const &V);
        
        void operator = (double);
        void operator = (Vertex const &);
        
        void set_loc(Vector3 const &V);
        void set_loc(double const &x,double const &y,double const &z);
        void set_norm(Vector3 const &V);
        void set_norm(double const &x,double const &y,double const &z);
        void show();
        void translate(Vector3 const &);
        void translate(double const &x,double const &y,double const &z);
};

class Face
{
    public:
        int V1,V2,V3;
        Vector3 norm;
        std::vector<int> ngb;
        
        Face();
        Face(int a,int b,int c);
        Face(Face const &);
        
        void operator = (Face const &);
        
        void comp_norm(std::vector<Vertex> const &V);
        void comp_norm(Grid1<Vertex> const &V);
        bool degeneracy_chk(int msg_ID);
        void get_edge(int edge,int &Va,int &Vb,int &Vc);
        bool is_connected(Face const &F);
        void set_ngb(int N1,int N2,int N3);
        void set_norm(double x,double y,double z);
        void set_norm(Vector3 const &N);
        void set_Vindex(int const &V1,int const &V2,int const &V3);
        void show();
        double smallest_height(std::vector<Vertex> const &V);
        bool update_ngb(Face const &F,int F_ID);
};

class Mesh
{
    public:
        int Nthr;
        std::vector<Vector2> uv_coords;
        std::vector<Vertex> v_arr;
        std::vector<Face> f_arr;
        
        std::vector<std::vector<int>> flist;
        
        // Convex hull
        
        std::vector<int> hull;
        
        FOctree octree;
        
        Mesh();
        
        void compute_convex_hull();
        void compute_neighbors();
        void construct_delaunay_incremental();
        void delaunay_flip();
        void load_obj(std::string const &fname);
        void rebuild_octree();
        void set_mesh(std::vector<Vertex> const &v_arr,std::vector<Face> const &f_arr);
        void set_threads_number(int Nthr);
        
        template<class TRay>
        void trace(int &ftarget,double &t,double &u,double &v,Vector3 const &O,Vector3 const &D,int fskip=-1,int ID=0)
        {
            TRay ray;
            
            ray.set_dir(D);
            ray.set_start(O);
                
            flist[ID].clear();
                
            octree.ray_check(ray,flist[ID]);
            
            ray_inter_vector(v_arr,f_arr,flist[ID],fskip,O,D,ftarget,t,u,v);
        }
        
        void vertex_interpolate_norm();
};

class QFace
{
    public:
        int V1,V2,V3,V4;
        
        QFace();
        QFace(int a,int b,int c,int d);
        QFace(QFace const &);
        
        void operator = (QFace const &);
        void set_Vindex(int const &V1,int const &V2,int const &V3,int const &V4);
};

#endif // MESH_BASE_H_INCLUDED
