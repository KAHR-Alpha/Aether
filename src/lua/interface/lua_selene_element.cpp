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

namespace LuaUI
{
    // Objects
    int create_selene_object(lua_State *L)
    {
        Sel::Frame **ppf=reinterpret_cast<Sel::Frame**>(lua_newuserdata(L,sizeof(Sel::Frame*)));
    
        luaL_getmetatable(L,"metatable_selene_object");
        lua_setmetatable(L,-2);
        
        Sel::Object *p_object=new Sel::Object;
        *ppf=p_object;
        
        create_selene_object_type(L,p_object);
        
        return 1;
    }
    
    void create_selene_object_type(lua_State *L,Sel::Object *object)
    {
        std::string type=lua_tostring(L,1);
        
             if(type=="boolean_exclusion" ||
                type=="boolean_intersection" ||
                type=="boolean_union")
        {
            Sel::Object *object_1=lua_get_metapointer<Sel::Object>(L,2);
            Sel::Object *object_2=lua_get_metapointer<Sel::Object>(L,3);
            
                 if(type=="boolean_exclusion") object->set_boolean(object_1,object_2,Sel::Object::Boolean_Type::EXCLUDE);
            else if(type=="boolean_intersection") object->set_boolean(object_1,object_2,Sel::Object::Boolean_Type::INTERSECT);
            else if(type=="boolean_union") object->set_boolean(object_1,object_2,Sel::Object::Boolean_Type::UNION);
        }
        else if(type=="box")
        {
            object->set_box(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4));
        }
        else if(type=="cone")
        {
            object->set_cone_volume(lua_tonumber(L,2),lua_tonumber(L,3),1.0);
        }
        else if(type=="conic_section")
        {
            object->conic_R=lua_tonumber(L,2);
            object->conic_K=lua_tonumber(L,3);
            object->conic_in_radius=lua_tonumber(L,4);
            object->conic_out_radius=lua_tonumber(L,5);
            
            object->set_conic_section();
        }
        else if(type=="cylinder")
        {
            object->set_cylinder_volume(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4));
        }
        else if(type=="disk")
        {
            object->set_disk(lua_tonumber(L,2),lua_tonumber(L,3));
        }
        else if(type=="lens")
        {
            object->set_lens(lua_tonumber(L,2),lua_tonumber(L,3),
                                lua_tonumber(L,4),lua_tonumber(L,5));
        }
        else if(type=="mesh")
        {
            std::vector<Sel::Vertex> v_arr;
            std::vector<Sel::SelFace> f_arr;
            
            std::string fname=lua_tostring(L,2);
            
            obj_file_load(fname,v_arr,f_arr);
            
            for(unsigned int i=0;i<f_arr.size();i++)
                f_arr[i].comp_norm(v_arr);
            
            object->mesh_fname=fname;
            object->set_mesh(v_arr,f_arr);
        }
        else if(type=="parabola")
        {
            object->set_parabola(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4));
        }
        else if(type=="rectangle")
        {
            object->set_rectangle(lua_tonumber(L,2),lua_tonumber(L,3));
        }
        else if(type=="sphere")
        {
            object->set_sphere(lua_tonumber(L,2),lua_tonumber(L,3));
        }
        else if(type=="spherical_patch")
        {
            object->set_spherical_patch(lua_tonumber(L,2),lua_tonumber(L,3));
        }
        else
        {
            std::cout<<"Unrecognized object type: "<<type<<std::endl;
            std::cin.get();
            std::exit(0);
        }
    }
    
    Sel::Object * get_object_cast_metapointer(lua_State *L,int index=1)
    {
        Sel::Frame *p_frame=lua_get_metapointer<Sel::Frame>(L,index);
        Sel::Object *p_object=dynamic_cast<Sel::Object*>(p_frame);
        
        return p_object;
    }
    
    int selene_object_add_mesh(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        if(lua_isstring(L,2))
        {
            std::vector<Sel::Vertex> v_arr;
            std::vector<Sel::SelFace> f_arr;
            
            obj_file_load(lua_tostring(L,2),v_arr,f_arr);
            
            for(unsigned int i=0;i<f_arr.size();i++)
                f_arr[i].comp_norm(v_arr);
            
            p_object->add_mesh(v_arr,f_arr);
        }
        else if(lua_islightuserdata(L,2))
        {
            Sel::Object *p_source=get_object_cast_metapointer(L,2);
            
            p_object->add_mesh(p_source->V_arr,p_source->F_arr);
        }
        
        return 0;
    }
    
    int selene_object_auto_recalc_normals(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        p_object->auto_recalc_normals();
        
        return 0;
    }
    
    int selene_object_contains(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        double x=lua_tonumber(L,2);
        double y=lua_tonumber(L,3);
        double z=lua_tonumber(L,4);
        
        if(p_object->contains(x,y,z)) lua_pushinteger(L,1);
        else lua_pushinteger(L,0);
        
        return 1;
    }
    
    int selene_object_get_variable_reference(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        std::string variable_name=lua_tostring(L,2);
        
        double *variable=p_object->reference_variable(variable_name);
        
        lua_pushlightuserdata(L,static_cast<void*>(variable));
        
        return 1;
    }
    
    int selene_object_get_N_faces(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        lua_pushinteger(L,p_object->get_N_faces());
        
        return 1;
    }
    
    int selene_object_rescale_mesh(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        p_object->rescale_mesh(lua_tonumber(L,2));
        
        return 1;
    }
    
    int selene_object_set_default_in_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,2))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,2));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,2);
        
        p_object->set_default_in_irf(p_irf);
        
        return 0;
    }
    
    int selene_object_set_default_in_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Material *p_mat=lua_get_metapointer<Material>(L,2);
        
        chk_var(p_mat);
        
        p_object->set_default_in_mat(p_mat);
        
        return 0;
    }
    
    int selene_object_set_default_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,2))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,2));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,2);
        
        p_object->set_default_irf(p_irf);
        
        return 0;
    }
    
    int selene_object_set_default_out_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,2))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,2));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,2);
        
        p_object->set_default_out_irf(p_irf);
        
        return 0;
    }
    
    int selene_object_set_default_out_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Material *p_mat=lua_get_metapointer<Material>(L,2);
        
        p_object->set_default_out_mat(p_mat);
        
        return 0;
    }
    
    ///##################
    ///   Faces Access
    ///##################
    
    int selene_object_set_face_down_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.down_irf=p_irf;
        
        return 0;
    }
    
    int selene_object_set_face_down_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        face.down_mat=lua_get_metapointer<Material>(L,3);
        
        return 0;
    }
    
    int selene_object_set_face_down_tangent(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        
        if(lua_gettop(L)>3)
        {
            face.tangent_down=Sel::TANGENT_FIXED;
            face.fixed_tangent_down.x=lua_tonumber(L,3);
            face.fixed_tangent_down.y=lua_tonumber(L,4);
            face.fixed_tangent_down.z=lua_tonumber(L,5);
        }
        else
        {
            std::string str=lua_tostring(L,3);
            
                 if(str=="expand") face.tangent_down=Sel::TANGENT_EXPAND;
            else if(str=="expand_neg") face.tangent_down=Sel::TANGENT_EXPAND_NEG;
            else if(str=="polar") face.tangent_down=Sel::TANGENT_POLAR;
            else if(str=="polar_neg") face.tangent_down=Sel::TANGENT_POLAR_NEG;
        }
        
        return 0;
    }
    
    int selene_object_set_face_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.down_irf=p_irf;
        face.up_irf=p_irf;
        
        return 0;
    }
    
    int selene_object_set_face_up_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.up_irf=p_irf;
        
        return 0;
    }
    
    int selene_object_set_face_up_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        face.up_mat=lua_get_metapointer<Material>(L,3);
        
        return 0;
    }
    
    int selene_object_set_face_up_tangent(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->face(lua_tointeger(L,2));
        
        if(lua_gettop(L)>3)
        {
            face.tangent_up=Sel::TANGENT_FIXED;
            face.fixed_tangent_up.x=lua_tonumber(L,3);
            face.fixed_tangent_up.y=lua_tonumber(L,4);
            face.fixed_tangent_up.z=lua_tonumber(L,5);
        }
        else
        {
            std::string str=lua_tostring(L,3);
            
                 if(str=="expand") face.tangent_up=Sel::TANGENT_EXPAND;
            else if(str=="expand_neg") face.tangent_up=Sel::TANGENT_EXPAND_NEG;
            else if(str=="polar") face.tangent_up=Sel::TANGENT_POLAR;
            else if(str=="polar_neg") face.tangent_up=Sel::TANGENT_POLAR_NEG;
        }
        
        return 0;
    }
    
    ///#########################
    ///   Faces Groups Access
    ///#########################
    
    int selene_object_define_faces_group(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        int index=lua_tointeger(L,2);
        int start=lua_tointeger(L,3);
        int end=lua_tointeger(L,4);
        
        p_object->define_faces_group(index,start,end);
        
        return 0;
    }
    
    int selene_object_set_faces_group_down_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.down_irf=p_irf;
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_down_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        face.down_mat=lua_get_metapointer<Material>(L,3);
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_down_tangent(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        
        if(lua_gettop(L)>3)
        {
            face.tangent_down=Sel::TANGENT_FIXED;
            face.fixed_tangent_down.x=lua_tonumber(L,3);
            face.fixed_tangent_down.y=lua_tonumber(L,4);
            face.fixed_tangent_down.z=lua_tonumber(L,5);
        }
        else
        {
            std::string str=lua_tostring(L,3);
            
                 if(str=="expand") face.tangent_down=Sel::TANGENT_EXPAND;
            else if(str=="expand_neg") face.tangent_down=Sel::TANGENT_EXPAND_NEG;
            else if(str=="polar") face.tangent_down=Sel::TANGENT_POLAR;
            else if(str=="polar_neg") face.tangent_down=Sel::TANGENT_POLAR_NEG;
        }
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.down_irf=p_irf;
        face.up_irf=p_irf;
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_up_irf(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        
        Sel::IRF *p_irf;
        
        if(lua_islightuserdata(L,3))p_irf=reinterpret_cast<Sel::IRF*>(lua_touserdata(L,3));
        else p_irf=lua_get_metapointer<Sel::IRF>(L,3);
        
        face.up_irf=p_irf;
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_up_mat(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        face.up_mat=lua_get_metapointer<Material>(L,3);
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_faces_group_up_tangent(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        Sel::SelFace &face=p_object->faces_group(lua_tointeger(L,2));
        
        if(lua_gettop(L)>3)
        {
            face.tangent_up=Sel::TANGENT_FIXED;
            face.fixed_tangent_up.x=lua_tonumber(L,3);
            face.fixed_tangent_up.y=lua_tonumber(L,4);
            face.fixed_tangent_up.z=lua_tonumber(L,5);
        }
        else
        {
            std::string str=lua_tostring(L,3);
            
                 if(str=="expand") face.tangent_up=Sel::TANGENT_EXPAND;
            else if(str=="expand_neg") face.tangent_up=Sel::TANGENT_EXPAND_NEG;
            else if(str=="polar") face.tangent_up=Sel::TANGENT_POLAR;
            else if(str=="polar_neg") face.tangent_up=Sel::TANGENT_POLAR_NEG;
        }
        
        p_object->propagate_faces_group(lua_tointeger(L,2));
        
        return 0;
    }
    
    int selene_object_set_sensor(lua_State *L)
    {
        Sel::Object *p_object=get_object_cast_metapointer(L);
        
        int N=lua_gettop(L)-1;
        
        for(int i=0;i<N;i++)
        {
            std::string str=lua_tostring(L,i+2);
            
                 if(str=="abs") p_object->set_sens_abs();
            else if(str=="transp") p_object->set_sens_transp();
            else if(str=="wavelength") p_object->sens_wavelength=true;
            else if(str=="source") p_object->sens_source=true;
            else if(str=="generation") p_object->sens_generation=true;
            else if(str=="path") p_object->sens_path=true;
            else if(str=="length") p_object->sens_length=true;
            else if(str=="phase") p_object->sens_phase=true;
            else if(str=="world_intersection") p_object->sens_ray_world_intersection=true;
            else if(str=="world_direction") p_object->sens_ray_world_direction=true;
            else if(str=="obj_intersection") p_object->sens_ray_obj_intersection=true;
            else if(str=="obj_direction") p_object->sens_ray_obj_direction=true;
            else if(str=="obj_face") p_object->sens_ray_obj_face=true;
        }
        
        return 0;
    }
}
