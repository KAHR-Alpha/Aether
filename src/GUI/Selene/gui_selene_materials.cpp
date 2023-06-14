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

#include <gui_selene.h>
#include <aether.h>

#include <wx/notebook.h>

namespace SelGUI
{

//###################
//   MaterialPanel
//###################
    
MaterialPanel::MaterialPanel(wxWindow *parent,
                             GUI::Material *material)
    :PanelsListBase(parent),
     original_material(nullptr)
{
    title->Hide();
    
    selector=new MaterialSelector(this,"",true,material);
    
    sizer->Add(selector,wxSizerFlags().Expand());
}

//#####################
//   MaterialsDialog
//#####################

int mat_ID=0;

MaterialsDialog::MaterialsDialog(std::vector<GUI::Material*> &materials_)
    :wxDialog(NULL,-1,"Materials",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     materials(materials_)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Materials
    
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    panel=new wxScrolledWindow(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_SUNKEN);
    wxBoxSizer *panel_sizer=new wxBoxSizer(wxVERTICAL);
    
    materials_panels=new PanelsList<MaterialPanel>(panel);
    
    for(std::size_t i=0;i<materials.size();i++)
        materials_panels->add_panel<MaterialPanel>(materials[i]);
    
    panel_sizer->Add(materials_panels,wxSizerFlags().Expand());
    panel->SetSizer(panel_sizer);
    panel->SetScrollRate(10,10);
    panel->FitInside();
    
    wxButton *add_mat_btn=new wxButton(this,wxID_ANY,"Add Material");
    add_mat_btn->Bind(wxEVT_BUTTON,&MaterialsDialog::evt_add_material,this);
    
    sizer->Add(panel,wxSizerFlags(1).Expand());
    sizer->Add(add_mat_btn);
    
    top_sizer->Add(sizer,wxSizerFlags(1).Expand());
    
    SetSizer(top_sizer);
    
    Bind(wxEVT_CLOSE_WINDOW,&MaterialsDialog::evt_close,this);
    Bind(EVT_MAT_SELECTOR,&MaterialsDialog::evt_list,this);
    Bind(EVT_PLIST_REMOVE,&MaterialsDialog::evt_list,this);
    Bind(EVT_PLIST_RESIZE,&MaterialsDialog::evt_list,this);
    
    ShowModal();
}

void MaterialsDialog::evt_add_material(wxCommandEvent &event)
{    
    MaterialPanel *mat_panel=materials_panels->add_panel<MaterialPanel>(nullptr);
    
    panel->Layout();
    panel->FitInside();
    
    event.Skip();
}

void MaterialsDialog::evt_close(wxCloseEvent &event)
{
    std::size_t N=materials_panels->get_size();
    
    materials.resize(N);
    
    for(std::size_t i=0;i<N;i++)
    {
        MaterialPanel *panel=materials_panels->get_panel(i);
        materials[i]=panel->selector->get_material();
    }
    
    Destroy();
}

void MaterialsDialog::evt_list(wxCommandEvent &event)
{
    panel->Layout();
    panel->FitInside();
}

//###############
//   IRF_Panel
//###############

IRF_Panel::IRF_Panel(wxWindow *parent,Sel::IRF const &irf)
    :PanelsListBase(parent),
     original_irf(nullptr)
{
    title->Hide();
    
    name=new NamedTextCtrl<std::string>(this,"Name : ",irf.name);
    
    wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    type=new wxChoice(this,wxID_ANY);
    type->Append("Grating");
    type->Append("Multilayers");
    type->Append("Snell Splitter");
    
    switch(irf.type)
    {
        case Sel::IRF_GRATING: type->SetSelection(0); break;
        case Sel::IRF_MULTILAYER: type->SetSelection(1); break;
        case Sel::IRF_SNELL_SPLITTER: type->SetSelection(2); break;
    }
    
    type->Bind(wxEVT_CHOICE,&IRF_Panel::evt_irf_type,this);
    
    type_sizer->Add(type);
    
    // Grating
    
    grating_panel=new wxPanel(this);
    
    wxFlexGridSizer *grating_sizer=new wxFlexGridSizer(3);
    grating_sizer->AddGrowableCol(1);
    
    grating_ref_file=new wxTextCtrl(grating_panel,wxID_ANY,irf.grat_ref_path.generic_string());
    grating_tra_file=new wxTextCtrl(grating_panel,wxID_ANY,irf.grat_tra_path.generic_string());
    
    grating_ref_file->SetEditable(false);
    grating_tra_file->SetEditable(false);
    
    wxButton *grating_ref_btn=new wxButton(grating_panel,0,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    wxButton *grating_tra_btn=new wxButton(grating_panel,1,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    grating_ref_btn->Bind(wxEVT_BUTTON,&IRF_Panel::evt_grating_file,this);
    grating_tra_btn->Bind(wxEVT_BUTTON,&IRF_Panel::evt_grating_file,this);
    
    grating_sizer->Add(new wxStaticText(grating_panel,wxID_ANY,"Reflection: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    grating_sizer->Add(grating_ref_file,wxSizerFlags(1).Expand());
    grating_sizer->Add(grating_ref_btn);
    grating_sizer->Add(new wxStaticText(grating_panel,wxID_ANY,"Transmission: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    grating_sizer->Add(grating_tra_file,wxSizerFlags(1).Expand());
    grating_sizer->Add(grating_tra_btn);
    
    grating_panel->SetSizer(grating_sizer);
    
    type_sizer->Add(grating_panel,wxSizerFlags(1));
    
    // Multilayer
    
    multilayer_panel=new wxPanel(this);
    
    layers=new PanelsList<LayerPanel>(multilayer_panel);
    
    for(std::size_t i=0;i<irf.ml_heights.size();i++)
        layers->add_panel<LayerPanel>(irf.ml_heights[i],0,dynamic_cast<GUI::Material*>(irf.ml_materials[i]),false);
    
    wxButton *add_layer_btn=new wxButton(multilayer_panel,wxID_ANY,"Add Layer");
    add_layer_btn->Bind(wxEVT_BUTTON,&IRF_Panel::evt_add_layer,this);
    
    wxBoxSizer *multilayer_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    multilayer_sizer->Add(layers,wxSizerFlags(1));
    multilayer_sizer->Add(add_layer_btn,wxSizerFlags().Align(wxALIGN_TOP));
    
    multilayer_panel->SetSizer(multilayer_sizer);
    
    type_sizer->Add(multilayer_panel,wxSizerFlags(1));
    
    // Splitter
    
    splitter=new NamedTextCtrl(this,"Factor: ",irf.splitting_factor);
    type_sizer->Add(splitter,wxSizerFlags(1));
    
    //
    
    layout();
        
    sizer->Add(name);
    sizer->Add(type_sizer,wxSizerFlags().Expand());
}

IRF_Panel::IRF_Panel(wxWindow *parent,Sel::IRF *irf)
    :IRF_Panel(parent,*irf)
{
    original_irf=irf;
}

void IRF_Panel::evt_add_layer(wxCommandEvent &event)
{
    layers->add_panel<LayerPanel>();
    
    layout();
}

void IRF_Panel::evt_irf_type(wxCommandEvent &event)
{
    layout();
}

void IRF_Panel::evt_grating_file(wxCommandEvent &event)
{
    wxFileName fname;
    fname=wxFileSelector("Load database",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         wxEmptyString,
                         wxFileSelectorDefaultWildcardStr,
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if(fname.IsOk()==false) return;
    
    if(event.GetId()==0) grating_ref_file->ChangeValue(fname.GetFullPath());
    else grating_tra_file->ChangeValue(fname.GetFullPath());
}

void IRF_Panel::layout()
{
    int selection=type->GetSelection();
    
    switch(selection)
    {
        case 0:
            grating_panel->Show();
            multilayer_panel->Hide();
            splitter->Hide();
            break;
        case 1:
            grating_panel->Hide();
            multilayer_panel->Show();
            splitter->Hide();
            break;
        case 2:
            grating_panel->Hide();
            multilayer_panel->Hide();
            splitter->Show();
            break;
    }
    
    Refresh();
    Layout();
    
    throw_resize_event();
}

//###############
//   IRF_Dialog
//###############

int irf_ID=0;

IRF_Dialog::IRF_Dialog(std::vector<Sel::IRF*> &irfs_)
    :wxDialog(NULL,-1,"Interface Response Functions",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     irfs(irfs_)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    // IRFs
    
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    panel=new wxScrolledWindow(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_SUNKEN);
    wxBoxSizer *panel_sizer=new wxBoxSizer(wxVERTICAL);
    
    irfs_panels=new PanelsList<IRF_Panel>(panel);
    
    for(std::size_t i=0;i<irfs.size();i++)
        irfs_panels->add_panel<IRF_Panel>(irfs[i]);
    
    panel_sizer->Add(irfs_panels,wxSizerFlags().Expand());
    panel->SetSizer(panel_sizer);
    panel->SetScrollRate(10,10);
    panel->FitInside();
    
    wxButton *add_mat_btn=new wxButton(this,wxID_ANY,"Add IRF");
    add_mat_btn->Bind(wxEVT_BUTTON,&IRF_Dialog::evt_add_irf,this);
    
    sizer->Add(panel,wxSizerFlags(1).Expand());
    sizer->Add(add_mat_btn);
    
    top_sizer->Add(sizer,wxSizerFlags(1).Expand());
    
    Bind(EVT_PLIST_RESIZE,&IRF_Dialog::evt_list,this);
    Bind(wxEVT_CLOSE_WINDOW,&IRF_Dialog::evt_ok,this);
    
    SetSizer(top_sizer);
    
    ShowModal();
}

void IRF_Dialog::evt_add_irf(wxCommandEvent &event)
{
    Sel::IRF new_irf;
    new_irf.set_type_snell_splitter(0.5);
    
    new_irf.name="IRF "+std::to_string(irf_ID);
    irf_ID++;
    
    irfs_panels->add_panel<IRF_Panel>(new_irf);
    
    panel->Layout();
    panel->FitInside();
    
    event.Skip();
}

void IRF_Dialog::evt_list(wxCommandEvent &event)
{
    panel->FitInside();
    panel->Layout();
}

void IRF_Dialog::evt_ok(wxCloseEvent &event)
{
    std::size_t N=irfs_panels->get_size();
    
    // Need to make sure some of the previous irfs haven't been deleted
    
    std::vector<IRF_Panel*> panels(N);
    std::vector<Sel::IRF*> original_irfs(N);
    
    for(std::size_t i=0;i<N;i++)
    {
        panels[i]=irfs_panels->get_panel(i);
        original_irfs[i]=panels[i]->original_irf;
    }
    
    for(std::size_t i=0;i<irfs.size();i++)
        if(!vector_contains(original_irfs,irfs[i]))
           delete irfs[i];
    
    irfs.resize(N);
    
    for(std::size_t i=0;i<N;i++)
    {
        if(original_irfs[i]==nullptr) irfs[i]=new Sel::IRF;
        else irfs[i]=original_irfs[i];
        
        int selection=panels[i]->type->GetSelection();
            
        if(selection==0)
        {
            std::string fname_ref=panels[i]->grating_ref_file->GetValue().ToStdString();
            std::string fname_tra=panels[i]->grating_tra_file->GetValue().ToStdString();
            
            irfs[i]->set_type_grating(fname_ref,fname_tra);
        }
        else if(selection==1)
        {
            irfs[i]->set_type_multilayer();
            
            std::size_t Nl=panels[i]->layers->get_size();
            
            irfs[i]->ml_model.set_N_layers(Nl);
            irfs[i]->ml_heights.resize(Nl);
            irfs[i]->ml_materials.resize(Nl);
            
            for(std::size_t j=0;j<Nl;j++)
            {
                irfs[i]->ml_heights[j]=panels[i]->layers->get_panel(j)->get_height();
                irfs[i]->ml_materials[j]=panels[i]->layers->get_panel(j)->get_material();
            }
        }
        else if(selection==2)
        {
            double factor=panels[i]->splitter->get_value();
            factor=std::clamp(factor,0.0,1.0);
            
            irfs[i]->set_type_snell_splitter(factor);
        }
        
        irfs[i]->name=panels[i]->name->get_value();
    }
    
    Destroy();
}

}
