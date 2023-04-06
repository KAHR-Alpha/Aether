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

MaterialSelector::MaterialSelector(wxWindow *parent,std::string name,bool no_box,
                                   bool (*validator)(Material*))
    :wxPanel(parent),
     mat_type(MatType::LIBRARY),
     const_index(1.0),
     weight(1.0),
     eff_mat_1_selector(nullptr),
     eff_mat_2_selector(nullptr),
     accept_material(validator)
{
    wxSizer *sizer=nullptr;
    
    if(no_box) sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    wxString choices[]={"Library","Real Index","Script","Real Eps","Effective","Custom"};
    
    mat_type_ctrl=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,6,choices);
    mat_type_ctrl->SetSelection(0);
    
    MaterialSelector_EffPanel(this);
    MaterialSelector_CustomPanel(this);
    
    mat_txt=new wxTextCtrl(this,wxID_ANY,"1.0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    load_btn=new wxButton(this,wxID_ANY,"Load");
    inspect_btn=new wxButton(this,wxID_ANY,"Inspect");
    
    sizer->Add(mat_type_ctrl);
    sizer->Add(mat_txt,wxSizerFlags(1));
    sizer->Add(eff_panel,wxSizerFlags(1));
    sizer->Add(custom_editor,wxSizerFlags(1));
    sizer->Add(load_btn,wxSizerFlags().Expand());
    sizer->Add(inspect_btn,wxSizerFlags().Expand());
    
    inspect_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_inspect,this);
    load_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_load,this);
    mat_txt->Bind(wxEVT_TEXT_ENTER,&MaterialSelector::const_index_event,this);
    mat_txt->Bind(wxEVT_KILL_FOCUS,&MaterialSelector::evt_const_index_focus,this);
    mat_type_ctrl->Bind(wxEVT_CHOICE,&MaterialSelector::evt_mat_type,this);
    
    layout_library();
    
    SetSizer(sizer);
}

MaterialSelector::MaterialSelector(wxWindow *parent,std::string name,bool no_box,Material const &material,
                                   bool (*validator)(Material*))
    :wxPanel(parent),
     mat_type(MatType::LIBRARY),
     const_index(1.0),
     weight(1.0),
     eff_mat_1_selector(nullptr),
     eff_mat_2_selector(nullptr),
     accept_material(validator)
{
    wxSizer *sizer=nullptr;
    
    if(no_box) sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    wxString choices[]={"Library","Real Index","Script","Real Eps","Effective","Custom"};
    
    mat_type_ctrl=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,6,choices);
    mat_type_ctrl->SetSelection(0);
    
    MaterialSelector_EffPanel(this);
    MaterialSelector_CustomPanel(this);
    
    mat_txt=new wxTextCtrl(this,wxID_ANY,"1.0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    load_btn=new wxButton(this,wxID_ANY,"Load");
    inspect_btn=new wxButton(this,wxID_ANY,"Inspect");
    
    sizer->Add(mat_type_ctrl);
    sizer->Add(mat_txt,wxSizerFlags(1));
    sizer->Add(eff_panel,wxSizerFlags(1));
    sizer->Add(custom_editor,wxSizerFlags(1));
    sizer->Add(load_btn,wxSizerFlags().Expand());
    sizer->Add(inspect_btn,wxSizerFlags().Expand());
    
    inspect_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_inspect,this);
    load_btn->Bind(wxEVT_BUTTON,&MaterialSelector::evt_load,this);
    mat_txt->Bind(wxEVT_TEXT_ENTER,&MaterialSelector::const_index_event,this);
    mat_txt->Bind(wxEVT_KILL_FOCUS,&MaterialSelector::evt_const_index_focus,this);
    mat_type_ctrl->Bind(wxEVT_CHOICE,&MaterialSelector::evt_mat_type,this);
    
    Material tmp_material(material);
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
    }
    
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
        eff_mat_1_selector=new MaterialSelector(eff_panel,"",true);
        eff_mat_2_selector=new MaterialSelector(eff_panel,"",true);
        
        eff_sizer->Add(eff_mat_1_selector);
        eff_sizer->Add(eff_mat_2_selector);
        
        eff_mat_1_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        eff_mat_2_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        
        int eff_mat_type_value=get_effective_material_type();
                
        eff_material.set_effective_material(eff_mat_type_value,
                                            eff_mat_1_selector->get_material(),
                                            eff_mat_2_selector->get_material());
    }
}

void MaterialSelector::allocate_effective_materials(Material const &eff_mat_1_,
                                                    Material const &eff_mat_2_)
{
    if(eff_mat_1_selector==nullptr)
    {
        eff_mat_1_selector=new MaterialSelector(eff_panel,"",true,eff_mat_1_);
        eff_mat_2_selector=new MaterialSelector(eff_panel,"",true,eff_mat_2_);
        
        eff_sizer->Add(eff_mat_1_selector);
        eff_sizer->Add(eff_mat_2_selector);
        
        eff_mat_1_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        eff_mat_2_selector->Bind(EVT_MAT_SELECTOR,&MaterialSelector::evt_effective_material,this);
        
        int eff_mat_type_value=get_effective_material_type();
                
        eff_material.set_effective_material(eff_mat_type_value,
                                            eff_mat_1_selector->get_material(),
                                            eff_mat_2_selector->get_material());
    }
}

void MaterialSelector::const_index_event(wxCommandEvent &event)
{
         if(mat_type==MatType::REAL_N) textctrl_to_T(mat_txt,const_index);
    else if(mat_type==MatType::REAL_EPS)
    {
        double tmp=1.0;
        textctrl_to_T(mat_txt,tmp);
        
        const_index=std::sqrt(tmp);
    }
    
    if(const_index<1.0)
    {
        const_index=1.0;
        mat_txt->SetValue("1.0");
    }
    
    throw_event();
}

void MaterialSelector::evt_const_index_focus(wxFocusEvent &event)
{
         if(mat_type==MatType::REAL_N) textctrl_to_T(mat_txt,const_index);
    else if(mat_type==MatType::REAL_EPS)
    {
        double tmp=1.0;
        textctrl_to_T(mat_txt,tmp);
        
        const_index=std::sqrt(tmp);
    }
    
    if(const_index<1.0)
    {
        const_index=1.0;
        mat_txt->SetValue("1.0");
    }
    
    throw_event();
    
    event.Skip();
}

void MaterialSelector::evt_custom_material(wxCommandEvent &event)
{
    throw_event();
}

int MaterialSelector::get_effective_material_type()
{
    switch(eff_type->GetSelection())
    {
        case 0: return MAT_EFF_BRUGGEMAN;
        case 1: return MAT_EFF_MG1;
        case 2: return MAT_EFF_MG2;
        case 3: return MAT_EFF_LOYENGA;
        case 4: return MAT_EFF_SUM;
        case 5: return MAT_EFF_SUM_INV;
        default: return MAT_EFF_BRUGGEMAN;
    }
}

void MaterialSelector::evt_effective_material(wxCommandEvent &event)
{
    int eff_type_value=get_effective_material_type();
    
    eff_material.set_effective_material(eff_type_value,
                                        eff_mat_1_selector->get_material(),
                                        eff_mat_2_selector->get_material());
    double val=std::clamp(eff_weight->get_value(),0.0,1.0);
    
    eff_material.eff_weight=val;
    eff_weight->set_value(val);
    
    throw_event();
}

void MaterialSelector::evt_inspect(wxCommandEvent &event)
{
    double lambda_min=get_lambda_validity_min();
    double lambda_max=get_lambda_validity_max();
    
    MaterialExplorer *mat_expl=new MaterialExplorer(lambda_min,lambda_max,401,this);
    null_function(mat_expl);
}

void MaterialSelector::evt_load(wxCommandEvent &event)
{
    wxFileName fname;
    fname=wxFileSelector("Load material script",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         wxEmptyString,
                         wxFileSelectorDefaultWildcardStr,
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if(fname.IsOk()==false) return;
    
//    wxMessageDialog path_dialog(0,"Make the path relative?","Path",wxYES_NO);
//    int dialog_choice=path_dialog.ShowModal();
//    
//    if(dialog_choice==wxID_YES) fname.MakeRelativeTo();
    
    fname.MakeRelativeTo();
    
    std::filesystem::path tmp_script=fname.GetFullPath().ToStdString();
    Material tmp_material(tmp_script);
    
    if(accept_material(&tmp_material))
    {
        script=tmp_script;
        mat_txt->SetValue(fname.GetFullPath());
        
        script_model=tmp_material;
    }
    else
    {
        wxMessageBox("Incompatible material in this configuration!","Error");
    }
    
    throw_event();
}

void MaterialSelector::evt_mat_type(wxCommandEvent &event)
{
    int selection=mat_type_ctrl->GetSelection();
    
         if(selection==0) mat_type=MatType::LIBRARY;
    else if(selection==1) mat_type=MatType::REAL_N;
    else if(selection==2) mat_type=MatType::SCRIPT;
    else if(selection==3) mat_type=MatType::REAL_EPS;
    else if(selection==4) mat_type=MatType::EFFECTIVE;
    else if(selection==5) mat_type=MatType::CUSTOM;
    
    if(mat_type==MatType::REAL_N)
    {
        layout_const();
        
        std::stringstream strm;
        strm<<const_index;
        mat_txt->SetValue(strm.str());
    }
    else if(mat_type==MatType::REAL_EPS)
    {
        layout_const();
        
        std::stringstream strm;
        strm<<const_index*const_index;
        mat_txt->SetValue(strm.str());
    }
    else if(mat_type==MatType::LIBRARY) layout_library();
    //else if(mat_type==MatType::SCRIPT) layout_script();
    else if(mat_type==MatType::EFFECTIVE)
    {
        allocate_effective_materials();
        layout_effective();
    }
    else if(mat_type==MatType::CUSTOM)
    {
        layout_custom();
    }
    
    throw_event();
}

Imdouble MaterialSelector::get_eps(double w)
{
    if(mat_type==MatType::REAL_N || mat_type==MatType::REAL_EPS) return const_index*const_index;
    else if(mat_type==MatType::LIBRARY)
    {
        return library_model.get_eps(w);
    }
    else if(mat_type==MatType::SCRIPT)
    {
        if(script.empty()) return 1.0;
        
        return script_model.get_eps(w);
    }
    else if(mat_type==MatType::EFFECTIVE)
    {
        return eff_material.get_eps(w);
    }
    
    return 1.0;
}

double MaterialSelector::get_lambda_validity_min()
{
    double lambda_min=400e-9;
    
    if(mat_type==MatType::LIBRARY) lambda_min=library_model.lambda_valid_min;
    else if(mat_type==MatType::SCRIPT) lambda_min=script_model.lambda_valid_min;
    
    return lambda_min;
}

double MaterialSelector::get_lambda_validity_max()
{
    double lambda_max=1000e-9;
    
    if(mat_type==MatType::LIBRARY) lambda_max=library_model.lambda_valid_max;
    else if(mat_type==MatType::SCRIPT) lambda_max=script_model.lambda_valid_max;
    
    return lambda_max;
}

std::string MaterialSelector::get_lua()
{
    if(mat_type==MatType::LIBRARY)
    {
        std::string tmp_str;
//        std::string ref_item=library_list_ctrl->GetString(library_list_ctrl->GetSelection()).ToStdString();
//        std::ifstream file(ref_item,std::ios::in|std::ios::binary);
//        
//        std::getline(file,tmp_str,'\0');
        
        return tmp_str;
    }
    else if(mat_type==MatType::REAL_N || mat_type==MatType::REAL_EPS)
    {
        std::stringstream strm;
        strm<<"set_constant()"<<std::endl;
        strm<<"index_infty("<<const_index<<")";
        return strm.str();
    }
    else
    {
        std::string tmp_str;
        std::ifstream file(script,std::ios::in|std::ios::binary);
        
        std::getline(file,tmp_str,'\0');
        return tmp_str;
    }
    
    return "";
}

Material MaterialSelector::get_material()
{
    Material out;
    
         if(mat_type==MatType::REAL_N) out.set_const_n(const_index);
    else if(mat_type==MatType::REAL_EPS) out.set_const_eps(const_index*const_index);
    else if(mat_type==MatType::LIBRARY) out=library_model;
    else if(mat_type==MatType::SCRIPT) out=script_model;
    else if(mat_type==MatType::EFFECTIVE) out=eff_material;
    
    return out;
}

wxString MaterialSelector::get_name()
{
    wxString out;
    
         if(mat_type==MatType::REAL_N)
    {
        out<<const_index;
    }
    else if(mat_type==MatType::REAL_EPS)
    {
        out<<const_index*const_index;
    }
    else if(mat_type==MatType::LIBRARY)
    {
        wxFileName w_script;
        //w_script.SetFullName(library_list_ctrl->GetString(library_list_ctrl->GetSelection()));
        out=w_script.GetName();
    }
    else if(mat_type==MatType::SCRIPT)
    {
        wxFileName w_script;
        w_script.SetFullName(wxString(script));
        out=w_script.GetName();
    }
    else if(mat_type==MatType::EFFECTIVE)
    {
        wxString w_script_1,w_script_2;
        
        w_script_1=eff_mat_1_selector->get_name();
        w_script_2=eff_mat_2_selector->get_name();
        
        int eff_mat_type=get_effective_material_type();
        
        switch(eff_mat_type)
        {
            case MAT_EFF_BRUGGEMAN: out="Brugg"; break;
            case MAT_EFF_MG1: out="MG1"; break;
            case MAT_EFF_MG2: out="MG2"; break;
            case MAT_EFF_LOYENGA: out="Loy"; break;
            case MAT_EFF_SUM: out="Sum"; break;
            case MAT_EFF_SUM_INV: out="ISum"; break;
        }
        
        out=out+" | "+w_script_1+" | "+w_script_2;
    }
    
    return out;
}

wxString MaterialSelector::get_title()
{
    wxString out;
    std::stringstream strm;
    
         if(mat_type==MatType::REAL_N)
    {
        strm<<const_index;
        out="Const refractive index: ";
        out.Append(strm.str());
    }
    else if(mat_type==MatType::REAL_EPS)
    {
        strm<<const_index*const_index;
        out="Const permittivity: ";
        out.Append(strm.str());
    }
    else if(mat_type==MatType::LIBRARY)
    {
        out="Library material: ";
        //out.Append(library_list_ctrl->GetString(library_list_ctrl->GetSelection()));
    }
    else if(mat_type==MatType::SCRIPT)
    {
        out="Script: ";
        out.Append(mat_txt->GetValue());
    }
    
    return out;
}

MatType MaterialSelector::get_type() { return mat_type; }

double MaterialSelector::get_weight() { return weight; }

void MaterialSelector::layout_const()
{
    mat_txt->Enable();
    mat_txt->Show();
    
    load_btn->Hide();
    inspect_btn->Hide();
    
    eff_panel->Hide();
    
    custom_editor->Hide();
    
    Layout();
}

void MaterialSelector::layout_custom()
{
    mat_txt->Hide();
    
    load_btn->Hide();
    inspect_btn->Hide();
    
    eff_panel->Hide();
    
    custom_editor->Show();
    
    Layout();
}

void MaterialSelector::layout_effective()
{
    mat_txt->Hide();
    
    load_btn->Hide();
    inspect_btn->Hide();
    
    mat_txt->Disable();
    load_btn->Disable();
    inspect_btn->Enable();
    
    eff_panel->Show();
    
    custom_editor->Hide();
    
    Layout();
}

void MaterialSelector::layout_library()
{
    mat_txt->Hide();
    
    load_btn->Hide();
    inspect_btn->Show();
    
    mat_txt->Show();
    load_btn->Show();
    inspect_btn->Show();
    
    eff_panel->Hide();
    
    custom_editor->Hide();
    
    Layout();
}

void MaterialSelector::set_const_model(double n)
{
    mat_type=MatType::REAL_N;
    
    const_index=n;
    mat_type_ctrl->SetSelection(1);
    layout_const();
    
    mat_txt->ChangeValue(std::to_string(const_index));
}

void MaterialSelector::operator = (MaterialSelector const &selector)
{
    mat_type=selector.mat_type;
    const_index=selector.const_index;
    weight=selector.weight;
    
    script=selector.script;
    
    script_model=selector.script_model;
    library_model=selector.library_model;
    
    mat_type_ctrl->SetSelection(selector.mat_type_ctrl->GetSelection());
    
    mat_txt->SetValue(selector.mat_txt->GetValue());
    
         if(mat_type==MatType::REAL_N || mat_type==MatType::REAL_EPS) layout_const();
    else if(mat_type==MatType::LIBRARY) layout_library();
    //else if(mat_type==MatType::SCRIPT) layout_script();
}

void MaterialSelector::throw_event()
{
    wxCommandEvent event(EVT_MAT_SELECTOR);
    wxPostEvent(this,event);
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
