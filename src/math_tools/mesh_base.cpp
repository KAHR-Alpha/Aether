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

#include <mesh_base.h>
#include <mesh_tools.h>

extern const double Pi;

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
    std::cout<<"Location: ";loc.show();
    std::cout<<"Norm: ";norm.show();
    std::cout<<std::endl;
}

void Vertex::translate(Vector3 const &V) { loc+=V; }
void Vertex::translate(double const &x,double const &y,double const &z) { loc+=Vector3(x,y,z); }

//####################
//      Face
//####################

Face::Face()
    :V1(0), V2(0), V3(0),
     norm(0,0,1), ngb(3)
{
    for(int i=0;i<3;i++) ngb[i]=-1;
}

Face::Face(int a,int b,int c)
    :V1(a), V2(b), V3(c),
     norm(0,0,1), ngb(3)
{
    for(int i=0;i<3;i++) ngb[i]=-1;
}

Face::Face(Face const &T)
    :V1(T.V1), V2(T.V2), V3(T.V3),
     norm(T.norm), ngb(T.ngb)
{
}

void Face::comp_norm(std::vector<Vertex> const &v_arr)
{
    norm.crossprod(v_arr[V2].loc-v_arr[V1].loc,v_arr[V3].loc-v_arr[V1].loc);
    norm.normalize();
}

void Face::comp_norm(Grid1<Vertex> const &v_arr)
{
    norm.crossprod(v_arr[V2].loc-v_arr[V1].loc,v_arr[V3].loc-v_arr[V1].loc);
    norm.normalize();
}

bool Face::is_connected(Face const &F)
{
    if(V1==F.V1 || V1==F.V2 || V1==F.V3 ||
       V2==F.V1 || V2==F.V2 || V2==F.V3 ||
       V3==F.V1 || V3==F.V2 || V3==F.V3 ) return 1;
    
    return 0;
}

bool Face::degeneracy_chk(int ID_msg)
{
    if(V1==V2 || V1==V3 || V2==V3)
    {
        std::cout<<"Degeneracy detected "<<0<<" "<<ID_msg<<std::endl;
        std::system("pause");
        return 1;
    }
    if(ngb[0]==ngb[1] || ngb[0]==ngb[2] || ngb[1]==ngb[2])
    {
        std::cout<<"Degeneracy detected "<<1<<" "<<ID_msg<<std::endl;
        std::system("pause");
        return 1;
    }
    return 0;
}

void Face::get_edge(int edge,int &Va,int &Vb,int &Vc)
{
    if(edge==0) { Va=V1; Vb=V2; Vc=V3; }
    else if(edge==1) { Va=V2; Vb=V3; Vc=V1; }
    else if(edge==2) { Va=V3; Vb=V1; Vc=V2; }
}

void Face::operator = (Face const &T)
{
    set_Vindex(T.V1,T.V2,T.V3);
        
    norm=T.norm;
    set_ngb(T.ngb[0],T.ngb[1],T.ngb[2]);
}

void Face::set_norm(double x,double y,double z)
{
    norm.x=x;
    norm.y=y;
    norm.z=z;
}

void Face::set_norm(Vector3 const &V)
{
    norm=V;
}

void Face::set_ngb(int N1,int N2,int N3)
{
    ngb[0]=N1;
    ngb[1]=N2;
    ngb[2]=N3;
}

void Face::set_Vindex(int const &V1_,int const &V2_,int const &V3_)
{
    V1=V1_;
    V2=V2_;
    V3=V3_;
}

void Face::show()
{
    std::cout<<"Vertices indices: "<<V1<<" "<<V2<<" "<<V3<<std::endl;
    std::cout<<"Neighbors indices: "; for(unsigned int i=0;i<ngb.size();i++) std::cout<<ngb[i]<<" ";
    std::cout<<std::endl;
}

double Face::smallest_height(std::vector<Vertex> const &v_arr)
{
    Vertex const &A=v_arr[V1];
    Vertex const &B=v_arr[V2];
    Vertex const &C=v_arr[V3];
    
    Vector3 AB=B.loc-A.loc;
    Vector3 AC=C.loc-A.loc;
    Vector3 BC=C.loc-B.loc;
    
    double ABn2=AB.norm_sqr();
    double ACn2=AC.norm_sqr();
    
    double CpAB=scalar_prod(AC,AB);
    double BpAC=scalar_prod(AB,AC);
    double ApBC=scalar_prod(-AB,BC);
    
    double r=std::min(ACn2-CpAB*CpAB/ABn2,
             std::min(ABn2-BpAC*BpAC/ACn2,ABn2-ApBC*ApBC/BC.norm_sqr()));
    
    return std::sqrt(std::abs(r));
}

bool Face::update_ngb(Face const &F,int F_ID)
{
    int edge=vID_compare_edge(V1,V2,V3,F.V1,F.V2,F.V3);
    
    if(edge<0) return 0;
    else
    {
        ngb[edge]=F_ID;
        return 1;
    }
}

//####################
//      Mesh
//####################

class vertex_compare
{
    public:
        std::vector<Vertex> &v_arr;
        
        vertex_compare(std::vector<Vertex> &v_arr_) :v_arr(v_arr_) {}
    
        bool operator () (int &i1,int &i2)
        {
            Vertex &V1=v_arr[i1];
            Vertex &V2=v_arr[i2];
            
            if(V1.loc.x<=V2.loc.x)
            {
                if(V1.loc.x<V2.loc.x) return true;
                else if(V1.loc.y<V2.loc.y) return true;
            }
            
            return false;
        }
};

extern std::ofstream plog;

Mesh::Mesh()
{
    set_threads_number(1);
}

void Mesh::compute_convex_hull()
{
    // Monotone Chaine Algorithm
    
    int i,NVtx=v_arr.size();
    
    std::vector<int> v_sort(NVtx),L,U;
        
    vertex_compare comp(v_arr);
    
    for(i=0;i<NVtx;i++) v_sort[i]=i;
    std::sort(v_sort.begin(),v_sort.end(),comp);
    
    for(i=0;i<NVtx;i++)
    {
        while(L.size()>=2)
        {
            int NL=L.size();
        
            Vector3 &V1=v_arr[L[NL-2]].loc;
            Vector3 &V2=v_arr[L[NL-1]].loc;
            Vector3 &V3=v_arr[v_sort[i]].loc;
            
            double ang=(V2.x-V1.x)*(V3.y-V2.y)-(V2.y-V1.y)*(V3.x-V2.x);
            
            if(ang<0) L.pop_back();
            else break;
        }
        
        L.push_back(v_sort[i]);
    }
    
    for(i=NVtx-1;i>=0;i--)
    {
        while(U.size()>=2)
        {
            int NU=U.size();
        
            Vector3 &V1=v_arr[U[NU-2]].loc;
            Vector3 &V2=v_arr[U[NU-1]].loc;
            Vector3 &V3=v_arr[v_sort[i]].loc;
            
            double ang=(V2.x-V1.x)*(V3.y-V2.y)-(V2.y-V1.y)*(V3.x-V2.x);
            
            if(ang<0) U.pop_back();
            else break;
        }
        
        U.push_back(v_sort[i]);
    }
    
    int NL=L.size();
    int NU=U.size();
    
    hull.clear();
    hull.reserve(NL+NU-2);
    
    for(i=0;i<NL;i++) hull.push_back(L[i]);
    for(i=1;i<NU;i++) hull.push_back(U[i]);
//    for(i=NU-2;i>0;i--) hull.push_back(U[i]);
}

int face_compare_edge(Face const &F_ref,Face const &F_comp)
{
    return vID_compare_edge(F_ref.V1,F_ref.V2,F_ref.V3,
                            F_comp.V1,F_comp.V2,F_comp.V3);
}

//void Mesh::compute_neighbors()
//{
//    int i,j;
//    
//    int Nf=f_arr.size();
//    
//    std::vector<int> F_tot(Nf,0);
//    
//    int tE=0;
//    
//    for(i=0;i<Nf;i++)
//    {
//        if(F_tot[i]<3)
//        {
//            for(j=i+1;j<Nf;j++)
//            {
//                tE=face_compare_edge(f_arr[i],f_arr[j]);
//                
//                if(tE!=-1)
//                {
//                    F_tot[i]+=1;
//                    F_tot[j]+=1;
//                    
//                    f_arr[i].ngb[tE]=j;
//                    
//                    tE=face_compare_edge(f_arr[j],f_arr[i]);
//                    f_arr[j].ngb[tE]=i;
//                }
//                
//                if(F_tot[i]==3) break;
//            }
//        }
//    }
//}

void Mesh::compute_neighbors()
{
    unsigned int i,j,k;
    
    bool accounted_face=false;
    std::vector<std::vector<unsigned int>> v_map(v_arr.size());
    
    for(i=0;i<f_arr.size();i++)
    {
        int V1=f_arr[i].V1,
            V2=f_arr[i].V2,
            V3=f_arr[i].V3;
        
        accounted_face=false;
        for(j=0;j<v_map[V1].size();j++)
            if(v_map[V1][j]==i) accounted_face=true;
        
        if(!accounted_face) v_map[V1].push_back(i);
        
        accounted_face=false;
        for(j=0;j<v_map[V2].size();j++)
            if(v_map[V2][j]==i) accounted_face=true;
        
        if(!accounted_face) v_map[V2].push_back(i);
        
        accounted_face=false;
        for(j=0;j<v_map[V3].size();j++)
            if(v_map[V3][j]==i) accounted_face=true;
        
        if(!accounted_face) v_map[V3].push_back(i);
    }
    
    int tE1=0,tE2=0;
    
    for(i=0;i<v_arr.size();i++)
    {
        for(j=0;j<v_map[i].size();j++) for(k=0;k<v_map[i].size();k++)
        {
            if(j!=k)
            {
                Face &f1=f_arr[v_map[i][j]];
                Face &f2=f_arr[v_map[i][k]];
                
                tE1=face_compare_edge(f1,f2);
                
                if(tE1!=-1)
                {
                    tE2=face_compare_edge(f2,f1);
                    
                    f1.ngb[tE1]=v_map[i][k];
                    f2.ngb[tE2]=v_map[i][j];
                }
            }
        }
    }
}

void Mesh::load_obj(std::string const &fname)
{
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    double x,y,z;
    
    std::string k;
    int VID[4];
    std::vector<std::string> split;
    
    while(std::getline(file,k))
    {
        split_string(split,k);
        
        if(split[0]=="v")
        {
            x=std::stod(split[1]);
            y=std::stod(split[2]);
            z=std::stod(split[3]);
            
            Vertex v(x,y,z);
            
            v_arr.push_back(v);
        }
        else if(split[0]=="f")
        {
            for(unsigned int i=0;i<split.size()-1;i++)
                VID[i]=std::stoi(split[i+1].substr(0,split[i+1].find("/")))-1;
            
            if(split.size()==4)
            {
                Face f(VID[0],VID[1],VID[2]);
                
                f_arr.push_back(f);
            }
            else if(split.size()==5)
            {
                Face f1(VID[0],VID[1],VID[2]),f2(VID[0],VID[2],VID[3]);
                
                f_arr.push_back(f1);
                f_arr.push_back(f2);
            }
        }
    }
}

void Mesh::set_mesh(std::vector<Vertex> const &v_arr_,std::vector<Face> const &f_arr_)
{
    v_arr.resize(v_arr_.size());
    f_arr.resize(f_arr_.size());
    
    v_arr=v_arr_;
    f_arr=f_arr_;
    
    octree.generate_tree(v_arr,f_arr);
}

void Mesh::set_threads_number(int Nthr_)
{
    Nthr=Nthr_;
    
    flist.resize(Nthr);
}

void Mesh::vertex_interpolate_norm()
{
    std::vector<int> Nf_contrib(f_arr.size(),0);
    
    for(unsigned int i=0;i<v_arr.size();i++) v_arr[i].norm=0;
    
    for(unsigned int i=0;i<f_arr.size();i++)
    {
        v_arr[f_arr[i].V1].norm+=f_arr[i].norm;
        Nf_contrib[f_arr[i].V1]+=1;
        
        v_arr[f_arr[i].V2].norm+=f_arr[i].norm;
        Nf_contrib[f_arr[i].V2]+=1;
        
        v_arr[f_arr[i].V3].norm+=f_arr[i].norm;
        Nf_contrib[f_arr[i].V3]+=1;
    }
    
    for(unsigned int i=0;i<v_arr.size();i++)
    {
        v_arr[i].norm/=Nf_contrib[i];
        v_arr[i].norm.normalize();
    }
}

//####################
//      QFace
//####################

QFace::QFace()
    :V1(0), V2(0), V3(0), V4(0)
{
}

QFace::QFace(int a,int b,int c,int d)
    :V1(a), V2(b), V3(c), V4(d)
{
}

QFace::QFace(QFace const &T)
    :V1(T.V1), V2(T.V2), V3(T.V3), V4(T.V4)
{
}

void QFace::operator = (QFace const &T)
{
    set_Vindex(T.V1,T.V2,T.V3,T.V4);
}

void QFace::set_Vindex(int const &V1i,int const &V2i,int const &V3i,int const &V4i)
{
    V1=V1i;
    V2=V2i;
    V3=V3i;
    V4=V4i;
}
