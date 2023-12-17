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

#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <mesh_base.h>
#include <mesh_tools.h>

#include <Eigen/Eigen>

#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Glite
{
    class Camera
    {
        public:
            double clip_min,clip_max;
            double h_ang,v_ang;
            Vector3 Cu,Cv,Cw,Co;
            Eigen::Matrix4d rot_mat,C_mat_inv,C_dsp,proj_mat;
            GLfloat proj_gl[16];
            
            Camera();
            
            void clip(double clip_min_i,double clip_max_i);
            void look_at(Vector3 const &V);
            void set_location(double x,double y,double z);
            void set_location(Vector3 const &V);
            void update_proj_angles(double Ch_ang,double Cv_ang);
            void update_proj_mat();
            void update_loc_mat();
    };
    
    class LineGrid_VAO
    {
        public:
            int Nu,Nv,NVtx;
            Vector3 P,U,V;
            
            GLuint vao,v_buff;
            
            LineGrid_VAO();
            
            void draw();
            void set_grid(int Nu,int Nv,Vector3 const &P,
                                        Vector3 const &U,
                                        Vector3 const &V);
            void set_location(Vector3 const &P);
    };
    
    class VAO
    {
        public:
            double x,y,z;
            GLuint vao,v_buff,vn_buff,index_buff;
            
            int Nv;
            int Nf;
            Grid1<Vertex> V_arr;
            Grid1<Face> F_arr;
            
            GLfloat proj_gl[16];
            
            VAO();
            
            void draw();
            void set_location(double x,double y,double z);
            void set_location(Vector3 const &V);
            void set_mesh(Grid1<Vertex> &V_arr,Grid1<Face> &F_arr);
    };
    
    void check_shader_compilation(GLuint shader);
    GLuint create_program(std::filesystem::path const &v_fname,
                          std::filesystem::path const &f_fname);
    void load_shader_file(GLuint shader,std::filesystem::path const &fname);
    
    class CSphere_Indexer
    {
        public:
            int N,Nm;
            double Da;
            int offset_1,offset_2,offset_3,offset_4,offset_5;
            
            CSphere_Indexer(int N);
            
            int get_total();
            int index_0(int i,int j);
            int index_1(int i,int j);
            int index_2(int i,int j);
            int index_3(int i,int j);
            int index_4(int i,int j);
            int index_5(int i,int j);
            Vector3 vertex_location(int i,int j,int f);
    };
    
    void make_block(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                    double lx,double ly,double lz,
                    double off_x=0,double off_y=0,double off_z=0);
    void make_block_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                          double lx,double ly,double lz,
                          double off_x=0,double off_y=0,double off_z=0);
    void make_unitary_block(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
    void make_unitary_block_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
    void make_unitary_cone(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_cone_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_csphere(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_csphere_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_cylinder(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_cylinder_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_disk(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc);
    void make_unitary_square(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
    void make_unitary_square_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
    
    class Text
    {
        public:
            int N_inst,N_max;
            int Sx,Sy; // Screen
            
            FT_Library library;
            FT_Face face;
            
            std::vector<char> char_ID;
            std::vector<int> char_code,ID_buffer;
            std::vector<GLfloat> glyph_sx,glyph_sy,glyph_offset_x,glyph_offset_y,glyph_advance,
                                 texture_u,texture_v,texture_us,texture_vs;
            
            GLuint vao,prog,sampler,map_tex;
            GLfloat *world_pos,
                    *screen_offset,*screen_scale,
                    *uv_offset,*uv_scale;
            GLfloat screen_matrix[16];
            GLuint world_pos_buffer,
                   screen_offset_buffer,screen_scale_buffer,
                   uv_offset_buffer,uv_scale_buffer;
            
            Text();
            ~Text();
            
            void add_character(char c,int code);
            void compute_atlas();
            int get_ID(char c);
            void initialize();
            void render();
            void render(GLfloat *proj_matrix);
            void request_text(double x,double y,double z,
                              std::string const &text,
                              double scale_x ,double scale_y ,
                              double offset_x,double offset_y);
            void request_text(double x,double y,double z,
                              std::string const &text,
                              double scale_x ,double scale_y ,
                              double offset_x,double offset_y,
                              double align_x ,double align_y);
            void set_screen(int Sx,int Sy);
    };
    
    void set_vector(GLfloat *glarray,int i,double x,double y,double z,double w=1.0);
}

#endif // GL_UTILS_H
