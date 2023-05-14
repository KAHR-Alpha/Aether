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

#ifndef GUI_MULTILAYERS_UTILS_H_INCLUDED
#define GUI_MULTILAYERS_UTILS_H_INCLUDED

#include <gui.h>
#include <gui_material.h>
#include <gui_panels_list.h>

wxDECLARE_EVENT(EVT_LAYER_PANEL,wxCommandEvent);

class LayerPanelBase: public PanelsListBase
{
    public:
        bool statistical;
        
        LayerPanelBase(wxWindow *parent)
            :PanelsListBase(parent),
             statistical(false)
        {
        }
        
        virtual wxString get_base_name() { return ""; }
        virtual void get_heights(std::vector<double> &h) { }
        virtual void get_materials(std::vector<Material> &mats) { }
        virtual int get_n_layers() { return 0; }
        virtual wxString get_lua_string() { return ""; }
        bool is_statistical() { return statistical; }
};

class LayerPanel: public LayerPanelBase
{
    public:
        double std_dev;
        LengthSelector *height_ctrl;
        MiniMaterialSelector *selector;
        
        LayerPanel(wxWindow *parent,bool wavy=false);
        LayerPanel(wxWindow *parent,double height,double std_dev,GUI::Material *material,bool wavy);
        
        void evt_waviness(wxCommandEvent &event);
        wxString get_base_name();
        double get_height();
        void get_heights(std::vector<double> &h);
        Material get_material();
        void get_materials(std::vector<Material> &mats);
        int get_n_layers();
        void set_std_dev(double std_dev_);
        wxString get_lua_string();
};

class BraggPanel: public LayerPanelBase
{
    public:
        std::vector<double> height_buffer,shift_buffer;
        
        double std_dev_1,std_dev_2,std_dev_core;
        LengthSelector *height_ctrl_1,*height_ctrl_2,*height_ctrl_core;
        MiniMaterialSelector *selector_1,*selector_2,*selector_core;
        NamedTextCtrl<int> *top_rep_ctrl,*bottom_rep_ctrl;
        LengthSelector *global_std_dev;
        NamedTextCtrl<double> *g_factor;
        
        BraggPanel(wxWindow *parent,
                   double height_1=0,double std_dev_1=0,GUI::Material *material_1=nullptr,
                   double height_2=0,double std_dev_2=0,GUI::Material *material_2=nullptr,
                   double height_core=0,double std_dev_core=0,GUI::Material *material_core=nullptr,
                   double global_std_dev=0,double g_factor=0,int N_top=1,int N_bottom=1);
        
        void check_statistical();
        void evt_periods(wxCommandEvent &event);
        void evt_g_factor(wxCommandEvent &event);
        void evt_global_std_dev(wxCommandEvent &event);
        void evt_waviness(wxCommandEvent &event);
        wxString get_base_name();
        wxString get_lua_string();
        int get_n_layers();
        void get_heights(std::vector<double> &h);
        void get_materials(std::vector<Material> &mats);
};


#endif // GUI_MULTILAYERS_UTILS_H_INCLUDED
