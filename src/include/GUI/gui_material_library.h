#ifndef GUI_MATERIAL_LIBRARY_H_INCLUDED
#define GUI_MATERIAL_LIBRARY_H_INCLUDED

class MaterialSelector;
class MaterialsManager;

enum class MatType
{
    EFFECTIVE,
    REAL_N,
    COMPLEX_N,
    REAL_EPS,
    COMPLEX_EPS,
    SCRIPT,
    LIBRARY,
    USER_LIBRARY,
    CUSTOM
};

class MaterialData
{
    public:
        Material material;
        MatType type;
        std::filesystem::path path;
};

bool default_material_validator(Material *material);

class MaterialsLibDialog: public wxDialog
{
    public:
        bool selection_ok;
        Material material;
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
        static std::vector<MaterialData*> data;
        
        static bool has_manager();
        static void load_material(std::filesystem::path const &fname,MatType type);
        static void write_user_lib();
        static void reorder_materials();
    public:
        static void add_material(std::filesystem::path const &fname);
        static void add_to_library(MaterialData *data);
        static void forget_manager();
        static MaterialsManager* get_manager();
        static MaterialData* get_material_data(unsigned int n);
        static std::filesystem::path get_material_name(unsigned int n);
        static MatType get_material_type(unsigned int n);
        static std::size_t get_N_materials();
        static void initialize();
        static Material* knows_material(unsigned int &n,Material const &material,
                                        bool (*validator)(Material*)=&default_material_validator);
        static void load_script(std::filesystem::path const &path);
};

#endif // GUI_MATERIAL_LIBRARY_H_INCLUDED
