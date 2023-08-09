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

#ifndef GUI_SELENE_SENSOR_H_INCLUDED
#define GUI_SELENE_SENSOR_H_INCLUDED

#include <gui.h>
#include <gui_gl_graph.h>
#include <selene.h>

namespace SelGUI
{

class RayCounterFrame: public BaseFrame
{
    public:
        bool linked;
        Sel::RayCounter *counter;
        
        wxPanel *left_panel;
        wxScrolledWindow *ctrl_panel;
        wxBoxSizer *ctrl_sizer;
        
        NamedTextCtrl<int> *face_number;
        
        std::vector<NamedTextCtrl<int>*> x_resolution,y_resolution;
        
        // Computation type
        wxStaticBoxSizer *computation_sizer;
        wxChoice *computation_type;
        
        // Spectral filter
        wxStaticBoxSizer *spectral_sizer;
        wxChoice *spectral_mode;
        WavelengthSelector *lambda_min,*lambda_max,
                           *lambda_center,*lambda_width;
        
        GL_2D_graph *display;
        
        wxTimer *timer;
        
        RayCounterFrame(wxString const &title,Sel::RayCounter *counter=nullptr);
        ~RayCounterFrame();
        
        void allocate_face_controls();
        void evt_computation_type(wxCommandEvent &event);
        void evt_export(wxCommandEvent &event);
        void evt_face_switch(wxCommandEvent &event);
        void evt_face_switch_text(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_resolution(wxCommandEvent &event);
        void evt_spectral_filter(wxCommandEvent &event);
        void evt_update(wxCommandEvent &event);
        void load_project(wxFileName const &fname);
        void update_graph();
        void update_face_controls();
};

}

#endif // GUI_SELENE_SENSOR_H_INCLUDED
