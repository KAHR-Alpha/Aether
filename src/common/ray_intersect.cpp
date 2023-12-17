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

#include <ray_intersect.h>

extern std::ofstream plog;

bool ray_inter_cone_x(Vector3 const &O,Vector3 const &D,
                      double x0,double r,double h,
                      double &t1,double &t2)
{
    t1=t2=-1;
    
    Vector3 L(x0,0,0);
    Vector3 LO=O-L;
    
    Vector3 V1=D-Vector3(D.x,0,0);
    Vector3 V2=LO-Vector3(LO.x,0,0);
    
    double R1=-r/h*D.x;
    double R2=r-r/h*LO.x;
    
    double a=V1.norm_sqr()-R1*R1;
    double b=2.0*scalar_prod(V1,V2)-2.0*R1*R2;
    double c=V2.norm_sqr()-R2*R2;
    
    if(!polyn2_real(t1,t2,a,b,c)) return false;
    
    Vector3 P1=O+t1*D;
    Vector3 P2=O+t2*D;
    
    double z1=(P1-L).x;
    double z2=(P2-L).x;
    
    if(z1<0 || z1>h || std::isnan(t1) || std::isinf(t1)) t1=-1;
    if(z2<0 || z2>h || std::isnan(t2) || std::isinf(t2)) t2=-1;
    
    if(t1<0 && t2<0) return false;
    else return true; 
}

bool ray_inter_conic_section_x(Vector3 const &O,Vector3 const &D,
                               double R,double K,double in_radius,double out_radius,
                               double &t1,double &t2)
{
    t1=t2=-1;
    
    double X=1.0+K;
    
    double a=D.y*D.y+D.z*D.z+X*D.x*D.x;
    double b=2.0*(O.y*D.y+O.z*D.z+(X*O.x-R)*D.x);
    double c=O.y*O.y+O.z*O.z+X*O.x*O.x-2.0*R*O.x;
    
    if(std::abs(a)<1e-10)
    {
        t2=-1;
        t1=-c/b;
        
        if(t1<0) return false;
    }
    else if(!polyn2_real(t1,t2,a,b,c)) return false;
    
    double R2;    
    Vector3 V;
    
    double in2=in_radius*in_radius;
    double out2=out_radius*out_radius;
    
    double max_length=std::numeric_limits<double>::max();
    if(K>-1) max_length=R/(1+K);
    
    V=O+t1*D;
    R2=V.y*V.y+V.z*V.z;
    
    if(V.x<0 || V.x>=max_length || R2<in2 || R2>out2) t1=-1;
    
    V=O+t2*D;
    R2=V.y*V.y+V.z*V.z;
    
    if(V.x<0 || V.x>=max_length || R2<in2 || R2>out2) t2=-1;
    
    if(t1<0 && t2<0) return false;
    
    return true;
}

bool ray_inter_coupola(Vector3 const &O,Vector3 const &D,
                       Vector3 const &C,Vector3 const &N,double r,double cth,
                       double &t1,double &t2)
{
    if(!ray_inter_sphere(O,D,C,r,t1,t2)) return false;
    
    Vector3 P1=O+t1*D-C;
    Vector3 P2=O+t2*D-C;
    
    P1.normalize();
    P2.normalize();
    
    double cth1=scalar_prod(N,P1);
    double cth2=scalar_prod(N,P2);
    
    if(cth1<cth && cth2<cth) return false;
    
    if(cth1<cth) t1=-1;
    if(cth2<cth) t2=-1;
    
    return true;
}

bool ray_inter_cylinder(Vector3 const &O,Vector3 const &D,
                        Vector3 const &L,Vector3 const &N,double r,double h,
                        double &t1,double &t2)
{
    Vector3 LO=O-L;
    
    Vector3 V1=D-scalar_prod(D,N)*N;
    Vector3 V2=LO-scalar_prod(LO,N)*N;
    
    double a=V1.norm_sqr();
    double b=2.0*scalar_prod(V1,V2);
    double c=V2.norm_sqr()-r*r;
    
    double delta=b*b-4.0*a*c;
    
    if(delta<0) return false;
    
    double srd=std::sqrt(delta);
    
    t1=(-b-srd)/(2.0*a);
    t2=(-b+srd)/(2.0*a);
    
    Vector3 P1=O+t1*D;
    Vector3 P2=O+t2*D;
    
    double z1=scalar_prod(N,P1-L);
    double z2=scalar_prod(N,P2-L);
    
    if(z1<0 || z1>h) t1=-1;
    if(z2<0 || z2>h) t2=-1;
    
    if(t1<0 && t2<0) return false;
    else return true; 
}

bool ray_inter_cylinder_x(Vector3 const &O,Vector3 const &D,
                          double x0,double r,double h,
                          double &t1,double &t2)
{
    t1=t2=-1;
    
    Vector3 L(x0,0,0);
    Vector3 LO=O-L;
    
    Vector3 V1=D-Vector3(D.x,0,0);
    Vector3 V2=LO-Vector3(LO.x,0,0);
    
    double a=V1.norm_sqr();
    double b=2.0*scalar_prod(V1,V2);
    double c=V2.norm_sqr()-r*r;
    
    if(!polyn2_real(t1,t2,a,b,c)) return false;
    
    Vector3 P1=O+t1*D;
    Vector3 P2=O+t2*D;
    
    double z1=(P1-L).x;
    double z2=(P2-L).x;
    
    if(z1<0 || z1>h) t1=-1;
    if(z2<0 || z2>h) t2=-1;
    
    if(t1<0 && t2<0) return false;
    else return true; 
}

bool ray_inter_disk_x(Vector3 const &O,Vector3 const &D,
                      double x0,double in_radius,double out_radius,
                      double &t)
{
    t=-1;
    
    if(!ray_inter_plane_x(O,D,x0,t)) return false;
    
    Vector3 V=O+t*D;
    
    double r_yz=V.yz();
    if(r_yz<in_radius || r_yz>out_radius)return false;
    else return true;
}

bool ray_inter_parabola_x(Vector3 const &O,Vector3 const &D,
                          double f,double in_radius,double length,
                          double &t1,double &t2)
{
    t1=t2=-1;
    
    double a=(D.y*D.y+D.z*D.z);
    double b=2.0*(O.y*D.y+O.z*D.z-2.0*f*D.x);
    double c=O.y*O.y+O.z*O.z-4.0*f*O.x;
    
    if(std::abs(D.y)<1e-10 && std::abs(D.z)<1e-10)
    {
        t2=-1;
//        t1=c/(4.0*f);
        t1=-c/b;
        
        if(t1<0) return false;
    }
    else if(!polyn2_real(t1,t2,a,b,c)) return false;
        
    Vector3 V;
    
    V=O+t1*D;
    if(V.x>length || V.y*V.y+V.z*V.z<in_radius*in_radius) t1=-1;
    
    V=O+t2*D;
    if(V.x>length || V.y*V.y+V.z*V.z<in_radius*in_radius) t2=-1;
    
    if(t1<0 && t2<0) return false;
    
    return true;
}

bool ray_inter_plane_x(Vector3 const &O,Vector3 const &D,
                       double x0,double &t)
{
    t=-1;
    
    if(D.x==0) return false;
    
    t=-(O.x-x0)/D.x;
    
    if(t<0) return false;
    else return true;
}

bool ray_inter_sphere(Vector3 const &O,Vector3 const &D,
                      Vector3 const &C,double r,
                      double &t1,double &t2)
{
    Vector3 CO=O-C;
    
    double a=D.norm_sqr();
    double b=2.0*scalar_prod(D,CO);
    double c=CO.norm_sqr()-r*r;
    
    double delta=b*b-4.0*a*c;
    
    if(delta<0) return false;
    
    double srd=std::sqrt(delta);
    
    t1=(-b-srd)/(2.0*a);
    t2=(-b+srd)/(2.0*a);
    
    return true; 
}

bool ray_inter_sphere(Vector3 const &O,Vector3 const &D,
                      double r,double &t1,double &t2)
{
    double a=D.norm_sqr();
    double b=2.0*scalar_prod(D,O);
    double c=O.norm_sqr()-r*r;
    
    if(b==0)
    {
        t1=-r;
        t2=r;
    }
    else
    {
        double delta=b*b-4.0*a*c;
        
        if(delta<0) return false;
        
        double srd=std::sqrt(delta);
        
        t1=(-b-srd)/(2.0*a);
        t2=(-b+srd)/(2.0*a);
    }
    
    return true; 
}
