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

#ifndef GUI_GEOMETRIC_GRADIENT_H_INCLUDED
#define GUI_GEOMETRIC_GRADIENT_H_INCLUDED

#include <multilayers.h>

#include <gui.h>
#include <gui_gl_fd.h>
#include <gui_material.h>

#include <wx/wx.h>
#include <wx/splitter.h>

enum
{
    LAT_SQUARE,
    LAT_HEXA,
    PROF_LINEAR,
    PROF_S_CURVE,
    PROF_GAUSSIAN,
    PROF_BEZIER
};

class GeoGradientHolder
{
    public:
        int lattice,profile;
        double height,periodicity,top_radius,bottom_radius;
        
        GeoGradientHolder();
        
        double get_ratio(double z);
        double get_z(double x,double y);
};

class GeoGradientDesigner: public wxDialog
{
    public:
        GeoGradientHolder *holder;
        
        LengthSelector *height_ctrl,*periodicity_ctrl,*top_radius_ctrl,*bottom_radius_ctrl;
        wxRadioBox *lattice_ctrl,*prof_ctrl;
        
        Graph *graph;
        
        std::vector<double> prof_x,prof_val;
        
        GeoGradientDesigner(GeoGradientHolder *holder,wxPoint const &pos,wxSize const &size);
        ~GeoGradientDesigner();
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_profile(wxCommandEvent &event);
        void recompute();
};

class GeoGradientFrame: public BaseFrame
{
    public:
        bool spectral;
        double curr_lambda,curr_angle;
        
        Multilayer_TMM_UD ml;
        
        wxRadioBox *computation_ctrl;
        
        GeoGradientHolder profile_holder;
        SpectrumSelector *spectrum;
        
        MiniMaterialSelector *substrate,*superstrate;
        
        NamedTextCtrl<int> *angles_ctrl,*layers_ctrl;
        
        wxCheckBox *max_gar_1_ctrl,
                   *max_gar_2_ctrl,
                   *bruggeman_ctrl,
                   *looyenga_ctrl,
                   *sum_ctrl,
                   *sum_inv_ctrl;
        
        Graph *graph;
        SliderDisplay *ctrl_slider;
        
        std::vector<double> lambda,angle,
                            min_R_TE,max_R_TE,min_T_TE,max_T_TE,
                            min_R_TM,max_R_TM,min_T_TM,max_T_TM;
        std::vector<AngleOld> angle_rad;
        
        GeoGradientFrame(wxString const &title);
        
        void evt_angles(wxCommandEvent &event);
        void evt_export(wxCommandEvent &event);
        void evt_export_index(wxCommandEvent &event);
        void evt_layers(wxCommandEvent &event);
        void evt_mode_switch(wxCommandEvent &event);
        void evt_models(wxCommandEvent &event);
        void evt_set_profile(wxCommandEvent &event);
        void evt_slider(wxCommandEvent &event);
        void evt_spectrum(wxCommandEvent &event);
        void evt_structure_change(wxCommandEvent &event);
        void recompute();
        void switch_slider_angular();
        void switch_slider_spectral();
};


#endif // GUI_GEOMETRIC_GRADIENT_H_INCLUDED
