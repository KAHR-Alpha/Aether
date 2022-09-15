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
#include <string_tools.h>

#include <gui_fieldblock_explr.h>

#include <iomanip>

wxDEFINE_EVENT(EVT_DECR_XY,wxCommandEvent);
wxDEFINE_EVENT(EVT_DECR_XZ,wxCommandEvent);
wxDEFINE_EVENT(EVT_DECR_YZ,wxCommandEvent);

wxDEFINE_EVENT(EVT_GL_PICK,wxCommandEvent);

wxDEFINE_EVENT(EVT_INCR_XY,wxCommandEvent);
wxDEFINE_EVENT(EVT_INCR_XZ,wxCommandEvent);
wxDEFINE_EVENT(EVT_INCR_YZ,wxCommandEvent);

NamedTextDisp::NamedTextDisp(wxWindow *parent,std::string const &name_,std::string const &data_)
    :wxPanel(parent)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    name=new wxStaticText(this,wxID_ANY,wxString(name_));
    data=new wxTextCtrl(this,wxID_ANY,wxString(data_));
    
    data->Disable();
    
    sizer->Add(name,wxSizerFlags().Center());
    sizer->Add(data,wxSizerFlags(1));
    
    SetSizer(sizer);
}

void NamedTextDisp::set_value(std::string const &value)
{
    data->SetValue(wxString(value));
}

void NamedTextDisp::set_value(wxString const &value)
{
    data->SetValue(value);
}

//###############
// PlaneMap_VAO
//###############

PlaneMap_VAO::PlaneMap_VAO(int N1_,int N2_)
    :N1(N1_), N2(N2_),
     v_arr(nullptr)
{
}

PlaneMap_VAO::~PlaneMap_VAO()
{
    delete[] v_arr;
}

void PlaneMap_VAO::draw()
{
    glBindVertexArray(vao);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
}

void PlaneMap_VAO::draw_data()
{
    glBindVertexArray(vao);
    
    glBindTexture(GL_TEXTURE_2D,texture_data);
    
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
}

void PlaneMap_VAO::init_opengl()
{
    v_arr=new GLfloat[16];
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1,&vertex_buff);
    glGenBuffers(1,&uv_buff);
    glGenBuffers(1,&index_buff);
    
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
    
    GLfloat *map=new GLfloat[3*N1*N2];
    
    int i,j;
    
    for(i=0;i<N1;i++) for(j=0;j<N2;j++)
    {
        map[3*(i+N1*j)+0]=i/(N1-1.0);
        map[3*(i+N1*j)+1]=j/(N2-1.0);
        map[3*(i+N1*j)+2]=0.0;
    }
    
//    for(i=0;i<4*N1*N2;i++) map[i]=1.0;
    
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
        
    glGenTextures(1,&texture_data);
    glBindTexture(GL_TEXTURE_2D,texture_data);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,N1,N2);
    
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RED,GL_FLOAT,map);
    
    delete[] uv_arr;
    delete[] index_arr;
    delete[] map;
}

void PlaneMap_VAO::reallocate(int N1_,int N2_)
{
    N1=N1_;
    N2=N2_;
    
    glDeleteTextures(1,&texture_rgb);
    glGenTextures(1,&texture_rgb);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
    
    glDeleteTextures(1,&texture_data);
    glGenTextures(1,&texture_data);
    glBindTexture(GL_TEXTURE_2D,texture_data);
    
    glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,N1,N2);
}

void PlaneMap_VAO::set_data(int N1_,int N2_,GLfloat *data)
{
    if(N1!=N1_ || N2!=N2_) reallocate(N1_,N2_);
        
    glBindTexture(GL_TEXTURE_2D,texture_data);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RED,GL_FLOAT,data);
}

void PlaneMap_VAO::set_map(int N1_,int N2_,GLfloat *map)
{
    if(N1!=N1_ || N2!=N2_) reallocate(N1_,N2_);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
}

void PlaneMap_VAO::set_vertices(Vector3 const &V1,
                                Vector3 const &V2,
                                Vector3 const &V3,
                                Vector3 const &V4)
{
    glBindVertexArray(vao);
    
    v_arr[0]= V1.x; v_arr[1]= V1.y; v_arr[2]= V1.z;
    v_arr[3]= 1.0;
    
    v_arr[4]= V2.x; v_arr[5]= V2.y; v_arr[6]= V2.z;
    v_arr[7]= 1.0;
    
    v_arr[8]= V3.x; v_arr[9]= V3.y; v_arr[10]=V3.z;
    v_arr[11]=1.0;
    
    v_arr[12]=V4.x; v_arr[13]=V4.y; v_arr[14]=V4.z;
    v_arr[15]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,vertex_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 16*sizeof(GLfloat),
                 (void*)v_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
}

//###############
//  GL_FBExplr
//###############

GL_FBExplr::GL_FBExplr(wxWindow *parent)
    :GL_FD_Base(parent),
     dxy(true), dxz(true), dyz(true),
     pxy(0), pxz(0), pyz(0),
     mxy(60,60), mxz(60,60), myz(60,60)
{
    SetWindowStyle(wxWANTS_CHARS);
    
    Bind(wxEVT_LEFT_DCLICK,&GL_FBExplr::get_value,this);
    Bind(wxEVT_KEY_DOWN,&GL_FBExplr::keyboard,this);
}

void GL_FBExplr::init_opengl()
{
    mxy.init_opengl();
    mxz.init_opengl();
    myz.init_opengl();
    
    GL_FD_Base::init_opengl();
    
    prog_planes=Glite::create_program(PathManager::locate_resource("resources/glsl/planemap_vshader.glsl"),
                                      PathManager::locate_resource("resources/glsl/planemap_fshader.glsl"));
    
    prog_data=Glite::create_program(PathManager::locate_resource("resources/glsl/planemap_data_vs.glsl"),
                                    PathManager::locate_resource("resources/glsl/planemap_data_fs.glsl"));
    
    uni_proj_planes=glGetUniformLocation(prog_planes,"proj_mat");
    uni_proj_data=glGetUniformLocation(prog_data,"proj_mat");
    
    mxy.set_vertices(Vector3(-0.5,-0.5,0),
                     Vector3(+0.5,-0.5,0),
                     Vector3(+0.5,+0.5,0),
                     Vector3(-0.5,+0.5,0));
    
    mxz.set_vertices(Vector3(-0.5,0,-0.5),
                     Vector3(+0.5,0,-0.5),
                     Vector3(+0.5,0,+0.5),
                     Vector3(-0.5,0,+0.5));
    
    myz.set_vertices(Vector3(0,-0.5,-0.5),
                     Vector3(0,+0.5,-0.5),
                     Vector3(0,+0.5,+0.5),
                     Vector3(0,-0.5,+0.5));
    
    glGenSamplers(1,&sampler);
    glBindSampler(0,sampler);
    
    glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

void GL_FBExplr::get_value(wxMouseEvent &event)
{
    SetCurrent(*glcnt);
    
//    std::cout<<"DClick"<<std::endl;
    GLuint fbo,texture_depth,texture_val,texture_xyz;
    
    int sx,sy;
    GetSize(&sx,&sy);
    
    glGenTextures(1,&texture_depth);
    glBindTexture(GL_TEXTURE_2D,texture_depth);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_DEPTH_COMPONENT32F,sx,sy);
    
    glGenTextures(1,&texture_val);
    glBindTexture(GL_TEXTURE_2D,texture_val);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,sx,sy);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glGenTextures(1,&texture_xyz);
    glBindTexture(GL_TEXTURE_2D,texture_xyz);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,sx,sy);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    
    glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,texture_xyz,0);
    glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,texture_val,0);
    glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,texture_depth,0);
    
//    chk_var(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    
    const GLenum attachments[2]={GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
    
    glDrawBuffers(2,attachments);
    
    //
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    camera.look_at(cam_t);
    
    glUseProgram(prog_data);
    glUniformMatrix4fv(uni_proj_data,1,0,camera.proj_gl);
    
    if(dxy) mxy.draw_data();
    if(dxz) mxz.draw_data();
    if(dyz) myz.draw_data();
    
    //
    
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    
    GLfloat pix[3];
    GLfloat val;
    
//    chk_var(pix[0]);
//    chk_var(pix[1]);
//    chk_var(pix[2]);
//    chk_var(event.GetX());
//    chk_var(event.GetY());
    
//    glReadPixels(sx/2,sy/2,1,1,GL_RGB,GL_FLOAT,pix);
    glReadPixels(event.GetX(),sy-event.GetY(),1,1,GL_RGB,GL_FLOAT,pix);
    
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(event.GetX(),sy-event.GetY(),1,1,GL_RED,GL_FLOAT,&val);
    
//    chk_var(pix[0]);
//    chk_var(pix[1]);
//    chk_var(pix[2]);
//    chk_var(val);
    
    glDeleteFramebuffers(1,&fbo);
    glDeleteTextures(1,&texture_depth);
    glDeleteTextures(1,&texture_val);
    glDeleteTextures(1,&texture_xyz);
    
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    
    val_pick=val;
    
    wxCommandEvent data_carrier(EVT_GL_PICK);
    data_carrier.SetClientData((void*)&val_pick);
    
    wxPostEvent(this,data_carrier);
}

void GL_FBExplr::keyboard(wxKeyEvent &event)
{
    int key=event.GetKeyCode();
    
    if(key==WXK_UP)
    {
        wxCommandEvent event_out(EVT_INCR_XY);
        wxPostEvent(this,event_out);
    }
    else if(key==WXK_DOWN)
    {
        wxCommandEvent event_out(EVT_DECR_XY);
        wxPostEvent(this,event_out);
    }
    else if(key==WXK_LEFT)
    {
        if(event.ShiftDown())
        {
            wxCommandEvent event_out(EVT_DECR_XZ);
            wxPostEvent(this,event_out);
        }
        else
        {
            wxCommandEvent event_out(EVT_DECR_YZ);
            wxPostEvent(this,event_out);
        }
    }
    else if(key==WXK_RIGHT)
    {
        if(event.ShiftDown())
        {
            wxCommandEvent event_out(EVT_INCR_XZ);
            wxPostEvent(this,event_out);
        }
        else
        {
            wxCommandEvent event_out(EVT_INCR_YZ);
            wxPostEvent(this,event_out);
        }
    }
}

void GL_FBExplr::render()
{
    if(!gl_ok) return;
    
    GL_FD_Base::render();
    
    glUseProgram(prog_planes);
    glUniformMatrix4fv(uni_proj_planes,1,0,camera.proj_gl);
    
    if(dxy) mxy.draw();
    if(dxz) mxz.draw();
    if(dyz) myz.draw();
}

void GL_FBExplr::set_data_xy(int Nx_,int Ny_,GLfloat *data) { mxy.set_data(Nx_,Ny_,data); }
void GL_FBExplr::set_data_xz(int Nx_,int Nz_,GLfloat *data) { mxz.set_data(Nx_,Nz_,data); }
void GL_FBExplr::set_data_yz(int Ny_,int Nz_,GLfloat *data) { myz.set_data(Ny_,Nz_,data); }

void GL_FBExplr::set_map_xy(int Nx_,int Ny_,GLfloat *map) { mxy.set_map(Nx_,Ny_,map); }
void GL_FBExplr::set_map_xz(int Nx_,int Nz_,GLfloat *map) { mxz.set_map(Nx_,Nz_,map); }
void GL_FBExplr::set_map_yz(int Ny_,int Nz_,GLfloat *map) { myz.set_map(Ny_,Nz_,map); }

void GL_FBExplr::set_pxy(int z)
{
    pxy=z;
    update_mxy();
}

void GL_FBExplr::set_pxz(int y)
{
    pxz=y;
    update_mxz();
}

void GL_FBExplr::set_pyz(int x)
{
    pyz=x;
    update_myz();
}

void GL_FBExplr::switch_pxy(bool val) { dxy=val; }
void GL_FBExplr::switch_pxz(bool val) { dxz=val; }
void GL_FBExplr::switch_pyz(bool val) { dyz=val; }

void GL_FBExplr::update_grid(int Nx_,int Ny_,int Nz_,
                           double Dx_,double Dy_,double Dz_)
{
    GL_FD_Base::update_grid(Nx_,Ny_,Nz_,Dx_,Dy_,Dz_);
    
    update_mxy();
    update_mxz();
    update_myz();
}

void GL_FBExplr::update_mxy()
{
    SetCurrent(*glcnt);
    
//    double z=lz*(pxy/(Nz-1.0)-0.5)+0.5*lz/Nz;
//    double z=(lz-lz/Nz)*(pxy/(Nz-1.0)-0.5);
    double z=-0.5*lz+(lz-lz/Nz)*(pxy/(Nz-1.0));
    
    mxy.set_vertices(Vector3(-0.5*lx,-0.5*ly,z),
                     Vector3(+0.5*lx,-0.5*ly,z),
                     Vector3(+0.5*lx,+0.5*ly,z),
                     Vector3(-0.5*lx,+0.5*ly,z));
}

void GL_FBExplr::update_mxz()
{
    SetCurrent(*glcnt);
    
//    double y=ly*(pxz/(Ny-1.0)-0.5)+0.5*ly/Ny;
//    double y=(ly-ly/Ny)*(pxz/(Ny-1.0)-0.5);
    double y=-0.5*ly+(ly-ly/Ny)*(pxz/(Ny-1.0));
    
    mxz.set_vertices(Vector3(-0.5*lx,y,-0.5*lz),
                     Vector3(+0.5*lx,y,-0.5*lz),
                     Vector3(+0.5*lx,y,+0.5*lz),
                     Vector3(-0.5*lx,y,+0.5*lz));
}

void GL_FBExplr::update_myz()
{
    SetCurrent(*glcnt);
    
//    double x=lx*(pyz/(Nx-1.0)-0.5)+0.5*lx/Nx;
//    double x=(lx-lx/Nx)*(pyz/(Nx-1.0)-0.5);
    double x=-0.5*lx+(lx-lx/Nx)*(pyz/(Nx-1.0));
    
    myz.set_vertices(Vector3(x,-0.5*ly,-0.5*lz),
                     Vector3(x,+0.5*ly,-0.5*lz),
                     Vector3(x,+0.5*ly,+0.5*lz),
                     Vector3(x,-0.5*ly,+0.5*lz));
}

//####################
//  FBESave_Dialog
//####################

FBESave_Dialog::FBESave_Dialog(wxWindow *parent,int *plane_,std::string *name_,bool *confirm_)
    :wxDialog(parent,wxID_ANY,"Select the plane to save"),
     confirm(confirm_),
     plane(plane_),
     name(name_)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxString choice_str[]={"XY","XZ","YZ"};
    
    choice=new wxRadioBox(this,wxID_ANY,"Plane:",wxDefaultPosition,wxDefaultSize,3,choice_str,1);
    
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    cancel_btn->Bind(wxEVT_BUTTON,&FBESave_Dialog::cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&FBESave_Dialog::ok,this);
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(choice,wxSizerFlags().Border(wxALL,1));
    top_sizer->Add(btn_sizer,wxSizerFlags().Border(wxALL,1));
    
    SetSizerAndFit(top_sizer);
    
    ShowModal();
}

void FBESave_Dialog::cancel(wxCommandEvent &event)
{
    Close();
}

void FBESave_Dialog::ok(wxCommandEvent &event)
{
    std::string fname("");
    
    fname=wxFileSelector("Select the file to save to:",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         wxEmptyString,
                         "MatLab script (*.m)|*.m",
                         wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                         
    if(fname.size()==0) return;
    
    *name=fname;
    *plane=choice->GetSelection();
    *confirm=true;
    Close();
}

//#########################
//   FieldBlockExplorer
//#########################

enum
{
    MENU_EXIT=0,
    MENU_EXTRACT_MAP,
    MENU_LOAD,
    MENU_SAVE_AS,
};

FieldBlockExplorer::FieldBlockExplorer(wxString const &title)
    :BaseFrame(title),
     dxy(true), dxz(true), dyz(true),
     baseline(1.0),
     clamp_max(1.0),
     E_max(1.0), Ex_max(1.0), Ey_max(1.0), Ez_max(1.0),
     data_xy(nullptr), data_xz(nullptr), data_yz(nullptr),
     map_xy(nullptr), map_xz(nullptr), map_yz(nullptr),
     field_display_type(DISP_E)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxPanel *ctrl_panel=new wxPanel(splitter);
    wxPanel *graph_panel=new wxPanel(splitter);
    
    wxBoxSizer *graph_sizer=new wxBoxSizer(wxVERTICAL);
    
    gl_disp=new GL_FBExplr(graph_panel);
    gl_data=new wxTextCtrl(graph_panel,wxID_ANY,"");
    gl_data->Disable();
    
    graph_sizer->Add(gl_disp,wxSizerFlags(1).Expand());
    graph_sizer->Add(gl_data,wxSizerFlags().Expand());
    
    graph_panel->SetSizer(graph_sizer);
    
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    // FieldBlock data
    
    wxStaticBoxSizer *data_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,wxString("FieldBlock data"));
    
    lambda_disp=new NamedTextDisp(ctrl_panel,"Lambda : ","");
    Nx_disp=new NamedTextDisp(ctrl_panel,"Nx : ","");
    Ny_disp=new NamedTextDisp(ctrl_panel,"Ny : ","");
    Nz_disp=new NamedTextDisp(ctrl_panel,"Nz : ","");
    Dx_disp=new NamedTextDisp(ctrl_panel,"Dx : ","");
    Dy_disp=new NamedTextDisp(ctrl_panel,"Dy : ","");
    Dz_disp=new NamedTextDisp(ctrl_panel,"Dz : ","");
    
    data_sizer->Add(lambda_disp,wxSizerFlags().Expand());
    data_sizer->Add(Nx_disp,wxSizerFlags().Expand());
    data_sizer->Add(Ny_disp,wxSizerFlags().Expand());
    data_sizer->Add(Nz_disp,wxSizerFlags().Expand());
    data_sizer->Add(Dx_disp,wxSizerFlags().Expand());
    data_sizer->Add(Dy_disp,wxSizerFlags().Expand());
    data_sizer->Add(Dz_disp,wxSizerFlags().Expand());
    
    top_sizer->Add(data_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Display
    
    wxStaticBoxSizer *disp_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,wxString("Display"));
    wxBoxSizer *pxy_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pxz_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pyz_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    pxy_ctrl=new wxSpinCtrl(ctrl_panel,wxID_ANY,"29");
    pxz_ctrl=new wxSpinCtrl(ctrl_panel,wxID_ANY,"29");
    pyz_ctrl=new wxSpinCtrl(ctrl_panel,wxID_ANY,"29");
    
    pxy_ctrl->Bind(wxEVT_SPINCTRL,&FieldBlockExplorer::update_pxy,this);
    pxz_ctrl->Bind(wxEVT_SPINCTRL,&FieldBlockExplorer::update_pxz,this);
    pyz_ctrl->Bind(wxEVT_SPINCTRL,&FieldBlockExplorer::update_pyz,this);
    
    wxStaticText *n_xy=new wxStaticText(ctrl_panel,wxID_ANY,wxString("XY : "));
    wxStaticText *n_xz=new wxStaticText(ctrl_panel,wxID_ANY,wxString("XZ : "));
    wxStaticText *n_yz=new wxStaticText(ctrl_panel,wxID_ANY,wxString("YZ : "));
    
    pxy_sizer->Add(n_xy,wxSizerFlags().Center());
    pxy_sizer->Add(pxy_ctrl,wxSizerFlags(1));
    pxz_sizer->Add(n_xz,wxSizerFlags().Center());
    pxz_sizer->Add(pxz_ctrl,wxSizerFlags(1));
    pyz_sizer->Add(n_yz,wxSizerFlags().Center());
    pyz_sizer->Add(pyz_ctrl,wxSizerFlags(1));
    
    disp_sizer->Add(pxy_sizer,wxSizerFlags().Expand());
    disp_sizer->Add(pxz_sizer,wxSizerFlags().Expand());
    disp_sizer->Add(pyz_sizer,wxSizerFlags().Expand());
    
    // Planes selection
    
    wxBoxSizer *dxyz_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    dxy_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"XY");
    dxz_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"XZ");
    dyz_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"YZ");
    
    dxy_ctrl->SetValue(true);
    dxz_ctrl->SetValue(true);
    dyz_ctrl->SetValue(true);
    
    dxy_ctrl->Bind(wxEVT_CHECKBOX,&FieldBlockExplorer::switch_pxy,this);
    dxz_ctrl->Bind(wxEVT_CHECKBOX,&FieldBlockExplorer::switch_pxz,this);
    dyz_ctrl->Bind(wxEVT_CHECKBOX,&FieldBlockExplorer::switch_pyz,this);
    
    dxyz_sizer->Add(dxy_ctrl,wxSizerFlags(1));
    dxyz_sizer->Add(dxz_ctrl,wxSizerFlags(1));
    dxyz_sizer->Add(dyz_ctrl,wxSizerFlags(1));
    
    disp_sizer->Add(dxyz_sizer,wxSizerFlags().Expand());
    
    top_sizer->Add(disp_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Baseline
    
    baseline_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Baseline: ",1.0);
    baseline_ctrl->Bind(EVT_NAMEDTXTCTRL,&FieldBlockExplorer::evt_baseline,this);
    
    disp_sizer->Add(baseline_ctrl,wxSizerFlags().Expand());
    
    // Clamp
    
    clamp_ctrl=new NamedTextCtrl<double>(ctrl_panel,"clamp: ",1.0);
    clamp_ctrl->Bind(EVT_NAMEDTXTCTRL,&FieldBlockExplorer::evt_clamp,this);
    
    disp_sizer->Add(clamp_ctrl,wxSizerFlags().Expand());
    
    // Field Display
    
    field_display=new wxChoice(ctrl_panel,wxID_ANY);
    field_display->Append("E");
    field_display->Append("Ex");
    field_display->Append("Ey");
    field_display->Append("Ez");
    
    field_display->SetSelection(0);
    field_display->Bind(wxEVT_CHOICE,&FieldBlockExplorer::evt_field_display,this);
    
    disp_sizer->Add(field_display,wxSizerFlags().Expand());
    
    // Padding
    
    wxPanel *padding=new wxPanel(ctrl_panel);
    top_sizer->Add(padding,wxSizerFlags(1).Expand());
    
    // Menu
    
    wxMenuBar *menu_bar=new wxMenuBar{};
    wxMenu *file_menu=new wxMenu{};
    
    file_menu->Append(MENU_LOAD,"Load FieldBlock");
    file_menu->Append(MENU_EXTRACT_MAP,"Extract Map");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    file_menu->Bind(wxEVT_MENU,&FieldBlockExplorer::evt_menu,this);
    
    menu_bar->Append(file_menu,"File");
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
        
    ctrl_panel->SetSizer(top_sizer);
    
    splitter->SplitVertically(ctrl_panel,graph_panel,200);
    
    Bind(EVT_GL_PICK,&FieldBlockExplorer::gl_pick,this);
    Bind(EVT_DECR_XY,&FieldBlockExplorer::decrement_pxy,this);
    Bind(EVT_DECR_XZ,&FieldBlockExplorer::decrement_pxz,this);
    Bind(EVT_DECR_YZ,&FieldBlockExplorer::decrement_pyz,this);
    Bind(EVT_INCR_XY,&FieldBlockExplorer::increment_pxy,this);
    Bind(EVT_INCR_XZ,&FieldBlockExplorer::increment_pxz,this);
    Bind(EVT_INCR_YZ,&FieldBlockExplorer::increment_pyz,this);
}

FieldBlockExplorer::~FieldBlockExplorer()
{
    if(data_xy!=nullptr) delete[] data_xy;
    if(data_xz!=nullptr) delete[] data_xz;
    if(data_yz!=nullptr) delete[] data_yz;
    
    if(map_xy!=nullptr) delete[] map_xy;
    if(map_xz!=nullptr) delete[] map_xz;
    if(map_yz!=nullptr) delete[] map_yz;
}

void FieldBlockExplorer::decrement_pxy(wxCommandEvent &event)
{
    pxy_ctrl->SetValue(std::max(0,pxy_ctrl->GetValue()-1));
    if(map_xy!=nullptr) update_map_xy();
}

void FieldBlockExplorer::decrement_pxz(wxCommandEvent &event)
{
    pxz_ctrl->SetValue(std::max(0,pxz_ctrl->GetValue()-1));
    if(map_xz!=nullptr) update_map_xz();
}

void FieldBlockExplorer::decrement_pyz(wxCommandEvent &event)
{
    pyz_ctrl->SetValue(std::max(0,pyz_ctrl->GetValue()-1));
    if(map_yz!=nullptr) update_map_yz();
}

void FieldBlockExplorer::evt_baseline(wxCommandEvent &event)
{
    baseline=baseline_ctrl->get_value();
    
    fbh.set_baseline(baseline);
    
    E_max=fbh.get_E_abs_max();
    Ex_max=fbh.get_Ex_abs_max();
    Ey_max=fbh.get_Ey_abs_max();
    Ez_max=fbh.get_Ez_abs_max();
    
    update_map_xy();
    update_map_xz();
    update_map_yz();
}

void FieldBlockExplorer::evt_clamp(wxCommandEvent &event)
{
    clamp_max=clamp_ctrl->get_value();
    
    update_map_xy();
    update_map_xz();
    update_map_yz();
}

void FieldBlockExplorer::evt_field_display(wxCommandEvent &event)
{
    int i=field_display->GetSelection();
    
         if(i==0) field_display_type=DISP_E;
    else if(i==1) field_display_type=DISP_EX_ABS;
    else if(i==2) field_display_type=DISP_EY_ABS;
    else if(i==3) field_display_type=DISP_EZ_ABS;
    
    update_map_xy();
    update_map_xz();
    update_map_yz();
    
    event.Skip();
}

void FieldBlockExplorer::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
         if(ID==MENU_EXIT) Close();
    else if(ID==MENU_EXTRACT_MAP) extract_map();
    else if(ID==MENU_LOAD) evt_load_project("afblock","Aether FieldBlock");
    else if(ID==MENU_SAVE_AS) evt_save_project_as("afblock","Aether FieldBlock") ;
}

void FieldBlockExplorer::extract_map()
{
    std::cout<<"Save"<<std::endl;
    
    int plane=0;
    std::string fname;
    bool confirm=false;
    
    FBESave_Dialog(this,&plane,&fname,&confirm);
    
    if(confirm)
    {
             if(plane==0) fbh.save_matlab(NORMAL_Z,pxy_ctrl->GetValue(),E_FIELD,fname);
        else if(plane==1) fbh.save_matlab(NORMAL_Y,pxz_ctrl->GetValue(),E_FIELD,fname);
        else if(plane==2) fbh.save_matlab(NORMAL_X,pyz_ctrl->GetValue(),E_FIELD,fname);
    }
}

void FieldBlockExplorer::increment_pxy(wxCommandEvent &event)
{
    pxy_ctrl->SetValue(std::min(Nz-1,pxy_ctrl->GetValue()+1));
    if(map_xy!=nullptr) update_map_xy();
}

void FieldBlockExplorer::increment_pxz(wxCommandEvent &event)
{
    pxz_ctrl->SetValue(std::min(Ny-1,pxz_ctrl->GetValue()+1));
    if(map_xz!=nullptr) update_map_xz();
}

void FieldBlockExplorer::increment_pyz(wxCommandEvent &event)
{
    pyz_ctrl->SetValue(std::min(Nx-1,pyz_ctrl->GetValue()+1));
    if(map_yz!=nullptr) update_map_yz();
}

void FieldBlockExplorer::gl_pick(wxCommandEvent &event)
{
    double *val=reinterpret_cast<double*>(event.GetClientData());
    std::stringstream strm;
    strm<<std::setprecision(6)<<*val;
    
    gl_data->SetValue(strm.str());
}

void FieldBlockExplorer::load_project(wxFileName const &fname)
{
    std::string data_tmp=fname.GetFullPath().ToStdString();
    
    if(data_tmp.size()==0) return;
    
    fbh.load(data_tmp);
    
    std::cout<<"Load "<<data_tmp<<std::endl;
    
    wxString new_title("FieldBlock Explorer : ");
    new_title.Append(data_tmp);
    
    SetTitle(new_title);
    
    Nx=fbh.Nx;
    Ny=fbh.Ny;
    Nz=fbh.Nz;
    
    Dx=fbh.Dx;
    Dy=fbh.Dy;
    Dz=fbh.Dz;
    
    E_max=fbh.get_E_abs_max();
    Ex_max=fbh.get_Ex_abs_max();
    Ey_max=fbh.get_Ey_abs_max();
    Ez_max=fbh.get_Ez_abs_max();
    
    clamp_max=E_max;
    clamp_ctrl->set_value(clamp_max);
    
    if(data_xy!=nullptr) delete[] data_xy;
    if(data_xz!=nullptr) delete[] data_xz;
    if(data_yz!=nullptr) delete[] data_yz;
    
    if(map_xy!=nullptr) delete[] map_xy;
    if(map_xz!=nullptr) delete[] map_xz;
    if(map_yz!=nullptr) delete[] map_yz;
    
    data_xy=new GLfloat[3*Nx*Ny];
    data_xz=new GLfloat[3*Nx*Nz];
    data_yz=new GLfloat[3*Ny*Nz];
    
    map_xy=new GLfloat[3*Nx*Ny];
    map_xz=new GLfloat[3*Nx*Nz];
    map_yz=new GLfloat[3*Ny*Nz];
    
    chk_var(E_max);
    chk_var(Ex_max);
    chk_var(Ey_max);
    chk_var(Ez_max);
    
    lambda_disp->set_value(add_unit_u(fbh.lambda));
    
    Nx_disp->set_value(Nx);
    Ny_disp->set_value(Ny);
    Nz_disp->set_value(Nz);
    
    Dx_disp->set_value(add_unit_u(Dx));
    Dy_disp->set_value(add_unit_u(Dy));
    Dz_disp->set_value(add_unit_u(Dz));
    
    pxy_ctrl->SetRange(0,Nz-1);
    pxz_ctrl->SetRange(0,Ny-1);
    pyz_ctrl->SetRange(0,Nx-1);
    
    gl_disp->update_grid(Nx,Ny,Nz,Dx,Dy,Dz);
    
    pxy_ctrl->SetValue(Nz/2);
    pxz_ctrl->SetValue(Ny/2);
    pyz_ctrl->SetValue(Nx/2);
    
    update_map_xy();
    update_map_xz();
    update_map_yz();
}

void FieldBlockExplorer::save_project(wxFileName const &fname)
{
    std::string data_tmp=fname.GetFullPath().ToStdString();
    
    if(data_tmp.size()==0) return;
    
    fbh.save(data_tmp);
}

void FieldBlockExplorer::switch_pxy(wxCommandEvent &event) { gl_disp->switch_pxy(dxy_ctrl->GetValue()); }
void FieldBlockExplorer::switch_pxz(wxCommandEvent &event) { gl_disp->switch_pxz(dxz_ctrl->GetValue()); }
void FieldBlockExplorer::switch_pyz(wxCommandEvent &event) { gl_disp->switch_pyz(dyz_ctrl->GetValue()); }

void FieldBlockExplorer::update_map_xy()
{
    int i,j;
    int k=pxy_ctrl->GetValue();
    
    double E,r,g,b;
        
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
    {
        switch(field_display_type)
        {
            case DISP_EX_ABS:
                E=fbh.get_Ex_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EY_ABS:
                E=fbh.get_Ey_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EZ_ABS:
                E=fbh.get_Ez_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            default:
                E=fbh.get_E_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
        }
        
        data_xy[i+j*Nx]=E;
        
        map_xy[3*(i+j*Nx)+0]=r;
        map_xy[3*(i+j*Nx)+1]=g;
        map_xy[3*(i+j*Nx)+2]=b;
    }
    
    gl_disp->set_pxy(k);
    gl_disp->set_data_xy(Nx,Ny,data_xy);
    gl_disp->set_map_xy(Nx,Ny,map_xy);
}

void FieldBlockExplorer::update_map_xz()
{
    int i,k;
    int j=pxz_ctrl->GetValue();
    
    double E,r,g,b;
    
    for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
    {
        switch(field_display_type)
        {
            case DISP_EX_ABS:
                E=fbh.get_Ex_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EY_ABS:
                E=fbh.get_Ey_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EZ_ABS:
                E=fbh.get_Ez_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            default:
                E=fbh.get_E_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
        }
        
        data_xz[i+k*Nx]=E;
        
        map_xz[3*(i+k*Nx)+0]=r;
        map_xz[3*(i+k*Nx)+1]=g;
        map_xz[3*(i+k*Nx)+2]=b;
    }
    
    gl_disp->set_pxz(j);
    gl_disp->set_data_xz(Nx,Nz,data_xz);
    gl_disp->set_map_xz(Nx,Nz,map_xz);
}

void FieldBlockExplorer::update_map_yz()
{
    int j,k;
    int i=pyz_ctrl->GetValue();
    
    double E,r,g,b;
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        switch(field_display_type)
        {
            case DISP_EX_ABS:
                E=fbh.get_Ex_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EY_ABS:
                E=fbh.get_Ey_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            case DISP_EZ_ABS:
                E=fbh.get_Ez_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
                break;
            default:
                E=fbh.get_E_abs(i,j,k);
                degra(E,std::min(E_max,clamp_max),r,g,b);
        }
        
        data_yz[j+k*Ny]=E;
        
        map_yz[3*(j+k*Ny)+0]=r;
        map_yz[3*(j+k*Ny)+1]=g;
        map_yz[3*(j+k*Ny)+2]=b;
    }
    
    gl_disp->set_pyz(i);
    gl_disp->set_data_yz(Ny,Nz,data_yz);
    gl_disp->set_map_yz(Ny,Nz,map_yz);
}

void FieldBlockExplorer::update_pxy(wxSpinEvent &event) { if(map_xy!=nullptr) update_map_xy(); }
void FieldBlockExplorer::update_pxz(wxSpinEvent &event) { if(map_xz!=nullptr) update_map_xz(); }
void FieldBlockExplorer::update_pyz(wxSpinEvent &event) { if(map_yz!=nullptr) update_map_yz(); }
