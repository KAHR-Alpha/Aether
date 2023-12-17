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

#ifndef GUI_CURVE_EXTRACTOR_H_INCLUDED
#define GUI_CURVE_EXTRACTOR_H_INCLUDED

#include <gui.h>
#include <gui_gl.h>

class GL_Curve_Extract: public wxGLCanvas
{
    public:
        bool busy;
        int N1,N2;
        int selected_point;
        double zoom_factor,x_factor,y_factor;
        double camera_x,camera_y;
        double mouse_x,mouse_xp,
               mouse_y,mouse_yp;
        
        std::vector<bool> anchors_lock;
        std::vector<std::vector<double>> anchors_x,anchors_y;
        
        GLfloat *map;
        
        wxGLContext *glcnt;
        wxTimer *timer;
        
        GLuint vao,vertex_buff,uv_buff,index_buff;
        GLuint texture_rgb;
        
        GLuint program_image,
               uniform_proj_mat,
               sampler;
        
        GL_Curve_Extract(wxWindow *parent);
        ~GL_Curve_Extract();
        
        void add_curve();
        void deep_render();
        void delete_curve(int i);
        void evt_keyboard(wxKeyEvent &event);
        void evt_mouse_left(wxMouseEvent &event);
        void evt_mouse_motion(wxMouseEvent &event);
        void evt_mouse_wheel(wxMouseEvent &event);
        void evt_resize(wxSizeEvent &event);
        void evt_timer(wxTimerEvent &event);
        void render();
        void set_image_data(int height,int width,unsigned char *data);
};

class Curve_Extract_Frame: public BaseFrame
{
    public:
        int Nx,Ny;
        wxScrolledWindow *ctrl_panel;
        GL_Curve_Extract *gl;
        
        NamedTextCtrl<std::string> *img_fname_ctrl;
        NamedTextCtrl<int> *width_ctrl,*height_ctrl;
        wxImage base_image;
        
        Curve_Extract_Frame(wxString const &title);
        ~Curve_Extract_Frame();
        
        void evt_load_btn(wxCommandEvent &event);
};

#endif // GUI_CURVE_EXTRACTOR_H_INCLUDED
