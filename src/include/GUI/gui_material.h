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

#ifndef GUI_MATERIAL_H_INCLUDED
#define GUI_MATERIAL_H_INCLUDED

#include <material.h>

#include <gui.h>
#include <gui_graph.h>
#include <gui_panels_list.h>
#include <gui_rsc.h>
//#include <phys_tools.h>

wxDECLARE_EVENT(EVT_MAT_SELECTOR,wxCommandEvent);
wxDECLARE_EVENT(EVT_MINIMAT_SELECTOR,wxCommandEvent);

enum
{
    GUI_MAT_EFFECTIVE,
    GUI_MAT_REAL_N,
    GUI_MAT_COMPLEX_N,
    GUI_MAT_REAL_EPS,
    GUI_MAT_COMPLEX_EPS,
    GUI_MAT_SCRIPT,
    GUI_MAT_LIBRARY
};

class MaterialSelector;

bool default_material_validator(Material *material);

class MaterialsLib
{
    private:
        static int Nm;
        static std::vector<std::filesystem::path> mat_fname;
        static std::vector<Material*> mat_arr;
        static std::vector<bool> user_material;
        static std::vector<MaterialSelector*> selector_arr;
        
        static void insert_material(int pos,std::filesystem::path const &fname,Material *material,bool user_material=true);
        static void load_material(std::filesystem::path const &fname,bool user_material=true);
    public:
        static void add_material(std::filesystem::path const &fname);
        static Material* get_material(unsigned int n);
        static Material* knows_material(unsigned int &n,Material const &material,
                                        bool (*validator)(Material*)=&default_material_validator);
        static std::filesystem::path get_material_name(unsigned int n);
        static int get_N_materials();
        static void initialize();
        static void register_material_selector(MaterialSelector *selector);
        static void remove_material_selector(MaterialSelector *selector);
};

class MaterialSelector: public wxPanel
{
    public:
        int mat_type;
        double const_index,weight;
        std::filesystem::path script;
        
        Material script_model,library_model;
        
        wxChoice *mat_type_ctrl,
                 *library_list_ctrl;
        wxButton *load_btn,*add_lib_btn,*inspect_btn;
        wxTextCtrl *mat_txt;
        
        // Effective Material
        
        Material eff_material;
        wxPanel *eff_panel;
        wxChoice *eff_type;
        wxBoxSizer *eff_sizer;
        MaterialSelector *eff_mat_1_selector,
                         *eff_mat_2_selector;
        NamedTextCtrl<double> *eff_weight;
        
        // Validator
        
        bool (*accept_material)(Material*);
        
        MaterialSelector(wxWindow *parent,std::string name,bool no_box=false,
                         bool (*validator)(Material*)=&default_material_validator);
        MaterialSelector(wxWindow *parent,std::string name,bool no_box,Material const &material,
                         bool (*validator)(Material*)=&default_material_validator);
        void MaterialSelector_LibraryList(wxWindow *parent);
        void MaterialSelector_EffPanel(wxWindow *parent);
        ~MaterialSelector();
        
        void allocate_effective_materials();
        void allocate_effective_materials(Material const &eff_mat_1,
                                          Material const &eff_mat_2);
//        virtual bool accept_material(Material *material);
        void const_index_event(wxCommandEvent &event);
        void evt_add_to_library(wxCommandEvent &event);
        void evt_const_index_focus(wxFocusEvent &event);
        void evt_effective_material(wxCommandEvent &event);
        void evt_inspect(wxCommandEvent &event);
        void evt_load(wxCommandEvent &event);
        void evt_mat_list(wxCommandEvent &event);
        void evt_mat_type(wxCommandEvent &event);
        Imdouble get_eps(double w);
        int get_effective_material_type();
        std::string get_lua();
        Material get_material();
        wxString get_name();
        wxString get_title();
        int get_type();
        double get_lambda_validity_min();
        double get_lambda_validity_max();
        double get_weight();
        void layout_const();
        void layout_effective();
        void layout_library();
        void layout_script();
        void operator = (MaterialSelector const &selector);
        void set_const_model(double n);
        void set_library_model(Material *material);
        void throw_event();
        void update_library_list();
};

class MiniMaterialSelector: public wxPanel
{
    public:
        int mat_type;
        wxGenericStaticBitmap *mat_bmp;
        wxStaticText *mat_txt;
        wxTextCtrl *mat_name;
        wxButton *edit_btn;
        NamedTextCtrl<double> *eff_weight;
        
        Material material;
        
        MiniMaterialSelector(wxWindow *parent,
                             Material const &material,
                             std::string const &name="");
        
        MiniMaterialSelector(wxWindow *parent,
                             std::string const &name="",
                             std::filesystem::path const &script_fname="");
        
        void copy_material(MiniMaterialSelector *mat);
        void evt_edit(wxCommandEvent &event);
        void evt_weight(wxCommandEvent &event);
        Imdouble get_eps(double w);
        wxString get_lua();
        Imdouble get_n(double w);
        Material& get_material();
        void set_material(std::filesystem::path const &script_fname);
        void update_label();
};

class MaterialExplorer: public BaseFrame
{
    public:
        unsigned int Np;
        double lambda_min,lambda_max;
        
        std::vector<double> lambda,disp_lambda,disp_real,disp_imag;
        
        wxChoice *disp_choice;
        
        MaterialSelector *mat_selector;
        SpectrumSelector *sp_selector;
        
        Graph *mat_graph;
        
        MaterialExplorer(wxString const &title);
        MaterialExplorer(double lambda_min,double lambda_max,int Np,MaterialSelector *selector=nullptr);
        
        void disp_choice_event(wxCommandEvent &event);
        void export_event(wxCommandEvent &event);
        void material_selector_event(wxCommandEvent &event);
        void spectrum_selector_event(wxCommandEvent &event);
        void recompute_model();
};

namespace MatGUI
{
    class SubmodelPanel: public PanelsListBase
    {
        public:
            Material *material;
            
            SubmodelPanel(wxWindow *parent);
            
            void evt_modification(wxCommandEvent &event);
            virtual void gui_to_mat();
    };
    
    class EpsInfPanel: public SubmodelPanel
    {
        public:
            double *mat_eps_inf;
            NamedTextCtrl<double> *eps_inf;
            
            EpsInfPanel(wxWindow *parent,double *eps_inf);
            
            void gui_to_mat();
    };
    
    class DrudePanel: public SubmodelPanel
    {
        public:
            int ID;
            DrudeModel *mat_drude;
            WavelengthSelector *wd,*g;
            
            DrudePanel(wxWindow *parent,DrudeModel *drude,int ID);
            
            void gui_to_mat();
    };
    
    class LorentzPanel: public SubmodelPanel
    {
        public:
            int ID;
            LorentzModel *mat_lorentz;
            
            NamedTextCtrl<double> *A;
            WavelengthSelector *O,*G;
            
            LorentzPanel(wxWindow *parent,LorentzModel *lorentz,int ID);
            
            void gui_to_mat();
    };
    
    class CritpointPanel: public SubmodelPanel
    {
        public:
            int ID;
            CritpointModel *mat_critpoint;
            
            NamedTextCtrl<double> *A,*P;
            WavelengthSelector *O,*G;
            
            CritpointPanel(wxWindow *parent,CritpointModel *critpoint,int ID);
            
            void gui_to_mat();
    };
    
    class CauchyPanel: public SubmodelPanel
    {
        public:
            int ID;
            std::vector<double> *mat_coeffs;
            
            NamedTextCtrl<int> *order;
            std::vector<NamedTextCtrl<double>*> coeffs;
            
            CauchyPanel(wxWindow *parent,std::vector<double> *coeffs,int ID);
            
            void evt_order(wxCommandEvent &event);
            void gui_to_mat();
    };
    
    class SellmeierPanel: public SubmodelPanel
    {
        public:
            int ID;
            double *mat_B,*mat_C;
            
            NamedTextCtrl<double> *B;
            WavelengthSelector *C;
            
            SellmeierPanel(wxWindow *parent,double *B,double *C,int ID);
            
            void gui_to_mat();
    };
    
    class DataPanel: public SubmodelPanel
    {
        public:
            int ID;
            std::vector<double> *lambda;
            std::vector<double> *data_r;
            std::vector<double> *data_i;
            char *index_type;
            
            Cspline *er_spline,*ei_spline;
            
            DataPanel(wxWindow *parent,int ID,
                      std::vector<double> *lambda,
                      std::vector<double> *data_r,
                      std::vector<double> *data_i,
                      char *index_type,
                      Cspline *er_spline,
                      Cspline *ei_spline);
            
            void evt_edit(wxCommandEvent &event);
            void gui_to_mat();
    };
}

class MaterialEditor: public wxPanel
{
    public:
        Material material;
        
        wxTextCtrl *description;
        WavelengthSelector *validity_min,*validity_max;
        
        wxChoice *model_choice;
        
        PanelsList<MatGUI::SubmodelPanel> *material_elements;
        
        MaterialEditor(wxWindow *parent);
        
        void evt_add_model(wxCommandEvent &event);
        void evt_description(wxCommandEvent &event);
        void evt_model_change(wxCommandEvent &event);
        void evt_validity(wxCommandEvent &event);
        void rebuild_elements_list();
        void throw_event_model();
        void throw_event_spectrum();
        void update_controls();
};

class MaterialManager: public BaseFrame
{
    public:
        unsigned int Np;
        double lambda_min,lambda_max;
        
        bool library_material;
        
        std::vector<double> lambda,disp_lambda,disp_real,disp_imag;
        
        // Controls
        
        NamedTextCtrl<std::string> *material_path;
        
        wxScrolledWindow *ctrl_panel;
        
        MaterialEditor *editor;
        
        // Display
        
        Graph *mat_graph;
        SpectrumSelector *sp_selector;
        wxChoice *disp_choice;
        
        MaterialManager(wxString const &title);
        
        void MaterialManager_Controls();
        void MaterialManager_Display(wxPanel *display_panel);
        
        void evt_display_choice(wxCommandEvent &event);
        void evt_material_editor_model(wxCommandEvent &event);
        void evt_material_editor_spectrum(wxCommandEvent &event);
//        void evt_material_selector(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_menu_exit();
        void evt_menu_load();
        void evt_menu_new();
        void evt_menu_save();
        void evt_menu_save_as();
        void evt_spectrum_selector(wxCommandEvent &event);
//        void export_event(wxCommandEvent &event);
        void recompute_model();
};

class MaterialsListSub: public PanelsListBase
{
    public:
        MaterialSelector *material;
        
        MaterialsListSub(wxWindow *parent);
        MaterialsListSub(wxWindow *parent,Material const &material);
};

class MaterialsListDialog: public wxDialog
{
    public:
        bool list_changed;
        std::vector<Material> *materials;
        
        wxScrolledWindow *mats_panel;
        PanelsList<> *materials_list;
        
        MaterialsListDialog(std::vector<Material> &materials);
        ~MaterialsListDialog();
        
        void evt_delete_material(wxCommandEvent &event);
        void evt_new_material(wxCommandEvent &event);
        void evt_material(wxCommandEvent &event);
        void evt_materials_reorder(wxCommandEvent &event);
        void materials_renumber();
};

#endif // GUI_MATERIAL_H_INCLUDED
