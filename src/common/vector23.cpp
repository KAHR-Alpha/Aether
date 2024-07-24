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

#include <geometry.h>


extern const Imdouble Im;

//#############
//   Vector2
//#############

Vector2::Vector2()
    :x(0), y(0)
{
}

Vector2::Vector2(double x_,double y_)
    :x(x_), y(y_)
{
}

Vector2::Vector2(double x1,double y1,
                 double x2,double y2)
    :x(x2-x1), y(y2-y1)
{
}

Vector2::Vector2(Vector2 const& V)
    :x(V.x), y(V.y)
{
}

double Vector2::norm() const
{
    return std::sqrt(x*x+y*y);
}

void Vector2::normalize()
{
    double n=norm();
    
    x=x/n;
    y=y/n;
}

void Vector2::operator = (double a)
{
    x=y=a;
}

void Vector2::operator =(Vector2 const &V)
{
    x=V.x;
    y=V.y;
}

void Vector2::operator =(Vector3 const &V)
{
    x=V.x;
    y=V.y;
}

void Vector2::operator *=(double val)
{
    x*=val;
    y*=val;
}

void Vector2::operator () (double a,double b)
{
    x=a;
    y=b;
}

void Vector2::randnorm()
{
    double a=2.0*Pi*std::rand()/(RAND_MAX-1.0);

    x=std::cos(a);
    y=std::sin(a);
}

void Vector2::split(double &x_,double &y_)
{
    x_=x;
    y_=y;
}

Vector2 operator + (Vector2 const&V1,Vector2 const&V2)
{
    return Vector2(V1.x+V2.x,V1.y+V2.y);
}

Vector2 operator - (Vector2 const&V1,Vector2 const&V2)
{
    return Vector2(V1.x-V2.x,V1.y-V2.y);
}

Vector2 operator * (double a,Vector2 const&V)
{
    return Vector2(a*V.x,a*V.y);
}

Vector2 operator * (Vector2 const&V,double a)
{
    return Vector2(a*V.x,a*V.y);
}

Vector2 operator / (Vector2 const&V,double a)
{
    return Vector2(V.x/a,V.y/a);
}

//#############
//   Vector3
//#############

Vector3::Vector3()
    :x(0), y(0), z(0)
{
}

Vector3::Vector3(double a)
    :x(a), y(a), z(a)
{
}

Vector3::Vector3(double x_,double y_,double z_)
    :x(x_), y(y_), z(z_)
{
}

Vector3::Vector3(double x1,double y1,double z1,
                 double x2,double y2,double z2)
    :x(x2-x1), y(y2-y1), z(z2-z1)
{
}

Vector3::Vector3(Vector3 const&V)
    :x(V.x), y(V.y), z(V.z)
{
}

void Vector3::crossprod(Vector3 const &V1,Vector3 const &V2)
{
    x=V1.y*V2.z-V1.z*V2.y;
    y=V1.z*V2.x-V1.x*V2.z;
    z=V1.x*V2.y-V1.y*V2.x;
}

double Vector3::dominance(int i) const
{
    double dom=1.0;
    
    using std::max;
    using std::abs;
    
         if(i==0) dom=max(abs(y/x),abs(z/x));
    else if(i==1) dom=max(abs(x/y),abs(z/y));
    else if(i==2) dom=max(abs(x/z),abs(y/z));
    
    if(std::isinf(dom) || std::isnan(dom)) dom=1e80;
    
    return dom;
}

int Vector3::dominant() const
{
    using std::abs;
    
    double xa=abs(x);
    double ya=abs(y);
    double za=abs(z);
    
    if(xa>=ya && xa>=za) return 0;
    if(ya>=xa && ya>=za) return 1;
    if(za>=xa && za>=ya) return 2;
    
    return 0;
}

double Vector3::dotprod(Vector3 &a)
{
	return x*a.x+y*a.y+z*a.z;
}

void Vector3::get_spherical(double &r,double &th,double &ph)
{
    r=norm();
    ph=std::atan2(y,x);
    th=Pi/2.0-std::atan2(z,std::sqrt(x*x+y*y));
}

int Vector3::least_dominant() const
{
    using std::abs;
    
    double xa=abs(x);
    double ya=abs(y);
    double za=abs(z);
    
    if(ya>=xa && za>=xa) return 0;
    if(xa>=ya && za>=ya) return 1;
    if(xa>=za && ya>=za) return 2;
    
    return 0;
}

double Vector3::least_dominant_val() const
{
    using std::abs;
    
    double xa=abs(x);
    double ya=abs(y);
    double za=abs(z);
    
    if(ya>=xa && za>=xa) return xa;
    if(xa>=ya && za>=ya) return ya;
    if(xa>=za && ya>=za) return za;
    
    return 0;
}

int Vector3::most_dominant() const
{
    using std::abs;
    
    double xa=abs(x);
    double ya=abs(y);
    double za=abs(z);
    
    if(ya<=xa && za<=xa) return 0;
    if(xa<=ya && za<=ya) return 1;
    if(xa<=za && ya<=za) return 2;
    
    return 0;
}

double Vector3::most_dominant_val() const
{
    using std::abs;
    
    double xa=abs(x);
    double ya=abs(y);
    double za=abs(z);
    
    if(ya<=xa && za<=xa) return xa;
    if(xa<=ya && za<=ya) return ya;
    if(xa<=za && ya<=za) return za;
    
    return 0;
}

double Vector3::norm() const
{
    return std::sqrt(x*x+y*y+z*z);
}

double Vector3::norm_sqr() const
{
    return x*x+y*y+z*z;
}

void Vector3::normalize()
{
    double n=norm();

    x=x/n;
    y=y/n;
    z=z/n;
}

void Vector3::show()
{
    std::cout<<"X="<<x<<" Y="<<y<<" Z="<<z<<std::endl;
}

void Vector3::operator =(double a)
{
    x=y=z=a;
}

void Vector3::operator =(Vector2 const &V)
{
    x=V.x;
    y=V.y;
    z=0;
}

void Vector3::operator =(Vector3 const &V)
{
    x=V.x;
    y=V.y;
    z=V.z;
}

void Vector3::operator +=(Vector3 const &V)
{
    x+=V.x;
    y+=V.y;
    z+=V.z;
}

void Vector3::operator -=(Vector3 const &V)
{
    x-=V.x;
    y-=V.y;
    z-=V.z;
}

void Vector3::operator *=(double s)
{
    x*=s;
    y*=s;
    z*=s;
}

void Vector3::operator /=(double s)
{
    x/=s;
    y/=s;
    z/=s;
}

void Vector3::operator ()(double a,double b,double c)
{
    x=a;
    y=b;
    z=c;
}

void Vector3::randnorm_s()
{
    x=randp(-1,1);
    y=randp(-1,1);
    z=randp(-1,1);
    
    normalize();
}

void Vector3::rand_sph()
{
    double x0=randp(-1.0,1.0);
    double x1=randp(-1.0,1.0);
    double x2=randp(-1.0,1.0);
    double x3=randp(-1.0,1.0);
    
    double D=x0*x0+x1*x1+x2*x2+x3*x3;
    
    while(D>=1.0)
    {
        x0=randp(-1.0,1.0);
        x1=randp(-1.0,1.0);
        x2=randp(-1.0,1.0);
        x3=randp(-1.0,1.0);
        
        D=x0*x0+x1*x1+x2*x2+x3*x3;
    }
    
    x=2.0*(x1*x3+x0*x2)/D;
    y=2.0*(x2*x3-x0*x1)/D;
    z=(x0*x0+x3*x3-x1*x1-x2*x2)/D;
    
    normalize();
}

void Vector3::rand_sph(double r)
{
    rand_sph();
    
    x*=r;
    y*=r;
    z*=r;
}

void Vector3::set_latlonr(double lat, double lon, double r)
{
    using std::sin;
    using std::cos;
    
    double clat=cos(lat);
    
	x=r*sin(lon)*clat;	// cos(lat) always positive
	y=r*sin(lat);
	z=-r*cos(lon)*clat;	// because latitude -90 - 90 degrees !
}

void Vector3::set_spherical(double r,double th,double ph)
{
    using std::sin;
    using std::cos;
    
    double sth=sin(th);
    
    x=r*cos(ph)*sth;
    y=r*sin(ph)*sth;
    z=r*cos(th);
}

void Vector3::spheriCoor(double &th,double &ph)
{
    ph=std::atan2(y,x);
    th=Pi/2.0-std::atan2(z,std::sqrt(x*x+y*y));
}

void Vector3::split(double &x_,double &y_,double &z_)
{
    x_=x;
    y_=y;
    z_=z;
}

Vector3 Vector3::transverse(Vector3 const &N)
{
    Vector3 U=N;
    U.normalize();
    double p=x*U.x+y*U.y+z*U.z;
    
    U.x=x-p*U.x;
    U.x=y-p*U.y;
    U.x=z-p*U.z;
    
    return U;
}

double Vector3::xy() { return std::sqrt(x*x+y*y); }
double Vector3::xz() { return std::sqrt(x*x+z*z); }
double Vector3::yz() { return std::sqrt(y*y+z*z); }

Vector3 operator + (Vector3 const&V1)
{
    return Vector3(V1);
}

Vector3 operator + (Vector3 const&V1,Vector3 const&V2)
{
    return Vector3(V1.x+V2.x,V1.y+V2.y,V1.z+V2.z);
}

Vector3 operator - (Vector3 const&V1)
{
    return Vector3(-V1.x,-V1.y,-V1.z);
}

Vector3 operator - (Vector3 const&V1,Vector3 const&V2)
{
    return Vector3(V1.x-V2.x,V1.y-V2.y,V1.z-V2.z);
}

Vector3 operator * (double a,Vector3 const&V)
{
    return Vector3(a*V.x,a*V.y,a*V.z);
}

Vector3 operator * (Vector3 const&V,double a)
{
    return Vector3(a*V.x,a*V.y,a*V.z);
}

Vector3 operator / (Vector3 const&V,double a)
{
    return Vector3(V.x/a,V.y/a,V.z/a);
}

std::ostream& operator << (std::ostream &strm,Vector3 const &V)
{
    strm<<"X="<<V.x<<" Y="<<V.y<<" Z="<<V.z;
    return strm;
}

//####################
//     ImVector3
//####################

ImVector3::ImVector3()
    :x(0), y(0), z(0)
{
}

ImVector3::ImVector3(double a)
    :x(a), y(a), z(a)
{
}

ImVector3::ImVector3(Imdouble a)
    :x(a), y(a), z(a)
{
}

ImVector3::ImVector3(double a,double b,double c)
    :x(a), y(b), z(c)
{
}

ImVector3::ImVector3(Imdouble a,Imdouble b,Imdouble c)
    :x(a), y(b), z(c)
{
}

ImVector3::ImVector3(Vector3 const&V)
    :x(V.x), y(V.y), z(V.z)
{
}

ImVector3::ImVector3(ImVector3 const&V)
    :x(V.x), y(V.y), z(V.z)
{
}

ImVector3 ImVector3::conj() const
{
    return ImVector3(std::conj(x),
                     std::conj(y),
                     std::conj(z));
}

void ImVector3::crossprod(ImVector3 const &V1,ImVector3 const &V2)
{
    x=V1.y*V2.z-V1.z*V2.y;
    y=V1.z*V2.x-V1.x*V2.z;
    z=V1.x*V2.y-V1.y*V2.x;
}

double ImVector3::norm() const
{
    return std::sqrt(norm_sqr());
}

double ImVector3::norm_sqr() const
{
    using std::real;
    using std::conj;
    
    return real(x*conj(x)+y*conj(y)+z*conj(z));
}

void ImVector3::normalize()
{
    double n=norm();

    x=x/n;
    y=y/n;
    z=z/n;
}

void ImVector3::show()
{
    std::cout<<"X="<<x<<" Y="<<y<<" Z="<<z<<std::endl;
}

void ImVector3::operator =(double a)
{
    x=y=z=a;
}

void ImVector3::operator =(Imdouble a)
{
    x=y=z=a;
}

void ImVector3::operator =(Vector3 const &V)
{
    x=V.x;
    y=V.y;
    z=V.z;
}

void ImVector3::operator =(ImVector3 const &V)
{
    x=V.x;
    y=V.y;
    z=V.z;
}

void ImVector3::operator +=(Vector3 const &V)
{
    x+=V.x;
    y+=V.y;
    z+=V.z;
}

void ImVector3::operator +=(ImVector3 const &V)
{
    x+=V.x;
    y+=V.y;
    z+=V.z;
}

void ImVector3::operator -=(Vector3 const &V)
{
    x-=V.x;
    y-=V.y;
    z-=V.z;
}

void ImVector3::operator -=(ImVector3 const &V)
{
    x-=V.x;
    y-=V.y;
    z-=V.z;
}

void ImVector3::operator *=(double s)
{
    x*=s;
    y*=s;
    z*=s;
}

void ImVector3::operator *=(Imdouble s)
{
    x*=s;
    y*=s;
    z*=s;
}

void ImVector3::operator /=(double s)
{
    x/=s;
    y/=s;
    z/=s;
}

void ImVector3::operator /=(Imdouble s)
{
    x/=s;
    y/=s;
    z/=s;
}

void ImVector3::operator ()(double a,double b,double c)
{
    x=a;
    y=b;
    z=c;
}

void ImVector3::operator ()(Imdouble a,Imdouble b,Imdouble c)
{
    x=a;
    y=b;
    z=c;
}

ImVector3 operator + (ImVector3 const&V1,ImVector3 const&V2)
{
    return ImVector3(V1.x+V2.x,V1.y+V2.y,V1.z+V2.z);
}

ImVector3 operator - (ImVector3 const&V1)
{
    return ImVector3(-V1.x,-V1.y,-V1.z);
}

ImVector3 operator - (ImVector3 const&V1,ImVector3 const&V2)
{
    return ImVector3(V1.x-V2.x,V1.y-V2.y,V1.z-V2.z);
}

ImVector3 operator * (Imdouble c,Vector3 const &V) { return ImVector3(c*V.x,c*V.y,c*V.z); }
ImVector3 operator * (double a,ImVector3 const &V) { return ImVector3(a*V.x,a*V.y,a*V.z); }
ImVector3 operator * (Imdouble c,ImVector3 const &V) { return ImVector3(c*V.x,c*V.y,c*V.z); }
ImVector3 operator * (Vector3 const &V,Imdouble c) { return ImVector3(c*V.x,c*V.y,c*V.z); }
ImVector3 operator * (ImVector3 const &V,double a) { return ImVector3(a*V.x,a*V.y,a*V.z); }
ImVector3 operator * (ImVector3 const &V,Imdouble c) { return ImVector3(c*V.x,c*V.y,c*V.z); }

ImVector3 operator / (ImVector3 const&V,double a) { return ImVector3(V.x/a,V.y/a,V.z/a); }
ImVector3 operator / (ImVector3 const&V,Imdouble c) { return ImVector3(V.x/c,V.y/c,V.z/c); }
ImVector3 operator / (Vector3 const&V,Imdouble c) { return ImVector3(V.x/c,V.y/c,V.z/c); }

Vector3 crossprod(Vector3 const &V1,Vector3 const &V2)
{
    return Vector3(V1.y*V2.z-V1.z*V2.y,
                   V1.z*V2.x-V1.x*V2.z,
                   V1.x*V2.y-V1.y*V2.x);
}

ImVector3 crossprod(ImVector3 const &V1,ImVector3 const &V2)
{
    return ImVector3(V1.y*V2.z-V1.z*V2.y,
                     V1.z*V2.x-V1.x*V2.z,
                     V1.x*V2.y-V1.y*V2.x);
}

double scalar_prod(Vector2 const&V1,Vector2 const&V2)
{
    return V1.x*V2.x+V1.y*V2.y;
}

double scalar_prod(Vector3 const&V1,Vector3 const&V2)
{
    return V1.x*V2.x+V1.y*V2.y+V1.z*V2.z;
}
