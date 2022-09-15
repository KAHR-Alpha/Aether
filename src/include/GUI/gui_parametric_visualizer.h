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

#ifndef GUI_PARAMETRIC_VISU_H_INCLUDED
#define GUI_PARAMETRIC_VISU_H_INCLUDED

#include <gui.h>
#include <gui_graph.h>

class PVisuFrame : public BaseFrame
{
    public:
        int Nx,N_data,Np;
        
        std::vector<double> x_data;
        std::vector<std::vector<double>> data;
        
        std::vector<int> index,steps;
        std::vector<std::vector<double>> params;
        std::vector<double> params_min,params_max;
        
        std::vector<std::vector<double>> disp_y;
        
        wxPanel *ctrl_panel;
        wxBoxSizer *ctrl_sizer;
        Graph *graph;
        
        std::vector<wxTextCtrl*> params_text;
        std::vector<wxSlider*> params_slider;
        
        PVisuFrame(wxString const &title);
        ~PVisuFrame();
        
        void evt_menu(wxCommandEvent &event);
        void evt_slider(wxCommandEvent &event);
        void from_linear(std::vector<int> &index,int i);
        void load(std::string const &fname);
        int to_linear(std::vector<int> const &index);
        void update_data();
};

#endif // GUI_PARAMETRIC_VISU_H_INCLUDED
