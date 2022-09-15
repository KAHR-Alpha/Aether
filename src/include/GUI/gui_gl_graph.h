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

#ifndef GUI_GL_GRAPH_H_INCLUDED
#define GUI_GL_GRAPH_H_INCLUDED

#include <GL/gl3w.h>

#include <bitmap3.h>
#include <gl_utils.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>

class GL_2D_graph: public wxGLCanvas
{
    public:
        double data_min,data_max;
        Grid2<double> data;
        
        int gradient;
        bool busy,gl_ok,show_colorbar;
        wxGLContext *glcnt;
        wxTimer *timer;
        
        int N1,N2;
        GLuint vao,vertex_buff,uv_buff,index_buff;
        GLuint texture_rgb;
        
        GLfloat *v_arr;
        GLfloat *map;
        
        GLuint prog_planes,
               uni_proj_planes,
               sampler;
               
        // Colorbar
        
        GLuint vao_colorbar,
               program_colorbar;
        
        // Text
        
        Glite::Text gl_text;
        
        GL_2D_graph(wxWindow *parent);
        ~GL_2D_graph();
        
        void base_render();
        void enable_colorbar(bool value=true);
        void evt_initialization_check(wxTimerEvent &event);
        void evt_paint_refresh(wxPaintEvent &event);
        void evt_resize(wxSizeEvent &event);
        void focus();
        void init_opengl();
        void reallocate(int N1,int N2);
        void refresh_from_bitmap();
        void render();
        void set_gradient(int gradient);
        void set_map(int N1,int N2,GLfloat *map);
};

#endif // GUI_GL_GRAPH_H_INCLUDED
