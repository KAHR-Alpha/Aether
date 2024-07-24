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

#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <mathUT.h>

class Vector3;

class Vector2
{
    public:
        double x,y;
        
        Vector2();
        Vector2(double x,double y);
        Vector2(double x1,double y1,double x2,double y2);
        Vector2(Vector2 const &V);
        
        double norm() const;
        void normalize();
        void operator = (double);
        void operator = (Vector2 const&);
        void operator = (Vector3 const&);
        void operator *=(double);
        void operator () (double,double);
        void randnorm();
        void split(double &x,double &y);
        
        friend Vector2 operator + (Vector2 const&,Vector2 const&);
        friend Vector2 operator - (Vector2 const&,Vector2 const&);
        friend Vector2 operator * (double,Vector2 const&);
        friend Vector2 operator * (Vector2 const&,double);
        friend Vector2 operator / (Vector2 const&,double);
};

class Vector3
{
    public:
        double x,y,z;
        
        Vector3();
        Vector3(double a);
        Vector3(double x,double y,double z=0);
        Vector3(double x1,double y1,double z1,
                double x2,double y2,double z2);
        Vector3(Vector3 const &V);
        
        void crossprod(Vector3 const&,Vector3 const&);
        double dominance(int i) const;
        int dominant() const;
        void get_spherical(double &r,double &th,double &ph);
        double dotprod(Vector3 &a);
        int least_dominant() const;
        double least_dominant_val() const;
        int most_dominant() const;
        double most_dominant_val() const;
        double norm() const;
        double norm_sqr() const;
        void normalize();
        void randnorm_s();          // Randomize a point in a cube, and project to the unit sphere. Uneven distribution.
        void rand_sph();
        void rand_sph(double r);
        void set_latlonr(double lat, double lon, double r);        // Feel free to rename this if you like, but let me know -Aldaron
        void set_spherical(double r,double th,double ph);
        void show();
        void spheriCoor(double &th,double &ph);
        void split(double &x,double &y,double &z);
        Vector3 transverse(Vector3 const &N);
        double xy();
        double xz();
        double yz();
        
        void operator =(double);
        void operator =(Vector2 const&);
        void operator =(Vector3 const&);
        void operator +=(Vector3 const&);
        void operator -=(Vector3 const&);
        void operator *=(double);
        void operator /=(double);
        void operator ()(double,double,double);
        
        friend Vector3 operator + (Vector3 const&);
        friend Vector3 operator + (Vector3 const&,Vector3 const&);
        friend Vector3 operator - (Vector3 const&);
        friend Vector3 operator - (Vector3 const&,Vector3 const&);
        friend Vector3 operator * (double,Vector3 const&);
        friend Vector3 operator * (Vector3 const&,double);
        friend Vector3 operator / (Vector3 const&,double);
        friend std::ostream& operator << (std::ostream &strm,Vector3 const &V);
};

inline const Vector3 unit_vec_x(1.0,0,0);
inline const Vector3 unit_vec_y(0,1.0,0);
inline const Vector3 unit_vec_z(0,0,1.0);

class ImVector3
{
    public:
        Imdouble x,y,z;
        
        ImVector3();
        ImVector3(double);
        ImVector3(Imdouble);
        ImVector3(double,double,double);
        ImVector3(Imdouble,Imdouble,Imdouble);
        ImVector3(Vector3 const&);
        ImVector3(ImVector3 const&);
        
        template<typename vec1,typename vec2>
        void crossprod(vec1 const &V1,vec2 const &V2)
        {
            x=V1.y*V2.z-V1.z*V2.y;
            y=V1.z*V2.x-V1.x*V2.z;
            z=V1.x*V2.y-V1.y*V2.x;
        }
        
        ImVector3 conj() const;
        void crossprod(ImVector3 const&,ImVector3 const&);
        double norm() const;
        double norm_sqr() const;
        void normalize();
        void show();
        
        void operator =(double);
        void operator =(Imdouble);
        void operator =(Vector3 const&);
        void operator =(ImVector3 const&);
        void operator +=(Vector3 const&);
        void operator +=(ImVector3 const&);
        void operator -=(Vector3 const&);
        void operator -=(ImVector3 const&);
        void operator *=(double);
        void operator *=(Imdouble);
        void operator /=(double);
        void operator /=(Imdouble);
        void operator ()(double,double,double);
        void operator ()(Imdouble,Imdouble,Imdouble);
        
};

ImVector3 operator * (Imdouble c,ImVector3 const &V);
ImVector3 operator * (Imdouble c,Vector3 const &V);
ImVector3 operator * (ImVector3 const &V,Imdouble c);
ImVector3 operator * (Vector3 const &V,Imdouble c);
ImVector3 operator / (ImVector3 const &V,double a);
ImVector3 operator / (ImVector3 const &V,Imdouble c);
ImVector3 operator / (Vector3 const &V,Imdouble c);

class Triangle
{
    public:
        Vector3 va,vb,vc;
        
        Triangle();
        Triangle(Vector3 const &va,Vector3 const &vb,Vector3 const &vc);
        Triangle(Triangle const &T);
        
        void set(Vector3 const &va,Vector3 const &vb,Vector3 const &vc);
        
        void operator = (double);
        void operator = (Triangle const &T);
        
        const Vector3 norm() const;
        void translate(Vector3);
        void translate(double x,double y,double z);
        
        Vector3 v_ab() const;
        Vector3 v_ba() const;
        Vector3 v_ac() const;
        Vector3 v_ca() const;
        Vector3 v_bc() const;
        Vector3 v_cb() const;
        
};

Vector3 crossprod(Vector3 const&,Vector3 const&);
ImVector3 crossprod(ImVector3 const&,ImVector3 const&);
Vector3 lines_intersect(Vector3 const &P1,Vector3 const &v1,Vector3 const &P2,Vector3 const &v2);
void lines_intersect(double &t1,double &t2,Vector3 const &P1,Vector3 const &v1,Vector3 const &P2,Vector3 const &v2);
double point_to_edge_distance(Vector3 const &P,Vector3 const &A,Vector3 const &B);
double point_to_line_distance(Vector3 const &P,Vector3 const &O,Vector3 const &v);
double scalar_prod(Vector2 const &V1,Vector2 const &V2);
double scalar_prod(Vector3 const &V1,Vector3 const &V2);
bool segments_intersect(Vector3 &P,
                        Vector3 const &A1,Vector3 const &A2,
                        Vector3 const &B1,Vector3 const &B2);
double vector_angle(Vector2 const &V1,Vector2 const &V2);
double vector_angle(Vector3 const &V1,Vector3 const &V2);
double vector_area(Vector3 const &V1,Vector3 const &V2,Vector3 const &V3);

#endif // GEOMETRY_H_INCLUDED
