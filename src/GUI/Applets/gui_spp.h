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

#ifndef GUI_SPP_H_INCLUDED
#define GUI_SPP_H_INCLUDED

#include <sstream>
#include <vector>

#include <material.h>
#include <mathUT.h>
#include <phys_tools.h>

#include <gui.h>
#include <gui_material.h>

class SppFrame: public BaseFrame
{
    public:
        bool no_metal;
        unsigned int Nl;
        double n_dielec,eps_dielec;
        double lambda_min,lambda_max;
        
        std::vector<double> lambda,disp_lambda,kspp_re,kspp_im;
        std::vector<double> profile_dielec,profile_dielec_z;
        std::vector<double> profile_metal,profile_metal_z;
        std::vector<Imdouble> kspp;
        
        Graph *disp_graph,*aux_graph,*profile_graph;
        
        wxTextCtrl *n_ctrl,
                   *mat_ctrl;
        
        SldCombi *sld_profile;
        
        MaterialSelector *mat_selector;
        SpectrumSelector *sp_selector;
        
        SppFrame(wxString const &title);
        ~SppFrame();
        
        void load_material(wxCommandEvent &event);
        void recomp_spp();
        void update_dielec(wxCommandEvent &event);
        void update_profile();
        void update_profile_event(wxCommandEvent &event);
        void update_spectrum(wxCommandEvent &event);
};

#endif // GUI_SPP_H_INCLUDED
