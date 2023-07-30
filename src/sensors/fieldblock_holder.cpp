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

#include <fieldblock_holder.h>

#include <fstream>

std::string filename_filter(std::string fname)
{
    std::string r;
    
    int pos1=0;
    int pos2=fname.size();
    
    if(fname.rfind("\\")!=std::string::npos)
    {
        pos1=fname.rfind("\\");
    }
    else pos1=-1;
    
    if(fname.rfind(".")!=std::string::npos)
    {
        pos2=fname.rfind(".");
    }
    
    chk_var(fname.size());
    chk_var(pos1);
    chk_var(pos2);
    
    r=fname.substr(pos1+1,pos2-(pos1+1));
    
    chk_var(fname);
    chk_var(r);
    
    return r;
}

void filename_filter(std::string fname,std::string &path,std::string &core,std::string &extension)
{
    int pos1=0;
    int pos2=fname.size();
    
    if(fname.rfind("\\")!=std::string::npos)
    {
        pos1=fname.rfind("\\");
    }
    else pos1=-1;
    
    if(fname.rfind(".")!=std::string::npos)
    {
        pos2=fname.rfind(".");
    }
    
    chk_var(fname.size());
    chk_var(pos1);
    chk_var(pos2);
    
    path=fname.substr(0,pos1+1);
    core=fname.substr(pos1+1,pos2-(pos1+1));
    extension=fname.substr(pos2);
    
    chk_var(fname);
    chk_var(path);
    chk_var(core);
    chk_var(extension);
    
    chk_var(path.size());
    chk_var(core.size());
    chk_var(extension.size());
}

void fmap_names(std::filesystem::path const &fname,int type,
                std::filesystem::path &fname_x,
                std::filesystem::path &fname_y,
                std::filesystem::path &fname_z)
{
    fname_x=fname;
    fname_y=fname;
    fname_z=fname;
    
    if(type==E_FIELD)
    {
        fname_x.replace_filename(fname_x.stem().generic_string()+"_Ex_raw");
        fname_y.replace_filename(fname_y.stem().generic_string()+"_Ey_raw");
        fname_z.replace_filename(fname_z.stem().generic_string()+"_Ez_raw");
    }
    else if(type==H_FIELD)
    {
        fname_x.replace_filename(fname_x.stem().generic_string()+"_Hx_raw");
        fname_y.replace_filename(fname_y.stem().generic_string()+"_Hy_raw");
        fname_z.replace_filename(fname_z.stem().generic_string()+"_Hz_raw");
    }
    else if(type==S_FIELD)
    {
        fname_x.replace_filename(fname_x.stem().generic_string()+"_Sx_raw");
        fname_y.replace_filename(fname_y.stem().generic_string()+"_Sy_raw");
        fname_z.replace_filename(fname_z.stem().generic_string()+"_Sz_raw");
    }
}

void fmap_mats_name(std::filesystem::path const &fname,std::filesystem::path &fname_mats)
{
    fname_mats=fname;
    fname_mats.replace_filename(fname_mats.stem().generic_string() + "_mats_raw");
}

void fmap_script(std::filesystem::path const &fname,int type,bool real,double D1,double D2)
{
    std::filesystem::path fname_x,fname_y,fname_z,fname_mats;
    fmap_names(fname,type,fname_x,fname_y,fname_z);
    fmap_mats_name(fname,fname_mats);
    
    std::filesystem::path fname_out=fname;
    if(fname_out.extension()!=".m") fname_out.replace_extension(".m");
    
    std::ofstream file(fname_out,std::ios::out|std::ios::trunc|std::ios::binary);
    
    file<<"function out="<<fname.stem()<<"(varargin)"<<'\n';
    
    file<<""<<'\n';
    file<<"baseline=1;"<<'\n';
    file<<""<<'\n';
    file<<"x=linspace(0,6,1000);"<<'\n';
    file<<""<<'\n';
    file<<"r=(x<2)+(3.0-x).*(x>=2).*(x<3)+(x-5).*(x>=5).*(x<6)+(x>=6);"<<'\n';
    file<<"g=(x-1.0).*(x>=1).*(x<2)+(x>=2).*(x<4)+(5.0-x).*(x>=4).*(x<5);"<<'\n';
    file<<"b=(1.0-x).*(x<1)+(x-3).*(x>=3).*(x<4)+(x>=4);"<<'\n';
    file<<""<<'\n';
    file<<"cmap=[transpose(r) transpose(g) transpose(b)];"<<'\n'; 
    file<<""<<'\n';
    file<<"Fx=dlmread('"<<fname_x.filename()<<"')/baseline;"<<'\n';
    file<<"Fy=dlmread('"<<fname_y.filename()<<"')/baseline;"<<'\n';
    file<<"Fz=dlmread('"<<fname_z.filename()<<"')/baseline;"<<'\n';
    file<<"mats=dlmread('"<<fname_mats.filename()<<"');"<<'\n';
    file<<""<<'\n';
    file<<"Nx=size(Fx,2);"<<'\n';
    file<<"Ny=size(Fy,2);"<<'\n';
    file<<"Nz=size(Fz,2);"<<'\n';
    file<<""<<'\n';
    if(!real)
    {
        file<<"Fx_r=Fx(:,1:2:Nx);"<<'\n';
        file<<"Fx_i=Fx(:,2:2:Nx);"<<'\n';
        file<<"Fx=Fx_r+i*Fx_i;"<<'\n';
        file<<""<<'\n';
        file<<"Fy_r=Fy(:,1:2:Ny);"<<'\n';
        file<<"Fy_i=Fy(:,2:2:Ny);"<<'\n';
        file<<"Fy=Fy_r+i*Fy_i;"<<'\n';
        file<<""<<'\n';
        file<<"Fz_r=Fz(:,1:2:Nz);"<<'\n';
        file<<"Fz_i=Fz(:,2:2:Nz);"<<'\n';
        file<<"Fz=Fz_r+i*Fz_i;"<<'\n';
    }
    file<<""<<'\n';
    file<<"N1=size(Fx,2);"<<'\n';
    file<<"N2=size(Fx,1);"<<'\n';
    file<<"N1t=ones(N2,1)*(1:N1)*"<<D1<<";"<<'\n';
    file<<"N2t=transpose((1:N2)*"<<D2<<")*ones(1,N1);"<<'\n';
    file<<"N1=N1t;"<<'\n';
    file<<"N2=N2t;"<<'\n';
    file<<'\n';
    file<<"silent=0;"<<'\n';
    file<<'\n';
    file<<"if nargin>0"<<'\n';
    file<<"    j=1;"<<'\n';
    file<<"    while j<=nargin"<<'\n';
    file<<"        if strcmp(varargin{j},'saturate')"<<'\n';
    file<<"            val=varargin{j+1}/sqrt(3.0);"<<'\n';
    file<<"            Fx=Fx.*(abs(Fx)<val)+val*exp(i*angle(Fx)).*(abs(Fx)>=val);"<<'\n';
    file<<"            Fy=Fy.*(abs(Fy)<val)+val*exp(i*angle(Fy)).*(abs(Fy)>=val);"<<'\n';
    file<<"            Fz=Fz.*(abs(Fz)<val)+val*exp(i*angle(Fz)).*(abs(Fz)>=val);"<<'\n';
    file<<"            j=j+2;"<<'\n';
    file<<"            continue;"<<'\n';
    file<<"        end"<<'\n';
    file<<"        if strcmp(varargin{j},'silent')"<<'\n';
    file<<"            silent=1;"<<'\n';
    file<<"            j=j+1;"<<'\n';
    file<<"            continue;"<<'\n';
    file<<"        end"<<'\n';
    file<<"    end"<<'\n';
    file<<"end"<<'\n';
    file<<'\n';
    file<<"F_tot=sqrt(abs(Fx).^2+abs(Fy).^2+abs(Fz).^2);"<<'\n';
    file<<'\n';
    file<<"if silent==0"<<'\n';
    file<<"    figure(1)"<<'\n';
    if(!real) file<<"    pcolor(N1,N2,abs(Fx))"<<'\n';
    else file<<"    pcolor(N1,N2,Fx)"<<'\n';
    file<<"    colormap(jet(500))"<<'\n';
    file<<"    colorbar"<<'\n';
    file<<"    title('|E_x|')"<<'\n';
    file<<"    shading flat"<<'\n';
    file<<"    figure(2)"<<'\n';
    if(!real) file<<"    pcolor(N1,N2,abs(Fy))"<<'\n';
    else file<<"    pcolor(N1,N2,Fy)"<<'\n';
    file<<"    colormap(jet(500))"<<'\n';
    file<<"    colorbar"<<'\n';
    file<<"    title('|E_y|')"<<'\n';
    file<<"    shading flat"<<'\n';
    file<<"    figure(3)"<<'\n';
    if(!real) file<<"    pcolor(N1,N2,abs(Fz))"<<'\n';
    else file<<"    pcolor(N1,N2,Fz)"<<'\n';
    file<<"    colormap(jet(500))"<<'\n';
    file<<"    colorbar"<<'\n';
    file<<"    title('|E_z|')"<<'\n';
    file<<"    shading flat"<<'\n';
    file<<""<<'\n';
    if(!real)
    {
        file<<"    figure(4)"<<'\n';
        file<<"    pcolor(N1,N2,angle(Fx))"<<'\n';
        file<<"    colormap(cmap)"<<'\n';
        file<<"    shading flat"<<'\n';
        file<<"    title('arg(E_x)')"<<'\n';
        file<<"    figure(5)"<<'\n';
        file<<"    pcolor(N1,N2,angle(Fy))"<<'\n';
        file<<"    colormap(cmap)"<<'\n';
        file<<"    shading flat"<<'\n';
        file<<"    title('arg(E_y)')"<<'\n';
        file<<"    figure(6)"<<'\n';
        file<<"    pcolor(N1,N2,angle(Fz))"<<'\n';
        file<<"    colormap(cmap)"<<'\n';
        file<<"    shading flat"<<'\n';
        file<<"    title('arg(E_z)')"<<'\n';
    }
    file<<""<<'\n';
    if(!real) file<<"    figure(7)"<<'\n';
    else file<<"    figure(4)"<<'\n';
    file<<"    pcolor(N1,N2,F_tot)"<<'\n';
    file<<"    title('|E|')"<<'\n';
    file<<"    colormap(jet(500))"<<'\n';
    file<<"    colorbar"<<'\n';
    file<<"    shading flat"<<'\n';
    file<<"end"<<'\n';
    file<<""<<'\n';
    file<<"out.Ex=Fx;"<<'\n';
    file<<"out.Ey=Fy;"<<'\n';
    file<<"out.Ez=Fz;"<<'\n';
    file<<"out.mats=mats;"<<'\n';
    file<<""<<'\n';
    file<<"end"<<'\n';
    
    file.close();
}

void fmap_raw(std::filesystem::path const &fname,int type,
              Grid2<Imdouble> const &Gx,Grid2<Imdouble> const &Gy,Grid2<Imdouble> const &Gz,bool real)
{
    int i,j;
    
    std::filesystem::path fname_x,fname_y,fname_z;
    fmap_names(fname,type,fname_x,fname_y,fname_z);
    
    std::ofstream fx(fname_x,std::ios::out|std::ios::trunc);
    std::ofstream fy(fname_y,std::ios::out|std::ios::trunc);
    std::ofstream fz(fname_z,std::ios::out|std::ios::trunc);
    
    int span1=Gx.L1();
    int span2=Gx.L2();
    
    std::stringstream strm;
    
    if(!real)
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++) strm<<Gx(i,j).real()<<" "<<Gx(i,j).imag()<<" ";
            strm<<std::endl;
        }
    }
    else
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++) strm<<Gx(i,j).real()<<" ";
            strm<<std::endl;
        }
    }
    
    fx<<strm.str();
    strm.str("");
    
    if(!real)
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++) strm<<Gy(i,j).real()<<" "<<Gy(i,j).imag()<<" ";
            strm<<std::endl;
        }
    }
    else
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++) strm<<Gy(i,j).real()<<" ";
            strm<<std::endl;
        }
    }
    
    fy<<strm.str();
    strm.str("");
    
    if(!real)
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++) strm<<Gz(i,j).real()<<" "<<Gz(i,j).imag()<<" ";
            strm<<std::endl;
        }
    }
    else
    {
        for(j=0;j<span2;j++)
        {
            for(i=0;i<span1;i++)strm<<Gz(i,j).real()<<" ";
            strm<<std::endl;
        }
    }
    
    fz<<strm.str();
    
    fx.close();
    fy.close();
    fz.close();
}

void fmap_mats_raw(std::filesystem::path const &fname,Grid2<unsigned int> const &mats)
{
    int i,j;
    
    std::filesystem::path fname_mats;
    fmap_mats_name(fname,fname_mats);
    
    std::ofstream file(fname_mats,std::ios::out|std::ios::trunc);
    
    int span1=mats.L1();
    int span2=mats.L2();
    
    std::stringstream strm;
    
    for(j=0;j<span2;j++)
    {
        for(i=0;i<span1;i++) strm<<mats(i,j)<<" ";
        
        strm<<std::endl;
    }
    
    file<<strm.str();
    
    file.close();
}

//#########################
//   FieldBlockHolder
//#########################

FieldBlockHolder::FieldBlockHolder()
    :x1(0), y1(0), z1(0),
     Nx(0), Ny(0), Nz(0),
     Dx(0), Dy(0), Dz(0),
     lambda(0), baseline(1.0)
{
}

FieldBlockHolder::FieldBlockHolder(FieldBlockHolder const &F)
    :x1(F.x1), y1(F.y1), z1(F.z1),
     Nx(F.Nx), Ny(F.Ny), Nz(F.Nz),
     Dx(F.Dx), Dy(F.Dy), Dz(F.Dz),
     lambda(F.lambda), baseline(F.baseline)
{
    int Nx=F.mats.L1();
    int Ny=F.mats.L2();
    int Nz=F.mats.L3();
    
    mats.init(Nx,Ny,Nz,0);
    
    Ex.init(Nx,Ny,Nz,0);
    Ey.init(Nx,Ny,Nz,0);
    Ez.init(Nx,Ny,Nz,0);
    
    Hx.init(Nx,Ny,Nz,0);
    Hy.init(Nx,Ny,Nz,0);
    Hz.init(Nx,Ny,Nz,0);
    
    // Copy
    
    mats=F.mats;
    
    Ex=F.Ex; Ey=F.Ey; Ez=F.Ez;
    Hx=F.Hx; Hy=F.Hy; Hz=F.Hz;
}

double FieldBlockHolder::get_E_abs(int i,int j,int k)
{
    return std::sqrt(std::norm(Ex(i,j,k))+
                     std::norm(Ey(i,j,k))+
                     std::norm(Ez(i,j,k)));
}

double FieldBlockHolder::get_E_abs_max()
{
    int i,j,k;
    double R=0;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        R=std::max(R,get_E_abs(i,j,k));
    }
    
    return R;
}

double FieldBlockHolder::get_Ex_abs(int i,int j,int k) { return std::abs(Ex(i,j,k)); }

double FieldBlockHolder::get_Ex_abs_max()
{
    int i,j,k;
    double R=0;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        R=std::max(R,std::abs(Ex(i,j,k)));
    }
    
    return R;
}

double FieldBlockHolder::get_Ey_abs(int i,int j,int k) { return std::abs(Ey(i,j,k)); }

double FieldBlockHolder::get_Ey_abs_max()
{
    int i,j,k;
    double R=0;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        R=std::max(R,std::abs(Ey(i,j,k)));
    }
    
    return R;
}

double FieldBlockHolder::get_Ez_abs(int i,int j,int k) { return std::abs(Ez(i,j,k)); }

double FieldBlockHolder::get_Ez_abs_max()
{
    int i,j,k;
    double R=0;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        R=std::max(R,std::abs(Ez(i,j,k)));
    }
    
    return R;
}

double FieldBlockHolder::integrate_poynting_plane(int direction,int i1,int i2,int j1,int j2,int k1,int k2)
{
    int i,j,k;
    double S=0,coeff=1.0;
    
         if(direction==NORMAL_X || direction==NORMAL_XM)
    {
        if(direction==NORMAL_X) coeff=Dy*Dz;
        else coeff=-Dy*Dz;
        
        i=i1;
        
        for(j=std::max(0,j1);j<std::min(Ny,j2);j++)
            for(k=std::max(0,k1);k<std::min(Nz,k2);k++)
                S+=0.5*real(Ey(i,j,k)*conj(Hz(i,j,k))-Ez(i,j,k)*conj(Hy(i,j,k)));
                
    }
    else if(direction==NORMAL_Y || direction==NORMAL_YM)
    {
        if(direction==NORMAL_Y) coeff=Dx*Dz;
        else coeff=-Dx*Dz;
        
        j=j1;
        
        for(i=std::max(0,i1);i<std::min(Nx,i2);i++)
            for(k=std::max(0,k1);k<std::min(Nz,k2);k++)
                S+=0.5*real(Ez(i,j,k)*conj(Hx(i,j,k))-Ex(i,j,k)*conj(Hz(i,j,k)));
    }
    else if(direction==NORMAL_Z || direction==NORMAL_ZM)
    {
        if(direction==NORMAL_Z) coeff=Dx*Dy;
        else coeff=-Dx*Dy;
        
        k=k1;
        
        for(i=std::max(0,i1);i<std::min(Nx,i2);i++)
            for(j=std::max(0,j1);j<std::min(Ny,j2);j++)
                S+=0.5*real(Ex(i,j,k)*conj(Hy(i,j,k))-Ey(i,j,k)*conj(Hx(i,j,k)));
    }
    
    return S*coeff;
}

bool FieldBlockHolder::load(std::string const &fname)
{
    int i,j,k;
        
    std::ifstream file(fname.c_str(),std::ios::in|std::ios::binary);
    
    file.read(reinterpret_cast<char*>(&lambda),sizeof(double));
    file.read(reinterpret_cast<char*>(&x1),sizeof(int));
    file.read(reinterpret_cast<char*>(&Nx),sizeof(int));
    file.read(reinterpret_cast<char*>(&y1),sizeof(int));
    file.read(reinterpret_cast<char*>(&Ny),sizeof(int));
    file.read(reinterpret_cast<char*>(&z1),sizeof(int));
    file.read(reinterpret_cast<char*>(&Nz),sizeof(int));
    file.read(reinterpret_cast<char*>(&Dx),sizeof(double));
    file.read(reinterpret_cast<char*>(&Dy),sizeof(double));
    file.read(reinterpret_cast<char*>(&Dz),sizeof(double));
    
    std::cout<<lambda<<std::endl;
    std::cout<<x1<<" "<<y1<<" "<<z1<<std::endl;
    std::cout<<"Nx: "<<Nx<<" Ny: "<<Ny<<" Nz: "<<Nz<<std::endl;
    std::cout<<Dx<<" "<<Dy<<" "<<Dz<<std::endl;
    std::cout<<file.tellg()<<std::endl;
    
    mats.init(Nx,Ny,Nz,0);
    
    Ex.init(Nx,Ny,Nz,0);
    Ey.init(Nx,Ny,Nz,0);
    Ez.init(Nx,Ny,Nz,0);
    
    Hx.init(Nx,Ny,Nz,0);
    Hy.init(Nx,Ny,Nz,0);
    Hz.init(Nx,Ny,Nz,0);
    
    double p_r,p_i;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Ex(i,j,k).real(p_r); Ex(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Ey(i,j,k).real(p_r); Ey(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Ez(i,j,k).real(p_r); Ez(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Hx(i,j,k).real(p_r); Hx(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Hy(i,j,k).real(p_r); Hy(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i),sizeof(double));
        Hz(i,j,k).real(p_r); Hz(i,j,k).imag(p_i);
        
        file.read(reinterpret_cast<char*>(&mats(i,j,k)),sizeof(unsigned int));
    }
    
    return true;
}

bool FieldBlockHolder::save(std::string const &fname)
{
    int i,j,k;
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc|std::ios::binary);
    
    if(!file.is_open()) return false;
    
    file.write(reinterpret_cast<char*>(&lambda),sizeof(double));
    file.write(reinterpret_cast<char*>(&x1),sizeof(int));
    file.write(reinterpret_cast<char*>(&Nx),sizeof(int));
    file.write(reinterpret_cast<char*>(&y1),sizeof(int));
    file.write(reinterpret_cast<char*>(&Ny),sizeof(int));
    file.write(reinterpret_cast<char*>(&z1),sizeof(int));
    file.write(reinterpret_cast<char*>(&Nz),sizeof(int));
    file.write(reinterpret_cast<char*>(&Dx),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dy),sizeof(double));
    file.write(reinterpret_cast<char*>(&Dz),sizeof(double));
    
    double p_r,p_i;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        p_r=std::real(Ex(i,j,k)); p_i=std::imag(Ex(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Ey(i,j,k)); p_i=std::imag(Ey(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Ez(i,j,k)); p_i=std::imag(Ez(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hx(i,j,k)); p_i=std::imag(Hx(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hy(i,j,k)); p_i=std::imag(Hy(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        p_r=std::real(Hz(i,j,k)); p_i=std::imag(Hz(i,j,k));
        file.write(reinterpret_cast<char*>(&p_r),sizeof(double));
        file.write(reinterpret_cast<char*>(&p_i),sizeof(double));
        
        file.write(reinterpret_cast<char*>(&mats(i,j,k)),sizeof(unsigned int));
    }
    
    file.close();
    
    return true;
}

void FieldBlockHolder::save_matlab(int direction,int location,int field,std::string fname)
{
    int i,j,k;
    int span1=0,span2=0;
    
         if(direction==NORMAL_X) { span1=Ny; span2=Nz; }
    else if(direction==NORMAL_Y) { span1=Nx; span2=Nz; }
    else if(direction==NORMAL_Z) { span1=Nx; span2=Ny; }
    
    Grid2<unsigned int> Gm(span1,span2);
    Grid2<Imdouble> Gx(span1,span2),  Gy(span1,span2),  Gz(span1,span2),
                    tHx(span1,span2), tHy(span1,span2), tHz(span1,span2);
    int index=location;
    
    if(direction==NORMAL_X)
    {
        for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
                Gm(j,k)=mats(index,j,k);
        
             if(field==E_FIELD) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            Gx(j,k)=Ex(index,j,k);
            Gy(j,k)=Ey(index,j,k);
            Gz(j,k)=Ez(index,j,k);
        }
        else if(field==H_FIELD) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            Gx(j,k)=Hx(index,j,k);
            Gy(j,k)=Hy(index,j,k);
            Gz(j,k)=Hz(index,j,k);
        }
//        else if(field==S_FIELD)
//            for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
//                { Gx(j,k)=Px(index,j,k); Gy(j,k)=Py(index,j,k); Gz(j,k)=Pz(index,j,k); }
    }
    else if(direction==NORMAL_Y)
    {
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
            Gm(i,k)=mats(i,index,k);
        
             if(field==E_FIELD) for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            Gx(i,k)=Ex(i,index,k);
            Gy(i,k)=Ey(i,index,k);
            Gz(i,k)=Ez(i,index,k);
        }
        else if(field==H_FIELD) for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            Gx(i,k)=Hx(i,index,k);
            Gy(i,k)=Hy(i,index,k);
            Gz(i,k)=Hz(i,index,k);
        }
//        else if(field==S_FIELD)
//            for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
//                { Gx(i,k)=Px(i,index,k); Gy(i,k)=Py(i,index,k); Gz(i,k)=Pz(i,index,k); }
    }
    else if(direction==NORMAL_Z)
    {
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
                Gm(i,j)=mats(i,j,index);
        
             if(field==E_FIELD) for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            Gx(i,j)=Ex(i,j,index);
            Gy(i,j)=Ey(i,j,index);
            Gz(i,j)=Ez(i,j,index);
        }
        else if(field==H_FIELD) for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            Gx(i,j)=Hx(i,j,index);
            Gy(i,j)=Hy(i,j,index);
            Gz(i,j)=Hz(i,j,index);
        }
//        else if(field==S_FIELD)
//            for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
//                { Gx(i,j)=Px(i,j,index); Gy(i,j)=Py(i,j,index); Gz(i,j)=Pz(i,j,index); }
    }
    
    if(field!=S_FIELD)
    {
             if(direction==NORMAL_X) fmap_script(fname,field,false,Dy,Dz);
        else if(direction==NORMAL_Y) fmap_script(fname,field,false,Dx,Dz);
        else if(direction==NORMAL_Z) fmap_script(fname,field,false,Dx,Dy);
        
        fmap_raw(fname,field,Gx,Gy,Gz);
        fmap_mats_raw(fname,Gm);
    }
    else
    {
             if(direction==NORMAL_X) fmap_script(fname,field,true,Dy,Dz);
        else if(direction==NORMAL_Y) fmap_script(fname,field,true,Dx,Dz);
        else if(direction==NORMAL_Z) fmap_script(fname,field,true,Dx,Dy);
        
        fmap_raw(fname,field,Gx,Gy,Gz,true);
        fmap_mats_raw(fname,Gm);
    }
}

void FieldBlockHolder::set_baseline(double baseline_)
{
    int i,j,k;
    
    undo_baseline();
    
    baseline=baseline_;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        Ex(i,j,k)/=baseline;
        Ey(i,j,k)/=baseline;
        Ez(i,j,k)/=baseline;
        
        Hx(i,j,k)/=baseline;
        Hy(i,j,k)/=baseline;
        Hz(i,j,k)/=baseline;
    }
}

void FieldBlockHolder::undo_baseline()
{
    int i,j,k;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        Ex(i,j,k)*=baseline;
        Ey(i,j,k)*=baseline;
        Ez(i,j,k)*=baseline;
        
        Hx(i,j,k)*=baseline;
        Hy(i,j,k)*=baseline;
        Hz(i,j,k)*=baseline;
    }
    
    baseline=1.0;
}
