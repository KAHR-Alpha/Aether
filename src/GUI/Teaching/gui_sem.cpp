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

#include <gui_sem.h>

extern const double Pi;
extern const Imdouble Im;

GL_SEM_2D::GL_SEM_2D(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     x_min(-3.0), x_max(3.0),
     scale(1.0),
     gl_ok(false),
     N_electrons(0)
{
    glcnt=new wxGLContext(this);
    SetCurrent(*glcnt);
    Show(true);
    
    Bind(wxEVT_MOUSEWHEEL,&GL_SEM_2D::evt_mouse,this);
}

GL_SEM_2D::~GL_SEM_2D()
{
    delete[] profile_vertex_buffer;
}

void GL_SEM_2D::evt_mouse(wxMouseEvent &event)
{
    int r=event.GetWheelRotation();
    
    if(r<0) scale*=1.1;
    else scale/=1.1;
}

void GL_SEM_2D::init_opengl(std::vector<double> const &profile_x,
                            std::vector<double> const &profile_z)
{
    x_min=x_max=profile_x[0];
    Ns_profile=profile_x.size();
    
    for(std::size_t i=0;i<profile_x.size();i++)
    {
        x_min=std::min(x_min,profile_x[i]);
        x_max=std::max(x_max,profile_x[i]);
    }
    
    gl_ok=true;
    
    SetCurrent(*glcnt);
    
    if(gl3wInit())
        wxMessageBox("Error: Could not initialize OpenGL");
    if(!gl3wIsSupported(4,3))
        wxMessageBox("Error: OpenGL 4.3 not supported.\n Update your graphics drivers.");
    
    update_viewport();
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glDisable(GL_CULL_FACE);
    
    // Profile
    
    prog_profile=Glite::create_program(PathManager::locate_resource("resources/glsl/sem/sem_2D_wires_vshader.glsl"),
                                       PathManager::locate_resource("resources/glsl/sem/sem_2D_wires_fshader.glsl"));
    
    GLuint index_buff;
    
    glGenBuffers(1,&gl_profile_vertex_buffer);
    glGenBuffers(1,&gl_profile_color_buffer);
    glGenBuffers(1,&index_buff);
    
    glGenVertexArrays(1,&vao_profile);

    int Np=profile_x.size();
    
    profile_vertex_buffer=new GLfloat[4*Np];
    profile_color_buffer=new GLfloat[Np];
    GLuint *ind_arr_buff=new GLuint[2*(Np-1)];
    
    for(int i=0;i<Np;i++)
    {
        profile_vertex_buffer[4*i+0]=profile_x[i];
        profile_vertex_buffer[4*i+1]=profile_z[i];
        profile_vertex_buffer[4*i+2]=0;
        profile_vertex_buffer[4*i+3]=1;
        
        profile_color_buffer[i]=1.0;
    }
    
    for(int i=0;i<Np-1;i++)
    {
        ind_arr_buff[2*i+0]=i;
        ind_arr_buff[2*i+1]=i+1;
    }
    
    glBindVertexArray(vao_profile);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_profile_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Np*sizeof(GLfloat),
                 (void*)profile_vertex_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_profile_color_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 Np*sizeof(GLfloat),
                 (void*)profile_color_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 2*(Np-1)*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] ind_arr_buff;
    
    // Electrons
    
    prog_electrons=Glite::create_program(PathManager::locate_resource("resources/glsl/sem/sem_2D_electrons_vshader.glsl"),
                                         PathManager::locate_resource("resources/glsl/sem/sem_2D_electrons_fshader.glsl"));
    
    glGenBuffers(1,&gl_electrons_vertex_buffer);
    glGenBuffers(1,&gl_electrons_color_buffer);
    
    glGenVertexArrays(1,&vao_electrons);
    
    N_electrons=100;
    
    electrons_vertex_buffer=new GLfloat[4*N_electrons];
    electrons_color_buffer=new GLfloat[3*N_electrons];
    
    for(int i=0;i<N_electrons;i++)
    {
        electrons_vertex_buffer[4*i+0]=randp(-3,3);
        electrons_vertex_buffer[4*i+1]=randp(-1,1);
        electrons_vertex_buffer[4*i+2]=0;
        electrons_vertex_buffer[4*i+3]=1;
        
        electrons_color_buffer[3*i+0]=0;
        electrons_color_buffer[3*i+1]=0;
        electrons_color_buffer[3*i+2]=0;
    }
    
    glBindVertexArray(vao_electrons);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_electrons_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 4*N_electrons*sizeof(GLfloat),
                 (void*)electrons_vertex_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_electrons_color_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 3*N_electrons*sizeof(GLfloat),
                 (void*)electrons_color_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);
    
    glLineWidth(4.0);
    glPointSize(3.0);
}

void GL_SEM_2D::update_viewport()
{
    Sx=GetSize().x;
    Sy=GetSize().y;
    
    glViewport(0, 0, (GLint)Sx, (GLint)Sy);
}

void GL_SEM_2D::render()
{
    SetCurrent(*glcnt);
    update_viewport();
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    
    GLfloat proj_gl[16];
    
    double y_scale=static_cast<double>(Sy)/Sx;
    
    proj_gl[0]=1.0/3.0/scale;
    proj_gl[1]=0;
    proj_gl[2]=0;
    proj_gl[3]=0;
    proj_gl[4]=0;
    proj_gl[5]=1.0/scale/3.0/y_scale;
    proj_gl[6]=0;
    proj_gl[7]=0;
    proj_gl[8]=0;
    proj_gl[9]=0;
    proj_gl[10]=1.0;
    proj_gl[11]=0;
    proj_gl[12]=0;
    proj_gl[13]=0;
    proj_gl[14]=0;
    proj_gl[15]=1;
    
    // Profile
    
    glUseProgram(prog_profile);
    glBindVertexArray(vao_profile);
    
    glUniformMatrix4fv(10,1,GL_FALSE,proj_gl);
    glDrawElements(GL_LINES,(Ns_profile-1)*2,GL_UNSIGNED_INT,0);
    
    // Electrons
    
    glUseProgram(prog_electrons);
    glBindVertexArray(vao_electrons);
    
    glUniformMatrix4fv(10,1,GL_FALSE,proj_gl);
    glDrawArrays(GL_POINTS,0,N_electrons);
    
    SwapBuffers();
}

void GL_SEM_2D::update_profile(std::vector<double> const &profile_x,
                               std::vector<double> const &profile_z,
                               std::vector<double> const &profile_color)
{
    if(!gl_ok) return;
    
    SetCurrent(*glcnt);
    
    int Np=profile_x.size();
    x_min=x_max=profile_x[0];
    
    for(int i=0;i<Np;i++)
    {
        x_min=std::min(x_min,profile_x[i]);
        x_max=std::max(x_max,profile_x[i]);
        
        profile_vertex_buffer[4*i+0]=profile_x[i];
        profile_vertex_buffer[4*i+1]=profile_z[i];
        profile_vertex_buffer[4*i+2]=0;
        profile_vertex_buffer[4*i+3]=1;
        
        profile_color_buffer[i]=profile_color[i];
    }
    
    glBindVertexArray(vao_profile);
        
    glBindBuffer(GL_ARRAY_BUFFER,gl_profile_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Np*sizeof(GLfloat),
                 (void*)profile_vertex_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_profile_color_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 Np*sizeof(GLfloat),
                 (void*)profile_color_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,1,GL_FLOAT,GL_TRUE,0,0);
}

void GL_SEM_2D::update_electrons(std::vector<Electron> const &electrons)
{
    if(!gl_ok) return;
    
    SetCurrent(*glcnt);
    
    if(N_electrons!=static_cast<int>(electrons.size()))
    {
        N_electrons=electrons.size();
        
        delete[] electrons_vertex_buffer;
        delete[] electrons_color_buffer;
        
        electrons_vertex_buffer=new GLfloat[4*N_electrons];
        electrons_color_buffer=new GLfloat[3*N_electrons];
    }
    
    for(int i=0;i<N_electrons;i++)
    {
        electrons_vertex_buffer[4*i+0]=electrons[i].loc.x;
        electrons_vertex_buffer[4*i+1]=electrons[i].loc.z;
        electrons_vertex_buffer[4*i+2]=0;
        electrons_vertex_buffer[4*i+3]=1;
        
        electrons_color_buffer[3*i+0]=electrons[i].color.x;
        electrons_color_buffer[3*i+1]=electrons[i].color.y;
        electrons_color_buffer[3*i+2]=electrons[i].color.z;
    }
    
    glBindVertexArray(vao_electrons);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_electrons_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 4*N_electrons*sizeof(GLfloat),
                 (void*)electrons_vertex_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,gl_electrons_color_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 3*N_electrons*sizeof(GLfloat),
                 (void*)electrons_color_buffer,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);
}

//####################
//   SEM_2D_Frame
//####################

SEM_2D_Frame::SEM_2D_Frame(wxString const &title)
    :BaseFrame(title),
     Nx(100),
     bins(Nx), bins_buffer(Nx),
     profile_x(Nx), profile_z(Nx), profile_color(Nx),
     scanning(false)
{
    for(int i=0;i<Nx;i++)
    {
        bins[i]=bins_buffer[i]=0;
        profile_color[i]=1.0;
    }

    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxPanel *ctrl_panel=new wxPanel(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    slope=new NamedTextCtrl<double>(ctrl_panel,"Geometry slope",1.0,true);
    slope->Bind(EVT_NAMEDTXTCTRL,&SEM_2D_Frame::evt_profile,this);
    
    ctrl_sizer->Add(slope,wxSizerFlags().Expand());
    
    // Electrons
    
    wxStaticBoxSizer *electrons_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Electrons");
    
    N_electrons=new NamedTextCtrl<int>(electrons_sizer->GetStaticBox(),"N: ",100);
    e_energy=new NamedTextCtrl<double>(electrons_sizer->GetStaticBox(),"Energy: ",10);
    e_mfp=new NamedTextCtrl<double>(electrons_sizer->GetStaticBox(),"MFP: ",0.1);
    SE_proportion=new NamedTextCtrl<double>(electrons_sizer->GetStaticBox(),"SE (0-1): ",0.05);
    
    electrons_sizer->Add(N_electrons,wxSizerFlags().Expand());
    electrons_sizer->Add(e_energy,wxSizerFlags().Expand());
    electrons_sizer->Add(e_mfp,wxSizerFlags().Expand());
    electrons_sizer->Add(SE_proportion,wxSizerFlags().Expand());
    ctrl_sizer->Add(electrons_sizer,wxSizerFlags().Expand());
    
    // Beam
    
    wxStaticBoxSizer *beam_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Beam Width");
    
    beam_width=new SliderDisplay(beam_sizer->GetStaticBox(),100,0,2);
    scanning_time=new NamedTextCtrl<int>(beam_sizer->GetStaticBox(),"Scanning (ms): ",5000);
    integration_time=new NamedTextCtrl<int>(beam_sizer->GetStaticBox(),"Integration (ms): ",10000);
    scan_btn=new wxToggleButton(beam_sizer->GetStaticBox(),wxID_ANY,"Scan");
    
    beam_sizer->Add(beam_width,wxSizerFlags().Expand());
    beam_sizer->Add(scanning_time,wxSizerFlags().Expand());
    beam_sizer->Add(integration_time,wxSizerFlags().Expand());
    beam_sizer->Add(scan_btn,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(beam_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display Panel
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    gl=new GL_SEM_2D(display_panel);
    beam_x_ctrl=new SliderDisplay(display_panel,1001,-3,3);
    beam_x_ctrl->set_value(0);
    
    display_sizer->Add(gl,wxSizerFlags(1).Expand());
    display_sizer->Add(beam_x_ctrl,wxSizerFlags().Expand());
    
    display_panel->SetSizer(display_sizer);
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    //
    
    timer=new wxTimer(this);
    Bind(wxEVT_TIMER,&SEM_2D_Frame::evt_timer,this);
    timer->Start(1000/60);
    
    last_watch=0;
    watch.Start();
}

SEM_2D_Frame::~SEM_2D_Frame()
{
    timer->Stop();
}

void SEM_2D_Frame::evt_profile(wxCommandEvent &event)
{
    double s=slope->get_value();
    
    if(s<0) slope->set_value(0);
    
    update_profile();
}

void SEM_2D_Frame::evt_timer(wxTimerEvent &event)
{
    scanning=scan_btn->GetValue();
    
    if(scanning)
    {
        int stime=scanning_time->get_value();
        
        double u=(watch.Time()%stime)/(stime+0.0);
        beam_x=-3.0+u*6.0;
    }
    else beam_x=beam_x_ctrl->get_value();
    
    update_electrons();
    
    if(gl->IsShown() && !gl->gl_ok)
    {
        update_profile();
        gl->init_opengl(profile_x,profile_z);
    }
    else
    {
        update_profile();
        gl->update_electrons(electrons);
        gl->render();
    }
}

double SEM_2D_Frame::profile(double x,double s)
{
    return std::tanh(s*x);
}

void SEM_2D_Frame::update_electrons()
{    
    double s=slope->get_value();
    double nominal_energy=e_energy->get_value();
    double mfp=e_mfp->get_value();
    double beam=beam_width->get_value();
    
    int Ne=N_electrons->get_value();
    int N_left=Ne/60;
    
    // Seeding new electrons
    
    while(N_left!=0)
    {
        int N=electrons.size();
        
        for(std::size_t i=0;i<electrons.size();i++)
        {
            if(electrons[i].show==false)
            {
                double x=randp(beam_x-beam,beam_x+beam);
                
                if(scanning) x+=randp(-0.1,0.1);
                
                electrons[i].initial_x=x;
                electrons[i].loc.x=x;
                electrons[i].loc.y=0;
                electrons[i].loc.z=2.0+randp(0.1);
                
                electrons[i].energy=nominal_energy;
                electrons[i].dir=Vector3(0,0,-1);
                
                electrons[i].beam=true;
                electrons[i].show=true;
                electrons[i].color=1.0;
                
                N_left--;
                if(N_left==0) break;
            }
        }
        
        if(N_left!=0)
        {
            electrons.resize(N+N_left);
            
            for(int i=N;i<N+N_left;i++)
            {
                electrons[i].beam=true;
                electrons[i].show=false;
            }
        }
    }
    
    // SE electrons
    
    double SE_proportion_value=SE_proportion->get_value();
    
    double z1=profile(beam_x,s);
    double z2=profile(beam_x+1e-6,s);
    
    Vector3 profile_tangent(1e-6,0,z2-z1);
    profile_tangent.normalize();
    
    Vector3 profile_normal=crossprod(profile_tangent,Vector3(0,1,0));
    profile_normal.normalize();
    
    // Propagating
    
    for(std::size_t i=0;i<electrons.size();i++)
    {
        if(electrons[i].show==true)
        {
            double x=electrons[i].loc.x;
            double z=electrons[i].loc.z;
            
            double disp=std::sqrt(electrons[i].energy)*0.01;
            
            if(std::abs(x)>4)
            {
                electrons[i].color=0;
                electrons[i].show=false;
            }
            else if(z>2.1)
            {
                electrons[i].color=0;
                electrons[i].show=false;
                
                int k;
                double u;
                vector_locate_linear(k,u,profile_x,electrons[i].initial_x);
                
                bins_buffer[k]++;
            }
            else if(z>profile(x,s))
            {
                if(electrons[i].beam)
                {
                    z=z-disp;
                    electrons[i].loc.z=z;
                }
                else electrons[i].loc+=disp*electrons[i].dir;
                    
                if(z<=profile(x,s))
                {
                    electrons[i].beam=false;
                    
                    if(randp()<SE_proportion_value)
                    {
                        double th=randp(Pi);
                        electrons[i].loc.z+=disp;
                        electrons[i].energy/=3.0;
                        electrons[i].dir=std::cos(th)*profile_tangent+std::sin(th)*profile_normal;
                        electrons[i].color=Vector3(1.0,0,0);
                    }
                    else electrons[i].last_loc=electrons[i].loc;
                }
            }
            else
            {
                disp/=2;
                
                electrons[i].loc+=disp*electrons[i].dir;
                
                double p=mfp*(1.0+randp(-0.2,0.2));
                
                if((electrons[i].last_loc-electrons[i].loc).norm_sqr()>=p*p)
                {
                    double th=randp(-Pi/2.0,Pi/2.0);
                    p=randp();
                    
                    while(p>std::pow(std::cos(th),4))
                    {
                        th=randp(-Pi/2.0,Pi/2.0);
                        p=randp();
                    }
                    
                    Vector3 u=electrons[i].dir;
                    Vector3 n=crossprod(u,Vector3(0,1,0));
                    
                    electrons[i].dir=std::cos(th)*u+std::sin(th)*n;
                    
                    electrons[i].last_loc=electrons[i].loc;
                    electrons[i].energy=electrons[i].energy*(1-randp(0.1))-1;
                    electrons[i].color=electrons[i].energy/nominal_energy;
                }
                
                if(electrons[i].energy<=0)
                {
                    electrons[i].loc.z=2;
                    electrons[i].color=0;
                    electrons[i].show=false;
                }
            }
        }
    }
}

void SEM_2D_Frame::update_profile()
{    
    double s=slope->get_value();
    
    for(int i=0;i<Nx;i++)
    {
        profile_x[i]=-3.0+6.0*i/(Nx-1.0);
        profile_z[i]=profile(profile_x[i],s);
    }
    
    long new_time=watch.Time();
    long itime=integration_time->get_value();
    
    if(new_time/itime>last_watch/itime)
    {
        bins=bins_buffer;
            
        int bin_max=bins[0];
        
        for(int i=0;i<Nx;i++)
        {
            bins_buffer[i]=0;
            bin_max=std::max(bin_max,bins[i]);
        }
        
        if(scanning)
        {
            for(int i=0;i<Nx;i++)
                profile_color[i]=bins[i]/(bin_max+0.0);
        }
        else
        {
            for(int i=0;i<Nx;i++) profile_color[i]=1.0;
        }
    }
    
    last_watch=new_time;
    
    gl->update_profile(profile_x,profile_z,profile_color);
}

