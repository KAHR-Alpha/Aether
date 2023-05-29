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

#include <gui_mie.h>

extern const double Pi;
extern const Imdouble Im;

//############

MieTool::MieTool(wxString const &title)
    :BaseFrame(title),
     Nl(401), Nr(25),
     comp_efficiencies(false),
     n_dielec(1.0),
     lambda_min(400e-9),
     lambda_max(800e-9),
     radius(50)
{
    wxBoxSizer *frame_sizer=new wxBoxSizer(wxVERTICAL);
    
    //----------------
    //     Header
    //----------------
    
    wxPanel *header_panel=new wxPanel(this);
    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxString disp_mode_choices[]={"Cross Sections","2D radiation pattern","3D radiation pattern","2D Field maps","3D Field maps"};
    
    wxStaticText *disp_mode_txt=new wxStaticText(header_panel,wxID_ANY,"Display: ");
    disp_mode_ctrl=new wxChoice(header_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_mode_choices);
    disp_mode_ctrl->SetSelection(0);
    disp_mode_ctrl->Bind(wxEVT_CHOICE,&MieTool::switch_disp_mode,this);
    
    header_sizer->Add(disp_mode_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    header_sizer->Add(disp_mode_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    header_panel->SetSizer(header_sizer);
    frame_sizer->Add(header_panel,wxSizerFlags().Expand());
    
    //------------------------
    //    Cross Sections
    //------------------------
    
    cs_panel=new wxPanel(this);
    wxBoxSizer *cs_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *cs_display_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    cross_graph=new Graph(cs_panel);
    
    // Spectrum
    
    sp_selector=new SpectrumSelector(cs_panel,400e-9,800e-9,401);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MieTool::update_spectrum,this);
    
    wxString switch_str[]={"Cross Sections","Efficiencies","Directonal Radiation"};
    
    cs_disp_switch=new wxRadioBox(cs_panel,wxID_ANY,"Display",wxDefaultPosition,wxDefaultSize,3,switch_str);
    cs_disp_switch->Bind(wxEVT_RADIOBOX,&MieTool::switch_disp_cs,this);
    
    cs_display_sizer->Add(sp_selector,wxSizerFlags().Expand());
    cs_display_sizer->Add(cs_disp_switch,wxSizerFlags().Expand());
    
    // Directional Radiation
    
    dr_panel=new wxPanel(cs_panel);
    
    dr_angle_sizer=new wxStaticBoxSizer(wxHORIZONTAL,dr_panel,"Angle");
    
    dr_theta=new NamedTextCtrl<double>(dr_panel,"Theta: ",0);
    dr_phi=new NamedTextCtrl<double>(dr_panel,"Phi: ",0);
    
    dr_theta->Bind(EVT_NAMEDTXTCTRL,&MieTool::update_directional_radiation_angle,this);
    dr_phi->Bind(EVT_NAMEDTXTCTRL,&MieTool::update_directional_radiation_angle,this);
    
    dr_angle_sizer->Add(dr_theta,wxSizerFlags().Border(wxRIGHT,10).Expand());
    dr_angle_sizer->Add(dr_phi,wxSizerFlags().Expand());
    
    dr_panel->SetSizer(dr_angle_sizer);
    dr_panel->Hide();
    
    cs_display_sizer->Add(dr_panel,wxSizerFlags().Expand());
        
    cs_sizer->Add(cross_graph,wxSizerFlags(1).Expand());
    cs_sizer->Add(cs_display_sizer,wxSizerFlags().Expand());
    
    cs_panel->SetSizer(cs_sizer);
    frame_sizer->Add(cs_panel,wxSizerFlags(1).Expand());
    
    //----------------------
    //     2D Radiation
    //----------------------
    
    r2_panel=new wxPanel(this);
    wxBoxSizer *r2_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // Controls
    
    wxPanel *r2_ctrl=new wxPanel(r2_panel);
    wxStaticBoxSizer *r2_ctrl_sizer=new wxStaticBoxSizer(wxVERTICAL,r2_ctrl,"Angle");
    
    WavelengthSelector *r2_wvl=new WavelengthSelector(r2_ctrl,"Lambda",500e-9);
    
    r2_ctrl_sizer->Add(r2_wvl);
    
    r2_ctrl->SetSizer(r2_ctrl_sizer);
    
    // Graph
    
    r2_graph=new Graph(r2_panel);
    
    r2_sizer->Add(r2_ctrl,wxSizerFlags().Expand());
    r2_sizer->Add(r2_graph,wxSizerFlags(1).Expand());
    
    r2_panel->SetSizer(r2_sizer);
    frame_sizer->Add(r2_panel,wxSizerFlags(1).Expand());
    
    //------------------
    //     Particle
    //------------------
    
    wxPanel *particle_panel=new wxPanel(this);
    wxBoxSizer  *particle_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // Material
    
    GUI::Material *tmp_material=nullptr;
    mat_selector=new MaterialSelector(particle_panel,"Particle Material",false,tmp_material);
    mat_selector->Bind(EVT_MAT_SELECTOR,&MieTool::load_material,this);
    
    // Environment sizer
    
    wxStaticBoxSizer *env_sizer=new wxStaticBoxSizer(wxHORIZONTAL,particle_panel,"Environment");
    
    wxStaticText *n_txt=new wxStaticText(particle_panel,wxID_ANY,"n: ");
    n_ctrl=new wxTextCtrl(particle_panel,wxID_ANY,"1.0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    n_ctrl->Bind(wxEVT_TEXT_ENTER,&MieTool::update_dielec,this);
    env_sizer->Add(n_txt);
    env_sizer->Add(n_ctrl,wxSizerFlags(1));
    
    // Radius sizer
    
    wxStaticBoxSizer *radius_sizer=new wxStaticBoxSizer(wxHORIZONTAL,particle_panel,"Radius (nm)");
    
    radius_ctrl=new wxTextCtrl(particle_panel,wxID_ANY,"50",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    radius_ctrl->Bind(wxEVT_TEXT_ENTER,&MieTool::update_radius,this);
    radius_sizer->Add(radius_ctrl,wxSizerFlags(1));
    
    // Recursion sizer
    
    wxStaticBoxSizer *rec_sizer=new wxStaticBoxSizer(wxHORIZONTAL,particle_panel,"Recursion");
    rec_ctrl=new wxTextCtrl(particle_panel,wxID_ANY,"25",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    rec_ctrl->Bind(wxEVT_TEXT_ENTER,&MieTool::update_spectrum,this);
    rec_sizer->Add(rec_ctrl,wxSizerFlags(1));
    
    
    particle_sizer->Add(mat_selector,wxSizerFlags(1));
    particle_sizer->Add(env_sizer,wxSizerFlags());
    particle_sizer->Add(radius_sizer,wxSizerFlags());
    particle_sizer->Add(rec_sizer,wxSizerFlags());
    
    particle_panel->SetSizer(particle_sizer);
    frame_sizer->Add(particle_panel,wxSizerFlags().Expand());
    
    //------------------------------
    
    SetSizer(frame_sizer);
    
    show_cs();
    recomp_cross_sections();
    
    this->Show(true);
    
    Maximize();
}

MieTool::~MieTool()
{
    std::cout<<"Destructing MieTool"<<std::endl;
}

void MieTool::load_material(wxCommandEvent &event)
{
    if(cs_disp_switch->GetSelection()==2) recomp_directional_radiation();
    else recomp_cross_sections();
}

void MieTool::recomp_cross_sections()
{
    int l;
    
    lambda.resize(Nl);
    disp_lambda.resize(Nl);
    C_abs.resize(Nl);
    C_ext.resize(Nl);
    C_scatt.resize(Nl);
    
    Mie mie;
    mie.set_index_ext(n_dielec);
    mie.set_NRec(Nr);
    mie.set_radius(radius*1e-9);
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        disp_lambda[l]=m_to_nm(lambda[l]);
        
        double k0=2.0*Pi/lambda[l];
        double w=k0*c_light;
        
        mie.set_index_in(std::sqrt(mat_selector->get_eps(w)));
        mie.set_lambda(lambda[l]);
                
        if(comp_efficiencies)
        {
            C_abs[l]=mie.get_Qabs();
            C_ext[l]=mie.get_Qext();
            C_scatt[l]=mie.get_Qscatt();
        }
        else
        {
            C_abs[l]=mie.get_Cabs();
            C_ext[l]=mie.get_Cext();
            C_scatt[l]=mie.get_Cscatt();
        }
    }
    
    cross_graph->clear_graph();
    
    if(comp_efficiencies)
    {
        cross_graph->add_external_data(&disp_lambda,&C_abs,0,0,1.0,"Absorption efficiency");
        cross_graph->add_external_data(&disp_lambda,&C_ext,0,1.0,0,"Extinction efficiency");
        cross_graph->add_external_data(&disp_lambda,&C_scatt,1.0,0,0,"Scattering efficiency");
    }
    else
    {
        cross_graph->add_external_data(&disp_lambda,&C_abs,0,0,1.0,"Absorption cross section");
        cross_graph->add_external_data(&disp_lambda,&C_ext,0,1.0,0,"Extinction cross section");
        cross_graph->add_external_data(&disp_lambda,&C_scatt,1.0,0,0,"Scattering cross section");
    }
    
    cross_graph->autoscale();
}

void MieTool::recomp_directional_radiation()
{
    int l;
    
    lambda.resize(Nl);
    disp_lambda.resize(Nl);
    dir_rad.resize(Nl);
    
    double theta=dr_theta->get_value();
    double phi=dr_phi->get_value();
    
    Mie mie;
    mie.set_index_ext(n_dielec);
    mie.set_NRec(Nr);
    mie.set_radius(radius*1e-9);
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        disp_lambda[l]=m_to_nm(lambda[l]);
        
        double k0=2.0*Pi/lambda[l];
        double w=k0*c_light;
        
        mie.set_index_in(std::sqrt(mat_selector->get_eps(w)));
        mie.set_lambda(lambda[l]);
        
        dir_rad[l]=mie.get_radiation(theta,phi);
    }
    
    cross_graph->clear_graph();
    
    cross_graph->add_external_data(&disp_lambda,&dir_rad,0,0,1.0,"Radiation");
    
    cross_graph->autoscale();
}

void MieTool::show_cs()
{
    cs_panel->Show();
    r2_panel->Hide();
//    r3_panel->Hide();
//    m2_panel->Hide();
//    m3_panel->Hide();
}

void MieTool::show_r2()
{
    cs_panel->Hide();
    r2_panel->Show();
//    r3_panel->Hide();
//    m2_panel->Hide();
//    m3_panel->Hide();
}

void MieTool::switch_disp_cs(wxCommandEvent &event)
{
    int i=cs_disp_switch->GetSelection();
    
         if(i==0)
    {
        comp_efficiencies=false;
        dr_panel->Hide();
        
        recomp_cross_sections();
    }
    else if(i==1)
    {
        comp_efficiencies=true;
        dr_panel->Hide();
        
        recomp_cross_sections();
    }
    else if(i==2)
    {
        dr_panel->Show();
        
        recomp_directional_radiation();
    }
    
    Layout();
}

void MieTool::switch_disp_mode(wxCommandEvent &event)
{
    int n=disp_mode_ctrl->GetSelection();
    
         if(n==0) show_cs();
    else if(n==1) show_r2();
    
    Layout();
}

void MieTool::update_dielec(wxCommandEvent &event)
{
    textctrl_to_value(n_ctrl,n_dielec);
    
    if(n_dielec<1.0)
    {
        n_dielec=1.0;
        n_ctrl->ChangeValue("1.0");
    }
    
    if(cs_disp_switch->GetSelection()==2) recomp_directional_radiation();
    else recomp_cross_sections();
}

void MieTool::update_directional_radiation_angle(wxCommandEvent &event)
{
    recomp_directional_radiation();
}

void MieTool::update_radius(wxCommandEvent &event)
{
    textctrl_to_value(radius_ctrl,radius);
    
    if(radius<0)
    {
        radius=0;
        radius_ctrl->ChangeValue("0");
    }
    
    if(cs_disp_switch->GetSelection()==2) recomp_directional_radiation();
    else recomp_cross_sections();
}

void MieTool::update_spectrum(wxCommandEvent &event)
{
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Nl=sp_selector->get_Np();
    
    textctrl_to_value(rec_ctrl,Nr);
    
         if(Nl<2) { Nl=2; sp_selector->set_Np(2); }
    else if(Nl>1000000) { Nl=1000000; sp_selector->set_Np(1000000); }
    
         if(Nr<2) { Nr=2; rec_ctrl->ChangeValue("2"); }
    else if(Nr>1000) { Nr=1000; rec_ctrl->ChangeValue("1000"); }
    
    if(cs_disp_switch->GetSelection()==2) recomp_directional_radiation();
    else recomp_cross_sections();
    
    event.Skip();
}
