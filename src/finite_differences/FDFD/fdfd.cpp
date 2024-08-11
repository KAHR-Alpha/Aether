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

#include <fdfd.h>


extern const Imdouble Im;
extern std::ofstream plog;

FDFD::FDFD(double Dx_,double Dy_,double Dz_)
    :lambda(500e-9),
     inc_theta(0), inc_phi(0),
     kx(0), ky(0),
     inj_type(SRC_NONE),
     F(6*Nxyz),
     D_mat(6*Nxyz,6*Nxyz),
     M_mat(6*Nxyz,6*Nxyz),
     solver_type(SOLVE_LU)
{
    Dx=Dx_;
    Dy=Dy_;
    Dz=Dz_;
    
    for(int i=0;i<6*Nxyz;i++)
    {
        F(i)=0;
    }
}

void FDFD::draw(int vmode,int pos_x,int pos_y,int pos_z,std::string name_mod)
{
    int i,j,k;
    
    std::stringstream im_name;
    im_name<<"render/render_";
    im_name<<"fdfd";
    if(name_mod!="") im_name<<"_"<<name_mod;
    im_name<<".png";
    
    using std::abs;
    using std::exp;
    using std::max;
    
    double max_Ex=1e-80,max_Ey=1e-80,max_Ez=1e-80,max_E=1e-80;
        
    int span1=Nx,span2=Nz;
    
    if(vmode==1) { span1=Ny; span2=Nz; }
    if(vmode==2) { span1=Nx; span2=Ny; }
    
    span1=3*span1+8;
    span2=3*span2+8;
    
    Bitmap im(span1,span2);
        
    if(vmode==0)
    {
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(get_Ex(i,pos_y,k)));
            max_Ey=max(max_Ey,abs(get_Ey(i,pos_y,k)));
            max_Ez=max(max_Ez,abs(get_Ez(i,pos_y,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            im.degra(i,k,1.0-exp(-1.0*abs(get_Ez(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k,1.0-exp(-1.0*abs(get_Ez(i,pos_y,k).real())),0,1.0);
            im.degra_circ(i+2*Nx+8,k,std::arg(get_Ez(i,pos_y,k)),-Pi,Pi);
            
            im.degra(i,k+Nz+4,1.0-exp(-1.0*abs(get_Ey(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k+Nz+4,1.0-exp(-1.0*abs(get_Ey(i,pos_y,k).real())),0,1.0);
            im.degra_circ(i+2*Nx+8,k+Nz+4,std::arg(get_Ey(i,pos_y,k)),-Pi,Pi);
            
            im.degra(i,k+2*Nz+8,1.0-exp(-1.0*abs(get_Ex(i,pos_y,k))),0,1.0);
            im.degra(i+Nx+4,k+2*Nz+8,1.0-exp(-1.0*abs(get_Ex(i,pos_y,k).real())),0,1.0);
            im.degra_circ(i+2*Nx+8,k+2*Nz+8,std::arg(get_Ex(i,pos_y,k)),-Pi,Pi);
        }
        
        Plog::print(max_Ex, "\n");
        Plog::print(max_Ey, "\n");
        Plog::print(max_Ez, "\n");
    }
    if(vmode==1)
    {
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            max_Ex=max(max_Ex,abs(get_Ex(pos_x,j,k)));
            max_Ey=max(max_Ey,abs(get_Ey(pos_x,j,k)));
            max_Ez=max(max_Ez,abs(get_Ez(pos_x,j,k)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            im.degra(j,k,1.0-exp(-1.0*abs(get_Ez(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k,1.0-exp(-1.0*abs(get_Ez(pos_x,j,k))/max_E),0,1.0);
            
            im.degra(j,k+Nz+4,1.0-exp(-1.0*abs(get_Ey(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k+Nz+4,1.0-exp(-1.0*abs(get_Ey(pos_x,j,k))/max_E),0,1.0);
            
            im.degra(j,k+2*Nz+8,1.0-exp(-1.0*abs(get_Ex(pos_x,j,k))),0,1.0);
            im.degra(j+Ny+4,k+2*Nz+8,1.0-exp(-1.0*abs(get_Ex(pos_x,j,k))/max_E),0,1.0);
        }
        
        Plog::print(max_Ex, "\n");
        Plog::print(max_Ey, "\n");
        Plog::print(max_Ez, "\n");
    }
    if(vmode==2)
    {
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            max_Ex=max(max_Ex,abs(get_Ex(i,j,pos_z)));
            max_Ey=max(max_Ey,abs(get_Ey(i,j,pos_z)));
            max_Ez=max(max_Ez,abs(get_Ez(i,j,pos_z)));
        }
        max_E=max(max_Ex,max(max_Ey,max_Ez));
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            im.degra(i,j,1.0-exp(-1.0*abs(get_Ez(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j,1.0-exp(-1.0*abs(get_Ez(i,j,pos_z))/max_E),0,1.0);
            
            im.degra(i,j+Ny+4,1.0-exp(-1.0*abs(get_Ey(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j+Ny+4,1.0-exp(-1.0*abs(get_Ey(i,j,pos_z))/max_E),0,1.0);
            
            im.degra(i,j+2*Ny+8,1.0-exp(-1.0*abs(get_Ex(i,j,pos_z))),0,1.0);
            im.degra(i+Nx+4,j+2*Ny+8,1.0-exp(-1.0*abs(get_Ex(i,j,pos_z))/max_E),0,1.0);
        }
        
        Plog::print(max_Ex, "\n");
        Plog::print(max_Ey, "\n");
        Plog::print(max_Ez, "\n");
    }
    
    im.write(im_name.str());
}

Imdouble FDFD::get_Ex(int i,int j,int k) { return F(index_Ex(i,j,k)); }
Imdouble FDFD::get_Ey(int i,int j,int k) { return F(index_Ey(i,j,k)); }
Imdouble FDFD::get_Ez(int i,int j,int k) { return F(index_Ez(i,j,k)); }

Imdouble FDFD::get_Hx(int i,int j,int k) { return F(index_Hx(i,j,k)); }
Imdouble FDFD::get_Hy(int i,int j,int k) { return F(index_Hy(i,j,k)); }
Imdouble FDFD::get_Hz(int i,int j,int k) { return F(index_Hz(i,j,k)); }

void FDFD::get_injection_cbox(int xm,int xp,int ym,int yp,int zm,int zp,
                             bool x_on,bool y_on,bool z_on,
                             Eigen::SparseVector<Imdouble> &F_src)
{
    int i,j,k;
    
    F_src.resize(6*Nxyz);
    
    if(x_on)
    {
        for(j=ym;j<yp;j++){ for(k=zm;k<zp;k++)
        {
            //xm
            
            F_src.coeffRef(index_Ey(xm,j,k))+=get_Hz(xm-1,j,k)/Dx;  //TF
            F_src.coeffRef(index_Ez(xm,j,k))-=get_Hy(xm-1,j,k)/Dx;  //TF
            F_src.coeffRef(index_Hy(xm-1,j,k))+=get_Ez(xm,j,k)/Dx;  //SF
            F_src.coeffRef(index_Hz(xm-1,j,k))-=get_Ey(xm,j,k)/Dx;  //SF
            
            //xp
            
            F_src.coeffRef(index_Ey(xp,j,k))-=get_Hz(xp-1,j,k)/Dx;  //SF
            F_src.coeffRef(index_Ez(xp,j,k))+=get_Hy(xp-1,j,k)/Dx;  //SF
            F_src.coeffRef(index_Hy(xp-1,j,k))-=get_Ez(xp,j,k)/Dx;  //TF
            F_src.coeffRef(index_Hz(xp-1,j,k))+=get_Ey(xp,j,k)/Dx;  //TF
        }}
    }
    if(z_on)
    {
        for(i=xm;i<xp;i++){ for(j=ym;j<yp;j++)
        {
            //zm
            
            F_src.coeffRef(index_Ex(i,j,zm))+=get_Hy(i,j,zm-1)/Dz;  //TF
            F_src.coeffRef(index_Ey(i,j,zm))-=get_Hx(i,j,zm-1)/Dz;  //TF
            F_src.coeffRef(index_Hx(i,j,zm-1))+=get_Ey(i,j,zm)/Dz;  //SF
            F_src.coeffRef(index_Hy(i,j,zm-1))-=get_Ex(i,j,zm)/Dz;  //SF
            
            //zp
            
            F_src.coeffRef(index_Ex(i,j,zp))-=get_Hy(i,j,zp-1)/Dz;  //SF
            F_src.coeffRef(index_Ey(i,j,zp))+=get_Hx(i,j,zp-1)/Dz;  //SF
            F_src.coeffRef(index_Hx(i,j,zp-1))-=get_Ey(i,j,zp)/Dz;  //TF
            F_src.coeffRef(index_Hy(i,j,zp-1))+=get_Ex(i,j,zp)/Dz;  //TF
        }}
    }
}

void FDFD::interp(double &x,double &y,double &z,
                  double offset_x,double offset_y,double offset_z,
                  int &i1,int &i2,
                  int &j1,int &j2,
                  int &k1,int &k2)
{
    while(x<0) x+=Nx;
    while(y<0) y+=Ny;
    while(z<0) z+=Nz;
    
    while(x>=Nx) x-=Nx;
    while(y>=Ny) y-=Ny;
    while(z>=Nz) z-=Ny;
    
    i1=std::floor(x-offset_x);
    i2=i1+1; x-=i1+offset_x;
    if(i1<0) i1=Nx-1;
    if(i2>=Nx) i2=0;
    
    j1=std::floor(y-offset_y);
    j2=j1+1; y-=j1+offset_y;
    if(j1<0) j1=Ny-1;
    if(j2>=Ny) j2=0;
    
    k1=std::floor(z-offset_z);
    k2=k1+1; z-=k1+offset_z;
    if(k1<0) k1=Nz-1;
    if(k2>=Nz) k2=0;
    
    if(Nx==1){ i1=i2=0; x=0; }
    if(Ny==1){ j1=j2=0; y=0; }
    if(Nz==1){ k1=k2=0; z=0; }
}

Imdouble FDFD::interp_Ex(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0.5,0,0,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Ex(i1,j1,k1)+z*get_Ex(i1,j1,k2))
               +y*(uz*get_Ex(i1,j2,k1)+z*get_Ex(i1,j2,k2)))
           +x*(uy*(uz*get_Ex(i2,j1,k1)+z*get_Ex(i2,j1,k2))
               +y*(uz*get_Ex(i2,j2,k1)+z*get_Ex(i2,j2,k2)));
}

Imdouble FDFD::interp_Ey(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0,0.5,0,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Ey(i1,j1,k1)+z*get_Ey(i1,j1,k2))
               +y*(uz*get_Ey(i1,j2,k1)+z*get_Ey(i1,j2,k2)))
           +x*(uy*(uz*get_Ey(i2,j1,k1)+z*get_Ey(i2,j1,k2))
               +y*(uz*get_Ey(i2,j2,k1)+z*get_Ey(i2,j2,k2)));
}

Imdouble FDFD::interp_Ez(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0,0,0.5,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Ez(i1,j1,k1)+z*get_Ez(i1,j1,k2))
               +y*(uz*get_Ez(i1,j2,k1)+z*get_Ez(i1,j2,k2)))
           +x*(uy*(uz*get_Ez(i2,j1,k1)+z*get_Ez(i2,j1,k2))
               +y*(uz*get_Ez(i2,j2,k1)+z*get_Ez(i2,j2,k2)));
}

Imdouble FDFD::interp_Hx(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0,0.5,0.5,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Hx(i1,j1,k1)+z*get_Hx(i1,j1,k2))
               +y*(uz*get_Hx(i1,j2,k1)+z*get_Hx(i1,j2,k2)))
           +x*(uy*(uz*get_Hx(i2,j1,k1)+z*get_Hx(i2,j1,k2))
               +y*(uz*get_Hx(i2,j2,k1)+z*get_Hx(i2,j2,k2)));
}

Imdouble FDFD::interp_Hy(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0.5,0,0.5,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Hy(i1,j1,k1)+z*get_Hy(i1,j1,k2))
               +y*(uz*get_Hy(i1,j2,k1)+z*get_Hy(i1,j2,k2)))
           +x*(uy*(uz*get_Hy(i2,j1,k1)+z*get_Hy(i2,j1,k2))
               +y*(uz*get_Hy(i2,j2,k1)+z*get_Hy(i2,j2,k2)));
}

Imdouble FDFD::interp_Hz(double x,double y,double z)
{
    int i1,i2,j1,j2,k1,k2;
    
    interp(x,y,z,0.5,0.5,0,i1,i2,j1,j2,k1,k2);
    
    double ux=1.0-x,uy=1.0-y,uz=1.0-z;
    
    return ux*(uy*(uz*get_Hz(i1,j1,k1)+z*get_Hz(i1,j1,k2))
               +y*(uz*get_Hz(i1,j2,k1)+z*get_Hz(i1,j2,k2)))
           +x*(uy*(uz*get_Hz(i2,j1,k1)+z*get_Hz(i2,j1,k2))
               +y*(uz*get_Hz(i2,j2,k1)+z*get_Hz(i2,j2,k2)));
}

void FDFD::set_injection_cbox(int xm,int xp,int ym,int yp,int zm,int zp,
                             Eigen::SparseVector<Imdouble> &F_src)
{
    inj_F_src.resize(6*Nxyz);
    
    inj_type=SRC_CBOX;
    inj_F_src=F_src;
}

void FDFD::set_injection_plane_z(int k)
{
    inj_type=SRC_PLANE_Z;
    inj_zp=k;
}

int FDFD::slc_Ex(int i,int j) { return index(i,j,0,0); }
int FDFD::slc_Ey(int i,int j) { return index(i,j,0,1); }
int FDFD::slc_Ez(int i,int j) { return index(i,j,0,2); }
int FDFD::slc_Hx(int i,int j) { return index(i,j,0,3); }
int FDFD::slc_Hy(int i,int j) { return index(i,j,0,4); }
int FDFD::slc_Hz(int i,int j) { return index(i,j,0,5); }

void FDFD::update_Nxyz()
{
    xs_s=pml_xm+pad_xm;
    ys_s=pml_ym+pad_ym;
    zs_s=pml_zm+pad_zm;
    
    xs_e=xs_s+Nx_s;
    ys_e=ys_s+Ny_s;
    zs_e=zs_s+Nz_s;
    
    Nx=xs_e+pad_xp+pml_xp;
    Ny=ys_e+pad_yp+pml_yp;
    Nz=zs_e+pad_zp+pml_zp;
    
    Nxy=Nx*Ny;
    Nxyz=Nx*Ny*Nz;
    
    chk_var(Nx_s);
    chk_var(Ny_s);
    chk_var(Nz_s);
    chk_var(xs_s);
    chk_var(ys_s);
    chk_var(zs_s);
    chk_var(xs_e);
    chk_var(ys_e);
    chk_var(zs_e);
    chk_var(Nx);
    chk_var(Ny);
    chk_var(Nz);
    chk_var(Nxy);
    chk_var(Nxyz);
    
    matsgrid.init(Nx,Ny,Nz,0);
    
    F.resize(6*Nxyz);
    
    D_mat.resize(6*Nxyz,6*Nxyz);
    M_mat.resize(6*Nxyz,6*Nxyz);
    
    for(int i=0;i<6*Nxyz;i++) F(i)=0;
}
