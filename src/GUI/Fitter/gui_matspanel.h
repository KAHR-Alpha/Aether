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

#ifndef GUI_MATSPANEL_H
#define GUI_MATSPANEL_H

#include <string>
#include <dielec_models.h>
#include <mathUT.h>
#include <gui.h>

wxDECLARE_EVENT(EVT_MODEL_NEW,wxCommandEvent);
wxDECLARE_EVENT(EVT_MODEL_REMOVE,wxCommandEvent);
wxDECLARE_EVENT(EVT_MODEL_RECOMP,wxCommandEvent);
wxDECLARE_EVENT(EVT_MODEL_GRAPH_REFRESH,wxCommandEvent);

class MatsPanel;
class ModelPanel;

enum
{
    FIT_CONST=0,
    FIT_DRUDE,
    FIT_LORENTZ,
    FIT_CRITPOINTS
};

class ModelPanel: public wxPanel
{
    public:
        int type;
        bool hide,substract;
        
        wxColour real_col,imag_col;
        wxPen real_pen,imag_pen;
        wxBrush real_brush,imag_brush;
        
        wxButton *real_col_btn;
        wxButton *imag_col_btn;
        
        // Models and data
        
//        double eps_inf;
//        
//        double wd,g; // Drude
//        double A,O,G; // Lorentz / Critpoints
//        double P; //Critpoints
        
        // Data mutation
        
        double eps_inf_mut;
        double wd_mut,g_mut;
        double A_mut,O_mut,G_mut;
        double P_mut;
        
        // Models
        
        DrudeModel drude_model;
        LorentzModel lorentz_model;
        CritpointModel critpoints_model;
        
        // Models mutation
        
        DrudeModel drude_model_mut;
        LorentzModel lorentz_model_mut;
        CritpointModel critpoints_model_mut;
        
        int N_data;
        std::vector<double> w,real_part,imag_part;
        
        // Panel
        
        NamedTextCtrl<double> *eps_inf;
        NamedTextCtrl<double> *wd,*g;
        NamedTextCtrl<double> *A,*O,*G;
        NamedTextCtrl<double> *P;
        
        // Drawing stuff
        
        double x_center,y_center;
        double x_hdl_1,y_hdl_1;
        double x_hdl_2,y_hdl_2;
        double x_hdl_3,y_hdl_3;
        
        bool move_center;
        bool move_hdl_1;
        bool move_hdl_2;
        bool move_hdl_3;
        
        /////
        
        ModelPanel(wxWindow *parent,int type,bool ctrl_only=false);
        
        void accept_mutation();
        void apoptose(wxCommandEvent &event);
        void change_imag_col(wxCommandEvent &event);
        void change_real_col(wxCommandEvent &event);
        void handles_to_model();
        bool is_selected(double x,double tol_x,double y,double tol_y);
        Imdouble get_eps(double w);
        Imdouble get_eps_mut(double w);
        void model_to_handles();
        void move(double x,double y);
        void mutate(double span);
        void recompute();
        void reset_motion();
        void set_optimization_engine(OptimEngine *engine);
        void set_spectrum(int N_data,double w1,double w2);
        void toggle_hide(wxCommandEvent &event);
        void toggle_substract(wxCommandEvent &event);
        void update_panel();
        void update_values();
        void update_widget(wxCommandEvent &event);
};

class Dielec_dialog: public wxDialog
{
    public:
        int selection;
        wxChoice *dielec_type;
        
        wxGenericStaticBitmap *formula_img;
        wxImage empty_formula,
                drude_formula,
                lorentz_formula,
                critpoints_formula;
        
        Dielec_dialog();
        
        void close_dialog(wxCommandEvent &event);
        void confirm_selection(wxCommandEvent &event);
        void select_model(wxCommandEvent &event);
};

class MatsPanel: public wxPanel
{
    public:
        bool ctrl_only;
        int Nl;
        double w1,w2;
        
        OptimEngine *optim_engine;
        
        wxButton *add_dielec_button;
        
        wxScrolledWindow *mats_list;
        wxBoxSizer *mats_sizer;
        
        std::vector<ModelPanel*> model_panels;
        
        MatsPanel(wxWindow *parent,OptimEngine *engine=nullptr);
        
        void add_dielec_button_click(wxCommandEvent &event);
        void add_dielec_model(int type);
        void delete_fitter(wxCommandEvent &event);
        void delete_all_fitters();
        Imdouble get_eps(double w);
        Imdouble get_eps_mut(double w);
        Imdouble get_eps_sub(double w);
        void optimize(bool *optim_toggle,std::vector<double> *w,
                      std::vector<double> *eps_real,std::vector<double> *eps_imag);
        void set_spectrum(int Nl,double w1,double w2);
        void unlock_dielec_button();
        void update_values();
};

#endif
