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

#include <selene_mesh.h>
#include <mesh_tools.h>

namespace Sel
{

//####################
//     Vertex
//####################

Vertex::Vertex()
    :loc(0,0,0), norm(0,0,1)
{
}

Vertex::Vertex(Vector3 const &V)
    :loc(V), norm(0,0,1)
{
}

Vertex::Vertex(double x,double y,double z)
    :loc(x,y,z), norm(0,0,1)
{
}

Vertex::Vertex(Vertex const& V)
    :loc(V.loc), norm(V.norm)
{
}

void Vertex::operator = (double a)
{
    loc(a,a,a);
    norm(0,0,1);
}

void Vertex::operator = (::Vertex const &V)
{
    loc=V.loc;
    norm=V.norm;
}

void Vertex::operator = (Vertex const &V)
{
    loc=V.loc;
    norm=V.norm;
}

void Vertex::set_loc(Vector3 const &V) { loc=V; }
void Vertex::set_loc(double const &x,double const &y,double const &z) { loc(x,y,z); }
void Vertex::set_norm(Vector3 const &V) { norm=V; }
void Vertex::set_norm(double const &x,double const &y,double const &z) { norm(x,y,z); }

void Vertex::show()
{
    Plog::print("Location: ");loc.show();
    Plog::print("Norm: ");norm.show();
    Plog::print("\n");
}

void Vertex::translate(Vector3 const &V) { loc+=V; }
void Vertex::translate(double const &x,double const &y,double const &z) { loc+=Vector3(x,y,z); }

//####################
//      SelFace
//####################

SelFace::SelFace()
    :V1(0), V2(0), V3(0),
     up_mat(0), down_mat(0),
     up_irf(0), down_irf(0),
     norm(0,0,1), ngb(3,-1),
     tangent_up(TANGENT_UNSET),
     tangent_down(TANGENT_UNSET)
{
}

SelFace::SelFace(int a,int b,int c)
    :V1(a), V2(b), V3(c),
     up_mat(0), down_mat(0),
     up_irf(0), down_irf(0),
     norm(0,0,1), ngb(3,-1),
     tangent_up(TANGENT_UNSET),
     tangent_down(TANGENT_UNSET)
{
}

SelFace::SelFace(SelFace const &T)
    :V1(T.V1), V2(T.V2), V3(T.V3),
     up_mat(T.up_mat), down_mat(T.down_mat),
     up_irf(T.up_irf), down_irf(T.down_irf),
     norm(T.norm), ngb(T.ngb),
     tangent_up(T.tangent_up), tangent_down(T.tangent_down),
     fixed_tangent_up(T.fixed_tangent_up), fixed_tangent_down(T.fixed_tangent_down)
{
}

void SelFace::comp_norm(std::vector<Sel::Vertex> const &v_arr)
{
    norm.crossprod(v_arr[V2].loc-v_arr[V1].loc,v_arr[V3].loc-v_arr[V1].loc);
    norm.normalize();
}

bool SelFace::is_connected(SelFace const &F)
{
    if(V1==F.V1 || V1==F.V2 || V1==F.V3 ||
       V2==F.V1 || V2==F.V2 || V2==F.V3 ||
       V3==F.V1 || V3==F.V2 || V3==F.V3 ) return 1;
    
    return 0;
}

bool SelFace::degeneracy_chk(int ID_msg)
{
    if(V1==V2 || V1==V3 || V2==V3)
    {
        Plog::print("Degeneracy detected ", 0, " ", ID_msg, "\n");
        Plog::print("Press enter to continue\n");
        std::cin.get();
        return 1;
    }
    if(ngb[0]==ngb[1] || ngb[0]==ngb[2] || ngb[1]==ngb[2])
    {
        Plog::print("Degeneracy detected ", 1, " ", ID_msg, "\n");
        Plog::print("Press enter to continue\n");
        std::cin.get();
        return 1;
    }
    return 0;
}

Vector3 SelFace::get_cmass(std::vector<Sel::Vertex> const &V)
{
    Vector3 O;
    
    O.x=(V[V1].loc.x+V[V2].loc.x+V[V3].loc.x)/3.0;
    O.y=(V[V1].loc.y+V[V2].loc.y+V[V3].loc.y)/3.0;
    O.z=(V[V1].loc.z+V[V2].loc.z+V[V3].loc.z)/3.0;
    
    return O;
}

void SelFace::get_edge(int edge,int &Va,int &Vb,int &Vc)
{
         if(edge==0) { Va=V1; Vb=V2; Vc=V3; }
    else if(edge==1) { Va=V2; Vb=V3; Vc=V1; }
    else if(edge==2) { Va=V3; Vb=V1; Vc=V2; }
}

void SelFace::operator = (Face const &T)
{
    set_Vindex(T.V1,T.V2,T.V3);
    
    norm=T.norm;
    set_ngb(T.ngb[0],T.ngb[1],T.ngb[2]);
    
    tangent_up=TANGENT_UNSET;
    tangent_down=TANGENT_UNSET;
}

void SelFace::operator = (SelFace const &T)
{
    set_Vindex(T.V1,T.V2,T.V3);
    
    up_mat=T.up_mat;
    down_mat=T.down_mat;
    
    up_irf=T.up_irf;
    down_irf=T.down_irf;
    
    norm=T.norm;
    set_ngb(T.ngb[0],T.ngb[1],T.ngb[2]);
    
    tangent_up=T.tangent_up;
    fixed_tangent_up=T.fixed_tangent_up;
    
    tangent_down=T.tangent_down;
    fixed_tangent_down=T.fixed_tangent_down;
}

void SelFace::set_norm(double x,double y,double z)
{
    norm.x=x;
    norm.y=y;
    norm.z=z;
}

void SelFace::set_norm(Vector3 const &V)
{
    norm=V;
}

void SelFace::set_ngb(int N1,int N2,int N3)
{
    ngb[0]=N1;
    ngb[1]=N2;
    ngb[2]=N3;
}

void SelFace::set_Vindex(int const &V1i,int const &V2i,int const &V3i)
{
    V1=V1i;
    V2=V2i;
    V3=V3i;
}

void SelFace::set_vertex_index(int const &V1_,int const &V2_,int const &V3_)
{
    V1=V1_;
    V2=V2_;
    V3=V3_;
}

void SelFace::show()
{
    Plog::print("Vertices indices: ", V1, " ", V2, " ", V3, "\n");
    Plog::print("Neighbors indices: "); for(int i=0;i<3;i++) Plog::print(ngb[i], " ");
    Plog::print("\n");
}

bool SelFace::update_ngb(SelFace const &F,int F_ID)
{
    int edge=vID_compare_edge(V1,V2,V3,F.V1,F.V2,F.V3);
    if(edge<0) return 0;
    else
    {
        ngb[edge]=F_ID;
        return 1;
    }
}

}
