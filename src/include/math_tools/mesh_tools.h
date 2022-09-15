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

#ifndef MESH_TOOLS_H
#define MESH_TOOLS_H

#include <mathUT.h>
#include <mesh_base.h>
#include <ray_intersect.h>
#include <string_tools.h>

#include <filesystem>
#include <fstream>
#include <list>

template<class V,class F>
void uv_to_xyz(Grid1<V> const &V_arr,F const &face,double u,double v,double &x,double &y,double &z)
{
    
}

template<class V,class F>
[[deprecated]]
void obj_file_load(std::string fname,Grid1<V> &V_arr,Grid1<F> &F_arr)
{
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    int V1,V2,V3;
    double x,y,z;
        
    std::string k;
    while(std::getline(file,k))
    {
        std::stringstream strm(k);
        char t; strm>>t;
        
        if(t=='f')
        {
            strm>>V1; strm>>V2; strm>>V3;
            V1-=1; V2-=1; V3-=1;
            
            F f(V1,V2,V3);
            
            F_arr.push_back(f);
        }
        else if(t=='v')
        {
            strm>>x; strm>>y; strm>>z;
            
            V v(x,y,z);
            
            V_arr.push_back(v);
        }
    }
}

//template<class V,class F>
//void obj_file_load(std::string fname,std::vector<V> &V_arr,std::vector<F> &F_arr)
//{
//    std::ifstream file(fname,std::ios::in|std::ios::binary);
//    
//    int V1,V2,V3;
//    double x,y,z;
//    
//    std::string k;
//    while(std::getline(file,k))
//    {
//        std::stringstream strm(k);
//        char t; strm>>t;
//        
//        if(t=='f')
//        {
//            strm>>V1; strm>>V2; strm>>V3;
//            V1-=1; V2-=1; V3-=1;
//            
//            F f(V1,V2,V3);
//            
//            F_arr.push_back(f);
//        }
//        else if(t=='v')
//        {
//            strm>>x; strm>>y; strm>>z;
//            
//            V v(x,y,z);
//            
//            V_arr.push_back(v);
//        }
//    }
//}

template<class V,class F>
void obj_file_load(std::filesystem::path const &fname,std::vector<V> &V_arr,std::vector<F> &F_arr)
{
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    double x,y,z;
    
    std::string k;
    int VID[4];
    std::vector<std::string> split;
    
    V_arr.clear();
    F_arr.clear();
    
    while(std::getline(file,k))
    {
        split_string(split,k);
        
        if(split[0]=="v")
        {
            x=std::stod(split[1]);
            y=std::stod(split[2]);
            z=std::stod(split[3]);
            
            V v(x,y,z);
            
            V_arr.push_back(v);
        }
        else if(split[0]=="f")
        {
            for(unsigned int i=0;i<split.size()-1;i++)
                VID[i]=std::stoi(split[i+1].substr(0,split[i+1].find("/")))-1;
            
            if(split.size()==4)
            {
                F f(VID[0],VID[1],VID[2]);
                
                F_arr.push_back(f);
            }
            else if(split.size()==5)
            {
                F f1(VID[0],VID[1],VID[2]),f2(VID[0],VID[2],VID[3]);
                
                F_arr.push_back(f1);
                F_arr.push_back(f2);
            }
        }
    }
    
    for(std::size_t i=0;i<F_arr.size();i++)
        F_arr[i].comp_norm(V_arr);
}

template<class V,class F>
void obj_file_save(std::string fname,std::vector<V> const &V_arr,std::vector<F> const &F_arr)
{
    std::ofstream file(fname,std::ios::out|std::ios::trunc|std::ios::binary);
    
    for(unsigned int i=0;i<V_arr.size();i++)
        file<<"v "<<V_arr[i].loc.x<<" "<<V_arr[i].loc.y<<" "<<V_arr[i].loc.z<<std::endl;
    
    for(unsigned int i=0;i<F_arr.size();i++)
        file<<"f "<<F_arr[i].V1+1<<" "<<F_arr[i].V2+1<<" "<<F_arr[i].V3+1<<std::endl;
    
    file.close();
}

template<class V,class F>
void obj_file_save_quad(std::string fname,std::vector<V> const &V_arr,std::vector<F> const &F_arr)
{
    std::ofstream file(fname,std::ios::out|std::ios::trunc|std::ios::binary);
    
    for(unsigned int i=0;i<V_arr.size();i++)
        file<<"v "<<V_arr[i].loc.x<<" "<<V_arr[i].loc.y<<" "<<V_arr[i].loc.z<<std::endl;
    
    for(unsigned int i=0;i<F_arr.size();i++)
        file<<"f "<<F_arr[i].V1+1<<" "<<F_arr[i].V2+1<<" "<<F_arr[i].V3+1<<" "<<F_arr[i].V4+1<<std::endl;
    
    file.close();
}

int obj_file_nfc(std::string fname);
int obj_file_nvtx(std::string fname);

template<class V,class F>
void obj_make_cube(std::vector<V> &V_arr,std::vector<F> &F_arr,double size=1.0)
{
    V_arr.resize(8);
    F_arr.resize(12);
    
    double s2=size/2.0;
    
    V_arr[0].loc(-s2,-s2,-s2);
    V_arr[1].loc(+s2,-s2,-s2);
    V_arr[2].loc(+s2,+s2,-s2);
    V_arr[3].loc(-s2,+s2,-s2);
    V_arr[4].loc(-s2,-s2,+s2);
    V_arr[5].loc(+s2,-s2,+s2);
    V_arr[6].loc(+s2,+s2,+s2);
    V_arr[7].loc(-s2,+s2,+s2);
    
    F_arr[0].set_Vindex(0,2,1);
    F_arr[1].set_Vindex(0,3,2);
    F_arr[2].set_Vindex(4,5,6);
    F_arr[3].set_Vindex(4,6,7);
    F_arr[4].set_Vindex(0,5,4);
    F_arr[5].set_Vindex(0,1,5);
    F_arr[6].set_Vindex(1,6,5);
    F_arr[7].set_Vindex(1,2,6);
    F_arr[8].set_Vindex(2,7,6);
    F_arr[9].set_Vindex(2,3,7);
    F_arr[10].set_Vindex(3,4,7);
    F_arr[11].set_Vindex(3,0,4);
}

template<class V,class F>
void obj_make_partial_cube(std::vector<V> &V_arr,std::vector<F> &F_arr,std::vector<int> const &face_ignore,double size=1.0)
{
    V_arr.resize(8);
    F_arr.clear();
    
    double s2=size/2.0;
    
    V_arr[0].loc(-s2,-s2,-s2);
    V_arr[1].loc(+s2,-s2,-s2);
    V_arr[2].loc(+s2,+s2,-s2);
    V_arr[3].loc(-s2,+s2,-s2);
    V_arr[4].loc(-s2,-s2,+s2);
    V_arr[5].loc(+s2,-s2,+s2);
    V_arr[6].loc(+s2,+s2,+s2);
    V_arr[7].loc(-s2,+s2,+s2);
    
    F face;
    
    if(!find_in_vector(face_ignore,0))
    {
        face.set_Vindex(0,2,1); F_arr.push_back(face);
        face.set_Vindex(0,3,2); F_arr.push_back(face);
    }
    if(!find_in_vector(face_ignore,1))
    {
        face.set_Vindex(4,5,6); F_arr.push_back(face);
        face.set_Vindex(4,6,7); F_arr.push_back(face);
    }
    if(!find_in_vector(face_ignore,2))
    {
        face.set_Vindex(0,5,4); F_arr.push_back(face);
        face.set_Vindex(0,1,5); F_arr.push_back(face);
    }
    if(!find_in_vector(face_ignore,3))
    {
        face.set_Vindex(1,6,5); F_arr.push_back(face);
        face.set_Vindex(1,2,6); F_arr.push_back(face);
    }
    if(!find_in_vector(face_ignore,4))
    {
        face.set_Vindex(2,7,6); F_arr.push_back(face);
        face.set_Vindex(2,3,7); F_arr.push_back(face);
    }
    if(!find_in_vector(face_ignore,5))
    {
        face.set_Vindex(3,4,7); F_arr.push_back(face);
        face.set_Vindex(3,0,4); F_arr.push_back(face);
    }
}

template<class V,class F>
void obj_make_plane(std::vector<V> &V_arr,std::vector<F> &F_arr,double size=1.0)
{
    V_arr.resize(4);
    F_arr.resize(2);
    
    double s2=size/2.0;
    
    V_arr[0].loc(-s2,-s2,0);
    V_arr[1].loc(+s2,-s2,0);
    V_arr[2].loc(+s2,+s2,0);
    V_arr[3].loc(-s2,+s2,0);
    
    F_arr[0].set_Vindex(0,2,1);
    F_arr[1].set_Vindex(0,3,2);
}

template<class V,class F>
void obj_hexa_sq_paving(std::vector<V> &V_arr,std::vector<F> &F_arr,int Nx,int Ny,double lx=1.0,double ly=std::sqrt(3.0))
{
    int i,j;
    
    int Nf=4*Nx*Ny;
    int Nv=Ny*(2*Nx+1)+Nx+1;
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    int V_off,F_off;
    int F_base_off=4*Nx;
    int V_base_off=2*Nx+1;
    
    //Vertex Locations
    
    double Dx=lx/(Nx+0.0);
    double Dy=ly/(Ny+0.0);
    
    for(j=0;j<Ny;j++)
    {
        V_off=j*V_base_off;
        
        for(i=0;i<Nx+1;i++)
            V_arr[i+V_off].loc(i*Dx,j*Dy,0);
        
        V_off=j*V_base_off+Nx+1;
        
        for(i=0;i<Nx;i++)
            V_arr[i+V_off].loc((i+0.5)*Dx,(j+0.5)*Dy,0);
    }
    
    V_off=Ny*V_base_off;
        
    for(i=0;i<Nx+1;i++)
        V_arr[i+V_off].loc(i*Dx,j*Dy,0);
    
    // Faces assignment
    
    for(j=0;j<Ny;j++)
    {
        V_off=j*V_base_off;
        
        F_off=j*F_base_off;
        
        for(i=0;i<Nx;i++)
        {
            F_arr[i+F_off].V1=V_off+i;
            F_arr[i+F_off].V2=V_off+i+1;
            F_arr[i+F_off].V3=V_off+i+Nx+1;
        }
        
        F_off+=Nx;
        
        for(i=0;i<Nx-1;i++)
        {
            F_arr[i+F_off].V1=V_off+i+1;
            F_arr[i+F_off].V2=V_off+i+Nx+1;
            F_arr[i+F_off].V3=V_off+i+Nx+2;
        }
        
        F_off+=Nx-1;
        
        for(i=0;i<Nx-1;i++)
        {
            F_arr[i+F_off].V1=V_off+i+2*Nx+2;
            F_arr[i+F_off].V2=V_off+i+Nx+1;
            F_arr[i+F_off].V3=V_off+i+Nx+2;
        }
        
        F_off+=Nx-1;
        
        for(i=0;i<Nx;i++)
        {
            F_arr[i+F_off].V1=V_off+i+2*Nx+1;
            F_arr[i+F_off].V2=V_off+i+2*Nx+2;
            F_arr[i+F_off].V3=V_off+i+Nx+1;
        }
        
        F_off+=Nx;
        
        F_arr[F_off].V1=V_off;
        F_arr[F_off].V2=V_off+Nx+1;
        F_arr[F_off].V3=V_off+2*Nx+1;
        
        F_off+=1;
        
        F_arr[F_off].V1=V_off+Nx;
        F_arr[F_off].V2=V_off+2*Nx;
        F_arr[F_off].V3=V_off+3*Nx+1;
    }
    
    for(i=0;i<Nv;i++)
    {
        V_arr[i].loc.x-=lx/2.0;
        V_arr[i].loc.y-=ly/2.0;
    }
}

template<class V,class F>
bool point_inside_mesh(std::vector<V> &V_arr,std::vector<F> &F_arr,Vector3 const &P)
{
    Vector3 D;
    D.rand_sph();
                    
    int N_inter=ray_N_inter(V_arr,F_arr,-1,P,D);
    if(N_inter%2!=0) return true;
    
    return false;
}

template<class V,class F>
void raise_cube_corners(std::vector<V> &V_arr,std::vector<F> &F_arr)
{
    int i;
    
    int Nf=F_arr.size();
    int Nv=V_arr.size();
    
    V_arr.resize(Nv+Nf);
    
    std::vector<F> F_tmp(3*Nf);
    
    for(i=0;i<Nf;i++)
    {
        int V_off=i+Nv;
        int &V1=F_arr[i].V1;
        int &V2=F_arr[i].V2;
        int &V3=F_arr[i].V3;
        
        double l1=(V_arr[V1].loc-V_arr[V2].loc).norm();
        double l2=(V_arr[V1].loc-V_arr[V3].loc).norm();
        double l3=(V_arr[V2].loc-V_arr[V3].loc).norm();
        
        double h=var_min(l1,l2,l3)/std::sqrt(6.0);
        
        V_arr[V_off].loc=(V_arr[V1].loc+V_arr[V2].loc+V_arr[V3].loc)/3.0;
        V_arr[V_off].loc.z=h;
        
        F_tmp[3*i+0]=F_arr[i];
        F_tmp[3*i+1]=F_arr[i];
        F_tmp[3*i+2]=F_arr[i];
        
        F_tmp[3*i+0].V1=V_off;
        F_tmp[3*i+1].V2=V_off;
        F_tmp[3*i+2].V3=V_off;
    }
    
    F_arr.resize(3*Nf);
    F_arr=F_tmp;
}

template<class V>
void rescale_mesh(std::vector<V> &V_arr,double scale)
{
    for(unsigned int i=0;i<V_arr.size();i++) V_arr[i].loc*=scale;
}

bool vID_compare(int const &Vb,int const &V1,int const &V2,int const &V3);
int vID_compare_edge(int const &V1_ref,int const &V2_ref,int const &V3_ref,int const &V1,int const &V2,int const &V3);
//void voxelize_full(Grid3<unsigned int> const &data,std::vector<Mesh<Vertex,Face>*> &mesh_arr);
void voxelize(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid3<unsigned int> const &data,unsigned int target);
void voxelize_flat(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid2<unsigned int> const &data,unsigned int target);
void voxelize_quad(std::vector<Vertex> &V_arr,std::vector<QFace> &F_arr,Grid3<unsigned int> const &data,unsigned int target);
void voxelize_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid3<unsigned int> const &data,unsigned int target);

#endif
