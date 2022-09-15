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

#ifndef GUI_SELENE_GL_H_INCLUDED
#define GUI_SELENE_GL_H_INCLUDED

#include <gui_gl_fd.h>

namespace SelGUI
{

void conic_section_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                                  double R,double K,double in_radius,double out_radius);
void cylinder_cut_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                                 double L,double r,double cut_factor);
void disk_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                         double r,double r_in);
void lens_mesh_solid(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                     unsigned int disc,double t,double r_max,double r1,double r2);
void lens_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                         unsigned int disc,double t,double r_max,double r1,double r2);
void parabolic_mirror_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                                     double focal,double thickness,double in_radius,double height);
void parabola_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,unsigned int disc,
                             double focal,double in_radius,double height);
void prism_mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                double length,double height,Angle const &a1,Angle const &a2,double width);
void prism_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                          double length,double height,Angle const &a1,Angle const &a2,double width);
void rectangle_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                              double ly,double lz);
void sphere_cut_mesh_wireframe(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                               double r,double cut_factor,bool volume_version);

class SeleneVAO
{
    public:
        bool display,wireframe;
        int Nv,Nf,Ne;
        Vector3 color;
        Vector3 O,A,B,C;
        
        GLuint vao,v_buff,vn_buff,index_buff;
        GLuint vao_w,v_buff_w,vn_buff_w,index_buff_w;
        
        SeleneVAO();
        
        void draw();
        void draw_wireframe();
        void init_opengl();
        void set_matrix(Vector3 const &O,
                        Vector3 const &A,
                        Vector3 const &B,
                        Vector3 const &C);
        void set_mesh_solid(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr);
        void set_mesh_wireframe(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr);
        void set_shading(Vector3 const &color,bool wires,bool hide);
};

class GL_Selene: public GL_3D_Base
{
    public:
        std::vector<SeleneVAO*> vao;
        
        GLuint prog_solid,prog_wires,prog_ray;
        
        // Rays
        
        int Nrays,max_gen,min_disp_gen,max_disp_gen;
        double lost_length;
        GLuint ray_vao,ray_v_buff,
               ray_offset_buff,ray_A_buff,ray_gen_buff,ray_lost_buff;
        
        GL_Selene(wxWindow *parent);
        
        void delete_vao(SeleneVAO *vao);
        void init_opengl();
        SeleneVAO* request_vao();
        void render();
        void set_rays(std::vector<double> const &x1,std::vector<double> const &x2,
                      std::vector<double> const &y1,std::vector<double> const &y2,
                      std::vector<double> const &z1,std::vector<double> const &z2,
                      std::vector<int> const &gen,std::vector<bool> const &lost);
                      
};

class GL_Selene_Minimal: public GL_3D_Base
{
    public:
        SeleneVAO vao;
        
        GLuint prog_solid,prog_wires;
        
        GL_Selene_Minimal(wxWindow *parent);
        
        void init_opengl();
        SeleneVAO* get_vao();
        void render();
};

}

#endif // GUI_SELENE_GL_H_INCLUDED
