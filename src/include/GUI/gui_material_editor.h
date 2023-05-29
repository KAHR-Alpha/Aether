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

#ifndef GUI_MATERIAL_EDITOR_H_INCLUDED
#define GUI_MATERIAL_EDITOR_H_INCLUDED

#include <gui_material_library.h>

#include <gui_material_editor_panels.h>

wxDECLARE_EVENT(EVT_MATERIAL_EDITOR_MODEL,wxCommandEvent);
wxDECLARE_EVENT(EVT_MATERIAL_EDITOR_SPECTRUM,wxCommandEvent);

class MaterialEditor: public wxPanel
{
    public:
        GUI::Material *material;
        bool read_only_material;
        
        NamedTextCtrl<std::string> *name;
        wxTextCtrl *description;
        WavelengthSelector *validity_min,*validity_max;
        
        wxChoice *model_choice;
        wxButton *add_btn;
        
        PanelsList<MatGUI::SubmodelPanel> *material_elements;
        
        MaterialEditor(wxWindow *parent,GUI::Material *material,bool self_controls);
        
        //
        
        void evt_load(wxCommandEvent &event);
        void evt_reset(wxCommandEvent &event);
        void evt_save(wxCommandEvent &event);
        void evt_save_as(wxCommandEvent &event);
        
        void load();
        void reset();
        bool save();
        bool save_as();
        
        //
        
        void evt_add_model(wxCommandEvent &event);
        void evt_description(wxCommandEvent &event);
        void evt_model_change(wxCommandEvent &event);
        void evt_validity(wxCommandEvent &event);
        void lock();
        void rebuild_elements_list();
        void throw_event_model();
        void throw_event_spectrum();
        void unlock();
        void update_controls();
        
        // Deletion events
        
        void evt_delete_cauchy(wxCommandEvent &event);
        void evt_delete_critpoint(wxCommandEvent &event);
        void evt_delete_debye(wxCommandEvent &event);
        void evt_delete_drude(wxCommandEvent &event);
        void evt_delete_lorentz(wxCommandEvent &event);
        void evt_delete_sellmeier(wxCommandEvent &event);
        void evt_delete_spline(wxCommandEvent &event);
};

#endif // GUI_MATERIAL_EDITOR_H_INCLUDED
