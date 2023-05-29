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

#include <gui_ellifr.h>
#include <gui_spp.h>

extern const double Pi;
extern const Imdouble Im;

//############

SppFrame::SppFrame(wxString const &title)
    :BaseFrame(title),
     no_metal(true),
     Nl(401),
     n_dielec(1.0),
     eps_dielec(1.0),
     lambda_min(400e-9),
     lambda_max(800e-9),
     profile_dielec(1000),
     profile_dielec_z(1000),
     profile_metal(1000),
     profile_metal_z(1000)
{
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL),
               *field_sizer=new wxBoxSizer(wxHORIZONTAL),
               *graph_sizer=new wxBoxSizer(wxHORIZONTAL),
               *mats_sizer=new wxBoxSizer(wxHORIZONTAL),
               *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    disp_graph=new Graph(this);
    aux_graph=new Graph(this);
    profile_graph=new Graph(this);
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    // Spectrum
    
    sp_selector=new SpectrumSelector(ctrl_panel,400e-9,800e-9,401);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&SppFrame::update_spectrum,this);
    
    sld_profile=new SldCombi(ctrl_panel,"Profile",4000,4000,8000,0.1,true);
    sld_profile->Bind(wxEVT_SLIDER,&SppFrame::update_profile_event,this);
//    Bind(wxEVT_SLIDER,&SppFrame::update_profile_event,this);
    
    field_sizer->Add(sp_selector,wxSizerFlags().Expand());
    field_sizer->Add(sld_profile,wxSizerFlags(1).Expand());
    
    // Metal
    
    GUI::Material *tmp_mat=nullptr;
    
    mat_selector=new MaterialSelector(ctrl_panel,"Metal",false,tmp_mat);
    mat_selector->Bind(EVT_MAT_SELECTOR,&SppFrame::load_material,this);
    
    // Dielectric sizer
    
    wxStaticBoxSizer *dielec_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Dielectric");
    
    wxStaticText *n_txt=new wxStaticText(ctrl_panel,wxID_ANY,"n: ");
    n_ctrl=new wxTextCtrl(ctrl_panel,wxID_ANY,"1.0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    n_ctrl->Bind(wxEVT_TEXT_ENTER,&SppFrame::update_dielec,this);
    dielec_sizer->Add(n_txt);
    dielec_sizer->Add(n_ctrl,wxSizerFlags(1));
    
    mats_sizer->Add(mat_selector,wxSizerFlags(1));
    mats_sizer->Add(dielec_sizer,wxSizerFlags());
    
    //ctrl_sizer
    
    ctrl_sizer->Add(field_sizer,wxSizerFlags().Expand());
    ctrl_sizer->Add(mats_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    graph_sizer->Add(disp_graph,wxSizerFlags(2).Expand());
    graph_sizer->Add(aux_graph,wxSizerFlags(2).Expand());
    graph_sizer->Add(profile_graph,wxSizerFlags(1).Expand());
    
    top_sizer->Add(graph_sizer,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    SetSizer(top_sizer);
    
    this->Show(true);
    
    Maximize();
}

SppFrame::~SppFrame()
{
    std::cout<<"Destructing SppFrame"<<std::endl;
}

void SppFrame::load_material(wxCommandEvent &event)
{
    no_metal=false;
    
    recomp_spp();
}

void SppFrame::recomp_spp()
{
    if(no_metal) return;
    
    update_profile();
    
    unsigned int l;
    
    lambda.resize(Nl);
    disp_lambda.resize(Nl);
    kspp_re.resize(Nl);
    kspp_im.resize(Nl);
    kspp.resize(Nl);
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        disp_lambda[l]=m_to_nm(lambda[l]);
        
        double k0=2.0*Pi/lambda[l];
        double w=k0*c_light;
        
        kspp[l]=spp_get_kspp(w,eps_dielec,mat_selector->get_eps(w));
        kspp_re[l]=m_to_nm(2.0*Pi/kspp[l].real());
        kspp_im[l]=m_to_nm(1.0/kspp[l].imag());
    }
    
    disp_graph->clear_graph();
    aux_graph->clear_graph();
    disp_graph->add_external_data(&disp_lambda,&kspp_re,0,0,1.0,"2*Pi/Re(k_spp)");
    aux_graph->add_external_data(&disp_lambda,&kspp_im,1.0,0,0,"1/Im(k_spp)");
    disp_graph->autoscale();
    aux_graph->autoscale();
}

void SppFrame::update_dielec(wxCommandEvent &event)
{
    textctrl_to_value(n_ctrl,n_dielec);
    
    if(n_dielec<1.0)
    {
        n_dielec=1.0;
        n_ctrl->ChangeValue("1.0");
    }
    
    eps_dielec=n_dielec*n_dielec;
    
    recomp_spp();
}

void SppFrame::update_profile()
{
    if(no_metal) return;
    
    int l;
    
    double lambda_tmp=sld_profile->get_value()*1e-9;
        
    double k0=2.0*Pi/lambda_tmp;
    double w=k0*c_light;
    
    Imdouble eps_metal=mat_selector->get_eps(w);
    
    Imdouble kspp=spp_get_kspp(w,eps_dielec,eps_metal);
    
    Imdouble kz_dielec=std::sqrt(k0*k0*eps_dielec-kspp*kspp);
    Imdouble kz_metal=std::sqrt(k0*k0*eps_metal-kspp*kspp);
//    Imdouble kz21=k0*k0*eps_dielec-kspp*kspp;
//    Imdouble kz22=k0*k0*eps_metal-kspp*kspp;
    
    if(kz_dielec.imag()<0) kz_dielec=-kz_dielec;
    if(kz_metal.imag()>0) kz_metal=-kz_metal;
    
    double zmax=-std::log(1e-2)/kz_dielec.imag()*1e9;
    double zmin=-std::log(1e-2)/kz_metal.imag()*1e9;
    
    int Nprof=1000;
    double z;
    
    for(l=0;l<Nprof;l++)
    {
        z=zmax*l/(Nprof-1.0);
        
        profile_dielec_z[l]=z;
        profile_dielec[l]=std::abs(std::exp(1e-9*z*kz_dielec*Im));
        
        z=zmin*l/(Nprof-1.0);
        
        profile_metal_z[l]=z;
        profile_metal[l]=std::abs(std::exp(1e-9*z*kz_metal*Im));
    }
    
    
    profile_graph->clear_graph();
    profile_graph->add_external_data(&profile_dielec,&profile_dielec_z,0,0,1.0,"H");
    profile_graph->add_external_data(&profile_metal,&profile_metal_z,0,0,1.0);
    profile_graph->autoscale();
}

void SppFrame::update_profile_event(wxCommandEvent &event)
{
    update_profile();
}

void SppFrame::update_spectrum(wxCommandEvent &event)
{
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Nl=sp_selector->get_Np();
    
    if(Nl>1000000)
    {
        Nl=1000000;
        sp_selector->set_Np(1000000);
    }
    
    sld_profile->set_range(10*int(m_to_nm(lambda_min)),10*int(m_to_nm(lambda_max)));
    sld_profile->set_value(10*m_to_nm(lambda_min));
    
    recomp_spp();
    
    event.Skip();
}
