/*Copyright 2008-2021 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GUI_PARAMETRIC_INTERP_H_INCLUDED
#define GUI_PARAMETRIC_INTERP_H_INCLUDED

#include <gui.h>
#include <gui_graph.h>

class PInterFrame : public BaseFrame
{
    public:
        bool timed_refresh;
        int Nx,N_data,Np;
        
        std::vector<double> x_data;
        std::vector<std::vector<double>> data;
        
        std::vector<int> index,current_index,steps;
        std::vector<std::vector<double>> params;
        std::vector<double> params_min,params_max;
        
        std::vector<std::vector<double>> uv;
        std::vector<std::vector<int>> combinations;
        
        std::vector<std::vector<double>> disp_y;
        
        wxTimer *timer;
        wxPanel *ctrl_panel;
        wxBoxSizer *ctrl_sizer;
        Graph *graph;
        
        std::vector<wxTextCtrl*> params_text;
        std::vector<wxSlider*> params_slider;
        
        PInterFrame(wxString const &title);
        ~PInterFrame();
        
        void evt_menu(wxCommandEvent &event);
        void evt_slider(wxCommandEvent &event);
        void evt_text(wxCommandEvent &event);
        void evt_timed_refresh(wxTimerEvent &event);
        void from_linear(std::vector<int> &index,int i);
        void interpolate_data();
        void load(std::string const &fname);
        int to_linear(std::vector<int> const &index);
};

#endif // GUI_PARAMETRIC_INTERP_H_INCLUDED
