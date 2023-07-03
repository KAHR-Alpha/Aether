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

#ifndef GUI_MATERIAL_H_INCLUDED
#define GUI_MATERIAL_H_INCLUDED

#include <gui.h>
#include <gui_graph.h>
#include <gui_material_editor.h>
#include <gui_material_library.h>
#include <gui_rsc.h>
//#include <phys_tools.h>

#include <wx/splitter.h>

wxDECLARE_EVENT(EVT_MAT_SELECTOR,wxCommandEvent);
wxDECLARE_EVENT(EVT_MINIMAT_SELECTOR,wxCommandEvent);

class MaterialSelector;
class MiniMaterialSelector;
class MaterialsEditor;

class EffMaterialPanel: public PanelsListBase
{
    public:
        MiniMaterialSelector *selector;
        NamedTextCtrl<double> *weight;
        wxCheckBox *host;
        
        EffMaterialPanel(wxWindow *parent,GUI::Material *material,double weight);
        
        void evt_host(wxCommandEvent &event);
        void evt_material(wxCommandEvent &event);
        void hide_host();
        void lock();
        void show_host();
        void unlock();
};

class MaterialSelector: public wxPanel
{
    public:
        wxWindow *parent_selector;
        
        MaterialSelector(wxWindow *parent,std::string name,bool no_box,
                         GUI::Material *material,
                         bool (*validator)(Material*)=&default_material_validator);
        ~MaterialSelector();
        
        GUI::Material* get_material();
        void hide_buttons();
        void hide_description();
        void hide_validity();
        void lock();
        void set_material(GUI::Material *material, bool new_material);
        void unlock();
    
    protected:
        void force_editor_display();
        bool load();
        
    private:
        GUI::Material *material;
        double const_index;
        
        wxPanel *buttons_panel;
        
        // Header
        
        NamedTextCtrl<std::string> *type_description;
        NamedTextCtrl<std::string> *name_ctrl;
        
        wxTextCtrl *description;
        wxPanel *description_panel;
        
        WavelengthSelector *validity_min,*validity_max;
        wxPanel *validity_panel;
        
        // Material Edition
        
        NamedTextCtrl<double> *index_ctrl;
        wxButton *inspect_btn;
        wxTextCtrl *mat_txt;
        
        // Effective Material
        
        wxPanel *eff_panel;
        wxChoice *eff_type;
        wxButton *add_mat_btn;
        PanelsList<EffMaterialPanel> *effective_ctrl;
        
        // Custom
        
        MaterialEditorPanel *custom_editor;
        
        // Validator
        
        bool (*accept_material)(Material*);
        
        void MaterialSelector_CustomPanel(wxWindow *parent);
        void MaterialSelector_EffPanel(wxWindow *parent);
        
        void add_effective_component();
        void evt_add_effective_component(wxCommandEvent &event);
        void evt_const_index(wxCommandEvent &event);
        void evt_custom_material(wxCommandEvent &event);
        void evt_description(wxCommandEvent &event);
        void evt_effective_component(wxCommandEvent &event);
        void evt_effective_host(wxCommandEvent &event);
        void evt_effective_type(wxCommandEvent &event);
        void evt_inspect(wxCommandEvent &event);
        void evt_library(wxCommandEvent &event);
        void evt_name(wxCommandEvent &event);
        void evt_validity(wxCommandEvent &event);
        EffectiveModel get_effective_material_type();
        void rebuild_effective_material();
        void throw_event();
        void update_header();
        void update_layout();
    
    friend MaterialsEditor;
};

class MiniMaterialSelector: public wxPanel
{
    public:
        MiniMaterialSelector(wxWindow *parent,
                             GUI::Material *material=nullptr,
                             std::string const &name="");
        
        ~MiniMaterialSelector();
        
        void copy_material(MiniMaterialSelector *mat);
        Imdouble get_eps(double w);
        Imdouble get_n(double w);
        GUI::Material* get_material();
        void lock();
        void set_material(Material *material);
        void set_material(GUI::Material *material);
        void unlock();
    
    protected:
        GUI::Material *material;
    
    private:
        wxStaticText *mat_txt;
        wxTextCtrl *mat_description;
        wxButton *edit_btn;
        NamedTextCtrl<std::string> *eff_weight;
        
        void evt_edit(wxCommandEvent &event);
        void evt_weight(wxCommandEvent &event);
        void update_display();
        void update_label();
    
    friend MaterialsLib;
};

class MaterialsEditor: public BaseFrame
{
    public:
        unsigned int Np;
        double lambda_min,lambda_max;
        
        std::vector<double> lambda,disp_lambda,disp_real,disp_imag;
        
        // Controls
        
        NamedTextCtrl<std::string> *material_path;
        
        wxButton *material_edit_btn;
        
        wxSplitterWindow *splitter;
        wxScrolledWindow *ctrl_panel;
        MaterialSelector *selector;
        
        // Display
        
        Graph *mat_graph;
        SpectrumSelector *sp_selector;
        wxChoice *disp_choice;
        
        MaterialsEditor(wxString const &title);
        ~MaterialsEditor();
        
        void MaterialsEditor_Controls();
        void MaterialsEditor_Display(wxPanel *display_panel);
        
        void evt_display_choice(wxCommandEvent &event);
        void evt_edit_material(wxCommandEvent &event);
        void evt_material_editor_model(wxCommandEvent &event);
        void evt_material_editor_spectrum(wxCommandEvent &event);
        void evt_material_selector(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_menu_exit();
        void evt_menu_library();
        void evt_menu_save();
        void evt_menu_save_as();
        void evt_spectrum_selector(wxCommandEvent &event);
//        void export_event(wxCommandEvent &event);
        void recompute_model();
};

#endif // GUI_MATERIAL_H_INCLUDED
