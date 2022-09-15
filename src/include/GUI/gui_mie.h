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

#ifndef GUI_MIE_H_INCLUDED
#define GUI_MIE_H_INCLUDED

#include <sstream>
#include <vector>

#include <material.h>
#include <mathUT.h>
#include <mie.h>
#include <phys_tools.h>

#include <gui.h>
#include <gui_gl.h>
#include <gui_material.h>

class MieVAO
{
    public:
        int type;
        GLuint vao;
        
        void draw();
};

//class MieGL:public GL_3D_PVAO<MieVAO>
//{
//    public:
//        
//        MieGL(wxWindow *parent);
//        
//        void hide_particle();
//        void show_particle();
//};

class MieTool: public BaseFrame
{
    public:
        int Nl,Nr;
        bool comp_efficiencies;
        double n_dielec;
        double lambda_min,lambda_max;
        double radius;
        
        std::vector<double> lambda,disp_lambda,C_abs,C_ext,C_scatt,dir_rad;
        
        // Subpanels
        
        wxPanel *cs_panel,*dr_panel,*r2_panel,*r3_panel,*m2_panel,*m3_panel;
        
        // Controls
        
        wxChoice *disp_mode_ctrl;
        
        Graph *cross_graph,*r2_graph;
        
        wxTextCtrl *n_ctrl,
                   *radius_ctrl,
                   *rec_ctrl;
        
        MaterialSelector *mat_selector;
        SpectrumSelector *sp_selector;
        
        wxRadioBox *cs_disp_switch;
        
        wxStaticBoxSizer *dr_angle_sizer;
        NamedTextCtrl<double> *dr_theta,*dr_phi;
        
//        MieGL *gl;
        
        MieTool(wxString const &title);
        ~MieTool();
        
        void load_material(wxCommandEvent &event);
        void recomp_cross_sections();
        void recomp_directional_radiation();
        void show_cs();
        void show_r2();
        void switch_disp_cs(wxCommandEvent &event);
        void switch_disp_mode(wxCommandEvent &event);
        void update_dielec(wxCommandEvent &event);
        void update_directional_radiation_angle(wxCommandEvent &event);
        void update_radius(wxCommandEvent &event);
        void update_spectrum(wxCommandEvent &event);
};

#endif // GUI_MIE_H_INCLUDED
