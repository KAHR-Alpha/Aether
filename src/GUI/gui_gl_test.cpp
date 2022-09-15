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

#include <filehdl.h>

#include <gui_gl_test.h>

extern std::ofstream plog;

TestGL::TestGL(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     busy(false),
     xmin(-0.5), xmax(1.5),
     ymin(-0.5), ymax(1.5),
     mouse_x(0), mouse_xp(0),
     mouse_y(0), mouse_yp(0)
{
    glcnt=new wxGLContext(this);
    SetCurrent(*glcnt);
    Show(true);
    
    timer=new wxTimer(this,0);
    
    busy=0;
    
    wxCHECK_RET(gl3wInit()==0,"Couldn't initialize gl3w");
    wxCHECK_RET(gl3wIsSupported(4,3)==1,"OpenGL 4.3 not supported, please update your graphics drivers");
    
    // Borders
    
    prog=Glite::create_program(PathManager::locate_resource("resources/glsl/test_vshader.glsl"),
                               PathManager::locate_resource("resources/glsl/test_fshader.glsl"));
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    GLuint vbuff;
    glGenBuffers(1,&vbuff);
    
    GLfloat *data_v=new GLfloat[24];
    
    double s=1;
    double z=0;
    
    data_v[ 0]=0; data_v[ 1]=0; data_v[ 2]=z; data_v[ 3]=1.0;
    data_v[ 4]=+s; data_v[ 5]=0; data_v[ 6]=z; data_v[ 7]=1.0;
    data_v[ 8]=0; data_v[ 9]=+s; data_v[10]=z; data_v[11]=1.0;
    
    data_v[12]=+s; data_v[13]=0; data_v[14]=z; data_v[15]=1.0;
    data_v[16]=+s; data_v[17]=+s; data_v[18]=z; data_v[19]=1.0;
    data_v[20]=0; data_v[21]=+s; data_v[22]=z; data_v[23]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,vbuff);
    glBufferData(GL_ARRAY_BUFFER,24*sizeof(GLfloat),(void*)data_v,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] data_v;
    
    Bind(wxEVT_LEFT_DCLICK,&TestGL::evt_mouse_motion,this);
    Bind(wxEVT_LEFT_DOWN,&TestGL::evt_mouse_motion,this);
    Bind(wxEVT_MOTION,&TestGL::evt_mouse_motion,this);
    Bind(wxEVT_RIGHT_DOWN,&TestGL::evt_mouse_motion,this);
    Bind(wxEVT_SIZE,&TestGL::resize,this);
    Bind(wxEVT_TIMER,&TestGL::timed_refresh,this);
    timer->Start(1000/30);
    
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
    
    glDisable(GL_CULL_FACE);
}

TestGL::~TestGL()
{
    timer->Stop();
    delete glcnt;
}

void TestGL::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(event.Dragging())
    {
        if(event.LeftIsDown())
        {
            double dx=(mouse_x-mouse_xp)/(GetSize().x+0.0)*(xmax-xmin);
            double dy=(mouse_y-mouse_yp)/(GetSize().y+0.0)*(ymax-ymin);
            
            xmin-=dx; xmax-=dx;
            ymin+=dy; ymax+=dy;
            
            update_camera();
        }
    }
    
    event.Skip();
}

double TestGL::mx_to_nr(int i)
{
    double u=(i)/static_cast<double>(sx);
    return xmin+u*(xmax-xmin);
}

double TestGL::my_to_ni(int j)
{
    double v=(sy-j)/static_cast<double>(sy);
    return ymin+v*(ymax-ymin);
}

void TestGL::render()
{
    if(IsShown())
    {
        SetCurrent(*glcnt);
        if(!busy)
        {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            update_camera();
            
            glUseProgram(prog);
            glBindVertexArray(vao);
            
            glUniformMatrix4fv(1,1,0,proj_gl);
            
            glDrawArrays(GL_TRIANGLES,0,6);
            
            SwapBuffers();
        }
    }
}

void TestGL::resize(wxSizeEvent &event)
{
    sx=GetSize().x;
    sy=GetSize().y;
    
    update_size();
}

void TestGL::timed_refresh(wxTimerEvent &event)
{
    render();
}

void TestGL::paint_refresh(wxPaintEvent &event)
{
    render();
}

void TestGL::update_camera()
{
    double xs=xmax-xmin;
    double xgs=g_xp-g_xm;
    
    double ys=ymax-ymin;
    double ygs=g_yp-g_ym;
    
    proj_gl[0 ]=xgs/xs;
    proj_gl[1 ]=0;
    proj_gl[2 ]=0;
    proj_gl[3 ]=0;
    
    proj_gl[4 ]=0;
    proj_gl[5 ]=ygs/ys;
    proj_gl[6 ]=0;
    proj_gl[7 ]=0;
    
    proj_gl[8 ]=0;
    proj_gl[9 ]=0;
    proj_gl[10]=1;
    proj_gl[11]=0;
    
    proj_gl[12]=g_xm-xmin*xgs/xs;
    proj_gl[13]=g_ym-ymin*ygs/ys;
    proj_gl[14]=0;
    proj_gl[15]=1;
}

void TestGL::update_size()
{
    SetCurrent(*glcnt);
    
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    g_xm=-1.0;
    g_ym=-1.0;
    
    g_xp=+1.0;
    g_yp=+1.0;
    
    glViewport(0,0,(GLint)Sx,(GLint)Sy);
}

TestGLFrame::TestGLFrame(wxString const &title)
    :BaseFrame(title)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    target_graph=new TestGL(this);
    
    sizer->Add(target_graph,wxSizerFlags(1).Expand());
    
    SetSizer(sizer);
    
    Show();
}

