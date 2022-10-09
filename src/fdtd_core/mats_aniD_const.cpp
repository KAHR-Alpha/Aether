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

#include <fdtd_material.h>

void FDTD_Material::set_ani_DC(double eps_x,double eps_y,double eps_z)
{
    m_type=MAT_ANI_DIAG_CONST;
    
    comp_simp=1;
    comp_ante=comp_self=comp_post=0;
    comp_D=1;
    
    set_mem_depth(0,0,0);
    
    ADC_ex=eps_x;
    ADC_ey=eps_y;
    ADC_ez=eps_z;
    
    ani_DC_recalc();
}

void FDTD_Material::ani_DC_D2E(int i,int j,int k,Grid3<double> &E,int dir,
                               Grid3<double> const &Dx,
                               Grid3<double> const &Dy,
                               Grid3<double> const &Dz)
{
    if(dir==0)
    {
        E(i,j,k)+=C2x*Dx(i,j,k);
    }
    else if(dir==1)
    {
        E(i,j,k)+=C2y*Dy(i,j,k);
    }
    else if(dir==2)
    {
        E(i,j,k)+=C2z*Dz(i,j,k);
    }
}

void FDTD_Material::ani_DC_recalc()
{
    C1=1;
    C2=Dt/(e0);
    C2x=Dt/(e0*ADC_ex);
    C2y=Dt/(e0*ADC_ey);
    C2z=Dt/(e0*ADC_ez);
    C3=0;
    C4=Dt/(e0);
}

/*void FDTD_Material::ani_DC_load_script(ScriptHandler &spt)
{
    using std::stringstream;
    
    double eps_x=1,eps_y=1,eps_z=1;
    
    if(spt.has("eps_x"))
    {
        stringstream strm; strm.str(spt.get_options());
        strm>>eps_x;
    }
    if(spt.has("eps_y"))
    {
        stringstream strm; strm.str(spt.get_options());
        strm>>eps_y;
    }
    if(spt.has("eps_z"))
    {
        stringstream strm; strm.str(spt.get_options());
        strm>>eps_z;
    }
    
    set_ani_DC(eps_x,eps_y,eps_z);
}*/
