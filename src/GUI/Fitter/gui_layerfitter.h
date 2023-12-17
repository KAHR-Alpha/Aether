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

#ifndef GUI_LAYERFITTER_H_INCLUDED
#define GUI_LAYERFITTER_H_INCLUDED

#include <berreman_strat.h>
#include <dielec_models.h>

#include <atomic>
#include <thread>

#include <gui.h>
#include <gui_graph.h>
#include <gui_matspanel.h>
#include <gui_panels_list.h>

class ModelPanel2: public PanelsListBase
{
    public:
        OptimEngine *engine;
        
        ModelPanel2(wxWindow *parent)
            :PanelsListBase(parent),
             engine(nullptr)
        {
        }
        
        void add_eps(std::vector<Imdouble> &eps,std::vector<double> const &w)
        {
            update_model();
            
            for(unsigned int i=0;i<eps.size();i++)
                eps[i]+=get_eps(w[i]);
        }
        
        virtual void clean_optimization_engine() {}
        virtual Imdouble get_eps(double w) { return 1.0; }
        virtual std::string get_matlab() { return ""; }
        
        void get_eps(std::vector<Imdouble> &eps,std::vector<double> const &w)
        {
            update_model();
            
            for(unsigned int i=0;i<eps.size();i++)
                eps[i]=get_eps(w[i]);
        }
        
        void set_optimization_engine(OptimEngine *engine_)
        {
            engine=engine_;
            
            set_optimization_engine_deep();
        }
        
        virtual void set_optimization_engine_deep() {}
        virtual void update_display() {}
        virtual void update_model() {}
};

class Dielec_Panel_Const : public ModelPanel2
{
    public:
        double eps_inf_val;
        
        NamedTextCtrl<double> *eps_inf;
        
        Dielec_Panel_Const(wxWindow *parent,double eps_inf=1.0);
        
        Imdouble get_eps(double w);
        std::string get_matlab();
        void set_optimization_engine_deep();
        void update_display();
        void update_model();
};

class Dielec_Panel_Drude : public ModelPanel2
{
    public:
        DrudeModel model;
        
        NamedTextCtrl<double> *wd,*g;
        
        Dielec_Panel_Drude(wxWindow *parent,double wd,double g);
        
        Imdouble get_eps(double w);
        std::string get_matlab();
        void set_optimization_engine_deep();
        void update_display();
        void update_model();
};

class Dielec_Panel_Lorentz : public ModelPanel2
{
    public:
        LorentzModel model;
        
        NamedTextCtrl<double> *A,*O,*G;
        
        Dielec_Panel_Lorentz(wxWindow *parent,double A,double O,double G);
        
        Imdouble get_eps(double w);
        std::string get_matlab();
        void set_optimization_engine_deep();
        void update_display();
        void update_model();
};

class Dielec_Panel_CritPoint : public ModelPanel2
{
    public:
        CritpointModel model;
        
        NamedTextCtrl<double> *A,*O,*G,*P;
        
        Dielec_Panel_CritPoint(wxWindow *parent,double A,double O,double G,double P);
        
        Imdouble get_eps(double w);
        void set_optimization_engine_deep();
        void update_display();
        void update_model();
};

class LayerFitter: public BaseFrame
{
    public:

        // Data
        int N_data;
        std::string data_fname;
        wxButton *data_load;
        wxChoice *data_type;
        wxTextCtrl *data_fname_disp;

        double lambda_min,lambda_max;
        std::vector<double> lambda,r_freq,data,data_comp;

        wxScrolledWindow *ctrl_panel;

        // Environment

        PanelsList<ModelPanel2> *env_mats,*layer_mats,*sub_mats;
        std::vector<Imdouble> env_eps,layer_eps,sub_eps,buffer_eps;
        
        NamedTextCtrl<double> *sub_th_ctrl;
        NamedTextCtrl<double> *sub_scatt_ctrl;
        NamedTextCtrl<double> *thickness_ctrl;

        // Graphs

        Graph *graph;

        // Optimization

        std::atomic<bool> optimizing;
        NamedTextCtrl<double> *x_min,*x_max;

        std::thread *optim_thread;
        wxTimer *timer;
        OptimEngine optim_engine;

        LayerFitter(wxString const &title);
        ~LayerFitter();
        
        void add_dielec_element(PanelsList<ModelPanel2> *panel);
        void compute_response(bool silent=true);
        void evt_add_dielec_layer(wxCommandEvent &event);
        void evt_add_dielec_substrate(wxCommandEvent &event);
        void evt_add_dielec_environment(wxCommandEvent &event);
        void evt_export(wxCommandEvent &event);
        void evt_update(wxCommandEvent &event);
        void evt_update_ui(wxCommandEvent &event);
        void evt_optimize(wxCommandEvent &event);
        void evt_timed_refresh(wxTimerEvent &event);
        void get_data_fname(wxCommandEvent &event);
        void threaded_optimization();
};


#endif // GUI_LAYERFITTER_H_INCLUDED
