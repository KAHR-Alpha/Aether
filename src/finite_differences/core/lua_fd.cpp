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

#include <filehdl.h>
#include <logger.h>
#include <lua_fd.h>
#include <lua_material.h>
#include <mesh_base.h>
#include <mesh_tools.h>
#include <string_tools.h>

//####################
//       FD
//####################

/*fdtd_structure_generator::fdtd_structure_generator()
    :set(false)
{
}

void fdtd_structure_generator::append(std::string cmd)
{
    append_cmd.push_back(cmd);
}

void fdtd_structure_generator::parameter(std::string cmd)
{
    parameter_cmd.push_back(cmd);
}*/


FD_Mode::FD_Mode()
    :type(MODE_NONE),
     polar_angle(0),
     polarization("TE"),
     Dx(5e-9), Dy(5e-9), Dz(5e-9),
     pad_xm(0), pad_xp(0),
     pad_ym(0), pad_yp(0),
     pad_zm(0), pad_zp(0),
     periodic_x(false),
     periodic_y(false),
     periodic_z(false),
     pml_xm(0), pml_xp(0),
     pml_ym(0), pml_yp(0),
     pml_zm(0), pml_zp(0),
     kappa_xm(25), kappa_xp(25),
     kappa_ym(25), kappa_yp(25),
     kappa_zm(25), kappa_zp(25),
     sigma_xm(1.0), sigma_xp(1.0),
     sigma_ym(1.0), sigma_yp(1.0),
     sigma_zm(1.0), sigma_zp(1.0),
     alpha_xm(0.2), alpha_xp(0.2),
     alpha_ym(0.2), alpha_yp(0.2),
     alpha_zm(0.2), alpha_zp(0.2)
{
}

void FD_Mode::compute_discretization(int &Nx,int &Ny,int &Nz,
                                     double lx,double ly,double lz) const
{
    Nx=std::max(1,nearest_integer(lx/Dx));
    Ny=std::max(1,nearest_integer(ly/Dy));
    Nz=std::max(1,nearest_integer(lz/Dz));
    
    chk_var(lx);
    chk_var(ly);
    chk_var(lz);
    chk_var(Nx);
    chk_var(Ny);
    chk_var(Nz);
}


std::filesystem::path const& FD_Mode::directory() const
{
    return p_directory;
}


double FD_Mode::get_Lx()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nx*Dx;
}

double FD_Mode::get_Ly()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Ny*Dy;
}

double FD_Mode::get_Lz()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nz*Dz;
}

int FD_Mode::get_Nx()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nx;
}

int FD_Mode::get_Ny()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Ny;
}

int FD_Mode::get_Nz()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nz;
}

void FD_Mode::set_discretization(double D_) { Dx=Dy=Dz=D_; }
void FD_Mode::set_discretization_x(double Dx_) { Dx=Dx_; }
void FD_Mode::set_discretization_y(double Dy_) { Dy=Dy_; }
void FD_Mode::set_discretization_z(double Dz_) { Dz=Dz_; }


void FD_Mode::set_material(int mat_index,Material const &material)
{
    if(mat_index<0) return;
    else if(mat_index>=static_cast<int>(materials.size()))
        materials.resize(mat_index+1);
    
    materials[mat_index]=material;
}


void FD_Mode::set_material(int mat_index,std::filesystem::path const &mat_file)
{
    #ifdef OLDMAT
    for(unsigned int i=0;i<materials_index.size();i++)
    {
        if(materials_index[i]==mat_index)
        {
            materials_str[i]=mat_file;
            return;
        }
    }
    
    materials_index.push_back(mat_index);
    materials_str.push_back(mat_file);
    #endif
    
    if(mat_index<0) return;
    else if(mat_index>=static_cast<int>(materials.size()))
        materials.resize(mat_index+1);
    
    lua_material::Loader loader;
    loader.load(&materials[mat_index],mat_file);
}


void FD_Mode::set_padding(int pad_xm_,int pad_xp_,int pad_ym_,int pad_yp_,int pad_zm_,int pad_zp_)
{
    pad_xm=pad_xm_; pad_xp=pad_xp_;
    pad_ym=pad_ym_; pad_yp=pad_yp_;
    pad_zm=pad_zm_; pad_zp=pad_zp_;
    
    Plog::print("Setting the window padding to\n");
    Plog::print("     X( ", pad_xm, " , ", pad_xp, " )\n");
    Plog::print("     Y( ", pad_ym, " , ", pad_yp, " )\n");
    Plog::print("     Z( ", pad_zm, " , ", pad_zp, " )\n");
}


void FD_Mode::set_pml_xm(int N_pml,double kap,double sig,double alp)
{
    pml_xm=N_pml;
    kappa_xm=kap;
    sigma_xm=sig;
    alpha_xm=alp;
    periodic_x=false;
    pad_xm=std::max(pad_xm,5);
    
    Plog::print("Setting the lower X PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_xm, "\n");
    Plog::print("     sigma= ", sigma_xm, "\n");
    Plog::print("     alpha= ", alpha_xm, "\n");
}

void FD_Mode::set_pml_xp(int N_pml,double kap,double sig,double alp)
{
    pml_xp=N_pml;
    kappa_xp=kap;
    sigma_xp=sig;
    alpha_xp=alp;
    periodic_x=false;
    pad_xp=std::max(pad_xp,5);
    
    Plog::print("Setting the upper X PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_xp, "\n");
    Plog::print("     sigma= ", sigma_xp, "\n");
    Plog::print("     alpha= ", alpha_xp, "\n");
}

void FD_Mode::set_pml_ym(int N_pml,double kap,double sig,double alp)
{
    pml_ym=N_pml;
    kappa_ym=kap;
    sigma_ym=sig;
    alpha_ym=alp;
    periodic_y=false;
    pad_ym=std::max(pad_ym,5);
    
    Plog::print("Setting the lower Y PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_ym, "\n");
    Plog::print("     sigma= ", sigma_ym, "\n");
    Plog::print("     alpha= ", alpha_ym, "\n");
}

void FD_Mode::set_pml_yp(int N_pml,double kap,double sig,double alp)
{
    pml_yp=N_pml;
    kappa_yp=kap;
    sigma_yp=sig;
    alpha_yp=alp;
    periodic_y=false;
    pad_yp=std::max(pad_yp,5);
    
    Plog::print("Setting the upper Y PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_yp, "\n");
    Plog::print("     sigma= ", sigma_yp, "\n");
    Plog::print("     alpha= ", alpha_yp, "\n");
}

void FD_Mode::set_pml_zm(int N_pml,double kap,double sig,double alp)
{
    pml_zm=N_pml;
    kappa_zm=kap;
    sigma_zm=sig;
    alpha_zm=alp;
    periodic_z=false;
    pad_zm=std::max(pad_zm,5);
    
    Plog::print("Setting the lower Z PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_zm, "\n");
    Plog::print("     sigma= ", sigma_zm, "\n");
    Plog::print("     alpha= ", alpha_zm, "\n");
}

void FD_Mode::set_pml_zp(int N_pml,double kap,double sig,double alp)
{
    pml_zp=N_pml;
    kappa_zp=kap;
    sigma_zp=sig;
    alpha_zp=alp;
    periodic_z=false;
    pad_zp=std::max(pad_zp,5);
    
    Plog::print("Setting the upper Z PML to ", N_pml, " cells with\n");
    Plog::print("     kappa= ", kappa_zp, "\n");
    Plog::print("     sigma= ", sigma_zp, "\n");
    Plog::print("     alpha= ", alpha_zp, "\n");
}

void FD_Mode::recompute_padding()
{
}

void FD_Mode::reset()
{
    type=MODE_NONE;
    polar_angle=0;
    polarization="TE";
    pad_xm=0; pad_xp=0;
    pad_ym=0; pad_yp=0;
    pad_zm=0; pad_zp=0;
    periodic_x=true;
    periodic_y=true;
    periodic_z=true;
    pml_xm=0; pml_xp=0;
    pml_ym=0; pml_yp=0;
    pml_zm=0; pml_zp=0;
    kappa_xm=25; kappa_xp=25;
    kappa_ym=25; kappa_yp=25;
    kappa_zm=25; kappa_zp=25;
    sigma_xm=1.0; sigma_xp=1.0;
    sigma_ym=1.0; sigma_yp=1.0;
    sigma_zm=1.0; sigma_zp=1.0;
    alpha_xm=0.2; alpha_xp=0.2;
    alpha_ym=0.2; alpha_yp=0.2;
    alpha_zm=0.2; alpha_zp=0.2;
    
    #ifdef OLDMAT
    materials_index.clear();
    materials_str.clear();
    #endif
    materials.clear();
    disable_fields.clear();
}


void FD_Mode::set_directory(std::filesystem::path const &directory_)
{
    p_directory=directory_;
    
    if(!p_directory.empty())
    {
        bool impossible = false;
            
        std::error_code error;
        
        if(std::filesystem::exists(p_directory, error))
        {
            std::error_code check_error;
            
            if(std::filesystem::is_directory(p_directory, check_error))
            {
                return;
            }
            else if(check_error)
            {
                Plog::print(LogType::FATAL, "Could not check the status of ", p_directory, " , error ", check_error.message(), "\n");
            }
            else
            {
                Plog::print(LogType::WARNING, p_directory, " exists but is not a directory\n");
                impossible = true;
            }
        }
        else if(error)
        {
            impossible = true;
            Plog::print(LogType::FATAL, "Could not access ", p_directory, " , error ", error.message(), "\n");
        }
        
        if(!impossible)
        {
            Plog::print(LogType::WARNING, "The directory ", p_directory, " does not exist, attempting to creatie it\n");
            
            std::error_code creation_error;
            
            if(std::filesystem::create_directories(p_directory, creation_error))
            {
                Plog::print(LogType::WARNING, "ok\n");
                return;
            }
            else if(creation_error)
            {
                Plog::print(LogType::FATAL, "Could not create ", p_directory, " , error: ", creation_error.message(), "\n");
            }
            else
            {
                Plog::print(LogType::WARNING, "Could not create ", p_directory, "\n");
            }
        }
    }
    
    Plog::print(LogType::FATAL, "The directory ", p_directory, " is not accessible");
    Plog::print(LogType::FATAL, ", falling back to the temporary directory: ", PathManager::tmp_path, "\n");
    
    p_directory=PathManager::tmp_path;
}


void FD_Mode::show() const
{
    Plog::print("FD Mmode\n");
    
    chk_msg_sc(type);
    chk_msg_sc(prefix);
    chk_msg_sc(p_directory);
    chk_msg_sc(polar_angle);
    chk_msg_sc(polarization);
    
    chk_msg_sc(pad_xm);
    chk_msg_sc(pad_xp);
    chk_msg_sc(pad_ym);
    chk_msg_sc(pad_yp);
    chk_msg_sc(pad_zm);
    chk_msg_sc(pad_zp);
    
    chk_msg_sc(pml_xm);
    chk_msg_sc(pml_xp);
    chk_msg_sc(pml_ym);
    chk_msg_sc(pml_yp);
    chk_msg_sc(pml_zm);
    chk_msg_sc(pml_zp);
    
    chk_msg_sc(kappa_xm);
    chk_msg_sc(kappa_xp);
    chk_msg_sc(kappa_ym);
    chk_msg_sc(kappa_yp);
    chk_msg_sc(kappa_zm);
    chk_msg_sc(kappa_zp);
    
    chk_msg_sc(sigma_xm);
    chk_msg_sc(sigma_xp);
    chk_msg_sc(sigma_ym);
    chk_msg_sc(sigma_yp);
    chk_msg_sc(sigma_zm);
    chk_msg_sc(sigma_zp);
    
    chk_msg_sc(alpha_xm);
    chk_msg_sc(alpha_xp);
    chk_msg_sc(alpha_ym);
    chk_msg_sc(alpha_yp);
    chk_msg_sc(alpha_zm);
    chk_msg_sc(alpha_zp);
    
    #ifdef OLDMAT
    for(unsigned int i=0;i<materials_str.size();i++)
    {
        chk_msg_sc(materials_index[i]);
        chk_msg_sc(materials_str[i]);
    }
    #endif
}

void FD_Mode::set_polarization(double polar)
{
    polar_angle=polar;
    polarization="mix";
}
void FD_Mode::set_polarization(std::string polar) { polarization=polar; }
void FD_Mode::set_prefix(std::string name) { prefix=name; }

void FD_Mode::set_output_directory(std::string dir) { p_directory=dir; }

void FD_Mode::set_structure(Structure *structure_)
{
    structure=structure_;
    structure->finalize();
}

void FD_Mode::process()
{
}


int FD_mode_disable_fields(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    int N=lua_gettop(L);
    
    for(int i=2;i<=N;i++)
    {
        std::string str=lua_tostring(L,i);
        
             if(str=="ex" || str=="Ex" || str=="EX" || str=="eX")
        {
            (*pp_fd)->disable_fields.push_back(EX_FIELD);
            Plog::print("Disabling the Ex field\n");
        }
        else if(str=="ey" || str=="Ey" || str=="EY" || str=="eY")
        {
            (*pp_fd)->disable_fields.push_back(EY_FIELD);
            Plog::print("Disabling the Ey field\n");
        }
        else if(str=="ez" || str=="Ez" || str=="EZ" || str=="eZ")
        {
            (*pp_fd)->disable_fields.push_back(EZ_FIELD);
            Plog::print("Disabling the Ez field\n");
        }
        else if(str=="hx" || str=="Hx" || str=="HX" || str=="hX")
        {
            (*pp_fd)->disable_fields.push_back(HX_FIELD);
            Plog::print("Disabling the Hx field\n");
        }
        else if(str=="hy" || str=="Hy" || str=="HY" || str=="hY")
        {
            (*pp_fd)->disable_fields.push_back(HY_FIELD);
            Plog::print("Disabling the Hy field\n");
        }
        else if(str=="hz" || str=="Hz" || str=="HZ" || str=="hZ")
        {
            (*pp_fd)->disable_fields.push_back(HZ_FIELD);
            Plog::print("Disabling the Hz field\n");
        }
    }
    
    return 1;
}


int FD_mode_get_lx(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Lx());
    
    return 1;
}

int FD_mode_get_ly(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Ly());
    
    return 1;
}

int FD_mode_get_lz(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Lz());
    
    return 1;
}

int FD_mode_get_nx(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Nx());
    
    return 1;
}

int FD_mode_get_ny(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Ny());
    
    return 1;
}

int FD_mode_get_nz(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Nz());
    
    return 1;
}

int FD_mode_set_material(lua_State *L)
{
    FD_Mode *p_fd=lua_get_metapointer<FD_Mode>(L,1);
    
    int mat_index=lua_tointeger(L,2);
    
    if(lua_isstring(L,3))
    {
        std::string mat_file=lua_tostring(L,3);
        
        lua_getglobal(L,"lua_caller_path");
        std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
        std::filesystem::path mat_path=PathManager::locate_file(mat_file,*caller_path);
        
        Plog::print("Setting the material ", mat_index, " to ", mat_path, "\n");
        
        p_fd->set_material(mat_index,mat_path);
    }
    else
    {
        Material *material=lua_get_metapointer<Material>(L,3);
        
        Plog::print("Setting the material ", mat_index, " to ", material->name, "\n");
        
        p_fd->set_material(mat_index,*material);
    }
    
    return 1;
}

int FD_mode_set_output_directory(lua_State *L)
{
    FD_Mode *p_fd=lua_get_metapointer<FD_Mode>(L,1);
    
    std::string directory=lua_tostring(L,2);
    
    p_fd->set_directory(directory);
    
    Plog::print("Setting the results output directory to ", p_fd->directory(), "\n");
    
    return 1;
}

int FD_mode_set_polarization(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    if(lua_isnumber(L,2))
    {
        double polar=lua_tonumber(L,2);
        
        Plog::print("Setting the polarization to ", polar, " degrees with respect to TE\n");
        (*pp_fd)->set_polarization(polar);
    }
    else
    {
        std::string polarization="TE";
        std::string tmp=lua_tostring(L,2);
        
        if(tmp=="te" || tmp=="TE" || tmp=="Te" || tmp=="tE") polarization="TE";
        if(tmp=="tm" || tmp=="TM" || tmp=="Tm" || tmp=="tM") polarization="TM";
        
        Plog::print("Setting the polarization to ", polarization, "\n");
        
        (*pp_fd)->set_polarization(polarization);
    }
    
    return 1;
}

int FD_mode_set_prefix(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    std::string prefix=lua_tostring(L,2);
    
    int N=prefix.size();
    if(prefix[N-1]!='_') prefix.append("_");
    
    Plog::print("Setting the simulation prefix to ", prefix, "\n");
    
    (*pp_fd)->set_prefix(prefix);
    
    return 1;
}

int FD_mode_set_structure(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    Structure *p_structure=lua_get_metapointer<Structure>(L,2);
    
    (*pp_fd)->set_structure(p_structure);
    
    Plog::print("Setting the structure to ", p_structure->get_script_path(), "\n");
    
    return 1;
}