/*Copyright 2008-2023 - Loïc Le Cunff

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

//#####################
//   Material script
//#####################

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
    
    CritpointModel critpoint;
    critpoint.set(A,O,P,G);
    
    mat->critpoint.push_back(critpoint);
    
    return 0;
}

int lmat_add_debye(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double ds=lua_tonumber(L,1);
    double t0=lua_tonumber(L,2);
    
    DebyeModel debye;
    debye.set(ds,t0);
    
    mat->debye.push_back(debye);
    
    return 0;
}

int lmat_add_drude(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double wd=lua_tonumber(L,1);
    double g=lua_tonumber(L,2);
    
    DrudeModel drude;
    drude.set(wd,g);
    
    mat->drude.push_back(drude);
    
    return 0;
}

int lmat_add_lorentz(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    double A=lua_tonumber(L,1);
    double O=lua_tonumber(L,2);
    double G=lua_tonumber(L,3);
    
    LorentzModel lorentz;
    lorentz.set(A,O,G);
    
    mat->lorentz.push_back(lorentz);
    
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

int lmat_add_cauchy(lua_State *L)
{
    int N=lua_gettop(L);
    
    Material *mat=get_mat_pointer(L,"binded_material");
    
    std::vector<double> coeffs(N);
    
    for(int i=0;i<N;i++)
        coeffs[i]=lua_tonumber(L,i+1);
    
    mat->cauchy_coeffs.push_back(coeffs);
    
    return 0;
}

template<int type>
int lmat_add_data_table(lua_State *L)
{
    Material *mat=get_mat_pointer(L,"binded_material");
    
    std::vector<double> lambda,data_r,data_i;
    
    lua_tools::extract_vector(lambda,L,1); // Wavelength at this point
    lua_tools::extract_vector(data_r,L,2); // Input real part
    lua_tools::extract_vector(data_i,L,3); // Input imag part
    
    bool type_index=false;
    
    if constexpr(type==0) type_index=true;
    
    mat->add_spline_data(lambda,data_r,data_i,type_index);
    
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
    reset();
    
    if(!std::filesystem::is_regular_file(script_path_))
    {
        std::cerr<<"Error: Couldn't find the material "<<script_path_<<" ...\nAborting...\n";
        std::exit(EXIT_FAILURE);
        return;
    }
    
    script_path=script_path_;
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"binded_material");
    
    std::filesystem::path caller_path=script_path.parent_path();
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&caller_path));
    lua_setglobal(L,"lua_caller_path");
    
    lua_register(L,"add_cauchy",lmat_add_cauchy);
    lua_register(L,"add_crit_point",lmat_add_crit_point);
    lua_register(L,"add_data_epsilon",lmat_add_data_table<1>);
    lua_register(L,"add_data_index",lmat_add_data_table<0>);
    lua_register(L,"add_debye",lmat_add_debye);
    lua_register(L,"add_drude",lmat_add_drude);
    lua_register(L,"add_lorentz",lmat_add_lorentz);
    lua_register(L,"add_sellmeier",lmat_set_sellmeier);
    lua_register(L,"description",lmat_description);
    lua_register(L,"epsilon_infinity",lmat_epsilon_infty);
    lua_register(L,"epsilon_infty",lmat_epsilon_infty);
    lua_register(L,"index_infinity",lmat_index_infty);
    lua_register(L,"index_infty",lmat_index_infty);
    lua_register(L,"validity_range",lmat_set_validity_range);
    
    luaL_loadfile(L,script_path.generic_string().c_str());
    lua_pcall(L,0,0,0);
    
    lua_close(L);
}

void Material::write_lua_script()
{
    std::size_t i;
    
    std::ofstream file(script_path,std::ios::out|std::ios::binary|std::ios::trunc);
    
    if(!description.empty()) file<<"description(\""<<description<<"\")\n\n";
    
    file<<"validity_range("<<lambda_valid_min<<","<<lambda_valid_max<<")\n\n";
    file<<"epsilon_infinity("<<eps_inf<<")\n\n";
    
    for(i=0;i<debye.size();i++)
        file<<"add_debye("<<debye[i].ds<<","<<debye[i].t0<<")\n";
        
    for(i=0;i<drude.size();i++)
        file<<"add_drude("<<drude[i].wd<<","<<drude[i].g<<")\n";
        
    for(i=0;i<lorentz.size();i++)
        file<<"add_lorentz("<<lorentz[i].A<<","<<lorentz[i].O<<","<<lorentz[i].G<<")\n";

    for(i=0;i<critpoint.size();i++)
        file<<"add_critpoint("<<critpoint[i].A<<","<<critpoint[i].O<<","<<critpoint[i].P<<","<<critpoint[i].G<<")\n";

    for(i=0;i<cauchy_coeffs.size();i++)
    {
        file<<"add_cauchy(";
        
        for(std::size_t j=0;j<cauchy_coeffs[i].size();j++)
        {
            file<<cauchy_coeffs[i][j];
            if(j+1!=cauchy_coeffs[i].size()) file<<",";
        }
        
        file<<")\n";
    }

    for(i=0;i<sellmeier_B.size();i++)
        file<<"add_sellmeier("<<sellmeier_B[i]<<","<<sellmeier_C[i]<<")\n";
        
    for(i=0;i<spd_lambda.size();i++)
    {
        file<<"lambda={";
        for(std::size_t j=0;j<spd_lambda[i].size();j++)
        {
            file<<spd_lambda[i][j];
            
            if(j+1<spd_lambda[i].size()) file<<",";
            else file<<"}\n";
        }
        file<<"data_r={";
        for(std::size_t j=0;j<spd_r[i].size();j++)
        {
            file<<spd_r[i][j];
            
            if(j+1<spd_r[i].size()) file<<",";
            else file<<"}\n";
        }
        file<<"data_i={";
        for(std::size_t j=0;j<spd_i[i].size();j++)
        {
            file<<spd_i[i][j];
            
            if(j+1<spd_i[i].size()) file<<",";
            else file<<"}\n";
        }
        
        file<<"\n";
        if(spd_type_index[i]) file<<"add_data_index";
        else file<<"add_data_epsilon";
        
        file<<"(lambda,data_r,data_i)\n\n";
    }

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

//########################
//   Material Metatable
//########################

int lua_create_material(lua_State *L)
{
    Material *p_material=lua_allocate_metapointer<Material>(L,"metatable_material");
    
    if(lua_gettop(L)>0)
    {
             if(lua_isnumber(L,1)) p_material->set_const_n(lua_tonumber(L,1));
        else if(lua_isstring(L,1)) p_material->load_lua_script(lua_tostring(L,1));
    }
    
    return 1;
}

int lua_material_set_index(lua_State *L)
{
    Material *mat=lua_get_metapointer<Material>(L,1);
    mat->set_const_n(lua_tonumber(L,2));
    
    return 0;
}

int lua_material_set_name(lua_State *L)
{
    Material *mat=lua_get_metapointer<Material>(L,1);
    mat->name=lua_tostring(L,2);
    
    return 0;
}

int lua_material_set_script(lua_State *L)
{
    Material *mat=lua_get_metapointer<Material>(L,1);
    mat->load_lua_script(lua_tostring(L,2));
    
    return 0;
}
