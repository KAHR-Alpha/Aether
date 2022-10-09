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

#include <filehdl.h>
#include <material.h>
#include <phys_tools.h>

extern std::ofstream plog;
extern const double Pi;
extern const Imdouble Im;

Material * get_mat_pointer(lua_State *L,std::string bind_name)
{
    lua_getglobal(L,bind_name.c_str());
    return reinterpret_cast<Material*>(lua_touserdata(L,-1));
}

int lmat_add_crit_point(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double A=lua_tonumber(L,1);
    double O=lua_tonumber(L,2);
    double P=lua_tonumber(L,3);
    double G=lua_tonumber(L,4);
    
    mat->dielec.add_critpoint(A,O,P,G);
    
    return 0;
}

int lmat_add_debye(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double ds=lua_tonumber(L,1);
    double t0=lua_tonumber(L,2);
    
    mat->dielec.add_debye(ds,t0);
    
    return 0;
}

int lmat_add_drude(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double wd=lua_tonumber(L,1);
    double g=lua_tonumber(L,2);
    
    mat->dielec.add_drude(wd,g);
    
    return 0;
}

int lmat_add_lorentz(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double A=lua_tonumber(L,1);
    double O=lua_tonumber(L,2);
    double G=lua_tonumber(L,3);
    
    mat->dielec.add_lorentz(A,O,G);
    
    return 0;
}

int lmat_description(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    mat->description=lua_tostring(L,1);
    
    return 0;
}

int lmat_epsilon_infty(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    mat->eps_inf=lua_tonumber(L,1);
    
    return 0;
}

int lmat_index_infty(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double n=lua_tonumber(L,1);
    mat->eps_inf=n*n;
    
    return 0;
}

int lmat_set_cauchy(lua_State *L)
{
    int N=lua_gettop(L);
    
    Material *mat=get_mat_pointer(L,"binded_material");
    
    std::vector<double> coeffs(N);
    
    for(int i=0;i<N;i++)
        coeffs[i]=lua_tonumber(L,i+1);
    
    mat->set_type_cauchy(coeffs);
    
    return 0;
}

template<int type>
int lmat_add_data_file(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    mat->type=MAT_SPLINE;
    
    std::string fname=lua_tostring(L,1);
    
    lua_getglobal(L,"lua_caller_path");
    
    std::filesystem::path *p_parent_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
    std::filesystem::path parent_path=*p_parent_path;
    
    std::ifstream file(parent_path/fname,std::ios::in|std::ios::binary);
    
//    if(!file.is_open())
//    {
//        lua_getglobal(L,"script_path");
//        std::string fname_relat=lua_tostring(L,-1);
//        fname_relat.append(fname);
//        
//        std::cout<<"Cannot open '"<<fname<<"' , checking for '"<<fname_relat<<"'"<<std::endl;
//        
//        file.open(fname_relat,std::ios::in|std::ios::binary);
//        
//        if(!file.is_open())
//        {
//            std::cout<<"Cannot open '"<<fname_relat<<"' , aborting"<<std::endl;
//            std::exit(0);
//        }
//        else
//        {
//            std::cout<<"'"<<fname_relat<<"' found"<<std::endl;
//            fname=fname_relat;
//        }
//    }
    
    int Nl=fcountlines((parent_path/fname).generic_string());
    
    double lambda;
    std::vector<double> w(Nl),er(Nl),ei(Nl);
    
    if constexpr(type==0)
    {
        double n,k;
        
        for(int i=0;i<Nl;i++)
        {
            file>>lambda;
            file>>n;
            file>>k;
            
            w[i]=2.0*Pi*c_light/lambda;
            
            Imdouble eps=n+k*Im;
            eps=eps*eps;
            
            er[i]=eps.real();
            ei[i]=eps.imag();
        }
    }
    else
    {
        for(int i=0;i<Nl;i++)
        {
            file>>lambda;
            
            w[i]=2.0*Pi*c_light/lambda;
            
            file>>er[i];
            file>>ei[i];
        }
    }
    
    if(w[0]>w[Nl-1])
    {
        for(int i=0;i<Nl/2;i++)
        {
            std::swap(w[i],w[Nl-1-i]);
            std::swap(er[i],er[Nl-1-i]);
            std::swap(ei[i],ei[Nl-1-i]);
        }
    }
    
    Cspline sp_er,sp_ei;
    
    sp_er.init(w,er);
    sp_ei.init(w,ei);
    
    mat->er_spline.push_back(sp_er);
    mat->ei_spline.push_back(sp_ei);
    
    return 0;
}

int lmat_set_sellmeier(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    mat->sellmeier_B.push_back(lua_tonumber(L,1));
    mat->sellmeier_C.push_back(lua_tonumber(L,2));
    
    return 0;
}

int lmat_set_validity_range(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    mat->lambda_valid_min=lua_tonumber(L,1);
    mat->lambda_valid_max=lua_tonumber(L,2);
    
    return 0;
}

void Material::load_lua_script(std::filesystem::path const &script_path_)
{
    if(!std::filesystem::is_regular_file(script_path_))
    {
        std::cerr<<"Error: Couldn't find the material "<<script_path_<<" ...\nAborting...\n";
        std::exit(EXIT_FAILURE);
        return;
    }
    
    script_path=script_path_;
    
    dielec.clear();
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"binded_material");
    
    std::filesystem::path caller_path=script_path.parent_path();
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&caller_path));
    lua_setglobal(L,"lua_caller_path");
    
    lua_register(L,"add_cauchy",lmat_set_cauchy);
    lua_register(L,"add_crit_point",lmat_add_crit_point);
    lua_register(L,"add_data_epsilon",lmat_add_data_file<1>);
    lua_register(L,"add_data_index",lmat_add_data_file<0>);
    lua_register(L,"add_debye",lmat_add_debye);
    lua_register(L,"add_drude",lmat_add_drude);
    lua_register(L,"add_lorentz",lmat_add_lorentz);
    lua_register(L,"add_sellmeier",lmat_set_sellmeier);
    lua_register(L,"description",lmat_description);
    lua_register(L,"epsilon_infty",lmat_epsilon_infty);
    lua_register(L,"index_infty",lmat_index_infty);
    lua_register(L,"validity_range",lmat_set_validity_range);
    
    luaL_loadfile(L,script_path.generic_string().c_str());
    lua_pcall(L,0,0,0);
    
    lua_close(L);
}

int spec_mat_ID=0;

//int gen_absorbing_material(lua_State *L)
//{
//    std::stringstream fname;
//    
//    fname<<"buf/absorbing_mat_"<<spec_mat_ID<<".lua";
//    
//    std::ofstream file(fname.str(),std::ios::out|std::ios::trunc);
//    
//    double lambda=lua_tonumber(L,1);
//    double n=lua_tonumber(L,2);
//    double alpha_coeff=lua_tonumber(L,3);
//    double lambda_width=lua_tonumber(L,4);
//    
//    double w=2.0*Pi*c_light/lambda;
//    
//    Imdouble n=real_part+imag_part*Im;
//    Imdouble n2=n*n;
//    
//    double eps_inf=n2.real();
//    double O=w;
//    double G=w;
//    double A=n2.imag();
//    
//    file<<"set_dielectric()"<<std::endl<<std::endl;
//    file<<"epsilon_infty("<<eps_inf<<")"<<std::endl<<std::endl;
//    file<<"add_lorentz("<<A<<","<<O<<","<<G<<")"<<std::endl;
//    
//    file.close();
//    lua_pushstring(L,fname.str().c_str());
//    
//    spec_mat_ID++;
//    
//    return 1;
//}

int gen_const_material(lua_State *L)
{
    std::filesystem::path fname=PathManager::to_temporary_path("const_mat_"+std::to_string(spec_mat_ID)+".lua");
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    file<<"set_constant()"<<std::endl;
    file<<"index_infty("<<lua_tonumber(L,1)<<")"<<std::endl;
    
    file.close();
    lua_pushstring(L,fname.generic_string().c_str());
    
    spec_mat_ID++;
    
    return 1;
}

int gen_complex_material(lua_State *L)
{
    std::filesystem::path fname=PathManager::to_temporary_path("complex_mat_"+std::to_string(spec_mat_ID)+".lua");
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    double lambda=lua_tonumber(L,1);
    double real_part=lua_tonumber(L,2);
    double imag_part=lua_tonumber(L,3);
    
    double w=2.0*Pi*c_light/lambda;
    
    Imdouble n=real_part+imag_part*Im;
    Imdouble n2=n*n;
    
    double eps_inf=n2.real();
    double O=w;
    double G=w;
    double A=n2.imag();
    
    file<<"set_dielectric()"<<std::endl<<std::endl;
    file<<"epsilon_infty("<<eps_inf<<")"<<std::endl<<std::endl;
    file<<"add_lorentz("<<A<<","<<O<<","<<G<<")"<<std::endl;
    
    file.close();
    lua_pushstring(L,fname.generic_string().c_str());
    
    spec_mat_ID++;
    
    return 1;
}
