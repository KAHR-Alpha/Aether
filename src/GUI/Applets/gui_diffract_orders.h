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

#ifndef GUI_DIFFRACT_ORDERS_H_INCLUDED
#define GUI_DIFFRACT_ORDERS_H_INCLUDED

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

class GL_DiffOrders: public GL_3D_Base
{
    public:
        bool display_superstrate,display_substrate;
        double R_sup,R_sub;
        Vector3 beta_u,beta_v,beta_w;
        std::vector<Vector3> u_sup,v_sup,w_sup;
        std::vector<Vector3> u_sub,v_sub,w_sub;
        
        GLuint prog_orders,vao_orders;
        GLuint prog_circles,vao_circles;
        
        GL_DiffOrders(wxWindow *parent);
        
        void init_opengl();
        void render();
        void set_beta(Vector3 const &beta,double k0);
        void set_display(bool display_superstrate,bool display_substrate);
        void set_R(double R_sup,double R_sub);
        void update_orders(std::vector<double> const &kx_sup,
                           std::vector<double> const &ky_sup,
                           std::vector<double> const &kz_sup,
                           std::vector<double> const &kx_sub,
                           std::vector<double> const &ky_sub,
                           std::vector<double> const &kz_sub);
};

class DiffOrdersSDFrame: public BaseFrame
{
    public:
        Vector3 a1,a2,
                b1,b2,Gn;
        
        SpectrumSelector *spectrum;
        
        MiniMaterialSelector *superstrate_selector,
                             *substrate_selector;
        
        LengthSelector *a1_length_ctrl;
        LengthSelector *a2_length_ctrl;
        NamedTextCtrl<double> *a1_ang_ctrl;
        NamedTextCtrl<double> *a2_ang_ctrl;
        
        NamedTextCtrl<double> *max_incidence_ctrl;
        
        NamedTextCtrl<std::string> *min_a1,*min_a2;
        
        Graph *graph;
        
        std::vector<double> lambda,
                            s_factor_1_sup,s_factor_1_sub,
                            s_factor_2_sup,s_factor_2_sub;
        
        DiffOrdersSDFrame(wxString const &title);
        DiffOrdersSDFrame(wxString const &title,
                          double lambda_min,double lambda_max,
                          Vector3 const &a1,Vector3 const &a2);
        
        void evt_incidence(wxCommandEvent &event);
        void evt_material(wxCommandEvent &event);
        void evt_spectrum(wxCommandEvent &event);
        void evt_vectors(wxCommandEvent &event);
        void recompute();
        void set_materials(MiniMaterialSelector *superstrate,
                           MiniMaterialSelector *substrate);
        void update_vectors();
};

class DiffOrdersFrame: public BaseFrame
{
    public:
        bool normalized,mode_multi;
        int Nl;
        double lambda;
        double lambda_min,lambda_max;
        double c_factor;
        Vector3 beta,
                a1,a2,
                b1,b2,
                p1,p2;
        AngleRad phi,theta;
        
        std::vector<double> kx_sup,ky_sup,kz_sup;
        std::vector<double> kx_sub,ky_sub,kz_sub;
        
        wxScrolledWindow *ctrl_panel;
        wxPanel *display_panel;
        
        GL_DiffOrders *gl;
        Graph *orders_graph;
        
        wxButton *surface_modes_btn;
        std::vector<double> N_ord_lambda,N_ord_sup,N_ord_sub;
        
        WavelengthSelector *lambda_ctrl;
        SpectrumSelector *spectrum_ctrl;
        SliderDisplay *theta_ctrl,*phi_ctrl;
        
        LengthSelector *a1_length_ctrl;
        LengthSelector *a2_length_ctrl;
        NamedTextCtrl<double> *a1_ang_ctrl;
        NamedTextCtrl<double> *a2_ang_ctrl;
        
        MiniMaterialSelector *superstrate_selector;
        MiniMaterialSelector *substrate_selector;
        
        wxCheckBox *normalized_ctrl,
                   *display_superstrate_ctrl,
                   *display_substrate_ctrl;
        
        DiffOrdersFrame(wxString const &title);
        
        void evt_a1(wxCommandEvent &event);
        void evt_a2(wxCommandEvent &event);
        void evt_angle(wxCommandEvent &event);
        void evt_display_switch(wxCommandEvent &event);
        void evt_lambda(wxCommandEvent &event);
        void evt_materials(wxCommandEvent &event);
        void evt_mode_switch(wxCommandEvent &event);
        void evt_normalize(wxCommandEvent &event);
        void evt_spectrum(wxCommandEvent &event);
        void evt_subdiff(wxCommandEvent &event);
        void evt_surface_modes(wxCommandEvent &event);
        void layout_mono();
        void layout_multi();
        void recompute();
        void update_vectors();
        void update_beta();
};


#endif // GUI_DIFFRACT_ORDERS_H_INCLUDED
