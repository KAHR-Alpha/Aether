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
#include <string_tools.h>

#include <gui_material.h>

wxDEFINE_EVENT(EVT_MAT_SELECTOR,wxCommandEvent);
wxDEFINE_EVENT(EVT_MAT_SELECTOR_VALIDITY,wxCommandEvent);

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
    selector->SetMinClientSize(wxSize(250,-1));
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

void EffMaterialPanel::lock()
{
    PanelsListBase::lock();
    
    selector->lock();
    weight->lock();
    host->Disable();
}

void EffMaterialPanel::show_host()
{
    host->Show();
}

void EffMaterialPanel::unlock()
{
    PanelsListBase::unlock();
    
    selector->unlock();
    weight->unlock();
    host->Enable();
}

//####################
//  MaterialSelector
//####################

MaterialSelector::MaterialSelector(wxWindow *parent,
                                   std::string name,bool no_box,
                                   GUI::Material *material_,
                                   bool (*validator)(Material*))
    :wxPanel(parent),
     parent_selector(nullptr),
     material(material_),
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
    
    // Buttons
    
    buttons_panel=new wxPanel(panel);
    
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxButton *lib_btn=new wxButton(buttons_panel,wxID_ANY,"Library");
    lib_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_library,this);
    
    inspect_btn=new wxButton(buttons_panel,wxID_ANY,"Inspect");
    inspect_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_inspect,this);
    
    buttons_sizer->Add(lib_btn,wxSizerFlags().Border(wxRIGHT,3));
    buttons_sizer->Add(inspect_btn,wxSizerFlags().Border(wxRIGHT,3));
    
    buttons_panel->SetSizer(buttons_sizer);
    
    sizer->Add(buttons_panel);
    
    // Central Panel
    
    wxBoxSizer *central_sizer=new wxBoxSizer(wxVERTICAL);
    sizer->Add(central_sizer,wxSizerFlags(1));
    
    // - Header
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxVERTICAL);
    
    type_description=new NamedTextCtrl<std::string>(panel,"Type: ","");
    type_description->lock();
    
    name_ctrl=new NamedTextCtrl<std::string>(panel,"Name: ",material->name);
    name_ctrl->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_name,this);
    
    header_sizer->Add(type_description,wxSizerFlags().Expand());
    header_sizer->Add(name_ctrl,wxSizerFlags().Expand());
    
    central_sizer->Add(header_sizer,wxSizerFlags().Expand());
    
    // - Description
    
    description_panel=new wxPanel(panel);
    
    wxStaticBoxSizer *description_sizer=new wxStaticBoxSizer(wxVERTICAL,description_panel,"Description");
    
    description=new wxTextCtrl(description_sizer->GetStaticBox(),
                               wxID_ANY,wxEmptyString,
                               wxDefaultPosition,wxDefaultSize,
                               wxTE_BESTWRAP|wxTE_MULTILINE|wxTE_RICH);
    description->Bind(wxEVT_TEXT,&MaterialSelector::evt_description,this);
    description->SetMinClientSize(wxSize(-1,150));
    
    description_sizer->Add(description,wxSizerFlags().Expand());
    description_panel->SetSizer(description_sizer);
    
    central_sizer->Add(description_panel,wxSizerFlags().Expand());
    
    // Validity Range
    
    validity_panel=new wxPanel(panel);
    
    wxStaticBoxSizer *validity_sizer=new wxStaticBoxSizer(wxVERTICAL,validity_panel,"Validity Range");
    
    validity_min=new WavelengthSelector(validity_sizer->GetStaticBox(),"Min: ",400e-9);
    validity_max=new WavelengthSelector(validity_sizer->GetStaticBox(),"Max: ",800e-9);
    
    validity_min->Bind(EVT_WAVELENGTH_SELECTOR,&MaterialSelector::evt_validity,this);
    validity_max->Bind(EVT_WAVELENGTH_SELECTOR,&MaterialSelector::evt_validity,this);
    
    validity_sizer->Add(validity_min,wxSizerFlags().Expand());
    validity_sizer->Add(validity_max,wxSizerFlags().Expand());
    
    validity_panel->SetSizer(validity_sizer);
    
    central_sizer->Add(validity_panel,wxSizerFlags().Expand());
    
    // - Material
    
    wxBoxSizer *material_sizer=new wxBoxSizer(wxVERTICAL);
    
    mat_txt=new wxTextCtrl(panel,wxID_ANY,"");
    mat_txt->SetEditable(false);
    mat_txt->SetMinClientSize(wxSize(250,-1));
    
    material_sizer->Add(mat_txt,wxSizerFlags().Expand());
    
    index_ctrl=new NamedTextCtrl(panel,"Refractive index: ",std::sqrt(material->eps_inf));
    index_ctrl->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_const_index,this);
    material_sizer->Add(index_ctrl,wxSizerFlags().Expand());
    
    MaterialSelector_CustomPanel(panel);
    MaterialSelector_EffPanel(panel);
    material_sizer->Add(custom_editor,wxSizerFlags().Expand());
    material_sizer->Add(eff_panel,wxSizerFlags().Expand());
    
    central_sizer->Add(material_sizer,wxSizerFlags().Expand());
    
    // Closing
        
    update_header();
    update_layout();
        
    SetSizer(sizer);
}

void MaterialSelector::MaterialSelector_CustomPanel(wxWindow *parent)
{
    custom_editor=new MaterialEditorPanel(parent,material,true);
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
    
    sizer->Add(header_sizer,wxSizerFlags().Expand());
    
    // Materials
    
    wxStaticBoxSizer *components_sizer=new wxStaticBoxSizer(wxHORIZONTAL,eff_panel,"Components");
    
    effective_ctrl=new PanelsList<EffMaterialPanel>(components_sizer->GetStaticBox());
    add_mat_btn=new wxButton(components_sizer->GetStaticBox(),wxID_ANY,"Add");
    add_mat_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_add_effective_component,this);
    
    components_sizer->Add(effective_ctrl);
    components_sizer->Add(add_mat_btn);
    
    sizer->Add(components_sizer,wxSizerFlags().Expand());    
    
    Bind(EVT_GARNETT_HOST,&MaterialSelector::evt_effective_host,this);
    Bind(EVT_EFFECTIVE_MATERIAL,&MaterialSelector::evt_effective_component,this);
    
    // Pre-existing material reload
    
    if(material->is_effective_material)
        rebuild_effective_panel();
}

MaterialSelector::~MaterialSelector()
{
}

// - MaterialSelector member functions

void MaterialSelector::add_effective_component(GUI::Material *effective_component,double weight)
{
    if(effective_component==nullptr)
        effective_component=MaterialsLib::request_material(MatType::REAL_N);
    
    effective_ctrl->add_panel<EffMaterialPanel>(effective_component,weight);
    
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

void MaterialSelector::evt_add_effective_component(wxCommandEvent &event)
{
    EffectiveModel model=get_effective_material_type();
    
    if(model!=EffectiveModel::BRUGGEMAN && model!=EffectiveModel::LOOYENGA) // Restricted to two submaterials
    {
        add_effective_component();
        
        rebuild_effective_material();
            
        throw_event();
    }
}

void MaterialSelector::evt_description(wxCommandEvent &event)
{
    material->description=replace_special_characters(description->GetValue().ToStdString());
}

void MaterialSelector::evt_effective_type(wxCommandEvent &event)
{
    EffectiveModel model=get_effective_material_type();
    
    std::size_t Nm=effective_ctrl->get_size();
    
    if(model==EffectiveModel::MAXWELL_GARNETT)
    {
        bool has_host=false;
        
        for(std::size_t i=0;i<Nm;i++)
        {
            EffMaterialPanel *panel=effective_ctrl->get_panel(i);
            
            panel->show_host();
            has_host|=panel->host->GetValue();
        }
        
        if(!has_host && Nm>0)
        {
            effective_ctrl->get_panel(0)->host->SetValue(true);
        }
    }
    else
    {
        for(std::size_t i=0;i<Nm;i++)
            effective_ctrl->get_panel(i)->hide_host();
    }
    
    if(Nm>2 && (model==EffectiveModel::BRUGGEMAN || model==EffectiveModel::LOOYENGA))
    {
        for(unsigned int i=0;i<Nm-2;i++)
        {
            effective_ctrl->pop_back();
        }
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

void MaterialSelector::hide_buttons()
{
    buttons_panel->Hide();
    Layout();
}

void MaterialSelector::hide_description()
{
    description_panel->Hide();
    Layout();
}

void MaterialSelector::hide_validity()
{
    validity_panel->Hide();
    Layout();
}

void MaterialSelector::evt_inspect(wxCommandEvent &event)
{
    MaterialsEditor *mat_edit=new MaterialsEditor("Materials Editor",material);
    null_function(mat_edit);
}

void MaterialSelector::evt_library(wxCommandEvent &event)
{
    load();
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

void MaterialSelector::evt_validity(wxCommandEvent &event) { throw_event_validity(); }

void MaterialSelector::force_edition_mode()
{
    if(material->is_effective_material)
    {
        material->type=MatType::EFFECTIVE;
        
        rebuild_effective_panel();
        eff_panel->Show();
    }
    else
    {
        if(material->type==MatType::REAL_N)
            material->type=MatType::CUSTOM;
        
        custom_editor->Show();
        custom_editor->rebuild_elements_list();
    }
    
    update_header();
    
    Layout();
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

GUI::Material* MaterialSelector::get_material()
{
    return material;
}

double MaterialSelector::get_validity_max()
{
    return validity_max->get_lambda();
}

double MaterialSelector::get_validity_min()
{
    return validity_min->get_lambda();
}

bool MaterialSelector::load()
{
    wxWindow *requester=parent_selector;
    if(requester==nullptr) requester=this;
    
    MaterialsLibDialog dialog(requester);
    
    if(dialog.selection_ok)
    {
        set_material(dialog.material,dialog.new_material);
    }
    
    return dialog.new_material;
}

void MaterialSelector::lock()
{
    description->SetEditable(false);
    description->SetBackgroundColour(wxColour(240,240,240));
    
    validity_min->lock();
    validity_max->lock();
    
    index_ctrl->lock();
    
    add_mat_btn->Disable();
    
    for(std::size_t i=0;i<effective_ctrl->get_size();i++)
    {
        effective_ctrl->get_panel(i)->lock();
    }
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

void MaterialSelector::rebuild_effective_panel()
{
    effective_ctrl->clear();
    
    switch(material->effective_type)
    {
        case EffectiveModel::BRUGGEMAN:
            eff_type->SetSelection(0);
            break;
        case EffectiveModel::LOOYENGA:
            eff_type->SetSelection(1);
            break;
        case EffectiveModel::MAXWELL_GARNETT:
            eff_type->SetSelection(2);
            break;
        case EffectiveModel::SUM:
            eff_type->SetSelection(3);
            break;
        case EffectiveModel::SUM_INV:
            eff_type->SetSelection(4);
            break;
    }
    
    std::size_t Nm=material->eff_mats.size();
    
    for(std::size_t i=0;i<Nm;i++)
    {
        add_effective_component(dynamic_cast<GUI::Material*>(material->eff_mats[i]),
                                material->eff_weights[i]);
    }
    
    for(std::size_t i=0;i<Nm;i++)
    {
        EffMaterialPanel *panel=effective_ctrl->get_panel(i);
        
        bool is_host=static_cast<int>(i)==material->maxwell_garnett_host;
        
        panel->host->SetValue(is_host);
        
        if(material->effective_type==EffectiveModel::MAXWELL_GARNETT)
            panel->show_host();
    }
}

void MaterialSelector::set_material(GUI::Material *material_, bool new_material)
{
    material=material_;
    custom_editor->material=material;
    
    if(new_material && material->type==MatType::EFFECTIVE)
    {
        add_effective_component();
        add_effective_component();
        
        rebuild_effective_material();
    }        

    update_header();
    update_layout();
    
    throw_event();
}

void MaterialSelector::throw_event()
{
    wxCommandEvent event(EVT_MAT_SELECTOR);
    wxPostEvent(this,event);
}

void MaterialSelector::throw_event_validity()
{
    wxCommandEvent event(EVT_MAT_SELECTOR_VALIDITY);
    wxPostEvent(this,event);
}

void MaterialSelector::unlock()
{
    description->SetEditable(true);
    description->SetBackgroundColour(wxColour(255,255,255));
    
    validity_min->unlock();
    validity_max->unlock();
    
    index_ctrl->unlock();
    
    add_mat_btn->Enable();
    
    for(std::size_t i=0;i<effective_ctrl->get_size();i++)
    {
        effective_ctrl->get_panel(i)->unlock();
    }
}

void MaterialSelector::update_header()
{
    name_ctrl->set_value(material->name);
    description->ChangeValue(material->description);
    
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
