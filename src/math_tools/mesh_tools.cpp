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

#include <mesh_tools.h>

extern const Vector3 unit_vec_x;
extern const Vector3 unit_vec_y;
extern const Vector3 unit_vec_z;

//######################################
//######################################

int obj_file_nl(std::string fname,char line_type)
{
    int R=0;
    
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    while(!file.eof())
    {
        std::string k;
        std::getline(file,k);
        
        if(!file.eof())
        {
            std::stringstream strm(k);
            
            char t;
            strm>>t;
            
            if(t==line_type) R++;
        }
    }
    
    file.close();
    
    return R;
}

int obj_file_nfc(std::string fname) { return obj_file_nl(fname,'f'); }
int obj_file_nvtx(std::string fname) { return obj_file_nl(fname,'v'); }

bool vID_compare(int const &Vb,int const &V1,int const &V2,int const &V3)
{
    if(Vb==V1 || Vb==V2 || Vb==V3) return true;
    
    return false;
}

int vID_compare_edge(int const &V1_ref,int const &V2_ref,int const &V3_ref,int const &V1,int const &V2,int const &V3)
{
    if(vID_compare(V1_ref,V1,V2,V3))
    {
        if(vID_compare(V2_ref,V1,V2,V3)) return 0;
        if(vID_compare(V3_ref,V1,V2,V3)) return 2;
    }
    else
    {
        if(vID_compare(V2_ref,V1,V2,V3))
        {
            if(vID_compare(V3_ref,V1,V2,V3)) return 1;
        }
    }
    
    return -1;
}

//void voxelize_full(Grid3<unsigned int> const &data,std::vector<Mesh<Vertex,Face>*> &mesh_arr)
//{
//    unsigned int i;
//    
//    unsigned int N=data.max();
//    
//    for(i=0;i<=N;i++)
//    {
//        Mesh<Vertex,Face> *mesh=new Mesh<Vertex,Face>;
//        
//        voxelize(mesh->V_arr,mesh->F_arr,data,i);
//        
//        if(mesh->F_arr.L1()>0) mesh_arr.push_back(mesh);
//    }
//}

void voxelize_flat(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid2<unsigned int> const &data,unsigned int target)
{
    int i,j;
    
    int Nx=data.L1();
    int Ny=data.L2();
    
    int NVtx=0;
    int Nf=0;
    
    double sx=1.0/(Nx+0.0);
    double sy=1.0/(Ny+0.0);
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        if(data(i,j)==target) Nf+=2;
    }}
    
    NVtx=3*Nf;
    
    V_arr.resize(NVtx);
    F_arr.resize(Nf);
    
    int V_offset=0;
    int F_offset=0;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        if(data(i,j)==target)
        {
            double x1=i*sx;
            double y1=j*sy;
            
            double x2=(i+1)*sx;
            double y2=(j+1)*sy;
            
            V_arr[V_offset+0].loc(x1,y1,0);
            V_arr[V_offset+1].loc(x2,y1,0);
            V_arr[V_offset+2].loc(x1,y2,0);
            
            V_arr[V_offset+0].norm=unit_vec_z;
            V_arr[V_offset+1].norm=unit_vec_z;
            V_arr[V_offset+2].norm=unit_vec_z;
            
            F_arr[F_offset].V1=V_offset+0;
            F_arr[F_offset].V2=V_offset+1;
            F_arr[F_offset].V3=V_offset+2;
            F_arr[F_offset].norm=unit_vec_z;
            
            V_arr[V_offset+3].loc(x2,y1,0);
            V_arr[V_offset+4].loc(x2,y2,0);
            V_arr[V_offset+5].loc(x1,y2,0);
            
            V_arr[V_offset+3].norm=unit_vec_z;
            V_arr[V_offset+4].norm=unit_vec_z;
            V_arr[V_offset+5].norm=unit_vec_z;
            
            F_arr[F_offset+1].V1=V_offset+3;
            F_arr[F_offset+1].V2=V_offset+4;
            F_arr[F_offset+1].V3=V_offset+5;
            F_arr[F_offset+1].norm=unit_vec_z;
            
            V_offset+=6;
            F_offset+=2;
        }
    }}
}

void voxelize(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid3<unsigned int> const &data,unsigned int target)
{
    int i,j,k;
    
    int Nx=data.L1();
    int Ny=data.L2();
    int Nz=data.L3();
    
    int NVtx=0;
    int Nf=0;
    
    double sx=1.0/(Nx+0.0);
    double sy=1.0/(Ny+0.0);
    double sz=1.0/(Nz+0.0);
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(data(i,j,k)==target)
        {
            if(i==0) Nf+=2;
            else if(data(i-1,j,k)!=target) Nf+=2;
            
            if(i==Nx-1) Nf+=2;
            else if(data(i+1,j,k)!=target) Nf+=2;
            
            if(j==0) Nf+=2;
            else if(data(i,j-1,k)!=target) Nf+=2;
            
            if(j==Ny-1) Nf+=2;
            else if(data(i,j+1,k)!=target) Nf+=2;
            
            if(k==0) Nf+=2;
            else if(data(i,j,k-1)!=target) Nf+=2;
            
            if(k==Nz-1) Nf+=2;
            else if(data(i,j,k+1)!=target) Nf+=2;
        }
    }}}
    
    NVtx=3*Nf;
    
    V_arr.resize(NVtx);
    F_arr.resize(Nf);
    
    int V_offset=0;
    int F_offset=0;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(data(i,j,k)==target)
        {
            bool xd=false,xu=false;
            bool yd=false,yu=false;
            bool zd=false,zu=false;
            
            if(i==0) xd=true;
            else if(data(i-1,j,k)!=target) xd=true;
            
            if(i==Nx-1) xu=true;
            else if(data(i+1,j,k)!=target) xu=true;
            
            if(j==0) yd=true;
            else if(data(i,j-1,k)!=target) yd=true;
            
            if(j==Ny-1) yu=true;
            else if(data(i,j+1,k)!=target) yu=true;
            
            if(k==0) zd=true;
            else if(data(i,j,k-1)!=target) zd=true;
            
            if(k==Nz-1) zu=true;
            else if(data(i,j,k+1)!=target) zu=true;
            
            double x1=i*sx;
            double y1=j*sy;
            double z1=k*sz;
            
            double x2=(i+1)*sx;
            double y2=(j+1)*sy;
            double z2=(k+1)*sz;
            
            if(xd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x1,y2,z1);
                V_arr[V_offset+2].loc(x1,y1,z2);
                
                V_arr[V_offset+0].norm=-unit_vec_x;
                V_arr[V_offset+1].norm=-unit_vec_x;
                V_arr[V_offset+2].norm=-unit_vec_x;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=-unit_vec_x;
                
                V_arr[V_offset+3].loc(x1,y2,z1);
                V_arr[V_offset+4].loc(x1,y2,z2);
                V_arr[V_offset+5].loc(x1,y1,z2);
                
                V_arr[V_offset+3].norm=-unit_vec_x;
                V_arr[V_offset+4].norm=-unit_vec_x;
                V_arr[V_offset+5].norm=-unit_vec_x;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=-unit_vec_x;
                
                V_offset+=6;
                F_offset+=2;
            }
            if(xu)
            {
                V_arr[V_offset+0].loc(x2,y1,z1);
                V_arr[V_offset+1].loc(x2,y2,z1);
                V_arr[V_offset+2].loc(x2,y1,z2);
                
                V_arr[V_offset+0].norm=unit_vec_x;
                V_arr[V_offset+1].norm=unit_vec_x;
                V_arr[V_offset+2].norm=unit_vec_x;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=unit_vec_x;
                
                V_arr[V_offset+3].loc(x2,y2,z1);
                V_arr[V_offset+4].loc(x2,y2,z2);
                V_arr[V_offset+5].loc(x2,y1,z2);
                
                V_arr[V_offset+3].norm=unit_vec_x;
                V_arr[V_offset+4].norm=unit_vec_x;
                V_arr[V_offset+5].norm=unit_vec_x;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=unit_vec_x;
                
                V_offset+=6;
                F_offset+=2;
            }
            if(yd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x2,y1,z1);
                V_arr[V_offset+2].loc(x1,y1,z2);
                
                V_arr[V_offset+0].norm=-unit_vec_y;
                V_arr[V_offset+1].norm=-unit_vec_y;
                V_arr[V_offset+2].norm=-unit_vec_y;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=-unit_vec_y;
                
                V_arr[V_offset+3].loc(x2,y1,z1);
                V_arr[V_offset+4].loc(x2,y1,z2);
                V_arr[V_offset+5].loc(x1,y1,z2);
                
                V_arr[V_offset+3].norm=-unit_vec_y;
                V_arr[V_offset+4].norm=-unit_vec_y;
                V_arr[V_offset+5].norm=-unit_vec_y;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=-unit_vec_y;
                
                V_offset+=6;
                F_offset+=2;
            }
            if(yu)
            {
                V_arr[V_offset+0].loc(x1,y2,z1);
                V_arr[V_offset+1].loc(x2,y2,z1);
                V_arr[V_offset+2].loc(x1,y2,z2);
                
                V_arr[V_offset+0].norm=unit_vec_y;
                V_arr[V_offset+1].norm=unit_vec_y;
                V_arr[V_offset+2].norm=unit_vec_y;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=unit_vec_y;
                
                V_arr[V_offset+3].loc(x2,y2,z1);
                V_arr[V_offset+4].loc(x2,y2,z2);
                V_arr[V_offset+5].loc(x1,y2,z2);
                
                V_arr[V_offset+3].norm=unit_vec_y;
                V_arr[V_offset+4].norm=unit_vec_y;
                V_arr[V_offset+5].norm=unit_vec_y;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=unit_vec_y;
                
                V_offset+=6;
                F_offset+=2;
            }
            if(zd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x2,y1,z1);
                V_arr[V_offset+2].loc(x1,y2,z1);
                
                V_arr[V_offset+0].norm=-unit_vec_z;
                V_arr[V_offset+1].norm=-unit_vec_z;
                V_arr[V_offset+2].norm=-unit_vec_z;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=-unit_vec_z;
                
                V_arr[V_offset+3].loc(x2,y1,z1);
                V_arr[V_offset+4].loc(x2,y2,z1);
                V_arr[V_offset+5].loc(x1,y2,z1);
                
                V_arr[V_offset+3].norm=-unit_vec_z;
                V_arr[V_offset+4].norm=-unit_vec_z;
                V_arr[V_offset+5].norm=-unit_vec_z;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=-unit_vec_z;
                
                V_offset+=6;
                F_offset+=2;
            }
            if(zu)
            {
                V_arr[V_offset+0].loc(x1,y1,z2);
                V_arr[V_offset+1].loc(x2,y1,z2);
                V_arr[V_offset+2].loc(x1,y2,z2);
                
                V_arr[V_offset+0].norm=unit_vec_z;
                V_arr[V_offset+1].norm=unit_vec_z;
                V_arr[V_offset+2].norm=unit_vec_z;
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].norm=unit_vec_z;
                
                V_arr[V_offset+3].loc(x2,y1,z2);
                V_arr[V_offset+4].loc(x2,y2,z2);
                V_arr[V_offset+5].loc(x1,y2,z2);
                
                V_arr[V_offset+3].norm=unit_vec_z;
                V_arr[V_offset+4].norm=unit_vec_z;
                V_arr[V_offset+5].norm=unit_vec_z;
                
                F_arr[F_offset+1].V1=V_offset+3;
                F_arr[F_offset+1].V2=V_offset+4;
                F_arr[F_offset+1].V3=V_offset+5;
                F_arr[F_offset+1].norm=unit_vec_z;
                
                V_offset+=6;
                F_offset+=2;
            }
        }
    }}}
}

void voxelize_quad(std::vector<Vertex> &V_arr,std::vector<QFace> &F_arr,Grid3<unsigned int> const &data,unsigned int target)
{
    int i,j,k;
    
    int Nx=data.L1();
    int Ny=data.L2();
    int Nz=data.L3();
    
    int NVtx=0;
    int Nf=0;
    
    double sx=1.0/(Nx+0.0);
    double sy=1.0/(Ny+0.0);
    double sz=1.0/(Nz+0.0);
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(data(i,j,k)==target)
        {
            if(i==0) Nf+=1;
            else if(data(i-1,j,k)!=target) Nf+=1;
            
            if(i==Nx-1) Nf+=1;
            else if(data(i+1,j,k)!=target) Nf+=1;
            
            if(j==0) Nf+=1;
            else if(data(i,j-1,k)!=target) Nf+=1;
            
            if(j==Ny-1) Nf+=1;
            else if(data(i,j+1,k)!=target) Nf+=1;
            
            if(k==0) Nf+=1;
            else if(data(i,j,k-1)!=target) Nf+=1;
            
            if(k==Nz-1) Nf+=1;
            else if(data(i,j,k+1)!=target) Nf+=1;
        }
    }}}
    
    NVtx=4*Nf;
    
    V_arr.resize(NVtx);
    F_arr.resize(Nf);
    
    int V_offset=0;
    int F_offset=0;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(data(i,j,k)==target)
        {
            bool xd=false,xu=false;
            bool yd=false,yu=false;
            bool zd=false,zu=false;
            
            if(i==0) xd=true;
            else if(data(i-1,j,k)!=target) xd=true;
            
            if(i==Nx-1) xu=true;
            else if(data(i+1,j,k)!=target) xu=true;
            
            if(j==0) yd=true;
            else if(data(i,j-1,k)!=target) yd=true;
            
            if(j==Ny-1) yu=true;
            else if(data(i,j+1,k)!=target) yu=true;
            
            if(k==0) zd=true;
            else if(data(i,j,k-1)!=target) zd=true;
            
            if(k==Nz-1) zu=true;
            else if(data(i,j,k+1)!=target) zu=true;
            
            double x1=i*sx;
            double y1=j*sy;
            double z1=k*sz;
            
            double x2=(i+1)*sx;
            double y2=(j+1)*sy;
            double z2=(k+1)*sz;
            
            if(xd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x1,y2,z1);
                V_arr[V_offset+2].loc(x1,y2,z2);
                V_arr[V_offset+3].loc(x1,y1,z2);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
            if(xu)
            {
                V_arr[V_offset+0].loc(x2,y1,z1);
                V_arr[V_offset+1].loc(x2,y2,z1);
                V_arr[V_offset+2].loc(x2,y2,z2);
                V_arr[V_offset+3].loc(x2,y1,z2);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
            if(yd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x2,y1,z1);
                V_arr[V_offset+2].loc(x2,y1,z2);
                V_arr[V_offset+3].loc(x1,y1,z2);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
            if(yu)
            {
                V_arr[V_offset+0].loc(x1,y2,z1);
                V_arr[V_offset+1].loc(x2,y2,z1);
                V_arr[V_offset+2].loc(x2,y2,z2);
                V_arr[V_offset+3].loc(x1,y2,z2);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
            if(zd)
            {
                V_arr[V_offset+0].loc(x1,y1,z1);
                V_arr[V_offset+1].loc(x2,y1,z1);
                V_arr[V_offset+2].loc(x2,y2,z1);
                V_arr[V_offset+3].loc(x1,y2,z1);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
            if(zu)
            {
                V_arr[V_offset+0].loc(x1,y1,z2);
                V_arr[V_offset+1].loc(x2,y1,z2);
                V_arr[V_offset+2].loc(x2,y2,z2);
                V_arr[V_offset+3].loc(x1,y2,z2);
                
                F_arr[F_offset].V1=V_offset+0;
                F_arr[F_offset].V2=V_offset+1;
                F_arr[F_offset].V3=V_offset+2;
                F_arr[F_offset].V4=V_offset+3;
                
                V_offset+=4;
                F_offset+=1;
            }
        }
    }}}
}

bool corner_check(unsigned int  t,
                  unsigned int d1,
                  unsigned int d2,
                  unsigned int d3,
                  unsigned int d4)
{
    int N=0;
    
    if(d1==t) N++;
    if(d2==t) N++;
    if(d3==t) N++;
    if(d4==t) N++;
    
    if(N==0 || N==4) return false;
    
    if(N==2)
    {
        if(d1==d3 || d2==d4) return true;
        else return false;
    }
    else return true;
}

void voxelize_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,Grid3<unsigned int> const &data,unsigned int target)
{
    int i,j,k;
    
    int Nx=data.L1();
    int Ny=data.L2();
    int Nz=data.L3();
    
    int Nf=0;
    
    double sx=1.0/(Nx+0.0);
    double sy=1.0/(Ny+0.0);
    double sz=1.0/(Nz+0.0);
    
    int N=0;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(data(i,j,k)==target) N++;
    }}}
    
    unsigned int imax=std::numeric_limits<unsigned int>::max();
    
    // Counting
    
    // - XY scan
    
    for(i=0;i<=Nx;i++){ for(j=0;j<=Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(corner_check(target,data.at(i  ,j  ,k,imax),data.at(i-1,j  ,k,imax),
                               data.at(i-1,j-1,k,imax),data.at(i  ,j-1,k,imax))) Nf++;
    }}}
    
    // - XZ scan
    
    for(i=0;i<=Nx;i++){ for(k=0;k<=Nz;k++){ for(j=0;j<Ny;j++)
    {
        if(corner_check(target,data.at(i  ,j,k  ,imax),data.at(i-1,j,k  ,imax),
                               data.at(i-1,j,k-1,imax),data.at(i  ,j,k-1,imax))) Nf++;
    }}}
    
    // - YZ scan
    
    for(j=0;j<=Ny;j++){ for(k=0;k<=Nz;k++){ for(i=0;i<Nx;i++)
    {
        if(corner_check(target,data.at(i,j  ,k  ,imax),data.at(i,j-1,k  ,imax),
                               data.at(i,j-1,k-1,imax),data.at(i,j  ,k-1,imax))) Nf++;
    }}}
    
    // Generating
    
    V_arr.resize(2*Nf);
    F_arr.resize(Nf);
    
    int V_offset=0;
    int F_offset=0;
    
    // XY scan
    
    for(i=0;i<=Nx;i++){ for(j=0;j<=Ny;j++){ for(k=0;k<Nz;k++)
    {
        if(corner_check(target,data.at(i  ,j  ,k,imax),data.at(i-1,j  ,k,imax),
                               data.at(i-1,j-1,k,imax),data.at(i  ,j-1,k,imax)))
        {
            V_arr[V_offset+0].loc.x=i*sx;
            V_arr[V_offset+0].loc.y=j*sy;
            V_arr[V_offset+0].loc.z=k*sz;
            
            V_arr[V_offset+1].loc.x=i*sx;
            V_arr[V_offset+1].loc.y=j*sy;
            V_arr[V_offset+1].loc.z=(k+1)*sz;
            
            F_arr[F_offset].V1=V_offset;
            F_arr[F_offset].V2=V_offset+1;
            
            V_offset+=2;
            F_offset++;
        }
    }}}
    
    // XZ scan
    
    for(i=0;i<=Nx;i++){ for(k=0;k<=Nz;k++){ for(j=0;j<Ny;j++)
    {
        if(corner_check(target,data.at(i  ,j,k  ,imax),data.at(i-1,j,k  ,imax),
                               data.at(i-1,j,k-1,imax),data.at(i  ,j,k-1,imax)))
        {
            V_arr[V_offset+0].loc.x=i*sx;
            V_arr[V_offset+0].loc.y=j*sy;
            V_arr[V_offset+0].loc.z=k*sz;
            
            V_arr[V_offset+1].loc.x=i*sx;
            V_arr[V_offset+1].loc.y=(j+1)*sy;
            V_arr[V_offset+1].loc.z=k*sz;
            
            F_arr[F_offset].V1=V_offset;
            F_arr[F_offset].V2=V_offset+1;
            
            V_offset+=2;
            F_offset++;
        }
    }}}
    
    // YZ scan
    
    for(j=0;j<=Ny;j++){ for(k=0;k<=Nz;k++){ for(i=0;i<Nx;i++)
    {
        if(corner_check(target,data.at(i,j  ,k  ,imax),data.at(i,j-1,k  ,imax),
                               data.at(i,j-1,k-1,imax),data.at(i,j  ,k-1,imax)))
        {
            V_arr[V_offset+0].loc.x=i*sx;
            V_arr[V_offset+0].loc.y=j*sy;
            V_arr[V_offset+0].loc.z=k*sz;
            
            V_arr[V_offset+1].loc.x=(i+1)*sx;
            V_arr[V_offset+1].loc.y=j*sy;
            V_arr[V_offset+1].loc.z=k*sz;
            
            F_arr[F_offset].V1=V_offset;
            F_arr[F_offset].V2=V_offset+1;
            
            V_offset+=2;
            F_offset++;
        }
    }}}
}
