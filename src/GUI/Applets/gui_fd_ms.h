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

#ifndef GUI_FD_MS_H_INCLUDED
#define GUI_FD_MS_H_INCLUDED

#include <gui.h>
#include <gui_material.h>
#include <gui_multilayers_utils.h>
#include <gui_panels_list.h>
#include <multilayers.h>

#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <mesh_base.h>
#include <mesh_tools.h>

class MSMapEvent: public wxEvent
{
    public:
        int Nr,Ni;
        double nr_min,nr_max,ni_min,ni_max;
        
        MSMapEvent(wxEventType type,int winid=0)
            :wxEvent(winid,type)
        {
            ResumePropagation(wxEVENT_PROPAGATE_MAX);
        }
        
        MSMapEvent(MSMapEvent const &event)
            :wxEvent(event),
             Nr(event.Nr),
             Ni(event.Ni),
             nr_min(event.nr_min), nr_max(event.nr_max),
             ni_min(event.ni_min), ni_max(event.ni_max)
        {
        }
        
        virtual wxEvent* Clone() const { return new MSMapEvent(*this); }
};

wxDECLARE_EVENT(EVT_MAP_REQUEST,MSMapEvent);
wxDECLARE_EVENT(EVT_MAP_READY,wxCommandEvent);
wxDECLARE_EVENT(EVT_NEW_TARGET,wxCommandEvent);
wxDECLARE_EVENT(EVT_MAP_REQUEST,MSMapEvent);

class FDSolverFrame: public BaseFrame
{
    public:
        wxScrolledWindow *structure_panel;
        wxPanel *display_panel,*mono_panel,*multi_panel;
        NamedTextCtrl <double> *Dz_ctrl;
        
        NamedTextCtrl<int> *N_zp_ctrl,*N_zm_ctrl;
        NamedTextCtrl<double> *kmax_zp_ctrl,*smax_zp_ctrl,*amax_zp_ctrl;
        NamedTextCtrl<double> *kmax_zm_ctrl,*smax_zm_ctrl,*amax_zm_ctrl;
        
        NamedTextCtrl<double> *pad_zp_ctrl,*pad_zm_ctrl;
        
        MiniMaterialSelector *superstrate_selector;
        MiniMaterialSelector *substrate_selector;
        
        PanelsList<> *layers_list;
        
        SpectrumSelector *multi_selector;
        
        wxStaticBoxSizer *mono_modes_sizer;
        wxScrolledWindow *mono_modes_panel;
        WavelengthSelector *mono_selector;
        Graph *mono_graph;
        wxRadioBox *mono_polar;
        NamedTextCtrl<int> *mono_N_ctrl;
        
        FDSolverFrame(wxString const &title);
        
        void evt_add_layer(wxCommandEvent &event);
        void evt_mode_change(wxCommandEvent &event);
        void evt_mono_compute(wxCommandEvent &event);
        void evt_mono_export(wxCommandEvent &event);
        void evt_mono_display(wxCommandEvent &event);
        void evt_panels_change(wxCommandEvent &event);
        void rename_panels();
};

class MSTarget
{
    public:
        double nr,ni;
        
        MSTarget(double nr_,double ni_)
            :nr(nr_), ni(ni_)
        {}
        
        MSTarget(MSTarget const &target)
            :nr(target.nr), ni(target.ni)
        {}
        
        void operator = (MSTarget const &target)
        {
            nr=target.nr;
            ni=target.ni;
        }
        
        bool operator == (MSTarget const &target)
        {
            if(nr==target.nr && ni==target.ni) return true;
            return false;
        }
};

class HAMS_GL: public wxGLCanvas
{
    public:
        bool busy,target_mode,zoom_mode;
        wxGLContext *glcnt;
        wxTimer *timer,*map_timer;
        
        Glite::Text gl_text;
        
        int sx,sy;
        int pad_xm,pad_xp,pad_ym,pad_yp;
        double g_xm,g_xp,g_ym,g_yp;
        
        double xmin,xmax,ymin,ymax;
        double c_xm,c_xp,c_ym,c_yp;
        double n_xm,n_xp,n_ym,n_yp;
        
        int mouse_x,mouse_xp;
        int mouse_y,mouse_yp;
        
        double target_nr,target_ni;
        double zoom_nr_s,zoom_nr_e;
        double zoom_ni_s,zoom_ni_e;
        
        double cam_x,cam_y;
        double cam_w,cam_h;
        GLfloat proj_gl[16];
        
        // Borders
        
        GLuint border_vao,prog_borders;
        
        // Cross
        
        GLuint cross_vao,prog_cross;
        GLfloat n_sub,n_sup;
        
        // Map
        
        int N1,N2;
        GLuint map_tex,map_vao,prog_map,sampler;
        GLfloat exposure;
        
        // Targets
        
        GLuint target_vao,prog_target,prog_target_outline;
        std::vector<MSTarget> targets;
        
        #ifndef WX30_RESTRICT
        HAMS_GL(wxWindow *parent,wxGLAttributes const &attrib);
        #else
        HAMS_GL(wxWindow *parent);
        #endif
        ~HAMS_GL();
        
        void add_target(double nr,double ni);
        void clear_targets();
        void enable_target_mode();
        void forget_target(MSTarget const &target);
        std::vector<MSTarget> get_targets();
        void keyboard(wxKeyEvent &event);
        void evt_mouse_motion(wxMouseEvent &event);
        void mouse_wheel(wxMouseEvent &event);
        double mx_to_nr(int i);
        double my_to_ni(int j);
        void paint_refresh(wxPaintEvent &event);
        void render();
        void request_map();
        void resize(wxSizeEvent &event);
        void set_exposure(double exposure);
        void set_map(int Nr,int Ni,
                     double nr_min,double nr_max,
                     double ni_min,double ni_max,
                     GLfloat *map);
        void set_substrate(double n_sub);
        void set_superstrate(double n_sup);
        void set_zoom(double nr_min,double nr_max,
                      double ni_min,double ni_max);
        void timed_refresh(wxTimerEvent &event);
        void update_camera();
        void update_size();
};

class MSTargetInfoPanel: public wxPanel
{
    public:
        bool display_E,display_Ex,display_Ey,display_Ez,display_complex;
        int polarization;
        double lambda;
        MSTarget target;
        Multilayer_TMM ml;
        
        MLFieldHolder holder;
        
        wxTextCtrl *real_n_disp,
                   *imag_n_disp,
                   *wvl_disp,
                   *propag_disp;
        Graph *field_graph;
        wxCheckBox *ex_ctrl,*ey_ctrl,*ez_ctrl,*e_ctrl;
        wxRadioBox *cpl_ctrl;
        
        MSTargetInfoPanel(wxWindow *parent,
                          int polarization,double lambda,
                          MSTarget const &target,Multilayer_TMM const &ml);
        
        void change_target(MSTarget const &target);
        void compute_field();
        void evt_cpl_switch(wxCommandEvent &event);
        void evt_field_check(wxCommandEvent &event);
        void export_field(std::string fname);
        void set_polarization(int polarization);
        void update_graph();
};

class MSSingleTargetDialog: public wxDialog
{
    public:
        bool confirm_target;
        MSTargetInfoPanel *target_panel;
        
        MSSingleTargetDialog(int px,int py,int sx,int sy,
                             int polarization,double lambda,
                             MSTarget const &target,
                             Multilayer_TMM const &ml);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
};

class MSTargetDeletionEvent: public wxEvent
{
    public:
        std::vector<MSTarget> targets;
        
        MSTargetDeletionEvent(wxEventType type,int winid=0)
            :wxEvent(winid,type)
        {
            ResumePropagation(wxEVENT_PROPAGATE_MAX);
        }
        
        MSTargetDeletionEvent(MSTargetDeletionEvent const &event)
            :wxEvent(event),
             targets(event.targets)
        {
        }
        
        virtual wxEvent* Clone() const { return new MSTargetDeletionEvent(*this); }
};

class MSMultipleTargetsDialog: public wxDialog
{
    public:
        unsigned int current_target;
        double lambda;
        std::vector<MSTarget> targets;
        std::vector<bool> deletion_flag;
        
        wxTextCtrl *current_target_ctrl;
        wxCheckBox *forget_target_ctrl;
        MSTargetInfoPanel *target_panel;
        
        MSMultipleTargetsDialog(wxWindow *parent,
                                int px,int py,int sx,int sy,
                                int polarization,double lambda,
                                std::vector<MSTarget> const &targets,
                                Multilayer_TMM const &ml);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_forget_switch(wxCommandEvent &event);
        void evt_export_data(wxCommandEvent &event);
        void evt_export_field(wxCommandEvent &event);
        void evt_next(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_previous(wxCommandEvent &event);
        void evt_text_enter(wxCommandEvent &event);
        void evt_text_focus(wxFocusEvent &event);
        void set_target(int n);
        void subevent_text();
        void update_target();
};

class HAPSolverFrame: public BaseFrame
{
    public:
        bool autorefresh,auto_exposure;
        int polarization;
        
        wxScrolledWindow *structure_panel;
        wxPanel *display_panel,*target_panel,*multi_panel;
        
        wxRadioBox *polar_ctrl;
        MiniMaterialSelector *superstrate_selector;
        MiniMaterialSelector *substrate_selector;
        
        PanelsList<LayerPanelBase> *layers_list;
        
        SpectrumSelector *multi_selector;
        
        // Target
        
        std::vector<double> nt_r,nt_i;
        WavelengthSelector *lambda_t_ctrl;
        
        NamedTextCtrl<double> *nr_t_min_ctrl,*nr_t_max_ctrl;
        NamedTextCtrl<double> *ni_t_min_ctrl,*ni_t_max_ctrl;
        
        bool computing_map,reallocate_map;
        int map_Nr,map_Ni;
        double map_nr_min,map_nr_max;
        double map_ni_min,map_ni_max;
        GLfloat *map_data;
        
        wxToggleButton *target_refresh_btn;
        wxSlider *exposure_slider;
        wxToggleButton *auto_exposure_btn;
        
        HAMS_GL *target_graph;
        
        // Dispersion
        
        HAPSolverFrame(wxString const &title);
        
        void compute_exposure();
        void compute_map();
        void construct_multilayer(Multilayer_TMM &ml);
        void evt_add_layer(wxCommandEvent &event);
        void evt_clear_targets(wxCommandEvent &event);
        void evt_exposure(wxCommandEvent &event);
        void evt_exposure_auto(wxCommandEvent &event);
        void evt_map_ready(wxCommandEvent &event);
        void evt_map_request(MSMapEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_new_target(wxCommandEvent &event);
        void evt_panels_change(wxCommandEvent &event);
        void evt_polarization_select(wxCommandEvent &event);
        void evt_refresh_toggle(wxCommandEvent &event);
        void evt_reset_zoom(wxCommandEvent &event);
        void evt_structure_change(wxCommandEvent &event);
        void evt_target_button(wxCommandEvent &event);
        void evt_targets_deletion(MSTargetDeletionEvent &event);
        void evt_targets_info(wxCommandEvent &event);
        void load_project(wxFileName const &fname);
        void refine_target(MSTarget &target);
        void rename_panels();
        void request_map_computation();
        void reset_targets();
        void save_project(wxFileName const &fname);
        
        // Lua Loaders
        
        [[deprecated]] void lua_add_layer(double height,double std_dev,GUI::Material *material);
        void lua_set_substrate(GUI::Material *material);
        void lua_set_superstrate(GUI::Material *material);
};

#endif // GUI_FD_MS_H_INCLUDED
