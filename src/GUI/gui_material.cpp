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

wxDEFINE_EVENT(EVT_MAT_SELECTOR,wxCommandEvent);
wxDEFINE_EVENT(EVT_MINIMAT_SELECTOR,wxCommandEvent);

//####################
// MaterialExplorer
//####################

MaterialExplorer::MaterialExplorer(wxString const &string)
    :MaterialExplorer(400e-9,800e-9,401,nullptr)
{
}

MaterialExplorer::MaterialExplorer(double lambda_min_,double lambda_max_,int Np_,MaterialSelector *selector)
    :BaseFrame(""),
     Np(Np_),
     lambda_min(lambda_min_), lambda_max(lambda_max_),
     lambda(Np), disp_lambda(Np),
     disp_real(Np), disp_imag(Np)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    mat_graph=new Graph(this);
    
    // Ctrl panel
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    wxBoxSizer *disp_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *mat_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *index_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Display");
    
    mat_selector=new MaterialSelector(ctrl_panel,"Material");
    sp_selector=new SpectrumSelector(ctrl_panel,lambda_min,lambda_max,Np);
    
    wxString disp_str[]={"Index","Permittivity"};
    
    disp_choice=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,2,disp_str);
    disp_choice->SetSelection(0);
    
    wxButton *export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    
    index_sizer->Add(disp_choice);
    
    disp_sizer->Add(sp_selector,wxSizerFlags());
    disp_sizer->Add(index_sizer);
    disp_sizer->Add(export_btn,wxSizerFlags().Expand().Border(wxALL,3));
    
    mat_sizer->Add(mat_selector,wxSizerFlags(1));
    
    ctrl_sizer->Add(disp_sizer);
    ctrl_sizer->Add(mat_sizer,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // 
    
    top_sizer->Add(mat_graph,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    export_btn->Bind(wxEVT_BUTTON,&MaterialExplorer::export_event,this);
    disp_choice->Bind(wxEVT_CHOICE,&MaterialExplorer::disp_choice_event,this);
    mat_selector->Bind(EVT_MAT_SELECTOR,&MaterialExplorer::material_selector_event,this);
    sp_selector->Bind(EVT_SPECTRUM_SELECTOR,&MaterialExplorer::spectrum_selector_event,this);
    
    if(selector!=nullptr) *mat_selector=*selector;
        
    SetTitle(mat_selector->get_title());
    
    SetSizer(top_sizer);
    
    Show();
    Maximize();
}

void MaterialExplorer::disp_choice_event(wxCommandEvent &event)
{
    recompute_model();
}

void MaterialExplorer::export_event(wxCommandEvent &event)
{
    std::vector<wxString> choices(3);
    
    choices[0]="Permittivity";
    choices[1]="Refractive index";
    choices[2]="MatLab Script";
    
    ChoiceDialog dialog("Export type",choices);
    
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
    
    if(dialog.choice==0 || dialog.choice==1)
    {
        for(unsigned int l=0;l<Np;l++)
        {
            Imdouble eps=mat_selector->get_eps(m_to_rad_Hz(lambda[l]));
            
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
        Material mat=mat_selector->get_material();
        
        file<<mat.get_matlab(data_tmp.GetFullPath().ToStdString());
    }
}

void MaterialExplorer::material_selector_event(wxCommandEvent &event)
{
    lambda_min=mat_selector->get_lambda_validity_min();
    lambda_max=mat_selector->get_lambda_validity_max();
    
    sp_selector->set_spectrum(lambda_min,lambda_max);
    
    SetTitle(mat_selector->get_title());
    
    recompute_model();
}

void MaterialExplorer::spectrum_selector_event(wxCommandEvent &event)
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

void MaterialExplorer::recompute_model()
{
    int display_type=disp_choice->GetSelection();
        
    for(unsigned int i=0;i<Np;i++)
    {
        lambda[i]=lambda_min+(lambda_max-lambda_min)*i/(Np-1.0);
        disp_lambda[i]=m_to_nm(lambda[i]);
        
        double w=m_to_rad_Hz(lambda[i]);
        
        Imdouble eps=mat_selector->get_eps(w);
        
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

//######################
// MiniMaterialSelector
//######################

class MMS_Dialog:public wxDialog
{
    public:
        bool selection_ok;
        MaterialSelector *selector;
        Material material;
        
        wxButton *ok_btn;
        wxPanel *container_panel;
        wxScrolledWindow *selector_panel;
        
        MMS_Dialog(Material const &material)
            :wxDialog(0,wxID_ANY,"Select the material",
                      wxGetApp().default_dialog_origin(),wxDefaultSize),
             selection_ok(false)
        {
            wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
            wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
            
            // Container Panel
            
            container_panel=new wxPanel(this);
            
            wxBoxSizer *container_sizer=new wxBoxSizer(wxVERTICAL);
            
            // Inside Panel
            
            selector_panel=new wxScrolledWindow(container_panel,wxID_ANY);
            
            wxBoxSizer *selector_sizer=new wxBoxSizer(wxVERTICAL);
            selector=new MaterialSelector(selector_panel,"Material",false,material);
            
            selector_sizer->Add(selector,wxSizerFlags());
            
            selector_panel->SetSizerAndFit(selector_sizer);
            selector_panel->SetScrollRate(50,50);
            
            container_sizer->Add(selector_panel,wxSizerFlags(1).Expand());
            container_panel->SetSizer(container_sizer);
            
            top_sizer->Add(container_panel,wxSizerFlags(1).Expand());
            
            // Buttons
            
            ok_btn=new wxButton(this,wxID_ANY,"Ok");
            wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
            
            btn_sizer->Add(ok_btn);
            btn_sizer->Add(cancel_btn);
            
            top_sizer->Add(btn_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
            
            SetSizerAndFit(top_sizer);
            
            ok_btn->Bind(wxEVT_BUTTON,&MMS_Dialog::evt_ok,this);
            cancel_btn->Bind(wxEVT_BUTTON,&MMS_Dialog::evt_cancel,this);
            
            Bind(EVT_MAT_SELECTOR,&MMS_Dialog::evt_material,this);
            
            ShowModal();
        }
                
        void evt_cancel(wxCommandEvent &event)
        {
            selection_ok=false;
            Close();
        }
        
        void evt_material(wxCommandEvent &event)
        {
            SetClientSize(wxSize(20,20));
            selector_panel->Layout();
            selector_panel->FitInside();
            
            int x,y,xb,yb;
            selector_panel->GetVirtualSize(&x,&y);
            ok_btn->GetClientSize(&xb,&yb);
            wxSize max_size=wxGetApp().default_dialog_size();
            
            SetClientSize(wxSize(std::min(max_size.x,x),std::min(max_size.y,y+yb+3)));
            
            selector_panel->FitInside();
        }
        
        void evt_ok(wxCommandEvent &event)
        {
            selection_ok=true;
            material=selector->get_material();
            
            Close();
        }
};

MiniMaterialSelector::MiniMaterialSelector(wxWindow *parent,
                                           Material const &material_,
                                           std::string const &name)
    :wxPanel(parent),
     mat_type(MatType::REAL_N)
{
    material.set_const_n(1.0);
    
    wxSizer *sizer=nullptr;
    if(name=="") sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    mat_bmp=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(PathManager::locate_resource("resources/n_16.png").generic_string()));
    mat_txt=new wxStaticText(this,wxID_ANY,"");
    mat_name=new wxTextCtrl(this,wxID_ANY,"1.0");
    eff_weight=new NamedTextCtrl<double>(this,"",0,false,4);
    edit_btn=new wxButton(this,wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    mat_name->Disable();
    mat_txt->Hide();
    
    eff_weight->Hide();
    eff_weight->Bind(EVT_NAMEDTXTCTRL,&MiniMaterialSelector::evt_weight,this);
    
    sizer->Add(mat_bmp,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,5).Border(wxRIGHT,5)); //to be replaced with CenterVertical
    sizer->Add(mat_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,5).Border(wxRIGHT,5));
    sizer->Add(mat_name,wxSizerFlags(1).Expand());
    sizer->Add(eff_weight,wxSizerFlags());
    sizer->Add(edit_btn,wxSizerFlags());
    
    SetSizer(sizer);
    
    edit_btn->Bind(wxEVT_BUTTON,&MiniMaterialSelector::evt_edit,this);
    
    material=material_;
        
    if(material.is_const())
    {
        mat_name->SetValue(std::to_string(material.get_n(0).real()));
    }
    else
    {
        unsigned int n;
        
        if(MaterialsLib::knows_material(n,material)==nullptr) mat_type=MatType::SCRIPT;
        else mat_type=MatType::LIBRARY;
        
        mat_name->SetValue(material.script_path.generic_string());
    }
}

MiniMaterialSelector::MiniMaterialSelector(wxWindow *parent,
                                           std::string const &name,
                                           std::filesystem::path const &script_)
    :wxPanel(parent),
     mat_type(MatType::REAL_N)
{
    material.set_const_n(1.0);
    
    wxSizer *sizer=nullptr;
    if(name=="") sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    mat_bmp=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(PathManager::locate_resource("resources/n_16.png").generic_string()));
    mat_txt=new wxStaticText(this,wxID_ANY,"");
    mat_name=new wxTextCtrl(this,wxID_ANY,"1.0");
    eff_weight=new NamedTextCtrl<double>(this,"",0,false,4);
    edit_btn=new wxButton(this,wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    mat_name->Disable();
    mat_txt->Hide();
    
    eff_weight->Hide();
    eff_weight->Bind(EVT_NAMEDTXTCTRL,&MiniMaterialSelector::evt_weight,this);
    
    sizer->Add(mat_bmp,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,5).Border(wxRIGHT,5)); //to be replaced with CenterVertical
    sizer->Add(mat_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,5).Border(wxRIGHT,5));
    sizer->Add(mat_name,wxSizerFlags(1).Expand());
    sizer->Add(eff_weight,wxSizerFlags());
    sizer->Add(edit_btn,wxSizerFlags());
    
    SetSizer(sizer);
    
    edit_btn->Bind(wxEVT_BUTTON,&MiniMaterialSelector::evt_edit,this);
    
    if(!script_.empty())
    {
        material.load_lua_script(script_);
        
        if(material.is_const())
        {
            mat_name->SetValue(std::to_string(material.get_n(0).real()));
        }
        else
        {
            unsigned int n;
            
            if(MaterialsLib::knows_material(n,material)==nullptr) mat_type=MatType::SCRIPT;
            else mat_type=MatType::LIBRARY;
            
            mat_name->SetValue(script_.filename().generic_string());
        }
        
        update_label();
    }
}

void MiniMaterialSelector::copy_material(MiniMaterialSelector *mat_)
{
    mat_type=mat_->mat_type;
    material=mat_->material;
    mat_name->SetValue(mat_->mat_name->GetValue());
    
    update_label();
}

void MiniMaterialSelector::evt_edit(wxCommandEvent &event)
{
    MMS_Dialog dialog(material);
    
    if(dialog.selection_ok)
    {
        mat_type=dialog.selector->get_type();
        update_label();
        
        wxString name=dialog.selector->get_name();
        
        mat_name->ChangeValue(name);
        edit_btn->SetToolTip(name);
        
        material=dialog.material;
        
        if(material.is_effective_material)
        {
            eff_weight->Show();
            eff_weight->set_value(material.eff_weight);
        }
        else eff_weight->Hide();
        
        Layout();
        
        wxCommandEvent event_out(EVT_MINIMAT_SELECTOR);
        wxPostEvent(this,event_out);
    }
    
    event.Skip();
}

void MiniMaterialSelector::evt_weight(wxCommandEvent &event)
{
    double val=std::clamp(eff_weight->get_value(),0.0,1.0);
    
    material.eff_weight=val;
    eff_weight->set_value(val);
    
    wxCommandEvent event_out(EVT_MINIMAT_SELECTOR);
    wxPostEvent(this,event_out);
}

Imdouble MiniMaterialSelector::get_eps(double w) { return material.get_eps(w); }

wxString MiniMaterialSelector::get_lua()
{
    wxString str;
        
    if(material.is_const()) str<<"const_material("<<material.get_n(0).real()<<")";
    else str<<"\""<<material.script_path.generic_string()<<"\"";
    
    return str;
}

Imdouble MiniMaterialSelector::get_n(double w) { return material.get_n(w); }

Material& MiniMaterialSelector::get_material() { return material; }

void MiniMaterialSelector::set_material(std::filesystem::path const &script_fname)
{
    material.load_lua_script(script_fname);
        
    if(material.is_const())
    {
        mat_name->SetValue(std::to_string(material.get_n(0).real()));
    }
    else
    {
        unsigned int n;
        
        if(MaterialsLib::knows_material(n,material)==nullptr) mat_type=MatType::SCRIPT;
        else mat_type=MatType::LIBRARY;
        
        mat_name->SetValue(script_fname.filename().generic_string());
    }
    
    update_label();
}

void MiniMaterialSelector::update_label()
{
         if(mat_type==MatType::REAL_N)
    {
        mat_txt->Hide();
        mat_bmp->Show();
        mat_bmp->SetBitmap(ImagesManager::get_bitmap(PathManager::locate_resource("resources/n_16.png").generic_string()));
    }
    else if(mat_type==MatType::REAL_EPS)
    {
        mat_txt->Hide();
        mat_bmp->Show();
        mat_bmp->SetBitmap(ImagesManager::get_bitmap(PathManager::locate_resource("resources/varepsilon_16.png").generic_string()));
    }
    else if(mat_type==MatType::LIBRARY)
    {
        mat_txt->SetLabel("Lib");
        mat_txt->Show();
        mat_bmp->Hide();
    }
    else if(mat_type==MatType::SCRIPT)
    {
        mat_txt->SetLabel("Scr");
        mat_txt->Show();
        mat_bmp->Hide();
    }
    else if(mat_type==MatType::EFFECTIVE)
    {
        mat_txt->SetLabel("Eff");
        mat_txt->Show();
        mat_bmp->Hide();
    }
    else if(mat_type==MatType::CUSTOM)
    {
        mat_txt->SetLabel("Ctm");
        mat_txt->Show();
        mat_bmp->Hide();
    }
    
    Layout();
}



//#########################
//   MaterialsListDialog
//#########################

MaterialsListSub::MaterialsListSub(wxWindow *parent)
    :PanelsListBase(parent)
{
    material=new MaterialSelector(this,"Material");
    sizer->Add(material,wxSizerFlags().Expand());
    
    Layout();
}

MaterialsListSub::MaterialsListSub(wxWindow *parent,Material const &material_)
    :PanelsListBase(parent)
{
    material=new MaterialSelector(this,"Material",true,material_);
    sizer->Add(material,wxSizerFlags().Expand());
    
    Layout();
}

MaterialsListDialog::MaterialsListDialog(std::vector<Material> &materials_)
    :wxDialog(NULL,wxID_ANY,"Materials",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     list_changed(false), materials(&materials_)
{
    wxBoxSizer *main_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // List
    
    wxStaticBoxSizer *mats_outline=new wxStaticBoxSizer(wxVERTICAL,this,"Ordered Materials List");
    
    mats_panel=new wxScrolledWindow(this);
    wxBoxSizer *mats_sizer=new wxBoxSizer(wxVERTICAL);
    
    materials_list=new PanelsList<>(mats_panel);
    
    for(unsigned int i=0;i<materials->size();i++)
        materials_list->add_panel<MaterialsListSub>((*materials)[i]);
    
    materials_renumber();
    
    mats_sizer->Add(materials_list,wxSizerFlags(1).Expand());
    
    mats_panel->SetSizer(mats_sizer);
    mats_panel->SetScrollRate(50,50);
    mats_panel->FitInside();
    
    mats_outline->Add(mats_panel,wxSizerFlags(1).Expand());
    
    //
    
    wxButton *add_btn=new wxButton(this,wxID_ANY,"Add Material");
    
    main_sizer->Add(mats_outline,wxSizerFlags(1).Expand());
    main_sizer->Add(add_btn,wxSizerFlags().Border(wxALL,3).Align(wxALIGN_TOP));
    
    SetSizer(main_sizer);
    
    // Bindings
    
    add_btn->Bind(wxEVT_BUTTON,&MaterialsListDialog::evt_new_material,this);
    Bind(EVT_MAT_SELECTOR,&MaterialsListDialog::evt_material,this);
    Bind(EVT_PLIST_DOWN,&MaterialsListDialog::evt_materials_reorder,this);
    Bind(EVT_PLIST_REMOVE,&MaterialsListDialog::evt_delete_material,this);
    Bind(EVT_PLIST_UP,&MaterialsListDialog::evt_materials_reorder,this);
    
    ShowModal();
}

MaterialsListDialog::~MaterialsListDialog()
{
    if(list_changed)
    {
        int i,Nmat=materials_list->get_size();
        
        materials->resize(Nmat);
        
        for(i=0;i<Nmat;i++)
        {
             MaterialsListSub *panel=dynamic_cast<MaterialsListSub*>(materials_list->get_panel(i));
             
             (*materials)[i]=panel->material->get_material();
        }
    }
}

void MaterialsListDialog::evt_delete_material(wxCommandEvent &event)
{
    list_changed=true;
    
    materials_renumber();
    
    mats_panel->FitInside();
}

void MaterialsListDialog::evt_new_material(wxCommandEvent &event)
{
    list_changed=true;
    
    Material tmp_mat;
    tmp_mat.set_const_n(1.0);
    
    materials_list->add_panel<MaterialsListSub>(tmp_mat);
    
    materials_renumber();
    
    mats_panel->FitInside();
    
    event.Skip();
}

void MaterialsListDialog::evt_material(wxCommandEvent &event)
{
    list_changed=true;
    
    event.Skip();
}

void MaterialsListDialog::evt_materials_reorder(wxCommandEvent &event)
{
    list_changed=true;
    
    materials_renumber();
}

void MaterialsListDialog::materials_renumber()
{
    for(unsigned int i=0;i<materials_list->get_size();i++)
    {
         PanelsListBase *panel=materials_list->get_panel(i);
         
         std::string mat_name("Material ");
         mat_name.append(std::to_string(i));
         panel->set_title(mat_name);
    }
}
