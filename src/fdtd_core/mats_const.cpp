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

#include <material.h>

void FDTD_Material::set_const(double eps)
{
    m_type=MAT_CONST;
    
    Np=0;
    Np_r=0;
    Np_c=0;
    
    set_mem_depth(0,0,0);
    
    ei=eps;
    sig=0;
    
    const_recalc();
    
    comp_simp=1;
    comp_ante=comp_self=comp_post=0;
    comp_D=0;
}

void FDTD_Material::const_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                              Grid3<double> const &Dx,
                              Grid3<double> const &Dy,
                              Grid3<double> const &Dz)
{
    double D_tmp=Dx(i,j,k);
    if(dir==1) D_tmp=Dy(i,j,k);
    else if(dir==2) D_tmp=Dz(i,j,k);
    
    E(i,j,k)+=C2*D_tmp;
}

/*void FDTD_Material::const_load_script(ScriptHandler &spt)
{
    double eps;
    
    std::stringstream strm;
    
    if(spt.has("index"))
    {
        strm<<spt.get_options();
        strm>>eps;
        eps=eps*eps;
    }
    else if(spt.has("epsilon"))
    {
        strm<<spt.get_options();
        strm>>eps;
    }
    
    set_const(eps);
}*/

void FDTD_Material::const_recalc()
{
    C1=1;
    C2=Dt/(e0*ei);
    C2x=Dt/(Dx*e0*ei);
    C2y=Dt/(Dy*e0*ei);
    C2z=Dt/(Dz*e0*ei);
    C3=0;
    C4=Dt/(e0*ei);
}
