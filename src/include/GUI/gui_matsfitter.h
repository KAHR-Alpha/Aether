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

#ifndef GUI_MATSFITTER_H_INCLUDED
#define GUI_MATSFITTER_H_INCLUDED

#include <gui_graph.h>
#include <gui_matspanel.h>

#include <wx/tglbtn.h>

class GraphMatsFitter: public wxPanel
{
    public:
        bool draw_grid,graph_drag;
        int sx,sy,padxm,padxp,padym,padyp;
        double xmin,xmax;
        double ymin,ymax;
        
        bool right_zoom;
        int right_zoom_xs;
        int right_zoom_ys;
        
        int mouse_x,mouse_y;
        int mouse_xp,mouse_yp;
        
        wxPen select_pen,grid_pen;
        wxBrush select_brush;
        
        int N_data;
        bool draw_real,draw_imag;
        std::vector<double> *w_base,*real_disp,*imag_disp
                                   ,*real_sum,*imag_sum;
        
        int widget_drag;
        std::vector<ModelPanel*> model_widgets;
        
        GraphMatsFitter(wxWindow *parent);
        
        void add_widget(ModelPanel *panel);
        void autoscale();
        int d2px(double x);
        int d2py(double y);
        void delete_widget(ModelPanel *params);
        void delete_all_widgets();
        void draw_border(wxGraphicsContext *gc);
        void draw_data(wxGraphicsContext *gc);
        void draw_grid_coord(wxGraphicsContext *gc);
        void evt_keyboard(wxKeyEvent &);
        void evt_mouse_hold_reset(wxMouseEvent &);
        void evt_mouse_left_down(wxMouseEvent &);
        void evt_mouse_left_up(wxMouseEvent &);
        void evt_mouse_motion(wxMouseEvent &);
        void evt_mouse_right_down(wxMouseEvent &);
        void evt_mouse_zoom(wxMouseEvent &);
        void on_paint(wxPaintEvent &);
        void on_resize(wxSizeEvent &);
        void process_right_zoom(wxMouseEvent &event);
        double px2dx(int i);
        double py2dy(int j);
        void replot(wxGraphicsContext *gc);
        void rescale();
        void set_base_data(std::vector<double> *w_base,
                           std::vector<double> *real_disp,
                           std::vector<double> *imag_disp);
};

class OptimThread: public wxThread
{
    public:
        bool *optim_toggle;
        std::vector<double> *w,*eps_real,*eps_imag;
        MatsPanel *mats_panel;
        
        ExitCode Entry()
        {
            mats_panel->optimize(optim_toggle,w,eps_real,eps_imag);
            
            Delete();
            
            return 0;
        }
};

class MatsFitter: public BaseFrame
{
    public:
        std::string data_fname;
        wxButton *data_load_btn;
        wxChoice *data_type_choice;
        wxTextCtrl *data_name_disp;
        wxCheckBox *disp_real_box,*disp_imag_box;
        GraphMatsFitter *fitter;
        Graph *error_graph;
        
        MatsPanel *mats_panel;
        
        bool optim_toggle;
        wxToggleButton *optim_button;
        wxMutex optim_mutex;
        OptimThread *optim_thread;
        
        int N_data;
        std::vector<double> w_base,real_base,imag_base,
                                   real_disp,imag_disp,
                                   real_error,imag_error,
                                   real_sum,imag_sum;
        
        MatsFitter(wxString const &title);
        
        void add_dielec_button_click(wxCommandEvent &event);
        void add_dielec_model(int type);
        void delete_fitter(wxCommandEvent &event);
        void delete_all_fitters();
        void get_data_fname(wxCommandEvent &event);
        void new_fitter(wxCommandEvent &event);
        void needs_refresh(wxCommandEvent &event);
        void optimize(wxCommandEvent &event);
        void top_computations(wxCommandEvent &event);
};


#endif // GUI_MATSFITTER_H_INCLUDED
