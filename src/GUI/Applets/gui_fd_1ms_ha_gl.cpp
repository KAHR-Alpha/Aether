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

#include <gui_fd_ms.h>
#include <gui_rsc.h>

#include <wx/splitter.h>

extern std::ofstream plog;

/*#ifndef WX30_RESTRICT
HAMS_GL::HAMS_GL(wxWindow *parent,wxGLAttributes const &attrib)
    :wxGLCanvas(parent,attrib,wxID_ANY),
#else*/
HAMS_GL::HAMS_GL(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
//#endif
     busy(false), target_mode(false), zoom_mode(false),
     pad_xm(2), pad_xp(2),
     pad_ym(2), pad_yp(2),
     xmin(0), xmax(1),
     ymin(-1), ymax(1),
     c_xm(0), c_xp(1), c_ym(-1), c_yp(1),
     n_xm(0), n_xp(1), n_ym(-1), n_yp(1),
     mouse_x(0), mouse_xp(0),
     mouse_y(0), mouse_yp(0),
     n_sub(1.0f), n_sup(1.0f),
     N1(256), N2(256), exposure(1.0)
{
    glcnt=new wxGLContext(this);
    SetCurrent(*glcnt);
    Show(true);
    
    timer=new wxTimer(this,0);
    map_timer=new wxTimer(this,1);
    
    busy=0;
    
    wxCHECK_RET(gl3wInit()==0,"Couldn't initialize gl3w");
    wxCHECK_RET(gl3wIsSupported(4,3)==1,"OpenGL 4.3 not supported, please update your graphics drivers");
    
    gl_text.initialize();
    
    // Borders
    
    prog_borders=Glite::create_program(PathManager::locate_resource("resources/glsl/HAFDMS_st_vs.glsl"),
                                       PathManager::locate_resource("resources/glsl/HAFDMS_st_fs.glsl"));
    
    glGenVertexArrays(1,&border_vao);
    glBindVertexArray(border_vao);
    
    GLuint border_vbuff;
    glGenBuffers(1,&border_vbuff);
    
    GLfloat *data_v=new GLfloat[24];
    
    double s=1;
    double z=0;
    
    data_v[ 0]=-s; data_v[ 1]=-s; data_v[ 2]=z; data_v[ 3]=1.0;
    data_v[ 4]=+s; data_v[ 5]=-s; data_v[ 6]=z; data_v[ 7]=1.0;
    data_v[ 8]=-s; data_v[ 9]=+s; data_v[10]=z; data_v[11]=1.0;
    
    data_v[12]=+s; data_v[13]=-s; data_v[14]=z; data_v[15]=1.0;
    data_v[16]=+s; data_v[17]=+s; data_v[18]=z; data_v[19]=1.0;
    data_v[20]=-s; data_v[21]=+s; data_v[22]=z; data_v[23]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,border_vbuff);
    glBufferData(GL_ARRAY_BUFFER,24*sizeof(GLfloat),(void*)data_v,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    // Cross
    
    prog_cross=Glite::create_program(PathManager::locate_resource("resources/glsl/HAFDMS_cross_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/HAFDMS_cross_fshader.glsl"));
    
    GLuint cross_vbuff;
    
    glGenVertexArrays(1,&cross_vao);
    glBindVertexArray(cross_vao);
    
    glGenBuffers(1,&cross_vbuff);
    
    GLfloat *cross_data=new GLfloat[16];
    
    cross_data[ 0]=-1.0; cross_data[ 1]= 0.0; cross_data[ 2]=0.0; cross_data[ 3]=1.0;
    cross_data[ 4]=+1.0; cross_data[ 5]= 0.0; cross_data[ 6]=0.0; cross_data[ 7]=1.0;
    cross_data[ 8]= 0.0; cross_data[ 9]=-1.0; cross_data[10]=0.0; cross_data[11]=1.0;
    cross_data[12]= 0.0; cross_data[13]=+1.0; cross_data[14]=0.0; cross_data[15]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,cross_vbuff);
    glBufferData(GL_ARRAY_BUFFER,16*sizeof(GLfloat),(void*)cross_data,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] cross_data;
    
    // Map
    
    prog_map=Glite::create_program(PathManager::locate_resource("resources/glsl/HAFDMS_map_vshader.glsl"),
                                   PathManager::locate_resource("resources/glsl/HAFDMS_map_fshader_jet.glsl"));
    
    glGenVertexArrays(1,&map_vao);
    glBindVertexArray(map_vao);
    
    GLuint map_vbuff,map_uv;
    glGenBuffers(1,&map_vbuff);
    glGenBuffers(1,&map_uv);
    
    data_v[ 0]=0; data_v[ 1]=0; data_v[ 2]=0; data_v[ 3]=1.0;
    data_v[ 4]=1; data_v[ 5]=0; data_v[ 6]=0; data_v[ 7]=1.0;
    data_v[ 8]=0; data_v[ 9]=1; data_v[10]=0; data_v[11]=1.0;
    
    data_v[12]=1; data_v[13]=0; data_v[14]=0; data_v[15]=1.0;
    data_v[16]=1; data_v[17]=1; data_v[18]=0; data_v[19]=1.0;
    data_v[20]=0; data_v[21]=1; data_v[22]=0; data_v[23]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,map_vbuff);
    glBufferData(GL_ARRAY_BUFFER,24*sizeof(GLfloat),(void*)data_v,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    GLfloat *uv_arr=new GLfloat[12];
    
    uv_arr[ 0]=0; uv_arr[ 1]=0;
    uv_arr[ 2]=1; uv_arr[ 3]=0;
    uv_arr[ 4]=0; uv_arr[ 5]=1;
    
    uv_arr[ 6]=1; uv_arr[ 7]=0;
    uv_arr[ 8]=1; uv_arr[ 9]=1;
    uv_arr[10]=0; uv_arr[11]=1;
    
    glBindBuffer(GL_ARRAY_BUFFER,map_uv);
    glBufferData(GL_ARRAY_BUFFER,
                 12*sizeof(GLfloat),
                 (void*)uv_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] data_v;
    delete[] uv_arr;
    
    glGenTextures(1,&map_tex);
    glBindTexture(GL_TEXTURE_2D,map_tex);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,N1,N2);
    
    GLfloat *tex_data=new GLfloat[256*256];
    for(int k=0;k<256*256;k++) tex_data[k]=((k/16)%16)/16.0;
    
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RED,GL_FLOAT,tex_data);
    
    delete[] tex_data;
    
    //
    
    glGenSamplers(1,&sampler);
    glBindSampler(0,sampler);
    
    glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    // Target
    
    prog_target=Glite::create_program(PathManager::locate_resource("resources/glsl/HAFDMS_target_vshader.glsl"),
                                      PathManager::locate_resource("resources/glsl/HAFDMS_target_fshader.glsl"));
    prog_target_outline=Glite::create_program(PathManager::locate_resource("resources/glsl/HAFDMS_target_vshader.glsl"),
                                              PathManager::locate_resource("resources/glsl/HAFDMS_target_outline_fshader.glsl"));
    
    glGenVertexArrays(1,&target_vao);
    glBindVertexArray(target_vao);
    
    GLuint target_vbuff;
    glGenBuffers(1,&target_vbuff);
    
    GLfloat *target_vdata=new GLfloat[4*68];
    
    target_vdata[ 0]=-1.0; target_vdata[ 1]= 0.0; target_vdata[ 2]=0.0; target_vdata[ 3]=1.0;
    target_vdata[ 4]=+1.0; target_vdata[ 5]= 0.0; target_vdata[ 6]=0.0; target_vdata[ 7]=1.0;
    target_vdata[ 8]= 0.0; target_vdata[ 9]=-1.0; target_vdata[10]=0.0; target_vdata[11]=1.0;
    target_vdata[12]= 0.0; target_vdata[13]=+1.0; target_vdata[14]=0.0; target_vdata[15]=1.0;
    
    for(int i=0;i<32;i++)
    {
        double ang_1=2.0*Pi*i/32.0;
        double ang_2=2.0*Pi*(i+1)/32.0;
        
        target_vdata[16+4*(2*i+0)+0]=0.75*std::cos(ang_1);
        target_vdata[16+4*(2*i+0)+1]=0.75*std::sin(ang_1);
        target_vdata[16+4*(2*i+0)+2]=0;
        target_vdata[16+4*(2*i+0)+3]=1.0;
        
        target_vdata[16+4*(2*i+1)+0]=0.75*std::cos(ang_2);
        target_vdata[16+4*(2*i+1)+1]=0.75*std::sin(ang_2);
        target_vdata[16+4*(2*i+1)+2]=0;
        target_vdata[16+4*(2*i+1)+3]=1.0;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER,target_vbuff);
    glBufferData(GL_ARRAY_BUFFER,4*68*sizeof(GLfloat),(void*)target_vdata,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] target_vdata;
    
    //
    
//    Bind(wxEVT_ENTER_WINDOW,&HAMS_GL::evt_mouse_motion,this);
    Bind(wxEVT_KEY_DOWN,&HAMS_GL::keyboard,this);
    Bind(wxEVT_LEFT_DCLICK,&HAMS_GL::evt_mouse_motion,this);
    Bind(wxEVT_LEFT_DOWN,&HAMS_GL::evt_mouse_motion,this);
    Bind(wxEVT_MOTION,&HAMS_GL::evt_mouse_motion,this);
    Bind(wxEVT_MOUSEWHEEL,&HAMS_GL::mouse_wheel,this);
    Bind(wxEVT_RIGHT_DOWN,&HAMS_GL::evt_mouse_motion,this);
    Bind(wxEVT_SIZE,&HAMS_GL::resize,this);
    Bind(wxEVT_TIMER,&HAMS_GL::timed_refresh,this);
    timer->Start(1000/30);
    
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
    
    glDisable(GL_CULL_FACE);
}

HAMS_GL::~HAMS_GL()
{
    timer->Stop();
    delete glcnt;
}

void HAMS_GL::add_target(double nr,double ni)
{
    MSTarget tmp(nr,ni);
    
    targets.push_back(tmp);
    
    wxCommandEvent event(EVT_NEW_TARGET);
    event.SetClientData(&targets.back());
    
    wxPostEvent(this,event);
}

void HAMS_GL::clear_targets()
{
    targets.clear();
}

void HAMS_GL::enable_target_mode() { target_mode=true; }

void HAMS_GL::forget_target(MSTarget const &target)
{
    unsigned int i,k;
    
    for(i=0;i<targets.size();i++)
    {
        if(targets[i]==target)
        {
            for(k=i+1;k<targets.size();k++)
                targets[k-1]=targets[k];
            
            targets.pop_back();
            return;
        }
    }
}

std::vector<MSTarget> HAMS_GL::get_targets() { return targets; }

void HAMS_GL::keyboard(wxKeyEvent &event)
{
    int key=event.GetKeyCode();
    
    if(key==WXK_ESCAPE)
    {
        if(target_mode) target_mode=false;
        if(zoom_mode) zoom_mode=false;
    }
}

void HAMS_GL::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(target_mode)
    {
        target_nr=mx_to_nr(mouse_x);
        target_ni=my_to_ni(mouse_y);
    }
    
    if(zoom_mode)
    {
        zoom_nr_e=mx_to_nr(mouse_x);
        zoom_ni_e=my_to_ni(mouse_y);
    }
    
//    if(event.Entering())
//    {
//        SetFocus();
//    }
    
    if(event.Dragging())
    {
        if(event.LeftIsDown())
        {
            map_timer->Stop();
            
            double dx=(mouse_x-mouse_xp)/(GetSize().x+0.0)*(xmax-xmin);
            double dy=(mouse_y-mouse_yp)/(GetSize().y+0.0)*(ymax-ymin);
            
            xmin-=dx; xmax-=dx;
            ymin+=dy; ymax+=dy;
            
            update_camera();
            
            map_timer->StartOnce(250);
        }
        else if(event.RightIsDown())
        {
            
        }
    }
    else if(event.LeftDClick())
    {
        target_mode=false;
        
        target_nr=mx_to_nr(mouse_x);
        target_ni=my_to_ni(mouse_y);
        
        target_ni=std::max(0.0,target_ni);
        
        add_target(target_nr,target_ni);
    }
    else
    {
        if(event.LeftIsDown())
        {
            if(zoom_mode==true)
            {
                zoom_mode=false;
                
                zoom_nr_e=mx_to_nr(mouse_x);
                zoom_ni_e=my_to_ni(mouse_y);
                
                xmin=std::min(zoom_nr_s,zoom_nr_e);
                xmax=std::max(zoom_nr_s,zoom_nr_e);
                ymin=std::min(zoom_ni_s,zoom_ni_e);
                ymax=std::max(zoom_ni_s,zoom_ni_e);
                
                request_map();
            }
            else if(target_mode==true)
            {
                target_mode=false;
                
                target_nr=mx_to_nr(mouse_x);
                target_ni=my_to_ni(mouse_y);
                
                target_ni=std::max(0.0,target_ni);
                
                add_target(target_nr,target_ni);
            }
        }
        else if(event.RightIsDown())
        {
            if(zoom_mode==false)
            {
                zoom_mode=true;
                zoom_nr_s=mx_to_nr(mouse_x);
                zoom_ni_s=my_to_ni(mouse_y);
                zoom_nr_e=zoom_nr_s;
                zoom_ni_e=zoom_ni_s;
            }
            else
            {
                zoom_mode=false;
                
                zoom_nr_e=mx_to_nr(mouse_x);
                zoom_ni_e=my_to_ni(mouse_y);
                
                xmin=std::min(zoom_nr_s,zoom_nr_e);
                xmax=std::max(zoom_nr_s,zoom_nr_e);
                ymin=std::min(zoom_ni_s,zoom_ni_e);
                ymax=std::max(zoom_ni_s,zoom_ni_e);
                
                request_map();
            }
        }
    }
    
    event.Skip();
}

double HAMS_GL::mx_to_nr(int i)
{
    double u=(i-pad_xm)/static_cast<double>(sx-pad_xp-pad_xm);
    return xmin+u*(xmax-xmin);
}

double HAMS_GL::my_to_ni(int j)
{
    double v=(sy-j-pad_ym)/static_cast<double>(sy-pad_yp-pad_ym);
    return ymin+v*(ymax-ymin);
}

void HAMS_GL::mouse_wheel(wxMouseEvent &event)
{
    map_timer->Stop();
    
    double nr_w=mx_to_nr(event.GetX());
    double ni_w=my_to_ni(event.GetY());
    
    double x1=nr_w-xmin;
    double x2=xmax-nr_w;
    
    double y1=ni_w-ymin;
    double y2=ymax-ni_w;
    
    if(event.GetWheelRotation()>0)
    {
        xmin=nr_w-0.95*x1; xmax=nr_w+0.95*x2;
        ymin=ni_w-0.95*y1; ymax=ni_w+0.95*y2;
    }
    else if(event.GetWheelRotation()<0)
    {
        xmin=nr_w-1.05*x1; xmax=nr_w+1.05*x2;
        ymin=ni_w-1.05*y1; ymax=ni_w+1.05*y2;
    }
    
    map_timer->StartOnce(250);
}

void HAMS_GL::render()
{
    unsigned int i;
    
    if(IsShown())
    {
        SetCurrent(*glcnt);
        if(!busy)
        {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            update_camera();
            
            glUseProgram(prog_map);
            glBindVertexArray(map_vao);
            
            glUniformMatrix4fv(10,1,0,proj_gl);
            glUniform1f(11,exposure);
            glVertexAttrib4f(2,GLfloat(c_xp-c_xm),0,0,0);
            glVertexAttrib4f(3,0,GLfloat(c_yp-c_ym),0,0);
            glVertexAttrib4f(4,0,0,1,0);
            glVertexAttrib4f(5,GLfloat(c_xm),GLfloat(c_ym),0,1);
            
            glBindTexture(GL_TEXTURE_2D,map_tex);
            
            glDrawArrays(GL_TRIANGLES,0,6);
            
            // Drawing Cross
            
            glUseProgram(prog_cross);
            glBindVertexArray(cross_vao);
            glUniformMatrix4fv(2,1,0,proj_gl);
            
            glVertexAttrib4f(1,0,0,0,1.0);
            glDrawArrays(GL_LINES,0,4);
            
            glVertexAttrib4f(1,n_sub,0,0,1.0);
            glDrawArrays(GL_LINES,0,4);
            
            glVertexAttrib4f(1,n_sup,0,0,1.0);
            glDrawArrays(GL_LINES,0,4);
            
            if(zoom_mode)
            {
                glVertexAttrib4f(1,zoom_nr_s,zoom_ni_s,0,1.0);
                glDrawArrays(GL_LINES,0,4);
                
                glVertexAttrib4f(1,zoom_nr_e,zoom_ni_e,0,1.0);
                glDrawArrays(GL_LINES,0,4);
            }
            
            if(target_mode)
            {
                glVertexAttrib4f(1,target_nr,target_ni,0,1.0);
                glDrawArrays(GL_LINES,0,4);
            }
            
            // Targets
            
            glUseProgram(prog_target_outline);
            
            glLineWidth(5.0);
            
            glBindVertexArray(target_vao);
            
            glVertexAttrib4f(2,40.0/sx,40.0/sy,0,0);
            glUniformMatrix4fv(3,1,0,proj_gl);
            
            for(i=0;i<targets.size();i++)
            {
                glVertexAttrib4f(1,targets[i].nr,targets[i].ni,0,1.0);
                glDrawArrays(GL_LINES,0,68);
            }
            
            glUseProgram(prog_target);
            
            glLineWidth(1.0);
            
            glVertexAttrib4f(2,40.0/sx,40.0/sy,0,0);
            glUniformMatrix4fv(3,1,0,proj_gl);
            
            for(i=0;i<targets.size();i++)
            {
                glVertexAttrib4f(1,targets[i].nr,targets[i].ni,0,1.0);
                glDrawArrays(GL_LINES,0,68);
                
                std::stringstream strm;
                strm<<targets[i].nr<<"\n"<<targets[i].ni;
                
                gl_text.request_text(targets[i].nr,targets[i].ni,0,strm.str(),
                                     0.02,sx*0.02/sy,0.06,sx*0.06/sy);
            }
            
            // Drawing Borders
            
            glUseProgram(prog_borders);
            glBindVertexArray(border_vao);
            
            glVertexAttrib4f(1,g_xp+1,0,0,0);
            glDrawArrays(GL_TRIANGLES,0,6);
            
            glVertexAttrib4f(1,g_xm-1,0,0,0);
            glDrawArrays(GL_TRIANGLES,0,6);
            
            glVertexAttrib4f(1,0,g_yp+1,0,0);
            glDrawArrays(GL_TRIANGLES,0,6);
            
            glVertexAttrib4f(1,0,g_ym-1,0,0);
            glDrawArrays(GL_TRIANGLES,0,6);
            
            gl_text.render(proj_gl);
            
            SwapBuffers();
        }
    }
}

void HAMS_GL::request_map()
{
    MSMapEvent event(EVT_MAP_REQUEST);
    
    event.nr_min=xmin;
    event.nr_max=xmax;
    
    event.ni_min=ymin;
    event.ni_max=ymax;
    
    event.Nr=sx-pad_xm-pad_xp;
    event.Ni=sy-pad_ym-pad_yp;
    
    wxPostEvent(this,event);
}

void HAMS_GL::resize(wxSizeEvent &event)
{
    sx=GetSize().x;
    sy=GetSize().y;
    
    update_size();
}

void HAMS_GL::set_exposure(double exposure_)
{
    exposure=static_cast<GLfloat>(std::pow(10.0,exposure_));
}

void HAMS_GL::set_map(int Nr,int Ni,double nr_min,double nr_max,double ni_min,double ni_max,GLfloat *map)
{
    SetCurrent(*glcnt);
    
    c_xm=nr_min;
    c_xp=nr_max;
    
    c_ym=ni_min;
    c_yp=ni_max;
    
    if(N1!=Nr || N2!=Ni)
    {
        N1=Nr;
        N2=Ni;
        
        glDeleteTextures(1,&map_tex);
        glGenTextures(1,&map_tex);
        glBindTexture(GL_TEXTURE_2D,map_tex);
        
        glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,N1,N2);
    }
    
    glBindTexture(GL_TEXTURE_2D,map_tex);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RED,GL_FLOAT,map);
}

void HAMS_GL::set_substrate(double n_sub_) { n_sub=static_cast<GLfloat>(n_sub_); }
void HAMS_GL::set_superstrate(double n_sup_) { n_sup=static_cast<GLfloat>(n_sup_); }

void HAMS_GL::set_zoom(double nr_min,double nr_max,double ni_min,double ni_max)
{
    xmin=nr_min;
    xmax=nr_max;
    
    ymin=ni_min;
    ymax=ni_max;
}

void HAMS_GL::timed_refresh(wxTimerEvent &event)
{
         if(event.GetId()==0) render();
    else if(event.GetId()==1) request_map();
}

void HAMS_GL::paint_refresh(wxPaintEvent &event)
{
    render();
}

void HAMS_GL::update_camera()
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

void HAMS_GL::update_size()
{
    SetCurrent(*glcnt);
    
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    g_xm=-1.0+2.0*pad_xm/(Sx+0.0);
    g_ym=-1.0+2.0*pad_ym/(Sy+0.0);
    
    g_xp=+1.0-2.0*pad_xp/(Sx+0.0);
    g_yp=+1.0-2.0*pad_yp/(Sy+0.0);
    
    glViewport(0,0,(GLint)Sx,(GLint)Sy);
}
