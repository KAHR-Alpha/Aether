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

#include <filehdl.h>
#include <phys_tools.h>
#include <string_tools.h>

#include <gui_material.h>

#include <wx/splitter.h>

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

//#####################
//   MaterialManager
//#####################

enum
{
    MENU_NEW,
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT
};

MaterialManager::MaterialManager(wxString const &title)
    :BaseFrame(title),
     Np(401),
     lambda_min(400e-9), lambda_max(800e-9),
     library_material(false),
     lambda(Np), disp_lambda(Np),
     disp_real(Np), disp_imag(Np)
{
    wxBoxSizer *main_sizer=new wxBoxSizer(wxVERTICAL);
    
    SetSizer(main_sizer);
    
    // Material Selector
    
    material_path=new NamedTextCtrl<std::string>(this,"Material","",true);
    material_path->lock();
    
    main_sizer->Add(material_path,wxSizerFlags().Expand());
    
    // Splitting
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    // - Controls
    
    ctrl_panel=new wxScrolledWindow(splitter);
//    ctrl_panel=new wxPanel(splitter);
    MaterialManager_Controls();
    
    // - Display
 
    wxPanel *display_panel=new wxPanel(splitter);   
    MaterialManager_Display(display_panel);
    
    // - Splitting wrapping up
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    main_sizer->Add(splitter,wxSizerFlags(1).Expand());
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    
    file_menu->Append(MENU_NEW,"New");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    // General Bindings
    
    Bind(wxEVT_MENU,&MaterialManager::evt_menu,this);
    Bind(EVT_MATERIAL_EDITOR_MODEL,&MaterialManager::evt_material_editor_model,this);
    Bind(EVT_MATERIAL_EDITOR_SPECTRUM,&MaterialManager::evt_material_editor_spectrum,this);
    
    Show();
    Maximize();
}

void MaterialManager::MaterialManager_Controls()
{
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    editor=new MaterialEditor(ctrl_panel);
    
    ctrl_sizer->Add(editor,wxSizerFlags(1).Expand());
    
    // Wrapping up
    
    ctrl_panel->SetSizer(ctrl_sizer);
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
}

void MaterialManager::MaterialManager_Display(wxPanel *display_panel)
{
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *display_subsizer=new wxBoxSizer(wxHORIZONTAL);
    
    display_panel->SetSizer(display_sizer);
    
    mat_graph=new Graph(display_panel);
    sp_selector=new SpectrumSelector(display_panel,lambda_min,lambda_max,Np);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MaterialManager::evt_spectrum_selector,this);
    
    wxStaticBoxSizer *index_sizer=new wxStaticBoxSizer(wxVERTICAL,display_panel,"Display");
    
    wxString disp_str[]={"Index","Permittivity"};
    
    disp_choice=new wxChoice(index_sizer->GetStaticBox(),wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_str);
    disp_choice->SetSelection(0);
    disp_choice->Bind(wxEVT_CHOICE,&MaterialManager::evt_display_choice,this);
    
    index_sizer->Add(disp_choice);
    
    display_subsizer->Add(sp_selector);
    display_subsizer->Add(index_sizer);
    
    display_sizer->Add(mat_graph,wxSizerFlags(1).Expand());
    display_sizer->Add(display_subsizer);
}

void MaterialManager::evt_display_choice(wxCommandEvent &event)
{
    recompute_model();
}

void MaterialManager::evt_material_editor_model(wxCommandEvent &event)
{
    ctrl_panel->Layout();
    ctrl_panel->FitInside();
    
    recompute_model();
}

void MaterialManager::evt_material_editor_spectrum(wxCommandEvent &event)
{
    sp_selector->set_spectrum(editor->validity_min->get_lambda(),
                              editor->validity_max->get_lambda());
    
    lambda_min=sp_selector->get_lambda_min();
    lambda_max=sp_selector->get_lambda_max();
    
    recompute_model();
}

void MaterialManager::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    switch(ID)
    {
        case MENU_NEW: evt_menu_new(); break;
        case MENU_LOAD: evt_menu_load(); break;
        case MENU_SAVE: evt_menu_save(); break;
        case MENU_SAVE_AS: evt_menu_save_as(); break;
        case MENU_EXIT: evt_menu_exit(); break;
    }
}

void MaterialManager::evt_menu_exit()
{
    Close();
}

void MaterialManager::evt_menu_load()
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
            editor->material=dialog.material;
            
            editor->update_controls();
            editor->rebuild_elements_list();
            
            material_path->set_value(editor->material.script_path.generic_string());
            
            chk_var(editor->material.script_path);
            if(PathManager::belongs_to_resources(editor->material.script_path))
            {
                library_material=true;
                editor->lock();
            }
            else
            {
                library_material=false;
                editor->unlock();
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
        
        editor->material.load_lua_script(new_path);
        
        editor->update_controls();
        editor->rebuild_elements_list();
        
        material_path->set_value(new_path.generic_string());
        
        if(PathManager::belongs_to_resources(new_path))
        {
            library_material=true;
            editor->lock();
        }
        else
        {
            library_material=false;
            editor->unlock();
        }
    }
}

void MaterialManager::evt_menu_new()
{
    wxFileName data_tmp=wxFileSelector("Please create a new material file",
                                       wxString(PathManager::user_profile_materials.generic_string()),
                                       "temporary_material",
                                       ".lua",
                                       "Lua script (*.lua)|*.lua",
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                                       
    if(data_tmp.IsOk()==false) return;
    
    std::filesystem::path new_path=data_tmp.GetFullPath().ToStdString();
        
    editor->material.reset();
    editor->material.script_path=new_path;
    editor->material.write_lua_script();
    
    editor->update_controls();
    editor->rebuild_elements_list();
    
    material_path->set_value(new_path.generic_string());
    
    library_material=false;
    editor->unlock();
}

void MaterialManager::evt_menu_save()
{
    if(!library_material)
    {
        editor->material.write_lua_script();
    }
}

void MaterialManager::evt_menu_save_as()
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
    
    editor->material.script_path=new_path;
    editor->material.write_lua_script();
    
    editor->rebuild_elements_list();
    editor->update_controls();
    
    material_path->set_value(new_path.generic_string());
}

void MaterialManager::evt_spectrum_selector(wxCommandEvent &event)
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

void MaterialManager::recompute_model()
{
    Material &material=editor->material;
    
    int display_type=disp_choice->GetSelection();
        
    for(unsigned int i=0;i<Np;i++)
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
