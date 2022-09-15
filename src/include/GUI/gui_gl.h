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

#ifndef GUI_GL_H_INCLUDED
#define GUI_GL_H_INCLUDED

#include <bitmap3.h>
#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <mesh_base.h>
#include <mesh_tools.h>

class GL_2D_display: public wxGLCanvas
{
    private:
        bool waiting_bitmap_refresh;
        void base_render();
        void focus();
        void init_opengl();
        void reallocate(int N1,int N2);
        void set_map(int N1,int N2,GLfloat *map);
        virtual void render();
        void evt_paint_refresh(wxPaintEvent &event);
        virtual void evt_resize(wxSizeEvent &event);
        void evt_timed_refresh(wxTimerEvent &event);
    public:
        Bitmap bitmap;
        
        bool busy,gl_ok;
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
        
        GL_2D_display(wxWindow *parent);
        ~GL_2D_display();
        void refresh_from_bitmap();
};

class GL_3D_Base: public wxGLCanvas
{
    public:
        bool busy;
        bool gl_ok;
        wxGLContext *glcnt;
        wxTimer *timer;
        
        int mouse_x,mouse_xp;
        int mouse_y,mouse_yp;
        double cam_r,cam_ph,cam_th;
        Vector3 cam_t;
        
        Glite::Camera camera;
        
        GL_3D_Base(wxWindow *parent);
        ~GL_3D_Base();
        
        void base_render();
        void evt_mouse_motion(wxMouseEvent &event);
        void evt_mouse_wheel(wxMouseEvent &event);
        virtual bool evt_mouse_motion_intercept(wxMouseEvent &event);
        virtual bool evt_mouse_wheel_intercept(wxMouseEvent &event);
        void evt_shown(wxWindowCreateEvent &event);
        void focus();
        virtual void init_opengl();
        virtual void render();
        virtual void resize(wxSizeEvent &event);
        void set_camera_direction(Vector3 const &dir);
        void set_camera_target(Vector3 const &target);
        void set_camera_radius(double radius);
        void timed_refresh(wxTimerEvent &event);
        void paint_refresh(wxPaintEvent &event);
        void update_camera();
        void update_viewport();
};

#endif // GUI_GL_H_INCLUDED
