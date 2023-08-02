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

#ifndef LUA_SELENE_H_INCLUDED
#define LUA_SELENE_H_INCLUDED

#include <lua_base.h>
#include <selene.h>

class Selene_Mode: public base_mode
{
    public:
        bool rendered;
        Sel::Selene selene;
        
        Selene_Mode();
        
        void add_object(Sel::Object *object);
        void add_light(Sel::Light *light);
        bool interruption_type() { return true; }
        void process();
        void render();
        void set_N_rays_disp(int Nr_disp);
        void set_N_rays_total(int Nr_tot);
        void set_output_directory(std::string const &output_directory);
};

namespace LuaUI
{
    // Helper Functions
    int selene_diffract_convert(lua_State *L);
    int selene_diffract_merge_polar(lua_State *L);
    
    // Frames
    int create_selene_frame(lua_State *L);
    
    int selene_frame_set_displacement(lua_State *L);
    int selene_frame_set_name(lua_State *L);
    int selene_frame_set_origin(lua_State *L);
    int selene_frame_set_rotation(lua_State *L);
    int selene_frame_set_relative_origin(lua_State *L);
    int selene_frame_set_rotation_frame(lua_State *L);
    int selene_frame_set_translation_frame(lua_State *L);
    
    // Objects
    int create_selene_object(lua_State *L);
    void Selene_create_object_metatable(lua_State *L);
    void create_selene_object_type(lua_State *L,Sel::Object *object);
    
    int selene_object_add_mesh(lua_State *L);
    int selene_object_auto_recalc_normals(lua_State *L);
    int selene_object_contains(lua_State *L);
    int selene_object_get_N_faces(lua_State *L);
    int selene_object_get_variable_reference(lua_State *L);
    int selene_object_rescale_mesh(lua_State *L);
    int selene_object_set_default_in_irf(lua_State *L);
    int selene_object_set_default_in_mat(lua_State *L);
    int selene_object_set_default_irf(lua_State *L);
    int selene_object_set_default_out_irf(lua_State *L);
    int selene_object_set_default_out_mat(lua_State *L);
    
    // - Faces
    
    int selene_object_set_face_down_irf(lua_State *L);
    int selene_object_set_face_down_mat(lua_State *L);
    int selene_object_set_face_down_tangent(lua_State *L);
    int selene_object_set_face_irf(lua_State *L);
    int selene_object_set_face_mat(lua_State *L);
    int selene_object_set_face_up_irf(lua_State *L);
    int selene_object_set_face_up_mat(lua_State *L);
    int selene_object_set_face_up_tangent(lua_State *L);
    
    // - Faces Groups
    
    int selene_object_define_faces_group(lua_State *L);
    int selene_object_set_faces_group_down_irf(lua_State *L);
    int selene_object_set_faces_group_down_mat(lua_State *L);
    int selene_object_set_faces_group_down_tangent(lua_State *L);
    int selene_object_set_faces_group_irf(lua_State *L);
    int selene_object_set_faces_group_mat(lua_State *L);
    int selene_object_set_faces_group_up_irf(lua_State *L);
    int selene_object_set_faces_group_up_mat(lua_State *L);
    int selene_object_set_faces_group_up_tangent(lua_State *L);
    
    int selene_object_set_sensor(lua_State *L);
    
    // IRF
    int create_selene_IRF(lua_State *L);
    int selene_irf_add_layer(lua_State *L);
    int selene_irf_set_name(lua_State *L);
    int selene_irf_set_splitting_factor(lua_State *L);
    int selene_irf_set_type(lua_State *L);
    
    // Sources
    int create_selene_light(lua_State *L);
    void Selene_create_light_metatable(lua_State *L);
    void create_selene_light_type(lua_State *L,Sel::Light *light);
    
    int selene_light_set_angle(lua_State *L);
    int selene_light_set_discrete_spectrum(lua_State *L);
    int selene_light_set_extent(lua_State *L);
    int selene_light_set_material(lua_State *L);
    int selene_light_set_numerical_aperture(lua_State *L);
    int selene_light_set_polar_along(lua_State *L);
    int selene_light_set_polar_not(lua_State *L);
    int selene_light_set_polar_unset(lua_State *L);
    int selene_light_set_power(lua_State *L);
    int selene_light_set_rays_file(lua_State *L);
    int selene_light_set_spectrum(lua_State *L);
    int selene_light_set_wavelength(lua_State *L);
    int selene_light_set_waist_distance(lua_State *L);
    
    // Selene
    
    void Selene_create_allocation_functions(lua_State *L);
    void Selene_create_base_metatable(lua_State *L);
    int selene_mode_add_object(lua_State *L);
    int selene_mode_add_light(lua_State *L);
    int selene_mode_output_directory(lua_State *L);
    int selene_mode_render(lua_State *L);
    int selene_mode_set_N_rays_disp(lua_State *L);
    int selene_mode_set_N_rays_total(lua_State *L);
}

#endif // LUA_SELENE_H_INCLUDED
