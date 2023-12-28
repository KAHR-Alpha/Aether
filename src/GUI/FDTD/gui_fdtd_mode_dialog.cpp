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

#include <gui_fdtd.h>
#include <gui_material.h>

#include <wx/notebook.h>

bool fdtd_material_validator(Material *material)
{
    return material->fdtd_compatible();
}

class FDTD_Material_Panel: public PanelsListBase
{
    public:
        MaterialSelector *material;
        
        FDTD_Material_Panel(wxWindow *parent,GUI::Material *material_)
            :PanelsListBase(parent)
        {
            set_title("Material");
            
            material=new MaterialSelector(this,"",true,material_,&fdtd_material_validator);
            material->hide_description();
            material->hide_validity();
            
            sizer->Add(material,wxSizerFlags().Expand().Border(wxALL,2));
        }
        
        GUI::Material* get_material() { return material->get_material(); }
};

//#######################
//   FD_Boundary_Panel
//#######################

FD_Boundary_Panel::FD_Boundary_Panel(wxWindow *parent,std::string const &base_name,bool periodic_,
                                     int N_pml_m_,double k_max_m_,double s_max_m_,double a_max_m_,int padding_m_,
                                     int N_pml_p_,double k_max_p_,double s_max_p_,double a_max_p_,int padding_p_)
    :wxPanel(parent)
{
    wxStaticText *type_txt=new wxStaticText(this,wxID_ANY,"Type: ");
    type_choice=new wxChoice(this,wxID_ANY);
    type_choice->Append("Periodic");
    type_choice->Append("PML");
    
    type_choice->Bind(wxEVT_CHOICE,&FD_Boundary_Panel::evt_periodic,this);
    
    if(periodic_) type_choice->SetSelection(0);
    else type_choice->SetSelection(1);
    
    wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    type_sizer->Add(type_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    type_sizer->Add(type_choice);
    
    // M
    
    N_ctrl_m=new NamedTextCtrl<int>(this,"N: ",N_pml_m_);
    k_max_ctrl_m=new NamedTextCtrl<double>(this,"",k_max_m_);
    s_max_ctrl_m=new NamedTextCtrl<double>(this,"",s_max_m_);
    a_max_ctrl_m=new NamedTextCtrl<double>(this,"",a_max_m_);
    padding_m=new NamedTextCtrl<int>(this,"Padding: ",padding_m_);
    
    padding_m->lock();
    
    padding_m->Bind(EVT_NAMEDTXTCTRL,&FD_Boundary_Panel::evt_padding,this);
    
    wxBoxSizer *k_sizer_m=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *s_sizer_m=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *a_sizer_m=new wxBoxSizer(wxHORIZONTAL);
    
    std::string kappa_16=PathManager::locate_resource("resources/kappa_16.png").generic_string();
    std::string sigma_16=PathManager::locate_resource("resources/sigma_16.png").generic_string();
    std::string alpha_16=PathManager::locate_resource("resources/alpha_16.png").generic_string();
    
    wxGenericStaticBitmap *k_bmp_m=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(kappa_16));
    wxGenericStaticBitmap *s_bmp_m=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(sigma_16));
    wxGenericStaticBitmap *a_bmp_m=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(alpha_16));
    
    k_sizer_m->Add(k_bmp_m,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    s_sizer_m->Add(s_bmp_m,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    a_sizer_m->Add(a_bmp_m,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    k_sizer_m->Add(k_max_ctrl_m,wxSizerFlags(1).Expand());
    s_sizer_m->Add(s_max_ctrl_m,wxSizerFlags(1).Expand());
    a_sizer_m->Add(a_max_ctrl_m,wxSizerFlags(1).Expand());
    
    wxString name_m("-");
    name_m<<base_name;
    
    wxStaticBoxSizer *sizer_m=new wxStaticBoxSizer(wxVERTICAL,this,name_m);
    
    sizer_m->Add(N_ctrl_m,wxSizerFlags().Expand());
    sizer_m->Add(k_sizer_m,wxSizerFlags().Expand());
    sizer_m->Add(s_sizer_m,wxSizerFlags().Expand());
    sizer_m->Add(a_sizer_m,wxSizerFlags().Expand());
    sizer_m->Add(padding_m,wxSizerFlags().Expand());
    
    // P
    
    N_ctrl_p=new NamedTextCtrl<int>(this,"N: ",N_pml_p_);
    k_max_ctrl_p=new NamedTextCtrl<double>(this,"",k_max_p_);
    s_max_ctrl_p=new NamedTextCtrl<double>(this,"",s_max_p_);
    a_max_ctrl_p=new NamedTextCtrl<double>(this,"",a_max_p_);
    padding_p=new NamedTextCtrl<int>(this,"Padding: ",padding_p_);
    
    padding_p->lock();
    
    padding_p->Bind(EVT_NAMEDTXTCTRL,&FD_Boundary_Panel::evt_padding,this);
    
    wxBoxSizer *k_sizer_p=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *s_sizer_p=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *a_sizer_p=new wxBoxSizer(wxHORIZONTAL);
    
    wxGenericStaticBitmap *k_bmp_p=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(kappa_16));
    wxGenericStaticBitmap *s_bmp_p=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(sigma_16));
    wxGenericStaticBitmap *a_bmp_p=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(alpha_16));
    
    k_sizer_p->Add(k_bmp_p,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    s_sizer_p->Add(s_bmp_p,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    a_sizer_p->Add(a_bmp_p,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    k_sizer_p->Add(k_max_ctrl_p,wxSizerFlags(1).Expand());
    s_sizer_p->Add(s_max_ctrl_p,wxSizerFlags(1).Expand());
    a_sizer_p->Add(a_max_ctrl_p,wxSizerFlags(1).Expand());
    
    wxString name_p("+");
    name_p<<base_name;
    
    wxStaticBoxSizer *sizer_p=new wxStaticBoxSizer(wxVERTICAL,this,name_p);
    
    sizer_p->Add(N_ctrl_p,wxSizerFlags().Expand());
    sizer_p->Add(k_sizer_p,wxSizerFlags().Expand());
    sizer_p->Add(s_sizer_p,wxSizerFlags().Expand());
    sizer_p->Add(a_sizer_p,wxSizerFlags().Expand());
    sizer_p->Add(padding_p,wxSizerFlags().Expand());
    
    wxString name(base_name);
    name<<" boundary";
    
    wxBoxSizer *pml_sizer=new wxBoxSizer(wxHORIZONTAL);
    pml_sizer->Add(sizer_m,wxSizerFlags(1));
    pml_sizer->Add(sizer_p,wxSizerFlags(1));
    
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,name);
    
    sizer->Add(type_sizer,wxSizerFlags().Border(wxALL,2));
    sizer->Add(pml_sizer,wxSizerFlags().Expand());
    
    layout_periodic();
    
    SetSizer(sizer);
}

void FD_Boundary_Panel::evt_padding(wxCommandEvent &event)
{
    if(padding_m->get_value()<5) padding_m->set_value(5);
    if(padding_p->get_value()<5) padding_p->set_value(5);
    
    event.Skip();
}

void FD_Boundary_Panel::evt_periodic(wxCommandEvent &event)
{
    layout_periodic();
    
    event.Skip();
}

void FD_Boundary_Panel::get_parameters(bool &periodic,
                                       int &N_pml_m_,double &k_max_m_,double &s_max_m_,double &a_max_m_,int &padding_m_,
                                       int &N_pml_p_,double &k_max_p_,double &s_max_p_,double &a_max_p_,int &padding_p_)
{
    if(type_choice->GetSelection()==0) periodic=true;
    else periodic=false;
    
    N_pml_m_=N_ctrl_m->get_value();
    k_max_m_=k_max_ctrl_m->get_value();
    s_max_m_=s_max_ctrl_m->get_value();
    a_max_m_=a_max_ctrl_m->get_value();
    padding_m_=padding_m->get_value();
    
    N_pml_p_=N_ctrl_p->get_value();
    k_max_p_=k_max_ctrl_p->get_value();
    s_max_p_=s_max_ctrl_p->get_value();
    a_max_p_=a_max_ctrl_p->get_value();
    padding_p_=padding_p->get_value();
}

void FD_Boundary_Panel::layout_periodic()
{
    int selection=type_choice->GetSelection();
    
    if(selection==0)
    {
        N_ctrl_m->Disable(); N_ctrl_p->Disable();
        padding_m->Disable(); padding_p->Disable();
        k_max_ctrl_m->Disable(); k_max_ctrl_p->Disable();
        s_max_ctrl_m->Disable(); s_max_ctrl_p->Disable();
        a_max_ctrl_m->Disable(); a_max_ctrl_p->Disable();
    }
    else
    {
        N_ctrl_m->Enable(); N_ctrl_p->Enable();
        padding_m->Enable(); padding_p->Enable();
        k_max_ctrl_m->Enable(); k_max_ctrl_p->Enable();
        s_max_ctrl_m->Enable(); s_max_ctrl_p->Enable();
        a_max_ctrl_m->Enable(); a_max_ctrl_p->Enable();
    }
}

//######################
//   FDTD_Mode_Dialog
//######################

FDTD_Mode_Dialog::FDTD_Mode_Dialog(GUI::FDTD_Mode *data_,int target_panel)
    :wxDialog(0,wxID_ANY,"FDTD Parameters",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     data(data_),
     new_structure(false),
     N_panels(0)
{
    force_xy_periodic=(data->type==FDTD_Mode::FDTD_NORMAL || 
                       data->type==FDTD_Mode::FDTD_OBLIQUE_ARS);
    
    wxNotebook *book=new wxNotebook(this,wxID_ANY);
    
    // Tabs creation
    
    FDTD_Mode_Dialog_Main(book,target_panel);
    FDTD_Mode_Dialog_Structure(book,target_panel);
    FDTD_Mode_Dialog_Incidence(book,target_panel);
    FDTD_Mode_Dialog_Materials(book,target_panel);
    FDTD_Mode_Dialog_Boundaries(book,target_panel);
    
    //
    
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    buttons_sizer->Add(ok_btn,wxSizerFlags().Border(wxALL,2));
    buttons_sizer->Add(cancel_btn,wxSizerFlags().Border(wxALL,2));
    
    ok_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_ok,this);
    cancel_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_cancel,this);
    
    top_sizer->Add(book,wxSizerFlags(1).Expand());
    top_sizer->Add(buttons_sizer,wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxRIGHT,10));
    
    SetSizer(top_sizer);
    
    // Global Bindings
    
    Bind(EVT_PLIST_DOWN,&FDTD_Mode_Dialog::evt_material_change,this);
    Bind(EVT_PLIST_RESIZE,&FDTD_Mode_Dialog::evt_material_change,this);
    Bind(EVT_PLIST_REMOVE,&FDTD_Mode_Dialog::evt_material_change,this);
    Bind(EVT_PLIST_UP,&FDTD_Mode_Dialog::evt_material_change,this);
    
    //
    
    ShowModal();
}

void FDTD_Mode_Dialog::FDTD_Mode_Dialog_Boundaries(wxNotebook *book,int target_panel)
{
    boundaries_panel=new wxScrolledWindow(book);
    
    boundary_x=nullptr;
    boundary_y=nullptr;
    boundary_z=nullptr;
    
    if(!force_xy_periodic)
    {
        boundary_x=new FD_Boundary_Panel(boundaries_panel,"X",data->periodic_x,
                                         data->pml_xm,data->kappa_xm,data->sigma_xm,data->alpha_xm,data->pad_xm,
                                         data->pml_xp,data->kappa_xp,data->sigma_xp,data->alpha_xp,data->pad_xp);
                                         
        boundary_y=new FD_Boundary_Panel(boundaries_panel,"Y",data->periodic_y,
                                         data->pml_ym,data->kappa_ym,data->sigma_ym,data->alpha_ym,data->pad_ym,
                                         data->pml_yp,data->kappa_yp,data->sigma_yp,data->alpha_yp,data->pad_yp);
    }
        
    boundary_z=new FD_Boundary_Panel(boundaries_panel,"Z",data->periodic_z,
                                     data->pml_zm,data->kappa_zm,data->sigma_zm,data->alpha_zm,data->pad_zm,
                                     data->pml_zp,data->kappa_zp,data->sigma_zp,data->alpha_zp,data->pad_zp);
    
    // Sizers
    
    wxBoxSizer *boundaries_sizer=new wxBoxSizer(wxVERTICAL);
    
    if(!force_xy_periodic)
    {
        boundaries_sizer->Add(boundary_x,wxSizerFlags().Expand());
        boundaries_sizer->Add(boundary_y,wxSizerFlags().Expand());
    }
    
    boundaries_sizer->Add(boundary_z,wxSizerFlags().Expand());
    
    boundaries_panel->SetScrollbars(1,1,50,50);
    boundaries_panel->SetSizerAndFit(boundaries_sizer);
    
    book->AddPage(boundaries_panel,"Boundaries");
    
    if(target_panel==4) book->SetSelection(N_panels); N_panels++;
}

void FDTD_Mode_Dialog::FDTD_Mode_Dialog_Incidence(wxNotebook *book,int target_panel)
{
    if(data->type!=FDTD_Mode::FDTD_CUSTOM)
    {
        wxPanel *incidence_panel=new wxPanel(book);
        
        wxBoxSizer *incidence_sizer=new wxBoxSizer(wxVERTICAL);
        
        // - Polarization
        
        wxBoxSizer *polarization_sizer=new wxBoxSizer(wxHORIZONTAL);
        
        polarization=new wxChoice(incidence_panel,wxID_ANY);
        polarization->Append("TE");
        polarization->Append("TM");
        
        if(data->polarization=="TE") polarization->SetSelection(0);
        else polarization->SetSelection(1);
        
        polarization_sizer->Add(new wxStaticText(incidence_panel,wxID_ANY,"Polarization: "),
                                wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        polarization_sizer->Add(polarization);
        
        incidence_sizer->Add(polarization_sizer,wxSizerFlags().Border(wxALL,2).Align(wxALIGN_LEFT));
        
        // - Spectrum
        
        inc_spectrum=new SpectrumSelector(incidence_panel,data->lambda_min,data->lambda_max,data->Nl);
        
        incidence_sizer->Add(inc_spectrum,wxSizerFlags().Expand().Border(wxALL,2));
        
        incidence_panel->SetSizer(incidence_sizer);
        
        book->AddPage(incidence_panel,"Incidence");
        
        if(target_panel==2) book->SetSelection(N_panels); N_panels++;
    }
}

void FDTD_Mode_Dialog::FDTD_Mode_Dialog_Main(wxNotebook *book,int target_panel)
{
    wxPanel *main_panel=new wxPanel(book);
    
    wxString fdtd_type_str("FDTD Type: ");
    
         if(data->type==FDTD_Mode::FDTD_CUSTOM) fdtd_type_str<<"Custom";
    else if(data->type==FDTD_Mode::FDTD_NORMAL) fdtd_type_str<<"Normal Incidence";
    else if(data->type==FDTD_Mode::FDTD_OBLIQUE_ARS) fdtd_type_str<<"Bloch Oblique Incidence";
    else if(data->type==FDTD_Mode::FDTD_SINGLE_PARTICLE) fdtd_type_str<<"Single Particle";
    
    // Name & dir
    
    wxStaticText *fdtd_type_static=new wxStaticText(main_panel,wxID_ANY,fdtd_type_str);
    name=new NamedTextCtrl<std::string>(main_panel,"Name: ",data->prefix);
    if(data->type==FDTD_Mode::FDTD_CUSTOM) name->Hide();
    
    wxBoxSizer *dir_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    output_directory=new NamedTextCtrl<std::string>(main_panel,"Output directory: ",data->directory.generic_string());
    output_directory->lock();
    
    wxButton *output_directory_btn=new wxButton(main_panel,wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    output_directory_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_output_directory,this);
    
    dir_sizer->Add(output_directory,wxSizerFlags(1));
    dir_sizer->Add(output_directory_btn,wxSizerFlags().Expand());
    
    // Iterations
    
    wxStaticBoxSizer *time_sizer=new wxStaticBoxSizer(wxVERTICAL,main_panel,"Time Control");
    
    Nt_max=new NamedTextCtrl<int>(main_panel,"Nt max: ",data->Nt);
    
    Nt_type=new wxChoice(main_panel,wxID_ANY);
    Nt_type->Append("Fixed");
    Nt_type->Append("Adaptive");
    Nt_type->SetSelection(0);
    Nt_type->Bind(wxEVT_CHOICE,&FDTD_Mode_Dialog::evt_time,this);
    
    Nt_layout_x=new wxChoice(main_panel,wxID_ANY);
    Nt_layout_x->Append("None");
    Nt_layout_x->Append("-X");
    Nt_layout_x->Append("+X");
    Nt_layout_x->Append("Both");
    Nt_layout_y=new wxChoice(main_panel,wxID_ANY);
    Nt_layout_y->Append("None");
    Nt_layout_y->Append("-Y");
    Nt_layout_y->Append("+Y");
    Nt_layout_y->Append("Both");
    Nt_layout_z=new wxChoice(main_panel,wxID_ANY);
    Nt_layout_z->Append("None");
    Nt_layout_z->Append("-Z");
    Nt_layout_z->Append("+Z");
    Nt_layout_z->Append("Both");
    
    Nt_layout_x->SetSelection(0);
    Nt_layout_y->SetSelection(0);
    Nt_layout_z->SetSelection(0);
    
    if(data->cc_layout.size()==3)
    {
        wxChoice* holder[3];
        holder[0]=Nt_layout_x; holder[1]=Nt_layout_y; holder[2]=Nt_layout_z;
        
        for(int i=0;i<3;i++)
        {
                 if(data->cc_layout[i]=='d') holder[i]->SetSelection(1);
            else if(data->cc_layout[i]=='u') holder[i]->SetSelection(2);
            else if(data->cc_layout[i]=='b') holder[i]->SetSelection(3);
        }
    }
    
    wxBoxSizer *time_sizer_1=new wxBoxSizer(wxHORIZONTAL);
    time_sizer_1->Add(Nt_max,wxSizerFlags().Border(wxRIGHT,10));
    time_sizer_1->Add(new wxStaticText(main_panel,wxID_ANY," Convergence: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    time_sizer_1->Add(Nt_type,wxSizerFlags().Border(wxRIGHT,10));
    time_sizer_1->Add(new wxStaticText(main_panel,wxID_ANY," Layout: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    time_sizer_1->Add(Nt_layout_x);
    time_sizer_1->Add(Nt_layout_y);
    time_sizer_1->Add(Nt_layout_z);
    
    Nt_lmin=new WavelengthSelector(main_panel,"Lambda Min",data->cc_lmin);
    Nt_lmax=new WavelengthSelector(main_panel,"Lambda Max",data->cc_lmax);
    Nt_level=new NamedTextCtrl<double>(main_panel,"Level",-std::log10(data->cc_coeff),true);
    Nt_points=new NamedTextCtrl<int>(main_panel,"N Points",data->cc_quant,true);
    
    wxBoxSizer *time_sizer_2=new wxBoxSizer(wxHORIZONTAL);
    time_sizer_2->Add(Nt_lmin,wxSizerFlags().Border(wxALL,2));
    time_sizer_2->Add(Nt_lmax,wxSizerFlags().Border(wxALL,2));
    time_sizer_2->Add(Nt_level,wxSizerFlags().Border(wxALL,2));
    time_sizer_2->Add(Nt_points,wxSizerFlags().Border(wxALL,2));
    
    time_sizer->Add(time_sizer_1,wxSizerFlags().Expand());
    time_sizer->Add(time_sizer_2,wxSizerFlags().Expand());
    
    if(data->time_type==TIME_FIXED)
    {
        Nt_type->SetSelection(0);
        layout_fixed_time();
    }
    else if(data->time_type==TIME_FT)
    {
        Nt_type->SetSelection(1);
        layout_adaptive_time();
    }
    
    wxBoxSizer *main_sizer=new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(fdtd_type_static,wxSizerFlags().Border(wxALL,2));
    main_sizer->Add(new wxStaticLine(main_panel),wxSizerFlags().Expand());
    main_sizer->Add(name,wxSizerFlags().Border(wxALL,2));
    main_sizer->Add(dir_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    if(data->type!=FDTD_Mode::FDTD_CUSTOM) main_sizer->Add(new wxStaticLine(main_panel),wxSizerFlags().Expand());
    main_sizer->Add(time_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    
    main_panel->SetSizer(main_sizer);
    book->AddPage(main_panel,"Main");
    
    if(target_panel==0) book->SetSelection(N_panels);
    N_panels++;
}

void FDTD_Mode_Dialog::FDTD_Mode_Dialog_Materials(wxNotebook *book,int target_panel)
{
    mats_panel=new wxScrolledWindow(book);
    
    mats_list=new PanelsList<>(mats_panel);
    
    for(unsigned int i=0;i<data->g_materials.size();i++)
    {
        FDTD_Material_Panel *panel=mats_list->add_panel<FDTD_Material_Panel>((data->g_materials)[i]);
        panel->set_title("Material "+std::to_string(i));
    }
    
    wxButton *mats_add_btn=new wxButton(mats_panel,wxID_ANY,"Add Material");
    mats_add_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_add_material,this);
    
    wxBoxSizer *mats_sizer=new wxBoxSizer(wxHORIZONTAL);
    mats_sizer->Add(mats_list,wxSizerFlags(1).Expand().Border(wxALL,2));
    mats_sizer->Add(mats_add_btn,wxSizerFlags().Border(wxALL,2));
    
    mats_panel->SetScrollbars(1,1,50,50);
    mats_panel->SetSizerAndFit(mats_sizer);
    
    book->AddPage(mats_panel,"Materials");
    
    if(target_panel==3) book->SetSelection(N_panels); N_panels++;
}

void FDTD_Mode_Dialog::FDTD_Mode_Dialog_Structure(wxNotebook *book,int target_panel)
{
    structure_panel=new wxScrolledWindow(book);
    wxBoxSizer *structure_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - Script
    
    wxStaticBoxSizer *structure_script_sizer=new wxStaticBoxSizer(wxHORIZONTAL,structure_panel,"Script");
    
    structure=new NamedTextCtrl<std::string>(structure_script_sizer->GetStaticBox(),"File: ",data->structure->script.generic_string());
    wxButton *structure_load_btn=new wxButton(structure_panel,wxID_ANY,"Load");
    structure_edit_btn=new wxButton(structure_panel,wxID_ANY,"Edit");
    structure_edit_btn->Disable();
    
    structure->lock();
    structure_load_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_load_structure,this);
    structure_edit_btn->Bind(wxEVT_BUTTON,&FDTD_Mode_Dialog::evt_edit_structure,this);
    
    structure_script_sizer->Add(structure,wxSizerFlags(1));
    structure_script_sizer->Add(structure_load_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    structure_script_sizer->Add(structure_edit_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    
    structure_sizer->Add(structure_script_sizer,wxSizerFlags().Expand());
    
    // - Discretization
    
    wxBoxSizer *bottom_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxStaticBoxSizer *structure_discr_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Discretization");
    dx_ctrl=new LengthSelector(structure_discr_sizer->GetStaticBox(),"Dx: ",data->Dx);
    dy_ctrl=new LengthSelector(structure_discr_sizer->GetStaticBox(),"Dy: ",data->Dy);
    dz_ctrl=new LengthSelector(structure_discr_sizer->GetStaticBox(),"Dz: ",data->Dz);
    
    dx_ctrl->Bind(EVT_LENGTH_SELECTOR,&FDTD_Mode_Dialog::evt_structure_parameter,this);
    dy_ctrl->Bind(EVT_LENGTH_SELECTOR,&FDTD_Mode_Dialog::evt_structure_parameter,this);
    dz_ctrl->Bind(EVT_LENGTH_SELECTOR,&FDTD_Mode_Dialog::evt_structure_parameter,this);
    
    structure_discr_sizer->Add(dx_ctrl,wxSizerFlags().Expand());
    structure_discr_sizer->Add(dy_ctrl,wxSizerFlags().Expand());
    structure_discr_sizer->Add(dz_ctrl,wxSizerFlags().Expand());
    
    bottom_sizer->Add(structure_discr_sizer);
    
    // - Parameters
    
    parameters_panel=new wxPanel(structure_panel);
    parameters_sizer=new wxStaticBoxSizer(wxVERTICAL,parameters_panel,"Parameters");
    
    if(data->structure->script.empty())
    {
        parameters_panel->Hide();
    }
    else
    {
        data->structure->finalize();
        parameter_names=data->structure->parameter_name;
        
        for(std::size_t i=0;i<data->structure->parameter_name.size();i++)
        {
            NamedTextCtrl<double> *param=new NamedTextCtrl(parameters_sizer->GetStaticBox(),
                                                           data->structure->parameter_name[i]+" : ",
                                                           data->structure->parameter_value[i]);
            param->Bind(EVT_NAMEDTXTCTRL,&FDTD_Mode_Dialog::evt_structure_parameter,this);
            
            parameters.push_back(param);
            parameters_sizer->Add(param,wxSizerFlags().Expand());
        }
    }
    
    parameters_panel->SetSizer(parameters_sizer);
    bottom_sizer->Add(parameters_panel);
    
    structure_sizer->Add(bottom_sizer);
    
    structure_panel->SetSizer(structure_sizer);
    book->AddPage(structure_panel,"Structure");
    
    
    if(target_panel==1) book->SetSelection(N_panels); N_panels++;
}

//----------------------
//   Member functions
//----------------------

void FDTD_Mode_Dialog::evt_add_material(wxCommandEvent &event)
{
    GUI::Material *material=MaterialsLib::request_material(MatType::REAL_N);
    
    PanelsListBase *panel=mats_list->add_panel<FDTD_Material_Panel>(material);
    // TODO material->original_requester=panel;
    
    std::string title("Material ");
    title.append(std::to_string(mats_list->get_size()-1));
    panel->set_title(title);
    
    mats_panel->FitInside();
    
    event.Skip();
}

void FDTD_Mode_Dialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void FDTD_Mode_Dialog::evt_edit_structure(wxCommandEvent &event)
{
//    wxFileName fname(structure->get_value());
//    
//    EMGeometry_Frame *mfr=new EMGeometry_Frame("Aether",fname);
    
    event.Skip();
}

void FDTD_Mode_Dialog::evt_load_structure(wxCommandEvent &event)
{
    wxFileName fname;
    fname=wxFileSelector("Load structure script",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         "ageom",
                         "Aether geometry file (*.ageom)|*.ageom|Lua scripts (*.lua)|*.lua",
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if(fname.IsOk()==false) return;
    
    fname.MakeRelativeTo();
    structure->set_value(fname.GetFullPath().ToStdString());
    
    new_structure=true;
    
    std::filesystem::path struct_path=structure->get_value();
    struct_path=std::filesystem::absolute(struct_path);
    
    // Cleanup
    
    for(NamedTextCtrl<double> *param : parameters)
        param->Destroy();
    
    parameters.clear();
    
    // Loading the new parameters
    
    Structure loader(struct_path);
    loader.finalize();
    
    parameter_names=loader.parameter_name;
    
    for(std::size_t i=0;i<loader.parameter_name.size();i++)
    {
        NamedTextCtrl<double> *param=new NamedTextCtrl(parameters_sizer->GetStaticBox(),
                                                       loader.parameter_name[i]+" : ",
                                                       loader.parameter_value[i]);
        param->Bind(EVT_NAMEDTXTCTRL,&FDTD_Mode_Dialog::evt_structure_parameter,this);
        
        parameters.push_back(param);
        parameters_sizer->Add(param,wxSizerFlags().Expand());
    }
    
    if(loader.parameter_name.empty())
    {
        parameters_panel->Hide();
    }
    else
    {
        parameters_panel->Show();
    }
    
    structure_panel->FitInside();
    
    event.Skip();
}

void FDTD_Mode_Dialog::evt_material_change(wxCommandEvent &event)
{
    mats_panel->FitInside();
    
    rename_materials();
    
    event.Skip();
}


void FDTD_Mode_Dialog::evt_structure_parameter(wxCommandEvent &event)
{
    new_structure=true;
}


void FDTD_Mode_Dialog::evt_ok(wxCommandEvent &event)
{
    // Main
    
    data->prefix=name->get_value();
    data->directory=output_directory->get_value();
    
    data->Nt=Nt_max->get_value();
    
    int selection_type=Nt_type->GetSelection();
    
         if(selection_type==0) data->time_type=TIME_FIXED;
    else if(selection_type==1) data->time_type=TIME_FT;
    
    int selection_layout[3];
    selection_layout[0]=Nt_layout_x->GetSelection();
    selection_layout[1]=Nt_layout_y->GetSelection();
    selection_layout[2]=Nt_layout_z->GetSelection();
    
    std::string layout_str("nnn");
    
    for(int i=0;i<3;i++)
    {
             if(selection_layout[i]==0) layout_str[i]='n';
        else if(selection_layout[i]==1) layout_str[i]='d';
        else if(selection_layout[i]==2) layout_str[i]='u';
        else if(selection_layout[i]==3) layout_str[i]='b';
    }
    
    data->cc_layout=layout_str;
    data->cc_lmin=Nt_lmin->get_lambda();
    data->cc_lmax=Nt_lmax->get_lambda();
    data->cc_coeff=std::pow(10.0,-Nt_level->get_value());
    data->cc_quant=Nt_points->get_value();
    
    // Incidence
    
    if(data->type!=FDTD_Mode::FDTD_CUSTOM)
    {
        if(polarization->GetSelection()==0) data->polarization="TE";
        else data->polarization="TM";
        
        data->lambda_min=inc_spectrum->get_lambda_min();
        data->lambda_max=inc_spectrum->get_lambda_max();
        data->Nl=inc_spectrum->get_Np();
    }
    
    // Structure
    data->structure->script=structure->get_value();
    data->structure->script=std::filesystem::absolute(data->structure->script);
        
    data->structure->parameter_name.clear();
    data->structure->parameter_value.clear();
    
    for(std::size_t i=0;i<parameters.size();i++)
    {
        data->structure->parameter_name.push_back(parameter_names[i]);
        data->structure->parameter_value.push_back(parameters[i]->get_value());
    }
    
    // Materials
    
    unsigned int N_mats=mats_list->get_size();
    
    data->g_materials.resize(N_mats);
    
    for(unsigned int i=0;i<N_mats;i++)
    {
        FDTD_Material_Panel *panel=dynamic_cast<FDTD_Material_Panel*>(mats_list->get_panel(i));
        (data->g_materials)[i]=panel->get_material();
    }
    
    // Boundaries
    
    if(!force_xy_periodic)
    {
        boundary_x->get_parameters(data->periodic_x,
                                   data->pml_xm,data->kappa_xm,data->sigma_xm,data->alpha_xm,data->pad_xm,
                                   data->pml_xp,data->kappa_xp,data->sigma_xp,data->alpha_xp,data->pad_xp);
        
        boundary_y->get_parameters(data->periodic_y,
                                   data->pml_ym,data->kappa_ym,data->sigma_ym,data->alpha_ym,data->pad_ym,
                                   data->pml_yp,data->kappa_yp,data->sigma_yp,data->alpha_yp,data->pad_yp);
    }
    
    boundary_z->get_parameters(data->periodic_z,
                               data->pml_zm,data->kappa_zm,data->sigma_zm,data->alpha_zm,data->pad_zm,
                               data->pml_zp,data->kappa_zp,data->sigma_zp,data->alpha_zp,data->pad_zp);
    
    Close();
}

void FDTD_Mode_Dialog::evt_output_directory(wxCommandEvent &event)
{
    wxDirDialog dialog(this,"Please choose a directory");
    dialog.ShowModal();
    
    wxString directory=dialog.GetPath();
    
    if(!directory.IsEmpty())
        output_directory->set_value(directory.ToStdString());
}

void FDTD_Mode_Dialog::evt_time(wxCommandEvent &event)
{
    int selection=Nt_type->GetSelection();
    
    if(selection==0) layout_fixed_time();
    else if(selection==1) layout_adaptive_time();
    
    event.Skip();
}

void FDTD_Mode_Dialog::layout_adaptive_time()
{
    Nt_layout_x->Enable();
    Nt_layout_y->Enable();
    Nt_layout_z->Enable();
    
    Nt_lmin->Enable();
    Nt_lmax->Enable();
    Nt_level->Enable();
    Nt_points->Enable();
}

void FDTD_Mode_Dialog::layout_fixed_time()
{
    Nt_layout_x->Disable();
    Nt_layout_y->Disable();
    Nt_layout_z->Disable();
    
    Nt_lmin->Disable();
    Nt_lmax->Disable();
    Nt_level->Disable();
    Nt_points->Disable();
}

void FDTD_Mode_Dialog::rename_materials()
{
    unsigned int Nmat=mats_list->get_size();
    
    for(unsigned int i=0;i<Nmat;i++)
    {
        std::string title="Material "+std::to_string(i);
        
        mats_list->get_panel(i)->set_title(title);
    }
}

//#####################
//   FDTD_Run_Dialog
//#####################

FDTD_Run_Dialog::FDTD_Run_Dialog(wxWindow *parent,GUI::FDTD_Mode const &data)
    :wxDialog(parent,wxID_ANY,"Simulation",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size(),wxCAPTION),
     end_computation(false), computation_done(false), dsp(10000)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Display
    
    wxBoxSizer *time_sizer=new wxBoxSizer(wxVERTICAL);
    
    text_iterations=new NamedTextCtrl<std::string>(this,"Iteration: ","");
    text_time_spent=new NamedTextCtrl<std::string>(this,"Time spent: ","");
    text_time_remaing=new NamedTextCtrl<std::string>(this,"Time remaining: ","");
    text_time_total=new NamedTextCtrl<std::string>(this,"Total time: ","");
    
    text_iterations->lock();
    text_time_spent->lock();
    text_time_remaing->lock();
    text_time_total->lock();
    
    time_sizer->Add(text_iterations,wxSizerFlags().Expand().Border(wxALL,2));
    time_sizer->Add(text_time_spent,wxSizerFlags().Expand().Border(wxALL,2));
    time_sizer->Add(text_time_remaing,wxSizerFlags().Expand().Border(wxALL,2));
    time_sizer->Add(text_time_total,wxSizerFlags().Expand().Border(wxALL,2));
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    display=new GL_2D_display(this);
    
    display_sizer->Add(time_sizer);
    display_sizer->Add(display,wxSizerFlags(1).Expand());
    
    top_sizer->Add(display_sizer,wxSizerFlags(1).Expand());
    
    wxBoxSizer *progression_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxStaticText *progression_txt=new wxStaticText(this,wxID_ANY,"Progression: ");
    progression_ctrl=new wxGauge(this,wxID_ANY,100);
    
    progression_sizer->Add(progression_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    progression_sizer->Add(progression_ctrl,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
    
    top_sizer->Add(new wxStaticLine(this),wxSizerFlags().Expand());
    top_sizer->Add(progression_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    
    // Buttons
    
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    ok_btn=new wxButton(this,wxID_ANY,"Ok");
    ok_btn->Disable();
    
    cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    cancel_btn->Bind(wxEVT_BUTTON,&FDTD_Run_Dialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&FDTD_Run_Dialog::evt_ok,this);
    
    buttons_sizer->Add(ok_btn);
    buttons_sizer->Add(cancel_btn);
    
    top_sizer->Add(new wxStaticLine(this),wxSizerFlags().Expand());
    top_sizer->Add(buttons_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
    
    SetSizer(top_sizer);
    
    // Timer
    
    thread=new std::thread(&FDTD_Run_Dialog::run_computation,this,data);
    
    timer=new wxTimer(this);
    
    Bind(wxEVT_TIMER,&FDTD_Run_Dialog::evt_timed_refresh,this);
    timer->Start(1000/2.0);
    
    ShowModal();
}

FDTD_Run_Dialog::~FDTD_Run_Dialog()
{
    timer->Stop();
    delete thread;
}

void FDTD_Run_Dialog::evt_cancel(wxCommandEvent &event)
{
    timer->Stop();
    end_computation=true;
    thread->join();
    
    Close();
}

void FDTD_Run_Dialog::evt_ok(wxCommandEvent &event)
{
    Close();
}

void FDTD_Run_Dialog::evt_timed_refresh(wxTimerEvent &event)
{
    wxString tmp_str;
    
    text_iterations->set_value(std::to_string(dsp.count));
    text_time_spent->set_value(dsp.time_spent);
    text_time_remaing->set_value(dsp.time_remaining);
    text_time_total->set_value(dsp.time_total);
    
    progression_ctrl->SetValue(static_cast<int>(dsp.completion_percent));
    
    display->refresh_from_bitmap();
    
    if(computation_done)
    {
        timer->Stop();
        thread->join();
        ok_btn->Enable();
        cancel_btn->Disable();
    }
}

void FDTD_Run_Dialog::run_computation(GUI::FDTD_Mode const &data)
{
         if(data.type==FDTD_Mode::FDTD_CUSTOM) mode_default_fdtd(data,&end_computation,&dsp,&display->bitmap);
    else if(data.type==FDTD_Mode::FDTD_NORMAL) FDTD_normal_incidence(data,&end_computation,&dsp,&display->bitmap);
    else if(data.type==FDTD_Mode::FDTD_OBLIQUE_ARS) FDTD_oblique_biphase(data,&end_computation,&dsp);
    else if(data.type==FDTD_Mode::FDTD_SINGLE_PARTICLE) FDTD_single_particle(data,&end_computation,&dsp,&display->bitmap);
    
    computation_done=true;
}
