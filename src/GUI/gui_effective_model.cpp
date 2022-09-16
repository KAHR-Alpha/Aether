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

#include <gui_effective_model.h>

extern const double Pi;
extern const Imdouble Im;

//###############
// EffModelFrame
//###############

EffModelFrame::EffModelFrame(wxString const &title)
    :BaseFrame(title),
     model_type(0),
     display_type(1),
     Nl(401),
     lambda_min(400e-9),
     lambda_max(800e-9),
     lambda(Nl),
     eff_model_re(Nl),
     eff_model_im(Nl)
{
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL),
               *field_sizer=new wxBoxSizer(wxHORIZONTAL),
               *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    coeff_graph=new Graph(this);
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    // Spectrum
    
    sp_selector=new SpectrumSelector(ctrl_panel,400e-9,800e-9,401);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&EffModelFrame::update_spectrum,this);
    
    wxString model_choices[]={"Maxwell-Garnett 1",
                              "Maxwell-Garnett 2",
                              "Bruggeman",
                              "Looyenga",
                              "Weighted Sum",
                              "Weighted Sum Inv"};
    
    wxStaticBoxSizer *model_type_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Model");
    model_type_ctrl=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,6,model_choices);
    model_type_ctrl->SetSelection(0);
    model_type_ctrl->Bind(wxEVT_CHOICE,&EffModelFrame::switch_model,this);
    model_type_sizer->Add(model_type_ctrl);
    
    wxString display_choices[]={"Refractive Index",
                                "Permittivity"};
    
    wxStaticBoxSizer *display_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Display");
    display_ctrl=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,display_choices);
    display_ctrl->SetSelection(1);
    display_ctrl->Bind(wxEVT_CHOICE,&EffModelFrame::switch_display,this);
    display_sizer->Add(display_ctrl);
    
    wxButton *export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    export_btn->Bind(wxEVT_BUTTON,&EffModelFrame::export_data,this);
    
    field_sizer->Add(sp_selector,wxSizerFlags().Expand());
    field_sizer->Add(model_type_sizer,wxSizerFlags().Expand());
    field_sizer->Add(display_sizer,wxSizerFlags().Expand());
    field_sizer->Add(export_btn,wxSizerFlags().Expand().Border(wxTOP,3).Align(wxCENTER));
    
    // Materials
    
    mat_1=new MaterialSelector(ctrl_panel,"Material 1");
    mat_2=new MaterialSelector(ctrl_panel,"Material 2");
    
    mat_1->show_weight();
    mat_2->show_weight();
    
    //ctrl_sizer
    
    ctrl_sizer->Add(field_sizer,wxSizerFlags().Expand());
    ctrl_sizer->Add(mat_1,wxSizerFlags().Expand());
    ctrl_sizer->Add(mat_2,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    top_sizer->Add(coeff_graph,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    SetSizer(top_sizer);
    
    this->Show(true);
    
    Maximize();
    
    recomp_model();
    
    Bind(EVT_MAT_SELECTOR,&EffModelFrame::recomp_model_event,this);
}

EffModelFrame::~EffModelFrame()
{
    std::cout<<"Destructing EffModelFrame"<<std::endl;
}

void EffModelFrame::export_data(wxCommandEvent &event)
{
    wxFileName data_tmp=wxFileSelector("Save the structure script as",
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxFileSelectorDefaultWildcardStr,
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(data_tmp.IsOk()==false) return;
    
    std::ofstream file(data_tmp.GetFullPath().ToStdString(),
                       std::ios::out|std::ios::trunc|std::ios::binary);
    
    for(unsigned int l=0;l<Nl;l++)
        file<<nm_to_m(lambda[l])<<" "<<eff_model_re[l]<<" "<<eff_model_im[l]<<std::endl;
}

void EffModelFrame::switch_display(wxCommandEvent &event)
{
    display_type=display_ctrl->GetSelection();
    
    recomp_model();
}

void EffModelFrame::switch_model(wxCommandEvent &event)
{
    model_type=model_type_ctrl->GetSelection();
    
    recomp_model();
}

void EffModelFrame::recomp_model()
{
    unsigned int l;
    
    Imdouble eps_1,eps_2,eps_eff(1.0),ref_index(1.0);
    
    double weight_1=mat_1->get_weight();
    double weight_2=mat_2->get_weight();
    
    double weight_sum=weight_1+weight_2;
    
    weight_1/=weight_sum;
    weight_2/=weight_sum;
    
    for(l=0;l<Nl;l++)
    {
        lambda[l]=m_to_nm(lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0));
        
        double w=2.0*Pi*c_light/(nm_to_m(lambda[l]));
        
        eps_1=mat_1->get_eps(w);
        eps_2=mat_2->get_eps(w);
        
             if(model_type==0) eps_eff=effmodel_maxwell_garnett_1(eps_1,eps_2,weight_1,weight_2);
        else if(model_type==1) eps_eff=effmodel_maxwell_garnett_2(eps_1,eps_2,weight_1,weight_2);
        else if(model_type==2) eps_eff=effmodel_bruggeman(eps_1,eps_2,weight_1,weight_2);
        else if(model_type==3) eps_eff=effmodel_looyenga(eps_1,eps_2,weight_1,weight_2);
        else if(model_type==4) eps_eff=effmodel_sum(eps_1,eps_2,weight_1,weight_2);
        else if(model_type==5) eps_eff=effmodel_sum_inv(eps_1,eps_2,weight_1,weight_2);
        
        if(display_type==0)
        {
            ref_index=std::sqrt(eps_eff);
            
            eff_model_re[l]=ref_index.real();
            eff_model_im[l]=ref_index.imag();
        }
        else
        {
            eff_model_re[l]=eps_eff.real();
            eff_model_im[l]=eps_eff.imag();
        }
    }
    
    coeff_graph->clear_graph();
    
    if(display_type==0)
    {
        coeff_graph->add_external_data(&lambda,&eff_model_re,0,0,1.0,"Real(Ref Index)");
        coeff_graph->add_external_data(&lambda,&eff_model_im,1.0,0,0,"Imag(Ref Index)");
    }
    else if(display_type==1)
    {
        coeff_graph->add_external_data(&lambda,&eff_model_re,0,0,1.0,"Real(Permittivity)");
        coeff_graph->add_external_data(&lambda,&eff_model_im,1.0,0,0,"Imag(Permittivity)");
    }
    
    coeff_graph->autoscale();
}

void EffModelFrame::recomp_model_event(wxCommandEvent &event)
{
    recomp_model();
}

void EffModelFrame::update_spectrum(wxCommandEvent &event)
{
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Nl=sp_selector->get_Np();
    
    if(Nl>1000000) Nl=1000000;
    
    lambda.resize(Nl);
    eff_model_re.resize(Nl);
    eff_model_im.resize(Nl);
        
    recomp_model();
    
    event.Skip();
}

