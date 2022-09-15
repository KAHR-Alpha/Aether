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

#ifndef GUI_GL_TEST_H_INCLUDED
#define GUI_GL_TEST_H_INCLUDED

#include <gui.h>
#include <gui_material.h>
#include <gui_panels_list.h>


#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <mesh_base.h>
#include <mesh_tools.h>

class TestGL: public wxGLCanvas
{
    public:
        bool busy;
        wxGLContext *glcnt;
        wxTimer *timer;
        
        int sx,sy;
        double g_xp,g_xm,g_yp,g_ym;
        double xmin,xmax,ymin,ymax;
        
        int mouse_x,mouse_xp;
        int mouse_y,mouse_yp;
        
        GLfloat proj_gl[16];
        
        GLuint vao,prog;
        
        TestGL(wxWindow *parent);
        ~TestGL();
        
        void evt_mouse_motion(wxMouseEvent &event);
        double mx_to_nr(int i);
        double my_to_ni(int j);
        void paint_refresh(wxPaintEvent &event);
        void render();
        void resize(wxSizeEvent &event);
        void timed_refresh(wxTimerEvent &event);
        void update_camera();
        void update_size();
};

class TestGLFrame: public BaseFrame
{
    public:
        TestGL *target_graph;
        
        TestGLFrame(wxString const &title);
};

#endif // GUI_GL_TEST_H_INCLUDED
