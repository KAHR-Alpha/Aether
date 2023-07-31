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
     n_dielec(1.0),
     lambda_min(400e-9),
     lambda_max(800e-9),
     radius(50)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxPanel *ctrl_panel=new wxPanel(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Control Panel
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // - Header
    
//    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
//    
//    wxString disp_mode_choices[]={"Cross Sections","2D radiation pattern","3D radiation pattern","2D Field maps","3D Field maps"};
//    
//    wxStaticText *disp_mode_txt=new wxStaticText(ctrl_panel,wxID_ANY,"Display: ");
//    
//    disp_mode_ctrl=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_mode_choices);
//    disp_mode_ctrl->SetSelection(0);
//    
//    disp_mode_ctrl->Bind(wxEVT_CHOICE,&MieTool::switch_disp_mode,this);
//    
//    header_sizer->Add(disp_mode_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
//    header_sizer->Add(disp_mode_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
//    
//    ctrl_sizer->Add(header_sizer,wxSizerFlags().Expand());
    
    // - Spectrum
    
    sp_selector=new SpectrumSelector(ctrl_panel,400e-9,800e-9,401,true);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MieTool::evt_recompute,this);
    
    ctrl_sizer->Add(sp_selector,wxSizerFlags().Expand());
    
    // - Particle
    
    radius_ctrl=new LengthSelector(ctrl_panel,"Radius",50e-9,true,"nm");
    radius_ctrl->Bind(EVT_LENGTH_SELECTOR,&MieTool::evt_recompute,this);
    ctrl_sizer->Add(radius_ctrl,wxSizerFlags().Expand());
    
    n_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Refractive Index",n_dielec,true);
    n_ctrl->Bind(EVT_NAMEDTXTCTRL,&MieTool::evt_recompute,this);
    ctrl_sizer->Add(n_ctrl,wxSizerFlags().Expand());
    
    material=new MiniMaterialSelector(ctrl_panel,nullptr,"Material");
    material->Bind(EVT_MINIMAT_SELECTOR,&MieTool::evt_recompute,this);
    ctrl_sizer->Add(material,wxSizerFlags().Expand());
    
    // - Calculation
    
    recursion=new NamedTextCtrl<int>(ctrl_panel,"Recursion",Nr,true);
    recursion->Bind(EVT_NAMEDTXTCTRL,&MieTool::evt_recompute,this);
    ctrl_sizer->Add(recursion,wxSizerFlags().Expand());
    
    wxString switch_str[]={"Cross Sections","Efficiencies"};
    
    cs_disp_switch=new wxRadioBox(ctrl_panel,wxID_ANY,"Display",wxDefaultPosition,wxDefaultSize,2,switch_str);
    cs_disp_switch->Bind(wxEVT_RADIOBOX,&MieTool::evt_recompute,this);
    
    ctrl_sizer->Add(cs_disp_switch,wxSizerFlags().Expand());
    
    // Display
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    display_panel->SetSizer(display_sizer);
    
    cross_graph=new Graph(display_panel);
    
    display_sizer->Add(cross_graph,wxSizerFlags(1).Expand());
    
    // Wrapping up
    
    splitter->SplitVertically(ctrl_panel,display_panel,270);
    splitter->SetMinimumPaneSize(20);
    
    recomp_cross_sections();
    
//    this->Show(true);
//    
//    Maximize();
    
    //------------------------
    //    Cross Sections
    //------------------------
    
//    cs_panel=new wxPanel(this);
//    wxBoxSizer *cs_sizer=new wxBoxSizer(wxVERTICAL);
//    wxBoxSizer *cs_display_sizer=new wxBoxSizer(wxHORIZONTAL);
//    
//    cross_graph=new Graph(cs_panel);
    
    // Directional Radiation
    
//    dr_panel=new wxPanel(cs_panel);
//    
//    dr_angle_sizer=new wxStaticBoxSizer(wxHORIZONTAL,dr_panel,"Angle");
//    
//    dr_theta=new NamedTextCtrl<double>(dr_panel,"Theta: ",0);
//    dr_phi=new NamedTextCtrl<double>(dr_panel,"Phi: ",0);
//    
//    dr_theta->Bind(EVT_NAMEDTXTCTRL,&MieTool::update_directional_radiation_angle,this);
//    dr_phi->Bind(EVT_NAMEDTXTCTRL,&MieTool::update_directional_radiation_angle,this);
//    
//    dr_angle_sizer->Add(dr_theta,wxSizerFlags().Border(wxRIGHT,10).Expand());
//    dr_angle_sizer->Add(dr_phi,wxSizerFlags().Expand());
//    
//    dr_panel->SetSizer(dr_angle_sizer);
//    dr_panel->Hide();
//    
//    cs_display_sizer->Add(dr_panel,wxSizerFlags().Expand());
//        
//    cs_sizer->Add(cross_graph,wxSizerFlags(1).Expand());
//    cs_sizer->Add(cs_display_sizer,wxSizerFlags().Expand());
//    
//    cs_panel->SetSizer(cs_sizer);
//    frame_sizer->Add(cs_panel,wxSizerFlags(1).Expand());
    
    //----------------------
    //     2D Radiation
    //----------------------
    
//    r2_panel=new wxPanel(this);
//    wxBoxSizer *r2_sizer=new wxBoxSizer(wxHORIZONTAL);
//    
//    // Controls
//    
//    wxPanel *r2_ctrl=new wxPanel(r2_panel);
//    wxStaticBoxSizer *r2_ctrl_sizer=new wxStaticBoxSizer(wxVERTICAL,r2_ctrl,"Angle");
//    
//    WavelengthSelector *r2_wvl=new WavelengthSelector(r2_ctrl,"Lambda",500e-9);
//    
//    r2_ctrl_sizer->Add(r2_wvl);
//    
//    r2_ctrl->SetSizer(r2_ctrl_sizer);
    
    // Graph
    
//    r2_graph=new Graph(r2_panel);
//    
//    r2_sizer->Add(r2_ctrl,wxSizerFlags().Expand());
//    r2_sizer->Add(r2_graph,wxSizerFlags(1).Expand());
//    
//    r2_panel->SetSizer(r2_sizer);
//    frame_sizer->Add(r2_panel,wxSizerFlags(1).Expand());
//    
//    
//    SetSizer(frame_sizer);
//    
//    show_cs();
}

MieTool::~MieTool()
{
    std::cout<<"Destructing MieTool"<<std::endl;
}

void MieTool::evt_recompute(wxCommandEvent &event)
{
    recomp_cross_sections();
}

void MieTool::load_material(wxCommandEvent &event)
{
    if(cs_disp_switch->GetSelection()==2) recomp_directional_radiation();
    else recomp_cross_sections();
}

void MieTool::recomp_cross_sections()
{
    int l;
    
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Nl=sp_selector->get_Np();
    
    radius=radius_ctrl->get_length();
    n_dielec=n_ctrl->get_value();
    GUI::Material *mat=material->get_material();
    
    Nr=recursion->get_value();
    bool comp_efficiencies=cs_disp_switch->GetSelection()==1;
    
    lambda.resize(Nl);
    disp_lambda.resize(Nl);
    C_abs.resize(Nl);
    C_ext.resize(Nl);
    C_scatt.resize(Nl);
    
    Mie mie;
    mie.set_index_ext(n_dielec);
    mie.set_NRec(Nr);
    mie.set_radius(radius);
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        disp_lambda[l]=m_to_nm(lambda[l]);
        
        double k0=2.0*Pi/lambda[l];
        double w=k0*c_light;
        
        mie.set_index_in(std::sqrt(mat->get_eps(w)));
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
//    int l;
//    
//    lambda.resize(Nl);
//    disp_lambda.resize(Nl);
//    dir_rad.resize(Nl);
//    
//    double theta=dr_theta->get_value();
//    double phi=dr_phi->get_value();
//    
//    Mie mie;
//    mie.set_index_ext(n_dielec);
//    mie.set_NRec(Nr);
//    mie.set_radius(radius*1e-9);
//    
//    for(l=0;l<Nl;l++)
//    {
//        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
//        disp_lambda[l]=m_to_nm(lambda[l]);
//        
//        double k0=2.0*Pi/lambda[l];
//        double w=k0*c_light;
//        
//        mie.set_index_in(std::sqrt(mat_selector->get_material()->get_eps(w)));
//        mie.set_lambda(lambda[l]);
//        
//        dir_rad[l]=mie.get_radiation(theta,phi);
//    }
//    
//    cross_graph->clear_graph();
//    
//    cross_graph->add_external_data(&disp_lambda,&dir_rad,0,0,1.0,"Radiation");
//    
//    cross_graph->autoscale();
}

void MieTool::show_cs()
{
//    cs_panel->Show();
//    r2_panel->Hide();
//    r3_panel->Hide();
//    m2_panel->Hide();
//    m3_panel->Hide();
}

void MieTool::show_r2()
{
//    cs_panel->Hide();
//    r2_panel->Show();
//    r3_panel->Hide();
//    m2_panel->Hide();
//    m3_panel->Hide();
}

void MieTool::switch_disp_mode(wxCommandEvent &event)
{
    int n=disp_mode_ctrl->GetSelection();
    
         if(n==0) show_cs();
    else if(n==1) show_r2();
    
    Layout();
}

void MieTool::update_directional_radiation_angle(wxCommandEvent &event)
{
    recomp_directional_radiation();
}
