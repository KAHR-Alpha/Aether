/*Copyright 2008-2021 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <geometry.h>

extern const double Pi;

//###############
//   Triangle
//###############

Triangle::Triangle()
    :va(0,0,0), vb(0,0,0), vc(0,0,0)
{
}

Triangle::Triangle(Vector3 const &vai,Vector3 const &vbi,Vector3 const &vci)
    :va(vai), vb(vbi), vc(vci)
{
}

Triangle::Triangle(Triangle const &T)
    :va(T.va), vb(T.vb), vc(T.vc)
{
}

void Triangle::set(Vector3 const &vai,Vector3 const &vbi,Vector3 const &vci)
{
    va=vai;
    vb=vbi;
    vc=vci;
}

void Triangle::operator = (double a)
{
    va(0,0,0);
    vb(0,0,0);
    vc(0,0,0);
}

void Triangle::operator = (Triangle const &T)
{
    va=T.va;
    vb=T.vb;
    vc=T.vc;
}

const Vector3 Triangle::norm() const
{
    Vector3 tmp;
    tmp.crossprod(v_ab(),v_ac());
    tmp.normalize();
    const Vector3 R=tmp;
    return R;
}

void Triangle::translate(Vector3 disp)
{
    va=va+disp;
    vb=vb+disp;
    vc=vc+disp;
}

void Triangle::translate(double x,double y,double z)
{
    Vector3 disp(x,y,z);
    translate(disp);
}

Vector3 Triangle::v_ab() const { return vb-va; }
Vector3 Triangle::v_ba() const { return va-vb; }
Vector3 Triangle::v_ac() const { return vc-va; }
Vector3 Triangle::v_ca() const { return va-vc; }
Vector3 Triangle::v_bc() const { return vc-vb; }
Vector3 Triangle::v_cb() const { return vb-vc; }

//########################
//########################

Vector3 lines_intersect(Vector3 const &P1,Vector3 const &v1,Vector3 const &P2,Vector3 const &v2)
{
    Vector3 v12=crossprod(v2,v1);
    
    double t=scalar_prod(crossprod(P1-P2,v2),v12)/v12.norm_sqr();
    
    return P1+t*v1;
}

void lines_intersect(double &t1,double &t2,Vector3 const &P1,Vector3 const &v1,Vector3 const &P2,Vector3 const &v2)
{
    Vector3 v12=crossprod(v2,v1);
    double v12n=v12.norm_sqr();
    
    t1=scalar_prod(crossprod(P1-P2,v2),v12)/v12n;
    t2=scalar_prod(crossprod(P1-P2,v1),v12)/v12n;
}

double point_to_edge_distance(Vector3 const &P,Vector3 const &A,Vector3 const &B)
{
    Vector3 v=B-A;
    
    double sp=scalar_prod(P-A,v);
    double v2=v.norm_sqr();
    
    double t=sp/v2;
    
    if(t<0) t=0;
    else if(t>1.0) t=1.0;
        
    return (A+t*v-P).norm();
}

double point_to_line_distance(Vector3 const &P,Vector3 const &O,Vector3 const &v)
{
    double sp=scalar_prod(P-O,v);
    
    return std::sqrt((P-O).norm_sqr()-sp*sp/v.norm_sqr());
}

bool segments_intersect(Vector3 &P,
                        Vector3 const &A1,Vector3 const &A2,
                        Vector3 const &B1,Vector3 const &B2)
{
    double Ax_min=std::min(A1.x,A2.x);
    double Ax_max=std::max(A1.x,A2.x);
    double Ay_min=std::min(A1.y,A2.y);
    double Ay_max=std::max(A1.y,A2.y);
    
    double Bx_min=std::min(B1.x,B2.x);
    double Bx_max=std::max(B1.x,B2.x);
    double By_min=std::min(B1.y,B2.y);
    double By_max=std::max(B1.y,B2.y);
    
    bool check_neg=Ax_max<Bx_min || Ax_min>Bx_max ||
                   Ay_max<By_min || Ay_min>By_max;
    
    if(!check_neg)
    {
        Vector3 V1=A2-A1;
        Vector3 V2=B2-B1;
        
        double u,v;
        
        lines_intersect(u,v,A1,V1,B1,V2);
        
        if(u>=0 && u<=1.0 && v>=0 && v<=1.0)
        {
            P=A1+u*V1;
            return true;
        }
    }
    
    return false;
}

double vector_angle(Vector2 const &V1,Vector2 const &V2)
{
    double a=scalar_prod(V1,V2)/(V1.norm()*V2.norm());
    
    if(a<-1 || a>1) return Pi;
    else return std::acos(a);
}

double vector_angle(Vector3 const &V1,Vector3 const &V2)
{
    double a=scalar_prod(V1,V2)/(V1.norm()*V2.norm());
    
    if(a<-1 || a>1) return Pi;
    else return std::acos(a);
}

double vector_area(Vector3 const &V1,Vector3 const &V2,Vector3 const &V3)
{
    Vector3 R;
    R.crossprod(V2-V1,V3-V1);
    return R.norm()/2.0;
}
