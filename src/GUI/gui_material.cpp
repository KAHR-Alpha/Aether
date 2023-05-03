/*Copyright 2008-2023 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <aether.h>
#include <filehdl.h>
#include <phys_tools.h>
#include <string_tools.h>
#include <gui_material.h>

wxDEFINE_EVENT(EVT_MAT_SELECTOR,wxCommandEvent);

//####################
// MaterialExplorer
//####################

MaterialExplorer::MaterialExplorer(wxString const &string)
    :MaterialExplorer(400e-9,800e-9,401,nullptr)
{
}

MaterialExplorer::MaterialExplorer(double lambda_min_,double lambda_max_,int Np_,MaterialSelector *selector)
    :BaseFrame(""),
     Np(Np_),
     lambda_min(lambda_min_), lambda_max(lambda_max_),
     lambda(Np), disp_lambda(Np),
     disp_real(Np), disp_imag(Np)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    mat_graph=new Graph(this);
    
    // Ctrl panel
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    wxBoxSizer *disp_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *mat_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *index_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Display");
    
    GUI::Material *external_mat=nullptr;
    if(selector!=nullptr) external_mat=selector->material;
    
    mat_selector=new MaterialSelector(ctrl_panel,"Material",true,external_mat);
    sp_selector=new SpectrumSelector(ctrl_panel,lambda_min,lambda_max,Np);
    
    wxString disp_str[]={"Index","Permittivity"};
    
    disp_choice=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_str);
    disp_choice->SetSelection(0);
    
    wxButton *export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    
    index_sizer->Add(disp_choice);
    
    disp_sizer->Add(sp_selector,wxSizerFlags());
    disp_sizer->Add(index_sizer);
    disp_sizer->Add(export_btn,wxSizerFlags().Expand().Border(wxALL,3));
    
    mat_sizer->Add(mat_selector,wxSizerFlags(1));
    
    ctrl_sizer->Add(disp_sizer);
    ctrl_sizer->Add(mat_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // 
    
    top_sizer->Add(mat_graph,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    export_btn->Bind(wxEVT_BUTTON,&MaterialExplorer::export_event,this);
    disp_choice->Bind(wxEVT_CHOICE,&MaterialExplorer::disp_choice_event,this);
    mat_selector->Bind(EVT_MAT_SELECTOR,&MaterialExplorer::material_selector_event,this);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MaterialExplorer::spectrum_selector_event,this);

    SetTitle(mat_selector->material->get_description());
    
    SetSizer(top_sizer);
    
    Show();
    Maximize();
}

void MaterialExplorer::disp_choice_event(wxCommandEvent &event)
{
    recompute_model();
}

void MaterialExplorer::export_event(wxCommandEvent &event)
{
    std::vector<wxString> choices(3);
    
    choices[0]="Permittivity";
    choices[1]="Refractive index";
    choices[2]="MatLab Script";
    
    ChoiceDialog dialog("Export type",choices);
    
    if(!dialog.choice_ok) return;
    
    wxFileName data_tmp=wxFileSelector("Save the structure script as",
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxFileSelectorDefaultWildcardStr,
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(data_tmp.IsOk()==false) return;
    
    std::ofstream file(data_tmp.GetFullPath().ToStdString(),
                       std::ios::out|std::ios::trunc|std::ios::binary);
    
    if(dialog.choice==0 || dialog.choice==1)
    {
        for(unsigned int l=0;l<Np;l++)
        {
            Imdouble eps=mat_selector->get_eps(m_to_rad_Hz(lambda[l]));
            
            if(dialog.choice==0) file<<lambda[l]<<" "<<eps.real()<<" "<<eps.imag()<<std::endl;
            else
            {
                Imdouble n=std::sqrt(eps);
                
                file<<lambda[l]<<" "<<n.real()<<" "<<n.imag()<<std::endl;
            }
        }
    }
    else
    {
        GUI::Material *mat=mat_selector->get_material();
        
        file<<mat->get_matlab(data_tmp.GetFullPath().ToStdString());
    }
}

void MaterialExplorer::material_selector_event(wxCommandEvent &event)
{
    lambda_min=mat_selector->material->get_lambda_validity_min();
    lambda_max=mat_selector->material->get_lambda_validity_max();
    
    sp_selector->set_spectrum(lambda_min,lambda_max);
    
    SetTitle(mat_selector->material->get_description());
    
    recompute_model();
}

void MaterialExplorer::spectrum_selector_event(wxCommandEvent &event)
{
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    Np=sp_selector->get_Np();
    
    lambda.resize(Np);
    disp_lambda.resize(Np);
    disp_real.resize(Np);
    disp_imag.resize(Np);
    
    recompute_model();
}

void MaterialExplorer::recompute_model()
{
    int display_type=disp_choice->GetSelection();
        
    for(unsigned int i=0;i<Np;i++)
    {
        lambda[i]=lambda_min+(lambda_max-lambda_min)*i/(Np-1.0);
        disp_lambda[i]=m_to_nm(lambda[i]);
        
        double w=m_to_rad_Hz(lambda[i]);
        
        Imdouble eps=mat_selector->get_eps(w);
        
        if(display_type==0)
        {
            Imdouble index=std::sqrt(eps);
            
            disp_real[i]=index.real();
            disp_imag[i]=index.imag();
        }
        else if(display_type==1)
        {
            disp_real[i]=eps.real();
            disp_imag[i]=eps.imag();
        }
    }
    
    mat_graph->clear_graph();
    
    if(display_type==0)
    {
        mat_graph->add_external_data(&disp_lambda,&disp_real,0,0,1.0,"Real(Ref Index)");
        mat_graph->add_external_data(&disp_lambda,&disp_imag,1.0,0,0,"Imag(Ref Index)");
    }
    else if(display_type==1)
    {
        mat_graph->add_external_data(&disp_lambda,&disp_real,0,0,1.0,"Real(Permittivity)");
        mat_graph->add_external_data(&disp_lambda,&disp_imag,1.0,0,0,"Imag(Permittivity)");
    }
    
    mat_graph->autoscale();
}
