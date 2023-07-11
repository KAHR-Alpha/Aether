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
#include <lua_material.h>
#include <material.h>
#include <phys_tools.h>

extern std::ofstream plog;
extern const double Pi;
extern const Imdouble Im;

int spec_mat_ID=0;

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

//#####################################
//   Material manipulation functions
//#####################################

namespace lua_material
{
    int allocate(lua_State *L)
    {
        Material *p_material=lua_allocate_metapointer<Material>(L,"metatable_material");
        
        if(lua_gettop(L)>0)
        {
                 if(lua_isnumber(L,1)) p_material->set_const_n(lua_tonumber(L,1));
            else if(lua_isstring(L,1))
            {
                Loader ld;
                ld.load(p_material,lua_tostring(L,1));
            }
        }
        
        return 1;
    }
    
    int get_shift(Mode mode)
    {
        if(mode==Mode::LIVE) return 1;
        else return 0;
    }
    
    template<Mode mode>
    int add_cauchy(lua_State *L)
    {
        int s=get_shift(mode);
        int N=lua_gettop(L)-s;
        
        Material *mat=get_mat_pointer<mode>(L);
        
        std::vector<double> coeffs(N);
        
        for(int i=0;i<N;i++)
            coeffs[i]=lua_tonumber(L,i+1+s);
        
        mat->cauchy_coeffs.push_back(coeffs);
        
        return 0;
    }
    
    template<Mode mode>
    int add_crit_point(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        double A=lua_tonumber(L,1+s);
        double O=lua_tonumber(L,2+s);
        double P=lua_tonumber(L,3+s);
        double G=lua_tonumber(L,4+s);
        
        CritpointModel critpoint;
        critpoint.set(A,O,P,G);
        
        mat->critpoint.push_back(critpoint);
        
        return 0;
    }
    
    template<Mode mode>
    int add_debye(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        double ds=lua_tonumber(L,1+s);
        double t0=lua_tonumber(L,2+s);
        
        DebyeModel debye;
        debye.set(ds,t0);
        
        mat->debye.push_back(debye);
        
        return 0;
    }
    
    template<Mode mode>
    int add_drude(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        double wd=lua_tonumber(L,1+s);
        double g=lua_tonumber(L,2+s);
        
        DrudeModel drude;
        drude.set(wd,g);
        
        mat->drude.push_back(drude);
        
        return 0;
    }
    
    template<Mode mode>
    int add_effective_component(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        Material *new_mat=nullptr;
        
        if(lua_isnumber(L,1+s))
        {
            new_mat=new Material;
            new_mat->set_const_n(lua_tonumber(L,1+s));
        }
        else if(lua_isstring(L,1+s))
        {
            lua_getglobal(L,"lua_caller_path");
            
            std::filesystem::path script=lua_tostring(L,1+s);
            std::filesystem::path caller_path=lua_tostring(L,-1);
            
            script=PathManager::locate_file(script,caller_path);
            
            new_mat=new Material;
            
            Loader ld;
            ld.load(new_mat,script);
        }
        else if(lua_isuserdata(L,1+s))
        {
            new_mat=lua_get_metapointer<Material>(L,1+s);
        }
            
        mat->eff_mats.push_back(new_mat);
        mat->eff_weights.push_back(lua_tonumber(L,2+s));
        
        return 0;
    }
    
    template<Mode mode>
    int add_lorentz(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        double A=lua_tonumber(L,1+s);
        double O=lua_tonumber(L,2+s);
        double G=lua_tonumber(L,3+s);
        
        LorentzModel lorentz;
        lorentz.set(A,O,G);
        
        mat->lorentz.push_back(lorentz);
        
        return 0;
    }

    template<Mode mode>
    int add_sellmeier(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->sellmeier_B.push_back(lua_tonumber(L,1+s));
        mat->sellmeier_C.push_back(lua_tonumber(L,2+s));
        
        return 0;
    }
    
    template<Mode mode>
    int set_description(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->description=lua_tostring(L,1+s);
        
        return 0;
    }
    
    template<Mode mode>
    int epsilon_infty(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->eps_inf=lua_tonumber(L,1+s);
        
        return 0;
    }

    template<Mode mode>
    int index_infty(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        double n=lua_tonumber(L,1+s);
        mat->eps_inf=n*n;
        
        return 0;
    }

    template<Mode mode,int type>
    int add_data_table(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        std::vector<double> lambda,data_r,data_i;
        
        lua_tools::extract_vector(lambda,L,1+s); // Wavelength at this point
        lua_tools::extract_vector(data_r,L,2+s); // Input real part
        lua_tools::extract_vector(data_i,L,3+s); // Input imag part
        
        bool type_index=false;
        
        if constexpr(type==0) type_index=true;
        
        mat->add_spline_data(lambda,data_r,data_i,type_index);
        
        return 0;
    }

    template<Mode mode>
    int set_validity_range(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->lambda_valid_min=lua_tonumber(L,1+s);
        mat->lambda_valid_max=lua_tonumber(L,2+s);
        
        return 0;
    }
    
    template<Mode mode>
    int set_effective_host(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->maxwell_garnett_host=lua_tointeger(L,1+s);
        
        return 0;
    }
    
    template<Mode mode>
    int set_effective_type(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        std::string model_str=lua_tostring(L,1+s);
        
        EffectiveModel model=EffectiveModel::BRUGGEMAN;
        
             if(model_str=="bruggeman")       model=EffectiveModel::BRUGGEMAN;
        else if(model_str=="looyenga")        model=EffectiveModel::LOOYENGA;
        else if(model_str=="maxwell_garnett") model=EffectiveModel::MAXWELL_GARNETT;
        else if(model_str=="sum")             model=EffectiveModel::SUM;
        else if(model_str=="inverse_sum")     model=EffectiveModel::SUM_INV;
        
        mat->is_effective_material=true;
        mat->effective_type=model;
        
        return 0;
    }
    
    template<Mode mode>
    int set_index(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->set_const_n(lua_tonumber(L,1+s));
        
        return 0;
    }
    
    template<Mode mode>
    int set_name(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        mat->name=lua_tostring(L,1+s);
        
        return 0;
    }
    
    template<Mode mode>
    int set_script(lua_State *L)
    {
        int s=get_shift(mode);
        Material *mat=get_mat_pointer<mode>(L);
        
        Loader loader;
        loader.load(mat,lua_tostring(L,1+s));
        
        return 0;
    }
    
    //############
    //   Loader
    //############
    
    Loader::Loader()
    {
        set_allocation_function(allocate);
        
        add_functions("add_cauchy",add_cauchy<Mode::LIVE>,
                                   add_cauchy<Mode::SCRIPT>);
        
        add_functions("add_crit_point",add_crit_point<Mode::LIVE>,
                                       add_crit_point<Mode::SCRIPT>);
        
        add_functions("add_data_epsilon",add_data_table<Mode::LIVE,1>,
                                         add_data_table<Mode::SCRIPT,1>);
        
        add_functions("add_data_index",add_data_table<Mode::LIVE,0>,
                                       add_data_table<Mode::SCRIPT,0>);
        
        add_functions("add_debye",add_debye<Mode::LIVE>,
                                  add_debye<Mode::SCRIPT>);
        
        add_functions("add_drude",add_drude<Mode::LIVE>,
                                  add_drude<Mode::SCRIPT>);
        
        add_functions("add_effective_component",add_effective_component<Mode::LIVE>,
                                                add_effective_component<Mode::SCRIPT>);
        
        add_functions("add_lorentz",add_lorentz<Mode::LIVE>,
                                    add_lorentz<Mode::SCRIPT>);
        
        add_functions("add_sellmeier",add_sellmeier<Mode::LIVE>,
                                      add_sellmeier<Mode::SCRIPT>);
        
        add_functions("description",set_description<Mode::LIVE>,
                                    set_description<Mode::SCRIPT>);
        
        add_functions("effective_host",set_effective_host<Mode::LIVE>,
                                       set_effective_host<Mode::SCRIPT>);
        
        add_functions("effective_type",set_effective_type<Mode::LIVE>,
                                       set_effective_type<Mode::SCRIPT>);
        
        add_functions("epsilon_infinity",epsilon_infty<Mode::LIVE>,
                                         epsilon_infty<Mode::SCRIPT>);
        
        add_functions("epsilon_infty",epsilon_infty<Mode::LIVE>,
                                      epsilon_infty<Mode::SCRIPT>);
        
        add_functions("index_infinity",index_infty<Mode::LIVE>,
                                       index_infty<Mode::SCRIPT>);
        
        add_functions("index_infty",index_infty<Mode::LIVE>,
                                    index_infty<Mode::SCRIPT>);
        
        add_functions("name",set_name<Mode::LIVE>,
                             set_name<Mode::SCRIPT>);
                             
        add_functions("load_script",set_script<Mode::LIVE>,
                                    set_script<Mode::SCRIPT>);
                                         
        add_functions("refractive_index",set_index<Mode::LIVE>,
                                         set_index<Mode::SCRIPT>);
        
        add_functions("validity_range",set_validity_range<Mode::LIVE>,
                                       set_validity_range<Mode::SCRIPT>);
    }
    
    void Loader::add_functions(std::string const &name,int (*live_function)(lua_State*),int (*script_function)(lua_State*))
    {
        function_names.push_back(name);
        functions_live.push_back(live_function);
        functions_script.push_back(script_function);
    }
    
    void Loader::create_metatable(lua_State *L)
    {
        lua_register(L,"Material",allocation_function);
        
        create_obj_metatable(L,"metatable_material");
        
        for(std::size_t i=0;i<function_names.size();i++)
            metatable_add_func(L,function_names[i],functions_live[i]);
    }
    
    void Loader::load(Material *material,std::filesystem::path const &script_path_)
    {
        if(material==nullptr) return;
        
        material->reset();
    
        if(!std::filesystem::is_regular_file(script_path_))
        {
            std::cerr<<"Error: Couldn't find the material "<<script_path_<<" ...\nAborting...\n";
            std::exit(EXIT_FAILURE);
            return;
        }
        
        material->script_path=script_path_;
        
        lua_State *L=luaL_newstate();
        luaL_openlibs(L);
        
        lua_pushlightuserdata(L,static_cast<void*>(material));
        lua_setglobal(L,"bound_material");
        
        std::filesystem::path caller_path=script_path_.parent_path();
        lua_pushlightuserdata(L,static_cast<void*>(&caller_path));
        lua_setglobal(L,"lua_caller_path");
        
        for(std::size_t i=0;i<function_names.size();i++)
            lua_register(L,function_names[i].c_str(),functions_script[i]);
        
        luaL_loadfile(L,script_path_.generic_string().c_str());
        lua_pcall(L,0,0,0);
        
        lua_close(L);
    }
    
    void Loader::replace_functions(std::string const &name,int (*live_function)(lua_State*),int (*script_function)(lua_State*))
    {
        for(std::size_t i=0;i<function_names.size();i++)
        {
            if(name==function_names[i])
            {
                functions_live[i]=live_function;
                functions_script[i]=script_function;
                return;
            }
        }
    }
    
    void Loader::set_allocation_function(int (*allocation_function_)(lua_State*))
    {
        allocation_function=allocation_function_;
    }
}
