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

wxDEFINE_EVENT(EVT_MINIMAT_SELECTOR,wxCommandEvent);

//################
//   MMS_Dialog
//################

class MMS_Dialog:public wxDialog
{
    public:
        GUI::Material *material;
        MaterialSelector *selector;
        
        wxPanel *container_panel;
        wxScrolledWindow *selector_panel;
        
        MMS_Dialog(GUI::Material *material_,wxWindow *parent_selector)
            :wxDialog(0,wxID_ANY,"Select the material",
                      wxGetApp().default_dialog_origin(),wxDefaultSize),
             material(material_)
        {            
            wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
            
            // Container Panel
            
            container_panel=new wxPanel(this);
            
            wxBoxSizer *container_sizer=new wxBoxSizer(wxVERTICAL);
            
            // Inside Panel
            
            selector_panel=new wxScrolledWindow(container_panel,wxID_ANY);
            
            wxBoxSizer *selector_sizer=new wxBoxSizer(wxVERTICAL);
            selector=new MaterialSelector(selector_panel,"Material",false,material);
            selector->parent_selector=parent_selector;
            
            selector->hide_description();
            selector->hide_validity();
            
            selector_sizer->Add(selector,wxSizerFlags());
            
            selector_panel->SetSizerAndFit(selector_sizer);
            selector_panel->SetScrollRate(50,50);
            
            container_sizer->Add(selector_panel,wxSizerFlags(1).Expand());
            container_panel->SetSizer(container_sizer);
            
            top_sizer->Add(container_panel,wxSizerFlags(1).Expand());
            
            SetSizerAndFit(top_sizer);
            
            Bind(EVT_MAT_SELECTOR,&MMS_Dialog::evt_material,this);
            
            ShowModal();
        }
                
        void evt_material(wxCommandEvent &event)
        {
            SetClientSize(wxSize(20,20));
            selector_panel->Layout();
            selector_panel->FitInside();
            
            int x,y;
            selector_panel->GetVirtualSize(&x,&y);
            wxSize max_size=wxGetApp().default_dialog_size();
            
            SetClientSize(wxSize(std::min(max_size.x,x),std::min(max_size.y,y+3)));
            
            selector_panel->FitInside();
            
            material=selector->get_material();
        }
};

//##########################
//   MiniMaterialSelector
//##########################

MiniMaterialSelector::MiniMaterialSelector(wxWindow *parent,
                                           GUI::Material *material_,
                                           std::string const &name)
    :wxPanel(parent),
     material(material_)
{
    if(material==nullptr)
    {
        material=MaterialsLib::request_material(MatType::REAL_N);
        material->original_requester=this;
    }
    
    wxSizer *sizer=nullptr;
    if(name=="") sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    // Symbol
    
    mat_txt=new wxStaticText(this,wxID_ANY,"n");
    
    // Controls
    
    mat_description=new wxTextCtrl(this,wxID_ANY,"1.0");
    mat_description->Disable();
    
    eff_weight=new NamedTextCtrl<std::string>(this,"","0",false,8);
    
    edit_btn=new wxButton(this,wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    edit_btn->Bind(wxEVT_BUTTON,&MiniMaterialSelector::evt_edit,this);
    
    eff_weight->Hide();
    eff_weight->Bind(EVT_NAMEDTXTCTRL,&MiniMaterialSelector::evt_weight,this);
    
    sizer->Add(mat_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,5).Border(wxRIGHT,5));
    sizer->Add(mat_description,wxSizerFlags(1).Expand());
    sizer->Add(eff_weight,wxSizerFlags());
    sizer->Add(edit_btn,wxSizerFlags());
    
    SetSizer(sizer);
    
    update_display();
    
    MaterialsLib::register_control(this);
}

MiniMaterialSelector::~MiniMaterialSelector()
{
    MaterialsLib::forget_control(this);
}

void MiniMaterialSelector::copy_material(MiniMaterialSelector *mat_)
{
    material=mat_->material;
    
    update_display();
}

void MiniMaterialSelector::evt_edit(wxCommandEvent &event)
{
    MMS_Dialog dialog(material,this);
    material=dialog.material;
    
    update_display();
    
    wxCommandEvent event_out(EVT_MINIMAT_SELECTOR);
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void MiniMaterialSelector::evt_weight(wxCommandEvent &event)
{
    std::size_t Nw=material->eff_weights.size();
    
    // Input
    
    std::stringstream strm_in(eff_weight->get_value());
    
    double tmp_weight,weight_sum=0;
    
    for(std::size_t i=0;i<Nw;i++)
    {
        if(strm_in.eof()) tmp_weight=0;
        else strm_in>>tmp_weight;
        
        if(tmp_weight<0) tmp_weight=0;
        
        weight_sum+=tmp_weight;
        material->eff_weights[i]=tmp_weight;
    }
    
    if(weight_sum==0) material->eff_weights[0]=1;
    
    // Reformatting
    
    std::stringstream strm_out;
    
    for(std::size_t i=0;i<Nw;i++)
    {
        strm_out<<material->eff_weights[i];
        if(i+1!=Nw) strm_out<<" ";
    }
    
    eff_weight->set_value(strm_out.str());
    
    wxCommandEvent event_out(EVT_MINIMAT_SELECTOR);
    wxPostEvent(this,event_out);
}

Imdouble MiniMaterialSelector::get_eps(double w) { return material->get_eps(w); }

Imdouble MiniMaterialSelector::get_n(double w) { return material->get_n(w); }

GUI::Material* MiniMaterialSelector::get_material() { return material; }

void MiniMaterialSelector::lock()
{
    edit_btn->Disable();
    eff_weight->lock();
}

void MiniMaterialSelector::set_material(Material *material_)
{
    set_material(dynamic_cast<GUI::Material*>(material_));
}

void MiniMaterialSelector::set_material(GUI::Material *material_)
{
    material=material_;
    
    update_display();
}

void MiniMaterialSelector::update_display()
{
    update_label();
    
    wxString name=material->get_short_description();
    
    mat_description->ChangeValue(name);
    edit_btn->SetToolTip(name);
    
    if(material->is_effective_material)
    {
        eff_weight->Show();
        
        std::stringstream strm;
        
        std::size_t Nw=material->eff_weights.size();
        
        for(std::size_t i=0;i<Nw;i++)
        {
            strm<<material->eff_weights[i];
            if(i+1!=Nw) strm<<" ";
        }
        
        eff_weight->set_value(strm.str());
    }
    else eff_weight->Hide();
    
    Layout();
}

void MiniMaterialSelector::unlock()
{
    edit_btn->Enable();
    eff_weight->unlock();
}

void MiniMaterialSelector::update_label()
{
    switch(material->type)
    {
        case MatType::REAL_N:
            mat_txt->SetLabel("n = ");
            break;
            
        case MatType::LIBRARY:
            mat_txt->SetLabel("Lib");
            break;
            
        case MatType::USER_LIBRARY:
            mat_txt->SetLabel("Usr");
            break;
            
        case MatType::SCRIPT:
            mat_txt->SetLabel("Scr");
            break;
    
        case MatType::EFFECTIVE:
            mat_txt->SetLabel("Eff");
            break;
            
        case MatType::CUSTOM:
            mat_txt->SetLabel("Ctm");
            break;
        
        default:
            mat_txt->SetLabel("Ukn");
    }
    
    Layout();
}
