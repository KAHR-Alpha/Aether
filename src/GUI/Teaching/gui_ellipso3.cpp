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

#include <gui_ellipso3.h>

extern const Imdouble Im;

GL_Ellipso3D::GL_Ellipso3D(wxWindow *parent,AngleOld const &incidence_,
                           Imdouble const &ip_,Imdouble const &is_,
                           Imdouble const &rp_,Imdouble const &rs_,
                           int density_,double span_)
    :GL_3D_Base(parent),
     watch_paused(false),
     period(10.0),
     incidence(incidence_),
     sv(0,1,0),
     ip(ip_), is(is_),
     rp(rp_), rs(rs_),
     density(density_), span(span_)
{
    
}

GL_Ellipso3D::~GL_Ellipso3D()
{
    watch->Pause();
}

void GL_Ellipso3D::init_opengl()
{
    cam_t(0,0,0.5);
    
    prog_field=Glite::create_program(PathManager::locate_resource("resources/glsl/ellipso3_field_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/ellipso3_field_fshader.glsl"));
    
    GLuint v_buff,vn_buff,index_buff;
    glGenBuffers(1,&v_buff);
    glGenBuffers(1,&vn_buff);
    glGenBuffers(1,&index_buff);
    
    // Field
    
    glGenVertexArrays(1,&vao_field);
    
    int i;
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
    Glite::make_unitary_block(V_arr,F_arr);
    
    int Nv=V_arr.size();
    int Nf=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[3*Nf];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x-0.5;
        v_arr_buff[4*i+1]=V_arr[i].loc.y-0.5;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Nf;i++)
    {
        ind_arr_buff[3*i+0]=F_arr[i].V1;
        ind_arr_buff[3*i+1]=F_arr[i].V2;
        ind_arr_buff[3*i+2]=F_arr[i].V3;
    }
    
    glBindVertexArray(vao_field);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3*Nf*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
    
    // Frame
    
    prog_frame=Glite::create_program(PathManager::locate_resource("resources/glsl/ellipso3_frame_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/ellipso3_frame_fshader.glsl"));
    
    v_arr_buff=new GLfloat[4*6];
    
    v_arr_buff[0]=-4.0; v_arr_buff[1]=0; v_arr_buff[2]=0; v_arr_buff[3]=1.0;
    v_arr_buff[4]=+4.0; v_arr_buff[5]=0; v_arr_buff[6]=0; v_arr_buff[7]=1.0;
    
    v_arr_buff[ 8]=0; v_arr_buff[ 9]=-1.0; v_arr_buff[10]=0; v_arr_buff[11]=1.0;
    v_arr_buff[12]=0; v_arr_buff[13]=+1.0; v_arr_buff[14]=0; v_arr_buff[15]=1.0;
    
    v_arr_buff[16]=0; v_arr_buff[17]=0; v_arr_buff[18]=-1.0; v_arr_buff[19]=1.0;
    v_arr_buff[20]=0; v_arr_buff[21]=0; v_arr_buff[22]=+4.0; v_arr_buff[23]=1.0;
    
    GLuint v_buff_frame;
    
    glGenBuffers(1,&v_buff_frame);
    glGenVertexArrays(1,&vao_frame);
    
    glBindVertexArray(vao_frame);    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff_frame);
    
    glBufferData(GL_ARRAY_BUFFER,24*sizeof(GLfloat),(void*)v_arr_buff,GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] v_arr_buff;
    
    watch=new wxStopWatch();
    
    recompute_vectors();
}

void GL_Ellipso3D::align_camera_incidence_plane()
{
    set_camera_target(Vector3(0,0,0));
    set_camera_radius(1.2);
    set_camera_direction(Vector3(0,1,0));
}

void GL_Ellipso3D::align_camera_polarization()
{
    set_camera_target(Vector3(0,0,0));
    set_camera_radius(1.2);
    set_camera_direction(-ki);
}

void GL_Ellipso3D::recompute_vectors()
{
    ki(std::cos(incidence),0,-std::sin(incidence));
    kr(std::cos(incidence),0,+std::sin(incidence));
    
    pi.crossprod(ki,sv);
    pr.crossprod(kr,sv);
    
    pi.normalize();
    pr.normalize();
}

void GL_Ellipso3D::render()
{
    // Frame
    
    glUseProgram(prog_frame);
    glBindVertexArray(vao_frame);
    
    glUniformMatrix4fv(1,1,GL_FALSE,camera.proj_gl);
    glDrawArrays(GL_LINES,0,6);
    
    // Field
    
    double time_arg=watch->Time()/1000.0/period;
    
    double df=1.0/(density-1.0);
    
    double s=0;
    
    glUseProgram(prog_field);
    
    glUniformMatrix4fv(10,1,GL_FALSE,camera.proj_gl);
    Vector3 sun_dir(1.0,1.0,1.0);
    sun_dir.set_spherical(1.0,Pi/4.0,Pi/3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    glUniform4f(11,sun_dir.x,
                   sun_dir.y,
                   sun_dir.z,
                   0);
    
    glBindVertexArray(vao_field);
    
    Vector3 E,nE;
    
    while(s<=span)
    {
        Imdouble ci=std::exp(2.0*Pi*(-s-time_arg)*Im);
        Imdouble cr=std::exp(2.0*Pi*(+s-time_arg)*Im);
        
        double cis=std::real(ci*is);
        double cip=std::real(ci*ip);
        
        // Incident
        
        E=cis*sv+cip*pi;
        
        nE.crossprod(ki,E);
        nE.normalize();
        
        double f=0.5*df;
        
        glVertexAttrib4f(2,1.0,0.4,0.4,1.0);
        glVertexAttrib4f(3,f*ki.x,f*ki.y,f*ki.z,0);
        glVertexAttrib4f(4,f*nE.x,f*nE.y,f*nE.z,0);
        glVertexAttrib4f(5,E.x,E.y,E.z,0);
        glVertexAttrib4f(6,-s*ki.x,-s*ki.y,-s*ki.z,1.0);
        
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        
        // Reflected
        
        double crs=std::real(cr*rs);
        double crp=std::real(cr*rp);
        
        E=crs*sv+crp*pr;
        
        nE.crossprod(kr,E);
        nE.normalize();
        
        glVertexAttrib4f(2,0.256,0.376,1.0,0);
        glVertexAttrib4f(3,f*kr.x,f*kr.y,f*kr.z,0);
        glVertexAttrib4f(4,f*nE.x,f*nE.y,f*nE.z,0);
        glVertexAttrib4f(5,E.x,E.y,E.z,0);
        glVertexAttrib4f(6,s*kr.x,s*kr.y,s*kr.z,1.0);
        
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        
        s+=df;
    }
}

void GL_Ellipso3D::set_coefficients(Imdouble const &ip_,Imdouble const &is_,
                                    Imdouble const &rp_,Imdouble const &rs_)
{
    is=is_; ip=ip_;
    rs=rs_; rp=rp_;
}

void GL_Ellipso3D::set_density(int density_)
{
    density=density_;
}

void GL_Ellipso3D::set_incidence(AngleOld const &angle)
{
    incidence=angle;
    
    recompute_vectors();
}

void GL_Ellipso3D::set_period(double period_)
{
    period=period_;
}

void GL_Ellipso3D::set_span(double span_)
{
    span=span_;
}

void GL_Ellipso3D::switch_watch()
{
    if(watch_paused)
    {
        watch->Resume();
        watch_paused=false;
    }
    else
    {
        watch->Pause();
        watch_paused=true;
    }
}

//####################
//   Ellipso3DFrame
//####################

Ellipso3DFrame::Ellipso3DFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxPanel *options_panel=new wxPanel(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    wxBoxSizer *options_sizer=new wxBoxSizer(wxVERTICAL);
    
    wavelength=new WavelengthSelector(options_panel,"Wavelength",550e-9);
    
    options_sizer->Add(wavelength,wxSizerFlags().Expand());
    
    // Substrate
    
    wxStaticBoxSizer *substrate_sizer=new wxStaticBoxSizer(wxVERTICAL,options_panel,"Substrate");
    
    substrate=new MiniMaterialSelector(options_panel);
    substrate_sizer->Add(substrate,wxSizerFlags().Expand());
    
    options_sizer->Add(substrate_sizer,wxSizerFlags().Expand());
    
    // Incidence
    
    wxStaticBoxSizer *angle_sizer=new wxStaticBoxSizer(wxVERTICAL,options_panel,"Angle");
    
    angle=new SliderDisplay(options_panel,901,0,90,1.0,"°",true);
    angle->set_value(45);
    
    angle_sizer->Add(angle,wxSizerFlags().Expand());
    
    options_sizer->Add(angle_sizer,wxSizerFlags().Expand());
    
    // Display Options
    
    wxStaticBoxSizer *display_options_sizer=new wxStaticBoxSizer(wxVERTICAL,options_panel,"Display");
    
    density=new NamedTextCtrl<int>(options_panel,"Density: ",30);
    span=new NamedTextCtrl<double>(options_panel,"Span: ",4.0);
    
    display_options_sizer->Add(density,wxSizerFlags().Expand());
    display_options_sizer->Add(span,wxSizerFlags().Expand());
    
    options_sizer->Add(display_options_sizer,wxSizerFlags().Expand());
    
    // Alignements
    
    wxStaticBoxSizer *align_sizer=new wxStaticBoxSizer(wxVERTICAL,options_panel,"Align Camera");
    
    wxButton *align_polar_btn=new wxButton(options_panel,wxID_ANY,"Polarization");
    wxButton *align_inc_btn=new wxButton(options_panel,wxID_ANY,"Incidence");
    
    align_sizer->Add(align_polar_btn,wxSizerFlags().Expand());
    align_sizer->Add(align_inc_btn,wxSizerFlags().Expand());
    
    options_sizer->Add(align_sizer,wxSizerFlags().Expand());
    
    options_panel->SetSizer(options_sizer);
    
    // Display Panel
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *display_ctrl_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *compo_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *p_sizer=new wxStaticBoxSizer(wxHORIZONTAL,display_panel,"P Component");
    wxBoxSizer *p_amp_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *p_phase_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *s_sizer=new wxStaticBoxSizer(wxHORIZONTAL,display_panel,"S Component");
    wxBoxSizer *s_amp_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *s_phase_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *time_sizer=new wxStaticBoxSizer(wxVERTICAL,display_panel,"Time");
    
    gl=new GL_Ellipso3D(display_panel,Degree(45),
                        1.0,0,1.0,0,
                        30,4.0);
    
    period_ctrl=new NamedTextCtrl<double>(display_panel,"",10);
    wxButton *pause_btn=new wxButton(display_panel,wxID_ANY,"Pause");
    
    // P Component
    
    p_amp=new SliderDisplay(display_panel,1001,0,1.0);
    p_phase=new SliderDisplay(display_panel,361,-180,180,1,"°");
    
    p_amp->set_value(1.0);
    p_phase->set_value(0);
    
    p_amp_sizer->Add(p_amp,wxSizerFlags().Expand());
    p_phase_sizer->Add(p_phase,wxSizerFlags().Expand());
    
    p_sizer->Add(p_amp_sizer,wxSizerFlags(1));
    p_sizer->Add(p_phase_sizer,wxSizerFlags(1));
    
    // S Component
    
    s_amp=new SliderDisplay(display_panel,1001,0,1.0);
    s_phase=new SliderDisplay(display_panel,361,-180,180,1,"°");
    
    s_amp->set_value(0);
    s_phase->set_value(0);
    
    s_amp_sizer->Add(s_amp,wxSizerFlags().Expand());
    s_phase_sizer->Add(s_phase,wxSizerFlags().Expand());
    
    s_sizer->Add(s_amp_sizer,wxSizerFlags(1));
    s_sizer->Add(s_phase_sizer,wxSizerFlags(1));
    
    compo_sizer->Add(p_sizer,wxSizerFlags().Expand());
    compo_sizer->Add(s_sizer,wxSizerFlags().Expand());
    
    time_sizer->Add(period_ctrl,wxSizerFlags().Expand().Border(wxALL,3));
    time_sizer->Add(pause_btn,wxSizerFlags().Expand());
    
    display_ctrl_sizer->Add(compo_sizer,wxSizerFlags(1));
    display_ctrl_sizer->Add(time_sizer);
    
    display_sizer->Add(gl,wxSizerFlags(1).Expand());
    display_sizer->Add(display_ctrl_sizer,wxSizerFlags().Expand());
    
    display_panel->SetSizer(display_sizer);
    
    splitter->SplitVertically(options_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    //Bindings
    
    wavelength->Bind(EVT_WAVELENGTH_SELECTOR,&Ellipso3DFrame::evt_wavelength,this);
    angle->Bind(EVT_SLIDERDISPLAY,&Ellipso3DFrame::evt_angle,this);
    substrate->Bind(EVT_MINIMAT_SELECTOR,&Ellipso3DFrame::evt_material,this);
    density->Bind(EVT_NAMEDTXTCTRL,&Ellipso3DFrame::evt_density,this);
    span->Bind(EVT_NAMEDTXTCTRL,&Ellipso3DFrame::evt_span,this);
    align_polar_btn->Bind(wxEVT_BUTTON,&Ellipso3DFrame::evt_align_polarization,this);
    align_inc_btn->Bind(wxEVT_BUTTON,&Ellipso3DFrame::evt_align_incidence_plane,this);
    
    p_amp->Bind(EVT_SLIDERDISPLAY,&Ellipso3DFrame::evt_component,this);
    p_phase->Bind(EVT_SLIDERDISPLAY,&Ellipso3DFrame::evt_component,this);
    s_amp->Bind(EVT_SLIDERDISPLAY,&Ellipso3DFrame::evt_component,this);
    s_phase->Bind(EVT_SLIDERDISPLAY,&Ellipso3DFrame::evt_component,this);
    
    pause_btn->Bind(wxEVT_BUTTON,&Ellipso3DFrame::evt_pause,this);
    period_ctrl->Bind(EVT_NAMEDTXTCTRL,&Ellipso3DFrame::evt_period,this);
    
    evt_component_sub();
}

void Ellipso3DFrame::evt_align_incidence_plane(wxCommandEvent &event)
{
    gl->align_camera_incidence_plane();
    
    span->set_value(4);
    gl->set_span(4);
    
    event.Skip();
}

void Ellipso3DFrame::evt_align_polarization(wxCommandEvent &event)
{
    gl->align_camera_polarization();
    
    span->set_value(0);
    gl->set_span(0);
    
    event.Skip();
}

void Ellipso3DFrame::evt_angle(wxCommandEvent &event)
{
    gl->set_incidence(Degree(90-angle->get_value()));
    
    evt_component_sub();
}

void Ellipso3DFrame::evt_component(wxCommandEvent &event)
{
    evt_component_sub();
}

void Ellipso3DFrame::evt_component_sub()
{
    Imdouble ip=p_amp->get_value()*std::exp(-Pi*p_phase->get_value()/180.0*Im);
    Imdouble is=s_amp->get_value()*std::exp(-Pi*s_phase->get_value()/180.0*Im);
    
    Imdouble r_te,r_tm,t_te,t_tm;
    
    fresnel_rt_te_tm(Degree(angle->get_value()),
                     1.0,substrate->get_n(m_to_rad_Hz(wavelength->get_lambda())),
                     r_te,r_tm,t_te,t_tm);
    
    gl->set_coefficients(ip,is,ip*r_tm,is*r_te);
}

void Ellipso3DFrame::evt_density(wxCommandEvent &event)
{
    int N=density->get_value();
    
    if(N<2)
    {
        N=2;
        density->set_value(2);
    }
    
    gl->set_density(N);
    
    event.Skip();
}

void Ellipso3DFrame::evt_material(wxCommandEvent &event)
{
    evt_component_sub();
}

void Ellipso3DFrame::evt_pause(wxCommandEvent &event)
{
    gl->switch_watch();
    
    event.Skip();
}

void Ellipso3DFrame::evt_period(wxCommandEvent &event)
{
    gl->set_period(period_ctrl->get_value());
}

void Ellipso3DFrame::evt_span(wxCommandEvent &event)
{
    gl->set_span(span->get_value());
    
    event.Skip();
}

void Ellipso3DFrame::evt_wavelength(wxCommandEvent &event)
{
    evt_component_sub();
}
