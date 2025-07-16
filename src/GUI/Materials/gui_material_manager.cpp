/*Copyright 2008-2025 - Loïc Le Cunff

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

extern const Imdouble Im;

//#####################
//   MaterialsManager
//#####################

enum
{
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT
};

MaterialsEditor::MaterialsEditor(wxString const &title, GUI::Material *material)
    :BaseFrame(title),
     edition_mode(true),
     lambda(401), disp_lambda(401),
     disp_real(401), disp_imag(401)
{
    
    wxBoxSizer *main_sizer=new wxBoxSizer(wxVERTICAL);
    
    SetSizer(main_sizer);
    
    // Material Selector
    
    material_path=new NamedTextCtrl<std::string>(this,"Path","",true);
    material_path->lock();
    
    if(material!=nullptr)
        material_path->set_value(material->script_path.generic_string());
    
    main_sizer->Add(material_path,wxSizerFlags().Expand());
    
    // Splitting
    
    splitter=new wxSplitterWindow(this);
    
    // - Controls
    
    ctrl_panel=new wxScrolledWindow(splitter);
    MaterialsEditor_Controls(material);
    
    // - Display
 
    wxPanel *display_panel=new wxPanel(splitter);   
    MaterialsEditor_Display(display_panel);
    
    // - Splitting wrapping up
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    main_sizer->Add(splitter,wxSizerFlags(1).Expand());
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    
    file_menu->Append(MENU_LOAD,"Library");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    // General Bindings
    
    Bind(wxEVT_MENU,&MaterialsEditor::evt_menu,this);
    Bind(EVT_MAT_SELECTOR,&MaterialsEditor::evt_material_model,this);
    Bind(EVT_MAT_SELECTOR_VALIDITY,&MaterialsEditor::evt_material_validity,this);
    
    Show();
    Maximize();
}

MaterialsEditor::~MaterialsEditor()
{
}

void MaterialsEditor::MaterialsEditor_Controls(GUI::Material *material)
{
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    material_edit_btn=new wxButton(ctrl_panel,wxID_ANY,"Edit Material");
    material_edit_btn->Bind(wxEVT_BUTTON,&MaterialsEditor::evt_edit_material,this);
    material_edit_btn->Hide();
    
    ctrl_sizer->Add(material_edit_btn,wxSizerFlags().Expand());
    
    if(material==nullptr)
        material=MaterialsLib::request_material(MatType::CUSTOM);
    
    selector=new MaterialSelector(ctrl_panel,"",true,material);
    selector->hide_buttons();
    
    ctrl_sizer->Add(selector,wxSizerFlags().Expand());
    
    // Wrapping up
    
    ctrl_panel->SetSizer(ctrl_sizer);
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
}

void MaterialsEditor::MaterialsEditor_Display(wxPanel *display_panel)
{
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *display_subsizer=new wxBoxSizer(wxHORIZONTAL);
    
    display_panel->SetSizer(display_sizer);
    
    mat_graph=new Graph(display_panel);
    sp_selector=new SpectrumSelector(display_panel,400e-9,800e-9,401);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MaterialsEditor::evt_spectrum_selector,this);
    
    wxStaticBoxSizer *index_sizer=new wxStaticBoxSizer(wxVERTICAL,display_panel,"Display");
    
    wxString disp_str[]={"Index","Permittivity"};
    
    disp_choice=new wxChoice(index_sizer->GetStaticBox(),wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_str);
    disp_choice->SetSelection(0);
    disp_choice->Bind(wxEVT_CHOICE,&MaterialsEditor::evt_display_choice,this);
    
    index_sizer->Add(disp_choice);
    
    wxButton *export_btn=new wxButton(display_panel,wxID_ANY,"Export");
    export_btn->Bind(wxEVT_BUTTON,&MaterialsEditor::evt_export,this);
    
    display_subsizer->Add(sp_selector);
    display_subsizer->Add(index_sizer);
    display_subsizer->Add(export_btn,wxSizerFlags().Expand().Border(wxALL,2));
    
    display_sizer->Add(mat_graph,wxSizerFlags(1).Expand());
    display_sizer->Add(display_subsizer);
}

void MaterialsEditor::evt_display_choice(wxCommandEvent &event)
{
    recompute_model();
}

void MaterialsEditor::evt_edit_material(wxCommandEvent &event)
{
    std::filesystem::path next_path=selector->get_material()->script_path;
    
    if(selector->get_material()->type==MatType::LIBRARY)
    {
        bool library_path=true;
        
        while(library_path)
        {
            wxFileName fname=wxFileSelector("Please select a new file to save to",
                                            wxFileSelectorPromptStr,
                                            wxEmptyString,
                                            "lua",
                                            "Aether material file (*.lua)|*.lua",
                                            wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
            
            next_path=fname.GetFullPath().ToStdString();
            
            if(next_path.empty()) return;
            
            if(PathManager::belongs_to_resources(next_path))
            {
                wxMessageBox("Error: don't overwrite default materials");
            }
            else library_path=false;
        }
        
        GUI::Material *new_material=MaterialsLib::duplicate_material(selector->get_material());
        new_material->name="";
        new_material->script_path=next_path;
        
        selector->set_material(new_material,true);
    }
    
    material_path->set_value(next_path.generic_string());
    
    selector->unlock();
    
    selector->force_edition_mode();
    edition_mode=true;
    
    material_edit_btn->Hide();
    
    ctrl_panel->Layout();
    ctrl_panel->FitInside();
}

void MaterialsEditor::evt_export(wxCommandEvent &event)
{
    std::vector<wxString> choices(3);

    choices[0]="Permittivity";
    choices[1]="Refractive index";
    choices[2]="MatLab Script";

    RadioBoxDialog dialog("Export type",choices);

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

    int Np=sp_selector->get_Np();
    
    if(dialog.choice==0 || dialog.choice==1)
    {
        for(int l=0;l<Np;l++)
        {
            Imdouble eps=selector->get_material()->get_eps(m_to_rad_Hz(lambda[l]));

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
        Material *mat=selector->get_material();

        file<<mat->get_matlab(data_tmp.GetFullPath().ToStdString());
    }
}

void MaterialsEditor::evt_material_model(wxCommandEvent &event)
{
    ctrl_panel->Layout();
    ctrl_panel->FitInside();
    
    int x,y;
    selector->GetVirtualSize(&x,&y);
    
    splitter->SetSashPosition(x+15);
    
    recompute_model();
}

void MaterialsEditor::evt_material_validity(wxCommandEvent &event)
{
    double lambda_min=selector->get_validity_min();
    double lambda_max=selector->get_validity_max();
    
    sp_selector->set_spectrum(lambda_min,lambda_max);
    
    recompute_model();
}

void MaterialsEditor::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    switch(ID)
    {
        case MENU_LOAD: evt_menu_library(); break;
        case MENU_SAVE: evt_menu_save(); break;
        case MENU_SAVE_AS: evt_menu_save_as(); break;
        case MENU_EXIT: evt_menu_exit(); break;
    }
}

void MaterialsEditor::evt_menu_exit()
{
    Close();
}

void MaterialsEditor::evt_menu_library()
{
    edition_mode=false;
    
    selector->load();
    
    material_path->set_value(selector->get_material()->script_path.generic_string());
    
    MatType type=selector->get_material()->type;
    
    if(   type==MatType::LIBRARY
       || type==MatType::USER_LIBRARY
       || type==MatType::SCRIPT)
    {
        material_edit_btn->Show();
        selector->lock();
    }
    else
    {
        edition_mode=true;
    }
    
    recompute_model();
}

void MaterialsEditor::evt_menu_save()
{
    if(edition_mode)
    {
        GUI::Material *material=selector->get_material();
        std::filesystem::path path=material->script_path;
        
        if(path.empty())
        {
            evt_menu_save_as();
        }
        else
        {
            lua_gui_material::Translator mtr(path);
            mtr.save_to_file(material);
        }
    }
}

void MaterialsEditor::evt_menu_save_as()
{
    wxFileName data_tmp=wxFileSelector("Please create a new material file",
                                       wxString(PathManager::user_profile_materials.generic_string()),
                                       "temporary_material",
                                       ".lua",
                                       "Lua script (*.lua)|*.lua",
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                                       
    if(data_tmp.IsOk()==false) return;
    
    std::filesystem::path new_path=data_tmp.GetFullPath().ToStdString();
    
    if(PathManager::belongs_to_resources(new_path))
    {
        wxMessageBox("Error: overwriting default library materials is forbidden.\nTry another file.");
        return;
    }
    
    edition_mode=true;
    
    GUI::Material *material=selector->get_material();
    
    material->script_path=new_path;
    material_path->set_value(new_path.generic_string());
    
    lua_gui_material::Translator mtr(new_path);
    mtr.save_to_file(material);
}

void MaterialsEditor::evt_spectrum_selector(wxCommandEvent &event)
{
    int Np=sp_selector->get_Np();
    
    lambda.resize(Np);
    disp_lambda.resize(Np);
    disp_real.resize(Np);
    disp_imag.resize(Np);
    
    recompute_model();
}

void MaterialsEditor::recompute_model()
{
    double lambda_min=sp_selector->get_lambda_min();
    double lambda_max=sp_selector->get_lambda_max();
    int Np=sp_selector->get_Np();
    
    Material &material=*(selector->get_material());
    
    int display_type=disp_choice->GetSelection();
        
    for(int i=0;i<Np;i++)
    {
        lambda[i]=lambda_min+(lambda_max-lambda_min)*i/(Np-1.0);
        disp_lambda[i]=m_to_nm(lambda[i]);
        
        double w=m_to_rad_Hz(lambda[i]);
        
        Imdouble eps=material.get_eps(w);
        
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
