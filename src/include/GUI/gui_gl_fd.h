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

#ifndef GUI_GL_FD_H_INCLUDED
#define GUI_GL_FD_H_INCLUDED

#include <gl_utils.h>
#include <gui_gl.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <mesh_base.h>
#include <mesh_tools.h>

class Basic_VAO
{
    public:
        bool display,wireframe;
        int Nf,Ne,ID;
        double lx,ly,lz,scale;
        Vector3 color;
        Vector3 O,A,B,C;
        Vector3 uO,uA,uB,uC;
        
        GLuint vao,v_buff,vn_buff,index_buff;
        GLuint vao_w,v_buff_w,vn_buff_w,index_buff_w;
        
        Basic_VAO();
        
        void draw();
        void draw_wireframe();
        void init_opengl();
        void rescale_vectors();
        void set_matrix(Vector3 const &O,
                        Vector3 const &A,
                        Vector3 const &B,
                        Vector3 const &C);
        void set_mesh(std::string mesh_type,int disc);
        void set_mesh_solid(std::string mesh_type,int disc);
        void set_mesh_solid(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr);
        void set_mesh_wireframe(std::string mesh_type,int disc);
        void set_mesh_wireframe(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr);
        void set_shading(Vector3 const &color,bool wires,bool hide);
        void set_world(double lx,double ly,double lz,double scale);
};

class GL_FD_Base: public GL_3D_Base
{
    public:
        int Nx,Ny,Nz;
        double Dx,Dy,Dz,scale;
        double lx,ly,lz;
        
        bool pml_x,pml_y,pml_z;
        int pml_xm,pml_xp,
            pml_ym,pml_yp,
            pml_zm,pml_zp,
            pad_xm,pad_xp,
            pad_ym,pad_yp,
            pad_zm,pad_zp;
        
        GLuint prog_grid,prog_solid,prog_wires;
        GLuint uni_proj_grid;
        
        Glite::LineGrid_VAO *gxd,*gxu,*gyd,*gyu,*gzd,*gzu;
        std::vector<Basic_VAO> pml_vao,pad_vao;
        std::vector<Basic_VAO*> vao;
        
        GL_FD_Base(wxWindow *parent);
        
        void forget_all_vaos();
        void forget_vao(Basic_VAO *vao);
        void forget_vao(std::vector<Basic_VAO*> const &vao);
        virtual void init_opengl();
        void recompute_pml_matrices();
        virtual void render();
        Basic_VAO* request_vao();
        void reset_pml_display();
        void set_pml(int pml_xm,int pml_xp,int pml_ym,int pml_yp,int pml_zm,int pml_zp,
                     int pad_xm,int pad_xp,int pad_ym,int pad_yp,int pad_zm,int pad_zp,
                     bool reset_display=false);
        void set_structure(Grid3<unsigned int> const &matgrid,std::vector<Basic_VAO*> &disc_vao);
        virtual void update_grid(int Nx,int Ny,int Nz,double Dx,double Dy,double Dz);
};

#endif // GUI_GL_FD_H_INCLUDED
