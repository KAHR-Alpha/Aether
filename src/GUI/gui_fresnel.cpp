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

#include <gui_fresnel.h>

extern const double Pi;
extern const Imdouble Im;

//############

FresnelFrame::FresnelFrame(wxString const &title)
    :BaseFrame(title),
     Nl(401),
     n_sup(1.0),
     lambda_min(400e-9),
     lambda_max(800e-9),
     angle(Nl),
     r_te(Nl), r_tm(Nl), r_avg(Nl),
     ar_te(Nl), ar_tm(Nl), ar_avg(Nl)
{
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL),
               *field_sizer=new wxBoxSizer(wxHORIZONTAL),
               *mats_sizer=new wxBoxSizer(wxHORIZONTAL),
               *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    coeff_graph=new Graph(this);
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    // Spectrum
    
    sp_selector=new SpectrumSelector(ctrl_panel,400e-9,800e-9,401);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&FresnelFrame::update_spectrum,this);
    
    sld_wavelength=new SldCombi(ctrl_panel,"Wavelength",4000,4000,8000,0.1,true);
    sld_wavelength->Bind(wxEVT_SLIDER,&FresnelFrame::update_coeffs_event,this);
    
    field_sizer->Add(sp_selector,wxSizerFlags().Expand());
    field_sizer->Add(sld_wavelength,wxSizerFlags(1).Expand());
    
    // Substrate
    
    mat_selector=new MaterialSelector(ctrl_panel,"Substrate");
    mat_selector->Bind(EVT_MAT_SELECTOR,&FresnelFrame::load_material,this);
    
    // Dielectric sizer
    
    wxStaticBoxSizer *sup_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Superstrate");
    
    wxStaticText *n_sup_txt=new wxStaticText(ctrl_panel,wxID_ANY,"n: ");
    sup_ctrl=new wxTextCtrl(ctrl_panel,wxID_ANY,"1.0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    sup_ctrl->Bind(wxEVT_TEXT_ENTER,&FresnelFrame::update_const_n,this);
    sup_sizer->Add(n_sup_txt);
    sup_sizer->Add(sup_ctrl,wxSizerFlags(1));
    
    mats_sizer->Add(sup_sizer,wxSizerFlags());
    mats_sizer->Add(mat_selector,wxSizerFlags(1));
    
    //ctrl_sizer
    
    ctrl_sizer->Add(field_sizer,wxSizerFlags().Expand());
    ctrl_sizer->Add(mats_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    top_sizer->Add(coeff_graph,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    SetSizer(top_sizer);
    
    this->Show(true);
    
    Maximize();
    
    update_coeffs();
}

FresnelFrame::~FresnelFrame()
{
    std::cout<<"Destructing FresnelFrame"<<std::endl;
}

void FresnelFrame::load_material(wxCommandEvent &event)
{
    update_coeffs();
}

void FresnelFrame::update_coeffs()
{
    unsigned int l;
    
    Imdouble r_te_tmp,
             r_tm_tmp,
             ar_te_tmp,
             ar_tm_tmp;
    
    double lambda_tmp=sld_wavelength->get_value()*1e-9;
    double w=2.0*Pi*c_light/lambda_tmp;
    
    Imdouble n2=std::sqrt(mat_selector->get_eps(w));
    
    for(l=0;l<Nl;l++)
    {
        angle[l]=90*l/(Nl-1.0);
        Angle ang_inc=Degree(angle[l]);
        
        fresnel_rt_te_tm(ang_inc,n_sup,n2,
                         r_te_tmp,r_tm_tmp,ar_te_tmp,ar_tm_tmp);
        
        r_te[l]=std::norm(r_te_tmp);
        r_tm[l]=std::norm(r_tm_tmp);
        r_avg[l]=0.5*(r_te[l]+r_tm[l]);
        
        ar_te[l]=1-r_te[l];
        ar_tm[l]=1-r_tm[l];
        ar_avg[l]=0.5*(ar_te[l]+ar_tm[l]);
    }
    
    coeff_graph->clear_graph();
    
    coeff_graph->add_data(&angle,&r_te,0,0,1.0,"R_s");
    coeff_graph->add_data(&angle,&r_tm,1.0,0,0,"R_p");
    coeff_graph->add_data(&angle,&r_avg,0,1.0,0,"R_avg");
    
    coeff_graph->add_data(&angle,&ar_te,0.5,0,0.5,"1-R_s");
    coeff_graph->add_data(&angle,&ar_tm,0.5,0.5,0,"1-R_p");
    coeff_graph->add_data(&angle,&ar_avg,0,0.5,0.5,"1-R_avg");
    
    coeff_graph->set_scale(0,90,0,1);
    coeff_graph->Refresh();
}

void FresnelFrame::update_coeffs_event(wxCommandEvent &event)
{
    update_coeffs();
}

void FresnelFrame::update_const_n(wxCommandEvent &event)
{
    textctrl_to_T(sup_ctrl,n_sup);
    
    if(n_sup<1.0)
    {
        n_sup=1.0;
        sup_ctrl->ChangeValue("1.0");
    }
    
    update_coeffs();
}

void FresnelFrame::update_spectrum(wxCommandEvent &event)
{
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Nl=sp_selector->get_Np();
    
    if(Nl>1000000)
    {
        Nl=1000000;
        sp_selector->set_Np(1000000);
    }
    
    angle.resize(Nl);
    r_te.resize(Nl);
    r_tm.resize(Nl);
    r_avg.resize(Nl);
    ar_te.resize(Nl);
    ar_tm.resize(Nl);
    ar_avg.resize(Nl);
    
    sld_wavelength->set_range(10*int(m_to_nm(lambda_min)),10*int(m_to_nm(lambda_max)));
    sld_wavelength->set_value(10*m_to_nm(lambda_min));
    
    update_coeffs();
    
    event.Skip();
}
