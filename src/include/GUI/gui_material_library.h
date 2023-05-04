#ifndef GUI_MATERIAL_LIBRARY_H_INCLUDED
#define GUI_MATERIAL_LIBRARY_H_INCLUDED

#include <material.h>
#include <lua_base.h>

class MaterialsLib;
class MaterialsManager;

enum class MatType
{
    EFFECTIVE,
    REAL_N,
    LIBRARY,
    SCRIPT,
    USER_LIBRARY,
    CUSTOM
};

namespace GUI
{
    class Material: public ::Material
    {
        public:
            MatType type;
            
            std::string get_description();
            std::string get_short_description();
            
            double get_lambda_validity_min();
            double get_lambda_validity_max();        
        
        friend MaterialsLib;
    };

    void create_material_metatable(lua_State *L);
    int lua_create_material(lua_State *L);
    int lua_material_set_index(lua_State *L);
    int lua_material_set_name(lua_State *L);
    int lua_material_set_script(lua_State *L);
}

bool default_material_validator(Material *material);

class MaterialsLibDialog: public wxDialog
{
    public:
        bool selection_ok;
        GUI::Material *material;
        wxTreeCtrl *materials;
        
        bool (*accept_material)(Material*); // Validator
        
        MaterialsLibDialog(bool (*validator)(Material*)=&default_material_validator);
        
        void evt_add_to_lib(wxCommandEvent &event);
        void evt_cancel(wxCommandEvent &event);
        void evt_load_script(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void rebuild_tree();
};

class MaterialsLib
{
    private:
        static MaterialsManager *manager;
        static std::vector<GUI::Material*> data;
        
        static bool has_manager();
        static void load_material(std::filesystem::path const &fname,MatType type);
        static void write_user_lib();
        static void reorder_materials();
    public:
        static void add_material(std::filesystem::path const &fname);
        [[deprecated]] static void add_to_library(GUI::Material *data);
        static void consolidate(GUI::Material *material);
        static void forget_manager();
        static MaterialsManager* get_manager();
        static GUI::Material* get_material_data(unsigned int n);
        static std::filesystem::path get_material_path(unsigned int n);
        static MatType get_material_type(unsigned int n);
        static std::size_t get_N_materials();
        static void initialize();
        static Material* knows_material(unsigned int &n,Material const &material,
                                        bool (*validator)(Material*)=&default_material_validator);
        static void load_script(std::filesystem::path const &path);
        [[nodiscard]] static GUI::Material* request_material(MatType type);
};

#endif // GUI_MATERIAL_LIBRARY_H_INCLUDED
