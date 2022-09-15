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

#ifndef GUI_GRAPH_H_INCLUDED
#define GUI_GRAPH_H_INCLUDED

#include <gui.h>
#include <gui_panels_list.h>

class Graph;

class BackdropPanel: public PanelsListBase
{
    public:
        int ID;
        Graph *graph;
        
        wxCheckBox *display;
        wxButton *color_btn;
        wxTextCtrl *legend;
        
        BackdropPanel(wxWindow *parent,Graph *graph,int ID);
        
        void evt_color(wxCommandEvent &event);
        void evt_display(wxCommandEvent &event);
        void evt_legend(wxCommandEvent &event);
};

class GraphOptionsDialog: public wxDialog
{
    public:
        Graph *graph;
        
        int N_data;
        std::vector<wxCheckBox*> checkbox;
        std::vector<wxButton*> color;
        std::vector<wxTextCtrl*> legend;
        
        NamedTextCtrl<double> *x_min,*x_max,*y_min,*y_max;
        wxCheckBox *lock_scale;
        
        NamedTextCtrl<double> *pointer_x;
        std::vector<NamedTextCtrl<double>*> pointer_y;
        
        wxChoice *memory_mode;
        wxSlider *memory_slider;
        
        wxScrolledWindow *backdrop_panel;
        PanelsList<BackdropPanel> *backdrops;
        
        GraphOptionsDialog(Graph *parent);
        
        // Spectral Colors
        std::vector<NamedTextCtrl<double>*> spcol_X,spcol_Y,spcol_Z;
        std::vector<wxButton*> spcol,spcol_norm;
        
        void GraphOptionsDialog_SpColors(wxWindow *panel);
        
        void evt_add_backdrop(wxCommandEvent &event);
        void evt_checkbox(wxCommandEvent &event);
        void evt_clear_memory(wxCommandEvent &event);
        void evt_color(wxCommandEvent &event);
        void evt_delete_backdrop(wxCommandEvent &event);
        void evt_legend(wxCommandEvent &event);
        void evt_memorize_data(wxCommandEvent &event);
        void evt_memory_mode(wxCommandEvent &event);
        void evt_memory_slider(wxCommandEvent &event);
        void evt_scale(wxCommandEvent &event);
        void rebuild_backdrop_list();
        void update_spectral_colors();
        void update_scale();
};

class Graph: public wxPanel
{
    public:
        bool spectral_data;
        bool draw_grid,graph_drag,legend,black,lock_scale;
        int sx,sy,padxm,padxp,padym,padyp;
        double xmin,xmax;
        double ymin,ymax;
        
        bool fixed_ratio;
        double x_ratio,y_ratio;
        
        bool right_zoom;
        int right_zoom_xs;
        int right_zoom_ys;
        
        int mouse_x,mouse_y;
        int mouse_xp,mouse_yp;
        
        wxPen select_pen,grid_pen;
        wxBrush select_brush;
        
        // External data
        
        int N_data;
        std::vector<std::vector<double>*> x_data;
        std::vector<std::vector<double>*> y_data;
        std::vector<bool> show_data_v;
        std::vector<wxPen> data_pen;
        std::vector<std::string> legend_str;
        
        // Memorised Data
        
        int show_memory,show_memory_target;
        std::vector<std::vector<std::vector<double>>> x_mem,y_mem;
        std::vector<std::vector<wxPen>> pen_mem;
        
        // Self Data
        
        std::vector<std::vector<double>> x_self;
        std::vector<std::vector<double>> y_self;
        std::vector<bool> show_self;
        std::vector<wxPen> pen_self;
        std::vector<std::string> fname_self,legend_self;
        
        GraphOptionsDialog *options_dialog;
        
        Graph(wxWindow *parent);
        
        [[deprecated]]
        void add_data(std::vector<double> *x_data,std::vector<double> *y_data,
                      double r=0,double g=0,double b=0,std::string legend_str="");
        void add_external_data(std::vector<double> *x_data,std::vector<double> *y_data,
                               double r=0,double g=0,double b=0,std::string legend_str="");
        virtual void autoscale();
        int d2px(double x);
        int d2py(double y);
        virtual void clear_graph();
        void clear_memorized_data();
        void close_options_dialog();
        void draw_border(wxGraphicsContext *gc);
        void draw_data(wxGraphicsContext *gc);
        void draw_data(wxGraphicsContext *gc,std::vector<double> const &x_data,std::vector<double> const &y_data,wxPen const &pen);
        void draw_memory_data(wxGraphicsContext *gc,int level);
        void draw_grid_coord(wxGraphicsContext *gc);
        void draw_legend(wxGraphicsContext *gc);
        void evt_dialog_closed(wxCloseEvent &event);
        void evt_keyboard(wxKeyEvent &event);
        void evt_mouse_hold_reset(wxMouseEvent &event);
        void evt_mouse_left_down(wxMouseEvent &event);
        void evt_mouse_left_up(wxMouseEvent &event);
        void evt_mouse_motion(wxMouseEvent &event);
        void evt_mouse_right_down(wxMouseEvent &event);
        void evt_mouse_zoom(wxMouseEvent &event);
        void evt_show_memorized_data(wxCommandEvent &event);
        virtual bool evt_mouse_left_down_spec(wxMouseEvent &event);
        virtual bool evt_mouse_left_up_spec(wxMouseEvent &event);
        virtual bool evt_mouse_motion_spec(wxMouseEvent &event);
        void force_ratio(double x_ratio,double y_ratio);
        [[deprecated]]
        void forget_data(std::vector<double> *x_data,std::vector<double> *y_data);
        void forget_external_data(std::vector<double> *x_data,std::vector<double> *y_data);
        void forget_self_data(int ID);
        void get_scale(double &xmin,double &xmax,double &ymin,double &ymax);
        void memorize_data();
        void on_paint(wxPaintEvent &event);
        void on_resize(wxSizeEvent &event);
        virtual void postplot(wxGraphicsContext *gc);
        void print();
        void process_right_zoom(wxMouseEvent &event);
        double px2dx(int i);
        double py2dy(int j);
        void replot(wxGraphicsContext *gc);
        void rescale();
        void set_scale(double xmin,double xmax,double ymin,double ymax);
        void set_scale_shift(double xmin,double xmax,double ymin,double ymax);
        void set_data_color(std::vector<double> *x_data,std::vector<double> *y_data,
                            double r,double g,double b);
        void show_data(std::vector<double> *x_data,std::vector<double> *y_data,bool val);
        void switch_dark();
};

#endif // GUI_GRAPH_H_INCLUDED
