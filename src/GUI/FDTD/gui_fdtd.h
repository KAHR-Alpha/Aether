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

#ifndef GUI_FDTD_H_INCLUDED
#define GUI_FDTD_H_INCLUDED

#include <gui.h>
#include <gui_material_library.h>
#include <gui_panels_list.h>
#include <gl_utils.h>
#include <gui_gl_fd.h>
#include <lua_fdtd.h>
#include <mathUT.h>

#include <wx/treectrl.h>

#include <Eigen/Eigen>

#include <atomic>

class FD_Boundary_Panel: public wxPanel
{
    public:
        wxChoice *type_choice;
        NamedTextCtrl<int> *N_ctrl_m,*N_ctrl_p,
                           *padding_m,*padding_p;
        NamedTextCtrl<double> *k_max_ctrl_m,*k_max_ctrl_p,
                              *s_max_ctrl_m,*s_max_ctrl_p,
                              *a_max_ctrl_m,*a_max_ctrl_p;
        
        FD_Boundary_Panel(wxWindow *parent,std::string const &base_name,bool periodic,
                          int N_pml_m,double k_max_m,double s_max_m,double a_max_m,int padding_m,
                          int N_pml_p,double k_max_p,double s_max_p,double a_max_p,int padding_p);
        
        void evt_padding(wxCommandEvent &event);
        void evt_periodic(wxCommandEvent &event);
        void get_parameters(bool &periodic,
                            int &N_pml_m,double &k_max_m,double &s_max_m,double &a_max_m,int &padding_m,
                            int &N_pml_p,double &k_max_p,double &s_max_p,double &a_max_p,int &padding_p);
        
        void layout_periodic();
};

namespace GUI
{
    class FDTD_Mode: public ::FDTD_Mode
    {
        public:
            std::vector<GUI::Material*> g_materials;
            
            void consolidate_materials()
            {
                materials.resize(g_materials.size());
                
                for(std::size_t i=0;i<materials.size();i++)
                {
                    ::Material *c_mat=dynamic_cast<::Material*>(g_materials[i]);
                    materials[i]=*c_mat;
                }
            }
    };
    
    int fdtd_compute(lua_State *L);
    int fdtd_set_material(lua_State *L);
}

class FDTD_Mode_Dialog: public wxDialog
{
    public:
        GUI::FDTD_Mode *data;
        bool new_structure,force_xy_periodic;
        int N_panels;
        
        // Main
        NamedTextCtrl<std::string> *name;
        NamedTextCtrl<std::string> *output_directory;
        
        NamedTextCtrl<int> *Nt_max;
        wxChoice *Nt_type;
        wxChoice *Nt_layout_x,
                 *Nt_layout_y,
                 *Nt_layout_z;
        WavelengthSelector *Nt_lmin,*Nt_lmax;
        NamedTextCtrl<double> *Nt_level;
        NamedTextCtrl<int> *Nt_points;
        
        // Structure
        NamedTextCtrl<std::string> *structure;
        wxButton *structure_edit_btn;
        NamedTextCtrl<double> *dx_ctrl,*dy_ctrl,*dz_ctrl;
        
        // Incidence
        wxChoice *polarization;
        SpectrumSelector *inc_spectrum;
        
        // Materials
        wxScrolledWindow *mats_panel;
        PanelsList<> *mats_list;
        
        // Boundaries
        wxScrolledWindow *boundaries_panel;
        
        FD_Boundary_Panel *boundary_x,*boundary_y,*boundary_z;
                 
        
        FDTD_Mode_Dialog(GUI::FDTD_Mode *data,int target_panel);
        
        // Subconstructors
        void FDTD_Mode_Dialog_Boundaries(wxNotebook *book,int target_panel);
        void FDTD_Mode_Dialog_Incidence(wxNotebook *book,int target_panel);
        void FDTD_Mode_Dialog_Main(wxNotebook *book,int target_panel);
        void FDTD_Mode_Dialog_Materials(wxNotebook *book,int target_panel);
        void FDTD_Mode_Dialog_Structure(wxNotebook *book,int target_panel);
        
        
        void evt_add_material(wxCommandEvent &event);
        void evt_cancel(wxCommandEvent &event);
        void evt_edit_structure(wxCommandEvent &event);
        void evt_load_structure(wxCommandEvent &event);
        void evt_material_change(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_output_directory(wxCommandEvent &event);
        void evt_time(wxCommandEvent &event);
        void layout_adaptive_time();
        void layout_fixed_time();
        void rename_materials();
};

class FDTD_Run_Dialog: public wxDialog
{
    public:
        std::atomic<bool> end_computation,
                          computation_done;
        
        ProgTimeDisp dsp;
        std::thread *thread;
        
        NamedTextCtrl<std::string> *text_iterations,
                                   *text_time_spent,
                                   *text_time_remaing,
                                   *text_time_total;
        wxGauge *progression_ctrl;
        
        GL_2D_display *display;
        
        wxTimer *timer;
        wxButton *ok_btn,*cancel_btn;
        
        FDTD_Run_Dialog(wxWindow *parent,GUI::FDTD_Mode const &data);
        ~FDTD_Run_Dialog();
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_timed_refresh(wxTimerEvent &event);
        void run_computation(GUI::FDTD_Mode const &data);
};

class Sensor_Gen_Dialog: public Sensor_generator, public wxDialog
{
    public:
        Sensor_generator *generator_holder;
        bool selection_ok;
        
        wxChoice *type_ctrl;
        NamedTextCtrl<std::string> *name_ctrl;
        SpectrumSelector *spectrum_ctrl;
        
        wxChoice *location_type_ctrl;
        NamedTextCtrl<double> *x1_ctrl,*x2_ctrl,
                              *y1_ctrl,*y2_ctrl,
                              *z1_ctrl,*z2_ctrl;
        wxChoice *orientation_ctrl;
        std::vector<wxCheckBox*> disable_plane_ctrl;
        
        Sensor_Gen_Dialog(Sensor_generator *generator);
        
        bool is_planar();
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_orientation(wxCommandEvent &event);
        void evt_type(wxCommandEvent &event);
        void refresh_layout();
        void save();
};

class Source_Gen_Dialog: public Source_generator, public wxDialog
{
    public:
        Source_generator *generator_holder;
        bool selection_ok;
        
        wxChoice *type_ctrl;
        WavelengthSelector *lambda_min_ctrl,
                           *lambda_max_ctrl;
        
        NamedTextCtrl<double> *x1_ctrl,*x2_ctrl,
                              *y1_ctrl,*y2_ctrl,
                              *z1_ctrl,*z2_ctrl;
        wxChoice *orientation_ctrl;
        
        Source_Gen_Dialog(Source_generator *generator);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_type(wxCommandEvent &event);
        void refresh_layout();
        void save();
};

class FDTD_Disp_Dialog: public wxDialog
{
    public:
        std::vector<Basic_VAO*> vao;
        bool &pml_x,&pml_y,&pml_z;
        
        wxCheckBox *pml_x_ctrl,
                   *pml_y_ctrl,
                   *pml_z_ctrl;
        
        std::vector<wxCheckBox*> wireframe,show;
        std::vector<wxButton*> color;
        
        FDTD_Disp_Dialog(wxFrame *parent,std::vector<Basic_VAO*> &vao,
                         bool &pml_x,bool &pml_y,bool &pml_z);
        
        void evt_color(wxCommandEvent &event);
        void evt_pml(wxCommandEvent &event);
        void evt_show(wxCommandEvent &event);
        void evt_wireframe(wxCommandEvent &event);
};

class FDTD_GL: public GL_FD_Base
{
    public:
        Basic_VAO sensor_vao,source_vao;
        std::vector<double> sensor_xm,sensor_xp,
                            sensor_ym,sensor_yp,
                            sensor_zm,sensor_zp;
        
        std::vector<int> source_orientation;
        std::vector<double> source_xm,source_xp,
                            source_ym,source_yp,
                            source_zm,source_zp;
        
//        std::vector<Basic_VAO*> source_vao;
        
        FDTD_GL(wxWindow *parent);
        
        void add_sensor(Sensor_generator const &generator);
        void add_source(Source_generator const &generator);
        void delete_sensor(int sensor_ID);
        void delete_source(int sensor_ID);
        void forget_sensors();
        void forget_sources();
        void init_opengl();
        void render();
        void update_sensor(int sensor_ID,Sensor_generator const &generator);
        void update_source(int source_ID,Source_generator const &generator);
};

class FDTD_Frame: public BaseFrame
{
    public:
        int Nx,Ny,Nz;
        double Dx,Dy,Dz;
        Grid3<unsigned int> matsgrid;
        
        GUI::FDTD_Mode fdtd_parameters;
        
        wxScrolledWindow *ctrl_panel;
        wxChoice *fdtd_type;
        
        wxTreeCtrl *tree;
        wxTreeItemId root_ID,
                     structure_ID,
                     incidence_ID,
                     boundaries_ID,
                     materials_ID,
                     sources_root_ID,
                     sensors_root_ID,
                     focus_ID;
        
        wxMenu fdtd_menu,
               sources_root_menu,
               sources_menu,
               sources_restricted_menu,
               sensors_root_menu,
               sensors_menu,
               sensors_restricted_menu;
        
        std::vector<wxTreeItemId> sources_ID,sensors_ID;
        
        FDTD_GL *gl;
        std::vector<Basic_VAO*> disc_vao,sensor_vao,source_vao;
        FDTD_Disp_Dialog *disp_options_dialog;
        
        FDTD_Frame(wxString const &title);
        ~FDTD_Frame();
        
        void append_tree_pml(wxTreeItemId const &ID,std::string const &name,
                             int N,double kappa,double sigma,double alpha,int pad);
        void append_tree_sensor(wxTreeItemId const &ID,Sensor_generator &generator);
        void append_tree_source(wxTreeItemId const &ID,Source_generator &generator);
        void evt_menu(wxCommandEvent &event);
        void evt_options_dialog_close(wxCloseEvent &event);
        void evt_popup_menu(wxCommandEvent &event);
        void evt_run(wxCommandEvent &event);
        void evt_tree(wxTreeEvent &event);
        void evt_type(wxCommandEvent &event);
        void load(wxFileName const &fname);
        void reconstruct_sensor_vaos();
        void reconstruct_source_vaos();
        void reconstruct_tree();
        void refresh_type_ctrl();
        void save(wxFileName const &fname);
        void subevt_menu_load();
        void subevt_menu_new();
        void subevt_menu_save();
        void subevt_menu_save_as();
        void subevt_menu_restore();
        void update_gl_pmls();
};

#endif // GUI_FDTD_H_INCLUDED
