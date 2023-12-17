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

#include <fdtd_core.h>


extern const Imdouble Im;
extern std::ofstream plog;

void FDTD::adv_dt_Dx(int i1,int i2)
{
    int i,j,k;
    int j1,k1;
    int j2,k2;
    double kappa_y,inv_kappa_y;
    double kappa_z,inv_kappa_z;
    
    for(j=0;j<Ny;j++)
    {
        j2=j;
        if(j==0) j1=Ny-1;
        else j1=j-1;
                
        if(pml_ym || pml_yp) kappa_y=kappa_y_E[j];
        else kappa_y=1.0;
        
        inv_kappa_y=1.0/(kappa_y*Dy);
        
        for(k=0;k<Nz;k++)
        {
            k2=k;
            if(k==0) k1=Nz-1;
            else k1=k-1;
            
            if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
            else kappa_z=1.0;
            
            inv_kappa_z=1.0/(kappa_z*Dz);
            
            for(i=i1;i<i2;i++) //0 - Nx
            {
                dt_Dx(i,j,k)=inv_kappa_y*(Hz(i,j2,k)-Hz(i,j1,k))
                             -inv_kappa_z*(Hy(i,j,k2)-Hy(i,j,k1));
            }
        }
    }
}

void FDTD::adv_dt_Dy(int j1,int j2)
{
    int i,j,k;
    int i1,i2,k1,k2;
    double kappa_x,inv_kappa_x;
    double kappa_z,inv_kappa_z;
    
    for(i=0;i<Nx;i++)
    {
        i2=i;
        if(mode!=M_OBLIQUE_PHASE)
        {
            if(i==0) i1=Nx-1;
            else i1=i-1;
        }
        else
        {
            if(i==0) i1=Nx;
            else i1=i-1;
        }
                
        if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
        else kappa_x=1.0;
        
        inv_kappa_x=1.0/(Dx*kappa_x);
        
        for(k=0;k<Nz;k++)
        {
            k2=k;
            if(k==0) k1=Nz-1;
            else k1=k-1;
                        
            if(pml_zm || pml_zp) kappa_z=kappa_z_E[k];
            else kappa_z=1.0;
            
            inv_kappa_z=1.0/(Dz*kappa_z);
            
            for(j=j1;j<j2;j++) //0 - Ny
            {
                dt_Dy(i,j,k)=inv_kappa_z*(Hx(i,j,k2)-Hx(i,j,k1))
                             -inv_kappa_x*(Hz(i2,j,k)-Hz(i1,j,k));
            }
        }
    }
    
}

void FDTD::adv_dt_Dz(int k1,int k2)
{
    int i,j,k;
    int i1,i2;
    int j1,j2;
    double kappa_x,inv_kappa_x;
    double kappa_y,inv_kappa_y;
    
    for(i=0;i<Nx;i++)
    {
        i2=i;
        if(mode!=M_OBLIQUE_PHASE)
        {
            if(i==0) i1=Nx-1;
            else i1=i-1;
        }
        else
        {
            if(i==0) i1=Nx;
            else i1=i-1;
        }
        
        if(pml_xm || pml_xp) kappa_x=kappa_x_E[i];
        else kappa_x=1.0;
        
        inv_kappa_x=1.0/(Dx*kappa_x);
        
        for(j=0;j<Ny;j++)
        {
            j2=j;
            if(j==0) j1=Ny-1;
            else j1=j-1;
            
            if(pml_ym || pml_yp) kappa_y=kappa_y_E[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/(Dy*kappa_y);
            
            for(k=k1;k<k2;k++) //0 - Nz
            {
                dt_Dz(i,j,k)=inv_kappa_x*(Hy(i2,j,k)-Hy(i1,j,k))
                             -inv_kappa_y*(Hx(i,j2,k)-Hx(i,j1,k));
            }
        }
    }
    
}

void FDTD::adv_dt_Bx(int i1,int i2)
{
    int i,j,k;
    int j1,j2;
    int k1,k2;
    double kappa_y,inv_kappa_y;
    double kappa_z,inv_kappa_z;
    
    for(j=0;j<Ny;j++)
    {
        if(j==Ny-1) j2=0;
        else j2=j+1;
        j1=j;
                
        if(pml_ym || pml_yp) kappa_y=kappa_y_H[j];
        else kappa_y=1.0;
        
        inv_kappa_y=1.0/(Dy*kappa_y);
        
        for(k=0;k<Nz;k++)
        {
            if(k==Nz-1) k2=0;
            else k2=k+1;
            k1=k;
            
            if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
            else kappa_z=1.0;
            
            inv_kappa_z=1.0/(Dz*kappa_z);
            
            for(i=i1;i<i2;i++) //0 - Nx
            {
                dt_Bx(i,j,k)=inv_kappa_z*(Ey(i,j,k2)-Ey(i,j,k1))
                             -inv_kappa_y*(Ez(i,j2,k)-Ez(i,j1,k));
            }
        }
    }
    
}

void FDTD::adv_dt_By(int j1,int j2)
{
    int i,j,k;
    int i1,i2;
    int k1,k2;
    double kappa_x,inv_kappa_x;
    double kappa_z,inv_kappa_z;
        
    for(i=0;i<Nx;i++)
    {
        if(mode!=M_OBLIQUE_PHASE)
        {
            if(i==Nx-1) i2=0;
            else i2=i+1;
        }
        else
        {
            if(i==Nx-1) i2=Nx;
            else i2=i+1;
        }
        i1=i;
                
        if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
        else kappa_x=1.0;
        
        inv_kappa_x=1.0/(Dx*kappa_x);
        
        for(k=0;k<Nz;k++)
        {
            if(k==Nz-1) k2=0;
            else k2=k+1;
            k1=k;
            
            if(pml_zm || pml_zp) kappa_z=kappa_z_H[k];
            else kappa_z=1.0;
            
            inv_kappa_z=1.0/(Dz*kappa_z);
            
            for(j=j1;j<j2;j++)
            {
                dt_By(i,j,k)=inv_kappa_x*(Ez(i2,j,k)-Ez(i1,j,k))
                             -inv_kappa_z*(Ex(i,j,k2)-Ex(i,j,k1));
            }
        }
    }
}

void FDTD::adv_dt_Bz(int k1,int k2)
{
    int i,j,k;
    int i1,i2;
    int j1,j2;
    double kappa_x,inv_kappa_x;
    double kappa_y,inv_kappa_y;
    
    for(i=0;i<Nx;i++)
    {
        if(mode!=M_OBLIQUE_PHASE)
        {
            if(i==Nx-1) i2=0;
            else i2=i+1;
        }
        else
        {
            if(i==Nx-1) i2=Nx;
            else i2=i+1;
        }
        i1=i;
        
        if(pml_xm || pml_xp) kappa_x=kappa_x_H[i];
        else kappa_x=1.0;
        
        inv_kappa_x=1.0/(Dx*kappa_x);
        
        for(j=0;j<Ny;j++)
        {
            if(j==Ny-1) j2=0;
            else j2=j+1;
            j1=j;
            
            if(pml_ym || pml_yp) kappa_y=kappa_y_H[j];
            else kappa_y=1.0;
            
            inv_kappa_y=1.0/(Dz*kappa_y);
            
            for(k=k1;k<k2;k++)
            {
                dt_Bz(i,j,k)=inv_kappa_y*(Ex(i,j2,k)-Ex(i,j1,k))
                             -inv_kappa_x*(Ey(i2,j,k)-Ey(i1,j,k));
            }
        }
    }
}

//####################
//####################
//####################

void FDTD::advMats_ext(int i1,int i2)
{
    int i,j,k;
    int M;
    
    for(j=0;j<Ny;j++)
    {
        for(k=0;k<Nz;k++)
        {
            for(i=i1;i<i2;i++) //0 - Nx
            {
                M=matsgrid(i,j,k);
                
                mats[M].apply_D2E(i,j,k,Ex,0,dt_Dx,dt_Dy,dt_Dz);
                mats[M].apply_D2E(i,j,k,Ey,1,dt_Dx,dt_Dy,dt_Dz);
                mats[M].apply_D2E(i,j,k,Ez,2,dt_Dx,dt_Dy,dt_Dz);
            }
        }
    }
}
