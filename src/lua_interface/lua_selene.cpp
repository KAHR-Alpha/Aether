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

#include <lua_selene.h>

Selene_Mode::Selene_Mode()
    :rendered(false)
{
}

void Selene_Mode::add_object(Sel::Object *object) { selene.add_object(object); }
void Selene_Mode::add_light(Sel::Light *light) { selene.add_light(light); }

void Selene_Mode::process()
{
    if(!rendered) selene.render();
}

void Selene_Mode::optimize(OptimEngine *engine)
{
    selene.render();
    rendered=true;
}

void Selene_Mode::render() { selene.render(); rendered=true; }
void Selene_Mode::set_N_rays_disp(int Nr_disp) { selene.set_N_rays_disp(Nr_disp); }
void Selene_Mode::set_N_rays_total(int Nr_tot) { selene.set_N_rays_total(Nr_tot); }
void Selene_Mode::set_output_directory(std::string const &output_directory) { selene.set_output_directory(output_directory); }

// Lua mode wrappers

namespace LuaUI
{
    // Helper functions
    
    int diff_index(int i_lambda,int N_lambda,
                   int i_th,int N_th,
                   int i_phi,int N_phi,
                   int i_p,int N_p,int p_min,
                   int i_q,int N_q,int q_min)
    {
        return i_q-q_min+N_q*(i_p-p_min+N_p*(i_phi+N_phi*(i_th+i_lambda*N_th)));
    }
    
    int selene_diffract_merge_polar(lua_State *L)
    {
        lua_getglobal(L,"lua_caller_path");
        
        std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
        
        // Files
        std::filesystem::path out_path=lua_tostring(L,1);
        std::filesystem::path TE_path=lua_tostring(L,2);
        std::filesystem::path TM_path=lua_tostring(L,3);
        
        std::ofstream file_out(out_path,std::ios::out|std::ios::trunc|std::ios::binary);
        std::ifstream file_TE(TE_path,std::ios::in|std::ios::binary);
        std::ifstream file_TM(TM_path,std::ios::in|std::ios::binary);
        
        std::string buffer_TE,buffer_TM;
        std::vector<bool> buffer_clean;
        std::vector<std::string> data_TE,data_TM;
        
        while(!file_TE.eof())
        {
            buffer_TE.clear();
            buffer_TM.clear();
            
            std::getline(file_TE,buffer_TE,'\n');
            std::getline(file_TM,buffer_TM,'\n');
            
            clean_data_string(buffer_TE,&buffer_clean);
            clean_data_string(buffer_TM,&buffer_clean);
            
            split_string(data_TE,buffer_TE,' ');
            split_string(data_TM,buffer_TM,' ');
            
            int N=data_TE.size();
            if(N==0) continue;
            
            for(int i=0;i<7;i++)
                file_out<<data_TE[i]<<" ";
            
            for(int i=7;i<N-1;i++)
            {
                if((i-7)%6==2) file_out<<0.5*(std::stod(data_TE[i])+std::stod(data_TM[i]))<<" ";
                else file_out<<data_TE[i]<<" ";
            }
            
            file_out<<0.5*(std::stod(data_TE[N-1])+std::stod(data_TM[N-1]))<<"\n";
        }
                
        return 0;
    }
    
    int selene_diffract_convert(lua_State *L)
    {
        lua_getglobal(L,"lua_caller_path");
        
        std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
        
        // Files
        std::filesystem::path out_path=lua_tostring(L,1);
        std::filesystem::path in_path=lua_tostring(L,2);
        
        if(!out_path.is_absolute()) out_path=(*caller_path)/out_path;
        if(!in_path.is_absolute()) in_path=(*caller_path)/in_path;
        
        std::ofstream file_out(out_path,std::ios::out|std::ios::trunc|std::ios::binary);
        std::ifstream file_in(in_path,std::ios::in|std::ios::binary);
        
        // Grating
        double a1=lua_tonumber(L,3);
        double a2=lua_tonumber(L,4);
        double n_index=lua_tonumber(L,5);
        
        std::string buffer;
        std::vector<bool> buffer_clean;
        std::vector<std::string> data;
        
        std::vector<double> lambda_vec,th_vec,phi_vec;
        
        int p_min=std::numeric_limits<int>::max();
        int p_max=std::numeric_limits<int>::min();
        
        int q_min=std::numeric_limits<int>::max();
        int q_max=std::numeric_limits<int>::min();
        
        // First run to know the size of the full array
        
        while(!file_in.eof())
        {
            buffer.clear();
            std::getline(file_in,buffer,'\n');
            clean_data_string(buffer,&buffer_clean);
            
            split_string(data,buffer,' ');
            
            if(data.size()==0) continue;
            
            double lambda=std::stod(data[0]);
            double th=std::stod(data[1]);
            double phi=std::stod(data[2]);
            
            if(!vector_contains(lambda_vec,lambda)) lambda_vec.push_back(lambda);
            if(!vector_contains(th_vec,th)) th_vec.push_back(th);
            if(!vector_contains(phi_vec,phi)) phi_vec.push_back(phi);
            
            p_min=std::min(p_min,std::stoi(data[3]));
            p_max=std::max(p_max,std::stoi(data[4]));
            
            q_min=std::min(q_min,std::stoi(data[5]));
            q_max=std::max(q_max,std::stoi(data[6]));
        }
//        chk_var("A");
        
        if(!vector_contains(th_vec,90.0)) th_vec.push_back(90);
        
        std::sort(lambda_vec.begin(),lambda_vec.end());
        std::sort(th_vec.begin(),th_vec.end());
        std::sort(phi_vec.begin(),phi_vec.end());
        
        for(unsigned int i=0;i<lambda_vec.size();i++)
        {
            file_out<<lambda_vec[i];
            
            if(i<lambda_vec.size()-1) file_out<<" ";
            else file_out<<"\n";
        }
        
        for(unsigned int i=0;i<th_vec.size();i++)
        {
            file_out<<th_vec[i];
            
            if(i<th_vec.size()-1) file_out<<" ";
            else file_out<<"\n";
        }
        
        for(unsigned int i=0;i<phi_vec.size();i++)
        {
            file_out<<phi_vec[i];
            
            if(i<phi_vec.size()-1) file_out<<" ";
            else file_out<<"\n";
        }
        
        file_out<<p_min<<" "<<p_max<<" "<<q_min<<" "<<q_max<<"\n";
        
        file_in.clear();
        file_in.seekg(0);
        
        int N_lambda=lambda_vec.size();
        int N_th=th_vec.size();
        int N_phi=phi_vec.size();
        int N_p=p_max-p_min+1;
        int N_q=q_max-q_min+1;
        
        int N_data=N_lambda*N_th*N_phi*N_p*N_q;
        
        std::vector<double> val(N_data);
        
        for(int i=0;i<N_data;i++) val[i]=0;
        
        while(!file_in.eof())
        {
            buffer.clear();
            
            std::getline(file_in,buffer,'\n');
            clean_data_string(buffer,&buffer_clean);
            
            split_string(data,buffer,' ');
            
            if(data.size()==0) continue;
            
            double lambda=std::stod(data[0]);
            double th=std::stod(data[1]);
            double phi=std::stod(data[2]);
            
            int i_lambda=vector_locate(lambda_vec,lambda);
            int i_th=vector_locate(th_vec,th);
            int i_phi=vector_locate(phi_vec,phi);
            
            for(unsigned int i=7;i+1<data.size();i+=6)
            {
                int p=std::stoi(data[i]);
                int q=std::stoi(data[i+1]);
                
                val[diff_index(i_lambda,N_lambda,
                               i_th,N_th,
                               i_phi,N_phi,
                               p,N_p,p_min,
                               q,N_q,q_min)]=std::stod(data[i+2]);
            }
        }
        
        for(int i_lambda=0;i_lambda<N_lambda;i_lambda++)
        for(int i_th=0;i_th<N_th;i_th++)
        for(int i_phi=0;i_phi<N_phi;i_phi++)
        {
            file_out<<lambda_vec[i_lambda]<<" ";
            file_out<<th_vec[i_th]<<" ";
            file_out<<phi_vec[i_phi];
            
            double lambda=lambda_vec[i_lambda];
            double theta=th_vec[i_th]*Pi/180.0;
            double phi=phi_vec[i_phi]*Pi/180.0;
            
            double kn=2.0*Pi*n_index/lambda;
            
            double beta_x=std::cos(phi)*std::sin(theta);
            double beta_y=std::sin(phi)*std::sin(theta);
            
            beta_x*=kn;
            beta_y*=kn;
            
            for(int p=p_min;p<=p_max;p++)
            for(int q=q_min;q<=q_max;q++)
            {
                int ind=diff_index(i_lambda,N_lambda,
                                   i_th,N_th,i_phi,N_phi,
                                   p,N_p,p_min,q,N_q,q_min);
                
                double x=beta_x+p*2.0*Pi/a1;
                double y=beta_y+q*2.0*Pi/a2;
                
                file_out<<" "<<p<<" "<<q<<" "<<val[ind]<<" "<<x/kn<<" "<<y/kn;
            }
            
            file_out<<"\n";
        }
        
        file_out.close();
        
        return 0;
    }
    
    // Frames
    
    int selene_frame_set_displacement(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        
        p_frame->set_displacement(lua_tonumber(L,2),
                                  lua_tonumber(L,3),
                                  lua_tonumber(L,4));
        
        return 0;
    }
    
    int selene_frame_set_name(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        
        p_frame->name=lua_tostring(L,2);
        
        return 0;
    }
    
    int selene_frame_set_origin(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        
        p_frame->set_origin(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_frame_set_relative_origin(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        Sel::Frame *p_ref=lua_get_metapointer<Sel::Object>(L,2);
        
        p_frame->set_relative_origin(p_ref,lua_tointeger(L,3));
        
        return 0;
    }
    
    int selene_frame_set_rotation(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        
        p_frame->set_rotation(lua_tonumber(L,2),
                              lua_tonumber(L,3),
                              lua_tonumber(L,4));
        
        return 0;
    }
    
    int selene_frame_set_rotation_frame(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        Sel::Frame *p_ref=lua_get_metapointer<Sel::Object>(L,2);
        
        p_frame->set_rotation_frame(p_ref);
        
        return 0;
    }
    
    int selene_frame_set_translation_frame(lua_State *L)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Object>(L,1);
        Sel::Frame *p_ref=lua_get_metapointer<Sel::Object>(L,2);
        
        p_frame->set_translation_frame(p_ref);
        
        return 0;
    }
    
    // IRF
    int create_selene_IRF(lua_State *L)
    {
        Lua_memory_register *mem_reg=nullptr;
        int lmem_reg=lua_getglobal(L,"lua_mem_register");
        
        if(lmem_reg==LUA_TLIGHTUSERDATA)
            mem_reg=reinterpret_cast<Lua_memory_register*>(lua_touserdata(L,-1));
        
        Sel::IRF *p_irf=lua_allocate_metapointer<Sel::IRF>(L,"metatable_selene_irf");
        
        if(lmem_reg==LUA_TLIGHTUSERDATA) mem_reg->add_userdata(p_irf);
        
        return 1;
    }
    
    int selene_irf_add_layer(lua_State *L)
    {
        Sel::IRF *p_irf=lua_get_metapointer<Sel::IRF>(L,1);
        
        double h=lua_tonumber(L,2);
        Material *mat=lua_get_metapointer<Material>(L,3);
        
        p_irf->ml_heights.push_back(h);
        p_irf->ml_materials.push_back(mat);
        p_irf->ml_model.set_N_layers(p_irf->ml_heights.size());
        
        std::cout<<std::endl;
        for(std::size_t i=0;i<p_irf->ml_heights.size();i++)
        {
            chk_var(p_irf->ml_heights[i]);
        }
        
        return 0;
    }
    
    int selene_irf_set_name(lua_State *L)
    {
        Sel::IRF *p_irf=lua_get_metapointer<Sel::IRF>(L,1);
        
        p_irf->name=lua_tostring(L,2);
        
        return 0;
    }
    
    int selene_irf_set_splitting_factor(lua_State *L)
    {
        Sel::IRF *p_irf=lua_get_metapointer<Sel::IRF>(L,1);
        
        p_irf->splitting_factor=lua_tonumber(L,2);
        
        return 0;
    }
    
    int selene_irf_set_type(lua_State *L)
    {
        Sel::IRF *p_irf=lua_get_metapointer<Sel::IRF>(L,1);
        
        std::string type=lua_tostring(L,2);
        
             if(type=="fresnel") p_irf->set_type_fresnel();
        else if(type=="grating") p_irf->set_type_grating(lua_tostring(L,3),lua_tostring(L,4));
        else if(type=="multilayer") p_irf->set_type_multilayer();
        else if(type=="scatt_abs") p_irf->set_type_scatt_abs(lua_tonumber(L,3));
        else if(type=="snell_scatt_file") p_irf->set_type_snell_scatt_file(lua_tostring(L,3));
        else if(type=="snell_splitter") p_irf->set_type_snell_splitter(lua_tonumber(L,3));
        else
        {
            std::cerr<<"Unknown IRF type\nAborting..."<<std::endl;
            std::exit(EXIT_FAILURE);
        }
        
        return 0;
    }
    
    void Selene_create_allocation_functions(lua_State *L)
    {
        lua_register(L,"Selene_diffraction_merger",&LuaUI::selene_diffract_merge_polar);
        lua_register(L,"Selene_diffraction_converter",&LuaUI::selene_diffract_convert);
        lua_register(L,"Selene_object",&LuaUI::create_selene_object);
        lua_register(L,"Selene_IRF",&LuaUI::create_selene_IRF);
        lua_register(L,"Selene_light",&LuaUI::create_selene_light);
        lua_register(L,"Selene_target",&LuaUI::create_selene_target);
    }
    
    void Selene_create_base_metatable(lua_State *L)
    {
        create_obj_metatable(L,"metatable_selene");
        
        metatable_add_func(L,"add_object",&LuaUI::selene_mode_add_object);
        metatable_add_func(L,"add_light",&LuaUI::selene_mode_add_light);
        metatable_add_func(L,"N_rays_disp",&LuaUI::selene_mode_set_N_rays_disp);
        metatable_add_func(L,"N_rays_total",&LuaUI::selene_mode_set_N_rays_total);
        metatable_add_func(L,"render",&LuaUI::selene_mode_render);
        metatable_add_func(L,"output_directory",&LuaUI::selene_mode_output_directory);
    }
    
    void Selene_create_light_metatable(lua_State *L)
    {
        create_obj_metatable(L,"metatable_selene_light");
        
        lua_wrapper<0,Sel::Light,double>::bind(L,"power",&Sel::Light::set_power);
        
        metatable_add_func(L,"discrete_spectrum",&LuaUI::selene_light_set_discrete_spectrum);
        metatable_add_func(L,"extent",&LuaUI::selene_light_set_extent);
        metatable_add_func(L,"full_angle",&LuaUI::selene_light_set_angle);
        metatable_add_func(L,"location",&LuaUI::selene_frame_set_displacement);
        metatable_add_func(L,"material",&LuaUI::selene_light_set_material);
        metatable_add_func(L,"name",&LuaUI::selene_frame_set_name);
        metatable_add_func(L,"numerical_aperture",&LuaUI::selene_light_set_numerical_aperture);
        metatable_add_func(L,"origin",&LuaUI::selene_frame_set_origin);
        metatable_add_func(L,"polar_along",&LuaUI::selene_light_set_polar_along);
        metatable_add_func(L,"polar_not",&LuaUI::selene_light_set_polar_not);
        metatable_add_func(L,"polar_unset",&LuaUI::selene_light_set_polar_unset);
        metatable_add_func(L,"power",&LuaUI::selene_light_set_power);
        metatable_add_func(L,"ray_file",&LuaUI::selene_light_set_rays_file);
        metatable_add_func(L,"relative_origin",&LuaUI::selene_frame_set_relative_origin);
        metatable_add_func(L,"rotation",&LuaUI::selene_frame_set_rotation);
        metatable_add_func(L,"rotation_frame",&LuaUI::selene_frame_set_rotation_frame);
        metatable_add_func(L,"spectrum",&LuaUI::selene_light_set_spectrum);
        metatable_add_func(L,"translation_frame",&LuaUI::selene_frame_set_translation_frame);
        metatable_add_func(L,"waist_distance",&LuaUI::selene_light_set_waist_distance);
        metatable_add_func(L,"wavelength",&LuaUI::selene_light_set_wavelength);
    }
    
    void Selene_create_object_metatable(lua_State *L)
    {
        Sel::Object obj;
        
        std::vector<int> types;
//        types.push_back(Sel::OBJ_BOOLEAN);
        types.push_back(Sel::OBJ_BOX);
        types.push_back(Sel::OBJ_CONIC);
        types.push_back(Sel::OBJ_LENS);
//        types.push_back(Sel::OBJ_MESH);
        types.push_back(Sel::OBJ_SPHERE);
        types.push_back(Sel::OBJ_VOL_CONE);
        types.push_back(Sel::OBJ_VOL_CYLINDER);
        types.push_back(Sel::OBJ_PARABOLA);
        types.push_back(Sel::OBJ_DISK);
        types.push_back(Sel::OBJ_RECTANGLE);
        types.push_back(Sel::OBJ_SPHERE_PATCH);
        
        lua_pushinteger(L,0);
        lua_setglobal(L,"SEL_OBJ_CENTER");
        
        for(std::size_t i=0;i<types.size();i++)
        {
            obj.type=types[i];
            int N=obj.get_anchors_number();
            
            for(int j=0;j<N;j++)
            {
                std::string an=obj.get_anchor_script_name(j);
                lua_pushinteger(L,j);
                lua_setglobal(L,an.c_str());
                #ifdef DEV_INFO
                std::cout<<j<<" "<<an.c_str()<<std::endl;
                #endif
            }
        }
        
//        std::exit(0);
        
        create_obj_metatable(L,"metatable_selene_object");
        
        metatable_add_func(L,"add_mesh",&LuaUI::selene_object_add_mesh);
        metatable_add_func(L,"auto_recalc_normals",&LuaUI::selene_object_auto_recalc_normals);
        metatable_add_func(L,"contains",&LuaUI::selene_object_contains);
        metatable_add_func(L,"default_in_IRF",&LuaUI::selene_object_set_default_in_irf);
        metatable_add_func(L,"default_in_mat",&LuaUI::selene_object_set_default_in_mat);
        metatable_add_func(L,"default_IRF",&LuaUI::selene_object_set_default_irf);
        metatable_add_func(L,"default_out_IRF",&LuaUI::selene_object_set_default_out_irf);
        metatable_add_func(L,"default_out_mat",&LuaUI::selene_object_set_default_out_mat);
        metatable_add_func(L,"define_faces_group",&LuaUI::selene_object_define_faces_group);
        
        // - Faces
        
        metatable_add_func(L,"face_down_IRF",&LuaUI::selene_object_set_face_down_irf);
        metatable_add_func(L,"face_down_mat",&LuaUI::selene_object_set_face_down_mat);
        metatable_add_func(L,"face_down_tangent",&LuaUI::selene_object_set_face_down_tangent);
        metatable_add_func(L,"face_IRF",&LuaUI::selene_object_set_face_irf);
        metatable_add_func(L,"face_up_IRF",&LuaUI::selene_object_set_face_up_irf);
        metatable_add_func(L,"face_up_mat",&LuaUI::selene_object_set_face_up_mat);
        metatable_add_func(L,"face_up_tangent",&LuaUI::selene_object_set_face_up_tangent);
        
        // - Faces Groups
        
        metatable_add_func(L,"faces_group_down_IRF",&LuaUI::selene_object_set_faces_group_down_irf);
        metatable_add_func(L,"faces_group_down_mat",&LuaUI::selene_object_set_faces_group_down_mat);
        metatable_add_func(L,"faces_group_down_tangent",&LuaUI::selene_object_set_faces_group_down_tangent);
        metatable_add_func(L,"faces_group_IRF",&LuaUI::selene_object_set_faces_group_irf);
        metatable_add_func(L,"faces_group_up_IRF",&LuaUI::selene_object_set_faces_group_up_irf);
        metatable_add_func(L,"faces_group_up_mat",&LuaUI::selene_object_set_faces_group_up_mat);
        metatable_add_func(L,"faces_group_up_tangent",&LuaUI::selene_object_set_faces_group_up_tangent);
        
        //
        
        metatable_add_func(L,"location",&LuaUI::selene_frame_set_displacement);
        metatable_add_func(L,"name",&LuaUI::selene_frame_set_name);
        metatable_add_func(L,"N_faces",&LuaUI::selene_object_get_N_faces);
        metatable_add_func(L,"origin",&LuaUI::selene_frame_set_origin);
        metatable_add_func(L,"reference",&LuaUI::selene_object_get_variable_reference);
        metatable_add_func(L,"relative_origin",&LuaUI::selene_frame_set_relative_origin);
        metatable_add_func(L,"rescale_mesh",&LuaUI::selene_object_rescale_mesh);
        metatable_add_func(L,"rotation",&LuaUI::selene_frame_set_rotation);
        metatable_add_func(L,"rotation_frame",&LuaUI::selene_frame_set_rotation_frame);
        metatable_add_func(L,"sensor",&LuaUI::selene_object_set_sensor);
        metatable_add_func(L,"translation_frame",&LuaUI::selene_frame_set_translation_frame);
        
        create_obj_metatable(L,"metatable_selene_irf");
        
        metatable_add_func(L,"add_layer",&LuaUI::selene_irf_add_layer);
        metatable_add_func(L,"name",&LuaUI::selene_irf_set_name);
        metatable_add_func(L,"splitting_factor",&LuaUI::selene_irf_set_splitting_factor);
        metatable_add_func(L,"type",&LuaUI::selene_irf_set_type);
    }
    
    int selene_mode_add_object(lua_State *L)
    {
        Selene_Mode *p_mode=lua_get_metapointer<Selene_Mode>(L,1);
        Sel::Object *p_elem=lua_get_metapointer<Sel::Object>(L,2);
        
//        p_mode->elem_arr.push_back(p_elem);
        p_mode->add_object(p_elem);
        
        return 0;
    }
    
    int selene_mode_add_light(lua_State *L)
    {
        Selene_Mode *p_mode=*(reinterpret_cast<Selene_Mode**>(lua_touserdata(L,1)));
        Sel::Light *p_light=*(reinterpret_cast<Sel::Light**>(lua_touserdata(L,2)));
        
//        p_mode->light_arr.push_back(p_light);
        p_mode->add_light(p_light);
        
        return 0;
    }
    
    int selene_mode_optimize(lua_State *L)
    {
        Selene_Mode *p_mode=lua_get_metapointer<Selene_Mode>(L,1);
        OptimEngine *p_engine=lua_get_metapointer<OptimEngine>(L,2);
        
        p_mode->optimize(p_engine);
        
        return 0;
    }
    
    int selene_mode_output_directory(lua_State *L)
    {
        Selene_Mode *p_mode=lua_get_metapointer<Selene_Mode>(L,1);
        
        p_mode->set_output_directory(lua_tostring(L,2));
        
        return 0;
    }
    
    int selene_mode_set_N_rays_disp(lua_State *L)
    {
        Selene_Mode *p_mode=*(reinterpret_cast<Selene_Mode**>(lua_touserdata(L,1)));
        
        p_mode->set_N_rays_disp(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_mode_render(lua_State *L)
    {
        Selene_Mode *p_mode=lua_get_metapointer<Selene_Mode>(L,1);
        
        p_mode->render();
        
        return 0;
    }
    
    int selene_mode_set_N_rays_total(lua_State *L)
    {
        Selene_Mode *p_mode=*(reinterpret_cast<Selene_Mode**>(lua_touserdata(L,1)));
        
        p_mode->set_N_rays_total(lua_tointeger(L,2));
        
        return 0;
    }
}
