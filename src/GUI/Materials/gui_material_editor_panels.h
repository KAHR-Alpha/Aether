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

#ifndef GUI_MATERIAL_EDITOR_PANELS_H_INCLUDED
#define GUI_MATERIAL_EDITOR_PANELS_H_INCLUDED

#include <material.h>

#include <gui.h>
#include <gui_panels_list.h>

wxDECLARE_EVENT(EVT_DELETE_CAUCHY,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_CRITPOINT,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_DEBYE,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_DRUDE,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_LORENTZ,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_SELLMEIER,wxCommandEvent);
wxDECLARE_EVENT(EVT_DELETE_SPLINE,wxCommandEvent);

namespace MatGUI
{
    class SubmodelPanel: public PanelsListBase
    {
        public:
            Material *material;
            
            SubmodelPanel(wxWindow *parent);
            
            void apoptose(wxCommandEvent &event);
            void evt_modification(wxCommandEvent &event);
            virtual void gui_to_mat();
            virtual void signal_type();
    };
    
    class EpsInfPanel: public SubmodelPanel
    {
        public:
            double *mat_eps_inf;
            NamedTextCtrl<double> *eps_inf;
            
            EpsInfPanel(wxWindow *parent,double *eps_inf);
            
            void gui_to_mat();
            void lock();
            void unlock();
    };
    
    class DebyePanel: public SubmodelPanel
    {
        public:
            int ID;
            DebyeModel *mat_debye;
            NamedTextCtrl<double> *ds,*t0;
            
            DebyePanel(wxWindow *parent,DebyeModel *debye,int ID);
            
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class DrudePanel: public SubmodelPanel
    {
        public:
            int ID;
            DrudeModel *mat_drude;
            WavelengthSelector *wd,*g;
            
            DrudePanel(wxWindow *parent,DrudeModel *drude,int ID);
            
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class LorentzPanel: public SubmodelPanel
    {
        public:
            int ID;
            LorentzModel *mat_lorentz;
            
            NamedTextCtrl<double> *A;
            WavelengthSelector *O,*G;
            
            LorentzPanel(wxWindow *parent,LorentzModel *lorentz,int ID);
            
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class CritpointPanel: public SubmodelPanel
    {
        public:
            int ID;
            CritpointModel *mat_critpoint;
            
            NamedTextCtrl<double> *A,*P;
            WavelengthSelector *O,*G;
            
            CritpointPanel(wxWindow *parent,CritpointModel *critpoint,int ID);
            
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class CauchyPanel: public SubmodelPanel
    {
        public:
            int ID;
            std::vector<double> *mat_coeffs;
            
            NamedTextCtrl<int> *order;
            std::vector<NamedTextCtrl<double>*> coeffs;
            
            CauchyPanel(wxWindow *parent,std::vector<double> *coeffs,int ID);
            
            void evt_order(wxCommandEvent &event);
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class SellmeierPanel: public SubmodelPanel
    {
        public:
            int ID;
            double *mat_B,*mat_C;
            
            NamedTextCtrl<double> *B;
            WavelengthSelector *C;
            
            SellmeierPanel(wxWindow *parent,double *B,double *C,int ID);
            
            void gui_to_mat();
            void lock();
            void signal_type();
            void unlock();
    };
    
    class DataPanel: public SubmodelPanel
    {
        public:
            int ID;
            std::vector<double> *lambda;
            std::vector<double> *data_r;
            std::vector<double> *data_i;
            char *index_type;
            
            Cspline *er_spline,*ei_spline;
            
            wxButton *load_btn, *edit_btn;
            
            DataPanel(wxWindow *parent,int ID,
                      std::vector<double> *lambda,
                      std::vector<double> *data_r,
                      std::vector<double> *data_i,
                      char *index_type,
                      Cspline *er_spline,
                      Cspline *ei_spline);
            
            void evt_edit(wxCommandEvent &event);
            void evt_load(wxCommandEvent &event);
            void lock();
            void signal_type();
            void unlock();
            
        private:
            void rebuild_splines();
    };
}

#endif // GUI_MATERIAL_EDITOR_PANELS_H_INCLUDED
