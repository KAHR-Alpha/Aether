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

#ifndef GUI_EFFECTIVE_MODEL_H_INCLUDED
#define GUI_EFFECTIVE_MODEL_H_INCLUDED

#include <sstream>
#include <vector>

#include <material.h>
#include <mathUT.h>
#include <phys_tools.h>

#include <gui.h>
#include <gui_material.h>

class EffModelFrame: public BaseFrame
{
    public:
        int model_type,display_type;
        unsigned int Nl;
        double lambda_min,lambda_max;
        
        MaterialSelector *mat_1,*mat_2;
        
        std::vector<double> lambda,eff_model_re,eff_model_im;
        
        Graph *coeff_graph;
        
        wxChoice *model_type_ctrl,*display_ctrl;
        SpectrumSelector *sp_selector;
        
        EffModelFrame(wxString const &title);
        ~EffModelFrame();
        
        void export_data(wxCommandEvent &event);
        void recomp_model();
        void recomp_model_event(wxCommandEvent &event);
        void switch_display(wxCommandEvent &event);
        void switch_model(wxCommandEvent &event);
        void update_spectrum(wxCommandEvent &event);
};

#endif // GUI_EFFECTIVE_MODEL_H_INCLUDED
