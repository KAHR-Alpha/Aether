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

#include <gui_diffract_orders.h>

#include <iomanip>
#include <limits>

extern const double Pi;
extern std::ofstream plog;

GL_DiffOrders::GL_DiffOrders(wxWindow *parent)
    :GL_3D_Base(parent),
     display_superstrate(true),
     display_substrate(true)
{
    
}

void GL_DiffOrders::init_opengl()
{
    prog_orders=Glite::create_program(PathManager::locate_resource("resources/glsl/diff_orders_rays_vshader.glsl"),
                                      PathManager::locate_resource("resources/glsl/diff_orders_rays_fshader.glsl"));
    
    GLuint v_buff,vn_buff,index_buff;
    glGenBuffers(1,&v_buff);
    glGenBuffers(1,&vn_buff);
    glGenBuffers(1,&index_buff);
    
    // Diffraction orders
    
    glGenVertexArrays(1,&vao_orders);
    
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
    
    glBindVertexArray(vao_orders);
    
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
    
    // Circles
    
    prog_circles=Glite::create_program(PathManager::locate_resource("resources/glsl/diff_orders_circ_vshader.glsl"),
                                       PathManager::locate_resource("resources/glsl/diff_orders_circ_fshader.glsl"));
    
    v_arr_buff=new GLfloat[4*2*64];
    
    for(i=0;i<64;i++)
    {
        v_arr_buff[4*(2*i+0)+0]=std::cos(2.0*Pi*i/64.0);
        v_arr_buff[4*(2*i+0)+1]=std::sin(2.0*Pi*i/64.0);
        v_arr_buff[4*(2*i+0)+2]=0;
        v_arr_buff[4*(2*i+0)+3]=1.0;
        
        v_arr_buff[4*(2*i+1)+0]=std::cos(2.0*Pi*(i+1.0)/64.0);
        v_arr_buff[4*(2*i+1)+1]=std::sin(2.0*Pi*(i+1.0)/64.0);
        v_arr_buff[4*(2*i+1)+2]=0;
        v_arr_buff[4*(2*i+1)+3]=1.0;
    }
    
    glGenBuffers(1,&v_buff);
    glBindBuffer(GL_ARRAY_BUFFER,v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*128*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glGenVertexArrays(1,&vao_circles);
    glBindVertexArray(vao_circles);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    delete[] v_arr_buff;
}

void GL_DiffOrders::set_display(bool display_superstrate_,bool display_substrate_)
{
    display_superstrate=display_superstrate_;
    display_substrate=display_substrate_;
}

void GL_DiffOrders::render()
{
    unsigned int l;
    
    // Circles
    
    glUseProgram(prog_circles);
    
    glBindVertexArray(vao_circles);
    glUniformMatrix4fv(2,1,GL_FALSE,camera.proj_gl);
    
    glVertexAttrib4f(1,R_sup,R_sup,1.0,1.0);
    glUniform4f(3,1.0,0.4,0.4,1.0);
    
    glDrawArrays(GL_LINES,0,128);
    
    glVertexAttrib4f(1,R_sub,R_sub,1.0,1.0);
    glUniform4f(3,0.256,0.376,1.0,1.0);
    
    glDrawArrays(GL_LINES,0,128);
    
    // Orders
    
    glUseProgram(prog_orders);
    
    glUniformMatrix4fv(10,1,GL_FALSE,camera.proj_gl);
    Vector3 sun_dir(1.0,1.0,1.0);
    sun_dir.set_spherical(1.0,Pi/4.0,Pi/3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    glUniform4f(11,sun_dir.x,
                   sun_dir.y,
                   sun_dir.z,
                   0);
    
    glBindVertexArray(vao_orders);
    
    // Beta
    
    glVertexAttrib4f(2,0.4,1.0,0.4,0);
    
    glVertexAttrib4f(3,beta_u.x,beta_u.y,beta_u.z,0);
    glVertexAttrib4f(4,beta_v.x,beta_v.y,beta_v.z,0);
    glVertexAttrib4f(5,beta_w.x,beta_w.y,beta_w.z,0);
    glVertexAttrib4f(6,0,0,0,1.0);
    
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    
    // Superstrate
    
    glVertexAttrib4f(2,1.0,0.4,0.4,1.0);
    
    if(display_superstrate) for(l=0;l<u_sup.size();l++)
    {
        glVertexAttrib4f(3,u_sup[l].x,u_sup[l].y,u_sup[l].z,0);
        glVertexAttrib4f(4,v_sup[l].x,v_sup[l].y,v_sup[l].z,0);
        glVertexAttrib4f(5,w_sup[l].x,w_sup[l].y,w_sup[l].z,0);
        glVertexAttrib4f(6,0,0,0,1.0);
        
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    }
    
    // Substrate
    
    glVertexAttrib4f(2,0.256,0.376,1.0,0);
    
    if(display_substrate) for(l=0;l<u_sub.size();l++)
    {
        glVertexAttrib4f(3,u_sub[l].x,u_sub[l].y,u_sub[l].z,0);
        glVertexAttrib4f(4,v_sub[l].x,v_sub[l].y,v_sub[l].z,0);
        glVertexAttrib4f(5,w_sub[l].x,w_sub[l].y,w_sub[l].z,0);
        glVertexAttrib4f(6,0,0,0,1.0);
        
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    }
}

void GL_DiffOrders::set_beta(Vector3 const &beta,double k0)
{
    beta_w=-1.1*beta/k0;
    
    double w=0.01;
    
    if(beta_w.x==0 && beta_w.y==0)
    {
        beta_u(w,0,0);
        beta_v(0,w,0);
    }
    else
    {
        beta_u.crossprod(beta_w,Vector3(0,0,1));
        beta_u.normalize();
        beta_u*=w;
        
        beta_v.crossprod(beta_w,beta_u);
        beta_v.normalize();
        beta_v*=w;
    }
}

void GL_DiffOrders::set_R(double R_sup_,double R_sub_)
{
    R_sup=R_sup_;
    R_sub=R_sub_;
}

void GL_DiffOrders::update_orders(std::vector<double> const &kx_sup,
                                  std::vector<double> const &ky_sup,
                                  std::vector<double> const &kz_sup,
                                  std::vector<double> const &kx_sub,
                                  std::vector<double> const &ky_sub,
                                  std::vector<double> const &kz_sub)
{
    unsigned int l;
    unsigned int N=kx_sup.size();
    
    u_sup.resize(N);
    v_sup.resize(N);
    w_sup.resize(N);
    
    double w=0.025;
    
    for(l=0;l<N;l++)
    {
        w_sup[l](kx_sup[l],ky_sup[l],kz_sup[l]);
        
        if(w_sup[l].x==0 && w_sup[l].y==0)
        {
            u_sup[l](w,0,0);
            v_sup[l](0,w,0);
        }
        else
        {
            u_sup[l].crossprod(w_sup[l],Vector3(0,0,1));
            u_sup[l].normalize();
            u_sup[l]*=w;
            
            v_sup[l].crossprod(w_sup[l],u_sup[l]);
            v_sup[l].normalize();
            v_sup[l]*=w;
        }
    }
    
    N=kx_sub.size();
    
    u_sub.resize(N);
    v_sub.resize(N);
    w_sub.resize(N);
    
    for(l=0;l<N;l++)
    {
        w_sub[l](kx_sub[l],ky_sub[l],-kz_sub[l]);
        
        if(w_sub[l].x==0 && w_sub[l].y==0)
        {
            u_sub[l](w,0,0);
            v_sub[l](0,w,0);
        }
        else
        {
            u_sub[l].crossprod(w_sub[l],Vector3(0,0,1));
            u_sub[l].normalize();
            u_sub[l]*=w;
            
            v_sub[l].crossprod(w_sub[l],u_sub[l]);
            v_sub[l].normalize();
            v_sub[l]*=w;
        }
    }
}

//#######################
//   DiffOrdersSDFrame
//#######################

DiffOrdersSDFrame::DiffOrdersSDFrame(wxString const &title)
    :DiffOrdersSDFrame(title,400e-9,800e-9,Vector3(1000e-9,0,0),Vector3(0,1000e-9,0))
{
}

DiffOrdersSDFrame::DiffOrdersSDFrame(wxString const &title,
                                     double lambda_min,double lambda_max,
                                     Vector3 const &a1_,Vector3 const &a2_)
    :BaseFrame(title),
     a1(a1_), a2(a2_)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    wxScrolledWindow *ctrl_panel=new wxScrolledWindow(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    spectrum=new SpectrumSelector(ctrl_panel,lambda_min,lambda_max,401,true);
    
    ctrl_sizer->Add(spectrum,std_flags);
    
    superstrate_selector=new MiniMaterialSelector(ctrl_panel,"Superstrate");
    substrate_selector=new MiniMaterialSelector(ctrl_panel,"Substrate");
    
    ctrl_sizer->Add(superstrate_selector,std_flags);
    ctrl_sizer->Add(substrate_selector,std_flags);
    
    wxStaticBoxSizer *ang_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Max Incidence");
    
    max_incidence_ctrl=new NamedTextCtrl<double>(ctrl_panel,"",90);
    
    ang_sizer->Add(max_incidence_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(ang_sizer,std_flags);
    
    wxStaticBoxSizer *a1_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Base Grating Vector 1");
    wxStaticBoxSizer *a2_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Base Grating Vector 2");
    
    a1_length_ctrl=new LengthSelector(ctrl_panel,"Length: ",a1.norm());
    a1_ang_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Angle: ",180.0/Pi*std::atan2(a1.y,a1.x));
    
    a1_sizer->Add(a1_length_ctrl,wxSizerFlags().Expand());
    a1_sizer->Add(a1_ang_ctrl,wxSizerFlags().Expand());
    
    a2_length_ctrl=new LengthSelector(ctrl_panel,"Length: ",a2.norm());
    a2_ang_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Angle: ",180.0/Pi*std::atan2(a2.y,a2.x));
    
    a2_sizer->Add(a2_length_ctrl,wxSizerFlags().Expand());
    a2_sizer->Add(a2_ang_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(a1_sizer,std_flags);
    ctrl_sizer->Add(a2_sizer,std_flags);
    
    wxStaticBoxSizer *min_a_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Subdiffractive Limit");
    
    min_a1=new NamedTextCtrl<std::string>(ctrl_panel,"a1: ",add_unit_u(a1.norm()));
    min_a2=new NamedTextCtrl<std::string>(ctrl_panel,"a2: ",add_unit_u(a2.norm()));
    
    min_a1->lock();
    min_a2->lock();
    
    min_a_sizer->Add(min_a1,wxSizerFlags().Expand());
    min_a_sizer->Add(min_a2,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(min_a_sizer,std_flags);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    graph=new Graph(display_panel);
    
    display_sizer->Add(graph,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    // Bindings
    
    a1_length_ctrl->Bind(EVT_LENGTH_SELECTOR,&DiffOrdersSDFrame::evt_vectors,this);
    a2_length_ctrl->Bind(EVT_LENGTH_SELECTOR,&DiffOrdersSDFrame::evt_vectors,this);
    a1_ang_ctrl->Bind(EVT_NAMEDTXTCTRL,&DiffOrdersSDFrame::evt_vectors,this);
    a2_ang_ctrl->Bind(EVT_NAMEDTXTCTRL,&DiffOrdersSDFrame::evt_vectors,this);
    
    max_incidence_ctrl->Bind(EVT_NAMEDTXTCTRL,&DiffOrdersSDFrame::evt_incidence,this);

    spectrum->Bind(EVT_SPECTRUM_SELECTOR,&DiffOrdersSDFrame::evt_spectrum,this);
    
    superstrate_selector->Bind(EVT_MINIMAT_SELECTOR,&DiffOrdersSDFrame::evt_material,this);
    substrate_selector->Bind(EVT_MINIMAT_SELECTOR,&DiffOrdersSDFrame::evt_material,this);
    
    // Wrapping up
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
    
    Show(true);
    Maximize(true);
    
    update_vectors();
    recompute();
}

void DiffOrdersSDFrame::evt_incidence(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void DiffOrdersSDFrame::evt_material(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void DiffOrdersSDFrame::evt_spectrum(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void DiffOrdersSDFrame::evt_vectors(wxCommandEvent &event)
{
    Angle ang;
    double length;
    
    ang=Degree(a1_ang_ctrl->get_value());
    length=a1_length_ctrl->get_length();
    a1(length*std::cos(ang),length*std::sin(ang),0);
    
    ang=Degree(a2_ang_ctrl->get_value());
    length=a2_length_ctrl->get_length();
    a2(length*std::cos(ang),length*std::sin(ang),0);
    
    update_vectors();
    recompute();
    
    event.Skip();
}

void DiffOrdersSDFrame::recompute()
{
    int Nl=spectrum->get_Np();
    
    spectrum->get_spectrum(lambda);
    s_factor_1_sup.resize(Nl);
    s_factor_1_sub.resize(Nl);
    s_factor_2_sup.resize(Nl);
    s_factor_2_sub.resize(Nl);
    
    double w,s_sup,s_sub;
    double a1_norm=a1.norm();
    double a2_norm=a2.norm();
    double Gnorm=Gn.norm();
    double Sang=std::sin(Pi/180.0*max_incidence_ctrl->get_value());
    
    Imdouble n1,n2;
    
    double val_min_a1=std::numeric_limits<double>::infinity();
    double val_min_a2=std::numeric_limits<double>::infinity();
    
    for(int l=0;l<Nl;l++)
    {
        w=m_to_rad_Hz(lambda[l]);
        
        n1=superstrate_selector->get_n(w);
        n2=substrate_selector->get_n(w);
        
        s_sup=lambda[l]*Gnorm/(2.0*Pi*(n1.real()+n1.real()*Sang));
        s_sub=lambda[l]*Gnorm/(2.0*Pi*(n2.real()+n1.real()*Sang));
        
        s_factor_1_sup[l]=a1_norm*s_sup;
        s_factor_1_sub[l]=a1_norm*s_sub;
        
        s_factor_2_sup[l]=a2_norm*s_sup;
        s_factor_2_sub[l]=a2_norm*s_sub;
        
        val_min_a1=var_min(val_min_a1,s_factor_1_sup[l],s_factor_1_sub[l]);
        val_min_a2=var_min(val_min_a2,s_factor_2_sup[l],s_factor_2_sub[l]);
    }
    
    graph->clear_graph();
    
    graph->add_data(&lambda,&s_factor_1_sup,1.0,0,0,"Superstrate a1");
    graph->add_data(&lambda,&s_factor_2_sup,1.0,0.5,0.5,"Superstrate a2");
    
    graph->add_data(&lambda,&s_factor_1_sub,0,0,1.0,"Substrate a1");
    graph->add_data(&lambda,&s_factor_2_sub,0.5,0.5,1.0,"Substrate a2");
    
    graph->autoscale();
    
    min_a1->set_value(add_unit_u(val_min_a1));
    min_a2->set_value(add_unit_u(val_min_a2));
}

void DiffOrdersSDFrame::set_materials(MiniMaterialSelector *superstrate_,
                                      MiniMaterialSelector *substrate_)
{
    superstrate_selector->copy_material(superstrate_);
    substrate_selector->copy_material(substrate_);
    
    recompute();
}

void DiffOrdersSDFrame::update_vectors()
{
    
    Vector3 z(0,0,1);
    
    double V=scalar_prod(z,crossprod(a1,a2));
    
    Vector3 p1=crossprod(a2,z)/V;
    Vector3 p2=crossprod(z,a1)/V;
    
    b1=2*Pi*p1;
    b2=2*Pi*p2;
    
    Vector3 Vtmp;
    double tmp,min_G=b1.norm_sqr();
    
    Gn=b1;
    
    int k=1,p,q;
    bool last_min=false;
    
    while(!last_min)
    {
        last_min=true;
        
        p=-k;
        
        for(q=-k;q<=k;q++)
        {
            Vtmp=p*b1+q*b2; tmp=Vtmp.norm_sqr();
            if(tmp<min_G) { last_min=false; min_G=tmp; Gn=Vtmp; }
        }
        
        p=+k;
        
        for(q=-k;q<=k;q++)
        {
            Vtmp=p*b1+q*b2; tmp=Vtmp.norm_sqr();
            if(tmp<min_G) { last_min=false; min_G=tmp; Gn=Vtmp; }
        }
        
        q=-k;
        
        for(p=-k;p<=k;p++)
        {
            Vtmp=p*b1+q*b2; tmp=Vtmp.norm_sqr();
            if(tmp<min_G) { last_min=false; min_G=tmp; Gn=Vtmp; }
        }
        
        q=+k;
        
        for(p=-k;p<=k;p++)
        {
            Vtmp=p*b1+q*b2; tmp=Vtmp.norm_sqr();
            if(tmp<min_G) { last_min=false; min_G=tmp; Gn=Vtmp; }
        }
        
        k++;
    }
}

//#####################
//   DiffOrdersFrame
//#####################

DiffOrdersFrame::DiffOrdersFrame(wxString const &title)
    :BaseFrame(title),
     normalized(true), mode_multi(false),
     Nl(481),
     lambda(500e-9),
     lambda_min(370e-9), lambda_max(850e-9),
     c_factor(1.0),
     beta(0,0,-2.0*Pi/lambda),
     a1(1000e-9,0,0),
     a2(0,1000e-9,0),
     N_ord_lambda(Nl),
     N_ord_sup(Nl), N_ord_sub(Nl)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *beta_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Incidence");
    wxStaticBoxSizer *a1_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Grating Vector 1");
    wxStaticBoxSizer *a2_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Grating Vector 2");
    
    wxString mode_str[]={"Monochromatic","Polychromatic"};
    wxRadioBox *mode_ctrl=new wxRadioBox(ctrl_panel,wxID_ANY,"Mode",wxDefaultPosition,wxDefaultSize,2,mode_str);
    
    lambda_ctrl=new WavelengthSelector(ctrl_panel,"Lambda",500e-9);
    spectrum_ctrl=new SpectrumSelector(ctrl_panel,lambda_min,lambda_max,Nl,true);
    
    wxStaticBoxSizer *superstrate_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Superstrate");
    wxStaticBoxSizer *substrate_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Substrate");
    
    superstrate_selector=new MiniMaterialSelector(ctrl_panel);
    substrate_selector=new MiniMaterialSelector(ctrl_panel);
    
    superstrate_sizer->Add(superstrate_selector,wxSizerFlags().Expand());
    substrate_sizer->Add(substrate_selector,wxSizerFlags().Expand());
    
    // Beta
    
    wxStaticBoxSizer *theta_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Theta");
    wxStaticBoxSizer *phi_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Phi");
    
    theta_ctrl=new SliderDisplay(ctrl_panel,901,0,90,1.0,"°",true);
    phi_ctrl=new SliderDisplay(ctrl_panel,3601,-180,180,1.0,"°",true);
    phi_ctrl->set_value(0);
    
    theta_sizer->Add(theta_ctrl,wxSizerFlags().Expand());
    phi_sizer->Add(phi_ctrl,wxSizerFlags().Expand());
    
    beta_sizer->Add(theta_sizer,wxSizerFlags().Expand());
    beta_sizer->Add(phi_sizer,wxSizerFlags().Expand());
    
    a1_length_ctrl=new LengthSelector(ctrl_panel,"Length: ",1000e-9);
    a1_ang_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Angle: ",0);
    
    a1_sizer->Add(a1_length_ctrl,wxSizerFlags().Expand());
    a1_sizer->Add(a1_ang_ctrl,wxSizerFlags().Expand());
    
    a2_length_ctrl=new LengthSelector(ctrl_panel,"Length: ",1000e-9);
    a2_ang_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Angle: ",90);
    
    a2_sizer->Add(a2_length_ctrl,wxSizerFlags().Expand());
    a2_sizer->Add(a2_ang_ctrl,wxSizerFlags().Expand());
    
    // Display Controls
    
    wxStaticBoxSizer *display_ctrl_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Display");
    
    normalized_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Normalized");
    display_superstrate_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Superstrate");
    display_substrate_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Substrate");
    
    normalized_ctrl->SetValue(true);
    display_superstrate_ctrl->SetValue(true);
    display_substrate_ctrl->SetValue(true);
    
    display_ctrl_sizer->Add(normalized_ctrl,wxSizerFlags().Expand());
    display_ctrl_sizer->Add(display_superstrate_ctrl,wxSizerFlags().Expand());
    display_ctrl_sizer->Add(display_substrate_ctrl,wxSizerFlags().Expand());
    
    wxButton *subdiff_btn=new wxButton(ctrl_panel,wxID_ANY,"Subdiffractive Limit");
    surface_modes_btn=new wxButton(ctrl_panel,wxID_ANY,"Surface Modes");
    
    ctrl_sizer->Add(mode_ctrl,std_flags);
    ctrl_sizer->Add(lambda_ctrl,std_flags);
    ctrl_sizer->Add(spectrum_ctrl,std_flags);
    ctrl_sizer->Add(superstrate_sizer,std_flags);
    ctrl_sizer->Add(substrate_sizer,std_flags);
    ctrl_sizer->Add(beta_sizer,std_flags);
    ctrl_sizer->Add(a1_sizer,std_flags);
    ctrl_sizer->Add(a2_sizer,std_flags);
    ctrl_sizer->Add(display_ctrl_sizer,std_flags);
    ctrl_sizer->Add(subdiff_btn,std_flags);
    ctrl_sizer->Add(surface_modes_btn,std_flags);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    gl=new GL_DiffOrders(display_panel);
    orders_graph=new Graph(display_panel);
    
    orders_graph->add_data(&N_ord_lambda,&N_ord_sup,1.0,0,0,"Superstrate");
    orders_graph->add_data(&N_ord_lambda,&N_ord_sub,0,0,1.0,"Substrate");
    
    display_sizer->Add(gl,wxSizerFlags(1).Expand());
    display_sizer->Add(orders_graph,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    // Bindings
    
    a1_length_ctrl->Bind(EVT_LENGTH_SELECTOR,&DiffOrdersFrame::evt_a1,this);
    a1_ang_ctrl->Bind(EVT_NAMEDTXTCTRL,&DiffOrdersFrame::evt_a1,this);
    
    a2_length_ctrl->Bind(EVT_LENGTH_SELECTOR,&DiffOrdersFrame::evt_a2,this);
    a2_ang_ctrl->Bind(EVT_NAMEDTXTCTRL,&DiffOrdersFrame::evt_a2,this);
    
    display_superstrate_ctrl->Bind(wxEVT_CHECKBOX,&DiffOrdersFrame::evt_display_switch,this);
    display_substrate_ctrl->Bind(wxEVT_CHECKBOX,&DiffOrdersFrame::evt_display_switch,this);
    
    mode_ctrl->Bind(wxEVT_RADIOBOX,&DiffOrdersFrame::evt_mode_switch,this);
    normalized_ctrl->Bind(wxEVT_CHECKBOX,&DiffOrdersFrame::evt_normalize,this);
    
    lambda_ctrl->Bind(EVT_WAVELENGTH_SELECTOR,&DiffOrdersFrame::evt_lambda,this);
    spectrum_ctrl->Bind(EVT_SPECTRUM_SELECTOR,&DiffOrdersFrame::evt_spectrum,this);
    
    phi_ctrl->Bind(EVT_SLIDERDISPLAY,&DiffOrdersFrame::evt_angle,this);
    theta_ctrl->Bind(EVT_SLIDERDISPLAY,&DiffOrdersFrame::evt_angle,this);
    
    substrate_selector->Bind(EVT_MINIMAT_SELECTOR,&DiffOrdersFrame::evt_materials,this);
    superstrate_selector->Bind(EVT_MINIMAT_SELECTOR,&DiffOrdersFrame::evt_materials,this);
    
    subdiff_btn->Bind(wxEVT_BUTTON,&DiffOrdersFrame::evt_subdiff,this);
    surface_modes_btn->Bind(wxEVT_BUTTON,&DiffOrdersFrame::evt_surface_modes,this);
    
    //
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
    
    layout_mono();
    
    update_vectors();
    recompute();
    
    double k0=2.0*Pi/lambda;
    gl->set_beta(beta,k0);
}

void DiffOrdersFrame::evt_a1(wxCommandEvent &event)
{
    Angle ang=Degree(a1_ang_ctrl->get_value());
    double length=a1_length_ctrl->get_length();
    
    a1(length*std::cos(ang),length*std::sin(ang),0);
    
    update_vectors();
    recompute();
}

void DiffOrdersFrame::evt_a2(wxCommandEvent &event)
{
    Angle ang=Degree(a2_ang_ctrl->get_value());
    double length=a2_length_ctrl->get_length();
    
    a2(length*std::cos(ang),length*std::sin(ang),0);
    
    update_vectors();
    recompute();
}

void DiffOrdersFrame::evt_angle(wxCommandEvent &event)
{
    phi.degree(phi_ctrl->get_value());
    theta.degree(theta_ctrl->get_value());
    
    recompute();
}

void DiffOrdersFrame::evt_display_switch(wxCommandEvent &event)
{
    gl->set_display(display_superstrate_ctrl->GetValue(),
                    display_substrate_ctrl->GetValue());
}

void DiffOrdersFrame::evt_lambda(wxCommandEvent &event)
{
    lambda=lambda_ctrl->get_lambda();
    recompute();
}

void DiffOrdersFrame::evt_materials(wxCommandEvent &event)
{
    recompute();
}

void DiffOrdersFrame::evt_mode_switch(wxCommandEvent &event)
{
    mode_multi=event.GetInt();
    
    recompute();
    
    if(mode_multi) layout_multi();
    else layout_mono();
}

void DiffOrdersFrame::evt_normalize(wxCommandEvent &event)
{
    normalized=normalized_ctrl->GetValue();
    recompute();
}

void DiffOrdersFrame::evt_spectrum(wxCommandEvent &event)
{
    Nl=spectrum_ctrl->get_Np();
    lambda_min=spectrum_ctrl->get_lambda_min();
    lambda_max=spectrum_ctrl->get_lambda_max();
    
    N_ord_lambda.resize(Nl);
    N_ord_sup.resize(Nl);
    N_ord_sub.resize(Nl);
    
    recompute();
}

void DiffOrdersFrame::evt_subdiff(wxCommandEvent &event)
{
    DiffOrdersSDFrame *sd_frame=new DiffOrdersSDFrame("Subdiffractive Limit",
                                                      lambda,2*lambda,a1,a2);
    sd_frame->set_materials(superstrate_selector,substrate_selector);
    
    event.Skip();
}

//###########################
//   OptFibersReportDialog
//###########################

class SurfaceModesDialog: public wxDialog
{
    public:
        
        SurfaceModesDialog(wxString const &report)
            :wxDialog(NULL,wxID_ANY,"Surface Modes",
                      wxGetApp().default_dialog_origin(),
                      wxGetApp().default_dialog_size())
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
            
            wxTextCtrl *text=new wxTextCtrl(this,wxID_ANY,report,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
            
            sizer->Add(text,wxSizerFlags(1).Expand());
            
            SetSizer(sizer);
            
            ShowModal();
        }
};

void DiffOrdersFrame::evt_surface_modes(wxCommandEvent &event)
{
    unsigned int i;
    std::stringstream superstrate_report,substrate_report;
    
    superstrate_report<<"Superstrate surface modes apparition:"<<std::endl<<std::endl;
    
    for(i=0;i<N_ord_lambda.size()-1;i++)
    {
        if(N_ord_sup[i]!=N_ord_sup[i+1])
            superstrate_report<<0.5*(N_ord_lambda[i]+N_ord_lambda[i+1])<<std::endl;
    }
    
    substrate_report<<std::endl<<"Substrate surface modes apparition:"<<std::endl<<std::endl;
    
    for(i=0;i<N_ord_lambda.size()-1;i++)
    {
        if(N_ord_sub[i]!=N_ord_sub[i+1])
            substrate_report<<0.5*(N_ord_lambda[i]+N_ord_lambda[i+1])<<std::endl;
    }
    
    wxString report;
    report<<superstrate_report.str()<<substrate_report.str();
    
    int sx,sy;
    GetSize(&sx,&sy);
    
    SurfaceModesDialog dialog(report);
    
}

void DiffOrdersFrame::layout_mono()
{
    lambda_ctrl->Show();
    spectrum_ctrl->Hide();
    surface_modes_btn->Hide();
    
    gl->Show();
    orders_graph->Hide();
    
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
    
    display_panel->Layout();
}

void DiffOrdersFrame::layout_multi()
{
    lambda_ctrl->Hide();
    spectrum_ctrl->Show();
    surface_modes_btn->Show();
    
    gl->Hide();
    orders_graph->Show();
    
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
    
    display_panel->Layout();
}

void DiffOrdersFrame::recompute()
{
    int l,p,q;
    int p_min=0,p_max=0;
    int q_min=0,q_max=0;
    
    double p1n=p1.norm_sqr();
    double p2n=p2.norm_sqr();
    double p12=scalar_prod(p1,p2);
    
    if(mode_multi)
    {
        for(int i=0;i<Nl;i++)
        {
            N_ord_lambda[i]=lambda_min+i/(Nl-1.0)*(lambda_max-lambda_min);
            
            double k0=2.0*Pi/N_ord_lambda[i];
            double w=2.0*Pi*c_light/N_ord_lambda[i];
            
            double index_sup=std::real(superstrate_selector->get_n(w));
            double index_sub=std::real(substrate_selector->get_n(w));
            
            beta(std::cos(phi)*std::sin(theta),
                 std::sin(phi)*std::sin(theta),
                 std::cos(theta));
            
            beta*=-k0*std::real(superstrate_selector->get_n(w));
            
            double a1b=scalar_prod(a1,beta);
            double a2b=scalar_prod(a2,beta);
            
            double sr1=std::sqrt(p2n/(p1n*p2n-p12));
            double sr2=std::sqrt(p1n/(p1n*p2n-p12));
            
            Vector3 kp;
            
            // Superstrate
            
            double kn_sup=k0*index_sup;
            
            double R=c_factor*kn_sup;
            double R2=R*R;
            
            N_ord_sup[i]=0;
            
            p_min=nearest_integer((-R*sr1-a1b)/(2.0*Pi));
            p_max=nearest_integer((+R*sr1-a1b)/(2.0*Pi));
            
            q_min=nearest_integer((-R*sr2-a2b)/(2.0*Pi));
            q_max=nearest_integer((+R*sr2-a2b)/(2.0*Pi));
            
            for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
            {
                kp=p*b1+q*b2+beta;
                kp.z=0;
                
                if(kp.norm_sqr()<=R2*1.00001) N_ord_sup[i]++;
            }
            
            // Substrate
            
            double kn_sub=k0*index_sub;
            R=c_factor*kn_sub;
            R2=R*R;
            
            N_ord_sub[i]=0;
            
            p_min=nearest_integer((-R*sr1-a1b)/(2.0*Pi));
            p_max=nearest_integer((+R*sr1-a1b)/(2.0*Pi));
            
            q_min=nearest_integer((-R*sr2-a2b)/(2.0*Pi));
            q_max=nearest_integer((+R*sr2-a2b)/(2.0*Pi));
            
            for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
            {
                kp=p*b1+q*b2+beta;
                kp.z=0;
                
                if(kp.norm_sqr()<R2*1.00001) N_ord_sub[i]++;
            }
        }
        
        orders_graph->Refresh();
        orders_graph->autoscale();
    }
    else
    {
        double k0=2.0*Pi/lambda;
        double w=2.0*Pi*c_light/lambda;
        
        double index_sup=std::real(superstrate_selector->get_n(w));
        double index_sub=std::real(substrate_selector->get_n(w));
        
        beta(std::cos(phi)*std::sin(theta),
             std::sin(phi)*std::sin(theta),
             std::cos(theta));
        
        beta*=-k0*std::real(superstrate_selector->get_n(w));
        
        double a1b=scalar_prod(a1,beta);
        double a2b=scalar_prod(a2,beta);
        
        double sr1=std::sqrt(p2n/(p1n*p2n-p12));
        double sr2=std::sqrt(p1n/(p1n*p2n-p12));
        
        Vector3 kp;
        double kpn2;
        
        // Superstrate
        
        double kn_sup=k0*index_sup;
        double kn_sup2=kn_sup*kn_sup;
        
        double R=c_factor*kn_sup;
        double R2=R*R;
        
        int N_sup=0;
        
        p_min=nearest_integer((-R*sr1-a1b)/(2.0*Pi));
        p_max=nearest_integer((+R*sr1-a1b)/(2.0*Pi));
        
        q_min=nearest_integer((-R*sr2-a2b)/(2.0*Pi));
        q_max=nearest_integer((+R*sr2-a2b)/(2.0*Pi));
        
//        chk_var(p_min);
//        chk_var(p_max);
//        chk_var(q_min);
//        chk_var(q_max);
        
        for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
        {
            kp=p*b1+q*b2+beta;
            kp.z=0;
            
            if(kp.norm_sqr()<=R2*1.00001) N_sup++;
        }
        
        kx_sup.resize(N_sup); ky_sup.resize(N_sup); kz_sup.resize(N_sup);
        
        l=0;
        
        for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
        {
            kp=p*b1+q*b2+beta;
            kp.z=0;
            
            kpn2=kp.norm_sqr();
            
            if(kpn2<=R2*1.00001)
            {
                kx_sup[l]=kp.x;
                ky_sup[l]=kp.y;
                
                if(kpn2>=kn_sup2) kz_sup[l]=0;
                else kz_sup[l]=std::sqrt(kn_sup2-kpn2);
                
                l++;
            }
        }
        
        // Substrate
        
        double kn_sub=k0*index_sub;
        double kn_sub2=kn_sub*kn_sub;
        R=c_factor*kn_sub;
        R2=R*R;
        
        int N_sub=0;
        
        p_min=nearest_integer((-R*sr1-a1b)/(2.0*Pi));
        p_max=nearest_integer((+R*sr1-a1b)/(2.0*Pi));
        
        q_min=nearest_integer((-R*sr2-a2b)/(2.0*Pi));
        q_max=nearest_integer((+R*sr2-a2b)/(2.0*Pi));
        
        for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
        {
            kp=p*b1+q*b2+beta;
            kp.z=0;
            
            if(kp.norm_sqr()<R2*1.00001) N_sub++;
        }
        
        kx_sub.resize(N_sub); ky_sub.resize(N_sub); kz_sub.resize(N_sub);
        
        l=0;
        
        for(p=p_min;p<=p_max;p++) for(q=q_min;q<=q_max;q++)
        {
            kp=p*b1+q*b2+beta;
            kp.z=0;
            
            kpn2=kp.norm_sqr();
            
            if(kpn2<R2*1.00001)
            {
                kx_sub[l]=kp.x;
                ky_sub[l]=kp.y;
                
                if(kpn2>=kn_sub2) kz_sub[l]=0;
                else kz_sub[l]=std::sqrt(kn_sub2-kpn2);
                
                l++;
            }
        }
        
        // Normalization
        
        if(normalized)
        {
            for(unsigned int i=0;i<kx_sup.size();i++)
            {
                kx_sup[i]/=k0*index_sup;
                ky_sup[i]/=k0*index_sup;
                kz_sup[i]/=k0*index_sup;
            }
            
            for(unsigned int i=0;i<kx_sub.size();i++)
            {
                kx_sub[i]/=k0*index_sub;
                ky_sub[i]/=k0*index_sub;
                kz_sub[i]/=k0*index_sub;
            }
        }
        else
        {
            for(unsigned int i=0;i<kx_sup.size();i++)
            {
                kx_sup[i]/=k0;
                ky_sup[i]/=k0;
                kz_sup[i]/=k0;
            }
            
            for(unsigned int i=0;i<kx_sub.size();i++)
            {
                kx_sub[i]/=k0;
                ky_sub[i]/=k0;
                kz_sub[i]/=k0;
            }
        }
        
        gl->update_orders(kx_sup,ky_sup,kz_sup,
                          kx_sub,ky_sub,kz_sub);
        
        gl->set_beta(beta,k0);
        gl->set_R(index_sup,index_sub);
    }
}

void DiffOrdersFrame::update_vectors()
{
    Vector3 z(0,0,1);
    
    double V=scalar_prod(z,crossprod(a1,a2));
    
    p1=crossprod(a2,z)/V;
    p2=crossprod(z,a1)/V;
    
    b1=2*Pi*p1;
    b2=2*Pi*p2;
}
