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

#ifndef GUI_FIELDBLOCK_EXPLR_H_INCLUDED
#define GUI_FIELDBLOCK_EXPLR_H_INCLUDED

#include <fieldblock_holder.h>
#include <gui.h>
#include <gui_gl_fd.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>

class PlaneMap_VAO
{
    public:
        int N1,N2;
        GLuint vao,vertex_buff,uv_buff,index_buff;
        GLuint texture_rgb,texture_data;
        
        GLfloat *v_arr;
        
        PlaneMap_VAO(int N1,int N2);
        ~PlaneMap_VAO();
        
        void draw();
        void draw_data();
        void init_opengl();
        void reallocate(int N1,int N2);
        void set_data(int N1,int N2,GLfloat *map);
        void set_map(int N1,int N2,GLfloat *map);
        void set_vertices(Vector3 const &V1,
                          Vector3 const &V2,
                          Vector3 const &V3,
                          Vector3 const &V4);
                          
};

class NamedTextDisp: public wxPanel
{
    public:
        wxStaticText *name;
        wxTextCtrl *data;
        
        NamedTextDisp(wxWindow *parent,std::string const &name,std::string const &data);
        
        template<class T>
        void set_value(T const &value)
        {
            std::stringstream strm;
            strm<<value;
            
            data->SetValue(wxString(strm.str()));
        }
        
        void set_value(std::string const &value);
        void set_value(wxString const &value);
};

class GL_FBExplr: public GL_FD_Base
{
    public:
        bool dxy,dxz,dyz;
        int pxy,pxz,pyz;
        double val_pick;
        
        GLuint prog_planes,
               prog_data,
               uni_proj_planes,
               uni_proj_data,
               sampler;
        
        PlaneMap_VAO mxy,mxz,myz;
        
        GL_FBExplr(wxWindow *parent);
        
        void get_value(wxMouseEvent &event);
        void init_opengl();
        void keyboard(wxKeyEvent &event);
        
        void render();
        
        void set_data_xy(int Nx,int Ny,GLfloat *data);
        void set_data_xz(int Nx,int Nz,GLfloat *data);
        void set_data_yz(int Ny,int Nz,GLfloat *data);
        void set_map_xy(int Nx,int Ny,GLfloat *map);
        void set_map_xz(int Nx,int Nz,GLfloat *map);
        void set_map_yz(int Ny,int Nz,GLfloat *map);
        void set_pxy(int z);
        void set_pxz(int y);
        void set_pyz(int x);
        
        void switch_pxy(bool val);
        void switch_pxz(bool val);
        void switch_pyz(bool val);
        
        void update_grid(int Nx,int Ny,int Nz,double Dx,double Dy,double Dz);
        
        void update_mxy();
        void update_mxz();
        void update_myz();
};

class FBESave_Dialog: public wxDialog
{
    public:
        bool *confirm;
        int *plane;
        std::string *name;
        
        wxRadioBox *choice;
        
        FBESave_Dialog(wxWindow *parent,int *plane,std::string *name,bool *confirm);
        
        void cancel(wxCommandEvent &event);
        void ok(wxCommandEvent &event);
};

class FieldBlockExplorer: public BaseFrame
{
    public:
        bool dxy,dxz,dyz;
        int Nx,Ny,Nz;
        double Dx,Dy,Dz;
        double baseline;
        
        FieldBlockHolder fbh;
        double clamp_max;
        double E_max,Ex_max,Ey_max,Ez_max;
        
        GLfloat *data_xy,*data_xz,*data_yz;
        GLfloat *map_xy,*map_xz,*map_yz;
        
        NamedTextDisp *lambda_disp,
                      *Nx_disp,
                      *Ny_disp,
                      *Nz_disp,
                      *Dx_disp,
                      *Dy_disp,
                      *Dz_disp;
                      
        wxSpinCtrl *pxy_ctrl,
                   *pxz_ctrl,
                   *pyz_ctrl;
        
        wxCheckBox *dxy_ctrl,
                   *dxz_ctrl,
                   *dyz_ctrl;
        
        
        NamedTextCtrl<double> *baseline_ctrl,*clamp_ctrl;
        
        
        enum
        {
            DISP_E=0,
            DISP_EX_ABS, DISP_EX_ARG,
            DISP_EY_ABS, DISP_EY_ARG,
            DISP_EZ_ABS, DISP_EZ_ARG,
        };
        
        int field_display_type;
        wxChoice *field_display,*cfield_display;
        
        GL_FBExplr *gl_disp;
        wxTextCtrl *gl_data;
        
        FieldBlockExplorer(wxString const &title);
        ~FieldBlockExplorer();
        
        void gl_pick(wxCommandEvent &event);
        
        void decrement_pxy(wxCommandEvent &event);
        void decrement_pxz(wxCommandEvent &event);
        void decrement_pyz(wxCommandEvent &event);
        
        void evt_baseline(wxCommandEvent &event);
        void evt_clamp(wxCommandEvent &event);
        void evt_field_display(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void extract_map();
        
        void increment_pxy(wxCommandEvent &event);
        void increment_pxz(wxCommandEvent &event);
        void increment_pyz(wxCommandEvent &event);
        
        void load(wxCommandEvent &event);
        void load_project(wxFileName const &fname);
        void save_project(wxFileName const &fname);
        
        void switch_pxy(wxCommandEvent &event);
        void switch_pxz(wxCommandEvent &event);
        void switch_pyz(wxCommandEvent &event);
        
        void update_map_xy();
        void update_map_xz();
        void update_map_yz();
        
        void update_pxy(wxSpinEvent &event);
        void update_pxz(wxSpinEvent &event);
        void update_pyz(wxSpinEvent &event);
};

#endif // GUI_FIELDBLOCK_EXPLR_H_INCLUDED
