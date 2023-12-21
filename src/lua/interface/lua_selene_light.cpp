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
    // Sources
    int create_selene_light(lua_State *L)
    {
        Sel::Light *p_light=lua_allocate_metapointer<Sel::Light>(L,"metatable_selene_light");
        
        create_selene_light_type(L,p_light);
        
        return 1;
    }
    
    void create_selene_light_type(lua_State *L,Sel::Light *p_light)
    {
        std::string type=lua_tostring(L,1);
        
             if(type=="cone") p_light->set_type(Sel::SRC_CONE);
        else if(type=="gaussian_beam") p_light->set_type(Sel::SRC_GAUSSIAN_BEAM);
        else if(type=="lambertian") p_light->set_type(Sel::SRC_LAMBERTIAN);
        else if(type=="perfect_beam") p_light->set_type(Sel::SRC_PERFECT_BEAM);
        else if(type=="point") p_light->set_type(Sel::SRC_POINT);
        else if(type=="point_planar") p_light->set_type(Sel::SRC_POINT_PLANAR);
        else if(type=="user_defined") p_light->set_type(Sel::SRC_USER_DEFINED);
    }
    
    int selene_light_set_angle(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->cone_angle=lua_tonumber(L,2)*Pi/180.0;
        
        return 0;
    }
    
    int selene_light_set_discrete_spectrum(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        std::vector<double> lambda,weight;
        
        lua_pushnil(L);
        while(lua_next(L,2)!=0)
        {
            lambda.push_back(lua_tonumber(L,-1));
            lua_pop(L,1);
        }
        
        lua_pushnil(L);
        while(lua_next(L,3)!=0)
        {
            weight.push_back(lua_tonumber(L,-1));
            lua_pop(L,1);
        }
        
        light->spectrum_type=Sel::SPECTRUM_POLYMONO;
        light->polymono_lambda=lambda;
        light->polymono_weight=weight;
        
        return 0;
    }
    
    int selene_light_set_extent(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        std::string type=lua_tostring(L,2);
        
        if(type=="circle")
        {
            light->extent=Sel::EXTENT_CIRCLE;
            light->extent_d=lua_tonumber(L,3);
        }
        else if(type=="ellipse")
        {
            light->extent=Sel::EXTENT_ELLIPSE;
            light->extent_y=lua_tonumber(L,3);
            light->extent_z=lua_tonumber(L,4);
        }
        else if(type=="ellipsoid")
        {
            light->extent=Sel::EXTENT_ELLIPSOID;
            light->extent_x=lua_tonumber(L,3);
            light->extent_y=lua_tonumber(L,4);
            light->extent_z=lua_tonumber(L,5);
        }
        else if(type=="rectangle")
        {
            light->extent=Sel::EXTENT_RECTANGLE;
            light->extent_y=lua_tonumber(L,3);
            light->extent_z=lua_tonumber(L,4);
        }
        else if(type=="sphere")
        {
            light->extent=Sel::EXTENT_SPHERE;
            light->extent_d=lua_tonumber(L,3);
        }
        
        return 0;
    }
    
    int selene_light_set_material(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        Material *p_mat=lua_get_metapointer<Material>(L,2);
        
        light->amb_mat=p_mat;
        
        return 0;
    }
    
    int selene_light_set_numerical_aperture(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->beam_numerical_aperture=lua_tonumber(L,2);
        
        return 0;
    }
    
    int selene_light_set_polar_along(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->polar_type=Sel::POLAR_ALONG;
        light->polar_vector(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4));
        
        return 0;
    }
    
    int selene_light_set_polar_not(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->polar_type=Sel::POLAR_NOT;
        light->polar_vector(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4));
        
        return 0;
    }
    
    int selene_light_set_polar_unset(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->polar_type=Sel::POLAR_UNSET;
        light->polar_vector(0,0,0);
        
        return 0;
    }
    
    int selene_light_set_power(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->power=lua_tonumber(L,2);
        
        return 0;
    }
    
    int selene_light_set_spectrum(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        std::string shape=lua_tostring(L,2);
        chk_var(shape);
        
        if(shape=="file") light->set_spectrum_file(lua_tostring(L,3));
        else if(shape=="flat") 
        {
            light->set_spectrum_flat(lua_tonumber(L,3),lua_tonumber(L,4));
        }
        else if(shape=="planck")
        {
            light->set_spectrum_planck(lua_tonumber(L,3),
                                       lua_tonumber(L,4),
                                       lua_tonumber(L,5));
        }
        
        return 0;
    }
    
    int selene_light_set_rays_file(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->ray_file.initialize(lua_tostring(L,2));
        
        if(!light->ray_file.file_ok)
        {
            std::cerr<<"Invalid ray file: "<<lua_tostring(L,2)<<std::endl;
            std::cerr<<"Aborting..."<<std::endl;
            std::exit(EXIT_FAILURE);
        }
        
        return 0;
    }
    
    int selene_light_set_wavelength(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        double lambda=lua_tonumber(L,2);
        
        light->spectrum_type=Sel::SPECTRUM_MONO;
        light->lambda_mono=lambda;
        
        return 0;
    }
    
    int selene_light_set_waist_distance(lua_State *L)
    {
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,1);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
        
        light->beam_waist_distance=lua_tonumber(L,2);
        
        return 0;
    }
}
