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

#ifndef GUI_FRESNEL_H_INCLUDED
#define GUI_FRESNEL_H_INCLUDED

#include <sstream>
#include <vector>

#include <material.h>
#include <mathUT.h>
#include <phys_tools.h>

#include <gui.h>
#include <gui_material.h>

class FresnelFrame: public BaseFrame
{
    public:
        unsigned int Nl;
        double n_sup;
        double lambda_min,lambda_max;
                
        std::vector<double> angle;
        std::vector<double> r_te,r_tm,r_avg,ar_te,ar_tm,ar_avg;
        
        Graph *coeff_graph;
        
        wxTextCtrl *sup_ctrl;
        
        SldCombi *sld_wavelength;
        
        MaterialSelector *mat_selector;
        SpectrumSelector *sp_selector;
        
        FresnelFrame(wxString const &title);
        ~FresnelFrame();
        
        void load_material(wxCommandEvent &event);
        void update_coeffs();
        void update_coeffs_event(wxCommandEvent &event);
        void update_const_n(wxCommandEvent &event);
        void update_spectrum(wxCommandEvent &event);
};

#endif // GUI_FRESNEL_H_INCLUDED
