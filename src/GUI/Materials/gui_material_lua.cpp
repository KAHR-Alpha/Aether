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

#include <enum_tools.h>
#include <filehdl.h>
#include <gui_material.h>
#include <lua_material.h>

namespace lua_gui_material
{
    int allocate(lua_State *L)
    {
        GUI::Material *p_material=MaterialsLib::request_material(MatType::CUSTOM);
        
        lua_set_metapointer<::Material>(L,"metatable_material",p_material); // polymorphic allocation
        
        if(lua_gettop(L)>0)
        {
                 if(lua_isnumber(L,1)) p_material->set_const_n(lua_tonumber(L,1));
            else if(lua_isstring(L,1))
            {
                Loader ld;
                ld.load(p_material,lua_tostring(L,1));
            }
        }
        
        MaterialsLib::consolidate(p_material);
        
        return 1;
    }
    
    template<lua_material::Mode mode>
    int add_effective_component(lua_State *L)
    {
        int s=lua_material::get_shift(mode);
        Material *mat=lua_material::get_mat_pointer<mode>(L);
        
        GUI::Material *new_mat=nullptr;
        
        if(lua_isnumber(L,1+s))
        {
            new_mat=MaterialsLib::request_material(MatType::REAL_N);
            new_mat->set_const_n(lua_tonumber(L,1+s));
        }
        else if(lua_isstring(L,1+s))
        {
            lua_getglobal(L,"lua_caller_path");
            
            std::filesystem::path script=lua_tostring(L,1+s);
            std::filesystem::path *caller_path=static_cast<std::filesystem::path*>(lua_touserdata(L,-1));
            
            script=PathManager::locate_file(script,*caller_path);
            
            new_mat=MaterialsLib::request_material(script);
        }
        else if(lua_isuserdata(L,1+s))
        {
            new_mat=dynamic_cast<GUI::Material*>(lua_get_metapointer<Material>(L,1+s));
        }
        
        mat->eff_mats.push_back(new_mat);
        mat->eff_weights.push_back(lua_tonumber(L,2+s));
        
        return 0;
    }
    
    template<lua_material::Mode mode>
    int set_effective_type(lua_State *L)
    {
        int s=lua_material::get_shift(mode);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(lua_material::get_mat_pointer<mode>(L));
        
        std::string model_str=lua_tostring(L,1+s);
        
        EffectiveModel model=EffectiveModel::BRUGGEMAN;
        
             if(model_str=="bruggeman")       model=EffectiveModel::BRUGGEMAN;
        else if(model_str=="looyenga")        model=EffectiveModel::LOOYENGA;
        else if(model_str=="maxwell_garnett") model=EffectiveModel::MAXWELL_GARNETT;
        else if(model_str=="sum")             model=EffectiveModel::SUM;
        else if(model_str=="inverse_sum")     model=EffectiveModel::SUM_INV;
        
        mat->is_effective_material=true;
        mat->effective_type=model;
        
        if(mat->type!=MatType::SCRIPT)
            mat->type=MatType::EFFECTIVE;
        
        return 0;
    }
    
    template<lua_material::Mode mode>
    int set_index(lua_State *L)
    {
        Material *base_mat=lua_material::get_mat_pointer<mode>(L);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
        
        mat->set_const_n(lua_tonumber(L,2));
        mat->type=MatType::REAL_N;
        
        return 0;
    }
    
    template<lua_material::Mode mode>
    int set_script(lua_State *L)
    {
        Material *base_mat=lua_material::get_mat_pointer<mode>(L);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
        
        Loader ld;
        ld.load(mat,lua_tostring(L,2));
        
        mat->type=MatType::SCRIPT;
        
        MaterialsLib::consolidate(&mat);
        
        return 0;
    }
    
    //############
    //   Loader
    //############
    
    Loader::Loader()
    {
        set_allocation_function(allocate);
                             
        replace_functions("add_effective_component",
                          add_effective_component<lua_material::Mode::LIVE>,
                          add_effective_component<lua_material::Mode::SCRIPT>);
                             
        replace_functions("load_script",set_script<lua_material::Mode::LIVE>,
                                        set_script<lua_material::Mode::SCRIPT>);
                                         
        replace_functions("refractive_index",set_index<lua_material::Mode::LIVE>,
                                             set_index<lua_material::Mode::SCRIPT>);
                                         
        replace_functions("effective_type",set_effective_type<lua_material::Mode::LIVE>,
                                           set_effective_type<lua_material::Mode::SCRIPT>);
    }
    
    //################
    //   Translator
    //################
    
    Translator::Translator(std::filesystem::path const &relative_path_)
        :finalized(false),
         relative_path(relative_path_)
    {
    }
    
    void Translator::finalize()
    {
        if(finalized) return; 
        
        std::stringstream strm;
        
        // Mapping pointer - names
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            name_map[materials[i]]="Material_" + std::to_string(i);
        }
        
        // Materials declaration
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            strm<<name_map[materials[i]]<<"=Material()\n";
        }
        
        strm<<"\n";
        
        // Materials definition
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            to_lua(materials[i],strm,name_map[materials[i]]+":");
            
            strm<<"\n";
        }
        
        header=strm.str();
        
        finalized=true;
    }
    
    void Translator::gather(GUI::Material *material)
    {
        if(!vector_contains(materials,material))
        {
            materials.push_back(material);
        }
    }
    
    std::string Translator::get_header()
    {
        if(!finalized) finalize();
        
        return header;
    }
    
    std::string Translator::name(GUI::Material *material) const
    {
        if(vector_contains(materials,material))
        {
            return name_map.at(material);
        }
        else
        {
            if(material->is_const())
            {
                return std::to_string(material->get_n(0).real());
            }
            else
            {
                std::filesystem::path output=material->script_path.generic_string();
                
                output=PathManager::to_default_path(output, relative_path);
                
                return "\""+output.generic_string()+"\"";
            }
        }
    }
    
    std::string Translator::operator() (GUI::Material *material) const
    {
        return name(material);
    }
    
    void Translator::save_to_file(GUI::Material *material)
    {
        std::ofstream file(material->script_path,std::ios::out|std::ios::binary|std::ios::trunc);
        
        to_lua(material,file,"");
    }
    
    void Translator::to_lua(GUI::Material *material,std::ostream &strm,std::string const &prefix)
    {
        MatType type=material->type;
        
        if(!material->name.empty()) strm<<prefix<<"name(\""<<material->name<<"\")\n";
        
        if(type==MatType::REAL_N)
        {
            strm<<prefix<<"refractive_index("<<std::real(material->get_n(0))<<")\n";
        }
        else if(type==AnyOf(MatType::LIBRARY,
                            MatType::USER_LIBRARY,
                            MatType::SCRIPT))
        {
            strm<<prefix<<"load_script(\""<<material->script_path.generic_string()<<"\")\n";
        }
        else if(type==MatType::EFFECTIVE) to_lua_effective(material,strm,prefix);
        else if(type==MatType::CUSTOM) to_lua_custom(material,strm,prefix);
    }
    
    void Translator::to_lua_custom(GUI::Material *material,std::ostream &strm,std::string const &prefix)
    {
        std::size_t i;
        
        if(!material->description.empty()) strm<<prefix<<"description(\""<<material->description<<"\")\n";
        
        strm<<prefix<<"validity_range("<<material->lambda_valid_min<<","<<material->lambda_valid_max<<")\n";
        strm<<prefix<<"epsilon_infinity("<<material->eps_inf<<")\n";
        
        for(i=0;i<material->debye.size();i++)
            strm<<prefix<<"add_debye("<<material->debye[i].ds<<","
                                      <<material->debye[i].t0<<")\n";
            
        for(i=0;i<material->drude.size();i++)
            strm<<prefix<<"add_drude("<<material->drude[i].wd<<","
                                      <<material->drude[i].g<<")\n";
            
        for(i=0;i<material->lorentz.size();i++)
            strm<<prefix<<"add_lorentz("<<material->lorentz[i].A<<","
                                        <<material->lorentz[i].O<<","
                                        <<material->lorentz[i].G<<")\n";

        for(i=0;i<material->critpoint.size();i++)
            strm<<prefix<<"add_critpoint("<<material->critpoint[i].A<<","
                                          <<material->critpoint[i].O<<","
                                          <<material->critpoint[i].P<<","
                                          <<material->critpoint[i].G<<")\n";

        for(i=0;i<material->cauchy_coeffs.size();i++)
        {
            strm<<prefix<<"add_cauchy(";
            
            for(std::size_t j=0;j<material->cauchy_coeffs[i].size();j++)
            {
                strm<<prefix<<material->cauchy_coeffs[i][j];
                if(j+1!=material->cauchy_coeffs[i].size()) strm<<prefix<<",";
            }
            
            strm<<prefix<<")\n";
        }

        for(i=0;i<material->sellmeier_B.size();i++)
            strm<<prefix<<"add_sellmeier("<<material->sellmeier_B[i]<<","<<material->sellmeier_C[i]<<")\n";
            
        for(i=0;i<material->spd_lambda.size();i++)
        {
            strm<<prefix<<"lambda={";
            for(std::size_t j=0;j<material->spd_lambda[i].size();j++)
            {
                strm<<prefix<<material->spd_lambda[i][j];
                
                if(j+1<material->spd_lambda[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            strm<<prefix<<"data_r={";
            for(std::size_t j=0;j<material->spd_r[i].size();j++)
            {
                strm<<prefix<<material->spd_r[i][j];
                
                if(j+1<material->spd_r[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            strm<<prefix<<"data_i={";
            for(std::size_t j=0;j<material->spd_i[i].size();j++)
            {
                strm<<prefix<<material->spd_i[i][j];
                
                if(j+1<material->spd_i[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            
            strm<<prefix<<"\n";
            if(material->spd_type_index[i]) strm<<prefix<<"add_data_index";
            else strm<<prefix<<"add_data_epsilon";
            
            strm<<prefix<<"(lambda,data_r,data_i)\n";
        }
    }
    
    void Translator::to_lua_effective(GUI::Material *material,std::ostream &strm,std::string const &prefix)
    {
        std::size_t i;
        
        strm<<prefix<<"effective_type(\"";
        
        switch(material->effective_type)
        {
            case EffectiveModel::BRUGGEMAN:
                strm<<"bruggeman";
                break;
                
            case EffectiveModel::LOOYENGA:
                strm<<"looyenga";
                break;
                
            case EffectiveModel::MAXWELL_GARNETT:
                strm<<"maxwell_garnett";
                break;
                
            case EffectiveModel::SUM:
                strm<<"sum";
                break;
                
            case EffectiveModel::SUM_INV:
                strm<<"inverse_sum";
                break;
        }
        
        strm<<"\")\n";
        
        for(i=0;i<material->eff_mats.size();i++)
        {
            strm<<prefix<<"add_effective_component("
                <<name(dynamic_cast<GUI::Material*>(material->eff_mats[i]))
                <<","
                <<material->eff_weights[i]
                <<")\n";
        }
        
        if(material->effective_type==EffectiveModel::MAXWELL_GARNETT)
        {
            strm<<prefix<<"effective_host("<<material->maxwell_garnett_host<<")\n";
        }
    }
}
