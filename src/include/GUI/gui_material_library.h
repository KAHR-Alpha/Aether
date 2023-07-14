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

#ifndef GUI_MATERIAL_LIBRARY_H_INCLUDED
#define GUI_MATERIAL_LIBRARY_H_INCLUDED

#include <material.h>
#include <lua_material.h>

class MaterialsLib;
class MaterialsEditor;

enum class MatType
{
    EFFECTIVE,
    REAL_N,
    LIBRARY,
    SCRIPT,
    USER_LIBRARY,
    CUSTOM
};

class wxWindow;

namespace GUI
{
    class Material: public ::Material
    {
        public:
            MatType type;
            wxWindow *original_requester;
            
            Material();
            
            std::string get_description();
            double get_lambda_validity_min();
            double get_lambda_validity_max();
            std::string get_short_description();
        
        friend MaterialsLib;
    };
}

namespace lua_gui_material
{
    class Loader : public lua_material::Loader
    {
        public:
            Loader();
            ~Loader() = default;
    };
    
    class Translator
    {
        public:
            
            Translator(std::filesystem::path const &relative_path);
            
            void gather(GUI::Material *material);
            std::string get_header();
            std::string operator() (GUI::Material *material) const;
            void save_to_file(GUI::Material *material);
            
        private:
            bool finalized;
            std::filesystem::path const &relative_path;
            std::vector<GUI::Material*> materials;
            
            std::string header;
            std::map<GUI::Material*,std::string> name_map;
            
            void finalize();
            std::string name(GUI::Material *material) const;
            void to_lua(GUI::Material *material,
                        std::ostream &strm,
                        std::string const &prefix);
            void to_lua_custom(GUI::Material *material,
                               std::ostream &strm,
                               std::string const &prefix);
            void to_lua_effective(GUI::Material *material,
                                  std::ostream &strm,
                                  std::string const &prefix);
    };
}



bool default_material_validator(Material *material);

class MaterialsLibDialog: public wxDialog
{
    public:
        bool selection_ok;
        bool new_material;
        GUI::Material *material;
        
        wxWindow *requester;
        GUI::Material *requester_own_material;
        
        wxTreeCtrl *materials;
        
        bool (*accept_material)(Material*); // Validator
        
        MaterialsLibDialog(wxWindow *requester,
                           bool (*validator)(Material*)=&default_material_validator);
        
        void evt_add_to_lib(wxCommandEvent &event);
        void evt_cancel(wxCommandEvent &event);
        void evt_load_script(wxCommandEvent &event);
        void evt_new_const_material(wxCommandEvent &event);
        void evt_new_custom_material(wxCommandEvent &event);
        void evt_new_effective_material(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void rebuild_tree();
};

class MiniMaterialSelector;

class MaterialsLib
{
    public:
        static void consolidate();
        static void consolidate(GUI::Material *material);
        static void consolidate(GUI::Material **material);
        [[nodiscard]]
        static GUI::Material* duplicate_material(GUI::Material *material);
        static void forget_control(MiniMaterialSelector *selector);
        static void initialize();
        static Material* knows_material(unsigned int &n,Material const &material,
                                        bool (*validator)(Material*)=&default_material_validator);
        static void register_control(MiniMaterialSelector *selector);
        [[nodiscard]]
        static GUI::Material* request_material(std::filesystem::path const &path);
        [[nodiscard]]
        static GUI::Material* request_material(MatType type);
    
    protected:
        static void add_to_library(GUI::Material *data);
        static void forget_material(GUI::Material *material);
        static GUI::Material* load_script(std::filesystem::path const &path);
        static GUI::Material* material(std::size_t n);
        static std::size_t size();
        
    private:
        static std::vector<GUI::Material*> data;
        static std::vector<MiniMaterialSelector*> mini_mats;
        
        static void load_material(std::filesystem::path const &fname,MatType type);
        static void write_user_lib();
        static void reorder_materials();
    
    friend MaterialsLibDialog;
};

#endif // GUI_MATERIAL_LIBRARY_H_INCLUDED
