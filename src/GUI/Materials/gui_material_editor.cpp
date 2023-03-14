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
#include <phys_tools.h>
#include <string_tools.h>

#include <gui_material.h>
#include <gui_material_editor.h>
#include <gui_material_editor_panels.h>

extern const Imdouble Im;

class MaterialsLibDialog: public wxDialog
{
    public:
        bool selection_ok;
        Material material;
        wxChoice *choice;
        
        MaterialsLibDialog()
            :wxDialog(nullptr,wxID_ANY,"Select a material",
                      wxGetApp().default_dialog_origin()),
             selection_ok(false)
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
            
            // Choice
            
            choice=new wxChoice(this,wxID_ANY);
            
            int Nmat=MaterialsLib::get_N_materials();
            
            for(int i=0;i<Nmat;i++)
                choice->Append(MaterialsLib::get_material_name(i).generic_string());
            
            if(Nmat>0) choice->SetSelection(0);
            
            sizer->Add(choice,wxSizerFlags().Expand().Border(wxALL,2));
            
            // Buttons
            
            wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
            
            wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
            wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
            
            ok_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_ok,this);
            cancel_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_cancel,this);
            
            btn_sizer->Add(new wxPanel(this),wxSizerFlags(1).Expand());
            btn_sizer->Add(ok_btn);
            btn_sizer->Add(cancel_btn);
            
            sizer->Add(btn_sizer,wxSizerFlags().Expand().Border(wxALL,2));
            
            SetSizerAndFit(sizer);
        }
        
        void evt_cancel(wxCommandEvent &event)
        {
            Close();
        }
        
        void evt_ok(wxCommandEvent &event)
        {
            selection_ok=true;
            
            int selection=choice->GetSelection();
            
            material=*(MaterialsLib::get_material(selection));
            Close();
        }
};

//####################
//   MaterialEditor
//####################

wxDEFINE_EVENT(EVT_MATERIAL_EDITOR_MODEL,wxCommandEvent);
wxDEFINE_EVENT(EVT_MATERIAL_EDITOR_SPECTRUM,wxCommandEvent);

MaterialEditor::MaterialEditor(wxWindow *parent,bool stand_alone)
    :wxPanel(parent),
     read_only_material(true)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    // Description
    
    wxPanel *description_panel=new wxPanel(this);
    
    wxStaticBoxSizer *description_sizer=new wxStaticBoxSizer(wxVERTICAL,description_panel,"Description");
    
    description=new wxTextCtrl(description_sizer->GetStaticBox(),
                               wxID_ANY,wxEmptyString,
                               wxDefaultPosition,wxDefaultSize,
                               wxTE_BESTWRAP|wxTE_MULTILINE|wxTE_RICH);
    description->Bind(wxEVT_TEXT,&MaterialEditor::evt_description,this);
    description->SetMinClientSize(wxSize(-1,150));
    
    description_sizer->Add(description,wxSizerFlags().Expand());
    description_panel->SetSizer(description_sizer);
    
    sizer->Add(description_panel,wxSizerFlags().Expand());
    
    // Validity Range
    
    wxPanel *validity_panel=new wxPanel(this);
    
    wxStaticBoxSizer *validity_sizer=new wxStaticBoxSizer(wxVERTICAL,validity_panel,"Validity Range");
    
    validity_min=new WavelengthSelector(validity_sizer->GetStaticBox(),"Min: ",400e-9);
    validity_max=new WavelengthSelector(validity_sizer->GetStaticBox(),"Max: ",800e-9);
    
    validity_min->Bind(EVT_WAVELENGTH_SELECTOR,&MaterialEditor::evt_validity,this);
    validity_max->Bind(EVT_WAVELENGTH_SELECTOR,&MaterialEditor::evt_validity,this);
    
    validity_sizer->Add(validity_min,wxSizerFlags().Expand());
    validity_sizer->Add(validity_max,wxSizerFlags().Expand());
    
    validity_panel->SetSizer(validity_sizer);
    
    sizer->Add(validity_panel,wxSizerFlags().Expand());
    
    // Model Addition
    
    wxBoxSizer *choice_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    model_choice=new wxChoice(this,wxID_ANY);
    
    model_choice->Append("Cauchy");
    model_choice->Append("Critical Point");
    model_choice->Append("Debye");
    model_choice->Append("Drude");
    model_choice->Append("Lorentz");
    model_choice->Append("Tabulated Data");
    model_choice->Append("Sellmeier");
    
    model_choice->SetSelection(3);
    
    choice_sizer->Add(model_choice,wxSizerFlags(1));
    
    add_btn=new wxButton(this,wxID_ANY,"Add",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    add_btn->Bind(wxEVT_BUTTON,&MaterialEditor::evt_add_model,this);
    
    choice_sizer->Add(add_btn,wxSizerFlags().Expand());
    sizer->Add(choice_sizer,wxSizerFlags().Expand());
    
    // List
    
    material_elements=new PanelsList<MatGUI::SubmodelPanel>(this);
    
    sizer->Add(material_elements,wxSizerFlags(1).Expand());
    
    if(stand_alone)
    {
        wxBoxSizer *stand_alone_sizer=new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *stand_alone_buttons=new wxBoxSizer(wxVERTICAL);
        
        wxButton *clear_btn=new wxButton(this,wxID_ANY,"Clear");
        wxButton *load_btn=new wxButton(this,wxID_ANY,"Load");
        wxButton *save_btn=new wxButton(this,wxID_ANY,"Save");
        wxButton *save_as_btn=new wxButton(this,wxID_ANY,"Save As");
        
        clear_btn->Bind(wxEVT_BUTTON,&MaterialEditor::evt_reset,this);
        load_btn->Bind(wxEVT_BUTTON,&MaterialEditor::evt_load,this);
        save_btn->Bind(wxEVT_BUTTON,&MaterialEditor::evt_save,this);
        save_as_btn->Bind(wxEVT_BUTTON,&MaterialEditor::evt_save_as,this);
        
        stand_alone_buttons->Add(clear_btn);
        stand_alone_buttons->Add(load_btn);
        stand_alone_buttons->Add(save_btn);
        stand_alone_buttons->Add(save_as_btn);
        
        stand_alone_sizer->Add(sizer,wxSizerFlags(1).Expand());
        stand_alone_sizer->Add(stand_alone_buttons,wxSizerFlags(0).Expand());
        
        description_panel->Hide();
        validity_panel->Hide();
        
        SetSizer(stand_alone_sizer);
    }
    else
    {
        SetSizer(sizer);
    }
    
    update_controls();
    rebuild_elements_list();
    
    // General bindings
    
    Bind(EVT_NAMEDTXTCTRL,&MaterialEditor::evt_model_change,this);
    Bind(EVT_WAVELENGTH_SELECTOR,&MaterialEditor::evt_model_change,this);
    
    Bind(EVT_DELETE_CAUCHY,&MaterialEditor::evt_delete_cauchy,this);
    Bind(EVT_DELETE_CRITPOINT,&MaterialEditor::evt_delete_critpoint,this);
    Bind(EVT_DELETE_DEBYE,&MaterialEditor::evt_delete_debye,this);
    Bind(EVT_DELETE_DRUDE,&MaterialEditor::evt_delete_drude,this);
    Bind(EVT_DELETE_LORENTZ,&MaterialEditor::evt_delete_lorentz,this);
    Bind(EVT_DELETE_SELLMEIER,&MaterialEditor::evt_delete_sellmeier,this);
    Bind(EVT_DELETE_SPLINE,&MaterialEditor::evt_delete_spline,this);
}

void MaterialEditor::evt_add_model(wxCommandEvent &event)
{
    int selection=model_choice->GetSelection();
    
         if(selection==0)
    {
        std::vector<double> new_cauchy(1);
        new_cauchy[0]=0;
        
        material.cauchy_coeffs.push_back(new_cauchy);
    }
    else if(selection==1)
    {
        CritpointModel new_critpoint;
        material.critpoint.push_back(new_critpoint);
    }
    else if(selection==2)
    {
        DebyeModel new_debye;
        material.debye.push_back(new_debye);
    }
    else if(selection==3)
    {
        DrudeModel new_drude;
        material.drude.push_back(new_drude);
    }
    else if(selection==4)
    {
        LorentzModel new_lorentz;
        material.lorentz.push_back(new_lorentz);
    }
    else if(selection==5)
    {
        std::vector<double> lambda(2),data_r(2),data_i(2);
        
        lambda[0]=400e-9; lambda[1]=800e-9;
        data_r[0]=1.0;    data_r[1]=1.0;
        data_i[0]=0;      data_i[1]=0;
        
        material.add_spline_data(lambda,data_r,data_i,true);
    }
    else if(selection==6)
    {
        material.sellmeier_B.push_back(0);
        material.sellmeier_C.push_back(0);
    }
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_cauchy(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.cauchy_coeffs.erase(material.cauchy_coeffs.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_critpoint(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.critpoint.erase(material.critpoint.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_debye(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.debye.erase(material.debye.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_drude(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.drude.erase(material.drude.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_lorentz(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.lorentz.erase(material.lorentz.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_sellmeier(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.sellmeier_B.erase(material.sellmeier_B.begin()+ID);
    material.sellmeier_C.erase(material.sellmeier_C.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_delete_spline(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    material.spd_lambda.erase(material.spd_lambda.begin()+ID);
    material.spd_r.erase(material.spd_r.begin()+ID);
    material.spd_i.erase(material.spd_i.begin()+ID);
    material.spd_type_index.erase(material.spd_type_index.begin()+ID);
    
    material.er_spline.erase(material.er_spline.begin()+ID);
    material.ei_spline.erase(material.ei_spline.begin()+ID);
    
    rebuild_elements_list();
}

void MaterialEditor::evt_description(wxCommandEvent &event)
{
    material.description=replace_special_characters(description->GetValue().ToStdString());
}

void MaterialEditor::evt_load(wxCommandEvent &event) { load(); }

void MaterialEditor::evt_model_change(wxCommandEvent &event) { throw_event_model(); }
void MaterialEditor::evt_reset(wxCommandEvent &event) { reset(); }
void MaterialEditor::evt_save(wxCommandEvent &event) { save(); }
void MaterialEditor::evt_save_as(wxCommandEvent &event) { save_as(); }
void MaterialEditor::evt_validity(wxCommandEvent &event) { throw_event_spectrum(); }

void MaterialEditor::load()
{
    std::vector<wxString> choices(2);
    
    choices[0]="Library";
    choices[1]="File";
    
    ChoiceDialog dialog("Load from:",choices);
    
    if(!dialog.choice_ok) return;
    
    if(dialog.choice==0)
    {
        MaterialsLibDialog dialog;
        dialog.ShowModal();
        
        if(dialog.selection_ok)
        {
            material=dialog.material;
            
            update_controls();
            rebuild_elements_list();
            
            if(PathManager::belongs_to_resources(material.script_path))
            {
                read_only_material=true;
                lock();
            }
            else
            {
                read_only_material=false;
                unlock();
            }
        }
    }
    else
    {
        wxFileName data_tmp=wxFileSelector("Please select a material file",
                                           wxString(PathManager::user_profile_materials.generic_string()),
                                           wxEmptyString,wxEmptyString,
                                           "Lua script (*.lua)|*.lua",
                                           wxFD_OPEN);
                                       
        if(data_tmp.IsOk()==false) return;
        
        std::filesystem::path new_path=data_tmp.GetFullPath().ToStdString();
        
        material.load_lua_script(new_path);
        
        update_controls();
        rebuild_elements_list();
        
        if(PathManager::belongs_to_resources(new_path))
        {
            read_only_material=true;
            lock();
        }
        else
        {
            read_only_material=false;
            unlock();
        }
    }
}

void MaterialEditor::lock()
{
    description->SetEditable(false);
    validity_min->lock();
    validity_max->lock();
    add_btn->Disable();
    
    for(std::size_t i=0;i<material_elements->get_size();i++)
        material_elements->get_panel(i)->lock();
}

void MaterialEditor::rebuild_elements_list()
{
    material_elements->clear();
    
    material_elements->add_panel<MatGUI::EpsInfPanel>(&material.eps_inf);
    
    for(std::size_t i=0;i<material.debye.size();i++)
        material_elements->add_panel<MatGUI::DebyePanel>(&material.debye[i],i);
        
    for(std::size_t i=0;i<material.drude.size();i++)
        material_elements->add_panel<MatGUI::DrudePanel>(&material.drude[i],i);
    
    for(std::size_t i=0;i<material.lorentz.size();i++)
        material_elements->add_panel<MatGUI::LorentzPanel>(&material.lorentz[i],i);
    
    for(std::size_t i=0;i<material.critpoint.size();i++)
        material_elements->add_panel<MatGUI::CritpointPanel>(&material.critpoint[i],i);
    
    for(std::size_t i=0;i<material.cauchy_coeffs.size();i++)
        material_elements->add_panel<MatGUI::CauchyPanel>(&material.cauchy_coeffs[i],i);
    
    for(std::size_t i=0;i<material.sellmeier_B.size();i++)
        material_elements->add_panel<MatGUI::SellmeierPanel>(&material.sellmeier_B[i],
                                                             &material.sellmeier_C[i],i);
    
    for(std::size_t i=0;i<material.er_spline.size();i++)
        material_elements->add_panel<MatGUI::DataPanel>(i,
                                                        &material.spd_lambda[i],
                                                        &material.spd_r[i],
                                                        &material.spd_i[i],
                                                        &material.spd_type_index[i],
                                                        &material.er_spline[i],
                                                        &material.ei_spline[i]);
    
    material_elements->Layout();
    
    throw_event_model();
}

void MaterialEditor::reset()
{
    material.reset();
    
    update_controls();
    rebuild_elements_list();
    
    read_only_material=true;
    unlock();
}

bool MaterialEditor::save()
{
    if(!read_only_material)
    {
        material.write_lua_script();
        return true;
    }
    else return save_as();
}

bool MaterialEditor::save_as()
{
    wxFileName data_tmp=wxFileSelector("Please create a new material file",
                                       wxString(PathManager::user_profile_materials.generic_string()),
                                       "temporary_material",
                                       ".lua",
                                       "Lua script (*.lua)|*.lua",
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                                       
    if(data_tmp.IsOk()==false) return false;
    
    std::filesystem::path new_path=data_tmp.GetFullPath().ToStdString();
    
    if(PathManager::belongs_to_resources(new_path))
    {
        wxMessageBox("Error: overwriting default library materials is forbidden.\nTry another file.");
        return false;
    }
    
    material.script_path=new_path;
    material.write_lua_script();
    read_only_material=false;
    
    rebuild_elements_list();
    update_controls();
    
    return true;
}

void MaterialEditor::throw_event_model()
{
    wxCommandEvent event(EVT_MATERIAL_EDITOR_MODEL);
    
    wxPostEvent(this,event);
}

void MaterialEditor::throw_event_spectrum()
{
    wxCommandEvent event(EVT_MATERIAL_EDITOR_SPECTRUM);
    
    wxPostEvent(this,event);
}

void MaterialEditor::unlock()
{
    description->SetEditable(true);
    validity_min->unlock();
    validity_max->unlock();
    add_btn->Enable();
    
    for(std::size_t i=0;i<material_elements->get_size();i++)
        material_elements->get_panel(i)->unlock();
}

void MaterialEditor::update_controls()
{
    description->ChangeValue(material.description);
    
    validity_min->set_lambda(material.lambda_valid_min);
    validity_max->set_lambda(material.lambda_valid_max);
    
    throw_event_spectrum();
}
