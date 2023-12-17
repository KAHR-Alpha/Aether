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

#ifndef GUI_FITTER_H_INCLUDED
#define GUI_FITTER_H_INCLUDED

#include <gui.h>
#include <gui_graph.h>
#include <gui_panels_list.h>

#include <wx/splitter.h>

class FitterHandle;
class FitterModel;

class FitterModelDialog: public wxDialog
{
    public:
        bool choice_ok;
        int choice_type,Np;
        
        std::vector<double> x_data,y_data,params;
        
        wxChoice *model_choice;
        Graph *graph;
        
        FitterModelDialog();
        
        void evt_cancel(wxCommandEvent &event);
        void evt_choice(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
};

class FitterHandle
{
    public:
        double x,y;
        
        FitterModel *model;
        
        FitterHandle();
        FitterHandle(FitterHandle const &handle);
        
        void set_position(double x,double y);
        void set_position_strict(double x,double y);
        
        void operator = (FitterHandle const &handle);
};

class GraphFitter:public Graph
{
    public:
        bool show_cropping;
        int target_crop;
        double x_min_crop,x_max_crop;
        std::vector<std::vector<FitterHandle*>> handles;
        std::vector<wxPen> handles_pens;
        std::vector<bool> show_handles_v;
        
        wxPen cropping_pen;
        wxBrush cropping_brush;
        
        FitterHandle *target_handle;
        std::vector<FitterHandle*> *target_group;
        
        GraphFitter(wxWindow *parent);
        ~GraphFitter();
        
        void add_handles(std::vector<FitterHandle> &handles);
        void autoscale();
        void clear_graph();
        bool evt_mouse_left_down_spec(wxMouseEvent &event);
        bool evt_mouse_left_up_spec(wxMouseEvent &event);
        bool evt_mouse_motion_spec(wxMouseEvent &event);
        void forget_handles(std::vector<FitterHandle> &handles);
        void hide_cropping();
        void postplot(wxGraphicsContext *gc);
        void set_cropping(double x_min_crop,double x_max_crop);
        void show_handles(std::vector<FitterHandle> &handles,bool val);
        void set_handles_color(std::vector<FitterHandle> &handles,
                               double r,double g,double b);
};

class FitterModel: public PanelsListBase
{
    public:
        int type,Np;
        bool hide,substract;
        std::vector<double> *x_data;
        std::vector<double> y_data,y_data_mutation;
        
        std::vector<double> params,params_mutation;
        std::vector<FitterHandle> handles;
        
        wxButton *color_btn;
        wxCheckBox *hide_ctrl,*substract_ctrl;
        
        std::vector<NamedTextCtrl<double>*> params_ctrl;
        
        GraphFitter *graph;
        
        void (*f)(std::vector<double> const&,std::vector<double> const&,std::vector<double> &);
        void (*h_to_p)(std::vector<FitterHandle> const&,std::vector<double> &);
        void (*p_to_h)(std::vector<double> const&,std::vector<FitterHandle> &);
        
        FitterModel(wxWindow *parent,int type,GraphFitter *graph,
                    std::vector<double> *x_data,
                    std::vector<double> const &params,
                    std::vector<std::string> const &param_names,
                    std::vector<FitterHandle> const &handles,
                    void (*f)(std::vector<double> const&,std::vector<double> const&,std::vector<double> &),
                    void (*h_to_p)(std::vector<FitterHandle> const&,std::vector<double> &),
                    void (*p_to_h)(std::vector<double> const&,std::vector<FitterHandle> &));
        ~FitterModel();
        
        void accept_mutation();
        void apoptose(wxCommandEvent &event);
        void evt_color(wxCommandEvent &event);
        void evt_hide(wxCommandEvent &event);
        void evt_substract(wxCommandEvent &event);
        void evt_value(wxCommandEvent &event);
        void get_latex(int id,std::string &formula_str,std::string &params_str);
        void get_matlab(int id,std::string &formula_str,std::string &params_str);
        void lock();
        void mutate(double eps);
        void recompute_from_handles();
        void reallocate();
        void unlock();
        void update_controls();
        void update_data();
        void update_handles();
        void update_params();
};

class FitterExportDialog: public wxDialog
{
    public:
        bool choice_ok;
        int export_type;
        wxString fname;
        
        wxRadioBox *type_ctrl;
        
        FitterExportDialog();
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
};

class FitterXDataDialog: public wxDialog
{
    public:
        NamedTextCtrl<int> *Np,*Nstep;
        NamedTextCtrl<double> *x_min,*x_max;
        
        FitterXDataDialog(wxWindow *parent,
                          double x_min,double x_max,
                          int Np,int Nstep);
        
        void evt_apply(wxCommandEvent &event);
        void evt_close(wxCloseEvent &event);
        void evt_reset(wxCommandEvent &event);
        void evt_stepping(wxCommandEvent &event);
        void evt_value(wxCommandEvent &event);
        double get_min();
        double get_max();
        int get_stepping();
        void set_cropping(double x_min,double x_max);
        void set_points(int Np);
};

class FitterFrame: public BaseFrame
{
    public:
        bool first_load,optim_toggle;
        int Nl,Nd,Nstep,N_models,N_models_tot;
        double cut_min,cut_max,error_value;
        std::vector<double> x_data,y_data,
                            x_data_display,y_data_display,y_data_work,
                            error_display;
        
        wxScrolledWindow *ctrl_panel;
        
        wxTextCtrl *data_name;
        wxButton *data_load_btn;
                
        wxButton *add_model_btn;
        PanelsList<> *models_panel;
        std::vector<FitterModel*> models_list;
        
        NamedTextCtrl<double> *error_ctrl;
        wxButton *export_btn;
        
        GraphFitter *fitter;
        Graph *fitter_error;
        
        // Optimization
        
        wxToggleButton *optim_btn;
        wxMutex optim_mutex,refresh_mutex;
        wxCondition optim_condition,refresh_condition;
        
        // X Data Rework
        
        double x_min_crop,x_max_crop;
        wxButton *x_data_btn;
        FitterXDataDialog *x_data_dialog;
        
        FitterFrame(wxString const &title);
        ~FitterFrame();
        
        void evt_add_model(wxCommandEvent &event);
        void evt_export(wxCommandEvent &event);
        void evt_load(wxCommandEvent &event);
        void evt_model_delete(wxCommandEvent &event);
        void evt_optimize(wxCommandEvent &event);
        void evt_optimization_cleanup(wxCommandEvent &event);
        void evt_pure_refresh(wxCommandEvent &event);
        void evt_recompute(wxCommandEvent &event);
        void evt_recompute_sub();
        void evt_x_data_dialog(wxCommandEvent &event);
        void evt_x_data_dialog_apply(wxCommandEvent &event);
        void evt_x_data_dialog_close(wxCommandEvent &event);
        void evt_x_data_dialog_reset(wxCommandEvent &event);
        void evt_x_data_dialog_value(wxCommandEvent &event);
        void evt_x_data_graph_value(wxCommandEvent &event);
        void optimize();
        void rebuild_data();
        void rebuild_models_list();
};

#endif // GUI_FITTER_H_INCLUDED
