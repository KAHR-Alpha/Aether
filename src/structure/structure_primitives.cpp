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
#include <structure.h>



//###############
//   Add_Block
//###############

Add_Block::Add_Block(double x1_,double x2_,
                     double y1_,double y2_,
                     double z1_,double z2_,int mat_index_)
    :Structure_OP(x1_,x2_,y1_,y2_,z1_,z2_,mat_index_)
{
    if(x1>x2) std::swap(x1,x2);
    if(y1>y2) std::swap(y1,y2);
    if(z1>z2) std::swap(z1,z2);
}

int Add_Block::index(double x,double y,double z)
{
    if(x<x1 || x>=x2 || y<y1 || y>=y2 || z<z1 || z>=z2) return -1;
    
    return mat_index;
}

//#################
//   Add_Coating
//#################

Add_Coating::Add_Coating(AngleOld const &theta,AngleOld const &phi,
                         double thickness_,double delta_,int origin_mat_,int mat_index_)
    :Structure_OP(0,0,0,0,0,0,mat_index_),
     thickness(thickness_), delta(delta_), origin_mat(origin_mat_)
{
    dir.x=std::cos(phi)*std::sin(theta);
    dir.y=std::sin(phi)*std::sin(theta);
    dir.z=-std::cos(theta);
}

int Add_Coating::index(double x,double y,double z)
{
    if(parent->index(x,y,z,stack_ID)!=origin_mat) return -1;
    
    Vector3 O(x,y,z);
    
    int i=0;
    Vector3 disp=O;
    double lz=parent->lz;
    
    bool support=false;
    
    while((disp-O).norm_sqr()<=thickness*thickness && disp.z>0)
    {
        disp=O+i*delta*dir;
        
        if(parent->index(disp.x,disp.y,disp.z,stack_ID)!=origin_mat)
        {
            support=true;
            break;
        }
        
        i++;
    }
    
    if(!support) return -1;
    
    i=0;
    disp=O;
    
    while(disp.z<=lz)
    {
        disp=O-i*delta*dir;
        
        if(parent->index(disp.x,disp.y,disp.z,stack_ID)!=origin_mat) return -1;
        
        i++;
    }
    
    return mat_index;
}

//##############
//   Add_Cone
//##############

Add_Cone::Add_Cone(double x1_,double y1_,double z1_,
                   double x2_,double y2_,double z2_,double r_,int mat_index_)
    :Structure_OP(x1_,x2_,y1_,y2_,z1_,z2_,mat_index_),
     base_vec(x2-x1,y2-y1,z2-z1)
{
    r=r_;
    
    length=base_vec.norm();
    base_vec.normalize();
}

int Add_Cone::index(double x,double y,double z)
{
    if(x<std::min(x1,x2)-r || x>std::max(x1,x2)+r ||
       y<std::min(y1,y2)-r || y>std::max(y1,y2)+r ||
       z<std::min(z1,z2)-r || z>std::max(z1,z2)+r) return -1;
    
    Vector3 dv{x-x1,y-y1,z-z1};
    
    double proj=scalar_prod(base_vec,dv);
    
    if(proj>=0 && proj<=length)
    {
        dv=dv-proj*base_vec;
        double rt=dv.norm_sqr();
        
        double r2=r*(1.0-proj/length);
        r2=r2*r2;
        
        if(rt<=r2) return mat_index;
    }
    
    return -1;
}

//##################
//   Add_Cylinder
//##################

Add_Cylinder::Add_Cylinder(double x1_,double y1_,double z1_,
                           double x2_,double y2_,double z2_,double r_,int mat_index_)
    :Structure_OP(x1_,x2_,y1_,y2_,z1_,z2_,mat_index_),
     base_vec(x2-x1,y2-y1,z2-z1)
{
    r=r_;
    
    length=base_vec.norm();
    base_vec.normalize();
}

int Add_Cylinder::index(double x,double y,double z)
{
    if(x<std::min(x1,x2)-r || x>std::max(x1,x2)+r ||
       y<std::min(y1,y2)-r || y>std::max(y1,y2)+r ||
       z<std::min(z1,z2)-r || z>std::max(z1,z2)+r) return -1;
    
    Vector3 dv{x-x1,y-y1,z-z1};
    
    double proj=scalar_prod(base_vec,dv);
    
    if(proj>=0 && proj<=length)
    {
        dv=dv-proj*base_vec;
        double rt=dv.norm_sqr();
        
        if(rt<=r*r) return mat_index;
    }
    
    return -1;
}

//###################
//   Add_Ellipsoid
//###################

Add_Ellipsoid::Add_Ellipsoid(double x1_,double y1_,double z1_,
                             double rx_,double ry_,double rz_,int mat_index_)
    :Structure_OP(x1_,x1_,y1_,y1_,z1_,z1_,mat_index_),
     rx(rx_), ry(ry_), rz(rz_)
{
}

int Add_Ellipsoid::index(double x,double y,double z)
{
    if(x<x1-rx || x>x1+rx ||
       y<y1-ry || y>y1+ry ||
       z<z1-rz || z>z1+rz) return -1;
    
    x=(x-x1)/rx;
    y=(y-y1)/ry;
    z=(z-z1)/rz;
    
    if(x*x+y*y+z*z<=1.0) return mat_index;
    
    return -1;
}

//###############
//   Add_Layer
//###############

Add_Layer::Add_Layer(int type_,double z1_,double z2_,int mat_index_)
    :Structure_OP(0,0,0,0,z1_,z2_,mat_index_),
     type(type_)
{
}

int Add_Layer::index(double x,double y,double z)
{
    switch(type)
    {
        case 0:
            if(x>=z1 && x<z2) return mat_index;
            break;
        case 1:
            if(y>=z1 && y<z2) return mat_index;
            break;
        case 2:
            if(z>=z1 && z<z2) return mat_index;
            break;
    }
    
    return -1;
}

//####################
//   Add_Lua_Def
//####################

Add_Lua_Def::Add_Lua_Def(lua_State *L_,
                         std::string const &fname_,
                         std::vector<lua_tools::lua_type*> const &parameters_,
                         int mat_index_)
    :Structure_OP(0,0,0,0,0,0,mat_index_),
     L(L_),
     fname(fname_),
     parameters(parameters_)
{
}

int Add_Lua_Def::index(double x,double y,double z)
{
    int Narg=parameters.size();
    
    lua_getglobal(L,fname.c_str());
    
    if(lua_isnil(L,-1))
    {
        std::cout<<"Error, unknown function: "<<fname<<std::endl;
        std::cout<<"Ignoring operation"<<std::endl;
        
        return -1;
    }
    
    lua_pushvalue(L,-1);
    lua_pushnumber(L,x);
    lua_pushnumber(L,y);
    lua_pushnumber(L,z);
    
    for(int l=0;l<Narg;l++)
        parameters[l]->push_value(L);
    
    lua_call(L,3+Narg,1);
    
    if(lua_tointeger(L,-1))
    {
        lua_pop(L,2);
        return mat_index;
    }
    else
    {
        lua_pop(L,2);
        return -1;
    }
}

//##############
//   Add_Mesh
//##############

Add_Mesh::Add_Mesh(double x1_,double y1_,double z1_,
                   double scale_,std::filesystem::path fname_,int mat_index_)
    :Structure_OP(x1_,x1_,y1_,y1_,z1_,z1_,mat_index_),
     scale(scale_), fname(fname_)
{
    obj_file_load(fname,V_arr,F_arr);
    
    if(V_arr.empty())
    {
        std::cerr<<"Error: empty mesh "<<fname<<"\nAborting...";
        std::exit(EXIT_FAILURE);
    }
    
    x_min=x_max=V_arr[0].loc.x;
    y_min=y_max=V_arr[0].loc.y;
    z_min=z_max=V_arr[0].loc.z;
    
    for(unsigned int i=0;i<V_arr.size();i++)
    {
        V_arr[i].loc*=scale;
    
        x_min=std::min(x_min,V_arr[i].loc.x);
        x_max=std::max(x_max,V_arr[i].loc.x);
        
        y_min=std::min(y_min,V_arr[i].loc.y);
        y_max=std::max(y_max,V_arr[i].loc.y);
        
        z_min=std::min(z_min,V_arr[i].loc.z);
        z_max=std::max(z_max,V_arr[i].loc.z);
    }
}

int Add_Mesh::index(double x,double y,double z)
{
    if(    x<x_min || x>x_max
        || y<y_min || y>y_max
        || z<z_min || z>z_max) return -1;
    
    Vector3 O(x,y,z);
    Vector3 D;
    
    D.rand_sph();
    
    int N_inter=ray_N_inter(V_arr,F_arr,-1,O,D);
    if(N_inter%2!=0) return mat_index;
    
    return -1;
}

//###################
//   Add_Sin_Layer
//###################

Add_Sin_Layer::Add_Sin_Layer(double z1_,double a1_,double z2_,double a2_,
                             double px_,double phi_x_,
                             double py_,double phi_y_,int mat_index_)
    :Structure_OP(0,0,0,0,z1_,z2_,mat_index_),
     a1(a1_), a2(a2_),
     px(px_), phi_x(phi_x_),
     py(py_), phi_y(phi_y_)
{
}

int Add_Sin_Layer::index(double x,double y,double z)
{
    double h1,h2,t;
    
    double inv_px=1.0/px;
    double inv_py=1.0/py;
    
    if(px==0) inv_px=0;
    if(py==0) inv_py=0;
    
    t=0.5*(std::sin(2.0*Pi*(x*inv_px+phi_x))+
           std::sin(2.0*Pi*(y*inv_py+phi_y)));
    
    h1=z1+a1*t;
    h2=z2+a2*t;
    
    if(h1<=z && h2>=z) return mat_index;
    
    return -1;
}

//################
//   Add_Sphere
//################

Add_Sphere::Add_Sphere(double x1_,double y1_,double z1_,double r_,int mat_index_)
    :Structure_OP(x1_,x1_,y1_,y1_,z1_,z1_,mat_index_)
{
    r=r_;
}

int Add_Sphere::index(double x,double y,double z)
{
    x=x-x1;
    y=y-y1;
    z=z-z1;
    
    if(x*x+y*y+z*z<=r*r) return mat_index;
    
    return -1;
}

//####################
//   Add_Vect_Block
//####################

Add_Vect_Block::Add_Vect_Block(double xO_,double yO_,double zO_,
                               double xA_,double yA_,double zA_,
                               double xB_,double yB_,double zB_,
                               double xC_,double yC_,double zC_,int mat_index_)
    :Structure_OP(0,0,0,0,0,0,mat_index_),
     xO(xO_), yO(yO_), zO(zO_),
     xA(xA_), yA(yA_), zA(zA_),
     xB(xB_), yB(yB_), zB(zB_),
     xC(xC_), yC(yC_), zC(zC_)
{
    vectmat(0,0)=xA; vectmat(1,0)=yA; vectmat(2,0)=zA;
    vectmat(0,1)=xB; vectmat(1,1)=yB; vectmat(2,1)=zB;
    vectmat(0,2)=xC; vectmat(1,2)=yC; vectmat(2,2)=zC;
    
    Vector3 O(xO,yO,zO);
    Vector3 A(xA,yA,zA);
    Vector3 B(xB,yB,zB);
    Vector3 C(xC,yC,zC);
    
    Vector3 AB=O+A+B;
    Vector3 AC=O+A+C;
    Vector3 BC=O+B+C;
    Vector3 ABC=O+A+B+C;
    
    A=A+O; B=B+O; C=C+O;
    
    x_min=var_min(O.x,A.x,B.x,C.x,AB.x,AC.x,BC.x,ABC.x);
    x_max=var_max(O.x,A.x,B.x,C.x,AB.x,AC.x,BC.x,ABC.x);
    
    y_min=var_min(O.y,A.y,B.y,C.y,AB.y,AC.y,BC.y,ABC.y);
    y_max=var_max(O.y,A.y,B.y,C.y,AB.y,AC.y,BC.y,ABC.y);
    
    z_min=var_min(O.z,A.z,B.z,C.z,AB.z,AC.z,BC.z,ABC.z);
    z_max=var_max(O.z,A.z,B.z,C.z,AB.z,AC.z,BC.z,ABC.z);
}

void Add_Vect_Block::get_abc(double &a,double &b,double &c,
                             double x,double y,double z)
{
    Eigen::Vector3d point{x,y,z};
    Eigen::Vector3d coeff=vectmat.fullPivLu().solve(point);
    
    a=coeff(0);
    b=coeff(1);
    c=coeff(2);
}

int Add_Vect_Block::index(double x,double y,double z)
{
    if(    x<x_min || x>x_max
        || y<y_min || y>y_max
        || z<z_min || z>z_max) return -1;
    
    double a,b,c;
    get_abc(a,b,c,x-xO,y-yO,z-zO);
    
    if(    a>=0 && a<=1.0
        && b>=0 && b<=1.0
        && c>=0 && c<=1.0) return mat_index;
    
    return -1;
}

//####################
//   Add_Vect_Tri
//####################

Add_Vect_Tri::Add_Vect_Tri(double xO_,double yO_,double zO_,
                           double xA_,double yA_,double zA_,
                           double xB_,double yB_,double zB_,
                           double xC_,double yC_,double zC_,
                           double alp_,int mat_index_)
    :Add_Vect_Block(xO_,yO_,zO_,
                    xA_,yA_,zA_,
                    xB_,yB_,zB_,
                    xC_,yC_,zC_,
                    mat_index_),
     alp(alp_)
{
}

int Add_Vect_Tri::index(double x,double y,double z)
{
    if(    x<x_min || x>x_max
        || y<y_min || y>y_max
        || z<z_min || z>z_max) return -1;
    
    double a,b,c;
    get_abc(a,b,c,x-xO,y-yO,z-zO);
    
    if(b>=0 && b<=1.0 && c>=0 && c<=1.0)
    {
        if(a>=c*alp && a<=1.0-c*(1.0-alp)) return mat_index;
    }
    
    return -1;
}
