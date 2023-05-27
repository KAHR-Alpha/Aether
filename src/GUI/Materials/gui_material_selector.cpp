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

//####################
//  MaterialSelector
//####################

MaterialSelector::MaterialSelector(wxWindow *parent,
                                   std::string name,bool no_box,
                                   GUI::Material *material_,
                                   bool (*validator)(Material*))
    :wxPanel(parent),
     material(material_),
     const_index(1.0),
     weight(1.0),
     eff_mat_1_selector(nullptr),
     eff_mat_2_selector(nullptr),
     accept_material(validator)
{
    if(material==nullptr)
        material=MaterialsLib::request_material(MatType::REAL_N);
    
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
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    name_ctrl=new NamedTextCtrl<std::string>(panel,"Name: ",material->name);
    name_ctrl->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_name,this);
    
    type_ctrl=new wxChoice(panel,wxID_ANY);
    type_ctrl->Append("Const Index");
    type_ctrl->Append("Custom");
    type_ctrl->Append("Effective");
    type_ctrl->Bind(wxEVT_CHOICE,&MaterialSelector::evt_mat_type,this);
    
    type_description=new wxStaticText(panel,wxID_ANY,"Library");
    
    header_sizer->Add(name_ctrl);
    header_sizer->Add(new wxStaticText(panel,wxID_ANY,"  Type: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    header_sizer->Add(type_ctrl);
    header_sizer->Add(type_description,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    
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

void MaterialSelector::MaterialSelector_EffPanel(wxWindow *parent)
{
    eff_panel=new wxPanel(parent);
    
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    eff_panel->SetSizer(sizer);
    
    eff_type=new wxChoice(eff_panel,wxID_ANY);
    
    eff_type->Append("Bruggeman");
    eff_type->Append("Maxwell-Garnett 1");
    eff_type->Append("Maxwell-Garnett 2");
    eff_type->Append("Looyenga");
    eff_type->Append("Weighted Sum");
    eff_type->Append("Weighted Sum Inv");
    eff_type->SetSelection(0);
    
    eff_type->Bind(wxEVT_CHOICE,&MaterialSelector::evt_effective_material,this);
    
    eff_sizer=new wxBoxSizer(wxVERTICAL);
    
    eff_weight=new NamedTextCtrl<double>(eff_panel," Weight: ",0);
    eff_weight->Bind(EVT_NAMEDTXTCTRL,&MaterialSelector::evt_effective_material,this);
    
    sizer->Add(eff_type,wxSizerFlags().Align(wxALIGN_TOP));
    sizer->Add(eff_sizer,wxSizerFlags(1));
    sizer->Add(eff_weight,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
}

void MaterialSelector::MaterialSelector_CustomPanel(wxWindow *parent)
{
    custom_editor=new MaterialEditor(parent,true);
    custom_editor->Bind(EVT_MATERIAL_EDITOR_MODEL,&MaterialSelector::evt_custom_material,this);
}

MaterialSelector::~MaterialSelector()
{
}

// - MaterialSelector member functions

void MaterialSelector::allocate_effective_materials()
{
    if(eff_mat_1_selector==nullptr)
    {
        GUI::Material *tmp=nullptr;
        
        eff_mat_1_selector=new MaterialSelector(eff_panel,"",true,tmp);
        eff_mat_2_selector=new MaterialSelector(eff_panel,"",true,tmp);
        
        eff_sizer->Add(eff_mat_1_selector);
        eff_sizer->Add(eff_mat_2_selector);
        
        eff_mat_1_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        eff_mat_2_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        
        MatEffType eff_mat_type_value=get_effective_material_type();
                
        eff_material->set_effective_material(eff_mat_type_value,
                                             *(eff_mat_1_selector->get_material()),
                                             *(eff_mat_2_selector->get_material()));
    }
}

void MaterialSelector::allocate_effective_materials(GUI::Material *eff_mat_1_,
                                                    GUI::Material *eff_mat_2_)
{
    if(eff_mat_1_selector==nullptr)
    {
        eff_mat_1_selector=new MaterialSelector(eff_panel,"",true,eff_mat_1_);
        eff_mat_2_selector=new MaterialSelector(eff_panel,"",true,eff_mat_2_);
        
        eff_sizer->Add(eff_mat_1_selector);
        eff_sizer->Add(eff_mat_2_selector);
        
        eff_mat_1_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        eff_mat_2_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        
        MatEffType eff_mat_type_value=get_effective_material_type();
                
        eff_material->set_effective_material(eff_mat_type_value,
                                             *(eff_mat_1_selector->get_material()),
                                             *(eff_mat_2_selector->get_material()));
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

MatEffType MaterialSelector::get_effective_material_type()
{
    switch(eff_type->GetSelection())
    {
        case 0: return MatEffType::MAT_EFF_BRUGGEMAN;
        case 1: return MatEffType::MAT_EFF_MG1;
        case 2: return MatEffType::MAT_EFF_MG2;
        case 3: return MatEffType::MAT_EFF_LOYENGA;
        case 4: return MatEffType::MAT_EFF_SUM;
        case 5: return MatEffType::MAT_EFF_SUM_INV;
        default: return MatEffType::MAT_EFF_BRUGGEMAN;
    }
}

void MaterialSelector::evt_effective_material(wxCommandEvent &event)
{
    MatEffType eff_type_value=get_effective_material_type();
    
    eff_material->set_effective_material(eff_type_value,
                                         *(eff_mat_1_selector->get_material()),
                                         *(eff_mat_2_selector->get_material()));
    double val=std::clamp(eff_weight->get_value(),0.0,1.0);
    
    eff_material->eff_weight=val;
    eff_weight->set_value(val);
    
    throw_event();
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
    MaterialsLibDialog dialog;
    
    if(dialog.selection_ok)
    {
        material=dialog.material;
    }
    
    update_header();
    update_layout();
    
    throw_event();
}

// TODO
void MaterialSelector::evt_mat_type(wxCommandEvent &event)
{
    int selection=type_ctrl->GetSelection();
    
    switch(selection)
    {
        case 0: material->type=MatType::REAL_N; break;
        case 1: material->type=MatType::CUSTOM; break;
        case 2: material->type=MatType::EFFECTIVE; break;
        default: material->type=MatType::REAL_N;
    }
    
    update_layout();
    throw_event();
}

void MaterialSelector::evt_name(wxCommandEvent &event)
{
    material->name=name_ctrl->get_value();
}

Imdouble MaterialSelector::get_eps(double w) { return material->get_eps(w); }

GUI::Material* MaterialSelector::get_material()
{
    return material;
}

double MaterialSelector::get_weight() { return weight; }

void MaterialSelector::throw_event()
{
    wxCommandEvent event(EVT_MAT_SELECTOR);
    wxPostEvent(this,event);
}

void MaterialSelector::update_header()
{
    name_ctrl->set_value(material->name);
    
    switch(material->type)
    {
        case MatType::REAL_N:
            type_ctrl->Show();
            type_ctrl->SetSelection(0);
            type_description->Hide();
            break;
        
        case MatType::CUSTOM:
            type_ctrl->Show();
            type_ctrl->SetSelection(1);
            type_description->Hide();
            break;
        
        case MatType::EFFECTIVE:
            type_ctrl->Show();
            type_ctrl->SetSelection(2);
            type_description->Hide();
            break;
        
        case MatType::LIBRARY:
            type_ctrl->Hide();
            type_description->Show();
            type_description->SetLabel("Library");
            break;
            
        case MatType::SCRIPT:
            type_ctrl->Hide();
            type_description->Show();
            type_description->SetLabel("Script");
            break;
        
        case MatType::USER_LIBRARY:
            type_ctrl->Hide();
            type_description->Show();
            type_description->SetLabel("User Library");
            break;
    }
}

void MaterialSelector::update_layout()
{
    type_ctrl->Hide();
    type_description->Hide();
    mat_txt->Hide();
    index_ctrl->Hide();
    custom_editor->Hide();
    eff_panel->Hide();
    
    inspect_btn->Show();
    
    switch(material->type)
    {
        case MatType::REAL_N:
            type_ctrl->Show();
            index_ctrl->Show();
            inspect_btn->Hide();
            break;
        
        case MatType::CUSTOM:
            type_ctrl->Show();
            custom_editor->Show();
            break;
        
        case MatType::EFFECTIVE:
            type_ctrl->Show();
            eff_panel->Show();
            break;
        
        case MatType::LIBRARY:
            type_description->Show();
            mat_txt->Show();
            mat_txt->ChangeValue(material->script_path.generic_string());
            break;
            
        case MatType::SCRIPT:
            type_description->Show();
            mat_txt->Show();
            mat_txt->ChangeValue(material->script_path.generic_string());
            break;
        
        case MatType::USER_LIBRARY:
            type_description->Show();
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
