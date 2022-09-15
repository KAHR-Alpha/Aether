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

#ifndef RAY_INTERSECT_H_INCLUDED
#define RAY_INTERSECT_H_INCLUDED

#include <algorithm>
#include <mathUT.h>
#include <geometry.h>
#include <list>

class RayFaceIntersect
{
    public:
        int ftarget;
        double t_intersect,u,v;
        
        RayFaceIntersect(int ftarget_,double t_intersect_,double u_,double v_)
            :ftarget(ftarget_), t_intersect(t_intersect_), u(u_), v(v_)
        {}
        
        bool operator < (RayFaceIntersect const &inter)
        {
            return t_intersect<inter.t_intersect;
        }
};

bool ray_inter_cone_x(Vector3 const &O,Vector3 const &D,
                      double x0,double r,double h,
                      double &t1,double &t2);
bool ray_inter_conic_section_x(Vector3 const &O,Vector3 const &D,
                               double R,double K,double in_radius,double out_radius,
                               double &t1,double &t2);
bool ray_inter_coupola(Vector3 const &O,Vector3 const &D,
                       Vector3 const &C,Vector3 const &N,double r,double cth,
                       double &t1,double &t2);
bool ray_inter_cylinder(Vector3 const &O,Vector3 const &D,
                        Vector3 const &L,Vector3 const &N,double r,double h,
                        double &t1,double &t2);
bool ray_inter_cylinder_x(Vector3 const &O,Vector3 const &D,
                          double x0,double r,double h,
                          double &t1,double &t2);
bool ray_inter_disk_x(Vector3 const &O,Vector3 const &D,
                      double x0,double in_radius,double out_radius,
                      double &t);
bool ray_inter_parabola_x(Vector3 const &O,Vector3 const &D,
                          double f,double in_radius,double length,
                          double &t1,double &t2);
bool ray_inter_plane_x(Vector3 const &O,Vector3 const &D,
                       double x0,double &t1);
bool ray_inter_sphere(Vector3 const &O,Vector3 const &D,
                      Vector3 const &C,double r,
                      double &t1,double &t2);
bool ray_inter_sphere(Vector3 const &O,Vector3 const &D,
                      double r,double &t1,double &t2);

/*//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
int ray_N_inter(Grid1<V> const &V_arr,Grid1<F> const &F_arr,
                int face_last_intersect,Vector3 const &O,Vector3 const &D)
{
    int k;
    int N_inter=0;
    
    int Nt=F_arr.L1();
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    for(k=0;k<Nt;k++)
    {
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr(k).V1;
            int const &V2=F_arr(k).V2;
            int const &V3=F_arr(k).V3;
            
            E1=V_arr(V2).loc-V_arr(V1).loc;
            E2=V_arr(V3).loc-V_arr(V1).loc;
            T=O-V_arr(V1).loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    if(scalar_prod(Q,E2)*invdet>=0) N_inter+=1;
                }
            }
        }
    }
    
    return N_inter;
}*/

//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
int ray_N_inter(std::vector<V> const &V_arr,std::vector<F> const &F_arr,
                int face_last_intersect,Vector3 const &O,Vector3 const &D)
{
    int k;
    int N_inter=0;
    
    int Nt=F_arr.size();
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    for(k=0;k<Nt;k++)
    {
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    if(scalar_prod(Q,E2)*invdet>=0) N_inter+=1;
                }
            }
        }
    }
    
    return N_inter;
}

/*template<class V,class F>
int ray_N_inter_list(Grid1<V> const &V_arr,Grid1<F> const &F_arr,std::list<int> const &flist,
                     int face_last_intersect,Vector3 const &O,Vector3 const& D)
{
    int k;
    int N_inter=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::list<int> hit_list;
    std::list<int>::const_iterator iterator,hit_iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr(k).V1;
            int const &V2=F_arr(k).V2;
            int const &V3=F_arr(k).V3;
            
            E1=V_arr(V2).loc-V_arr(V1).loc;
            E2=V_arr(V3).loc-V_arr(V1).loc;
            T=O-V_arr(V1).loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    if(scalar_prod(Q,E2)*invdet>=0)
                    {
                        bool already_hit=false;
                        
                        for(hit_iterator=hit_list.begin();hit_iterator!=hit_list.end();hit_iterator++)
                            if(k==*hit_iterator)
                            {
                                already_hit=true;
                                break;
                            }
                        
                        if(!already_hit)
                        {
                            N_inter+=1;
                            hit_list.push_back(k);
                        }
                    }
                }
            }
        }
    }
    
    return N_inter;
}*/

template<class V,class F>
int ray_N_inter_list(std::vector<V> const &V_arr,std::vector<F> const &F_arr,std::list<int> const &flist,
                     int face_last_intersect,Vector3 const &O,Vector3 const& D)
{
    int k;
    int N_inter=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::list<int> hit_list;
    std::list<int>::const_iterator iterator,hit_iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    if(scalar_prod(Q,E2)*invdet>=0)
                    {
                        bool already_hit=false;
                        
                        for(hit_iterator=hit_list.begin();hit_iterator!=hit_list.end();hit_iterator++)
                            if(k==*hit_iterator)
                            {
                                already_hit=true;
                                break;
                            }
                        
                        if(!already_hit)
                        {
                            N_inter+=1;
                            hit_list.push_back(k);
                        }
                    }
                }
            }
        }
    }
    
    return N_inter;
}

template<class V,class F>
void ray_inter(std::vector<V> const &V_arr,std::vector<F> const &F_arr,int face_last_intersect,
               Vector3 const &O,Vector3 const& D,int &ftarget,double &t_intersec,double &uo,double &vo)
{
    int k;
    
    ftarget=-1;
    t_intersec=1e100;
    
    int Nt=F_arr.size();
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    for(k=0;k<Nt;k++)
    {
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0 && t<t_intersec)
                    {
                        ftarget=k;
                        t_intersec=t;
                        uo=u;
                        vo=v;
                    }
                }
            }
        }
    }
}

template<class V,class F>
void ray_inter(std::vector<V> const &V_arr,std::vector<F> const &F_arr,int face_last_intersect,
               Vector3 const &O,Vector3 const& D,std::vector<RayFaceIntersect> &ray_face_intersect)
{
    int k;
    
    int Nt=F_arr.size();
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    for(k=0;k<Nt;k++)
    {
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0)
                    {
                        ray_face_intersect.push_back(RayFaceIntersect(k,t,u,v));
                    }
                }
            }
        }
    }
}

/*//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
void ray_inter_list(Grid1<V> const &V_arr,Grid1<F> const &F_arr,std::list<int> const &flist,int face_last_intersect,
                    Vector3 const &O,Vector3 const& D,int &ftarget,double &t_intersec,double &uo,double &vo)
{
    int k;
    
    ftarget=-1;
    t_intersec=1e100;
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::list<int>::const_iterator iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr(k).V1;
            int const &V2=F_arr(k).V2;
            int const &V3=F_arr(k).V3;
            
            E1=V_arr(V2).loc-V_arr(V1).loc;
            E2=V_arr(V3).loc-V_arr(V1).loc;
            T=O-V_arr(V1).loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0 && t<t_intersec)
                    {
                        ftarget=k;
                        t_intersec=t;
                        uo=u;
                        vo=v;
                    }
                }
            }
        }
    }
}*/

//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
void ray_inter_list(std::vector<V> const &V_arr,std::vector<F> const &F_arr,std::list<int> const &flist,int face_last_intersect,
                    Vector3 const &O,Vector3 const& D,int &ftarget,double &t_intersec,double &uo,double &vo)
{
    int k;
    
    ftarget=-1;
    t_intersec=1e100;
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::list<int>::const_iterator iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0 && t<t_intersec)
                    {
                        ftarget=k;
                        t_intersec=t;
                        uo=u;
                        vo=v;
                    }
                }
            }
        }
    }
}

//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
void ray_inter_vector(std::vector<V> const &V_arr,std::vector<F> const &F_arr,std::vector<int> const &flist,int face_last_intersect,
                      Vector3 const &O,Vector3 const& D,int &ftarget,double &t_intersec,double &uo,double &vo)
{
    int k;
    
    ftarget=-1;
    t_intersec=1e100;
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::vector<int>::const_iterator iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0 && t<t_intersec)
                    {
                        ftarget=k;
                        t_intersec=t;
                        uo=u;
                        vo=v;
                    }
                }
            }
        }
    }
}

//Ray intersection using Möller & Trumbore's algorithm
// "Fast, Minimum Storage Ray/Triangle Intersection"
template<class V,class F>
void ray_inter_vector(std::vector<V> const &V_arr,std::vector<F> const &F_arr,std::vector<int> const &flist,int face_last_intersect,
                      Vector3 const &O,Vector3 const& D,std::vector<RayFaceIntersect> &ray_face_intersect)
{
    int k;
    
    double t=0;
    
    Vector3 E1,E2,P,Q,T;
    double det,u,v;
    double invdet;
    
    std::vector<int>::const_iterator iterator;
    
    for(iterator=flist.begin();iterator!=flist.end();iterator++)
    {
        k=*iterator;
        
        if(k!=face_last_intersect)
        {
            int const &V1=F_arr[k].V1;
            int const &V2=F_arr[k].V2;
            int const &V3=F_arr[k].V3;
            
            E1=V_arr[V2].loc-V_arr[V1].loc;
            E2=V_arr[V3].loc-V_arr[V1].loc;
            T=O-V_arr[V1].loc;
            
            P.crossprod(D,E2);
            Q.crossprod(T,E1);
            
            det=scalar_prod(P,E1);
            invdet=1.0/det;
            
            u=scalar_prod(P,T)*invdet;
            
            if(u>=0 && u<=1.0)
            {
                v=scalar_prod(Q,D)*invdet;
                
                if(v>=0 && u+v<=1.0)
                {
                    t=scalar_prod(Q,E2)*invdet;
                    
                    if(t>=0)
                    {
                        ray_face_intersect.push_back(RayFaceIntersect(k,t,u,v));
                    }
                }
            }
        }
    }
}

#endif // RAY_INTERSECT_H_INCLUDED
