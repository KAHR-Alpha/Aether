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

#include <gui_selene_gl.h>
#include <selene.h>

namespace SelGUI
{

void conic_section_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                                  double R,double K,double in_radius,double out_radius)
{
    V_arr.clear();
    F_arr.clear();
    
    double x_in=0;
    if(in_radius>0) x_in=conic_invert(in_radius,R,K);
        
    double x_out=R/(1.0+K);
    
    if(K>-1.0)
    {
        double r_half=conic(x_out,R,K);
        
        if(out_radius<r_half) x_out=conic_invert(out_radius,R,K);
        else out_radius=r_half;
    }
    else x_out=conic_invert(out_radius,R,K);
    
    wireframe_mesh_add_cross(V_arr,F_arr,R,0,0,R/4.0);
    
    double x,y;
    
    conic_near_focus(x,y,R,K);
    wireframe_mesh_add_cross(V_arr,F_arr,x,y,0,R/6.0);
    
    conic_far_focus(x,y,R,K);
    wireframe_mesh_add_cross(V_arr,F_arr,x,y,0,R/6.0);
    
    wireframe_mesh_add_circle(V_arr,F_arr,disc,x_in,in_radius);
    wireframe_mesh_add_circle(V_arr,F_arr,disc,x_out,out_radius);
    
    std::size_t v_offset=V_arr.size();
    std::size_t f_offset=F_arr.size();
    
    V_arr.resize(v_offset+4*(disc+1));
    F_arr.resize(f_offset+4*disc);
    
    for(unsigned int i=0;i<=disc;i++)
    {
        double r=in_radius+(out_radius-in_radius)*i/(disc+0.0);
        
        double h=conic_invert(r,R,K);
        if(i==disc) h=x_out;
        
        V_arr[i+v_offset+0*(disc+1)].loc(h,+r,0);
        V_arr[i+v_offset+1*(disc+1)].loc(h,-r,0);
        V_arr[i+v_offset+2*(disc+1)].loc(h,0,+r);
        V_arr[i+v_offset+3*(disc+1)].loc(h,0,-r);
    }
    
    for(unsigned int i=0;i<disc;i++) for(unsigned int k=0;k<4;k++)
    {
        F_arr[i+f_offset+k*disc].V1=i+v_offset+k*(disc+1);
        F_arr[i+f_offset+k*disc].V2=i+v_offset+k*(disc+1)+1;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void cylinder_cut_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                                 double L,double r,double cut_factor)
{
    int i;
    
    int disc=128;
    
    double x1=-L/2.0;
    double x2=+L/2.0;
    
    cut_factor=std::min(1.0,cut_factor);
    cut_factor=std::max(0.0,cut_factor);
    
    double z=r*(1.0-2.0*cut_factor);
    double th=std::acos(z/r);
    double y=r*std::sin(th);
    
    int Nv=2*(disc+1)+8;
    int Nf=2*disc+8;
    
    if(cut_factor>=0.5) { Nv+=4; Nf+=4; }
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    for(i=0;i<=disc;i++)
    {
        double ang=-th+i/(disc+0.0)*2.0*th;
        
        V_arr[i].loc(x1,r*std::sin(ang),r*std::cos(ang));
        V_arr[i+disc+1].loc(x2,r*std::sin(ang),r*std::cos(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[i].V1=i; F_arr[i].V2=i+1;
        F_arr[i+disc].V1=i+disc+1; F_arr[i+disc].V2=i+disc+2;
    }
    
    int v_offset=2*(disc+1);
    int f_offset=2*disc;
    
    V_arr[v_offset+0].loc(x1,0,z);
    V_arr[v_offset+1].loc(x1,0,r);
    V_arr[v_offset+2].loc(x2,0,r);
    V_arr[v_offset+3].loc(x2,0,z);
    
    F_arr[f_offset+0].V1=v_offset+0; F_arr[f_offset+0].V2=v_offset+1;
    F_arr[f_offset+1].V1=v_offset+1; F_arr[f_offset+1].V2=v_offset+2;
    F_arr[f_offset+2].V1=v_offset+2; F_arr[f_offset+2].V2=v_offset+3;
    F_arr[f_offset+3].V1=v_offset+3; F_arr[f_offset+3].V2=v_offset+0;
    
    v_offset+=4;
    f_offset+=4;
    
    V_arr[v_offset+0].loc(x1,+y,z);
    V_arr[v_offset+1].loc(x1,-y,z);
    V_arr[v_offset+2].loc(x2,-y,z);
    V_arr[v_offset+3].loc(x2,+y,z);
    
    F_arr[f_offset+0].V1=v_offset+0; F_arr[f_offset+0].V2=v_offset+1;
    F_arr[f_offset+1].V1=v_offset+1; F_arr[f_offset+1].V2=v_offset+2;
    F_arr[f_offset+2].V1=v_offset+2; F_arr[f_offset+2].V2=v_offset+3;
    F_arr[f_offset+3].V1=v_offset+3; F_arr[f_offset+3].V2=v_offset+0;
    
    v_offset+=4;
    f_offset+=4;
    
    if(cut_factor>=0.5)
    {
        V_arr[v_offset+0].loc(x1,+r,0);
        V_arr[v_offset+1].loc(x1,-r,0);
        V_arr[v_offset+2].loc(x2,-r,0);
        V_arr[v_offset+3].loc(x2,+r,0);
        
        F_arr[f_offset+0].V1=v_offset+0; F_arr[f_offset+0].V2=v_offset+1;
        F_arr[f_offset+1].V1=v_offset+1; F_arr[f_offset+1].V2=v_offset+2;
        F_arr[f_offset+2].V1=v_offset+2; F_arr[f_offset+2].V2=v_offset+3;
        F_arr[f_offset+3].V1=v_offset+3; F_arr[f_offset+3].V2=v_offset+0;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void disk_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                         double r,double r_in)
{
    int i;
    
    int disc=128;
    
    
    int Nv=2*(disc+1)+8;
    int Nf=2*disc+4;
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    for(i=0;i<=disc;i++)
    {
        double ang=2.0*Pi*i/(disc+0.0);
        
        V_arr[i].loc(0,r*std::sin(ang),r*std::cos(ang));
        V_arr[i+disc+1].loc(0,r_in*std::sin(ang),r_in*std::cos(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[i].V1=i; F_arr[i].V2=i+1;
        F_arr[i+disc].V1=i+disc+1; F_arr[i+disc].V2=i+disc+2;
    }
    
    int v_offset=2*(disc+1);
    int f_offset=2*disc;
    
    V_arr[v_offset+0].loc(0,0,r);
    V_arr[v_offset+1].loc(0,0,r_in);
    V_arr[v_offset+2].loc(0,0,-r);
    V_arr[v_offset+3].loc(0,0,-r_in);
    
    V_arr[v_offset+4].loc(0,r,0);
    V_arr[v_offset+5].loc(0,r_in,0);
    V_arr[v_offset+6].loc(0,-r,0);
    V_arr[v_offset+7].loc(0,-r_in,0);
    
    F_arr[f_offset+0].V1=v_offset+0; F_arr[f_offset+0].V2=v_offset+1;
    F_arr[f_offset+1].V1=v_offset+2; F_arr[f_offset+1].V2=v_offset+3;
    F_arr[f_offset+2].V1=v_offset+4; F_arr[f_offset+2].V2=v_offset+5;
    F_arr[f_offset+3].V1=v_offset+6; F_arr[f_offset+3].V2=v_offset+7;
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}
                         
void lens_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                         unsigned int disc,double t,double r_max,double r1,double r2)
{
    unsigned int i;
    
    double A=0,B=0,r_max_=r_max,th_1=0,th_2=0;
    
    Sel::lens_geometry(A,B,r_max,th_1,th_2,t,r_max_,r1,r2);
    
    double x1=A-r1*std::cos(th_1);
    double x2=B-r2*std::cos(th_2);
    
    if(disc%2!=0) disc++;
    
    unsigned int Nv=2*disc+4*(disc+1)+8;
    unsigned int Nf=2*disc+4*disc+4;
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    int v_offset=0;
    int f_offset=0;
    
    // Edges
    
    for(i=0;i<disc;i++)
    {
        double ang=i*(2.0*Pi/disc);
        
        V_arr[i].loc=Vector3(x1,r_max*std::cos(ang),r_max*std::sin(ang));
        V_arr[i+disc].loc=Vector3(x2,r_max*std::cos(ang),r_max*std::sin(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[i].V1=i;
        F_arr[i].V2=i+1;
        
        F_arr[i+disc].V1=i+disc;
        F_arr[i+disc].V2=i+1+disc;
    }
    
    F_arr[disc-1].V2=0;
    F_arr[2*disc-1].V2=disc;
    
    // Crosses
    
    v_offset+=2*disc;
    f_offset+=2*disc;
    
    for(i=0;i<=disc;i++)
    {
        double ang=2.0*th_1*i/(disc+0.0)-th_1;
        
        V_arr[v_offset+i].loc=Vector3(A-r1*std::cos(ang),r1*std::sin(ang),0);
        V_arr[v_offset+i+disc+1].loc=Vector3(A-r1*std::cos(ang),0,r1*std::sin(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[f_offset+i].V1=v_offset+i;
        F_arr[f_offset+i].V2=v_offset+i+1;
        
        F_arr[f_offset+i+disc].V1=v_offset+i+disc+1;
        F_arr[f_offset+i+disc].V2=v_offset+i+disc+2;
    }
    
    v_offset+=2*(disc+1);
    f_offset+=2*disc;
    
    for(i=0;i<=disc;i++)
    {
        double ang=2.0*th_2*i/(disc+0.0)-th_2;
        
        V_arr[v_offset+i].loc=Vector3(B-r2*std::cos(ang),-r2*std::sin(ang),0);
        V_arr[v_offset+i+disc+1].loc=Vector3(B-r2*std::cos(ang),0,-r2*std::sin(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[f_offset+i].V1=v_offset+i;
        F_arr[f_offset+i].V2=v_offset+i+1;
        
        F_arr[f_offset+i+disc].V1=v_offset+i+disc+1;
        F_arr[f_offset+i+disc].V2=v_offset+i+disc+2;
    }
    
    // Cross-edges
    
    v_offset+=2*(disc+1);
    f_offset+=2*disc;
    
    V_arr[v_offset+0].loc=Vector3(x1,r_max,0);
    V_arr[v_offset+1].loc=Vector3(x2,r_max,0);
    V_arr[v_offset+2].loc=Vector3(x1,-r_max,0);
    V_arr[v_offset+3].loc=Vector3(x2,-r_max,0);
    V_arr[v_offset+4].loc=Vector3(x1,0,r_max);
    V_arr[v_offset+5].loc=Vector3(x2,0,r_max);
    V_arr[v_offset+6].loc=Vector3(x1,0,-r_max);
    V_arr[v_offset+7].loc=Vector3(x2,0,-r_max);
    
    F_arr[f_offset+0].V1=v_offset+0;
    F_arr[f_offset+0].V2=v_offset+1;
    F_arr[f_offset+1].V1=v_offset+2;
    F_arr[f_offset+1].V2=v_offset+3;
    F_arr[f_offset+2].V1=v_offset+4;
    F_arr[f_offset+2].V2=v_offset+5;
    F_arr[f_offset+3].V1=v_offset+6;
    F_arr[f_offset+3].V2=v_offset+7;
    
    for(i=0;i<V_arr.size();i++)
    {
        V_arr[i].norm=V_arr[i].loc;
        V_arr[i].norm.normalize();
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void parabolic_mirror_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                                     double focal,double thickness,double in_radius,double height)
{
    V_arr.resize(6+16+4*disc+8*(disc+1));
    F_arr.resize(3+8+4*disc+8*disc);
    
    double a=1.0/(4.0*focal);
    
    double x2=a*in_radius*in_radius-focal;
    double x1=x2-thickness;
    
    double r1=std::sqrt(4.0*focal*height);
    
    V_arr[0].loc(-focal/4.0,0,0);
    V_arr[1].loc(+focal/4.0,0,0);
    V_arr[2].loc(0,-focal/4.0,0);
    V_arr[3].loc(0,+focal/4.0,0);
    V_arr[4].loc(0,0,-focal/4.0);
    V_arr[5].loc(0,0,+focal/4.0);
    
    F_arr[0].V1=0; F_arr[0].V2=1;
    F_arr[1].V1=2; F_arr[1].V2=3;
    F_arr[2].V1=4; F_arr[2].V2=5;
    
    V_arr[ 6].loc(x1,+in_radius,0);
    V_arr[ 7].loc(x2,+in_radius,0);
    V_arr[ 8].loc(x1,-in_radius,0);
    V_arr[ 9].loc(x2,-in_radius,0);
    V_arr[10].loc(x1,0,+in_radius);
    V_arr[11].loc(x2,0,+in_radius);
    V_arr[12].loc(x1,0,-in_radius);
    V_arr[13].loc(x2,0,-in_radius);
    
    F_arr[3].V1= 6; F_arr[3].V2= 7;
    F_arr[4].V1= 8; F_arr[4].V2= 9;
    F_arr[5].V1=10; F_arr[5].V2=11;
    F_arr[6].V1=12; F_arr[6].V2=13;
    
    V_arr[14].loc(height-focal,+r1,0);
    V_arr[15].loc(height-focal-thickness,+r1,0);
    V_arr[16].loc(height-focal,-r1,0);
    V_arr[17].loc(height-focal-thickness,-r1,0);
    V_arr[18].loc(height-focal,0,+r1);
    V_arr[19].loc(height-focal-thickness,0,+r1);
    V_arr[20].loc(height-focal,0,-r1);
    V_arr[21].loc(height-focal-thickness,0,-r1);
    
    F_arr[ 7].V1=14; F_arr[ 7].V2=15;
    F_arr[ 8].V1=16; F_arr[ 8].V2=17;
    F_arr[ 9].V1=18; F_arr[ 9].V2=19;
    F_arr[10].V1=20; F_arr[10].V2=21;
    
    int v_offset=22;
    int f_offset=11;
    
    for(unsigned int i=0;i<disc;i++)
    {
        double ang=2.0*Pi*i/(disc-1.0);
        double c=std::cos(ang),s=std::sin(ang);
        
        V_arr[i+v_offset].loc(x1,in_radius*c,in_radius*s);
        V_arr[i+v_offset+disc].loc(x2,in_radius*c,in_radius*s);
        V_arr[i+v_offset+2*disc].loc(height-focal,r1*c,r1*s);
        V_arr[i+v_offset+3*disc].loc(height-focal-thickness,r1*c,r1*s);
        
        F_arr[i+f_offset].V1=i+v_offset;
        F_arr[i+f_offset].V2=i+v_offset+1;
        
        F_arr[i+f_offset+disc].V1=i+v_offset+disc;
        F_arr[i+f_offset+disc].V2=i+v_offset+1+disc;
        
        F_arr[i+f_offset+2*disc].V1=i+v_offset+2*disc;
        F_arr[i+f_offset+2*disc].V2=i+v_offset+1+2*disc;
        
        F_arr[i+f_offset+3*disc].V1=i+v_offset+3*disc;
        F_arr[i+f_offset+3*disc].V2=i+v_offset+1+3*disc;
    }
    
    F_arr[f_offset+disc-1].V2=v_offset;
    F_arr[f_offset+2*disc-1].V2=v_offset+disc;
    F_arr[f_offset+3*disc-1].V2=v_offset+2*disc;
    F_arr[f_offset+4*disc-1].V2=v_offset+3*disc;
    
    v_offset+=4*disc;
    f_offset+=4*disc;
    
    for(unsigned int i=0;i<=disc;i++)
    {
        double r=in_radius+(r1-in_radius)*i/(disc+0.0);
        double h=a*r*r-focal;
        
        V_arr[i+v_offset+0*(disc+1)].loc(h,+r,0);
        V_arr[i+v_offset+1*(disc+1)].loc(h,-r,0);
        V_arr[i+v_offset+2*(disc+1)].loc(h,0,+r);
        V_arr[i+v_offset+3*(disc+1)].loc(h,0,-r);
        
        V_arr[i+v_offset+4*(disc+1)].loc(h-thickness,+r,0);
        V_arr[i+v_offset+5*(disc+1)].loc(h-thickness,-r,0);
        V_arr[i+v_offset+6*(disc+1)].loc(h-thickness,0,+r);
        V_arr[i+v_offset+7*(disc+1)].loc(h-thickness,0,-r);
    }
    
    for(unsigned int i=0;i<disc;i++) for(unsigned int k=0;k<8;k++)
    {
        F_arr[i+f_offset+k*disc].V1=i+v_offset+k*(disc+1);
        F_arr[i+f_offset+k*disc].V2=i+v_offset+k*(disc+1)+1;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void parabola_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                             double focal,double in_radius,double height)
{
    V_arr.resize(6+2*disc+4*(disc+1));
    F_arr.resize(3+2*disc+4*disc);
    
    double a=1.0/(4.0*focal);
    
    double x_in=a*in_radius*in_radius;
    
    double r_out=std::sqrt(4.0*focal*height);
    
    V_arr[0].loc(focal-focal/4.0,0,0);
    V_arr[1].loc(focal+focal/4.0,0,0);
    V_arr[2].loc(focal,-focal/4.0,0);
    V_arr[3].loc(focal,+focal/4.0,0);
    V_arr[4].loc(focal,0,-focal/4.0);
    V_arr[5].loc(focal,0,+focal/4.0);
    
    F_arr[0].V1=0; F_arr[0].V2=1;
    F_arr[1].V1=2; F_arr[1].V2=3;
    F_arr[2].V1=4; F_arr[2].V2=5;
    
    int v_offset=6;
    int f_offset=3;
    
    for(unsigned int i=0;i<disc;i++)
    {
        double ang=2.0*Pi*i/(disc-1.0);
        double c=std::cos(ang),s=std::sin(ang);
        
        V_arr[i+v_offset].loc(x_in,in_radius*c,in_radius*s);
        V_arr[i+v_offset+disc].loc(height,r_out*c,r_out*s);
        
        F_arr[i+f_offset].V1=i+v_offset;
        F_arr[i+f_offset].V2=i+v_offset+1;
        
        F_arr[i+f_offset+disc].V1=i+v_offset+disc;
        F_arr[i+f_offset+disc].V2=i+v_offset+1+disc;
    }
    
    F_arr[f_offset+disc-1].V2=v_offset;
    F_arr[f_offset+2*disc-1].V2=v_offset+disc;
    
    v_offset+=2*disc;
    f_offset+=2*disc;
    
    for(unsigned int i=0;i<=disc;i++)
    {
        double r=in_radius+(r_out-in_radius)*i/(disc+0.0);
        double h=a*r*r;
        
        V_arr[i+v_offset+0*(disc+1)].loc(h,+r,0);
        V_arr[i+v_offset+1*(disc+1)].loc(h,-r,0);
        V_arr[i+v_offset+2*(disc+1)].loc(h,0,+r);
        V_arr[i+v_offset+3*(disc+1)].loc(h,0,-r);
    }
    
    for(unsigned int i=0;i<disc;i++) for(unsigned int k=0;k<4;k++)
    {
        F_arr[i+f_offset+k*disc].V1=i+v_offset+k*(disc+1);
        F_arr[i+f_offset+k*disc].V2=i+v_offset+k*(disc+1)+1;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void prism_mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                double length,double height,AngleRad const &a1,AngleRad const &a2,double width)
{
    double x1=-length/2.0;
    double x2=+length/2.0;
    double z1=-height/2.0;
    double z2=+height/2.0;
    
    Vector3 P1(x1,0,0);
    Vector3 P2(x2,0,0);
    
    Vector3 v1(std::cos(a1),std::sin(a1),0);
    Vector3 v2(-std::cos(a2),std::sin(a2),0);
    
    Vector3 P21=P1-P2,v2x1,C;
    v2x1.crossprod(v2,v1);
    C.crossprod(P21,v2);
    
    double t=0,n=v2x1.norm_sqr();
    
    if(n!=0) t=scalar_prod(C,v2x1)/n;
    
    C=P1+t*v1;
    
    double xc=C.x;
    double yc=C.y;
    
    if(std::abs(yc)<std::abs(width))
    {
        V_arr.resize(18);
        F_arr.resize(8);
        
        V_arr[0].loc(x1,0,z1);  // Bottom
        V_arr[1].loc(x2,0,z1);
        V_arr[2].loc(xc,yc,z1);
        
        F_arr[0].set_Vindex(0,1,2);
        
        V_arr[3].loc(x1,0,z2);  // Top
        V_arr[4].loc(x2,0,z2);
        V_arr[5].loc(xc,yc,z2);
        
        F_arr[1].set_Vindex(3,4,5);
        
        V_arr[6].loc(x1,0,z1);
        V_arr[7].loc(x2,0,z1);
        V_arr[8].loc(x2,0,z2);
        V_arr[9].loc(x1,0,z2);
        
        F_arr[2].set_Vindex(6,7,8);
        F_arr[3].set_Vindex(6,8,9);
        
        V_arr[10].loc(x2,0,z1);
        V_arr[11].loc(xc,yc,z1);
        V_arr[12].loc(xc,yc,z2);
        V_arr[13].loc(x2,0,z2);
        
        F_arr[4].set_Vindex(10,11,12);
        F_arr[5].set_Vindex(10,12,13);
        
        V_arr[14].loc(xc,yc,z1);
        V_arr[15].loc(x1,0,z1);
        V_arr[16].loc(x1,0,z2);
        V_arr[17].loc(xc,yc,z2);
        
        F_arr[6].set_Vindex(14,15,16);
        F_arr[7].set_Vindex(14,16,17);
    }
    else
    {
        yc=std::abs(width);
        
        Vector3 H(0,yc,0);
        
        v2x1.crossprod(unit_vec_x,v1);
        Vector3 D=P1-H; C.crossprod(D,unit_vec_x);
        t=scalar_prod(C,v2x1)/v2x1.norm_sqr();
        
        C=P1+t*v1;
        double x1=C.x;
        
        v2x1.crossprod(unit_vec_x,v2);
        D=P2-H; C.crossprod(D,unit_vec_x);
        t=scalar_prod(C,v2x1)/v2x1.norm_sqr();
        
        C=P2+t*v2;
        double x2=C.x;
        
        Glite::make_block(V_arr,F_arr,length,width,height,-0.5,0,-0.5);
        
        V_arr[5].loc.x=x2;
        V_arr[7].loc.x=x2;
        
        V_arr[ 8].loc.x=x1;
        V_arr[ 9].loc.x=x2;
        V_arr[10].loc.x=x2;
        V_arr[11].loc.x=x1;
        
        V_arr[13].loc.x=x1;
        V_arr[15].loc.x=x1;
        
        V_arr[18].loc.x=x2;
        V_arr[19].loc.x=x1;
        
        V_arr[22].loc.x=x2;
        V_arr[23].loc.x=x1;
    }
}

void prism_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                          double length,double height,AngleRad const &a1,AngleRad const &a2,double width)
{
    double x1=-length/2.0;
    double x2=+length/2.0;
    double z1=-height/2.0;
    double z2=+height/2.0;
    
    Vector3 P1(x1,0,0);
    Vector3 P2(x2,0,0);
    
    Vector3 v1(std::cos(a1),std::sin(a1),0);
    Vector3 v2(-std::cos(a2),std::sin(a2),0);
    
    Vector3 P21=P1-P2,v2x1,C;
    v2x1.crossprod(v2,v1);
    C.crossprod(P21,v2);
    
    double t=0,n=v2x1.norm_sqr();
    
    if(n!=0) t=scalar_prod(C,v2x1)/n;
    
    C=P1+t*v1;
    
    double xc=C.x;
    double yc=C.y;
    
    if(std::abs(yc)<std::abs(width))
    {
        V_arr.resize(6);
        F_arr.resize(9);
        
        V_arr[0].loc(x1,0,z1);
        V_arr[1].loc(x1,0,z2);
        V_arr[2].loc(x2,0,z1);
        V_arr[3].loc(x2,0,z2);
        V_arr[4].loc(xc,yc,z1);
        V_arr[5].loc(xc,yc,z2);
        
        F_arr[0].V1=0; F_arr[0].V2=1;
        F_arr[1].V1=2; F_arr[1].V2=3;
        F_arr[2].V1=4; F_arr[2].V2=5;
        
        F_arr[3].V1=0; F_arr[3].V2=2;
        F_arr[4].V1=2; F_arr[4].V2=4;
        F_arr[5].V1=4; F_arr[5].V2=0;
        
        F_arr[6].V1=1; F_arr[6].V2=3;
        F_arr[7].V1=3; F_arr[7].V2=5;
        F_arr[8].V1=5; F_arr[8].V2=1;
    }
    else
    {
        yc=std::abs(width);
        
        Vector3 H(0,yc,0);
        
        v2x1.crossprod(unit_vec_x,v1);
        Vector3 D=P1-H; C.crossprod(D,unit_vec_x);
        t=scalar_prod(C,v2x1)/v2x1.norm_sqr();
        
        C=P1+t*v1;
        double x1=C.x;
        
        v2x1.crossprod(unit_vec_x,v2);
        D=P2-H; C.crossprod(D,unit_vec_x);
        t=scalar_prod(C,v2x1)/v2x1.norm_sqr();
        
        C=P2+t*v2;
        double x2=C.x;
        
        Glite::make_block_wires(V_arr,F_arr,length,width,height,-0.5,0,-0.5);
        
        V_arr[2].loc.x=x2;
        V_arr[3].loc.x=x1;
        
        V_arr[6].loc.x=x2;
        V_arr[7].loc.x=x1;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void rectangle_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                              double ly,double lz)
{
    int Nv=8;
    int Nf=6;
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    V_arr[0].loc(0,-ly/2.0,-lz/2.0);
    V_arr[1].loc(0,+ly/2.0,-lz/2.0);
    V_arr[2].loc(0,+ly/2.0,+lz/2.0);
    V_arr[3].loc(0,-ly/2.0,+lz/2.0);
    
    F_arr[0].V1=0; F_arr[0].V2=1;
    F_arr[1].V1=1; F_arr[1].V2=2;
    F_arr[2].V1=2; F_arr[2].V2=3;
    F_arr[3].V1=3; F_arr[3].V2=0;
    
    V_arr[4].loc(0,-ly/2.0,0);
    V_arr[5].loc(0,+ly/2.0,0);
    V_arr[6].loc(0,0,-lz/2.0);
    V_arr[7].loc(0,0,+lz/2.0);
    
    F_arr[4].V1=4; F_arr[4].V2=5;
    F_arr[5].V1=6; F_arr[5].V2=7;
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
}

void sphere_cut_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                               double r,double cut_factor,bool volume_version)
{
    int i;
    
    int disc=128;
    
    cut_factor=std::min(1.0,cut_factor);
    cut_factor=std::max(0.0,cut_factor);
    
    double x_cut=r*(1.0-2.0*cut_factor);
    double th=std::acos(x_cut/r);
    double r_cut=r*std::sin(th);
    
    int Nv=3*(disc+1);
    int Nf=3*disc;
    
    if(cut_factor>0.5) { Nv+=disc+1; Nf+=disc; }
    if(volume_version) { Nv+=4; Nf+=2; }
    
    V_arr.resize(Nv);
    F_arr.resize(Nf);
    
    // Arcs
    
    for(i=0;i<=disc;i++)
    {
        double ang=-th+i/(disc+0.0)*2.0*th;
        
        V_arr[i].loc(r*std::cos(ang),r*std::sin(ang),0);
        V_arr[i+disc+1].loc(r*std::cos(ang),0,r*std::sin(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[i].V1=i; F_arr[i].V2=i+1;
        F_arr[i+disc].V1=i+disc+1; F_arr[i+disc].V2=i+disc+2;
    }
    
    int v_offset=2*(disc+1);
    int f_offset=2*disc;
    
    // Flat Patch
    
    for(i=0;i<=disc;i++)
    {
        double ang=2.0*Pi*i/(disc+0.0);
        V_arr[i+v_offset].loc(x_cut,r_cut*std::cos(ang),r_cut*std::sin(ang));
    }
    
    for(i=0;i<disc;i++)
    {
        F_arr[i+f_offset].V1=i+v_offset;
        F_arr[i+f_offset].V2=i+1+v_offset;
    }
    
    v_offset+=disc+1;
    f_offset+=disc;
    
    // Perimeter
    
    if(cut_factor>0.5)
    {
        for(i=0;i<=disc;i++)
        {
            double ang=2.0*Pi*i/(disc+0.0);
            V_arr[i+v_offset].loc(0,r*std::cos(ang),r*std::sin(ang));
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+f_offset].V1=i+v_offset;
            F_arr[i+f_offset].V2=i+1+v_offset;
        }
        
        v_offset+=disc+1;
        f_offset+=disc;
    }
    
    chk_var(Nv);
    chk_var(Nf);
    chk_var(v_offset);
    chk_var(f_offset);
    
    if(volume_version)
    {
        V_arr[v_offset  ].loc(x_cut,-r_cut,0);
        V_arr[v_offset+1].loc(x_cut,+r_cut,0);
        V_arr[v_offset+2].loc(x_cut,0,-r_cut);
        V_arr[v_offset+3].loc(x_cut,0,+r_cut);
        
        F_arr[f_offset].V1=v_offset;
        F_arr[f_offset].V2=v_offset+1;
        
        F_arr[f_offset+1].V1=v_offset+2;
        F_arr[f_offset+1].V2=v_offset+3;
    }
    
    for(unsigned int i=0;i<F_arr.size();i++)
        F_arr[i].V3=F_arr[i].V2;
//    
//    // Perimeter
//    
}

void wireframe_mesh_add_arc(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                             Vector3 const &V1,Vector3 const &V2,double th_start,double th_end)
{
    std::size_t v_offset=V_arr.size();
    std::size_t f_offset=F_arr.size();
    
    V_arr.resize(v_offset+disc+1);
    F_arr.resize(f_offset+disc);
    
    for(unsigned int i=0;i<=disc;i++)
    {
        double ang=th_start+(th_end-th_start)*i/static_cast<double>(disc);
        
        V_arr[v_offset+i].loc=std::cos(ang)*V1+std::sin(ang)*V2;
    }
    
    for(unsigned int i=0;i<disc;i++)
    {
        F_arr[f_offset+i].V1=v_offset+i;
        F_arr[f_offset+i].V2=v_offset+i+1;
    }
}

void wireframe_mesh_add_line(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                             Vector3 const &line_start,Vector3 const &line_end)
{
    std::size_t v_offset=V_arr.size();
    std::size_t f_offset=F_arr.size();
    
    V_arr.resize(v_offset+2);
    F_arr.resize(f_offset+1);
    
    V_arr[v_offset].loc=line_start;
    V_arr[v_offset+1].loc=line_end;
    
    F_arr[f_offset].V1=v_offset;
    F_arr[f_offset].V2=v_offset+1;
}

void wireframe_mesh_add_circle(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                               unsigned int disc,double x,double R)
{
    std::size_t v_offset=V_arr.size();
    std::size_t f_offset=F_arr.size();
    
    V_arr.resize(v_offset+disc);
    F_arr.resize(f_offset+disc);
        
    for(unsigned int i=0;i<disc;i++)
    {
        double ang=2.0*Pi*i/(disc-1.0);
        double c=std::cos(ang),s=std::sin(ang);
        
        V_arr[i+v_offset].loc(x,R*c,R*s);
        
        F_arr[i+f_offset].V1=i+v_offset;
        F_arr[i+f_offset].V2=i+v_offset+1;
    }
    
    F_arr[f_offset+disc-1].V2=v_offset;
}

void wireframe_mesh_add_cross(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                              double x,double y,double z,double R)
{
    std::size_t v_offset=V_arr.size();
    std::size_t f_offset=F_arr.size();
    
    V_arr.resize(v_offset+6);
    F_arr.resize(f_offset+3);
    
    V_arr[0+v_offset].loc(x-R,y,z);
    V_arr[1+v_offset].loc(x+R,y,z);
    V_arr[2+v_offset].loc(x,y-R,z);
    V_arr[3+v_offset].loc(x,y+R,z);
    V_arr[4+v_offset].loc(x,y,z-R);
    V_arr[5+v_offset].loc(x,y,z+R);
    
    F_arr[0+f_offset].V1=0+v_offset; F_arr[0+f_offset].V2=1+v_offset;
    F_arr[1+f_offset].V1=2+v_offset; F_arr[1+f_offset].V2=3+v_offset;
    F_arr[2+f_offset].V1=4+v_offset; F_arr[2+f_offset].V2=5+v_offset;
}

//###############
//  SeleneVAO
//###############

SeleneVAO::SeleneVAO()
    :display(false), wireframe(false),
     color(1.0,1.0,1.0),
     O(Vector3(0,0,0)),
     A(Vector3(1.0,0,0)),
     B(Vector3(0,1.0,0)),
     C(Vector3(0,0,1.0))
{
}

void SeleneVAO::draw()
{
    if(!display) return;
    
    if(!wireframe)
    {
        glBindVertexArray(vao);
        
        glVertexAttrib4f(2,color.x,color.y,color.z,1.0);
        glVertexAttrib4f(3,A.x,A.y,A.z,0);
        glVertexAttrib4f(4,B.x,B.y,B.z,0);
        glVertexAttrib4f(5,C.x,C.y,C.z,0);
        glVertexAttrib4f(6,O.x,O.y,O.z,1.0);
        
        glDrawElements(GL_TRIANGLES,3*Nf,GL_UNSIGNED_INT,0);
    }
}

void SeleneVAO::draw_wireframe()
{
    if(!display) return;
    
    if(wireframe)
    {
        glBindVertexArray(vao_w);
        
        glVertexAttrib4f(2,color.x,color.y,color.z,1.0);
        glVertexAttrib4f(3,A.x,A.y,A.z,0);
        glVertexAttrib4f(4,B.x,B.y,B.z,0);
        glVertexAttrib4f(5,C.x,C.y,C.z,0);
        glVertexAttrib4f(6,O.x,O.y,O.z,1.0);
        
        glDrawElements(GL_LINES,6*Ne,GL_UNSIGNED_INT,0);
    }
}

void SeleneVAO::init_opengl()
{
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&v_buff);
    glGenBuffers(1,&vn_buff);
    glGenBuffers(1,&index_buff);
    
    glGenVertexArrays(1,&vao_w);
    glGenBuffers(1,&v_buff_w);
    glGenBuffers(1,&vn_buff_w);
    glGenBuffers(1,&index_buff_w);
}

void SeleneVAO::set_matrix(Vector3 const &O_,
                           Vector3 const &A_,
                           Vector3 const &B_,
                           Vector3 const &C_)
{
    O=O_;
    A=A_;
    B=B_;
    C=C_;
}

void SeleneVAO::set_mesh_solid(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr)
{
    int i;
    
    int Nv=V_arr.size();
    Nf=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[3*Nf];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Nf;i++)
    {
        ind_arr_buff[3*i+0]=F_arr[i].V1;
        ind_arr_buff[3*i+1]=F_arr[i].V2;
        ind_arr_buff[3*i+2]=F_arr[i].V3;
    }
    
    // Solid VAO
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3*Nf*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void SeleneVAO::set_mesh_wireframe(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr)
{
    int i;
    
    int Nv=V_arr.size();
    Ne=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[6*Ne];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
        
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Ne;i++)
    {
        ind_arr_buff[6*i+0]=F_arr[i].V1;
        ind_arr_buff[6*i+1]=F_arr[i].V2;
        ind_arr_buff[6*i+2]=F_arr[i].V2;
        ind_arr_buff[6*i+3]=F_arr[i].V3;
        ind_arr_buff[6*i+4]=F_arr[i].V3;
        ind_arr_buff[6*i+5]=F_arr[i].V1;
    }
    
    // Wires VAO
    
    glBindVertexArray(vao_w);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff_w);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 6*Ne*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void SeleneVAO::set_shading(Vector3 const &color_,bool wireframe_,bool display_)
{
    color=color_;
    wireframe=wireframe_;
    display=display_;
}

//###############
//   GL_Selene
//###############

GL_Selene::GL_Selene(wxWindow *parent)
    :GL_3D_Base(parent),
     Nrays(0),
     display_type(1),
     gen_max(0),
     gen_min_disp(1), gen_max_disp(10),
     lambda_min(370e-9), lambda_max(850e-9),
     lambda_min_disp(lambda_min),
     lambda_max_disp(lambda_max),
     lost_length(0.1)
{
}

void GL_Selene::delete_vao(SeleneVAO *vao_)
{
    unsigned int i,j;
    
    for(i=0;i<vao.size();i++)
    {
        if(vao[i]==vao_)
        {
            delete vao[i];
            
            for(j=i+1;j<vao.size();j++) vao[j-1]=vao[j];
            vao.pop_back();
            
            break;
        }
    }
}

void GL_Selene::init_opengl()
{
    std::filesystem::path v_shader,f_shader;
    
    v_shader=PathManager::locate_resource("resources/glsl/FD_vshader.glsl");
    f_shader=PathManager::locate_resource("resources/glsl/FD_solid_fshader.glsl");
    
    prog_solid=Glite::create_program(v_shader,f_shader);
    
    v_shader=PathManager::locate_resource("resources/glsl/FD_vshader.glsl");
    f_shader=PathManager::locate_resource("resources/glsl/FD_wires_fshader.glsl");
    
    prog_wires=Glite::create_program(v_shader,f_shader);
    
    // Ray
        
    v_shader=PathManager::locate_resource("resources/glsl/selene/rays_disp_vshader.glsl");
    f_shader=PathManager::locate_resource("resources/glsl/selene/rays_disp_fshader.glsl");
    
    prog_rays=Glite::create_program(v_shader,f_shader);
    
    SeleneVAO tmp_ray_vao;
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
    Glite::make_unitary_block(V_arr,F_arr);
    
    glBindVertexArray(ray_vao);
    
    glGenVertexArrays(1,&ray_vao);
    glGenBuffers(1,&ray_v_buff);
    glGenBuffers(1,&ray_offset_buff);
    glGenBuffers(1,&ray_A_buff);
    glGenBuffers(1,&ray_gen_buff);
    glGenBuffers(1,&ray_lambda_buff);
    glGenBuffers(1,&ray_lost_buff);
    
    int Nv=V_arr.size();
    GLfloat v_arr_buff[8];
    
    v_arr_buff[0]=0;
    v_arr_buff[1]=0;
    v_arr_buff[2]=0;
    v_arr_buff[3]=1;
    
    v_arr_buff[4]=1;
    v_arr_buff[5]=0;
    v_arr_buff[6]=0;
    v_arr_buff[7]=1;
    
    glBindVertexArray(ray_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_offset_buff);
    glBufferData(GL_ARRAY_BUFFER,4*50000*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_A_buff);
    glBufferData(GL_ARRAY_BUFFER,4*50000*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_gen_buff);
    glBufferData(GL_ARRAY_BUFFER,50000*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_lambda_buff);
    glBufferData(GL_ARRAY_BUFFER,50000*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    glVertexAttribPointer(5,1,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_lost_buff);
    glBufferData(GL_ARRAY_BUFFER,50000*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    glVertexAttribPointer(6,1,GL_FLOAT,GL_FALSE,0,0);
    
    glVertexAttribDivisor(2,1);
    glVertexAttribDivisor(3,1);
    glVertexAttribDivisor(4,1);
    glVertexAttribDivisor(5,1);
    glVertexAttribDivisor(6,1);
    
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    
    //

    glLineWidth(3);
}

SeleneVAO* GL_Selene::request_vao()
{
    focus();
    
    SeleneVAO *n_vao=new SeleneVAO;
    n_vao->init_opengl();
    
    vao.push_back(n_vao);
    
    return n_vao;
}

void GL_Selene::render()
{
    std::unique_lock lock(display_mutex);
    
    glLineWidth(1);
    
    glUseProgram(prog_rays);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    glUniform1f(14,lost_length);
    
    if(display_type==0)
    {
        glUniform1f(15,0);
        glUniform1f(16,gen_max);
        glUniform1f(17,lambda_min_disp);
        glUniform1f(18,lambda_max_disp);
    }
    else
    {
        glUniform1f(15,gen_min_disp);
        glUniform1f(16,gen_max_disp);
        glUniform1f(17,lambda_min);
        glUniform1f(18,lambda_max);
    }
    glUniform1i(19,display_type);
    
    glBindVertexArray(ray_vao);
    glDrawArraysInstanced(GL_LINES,0,2,Nrays);
    
//    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
//    glDrawElementsInstanced(GL_TRIANGLES,36,GL_UNSIGNED_INT,0,Nrays);
    
//    glUseProgram(prog_solid);
//    
//    Vector3 sun_dir(1.0,1.0,1.0);
//    sun_dir.set_spherical(1.0,Pi/4.0,Pi/3.0);
//    
//    glUniformMatrix4fv(10,1,0,camera.proj_gl);
//    glUniform4f(11,sun_dir.x,
//                   sun_dir.y,
//                   sun_dir.z,
//                   0);
//    
//    vao.draw();
    
    
    glLineWidth(3);
    
    glUseProgram(prog_wires);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    for(unsigned int i=0;i<vao.size();i++) vao[i]->draw_wireframe();
}

void GL_Selene::set_rays(std::vector<double> const &x1,std::vector<double> const &x2,
                         std::vector<double> const &y1,std::vector<double> const &y2,
                         std::vector<double> const &z1,std::vector<double> const &z2,
                         std::vector<int> const &gen,std::vector<double> const &lambda,
                         std::vector<bool> const &lost)
{
    std::unique_lock lock(display_mutex);
    
    Nrays=std::min(50000,int(x1.size()));
    
    GLfloat *buffer_pos=new GLfloat[4*Nrays];
    GLfloat *buffer_A=new GLfloat[4*Nrays];
    GLfloat *buffer_gen=new GLfloat[Nrays];
    GLfloat *buffer_lambda=new GLfloat[Nrays];
    GLfloat *buffer_lost=new GLfloat[Nrays];
    
    gen_max=0;
    lambda_min=std::numeric_limits<double>::max();
    lambda_max=0;
    
    for(int i=0;i<Nrays;i++)
    {
        gen_max=std::max(gen_max,gen[i]);
        lambda_min=std::min(lambda_min,lambda[i]);
        lambda_max=std::max(lambda_max,lambda[i]);
    }
    gen_max=std::max(1,gen_max);
    
    for(int i=0;i<Nrays;i++)
    {
        int j=4*i;
        
        Vector3 A(x2[i]-x1[i],y2[i]-y1[i],z2[i]-z1[i]),B,C;
        
        buffer_pos[j+0]=x1[i]; buffer_pos[j+1]=y1[i]; buffer_pos[j+2]=z1[i]; buffer_pos[j+3]=1;
        buffer_A[j+0]=A.x; buffer_A[j+1]=A.y; buffer_A[j+2]=A.z; buffer_A[j+3]=0;
        buffer_gen[i]=gen[i];
        buffer_lambda[i]=lambda[i];
        buffer_lost[i]=lost[i];
    }
    
    focus();
    
    glBindVertexArray(ray_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_offset_buff);
    glBufferSubData(GL_ARRAY_BUFFER,0,4*Nrays*sizeof(GLfloat),buffer_pos);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_A_buff);
    glBufferSubData(GL_ARRAY_BUFFER,0,4*Nrays*sizeof(GLfloat),buffer_A);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_gen_buff);
    glBufferSubData(GL_ARRAY_BUFFER,0,Nrays*sizeof(GLfloat),buffer_gen);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_lambda_buff);
    glBufferSubData(GL_ARRAY_BUFFER,0,Nrays*sizeof(GLfloat),buffer_lambda);
    
    glBindBuffer(GL_ARRAY_BUFFER,ray_lost_buff);
    glBufferSubData(GL_ARRAY_BUFFER,0,Nrays*sizeof(GLfloat),buffer_lost);
    
    delete[] buffer_pos;
    delete[] buffer_A;
    delete[] buffer_gen;
    delete[] buffer_lambda;
    delete[] buffer_lost;
}

//#######################
//   GL_Selene_Minimal
//#######################

GL_Selene_Minimal::GL_Selene_Minimal(wxWindow *parent)
    :GL_3D_Base(parent)
{
    
}

SeleneVAO* GL_Selene_Minimal::get_vao() { return &vao; }

void GL_Selene_Minimal::init_opengl()
{
    prog_solid=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_solid_fshader.glsl"));
    
    prog_wires=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_wires_fshader.glsl"));
    
    glLineWidth(3);
    
    vao.init_opengl();
}

void GL_Selene_Minimal::render()
{
//    glUseProgram(prog_solid);
//    
//    Vector3 sun_dir(1.0,1.0,1.0);
//    sun_dir.set_spherical(1.0,Pi/4.0,Pi/3.0);
//    
//    glUniformMatrix4fv(10,1,0,camera.proj_gl);
//    glUniform4f(11,sun_dir.x,
//                   sun_dir.y,
//                   sun_dir.z,
//                   0);
//    
//    vao.draw();
    
    glUseProgram(prog_wires);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    vao.draw_wireframe();
}

}
