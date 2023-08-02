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

#ifndef GUI_ELLIFR_H_INCLUDED
#define GUI_ELLIFR_H_INCLUDED

#include <mathUT.h>
#include <gui.h>
#include <wx/stopwatch.h>

class ImdoubleCtrl: public wxPanel
{
    public:
        wxTextCtrl *amp_ctrl,*phase_ctrl;
        
        ImdoubleCtrl(wxWindow *parent,std::string const &name);
        
        void set_amp(double amp);
        void set_phase(AngleOld phase);
};

class ElliGraph: public wxPanel
{
    public:
        int sx,sy;
        double r_max,t,period;
        double base_unit;
        
        wxPen *field_pen;
        
        double p_amp_1,p_phase_1;
        double s_amp_1,s_phase_1;
        double p_amp_2,p_phase_2;
        double s_amp_2,s_phase_2;
        
        double elli_1_x0;
        double elli_1_y0;
        double elli_2_x0;
        double elli_2_y0;
        
        std::vector<double> elli_1_x;
        std::vector<double> elli_1_y;
        std::vector<double> elli_2_x;
        std::vector<double> elli_2_y;
        
        ElliGraph(wxWindow *parent);
        ~ElliGraph();
        
        void draw_data(wxGraphicsContext *gc);
        void on_paint(wxPaintEvent &);
        void on_resize(wxSizeEvent &);
        void replot(wxGraphicsContext *gc);
        void rescale();
        void recomp_ellipses();
        void set_fields(double p_amp_1,double p_phase_1,
                        double s_amp_1,double s_phase_1,
                        double p_amp_2,double p_phase_2,
                        double s_amp_2,double s_phase_2);
        void set_time(double t,double period);
};

class ElliFrame: public BaseFrame
{
    public:
        bool paused;
        wxStopWatch *watch;
        wxTimer *timer;
        ElliGraph *draw_panel;
        
        double p_amp,p_phase,s_amp,s_phase;
        
        SldCombi *p_amp_sld,
                 *p_phase_sld,
                 *s_amp_sld,
                 *s_phase_sld,
                 *angle_sld;
                 
        wxTextCtrl *n_ctrl,*k_ctrl,
                   *period_ctrl,
                   *angle_ctrl;
        
        ImdoubleCtrl *rp_ctrl,*rs_ctrl;
        
        ElliFrame(wxString const &title);
        ~ElliFrame();
        
        void pause_toggle(wxCommandEvent &event);
        void timed_refresh(wxTimerEvent &event);
};

#endif // GUI_ELLIFR_H_INCLUDED
