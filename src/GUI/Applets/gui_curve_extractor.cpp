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

#include <gui_curve_extractor.h>

#include <wx/splitter.h>

//######################
//   GL_Curve_Extract
//######################

GL_Curve_Extract::GL_Curve_Extract(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     busy(false),
     N1(512), N2(512),
     zoom_factor(1.0),
     x_factor(1.0), y_factor(1.0),
     camera_x(0), camera_y(0),
     mouse_x(0), mouse_xp(0),
     mouse_y(0), mouse_yp(0)
{
    glcnt=new wxGLContext(this);
    SetCurrent(*glcnt);
    Show(true);
    
    timer=new wxTimer(this);
    
    //
    
    if(gl3wInit())
        wxMessageBox("Error: Could not initialize OpenGL");
    if(!gl3wIsSupported(4,3))
        wxMessageBox("Error: OpenGL 4.3 not supported.\n Update your graphics drivers.");;
    
    //
    
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glDisable(GL_CULL_FACE);
    
    //
    
    // Vao
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1,&vertex_buff);
    glGenBuffers(1,&uv_buff);
    glGenBuffers(1,&index_buff);
    
    GLfloat *v_arr=new GLfloat[16];
    
    v_arr[0]=-1.0; v_arr[1]=-1.0; v_arr[2]=0;
    v_arr[3]= 1.0;
    
    v_arr[4]=+1.0; v_arr[5]=-1.0; v_arr[6]=0;
    v_arr[7]= 1.0;
    
    v_arr[8]=+1.0; v_arr[9]=+1.0; v_arr[10]=0;
    v_arr[11]=1.0;
    
    v_arr[12]=-1.0; v_arr[13]=+1.0; v_arr[14]=0;
    v_arr[15]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,vertex_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 16*sizeof(GLfloat),
                 (void*)v_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    GLfloat *uv_arr=new GLfloat[8];
    
    uv_arr[0]=0.0;
    uv_arr[1]=0.0;
    uv_arr[2]=1.0;
    uv_arr[3]=0.0;
    uv_arr[4]=1.0;
    uv_arr[5]=1.0;
    uv_arr[6]=0.0;
    uv_arr[7]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,uv_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 8*sizeof(GLfloat),
                 (void*)uv_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
    
    GLuint *index_arr=new GLuint[6];
    
    index_arr[0]=0;
    index_arr[1]=1;
    index_arr[2]=2;
    index_arr[3]=0;
    index_arr[4]=2;
    index_arr[5]=3;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 6*sizeof(GLuint),
                 (void*)index_arr,
                 GL_STATIC_DRAW);
    
    glGenTextures(1,&texture_rgb);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
    
    map=new GLfloat[3*N1*N2];
    
    int i,j;
    
    for(i=0;i<N1;i++) for(j=0;j<N2;j++)
    {
        map[3*(i+N1*j)+0]=0.0;
        map[3*(i+N1*j)+1]=0.0;
        map[3*(i+N1*j)+2]=0.0;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
    
    delete[] v_arr;
    delete[] uv_arr;
    delete[] index_arr;
    
    // Programs 
    
    program_image=Glite::create_program(PathManager::locate_resource("resources/glsl/planemap_vshader.glsl"),
                                        PathManager::locate_resource("resources/glsl/planemap_fshader.glsl"));
    
    uniform_proj_mat=glGetUniformLocation(program_image,"proj_mat");
    
    glGenSamplers(1,&sampler);
    glBindSampler(0,sampler);
    
    glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    //
    
    Bind(wxEVT_KEY_DOWN,&GL_Curve_Extract::evt_keyboard,this);
    Bind(wxEVT_LEFT_DOWN,&GL_Curve_Extract::evt_mouse_left,this);
    Bind(wxEVT_MOTION,&GL_Curve_Extract::evt_mouse_motion,this);
    Bind(wxEVT_MOUSEWHEEL,&GL_Curve_Extract::evt_mouse_wheel,this);
    Bind(wxEVT_SIZE,&GL_Curve_Extract::evt_resize,this);
    Bind(wxEVT_TIMER,&GL_Curve_Extract::evt_timer,this);
    timer->Start(1000/60);
}

GL_Curve_Extract::~GL_Curve_Extract()
{
    timer->Stop();
    delete glcnt;
    delete map;
}

void GL_Curve_Extract::add_curve()
{
    int N=anchors_x.size();
    
    anchors_x.resize(N+1);
    anchors_y.resize(N+1);
    anchors_lock.resize(N+1);
}

void GL_Curve_Extract::deep_render()
{
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    x_factor=1.0,y_factor=1.0;
    
    if(static_cast<double>(Sy)/Sx<=static_cast<double>(N2)/N1) 
        x_factor=static_cast<float>(N1)/Sx/(static_cast<double>(N2)/Sy);
    else
        y_factor=static_cast<double>(N2)/Sy/(static_cast<double>(N1)/Sx);
    
//    double r=static_cast<double>(N1)/N2;
//    double R=static_cast<double>(Sx)/Sy;
//    
//    if(r>=R) y_factor=(r*Sy)/Sx;
//    else x_factor=Sx/(r*Sy);
    
    glUseProgram(program_image);
    
    GLfloat proj_gl[16];
    
    proj_gl[0 ]=x_factor*zoom_factor;
    proj_gl[1 ]=0;
    proj_gl[2 ]=0;
    proj_gl[3 ]=0;
    
    proj_gl[4 ]=0;
    proj_gl[5 ]=y_factor*zoom_factor;
    proj_gl[6 ]=0;
    proj_gl[7 ]=0;
    
    proj_gl[8 ]=0;
    proj_gl[9 ]=0;
    proj_gl[10]=1;
    proj_gl[11]=0;
    
    proj_gl[12]=camera_x/x_factor;
    proj_gl[13]=camera_y/y_factor;
    proj_gl[14]=0;
    proj_gl[15]=1;
    
    glUniformMatrix4fv(uniform_proj_mat,1,0,proj_gl);
    
    glBindVertexArray(vao);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
}

void GL_Curve_Extract::delete_curve(int i)
{
    std::vector<bool>::const_iterator it1=anchors_lock.begin()+i;
    anchors_lock.erase(it1);
    
    std::vector<std::vector<double>>::const_iterator it2=anchors_x.begin()+i;
    anchors_x.erase(it2);
    
    it2=anchors_y.begin()+i;
    anchors_y.erase(it2);
}

void GL_Curve_Extract::evt_keyboard(wxKeyEvent &event)
{
    event.Skip();
}

void GL_Curve_Extract::evt_mouse_left(wxMouseEvent &event)
{
    event.Skip();
}

void GL_Curve_Extract::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(event.LeftIsDown() && event.Dragging())
    {
        double dx=(mouse_x-mouse_xp)/(GetSize().x+0.0)*x_factor;
        double dy=(mouse_y-mouse_yp)/(GetSize().y+0.0)*y_factor;
        
        camera_x+=dx*2.0;
        camera_y-=dy*2.0;
    }
    
    event.Skip();
}

void GL_Curve_Extract::evt_mouse_wheel(wxMouseEvent &event)
{
    if(event.GetWheelRotation()>0) zoom_factor*=0.95;
    else zoom_factor/=0.95;
    
    event.Skip();
}

void GL_Curve_Extract::evt_resize(wxSizeEvent &event)
{
    SetCurrent(*glcnt);
    
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    glViewport(0, 0, (GLint)Sx, (GLint)Sy);
}

void GL_Curve_Extract::evt_timer(wxTimerEvent &event)
{
    render();
}

void GL_Curve_Extract::render()
{
    if(IsShown())
    {
        SetCurrent(*glcnt);
        if(!busy)
        {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            deep_render();
            
            SwapBuffers();
        }
    }
}

void GL_Curve_Extract::set_image_data(int height_,int width_,unsigned char *data_)
{
    zoom_factor=1.0;
    
    SetCurrent(*glcnt);
    
    if(N1!=width_ || N2!=height_)
    {
        N1=width_;
        N2=height_;
        
        delete map;
        map=new GLfloat[3*N1*N2];
        
        glDeleteTextures(1,&texture_rgb);
        glGenTextures(1,&texture_rgb);
        glBindTexture(GL_TEXTURE_2D,texture_rgb);
        
        glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
    }
    
    int i,j;
    
    for(i=0;i<N1;i++) for(j=0;j<N2;j++)
    {
        map[3*(i+N1*j)+0]=data_[3*(i+N1*(N2-1-j))+0]/255.0;
        map[3*(i+N1*j)+1]=data_[3*(i+N1*(N2-1-j))+1]/255.0;
        map[3*(i+N1*j)+2]=data_[3*(i+N1*(N2-1-j))+2]/255.0;
    }
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
    
//    glBindVertexArray(vao);
//    
//    GLfloat v_arr[16];
//    
//    v_arr[0]=0; v_arr[1]=0; v_arr[2]=0;
//    v_arr[3]= 1.0;
//    
//    v_arr[4]=0; v_arr[5]=0; v_arr[6]=0;
//    v_arr[7]= 1.0;
//    
//    v_arr[8]=+1.0; v_arr[9]=+1.0; v_arr[10]=0;
//    v_arr[11]=1.0;
//    
//    v_arr[12]=-1.0; v_arr[13]=+1.0; v_arr[14]=0;
//    v_arr[15]=1.0;
//    
//    glBindBuffer(GL_ARRAY_BUFFER,vertex_buff);
//    glBufferData(GL_ARRAY_BUFFER,
//                 16*sizeof(GLfloat),
//                 (void*)v_arr,
//                 GL_STATIC_DRAW);
//    
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
}

//#########################
//   Curve_Extract_Frame
//#########################

Curve_Extract_Frame::Curve_Extract_Frame(wxString const &title)
    :BaseFrame(title),
     Nx(512), Ny(512)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    gl=new GL_Curve_Extract(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer *img_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Input image");
    
    wxBoxSizer *load_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxButton *img_btn=new wxButton(ctrl_panel,wxID_ANY,"Load",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    img_fname_ctrl=new NamedTextCtrl<std::string>(ctrl_panel,"","");
    img_fname_ctrl->lock();
    
    img_btn->Bind(wxEVT_BUTTON,&Curve_Extract_Frame::evt_load_btn,this);
    
    load_sizer->Add(img_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    load_sizer->Add(img_fname_ctrl,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
    
    img_sizer->Add(load_sizer,wxSizerFlags().Expand());
    
    width_ctrl=new NamedTextCtrl<int>(ctrl_panel,"Width: ",0);
    height_ctrl=new NamedTextCtrl<int>(ctrl_panel,"Heigth: ",0);
    width_ctrl->lock();
    height_ctrl->lock();
    
    img_sizer->Add(width_ctrl,wxSizerFlags().Expand());
    img_sizer->Add(height_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(img_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    wxStaticBoxSizer *x_axis_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"X axis");
    
    ctrl_sizer->Add(x_axis_sizer,wxSizerFlags().Expand());
    
    wxStaticBoxSizer *y_axis_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Y axis");
    
    ctrl_sizer->Add(y_axis_sizer,wxSizerFlags().Expand());
    
    // Wrapping up
    
    splitter->SplitVertically(ctrl_panel,gl,300);
    splitter->SetMinimumPaneSize(250);
    
    ctrl_panel->FitInside();
}

Curve_Extract_Frame::~Curve_Extract_Frame()
{
}

void Curve_Extract_Frame::evt_load_btn(wxCommandEvent &event)
{
    wxString fname=wxFileSelector("Select an image",
                                  wxEmptyString,wxEmptyString,wxEmptyString,
                                  "Image files (*.bmp,*.gif,*.jpg,*.jpeg,*.png)|*.bmp;*.gif;*.jpg;*.jpeg;*.png",
                                  wxFD_OPEN);
    
    if(!fname.empty())
    {
        base_image.LoadFile(fname);
        gl->set_image_data(base_image.GetHeight(),
                           base_image.GetWidth(),
                           base_image.GetData());
        
        wxFileName name(fname);
        
        img_fname_ctrl->set_value(name.GetFullName().ToStdString());
        
        Nx=base_image.GetWidth();
        Ny=base_image.GetHeight();
        
        width_ctrl->set_value(Nx);
        height_ctrl->set_value(Ny);
    }
    
    event.Skip();
}
