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

#ifndef GUI_DIFFRACT_PATTERN_H_INCLUDED
#define GUI_DIFFRACT_PATTERN_H_INCLUDED

#include <gui.h>
#include <gui_gl.h>

class DiffPatternFrame: public BaseFrame
{
    public:
        int Nx_d,Ny_d,
            Nx_s,Ny_s;
        double Dx;
        Grid2<double> data,screen_data;
        
        std::vector<std::vector<int>> compressed_data_start,compressed_data_end;
        
        // Controls
        
        wxScrolledWindow *ctrl_panel;
        
        FileSelector *input_data;
        WavelengthSelector *lambda;
        LengthSelector *input_scale,*screen_distance;
        
        NamedTextCtrl<int> *screen_x_res,*screen_y_res;
        
        // Display
        
        GL_2D_display *input_display,*output_display;
        
        DiffPatternFrame(wxString const &title);
        ~DiffPatternFrame();
        
        Imdouble compute_data_spectrum(double kx,double ky);
        void evt_compute(wxCommandEvent &event);
        void evt_compute_double_FFT();
        void evt_compute_fraunhofer();
        void evt_compute_FT_growth();
        void evt_new_data(wxCommandEvent &event);
        
};

#endif // GUI_DIFFRACT_PATTERN_H_INCLUDED
