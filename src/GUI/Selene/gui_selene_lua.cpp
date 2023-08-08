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

#include <lua_optim.h>
#include <lua_selene.h>

#include <gui_selene.h>

extern std::ofstream plog;

namespace SelGUI
{

int lua_allocate_selene_IRF(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    Sel::IRF *irf=lua_allocate_metapointer<Sel::IRF>(L,"metatable_selene_irf");
    
    frame->user_irfs.push_back(irf);
    
    return 1;
}

int lua_allocate_selene_light(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Light
    
    Sel::Frame **ppf=reinterpret_cast<Sel::Frame**>(lua_newuserdata(L,sizeof(Sel::Frame*)));
    
    luaL_getmetatable(L,"metatable_selene_light");
    lua_setmetatable(L,-2);
    
    Sel::Light *p_light=new Sel::Light;
    frame->frames.push_back(p_light);
    *ppf=p_light;
    
    LuaUI::create_selene_light_type(L,p_light);
    
    return 1;
}

int lua_allocate_selene_object(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Object
    
    Sel::Frame **ppf=reinterpret_cast<Sel::Frame**>(lua_newuserdata(L,sizeof(Sel::Frame*)));
    
    luaL_getmetatable(L,"metatable_selene_object");
    lua_setmetatable(L,-2);
    
    Sel::Object *p_object=new Sel::Object;
    frame->frames.push_back(p_object);
    *ppf=p_object;
    
    LuaUI::create_selene_object_type(L,p_object);
    
    return 1;
}

int lua_allocate_SeleneFrame_pointer(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Object
    
    SeleneFrame **ppf=reinterpret_cast<SeleneFrame**>(lua_newuserdata(L,sizeof(SeleneFrame*)));
    
    *ppf=frame;
    
    luaL_getmetatable(L,"metatable_selene");
    lua_setmetatable(L,-2);
        
    return 1;
}

int lua_selene_add_light(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    null_function(frame);
    
    return 0;
}

int lua_selene_add_object(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    null_function(frame);
    
    return 0;
}

int lua_selene_set_N_rays_disp(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    frame->nr_disp->set_value(lua_tointeger(L,2));
    
    return 0;
}

int lua_selene_set_N_rays_total(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    frame->nr_tot->set_value(lua_tointeger(L,2));
    
    return 0;
}

int lua_selene_output_directory(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    std::string directory=lua_tostring(L,2);
    
    frame->output_directory_std=directory;
    frame->output_directory->ChangeValue(directory);
    
    return 0;
}

int lua_selene_optimization_engine(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    lua_set_metapointer<OptimEngine>(L,"metatable_optimization_engine",&frame->optim_engine);
    
    return 1;
}

std::string SeleneFrame::get_IRF_script_name(Sel::IRF *irf)
{
    int k;
    bool found;
    
    k=vector_locate(found,user_irfs,irf);
                
    if(found) return "irf_"+std::to_string(k);
    else
    {
             if(irf==&irfs[0]) return "SEL_IRF_FRESNEL";
        else if(irf==&irfs[1]) return "SEL_IRF_PERFECT_ABSORBER";
        else if(irf==&irfs[2]) return "SEL_IRF_PERFECT_ANTIREFLECTOR";
        else if(irf==&irfs[3]) return "SEL_IRF_PERFECT_MIRROR";
    }
    
    return "";
}

void SeleneFrame::load_project(wxFileName const &fname_)
{
    if(optimization_running) return;
    
    std::cout<<"\n\nThis: "<<this<<"\n\n";
    
    clear_state();
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    // Lua
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    // Globals allocation
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"bound_class");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[0]));
    lua_setglobal(L,"SEL_IRF_FRESNEL");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[1]));
    lua_setglobal(L,"SEL_IRF_PERFECT_ABSORBER");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[2]));
    lua_setglobal(L,"SEL_IRF_PERFECT_ANTIREFLECTOR");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[3]));
    lua_setglobal(L,"SEL_IRF_PERFECT_MIRROR");
    
    // Allocation functions
    
    lua_register(L,"Selene_IRF",&lua_allocate_selene_IRF);
    lua_register(L,"Selene_light",&lua_allocate_selene_light);
    lua_register(L,"Selene_object",&lua_allocate_selene_object);
    
    // Metatables
    
    // - Materials
    
    lua_gui_material::Loader loader;
    loader.create_metatable(L);
    
    // - Optimization Engine
    
    LuaUI::create_optimization_metatable(L);
    lua_register(L,"Optimizer",&lua_selene_optimization_engine); // Override
    
    // - Simulations parameters
    
    lua_register(L,"MODE",&lua_allocate_SeleneFrame_pointer);
    
    create_obj_metatable(L,"metatable_selene");
    
    metatable_add_func(L,"add_object",&SelGUI::lua_selene_add_object);
    metatable_add_func(L,"add_light",&SelGUI::lua_selene_add_light);
    metatable_add_func(L,"N_rays_disp",&SelGUI::lua_selene_set_N_rays_disp);
    metatable_add_func(L,"N_rays_total",&SelGUI::lua_selene_set_N_rays_total);
    metatable_add_func(L,"render",&null_lua);
    metatable_add_func(L,"output_directory",&SelGUI::lua_selene_output_directory);
    
    // - Selene elements
    
    LuaUI::Selene_create_light_metatable(L);
    LuaUI::Selene_create_object_metatable(L);
    
//    int load_err = luaL_loadfile(L,fname.c_str());
//    
//    if(load_err!=LUA_OK)
//    {
//        std::cout<<"Lua syntax error with "<<fname<<std::endl;
//        std::cin.get();
//        return;
//    }
    
//    lua_pcall(L,0,0,0);
    
    if(luaL_loadfile(L,fname.c_str()) || docall(L, 0, 0))
    {
        std::cerr << lua_tostring(L, -1) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    lua_close(L);
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        frames[i]->consolidate_position();
        
        SeleneVAO *vao=gl->request_vao();
        update_vao(vao,frames[i]);
        
        frames_vao.push_back(vao);
        
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            if(object->name.size()==0)
            {
                object->name="Object_"+std::to_string(item_count);
                item_count++;
            }
        }
        else if(frames[i]->name.size()==0)
        {
            frames[i]->name="Light_"+std::to_string(item_count);
            item_count++;
        }
    }
    
    item_count=frames.size();
    
    //MaterialsLib::consolidate();
    gather_materials();
    
//    for(GUI::Material *m : materials)
//    {
//        std::cout<<m->name<<std::endl;
//    }
    
    rebuild_tree();
}

void SeleneFrame::save_project(wxFileName const &fname_)
{
    std::string fname=fname_.GetFullPath().ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    // Materials
    
    lua_gui_material::Translator mtr("");
    
    for(std::size_t i=0;i<materials.size();i++)
        mtr.gather(materials[i]);
    
    for(std::size_t i=0;i<user_irfs.size();i++)
    {
        Sel::IRF *irf=user_irfs[i];
        
        for(std::size_t j=0;j<irf->ml_materials.size();j++)
        {
            mtr.gather(dynamic_cast<GUI::Material*>(irf->ml_materials[j]));
        }
    }
    
    file<<mtr.get_header()<<"\n";
    
    // Optimization Engine
    
    file<<"optim=Optimizer()\n\n";
        
    // IRFs
    
    for(std::size_t i=0;i<user_irfs.size();i++)
    {
        std::string name="irf_"+std::to_string(i);
        
        file<<name<<"=Selene_IRF()\n";
        
        file<<name<<":type(";
        switch(user_irfs[i]->type)
        {
            case Sel::IRF_GRATING:
                file<<"\"grating\","<<user_irfs[i]->grat_ref_path.generic_string()<<","
                                    <<user_irfs[i]->grat_ref_path.generic_string()<<")\n";
                break;
            case Sel::IRF_MULTILAYER:
                file<<"\"multilayer\")\n";
                
                for(std::size_t j=0;j<user_irfs[i]->ml_heights.size();j++)
                {
                    GUI::Material *mat=dynamic_cast<GUI::Material*>(user_irfs[i]->ml_materials[j]);
                    file<<name<<":add_layer("<<user_irfs[i]->ml_heights[j]<<","<<mtr(mat)<<")\n";
                }
                break;
            case Sel::IRF_SNELL_SPLITTER:
                file<<"\"snell_splitter\","<<user_irfs[i]->splitting_factor<<")\n";
                break;
            default:
                file<<"\"fresnel\")\n";
                break;
        }
        file<<name<<":name(\""<<user_irfs[i]->name<<"\")\n";
        file<<"\n";
    }
    
    // Objects definition
    file<<"-- Objects definition\n\n";
    
    std::vector<std::string> ID(frames.size());
    
    int obj_counter=0;
    int light_counter=0;
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        if(dynamic_cast<Sel::Object*>(frames[i])!=nullptr)
        {
            ID[i]="object_"+std::to_string(obj_counter);
            obj_counter++;
        }
        else
        {
            ID[i]="light_"+std::to_string(light_counter);
            light_counter++;
        }
    }
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            file<<ID[i]<<"=Selene_object(";
            
            switch(object->type)
            {
                case Sel::OBJ_BOX:
                    file<<"\"box\","<<object->box_lx<<","<<object->box_ly<<","<<object->box_lz;
                    break;
                case Sel::OBJ_VOL_CONE:
                    break;
                case Sel::OBJ_CONIC:
                    file<<"\"conic_section\","<<object->conic_R<<","<<object->conic_K<<","<<object->conic_in_radius<<","<<object->conic_out_radius;
                    break;
                case Sel::OBJ_VOL_CYLINDER:
                    file<<"\"cylinder\","<<object->cyl_l<<","<<object->cyl_r<<","<<object->cyl_cut;
                    break;
                case Sel::OBJ_DISK:
                    file<<"\"disk\","<<object->dsk_r<<","<<object->dsk_r_in;
                    break;
                case Sel::OBJ_LENS:
                    file<<"\"lens\","<<object->ls_thickness<<","<<object->ls_r_max_nominal<<","<<object->ls_r1<<","<<object->ls_r2;
                    break;
                case Sel::OBJ_MESH:
                    file<<"\"mesh\",\""<<object->mesh_fname<<"\"";
                    break;
                case Sel::OBJ_RECTANGLE:
                    file<<"\"rectangle\","<<object->box_ly<<","<<object->box_lz;
                    break;
                case Sel::OBJ_PARABOLA:
                    file<<"\"parabola\","<<object->pr_f<<","<<object->pr_in_radius<<","<<object->pr_length;
                    break;
                case Sel::OBJ_SPHERE:
                    file<<"\"sphere\","<<object->sph_r<<","<<object->sph_cut;
                    break;
                case Sel::OBJ_SPHERE_PATCH:
                    file<<"\"spherical_patch\","<<object->sph_r<<","<<object->sph_cut;
                    break;
            }
            
            file<<")\n";
        }
    }
    
    file<<"\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr)
        {
            file<<ID[i]<<"=Selene_light(";
            
            switch(light->type)
            {
                case Sel::SRC_CONE: file<<"\"cone\""; break;
                case Sel::SRC_GAUSSIAN_BEAM: file<<"\"gaussian_beam\""; break;
                case Sel::SRC_LAMBERTIAN: file<<"\"lambertian\""; break;
                case Sel::SRC_POINT: file<<"\"point\""; break;
                case Sel::SRC_POINT_PLANAR: file<<"\"point_planar\""; break;
                case Sel::SRC_PERFECT_BEAM: file<<"\"perfect_beam\""; break;
                case Sel::SRC_USER_DEFINED: file<<"\"user_defined\""; break;
            }
            
            file<<")\n";
        }
    }
    
    file<<"\n";
    
    // Frame properties
    file<<"-- Frame properties\n\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        file<<ID[i]<<":name(\""<<frames[i]->name<<"\")\n";
        
        bool found;
        int k;
        
        if(frames[i]->origin_anchor!=0)
            file<<ID[i]<<":origin("<<frames[i]->get_anchor_script_name(frames[i]->origin_anchor)<<")\n";
        
        if(frames[i]->relative_origin!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->relative_origin);
            file<<ID[i]<<":relative_origin("<<ID[k]<<","
                <<frames[k]->get_anchor_script_name(frames[i]->relative_anchor)<<")\n";
        }
        
        if(frames[i]->translation_frame!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->translation_frame);
            file<<ID[i]<<":translation_frame("<<ID[k]<<")\n";
        }
        
        if(frames[i]->rotation_frame!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->rotation_frame);
            file<<ID[i]<<":rotation_frame("<<ID[k]<<")\n";
        }
        
        file<<ID[i]<<":location("<<frames[i]->in_displacement.x<<","
                                 <<frames[i]->in_displacement.y<<","
                                 <<frames[i]->in_displacement.z<<")\n";
        file<<ID[i]<<":rotation("<<frames[i]->in_A.degree()<<","
                                 <<frames[i]->in_B.degree()<<","
                                 <<frames[i]->in_C.degree()<<")\n";
        
        file<<"\n";
    }
    
    // Specific object properties
    file<<"-- Specific object properties\n\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            int j;
            
            int N=object->get_N_faces();
            Sel::SelFace& (Sel::Object::*face_access)(int)=&Sel::Object::face;
            std::string face_name="face";
            
            if(object->type==Sel::OBJ_MESH)
            {
                N=object->get_N_faces_groups();
                face_access=&Sel::Object::faces_group;
                face_name="faces_group";
            }
            
            // Mesh Specifics
            
            if(object->scaled_mesh)
                file<<ID[i]<<":rescale_mesh("<<object->scaling_factor<<")\n";
            for(std::size_t k=0;k<object->Fg_arr.size();k++)
                file<<ID[i]<<":define_faces_group("<<k<<","<<object->Fg_start[k]<<","<<object->Fg_end[k]<<")\n";
                        
            //#####################
            //   Materials check
            //#####################
            
            // Up Material
            
            GUI::Material *mat=dynamic_cast<GUI::Material*>(object->face(0).up_mat);
            bool out_material_valid=true;
            
            for(j=1;j<object->get_N_faces();j++) if(mat!=object->face(j).up_mat)
                { out_material_valid=false; break; }
            
            if(out_material_valid)
            {
                file<<ID[i]<<":default_out_mat("<<mtr(mat)<<")\n";
            }
            else
            {
                for(j=0;j<N;j++)
                {
                    ::Material *base_mat=(object->*face_access)(j).up_mat;
                    GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
                    
                    file<<ID[i]<<":"<<face_name<<"_up_mat("<<j<<","<<mtr(mat)<<")\n";
                }
            }
            
            // Down Material
            
            mat=dynamic_cast<GUI::Material*>(object->face(0).down_mat);
            bool in_material_valid=true;
            
            for(int j=1;j<object->get_N_faces();j++) if(mat!=object->face(j).down_mat)
                { in_material_valid=false; break; }
            
            if(in_material_valid)
            {
                file<<ID[i]<<":default_in_mat("<<mtr(mat)<<")\n";
            }
            else
            {
                for(j=0;j<N;j++)
                {
                    ::Material *base_mat=(object->*face_access)(j).down_mat;
                    GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
                    
                    file<<ID[i]<<":"<<face_name<<"_down_mat("<<j<<","<<mtr(mat)<<")\n";
                }
            }
            
            //################
            //   IRFs check
            //################
            
            Sel::IRF *irf1=object->face(0).up_irf;
            Sel::IRF *irf2=object->face(0).down_irf;
            
            bool default_IRF_valid=true;
            bool out_IRF_valid=true;
            bool in_IRF_valid=true;
            
            for(j=1;j<object->get_N_faces();j++)
            {
                if(irf1!=object->face(j).up_irf ||
                   irf1!=object->face(j).down_irf)
                {
                    default_IRF_valid=false;
                }
                
                if(irf1!=object->face(j).up_irf) out_IRF_valid=false;
                if(irf2!=object->face(j).down_irf) in_IRF_valid=false;
            }
            
            std::string IRF_ID;
            
            if(default_IRF_valid)
            {
                IRF_ID=get_IRF_script_name(irf1);
                file<<ID[i]<<":default_IRF("<<IRF_ID<<")\n";
            }
            else
            {
                if(out_IRF_valid)
                {
                    IRF_ID=get_IRF_script_name(irf1);
                    file<<ID[i]<<":default_out_IRF("<<IRF_ID<<")\n";
                }
                else
                {
                    for(int j=0;j<N;j++)
                    {
                        IRF_ID=get_IRF_script_name((object->*face_access)(j).up_irf);
                        file<<ID[i]<<":"<<face_name<<"_up_IRF("<<j<<","<<IRF_ID<<")\n";
                    }
                }
                
                if(in_IRF_valid)
                {
                    IRF_ID=get_IRF_script_name(irf2);
                    file<<ID[i]<<":default_in_IRF("<<IRF_ID<<")\n";
                }
                else
                {
                    for(int j=0;j<N;j++)
                    {
                        IRF_ID=get_IRF_script_name((object->*face_access)(j).down_irf);
                        file<<ID[i]<<":"<<face_name<<"_down_IRF("<<j<<","<<IRF_ID<<")\n";
                    }
                }
            }
            
            // Tangents
            
            for(j=0;j<N;j++)
            {
                switch((object->*face_access)(j).tangent_up)
                {
                    case Sel::TANGENT_FIXED:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.x<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.y<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.z<<")\n";
                        break;
                    case Sel::TANGENT_EXPAND:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"expand\")\n";
                        break;
                    case Sel::TANGENT_EXPAND_NEG:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"expand_neg\")\n";
                        break;
                    case Sel::TANGENT_POLAR:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"polar\")\n";
                        break;
                    case Sel::TANGENT_POLAR_NEG:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"polar_neg\")\n";
                        break;
                }
                switch((object->*face_access)(j).tangent_down)
                {
                    case Sel::TANGENT_FIXED:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.x<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.y<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.z<<")\n";
                        break;
                    case Sel::TANGENT_EXPAND:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"expand\")\n";
                        break;
                    case Sel::TANGENT_EXPAND_NEG:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"expand_neg\")\n";
                        break;
                    case Sel::TANGENT_POLAR:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"polar\")\n";
                        break;
                    case Sel::TANGENT_POLAR_NEG:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"polar_neg\")\n";
                        break;
                }
            }
            
            // Sensor properties
            
            if(object->sensor_type!=Sel::Sensor::NONE)
            {
                file<<ID[i]<<":sensor(";
                     if(object->sensor_type==Sel::Sensor::ABS) file<<"\"abs\"";
                else if(object->sensor_type==Sel::Sensor::TRANSP) file<<"\"transp\"";
                
                if(object->sens_wavelength) file<<",\"wavelength\"";
                if(object->sens_source) file<<",\"source\"";
                if(object->sens_generation) file<<",\"generation\"";
                if(object->sens_path) file<<",\"path\"";
                if(object->sens_phase) file<<",\"phase\"";
                if(object->sens_ray_world_intersection) file<<",\"world_intersection\"";
                if(object->sens_ray_world_direction) file<<",\"world_direction\"";
                if(object->sens_ray_obj_intersection) file<<",\"obj_intersection\"";
                if(object->sens_ray_obj_direction) file<<",\"obj_direction\"";
                if(object->sens_ray_obj_face) file<<",\"obj_face\"";
                
                file<<")\n";
            }
            
            // Optimized components
            
            std::map<std::string,double*> &variables_map=object->variables_map;
            
            for(auto [key,var]:variables_map)
            {
                OptimRule rule;
                bool known=optim_engine.get_rule(var,rule);
                
                if(known)
                {
                    file<<"optim:optimize("<<ID[i]<<":reference(\""<<key<<"\"),";
                    
                    switch(rule.operation_type)
                    {
                        case OptimRule::ADD:
                            file<<"OPTIM_ADD,"<<rule.delta_add<<",";
                            break;
                        case OptimRule::GROW:
                            file<<"OPTIM_GROW,"<<rule.delta_grow<<",";
                            break;
                    }
                    file<<rule.limit_down<<","
                        <<rule.limit_up<<",";
                    switch(rule.limit_type)
                    {
                        case OptimRule::NONE: file<<"OPTIM_LIMIT_NONE"; break;
                        case OptimRule::DOWN: file<<"OPTIM_LIMIT_DOWN"; break;
                        case OptimRule::UP: file<<"OPTIM_LIMIT_UP"; break;
                        case OptimRule::BOTH: file<<"OPTIM_LIMIT_BOTH"; break;
                    }
                    file<<","<<static_cast<int>(rule.lock)<<")\n";
                }
            }
            
            file<<"\n";
        }
    }
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr)
        {
            if(light->power!=1.0)
                file<<ID[i]<<":power("<<light->power<<")\n";
            
            // Geometric properties
            
            if(light->type==Sel::SRC_CONE)
            {
                file<<ID[i]<<":full_angle("<<light->cone_angle*180.0/Pi<<")\n";
            }
            else if(light->type==Sel::SRC_GAUSSIAN_BEAM)
            {
                file<<ID[i]<<":numerical_aperture("<<light->beam_numerical_aperture<<")\n";
                file<<ID[i]<<":waist_distance("<<light->beam_waist_distance<<")\n";
            }
            
            // Materials check
            
            GUI::Material *mat=dynamic_cast<GUI::Material*>(light->amb_mat);
            file<<ID[i]<<":material("<<mtr(mat)<<")\n";
            
            // Spectral Properties
            
            if(light->spectrum_type==Sel::SPECTRUM_MONO)
            {
                file<<ID[i]<<":wavelength("<<light->lambda_mono<<")\n";
            }
            else if(light->spectrum_type==Sel::SPECTRUM_POLYMONO)
            {
                file<<ID[i]<<":discrete_spectrum({";
                
                for(std::size_t j=0;j<light->polymono_lambda.size();j++)
                {
                    file<<light->polymono_lambda[j];
                    if(j+1!=light->polymono_lambda.size())
                        file<<",";
                }
                
                file<<"},{";
                
                for(std::size_t j=0;j<light->polymono_weight.size();j++)
                {
                    file<<light->polymono_weight[j];
                    if(j+1!=light->polymono_weight.size())
                        file<<",";
                }
                
                file<<"})\n";
            }
            else if(light->spectrum_type==Sel::SPECTRUM_POLY)
            {
                switch(light->spectrum_shape)
                {
                    case Sel::SPECTRUM_FILE:
                        file<<ID[i]<<":spectrum(\"file\",\""<<light->spectrum_file<<"\")\n";
                        break;
                    case Sel::SPECTRUM_FLAT:
                        file<<ID[i]<<":spectrum(\"flat\","<<light->lambda_min<<","<<light->lambda_max<<")\n";
                        break;
                    case Sel::SPECTRUM_PLANCK:
                        file<<ID[i]<<":spectrum(\"planck\","
                                              <<light->lambda_min<<","
                                              <<light->lambda_max<<","
                                              <<light->planck_temperature<<")\n";
                        break;
                }
            }
            
            // Polarization Properties
            
            switch(light->polar_type)
            {
                case Sel::POLAR_ALONG:
                    file<<ID[i]<<":polar_along("<<light->polar_vector.x<<","
                                                <<light->polar_vector.y<<","
                                                <<light->polar_vector.z<<")\n";
                    break;
                case Sel::POLAR_NOT:
                    file<<ID[i]<<":polar_not("<<light->polar_vector.x<<","
                                              <<light->polar_vector.y<<","
                                              <<light->polar_vector.z<<")\n";
                    break;
            }
            
            // Geometric Properties
            
            switch(light->extent)
            {
                case Sel::EXTENT_CIRCLE:
                    file<<ID[i]<<":extent(\"circle\","<<light->extent_d<<")\n";
                    break;
                case Sel::EXTENT_ELLIPSE:
                    file<<ID[i]<<":extent(\"ellipse\","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_ELLIPSOID:
                    file<<ID[i]<<":extent(\"ellipsoid\","<<light->extent_x<<","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_RECTANGLE:
                    file<<ID[i]<<":extent(\"rectangle\","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_SPHERE:
                    file<<ID[i]<<":extent(\"sphere\","<<light->extent_d<<")\n";
                    break;
            }
                
            file<<"\n";
            
        }
    }
    
    file<<"\n";
    
    // Simulation properties
    file<<"-- Simulation properties\n\n";
    
    file<<"selene=MODE(\"selene\")\n\n";
    file<<"selene:N_rays_total("<<nr_tot->get_value()<<")\n";
    file<<"selene:N_rays_disp("<<nr_disp->get_value()<<")\n\n";
    file<<"selene:output_directory(\""<<output_directory_std.generic_string()<<"\")\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr) file<<"selene:add_object("<<ID[i]<<")\n";
    }
    
    file<<"\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr) file<<"selene:add_light("<<ID[i]<<")\n";
    }
    
    file<<"\nselene:render()";
}
}
