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

#ifndef GUI_MULTILAYERS_H_INCLUDED
#define GUI_MULTILAYERS_H_INCLUDED

#include <multilayers.h>

#include <gui.h>
#include <gui_material.h>
#include <gui_multilayers_utils.h>
#include <gui_panels_list.h>

#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>

class MultilayerFrame;

class MLF_computation_thread: public wxThread
{
    public:
        MultilayerFrame *frame;
        
        MLF_computation_thread();
        ExitCode Entry();
};

class MultilayerFrame: public BaseFrame
{
    public:
        bool spectral;
        double curr_lambda,curr_angle;
        
        Multilayer_TMM_UD ml;
        
        std::vector<double> lambda,angle;
        std::vector<double> lambda_disp,angle_disp;
        std::vector<double> R_TE,R_TM,R_avg,R_TE_sum,R_TM_sum;
        std::vector<double> T_TE,T_TM,T_avg,T_TE_sum,T_TM_sum;
        std::vector<double> A_TE,A_TM,A_avg;
        
        // Statistical computation
        
        bool statistical_computation,thread_running;
        int N_samples,Nc_samples;
        MLF_computation_thread *thread;
        wxMutex computation_mutex;
        
        std::vector<std::vector<double>> layers_height_samples;
        
        // GUI
        
        wxScrolledWindow *structure_panel;
        wxPanel *display_panel;
        
        wxRadioBox *computation_ctrl;
        Graph *graph;
        SliderDisplay *ctrl_slider;
        
        SpectrumSelector *spectrum;
        NamedTextCtrl<int> *angle_ctrl;
        
        bool disp_Rs,disp_Rp,disp_R_avg,
             disp_Ts,disp_Tp,disp_T_avg,
             disp_As,disp_Ap,disp_A_avg;
        
        wxCheckBox *polar_all_ctrl,*polar_S_ctrl, *polar_P_ctrl, *polar_avg_ctrl,
                   *polar_R_ctrl,  *polar_Rs_ctrl,*polar_Rp_ctrl,*polar_R_avg_ctrl,
                   *polar_T_ctrl,  *polar_Ts_ctrl,*polar_Tp_ctrl,*polar_T_avg_ctrl,
                   *polar_A_ctrl,  *polar_As_ctrl,*polar_Ap_ctrl,*polar_A_avg_ctrl;
        
        MiniMaterialSelector *superstrate_selector;
        MiniMaterialSelector *substrate_selector;
        
        wxChoice *add_layer_type;
        PanelsList<LayerPanelBase> *layers_list;
        
        NamedTextCtrl<int> *sampling_ctrl;
        
        int N_layers;
        std::vector<double> layers_height;
        std::vector<GUI::Material*> layers_material;
        
        MultilayerFrame(wxString const &title);
        ~MultilayerFrame();
        
        void end_computation();
        void evt_add_layer(wxCommandEvent &event);
        void evt_angle(wxCommandEvent &event);
        void evt_export_data(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_mode_switch(wxCommandEvent &event);
        void evt_panels_change(wxCommandEvent &event);
        void evt_polar_A(wxCommandEvent &event);
        void evt_polar_all(wxCommandEvent &event);
        void evt_polar_avg(wxCommandEvent &event);
        void evt_polar_P(wxCommandEvent &event);
        void evt_polar_R(wxCommandEvent &event);
        void evt_polar_S(wxCommandEvent &event);
        void evt_polar_switch(wxCommandEvent &event);
        void evt_polar_T(wxCommandEvent &event);
        void evt_refresh(wxCommandEvent &event);
        void evt_reset_project();
        void evt_slider(wxCommandEvent &event);
        void evt_spectrum(wxCommandEvent &event);
        void evt_structure_change(wxCommandEvent &event);
        void load_project(wxFileName const &fname);
        void recompute();
        void recompute_statistical();
        void recompute_statistical_thread();
        void recompute_straight();
        void rename_panels();
        void save_project(wxFileName const &fname);
        void switch_slider_angular();
        void switch_slider_spectral();
        void update_aggregate_polar_ctrl();
};

namespace lua_gui_multilayer
{
    int allocate(lua_State *L);
    
    int add_bragg(lua_State *L);
    int add_layer(lua_State *L);
    int set_angles(lua_State *L);
    int set_spectrum(lua_State *L);
    int set_substrate(lua_State *L);
    int set_superstrate(lua_State *L);
}

#endif // GUI_MULTILAYERS_H_INCLUDED
