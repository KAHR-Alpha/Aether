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

#include <gui_material.h>

#include <wx/splitter.h>

//###################
//   MatGUI Panels
//###################

namespace MatGUI
{
    // SubmodelPanel
    
    SubmodelPanel::SubmodelPanel(wxWindow *parent)
        :PanelsListBase(parent)
    {
        up_btn->Hide();
        down_btn->Hide();
        
        double R=randp(0.2);
        double G=randp(0.2);
        double B=randp(0.2);
        
        SetBackgroundColour(wxColour(255*(0.8+R),
                                     255*(0.8+G),
                                     255*(0.8+B)));
    }
    
    void SubmodelPanel::evt_modification(wxCommandEvent &event)
    {
        gui_to_mat();
    }
    
    void SubmodelPanel::gui_to_mat() {}
    
    // EpsInfPanel
    
    EpsInfPanel::EpsInfPanel(wxWindow *parent,double *eps_inf_)
        :SubmodelPanel(parent),
         mat_eps_inf(eps_inf_)
    {
        eps_inf=new NamedTextCtrl<double>(this,"Eps Infinity",*mat_eps_inf,true);
        
        kill->Hide();
        
        sizer->Add(eps_inf,wxSizerFlags().Expand());
    }
    
    void EpsInfPanel::gui_to_mat()
    {
        *mat_eps_inf=eps_inf->get_value();
    }
    
    // DrudePanel
    
    DrudePanel::DrudePanel(wxWindow *parent,DrudeModel *drude_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_drude(drude_)
    {
        set_title("Drude "+std::to_string(ID));
        
        wd=new WavelengthSelector(this,"wd: ",rad_Hz_to_m(mat_drude->wd));
        g=new WavelengthSelector(this,"g: ",rad_Hz_to_m(mat_drude->g));
        
        wd->change_unit("rad Hz");
        g->change_unit("rad Hz");
                
        sizer->Add(wd,wxSizerFlags().Expand());
        sizer->Add(g,wxSizerFlags().Expand());
    }
    
    void DrudePanel::gui_to_mat()
    {
        mat_drude->set(m_to_rad_Hz(wd->get_lambda()),
                       m_to_rad_Hz(g->get_lambda()));
    }
    
    // LorentzPanel
    
    LorentzPanel::LorentzPanel(wxWindow *parent,LorentzModel *lorentz_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_lorentz(lorentz_)
    {
        set_title("Lorentz "+std::to_string(ID));
        
        A=new NamedTextCtrl<double>(this,"A: ",mat_lorentz->A,true);
        O=new WavelengthSelector(this,"O: ",rad_Hz_to_m(mat_lorentz->O));
        G=new WavelengthSelector(this,"G: ",rad_Hz_to_m(mat_lorentz->G));
        
        O->change_unit("rad Hz");
        G->change_unit("rad Hz");
                
        sizer->Add(A,wxSizerFlags().Expand());
        sizer->Add(O,wxSizerFlags().Expand());
        sizer->Add(G,wxSizerFlags().Expand());
    }
    
    void LorentzPanel::gui_to_mat()
    {
        mat_lorentz->set(A->get_value(),
                         m_to_rad_Hz(O->get_lambda()),
                         m_to_rad_Hz(G->get_lambda()));
    }
    
    // CritpointPanel
    
    CritpointPanel::CritpointPanel(wxWindow *parent,CritpointModel *critpoint_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_critpoint(critpoint_)
    {
        set_title("Critpoint "+std::to_string(ID));
        
        A=new NamedTextCtrl<double>(this,"A: ",mat_critpoint->A,true);
        O=new WavelengthSelector(this,"O: ",rad_Hz_to_m(mat_critpoint->O));
        P=new NamedTextCtrl<double>(this,"P: ",mat_critpoint->P,true);
        G=new WavelengthSelector(this,"G: ",rad_Hz_to_m(mat_critpoint->G));
        
        O->change_unit("rad Hz");
        G->change_unit("rad Hz");
                
        sizer->Add(A,wxSizerFlags().Expand());
        sizer->Add(O,wxSizerFlags().Expand());
        sizer->Add(P,wxSizerFlags().Expand());
        sizer->Add(G,wxSizerFlags().Expand());
    }
    
    void CritpointPanel::gui_to_mat()
    {
        mat_critpoint->set(A->get_value(),
                           m_to_rad_Hz(O->get_lambda()),
                           P->get_value(),
                           m_to_rad_Hz(G->get_lambda()));
    }
    
    // CauchyPanel
    
    CauchyPanel::CauchyPanel(wxWindow *parent,std::vector<double> *coeffs_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_coeffs(coeffs_)
    {
        set_title("Cauchy "+std::to_string(ID));
        
        std::size_t N=mat_coeffs->size();
        
        coeffs.resize(N);
        
        for(std::size_t i=0;i<N;i++)
        {
            coeffs[i]=new NamedTextCtrl(this,"C"+std::to_string(i)+": ",(*mat_coeffs)[i],true);
            sizer->Add(coeffs[i],wxSizerFlags().Expand());
        }
    }
    
    void CauchyPanel::gui_to_mat()
    {
        mat_coeffs->resize(coeffs.size());
        
        for(std::size_t i=0;i<coeffs.size();i++)
        {
            (*mat_coeffs)[i]=coeffs[i]->get_value();
        }
    }
    
    // SellmeierPanel
    
    SellmeierPanel::SellmeierPanel(wxWindow *parent,double *B_,double *C_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_B(B_), mat_C(C_)
    {
        set_title("Sellmeier "+std::to_string(ID));
        
        B=new NamedTextCtrl(this,"B: ",*mat_B,true);
        C=new WavelengthSelector(this,"C: ",*mat_C);
        
        sizer->Add(B,wxSizerFlags().Expand());
        sizer->Add(C,wxSizerFlags().Expand());
    }
    
    void SellmeierPanel::gui_to_mat()
    {
        *mat_B=B->get_value();
        *mat_C=C->get_lambda();
    }
}

//####################
//   MaterialEditor
//####################

wxDEFINE_EVENT(EVT_MATERIAL_EDITOR,wxCommandEvent);

MaterialEditor::MaterialEditor(wxWindow *parent)
    :wxPanel(parent)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    // Description
    
    wxStaticBoxSizer *description_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Description");
    
    description=new wxTextCtrl(description_sizer->GetStaticBox(),
                               wxID_ANY,wxEmptyString,
                               wxDefaultPosition,wxDefaultSize,
                               wxTE_BESTWRAP|wxTE_MULTILINE|wxTE_RICH);
    description->SetMinClientSize(wxSize(-1,150));
    
    description_sizer->Add(description,wxSizerFlags().Expand());
    sizer->Add(description_sizer,wxSizerFlags().Expand());
    
    // Validity Range
    
    wxStaticBoxSizer *validity_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Validity Range");
    
    validity_min=new WavelengthSelector(validity_sizer->GetStaticBox(),"Min: ",400e-9);
    validity_max=new WavelengthSelector(validity_sizer->GetStaticBox(),"Max: ",800e-9);
    
    validity_sizer->Add(validity_min,wxSizerFlags().Expand());
    validity_sizer->Add(validity_max,wxSizerFlags().Expand());
    
    sizer->Add(validity_sizer,wxSizerFlags().Expand());
    
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
    
    wxButton *add_btn=new wxButton(this,wxID_ANY,"Add",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    choice_sizer->Add(add_btn,wxSizerFlags().Expand());
    
    sizer->Add(choice_sizer,wxSizerFlags().Expand());
    
    //
    
    material_elements=new PanelsList<MatGUI::SubmodelPanel>(this);
    
    sizer->Add(material_elements,wxSizerFlags(1).Expand());
    
    SetSizer(sizer);
}

void MaterialEditor::rebuild_elements_list()
{
    material_elements->clear();
    
    material_elements->add_panel<MatGUI::EpsInfPanel>(&material.eps_inf);
    
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
    
    material_elements->Layout();
}

void MaterialEditor::throw_event()
{
    wxCommandEvent event(EVT_MATERIAL_EDITOR);
    
    wxPostEvent(this,event);
}

void MaterialEditor::update_controls()
{
    description->ChangeValue(material.description);
    
    validity_min->set_lambda(material.lambda_valid_min);
    validity_max->set_lambda(material.lambda_valid_max);
}

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
    
    Bind(wxEVT_MENU,&MaterialManager::evt_menu,this);
    
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
    disp_choice->Bind(wxEVT_CHOICE,&MaterialManager::disp_choice_event,this);
    
    index_sizer->Add(disp_choice);
    
    display_subsizer->Add(sp_selector);
    display_subsizer->Add(index_sizer);
    
    display_sizer->Add(mat_graph,wxSizerFlags(1).Expand());
    display_sizer->Add(display_subsizer);
}

void MaterialManager::disp_choice_event(wxCommandEvent &event)
{
    recompute_model();
}

//void MaterialManager::evt_material_selector(wxCommandEvent &event)
//{
//    lambda_min=mat_selector->get_lambda_validity_min();
//    lambda_max=mat_selector->get_lambda_validity_max();
//    
//    sp_selector->set_spectrum(lambda_min,lambda_max);
//    material=mat_selector->get_material();
//    
//    SetTitle(mat_selector->get_title());
//    
//    rebuild_elements_list();
//    recompute_model();
//    update_controls();
//    
//    Layout();
//}

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
        
        editor->rebuild_elements_list();
        editor->update_controls();
        
        material_path->set_value(new_path.generic_string());
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
    
    editor->material.reset();
    editor->material.script_path=data_tmp.GetFullPath().ToStdString();
    editor->material.write_lua_script();
    
    editor->rebuild_elements_list();
    editor->update_controls();
}

void MaterialManager::evt_menu_save()
{
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

//void MaterialManager::export_event(wxCommandEvent &event)
//{
//    std::vector<wxString> choices(3);
//    
//    choices[0]="Permittivity";
//    choices[1]="Refractive index";
//    choices[2]="MatLab Script";
//    
//    ChoiceDialog dialog("Export type",choices);
//    
//    if(!dialog.choice_ok) return;
//    
//    wxFileName data_tmp=wxFileSelector("Save the structure script as",
//                                       wxEmptyString,
//                                       wxEmptyString,
//                                       wxEmptyString,
//                                       wxFileSelectorDefaultWildcardStr,
//                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
//    if(data_tmp.IsOk()==false) return;
//    
//    std::ofstream file(data_tmp.GetFullPath().ToStdString(),
//                       std::ios::out|std::ios::trunc|std::ios::binary);
//    
//    if(dialog.choice==0 || dialog.choice==1)
//    {
//        for(unsigned int l=0;l<Np;l++)
//        {
//            Imdouble eps=mat_selector->get_eps(m_to_rad_Hz(lambda[l]));
//            
//            if(dialog.choice==0) file<<lambda[l]<<" "<<eps.real()<<" "<<eps.imag()<<std::endl;
//            else
//            {
//                Imdouble n=std::sqrt(eps);
//                
//                file<<lambda[l]<<" "<<n.real()<<" "<<n.imag()<<std::endl;
//            }
//        }
//    }
//    else
//    {
//        Material mat=mat_selector->get_material();
//        
//        file<<mat.get_matlab(data_tmp.GetFullPath().ToStdString());
//    }
//}

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
