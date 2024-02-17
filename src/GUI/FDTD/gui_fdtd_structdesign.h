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

#ifndef GUI_FDTD_STRUCTDESIGN_H
#define GUI_FDTD_STRUCTDESIGN_H

#include <mathUT.h>

#include <gl_utils.h>
#include <gui.h>
#include <gui_gl_fd.h>
#include <gui_panels_list.h>
#include <gui_vector3ctrl.h>

#include <Eigen/Eigen>

#include <wx/choicdlg.h>
#include <wx/minifram.h>
#include <wx/splitter.h>
#include <wx/treelist.h>

class EMGeometry_GL: public GL_3D_Base
{
    public:
        double lx,ly,lz,scale;
                
        GLuint prog_grid,prog_solid,prog_wires;
        GLuint uni_proj_grid;
        
        Glite::LineGrid_VAO *gxd,*gxu,*gyd,*gyu,*gzd,*gzu;
        std::vector<Basic_VAO*> vao;
        
        EMGeometry_GL(wxWindow *parent,
                      double lx,
                      double ly,
                      double lz);
        
        void forget_all_vaos();
        void forget_vao(Basic_VAO *vao);
        void forget_vao(std::vector<Basic_VAO*> const &vao);
        virtual void init_opengl();
        virtual void render();
        Basic_VAO* request_vao();
        virtual void update_grid(double lx,double ly,double lz);
};

class GeomOP_Panel: public PanelsListBase
{
    public:
        double lx,ly,lz;
        Vector3 A,B,C,O,color;
        EMGeometry_GL *engine;
        Basic_VAO *vao;
        
        wxButton *color_btn;
        wxToggleButton *wires_btn,*hide_btn;
        
        GeomOP_Panel(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        ~GeomOP_Panel();
        
        void add_solid_switch();
        void apoptose(wxCommandEvent &event);
        virtual void collapse();
        virtual void expand();
        virtual std::string get_lua();
        virtual int get_material();
        virtual void set(std::vector<std::string> const &args);
        void set_color(Vector3 const &color);
        void update_vao_display();
        void vao_color_event(wxCommandEvent &event);
        void vao_hide_event(wxCommandEvent &event);
        void vao_wires_event(wxCommandEvent &event);
        void update_geometry();
        virtual void update_vao() {};
        virtual void update_world(double lx,double ly,double lz);
};

class GOP_Block: public GeomOP_Panel
{
    public:
        NamedSymCtrl *x1,*x2,*y1,*y2,*z1,*z2;
        NamedSymCtrl *mat;
        
        GOP_Block(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        
        void collapse();
        void evt_geometry(wxCommandEvent &event);
        void expand();
        std::string get_lua();
        int get_material();
        void set(std::vector<std::string> const &args);
        void update_vao() override;
};

class GOP_Cone: public GeomOP_Panel
{
    public:
        NamedSymCtrl *Ox,*Oy,*Oz,*Hx,*Hy,*Hz,*radius;
        NamedSymCtrl *mat;
        
        GOP_Cone(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        
        void collapse();
        void evt_geometry(wxCommandEvent &event);
        void expand();
        std::string get_lua();
        int get_material();
        void set(std::vector<std::string> const &args);
        void update_vao() override;
};

class GOP_Cylinder: public GeomOP_Panel
{
    public:
        NamedSymCtrl *Ox,*Oy,*Oz,*Hx,*Hy,*Hz,*radius;
        NamedSymCtrl *mat;
        
        GOP_Cylinder(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        
        void collapse();
        void evt_geometry(wxCommandEvent &event);
        void expand();
        std::string get_lua();
        int get_material();
        void set(std::vector<std::string> const &args);
        void update_vao() override;
};

class GOP_Layer: public GeomOP_Panel
{
    public:
        wxStaticText *orient_txt;
        wxChoice *orientation;
        
        NamedSymCtrl *h1,*h2;
        NamedSymCtrl *mat;
        
        GOP_Layer(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        
        void collapse();
        void evt_geometry(wxCommandEvent &event);
        void expand();
        std::string get_lua();
        int get_material();
        void set(std::vector<std::string> const &args);
        void update_world(double lx,double ly,double lz);
        void update_vao() override;
};

class GOP_Sphere: public GeomOP_Panel
{
    public:
        NamedSymCtrl *Ox,*Oy,*Oz,*radius;
        NamedSymCtrl *mat;
        
        GOP_Sphere(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine);
        
        void collapse();
        void evt_geometry(wxCommandEvent &event);
        void expand();
        std::string get_lua();
        int get_material();
        void set(std::vector<std::string> const &args);
        void update_vao() override;
};

class EMGeometry_Frame: public BaseFrame
{
    public:
        SymNode lx,ly,lz;
        SymLib lib;
        
        std::vector<std::string> input_keys;
        std::vector<std::string> input_values;
        std::vector<SymNode*> inputs;
        
        // 
        
        // GUI Controls
        
        wxScrolledWindow *ctrl_panel;
        
        NamedSymCtrl *lx_ctrl,*ly_ctrl,*lz_ctrl;
        wxStaticBoxSizer *geom_top_sizer;
        
        NamedTextCtrl<int> *def_mat_ctrl;
        wxChoice *op_add_choice;
        
        EMGeometry_GL *gl;
        
        PanelsList<GeomOP_Panel> *op;
        
        EMGeometry_Frame(wxString const &title,wxFileName const &fname=wxFileName{});
        ~EMGeometry_Frame();
        
        template<class T>
        void add_command(std::vector<std::string> const &args)
        {
            T *panel=op->add_panel<T>(&lib,gl);
            
            panel->update_world(lx.evaluate(),
                                ly.evaluate(),
                                lz.evaluate());
            panel->set(args);
            
            ctrl_panel->FitInside();
            Layout();
            
            ctrl_panel->FitInside();
            Refresh();
            
            refit();
        }
        
        void autocolor();
        void evt_add_operation(wxCommandEvent &event);
        void evt_autocolor(wxCommandEvent &event);
        void evt_inputs(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_operation_down(wxCommandEvent &event);
        void evt_operation_up(wxCommandEvent &event);
        void evt_remove_operation(wxCommandEvent &event);
        void evt_update_grid(wxCommandEvent &event);
        void evt_variables(wxCommandEvent &event);
        void load_project(wxFileName const &fname);
        void save_project(wxFileName const &fname);
        void refit();
        void evt_refit(wxCommandEvent &event);
        void update_geometry();
        void update_grid();
};

#endif // GUI_FDTD_STRUCTDESIGN_H
