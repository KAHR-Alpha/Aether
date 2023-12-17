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

#ifndef GUI_ELLIPSO3_H_INCLUDED
#define GUI_ELLIPSO3_H_INCLUDED

#include <phys_tools.h>

#include <gui.h>
#include <gui_gl_fd.h>
#include <gui_material.h>

#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/splitter.h>

class GL_Ellipso3D: public GL_3D_Base
{
    public:
        bool watch_paused;
        double period;
        AngleRad incidence;
        Vector3 ki,kr,pi,pr,sv;
        Imdouble ip,is,rp,rs;
        int density;
        double span;
        
        GLuint prog_field,vao_field,
               prog_frame,vao_frame;
        
        wxStopWatch *watch;
        
        GL_Ellipso3D(wxWindow *parent,AngleRad const &incidence,
                     Imdouble const &ip,Imdouble const &is,
                     Imdouble const &rp,Imdouble const &rs,
                     int density,double span);
        ~GL_Ellipso3D();
        
        void align_camera_polarization();
        void align_camera_incidence_plane();
        void init_opengl();
        void recompute_vectors();
        void render();
        void set_coefficients(Imdouble const &ip,Imdouble const &is,
                              Imdouble const &rp,Imdouble const &rs);
        void set_density(int density);
        void set_incidence(AngleRad const &angle);
        void set_period(double period);
        void set_span(double span);
        void switch_watch();
};

class Ellipso3DFrame: public BaseFrame
{
    public:
        
        NamedTextCtrl<int> *density;
        NamedTextCtrl<double> *span;
        NamedTextCtrl<double> *period_ctrl;
        
        WavelengthSelector *wavelength;
        MiniMaterialSelector *substrate;
        
        SliderDisplay *angle;
        
        SliderDisplay *p_amp,*p_phase;
        SliderDisplay *s_amp,*s_phase;
        
        GL_Ellipso3D *gl;
        
        Ellipso3DFrame(wxString const &title);
        
        void evt_align_incidence_plane(wxCommandEvent &event);
        void evt_align_polarization(wxCommandEvent &event);
        void evt_angle(wxCommandEvent &event);
        void evt_component(wxCommandEvent &event);
        void evt_component_sub();
        void evt_density(wxCommandEvent &event);
        void evt_material(wxCommandEvent &event);
        void evt_pause(wxCommandEvent &event);
        void evt_period(wxCommandEvent &event);
        void evt_span(wxCommandEvent &event);
        void evt_wavelength(wxCommandEvent &event);
};

#endif // GUI_ELLIPSO3_H_INCLUDED
