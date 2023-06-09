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

#include <filehdl.h>

#include <gui_material.h>

//######################
//   EffMaterialPanel
//######################

wxDEFINE_EVENT(EVT_GARNETT_HOST,wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECTIVE_MATERIAL,wxCommandEvent);

EffMaterialPanel::EffMaterialPanel(wxWindow *parent,GUI::Material *material,double weight_)
    :PanelsListBase(parent)
{
    wxBoxSizer *mat_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    selector=new MiniMaterialSelector(this,material,"");
    selector->SetMinClientSize(wxSize(500,-1));
    selector->Bind(EVT_MINIMAT_SELECTOR,&EffMaterialPanel::evt_material,this);
    
    weight=new NamedTextCtrl<double>(this," weight: ",weight_,false);
    weight->Bind(EVT_NAMEDTXTCTRL,&EffMaterialPanel::evt_material,this);
    
    host=new wxCheckBox(this,wxID_ANY,"Host");
    host->Hide();
    host->Bind(wxEVT_CHECKBOX,&EffMaterialPanel::evt_host,this);
    
    mat_sizer->Add(selector);
    mat_sizer->Add(weight);
    mat_sizer->Add(host);
    
    sizer->Add(mat_sizer);
}

void EffMaterialPanel::evt_host(wxCommandEvent &event)
{
    if(host->GetValue()==true)
    {
        wxCommandEvent event_out(EVT_GARNETT_HOST);
        event_out.SetClientData(this);
        
        wxPostEvent(this,event_out);
    }
    else host->SetValue(true);
}

void EffMaterialPanel::evt_material(wxCommandEvent &event)
{
    wxCommandEvent event_out(EVT_EFFECTIVE_MATERIAL);
    event_out.SetClientData(this);
    
    wxPostEvent(this,event_out);
}

void EffMaterialPanel::hide_host()
{
    host->Hide();
}

void EffMaterialPanel::show_host()
{
    host->Show();
}

//####################
//  MaterialSelector
//####################

MaterialSelector::MaterialSelector(wxWindow *parent,
                                   std::string name,bool no_box,
                                   GUI::Material *material_,
                                   bool (*validator)(Material*))
    :wxPanel(parent),
     material(material_),
     parent_selector(nullptr),
     const_index(1.0),
     accept_material(validator)
{
    if(material==nullptr)
    {
        material=MaterialsLib::request_material(MatType::REAL_N);
        material->original_requester=this;
    }
    
    wxSizer *sizer=nullptr;
    
    wxWindow *panel=this;
    
    if(no_box) sizer=new wxBoxSizer(wxHORIZONTAL);
    else
    {
        wxStaticBoxSizer *box_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
        
        sizer=box_sizer;
        panel=box_sizer->GetStaticBox();
    }
    
    // Library
    
    wxButton *lib_btn=new wxButton(panel,wxID_ANY,"Library");
    lib_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_library,this);
    
    sizer->Add(lib_btn,wxSizerFlags().Expand().Border(wxRIGHT,3));
    
    // Central Panel
    
    wxBoxSizer *central_sizer=new wxBoxSizer(wxVERTICAL);
    sizer->Add(central_sizer);
    
    // - Header
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxVERTICAL);
    
    type_description=new NamedTextCtrl<std::string>(panel,"Type: ","");
    type_description->lock();
    
    name_ctrl=new NamedTextCtrl<std::string>(panel,"Name: ",material->name);
    name_ctrl->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_name,this);
    
    header_sizer->Add(type_description);
    header_sizer->Add(name_ctrl);
    
    update_header();
    
    central_sizer->Add(header_sizer);
    
    // - Material
    
    wxBoxSizer *material_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    index_ctrl=new NamedTextCtrl(panel,"Refractive index: ",std::sqrt(material->eps_inf));
    index_ctrl->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_const_index,this);
    material_sizer->Add(index_ctrl);
    
    MaterialSelector_CustomPanel(panel);
    MaterialSelector_EffPanel(panel);
    
    mat_txt=new wxTextCtrl(panel,wxID_ANY,"");
    mat_txt->SetEditable(false);
    mat_txt->SetMinClientSize(wxSize(500,-1));
    
    material_sizer->Add(custom_editor,wxSizerFlags(1));
    material_sizer->Add(eff_panel,wxSizerFlags(1));
    material_sizer->Add(mat_txt,wxSizerFlags(1));
    
    central_sizer->Add(material_sizer);
    
    // Closing
    
    inspect_btn=new wxButton(panel,wxID_ANY,"Inspect");
    inspect_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_inspect,this);
    
    sizer->Add(inspect_btn,wxSizerFlags().Expand().Border(wxLEFT,3));
    
    update_layout();
    
    // TODO
    /*Material tmp_material(material);
    bool input_ok=accept_material(&tmp_material);
    
    if(material.is_effective_material)
    {
        mat_type=MatType::EFFECTIVE;
        
        allocate_effective_materials(*(material.eff_mat_1),*(material.eff_mat_2));
        
        switch(material.effective_type)
        {
            case MAT_EFF_BRUGGEMAN: eff_type->SetSelection(0); break;
            case MAT_EFF_MG1: eff_type->SetSelection(1); break;
            case MAT_EFF_MG2: eff_type->SetSelection(2); break;
            case MAT_EFF_LOYENGA: eff_type->SetSelection(3); break;
            case MAT_EFF_SUM: eff_type->SetSelection(4); break;
            case MAT_EFF_SUM_INV: eff_type->SetSelection(5); break;
        }
        
        eff_weight->set_value(material.eff_weight);
        
        layout_effective();
    }
    else
    {
        if(material.is_const() || input_ok==false)
        {
            set_const_model(material.get_n(0).real());
        }
        else
        {
            unsigned int l;
            Material *pm=MaterialsLib::knows_material(l,material,validator);
            
            if(pm==nullptr)
            {
                mat_type=MatType::SCRIPT;
                mat_type_ctrl->SetSelection(2);
                
                script_model=material;
                script=material.script_path;
                
                mat_txt->ChangeValue(wxString(script.generic_string()));
                //layout_script();
            }
            else
            {
                mat_type=MatType::LIBRARY;
                mat_type_ctrl->SetSelection(0);
                layout_library();
            }
        }
    }*/
    
    SetSizer(sizer);
}

void MaterialSelector::MaterialSelector_CustomPanel(wxWindow *parent)
{
    custom_editor=new MaterialEditor(parent,material,true);
    custom_editor->Bind(EVT_MATERIAL_EDITOR_MODEL,&MaterialSelector::evt_custom_material,this);
}

void MaterialSelector::MaterialSelector_EffPanel(wxWindow *parent)
{
    eff_panel=new wxPanel(parent);
    
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    eff_panel->SetSizer(sizer);
    
    // Header
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    eff_type=new wxChoice(eff_panel,wxID_ANY);
    
    eff_type->Append("Bruggeman");
    eff_type->Append("Looyenga");
    eff_type->Append("Maxwell-Garnett");
    eff_type->Append("Weighted Sum");
    eff_type->Append("Weighted Sum Inv");
    eff_type->SetSelection(0);
    
    eff_type->Bind(wxEVT_CHOICE,&MaterialSelector::evt_effective_type,this);
    
    header_sizer->Add(new wxStaticText(eff_panel,wxID_ANY,"Effective Model: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    header_sizer->Add(eff_type);
    
    sizer->Add(header_sizer);
    
    // Materials
    
    wxStaticBoxSizer *components_sizer=new wxStaticBoxSizer(wxHORIZONTAL,eff_panel,"Components");
    
    effective_ctrl=new PanelsList<EffMaterialPanel>(components_sizer->GetStaticBox());
    wxButton *add_mat_btn=new wxButton(components_sizer->GetStaticBox(),wxID_ANY,"Add");
    add_mat_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_add_effective_component,this);
    
    components_sizer->Add(effective_ctrl);
    components_sizer->Add(add_mat_btn);
    
    sizer->Add(components_sizer);    
    
    Bind(EVT_GARNETT_HOST,&MaterialSelector::evt_effective_host,this);
    Bind(EVT_EFFECTIVE_MATERIAL,&MaterialSelector::evt_effective_component,this);
}

MaterialSelector::~MaterialSelector()
{
}

// - MaterialSelector member functions

void MaterialSelector::add_effective_component()
{
    GUI::Material *effective_component=MaterialsLib::request_material(MatType::REAL_N);
    
    effective_ctrl->add_panel<EffMaterialPanel>(effective_component,1.0);
    
    EffectiveModel type=get_effective_material_type();
    
    if(type==EffectiveModel::MAXWELL_GARNETT)
    {
        for(std::size_t i=0;i<effective_ctrl->get_size();i++)
        {
            EffMaterialPanel *panel=effective_ctrl->get_panel(i);
            panel->host->SetValue(false);
            panel->show_host();
        }
                
        EffMaterialPanel *panel=effective_ctrl->get_panel(0);
        panel->host->SetValue(true);
    }
    
    rebuild_effective_material();
    
    throw_event();
}

void MaterialSelector::evt_const_index(wxCommandEvent &event)
{
    double n=index_ctrl->get_value();
    
    material->eps_inf=n*n;
    
    throw_event();
}

void MaterialSelector::evt_custom_material(wxCommandEvent &event)
{
    throw_event();
}

EffectiveModel MaterialSelector::get_effective_material_type()
{
    switch(eff_type->GetSelection())
    {
        case 0: return EffectiveModel::BRUGGEMAN;
        case 1: return EffectiveModel::LOOYENGA;
        case 2: return EffectiveModel::MAXWELL_GARNETT;
        case 3: return EffectiveModel::SUM;
        case 4: return EffectiveModel::SUM_INV;
        default: return EffectiveModel::BRUGGEMAN;
    }
}

void MaterialSelector::evt_add_effective_component(wxCommandEvent &event)
{
    add_effective_component();
    
    rebuild_effective_material();
}

void MaterialSelector::evt_effective_type(wxCommandEvent &event)
{
    EffectiveModel eff_type_value=get_effective_material_type();
    
    if(eff_type_value==EffectiveModel::MAXWELL_GARNETT)
    {
        bool has_host=false;
        
        for(std::size_t i=0;i<effective_ctrl->get_size();i++)
        {
            EffMaterialPanel *panel=effective_ctrl->get_panel(i);
            
            panel->show_host();
            has_host|=panel->host->GetValue();
        }
        
        if(!has_host && effective_ctrl->get_size()>0)
        {
            effective_ctrl->get_panel(0)->host->SetValue(true);
        }
    }
    else
    {
        for(std::size_t i=0;i<effective_ctrl->get_size();i++)
            effective_ctrl->get_panel(i)->hide_host();
    }
    
    rebuild_effective_material();
    
    throw_event();
}

void MaterialSelector::evt_effective_component(wxCommandEvent &event)
{
    rebuild_effective_material();
}

void MaterialSelector::evt_effective_host(wxCommandEvent &event)
{
    EffMaterialPanel *caller=static_cast<EffMaterialPanel*>(event.GetClientData());
    
    for(std::size_t i=0;i<effective_ctrl->get_size();i++)
    {
        EffMaterialPanel *panel=effective_ctrl->get_panel(i);
        
        if(panel!=caller)
        {
            panel->host->SetValue(false);
        }
    }
    
    rebuild_effective_material();
}

void MaterialSelector::evt_inspect(wxCommandEvent &event)
{
    double lambda_min=material->get_lambda_validity_min();
    double lambda_max=material->get_lambda_validity_max();
    
    MaterialExplorer *mat_expl=new MaterialExplorer(lambda_min,lambda_max,401,this);
    null_function(mat_expl);
}

void MaterialSelector::evt_library(wxCommandEvent &event)
{
    wxWindow *requester=parent_selector;
    if(requester==nullptr) requester=this;
    
    MaterialsLibDialog dialog(requester);
    
    if(dialog.selection_ok)
    {
        material=dialog.material;
        
        if(dialog.new_material && material->type==MatType::EFFECTIVE)
        {
            add_effective_component();
            add_effective_component();
            
            rebuild_effective_material();
        }
    }
    
    update_header();
    update_layout();
    
    throw_event();
}

void MaterialSelector::evt_name(wxCommandEvent &event)
{
    material->name=name_ctrl->get_value();
    
    wxWindow *requester=parent_selector;
    if(requester==nullptr) requester=this;
    
    if(   !material->name.empty()
       && material->original_requester==requester)
    {
        material->original_requester=nullptr;
    }
}

Imdouble MaterialSelector::get_eps(double w) { return material->get_eps(w); }

GUI::Material* MaterialSelector::get_material()
{
    return material;
}

double MaterialSelector::get_weight() { return 0; }

void MaterialSelector::throw_event()
{
    wxCommandEvent event(EVT_MAT_SELECTOR);
    wxPostEvent(this,event);
}

void MaterialSelector::rebuild_effective_material()
{
    material->is_effective_material=true;
    material->effective_type=get_effective_material_type();
    
    std::size_t N_eff=effective_ctrl->get_size();
    
    material->eff_mats.resize(N_eff);
    material->eff_weights.resize(N_eff);
    
    material->maxwell_garnett_host=0;
    material->effective_type=get_effective_material_type();
    
    for(std::size_t i=0;i<N_eff;i++)
    {
        EffMaterialPanel *panel=effective_ctrl->get_panel(i);
        
        material->eff_mats[i]=panel->selector->get_material();
        material->eff_weights[i]=panel->weight->get_value();
        
        if(panel->host->GetValue())
            material->maxwell_garnett_host=i;
    }
}

void MaterialSelector::update_header()
{
    name_ctrl->set_value(material->name);
    
    switch(material->type)
    {
        case MatType::REAL_N:
            type_description->set_value("Constant");
            break;
        
        case MatType::CUSTOM:
            type_description->set_value("Custom");
            break;
        
        case MatType::EFFECTIVE:
            type_description->set_value("Effective");
            break;
        
        case MatType::LIBRARY:
            type_description->set_value("Library");
            break;
            
        case MatType::SCRIPT:
            type_description->set_value("Script");
            break;
        
        case MatType::USER_LIBRARY:
            type_description->set_value("User Library");
            break;
    }
}

void MaterialSelector::update_layout()
{
    mat_txt->Hide();
    index_ctrl->Hide();
    custom_editor->Hide();
    eff_panel->Hide();
    
    inspect_btn->Show();
    
    switch(material->type)
    {
        case MatType::REAL_N:
            index_ctrl->Show();
            index_ctrl->set_value(std::sqrt(material->eps_inf));
            inspect_btn->Hide();
            break;
        
        case MatType::CUSTOM:
            custom_editor->Show();
            custom_editor->rebuild_elements_list();
            break;
        
        case MatType::EFFECTIVE:
            eff_panel->Show();
            break;
        
        case MatType::LIBRARY:
            mat_txt->Show();
            mat_txt->ChangeValue(material->script_path.generic_string());
            break;
            
        case MatType::SCRIPT:
            mat_txt->Show();
            mat_txt->ChangeValue(material->script_path.generic_string());
            break;
        
        case MatType::USER_LIBRARY:
            mat_txt->Show();
            mat_txt->ChangeValue(material->script_path.generic_string());
            break;
    }
    
    Layout();
}

//void MaterialSelector::weight_change_event(wxCommandEvent &event)
//{
//    textctrl_to_T(weight_ctrl,weight);
//    
//    if(weight<0)
//    {
//        weight=1.0;
//        weight_ctrl->SetValue("1.0");
//    }
//    
//    throw_event();
//}
