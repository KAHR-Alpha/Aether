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

#include <fdfd.h>
#include <filehdl.h>

#include <gui_fd_ms.h>
#include <gui_multilayers_utils.h>
#include <gui_rsc.h>

#include <wx/splitter.h>


void gui_pml_maker(wxWindow *w,wxStaticBoxSizer *sizer,
                   NamedTextCtrl<int> **N,
                   NamedTextCtrl<double> **k,
                   NamedTextCtrl<double> **s,
                   NamedTextCtrl<double> **a)
{
    *N=new NamedTextCtrl<int>(w,"N: ",25);
    *k=new NamedTextCtrl<double>(w,"",25);
    *s=new NamedTextCtrl<double>(w,"",1);
    *a=new NamedTextCtrl<double>(w,"",0.2);
    
    wxBoxSizer *k_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *s_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *a_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    std::string kappa_16=PathManager::locate_resource("resources/kappa_16.png").generic_string();
    std::string sigma_16=PathManager::locate_resource("resources/sigma_16.png").generic_string();
    std::string alpha_16=PathManager::locate_resource("resources/alpha_16.png").generic_string();
    
    wxGenericStaticBitmap *k_bmp=new wxGenericStaticBitmap(w,wxID_ANY,ImagesManager::get_bitmap(kappa_16));
    wxGenericStaticBitmap *s_bmp=new wxGenericStaticBitmap(w,wxID_ANY,ImagesManager::get_bitmap(sigma_16));
    wxGenericStaticBitmap *a_bmp=new wxGenericStaticBitmap(w,wxID_ANY,ImagesManager::get_bitmap(alpha_16));
    
    k_sizer->Add(k_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    s_sizer->Add(s_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    a_sizer->Add(a_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    k_sizer->Add(*k,wxSizerFlags(1).Expand());
    s_sizer->Add(*s,wxSizerFlags(1).Expand());
    a_sizer->Add(*a,wxSizerFlags(1).Expand());
    
    sizer->Add(*N,wxSizerFlags().Expand());
    sizer->Add(k_sizer,wxSizerFlags().Expand());
    sizer->Add(s_sizer,wxSizerFlags().Expand());
    sizer->Add(a_sizer,wxSizerFlags().Expand());
}

FDSolverFrame::FDSolverFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    structure_panel=new wxScrolledWindow(splitter);
    display_panel=new wxPanel(splitter);
    
    // Structure Panel
    
    wxBoxSizer *structure_sizer=new wxBoxSizer(wxVERTICAL);
    
    Dz_ctrl=new NamedTextCtrl<double>(structure_panel,"Dz: ",5e-9);
    
    structure_sizer->Add(Dz_ctrl,std_flags);
    
    wxStaticBoxSizer *pml_u_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"PML Up");
    wxStaticBoxSizer *pml_d_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"PML Down");
    
    structure_sizer->Add(pml_u_sizer,std_flags);
    structure_sizer->Add(pml_d_sizer,std_flags);
    
    gui_pml_maker(structure_panel,pml_u_sizer,
                  &N_zp_ctrl,&kmax_zp_ctrl,&smax_zp_ctrl,&amax_zp_ctrl);
    gui_pml_maker(structure_panel,pml_d_sizer,
                  &N_zm_ctrl,&kmax_zm_ctrl,&smax_zm_ctrl,&amax_zm_ctrl);
    
    wxStaticBoxSizer *superstrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Superstrate");
    wxStaticBoxSizer *layers_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Layers");
    wxStaticBoxSizer *substrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Substrate");
    
    pad_zp_ctrl=new NamedTextCtrl<double>(structure_panel,"Padding: ",250e-9);
    pad_zm_ctrl=new NamedTextCtrl<double>(structure_panel,"Padding: ",250e-9);
    
    superstrate_selector=new MiniMaterialSelector(structure_panel);
    wxButton *add_layer_btn=new wxButton(structure_panel,wxID_ANY,"Add Layer");
    layers_list=new PanelsList<>(structure_panel);
    substrate_selector=new MiniMaterialSelector(structure_panel);
    
    superstrate_sizer->Add(superstrate_selector,wxSizerFlags().Expand());
    superstrate_sizer->Add(pad_zp_ctrl,wxSizerFlags().Expand());
    layers_sizer->Add(add_layer_btn,wxSizerFlags().Expand());
    layers_sizer->Add(layers_list,wxSizerFlags(1).Expand());
    substrate_sizer->Add(substrate_selector,wxSizerFlags().Expand());
    substrate_sizer->Add(pad_zm_ctrl,wxSizerFlags().Expand());
    
    structure_sizer->Add(superstrate_sizer,std_flags);
    structure_sizer->Add(layers_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    structure_sizer->Add(substrate_sizer,std_flags);
    
    structure_panel->SetSizer(structure_sizer);
    
    // Display Panel
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxString mode_choices[]={"Monochromatic","Polychromatic"};
    wxString polar_choices[]={"TE","TM"};
    
    wxRadioBox *mode_ctrl=new wxRadioBox(display_panel,wxID_ANY,"Mode",wxDefaultPosition,wxDefaultSize,2,mode_choices);
    
    mono_panel=new wxPanel(display_panel);
    multi_panel=new wxPanel(display_panel);
    
    // Mono
    
    wxBoxSizer *mono_panel_sizer=new wxBoxSizer(wxVERTICAL);
    mono_modes_sizer=new wxStaticBoxSizer(wxVERTICAL,mono_panel,"Modes Data");
    wxStaticBoxSizer *mono_N_sizer=new wxStaticBoxSizer(wxVERTICAL,mono_panel,"N Modes");
    wxBoxSizer *mono_data_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *mono_ctrl_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    mono_graph=new Graph(mono_panel);
    
    mono_data_sizer->Add(mono_modes_sizer,wxSizerFlags().Expand());
    mono_data_sizer->Add(mono_graph,wxSizerFlags(1).Expand());
    
    mono_selector=new WavelengthSelector(mono_panel,"Wavelength",500e-9);
    mono_polar=new wxRadioBox(mono_panel,wxID_ANY,"Polarization",wxDefaultPosition,wxDefaultSize,2,polar_choices);
    
    mono_N_ctrl=new NamedTextCtrl<int>(mono_panel,"",1);
    mono_N_sizer->Add(mono_N_ctrl);
    
    wxButton *mono_compute_btn=new wxButton(mono_panel,wxID_ANY,"Compute");
    wxButton *mono_display_btn=new wxButton(mono_panel,wxID_ANY,"Display");
    wxButton *mono_export_btn=new wxButton(mono_panel,wxID_ANY,"Export");
    
    mono_ctrl_sizer->Add(mono_selector,wxSizerFlags().Expand());
    mono_ctrl_sizer->Add(mono_polar,wxSizerFlags().Expand());
    mono_ctrl_sizer->Add(mono_N_sizer,wxSizerFlags().Expand());
    mono_ctrl_sizer->Add(mono_compute_btn,wxSizerFlags().Expand());
    mono_ctrl_sizer->Add(mono_display_btn,wxSizerFlags().Expand());
    mono_ctrl_sizer->Add(mono_export_btn,wxSizerFlags().Expand());
    
    mono_panel_sizer->Add(mono_data_sizer,wxSizerFlags(1).Expand());
    mono_panel_sizer->Add(mono_ctrl_sizer,wxSizerFlags().Expand());
    
    mono_panel->SetSizer(mono_panel_sizer);
    
    // Multi
    
    multi_panel->Hide();
    
    // Display Panel Wrapping
    
    display_sizer->Add(mode_ctrl,wxSizerFlags().Border(wxALL,4));
    display_sizer->Add(mono_panel,wxSizerFlags(1).Expand());
    display_sizer->Add(multi_panel,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    splitter->SplitVertically(structure_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    // Bindings
    
    add_layer_btn->Bind(wxEVT_BUTTON,&FDSolverFrame::evt_add_layer,this);
    
    mono_compute_btn->Bind(wxEVT_BUTTON,&FDSolverFrame::evt_mono_compute,this);
        
    Bind(EVT_PLIST_DOWN,&FDSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_UP,&FDSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_RESIZE,&FDSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_REMOVE,&FDSolverFrame::evt_panels_change,this);
    
    structure_panel->SetScrollbars(10,10,50,50);
    structure_panel->FitInside();
}

void FDSolverFrame::evt_add_layer(wxCommandEvent &event)
{
    layers_list->add_panel<LayerPanel>();
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
}

void FDSolverFrame::evt_mono_compute(wxCommandEvent &event)
{
    chk_var(N_zm_ctrl->get_value());
    chk_var(kmax_zm_ctrl->get_value());
    chk_var(smax_zm_ctrl->get_value());
    chk_var(amax_zm_ctrl->get_value());
    
    int k,l;
    double tot_height=pad_zp_ctrl->get_value()+pad_zm_ctrl->get_value();
    
    int Nl=layers_list->get_size();
    unsigned int Nmat=2+Nl;
    
    Grid1<double> lim_down(Nl+1,0),lim_up(Nl+1,0);
    lim_up[0]=pad_zp_ctrl->get_value();
    
    Grid1<Material> tmp_materials(Nmat);
    tmp_materials[0]=*(superstrate_selector->get_material());
    tmp_materials[Nmat-1]=*(substrate_selector->get_material());
    
    for(k=0;k<Nl;k++)
    {
        LayerPanel *panel=reinterpret_cast<LayerPanel*>(layers_list->get_panel(k));
        double tmp_height=panel->get_height();
        
        lim_down[k+1]=lim_up[k];
        lim_up[k+1]=lim_down[k+1]+tmp_height;
        
        tot_height+=tmp_height;
        
        tmp_materials[k+1]=*(panel->get_material());
    }
    
    lim_down.show();
    lim_up.show();
    
    double Dz=Dz_ctrl->get_value();
    int Nz=static_cast<int>(tot_height/Dz+0.5);
    
    Grid3<unsigned int> matsgrid(1,1,Nz,Nmat-1);
    
    for(k=0;k<Nz;k++)
    {
        double z=k*Dz;
        
        for(l=0;l<Nl+1;l++) 
            if(z>=lim_down[l] && z<lim_up[l])
            {
                matsgrid(0,0,k)=l;
                break;
            }
    }
    
    FDFD fdfd(Dz,Dz,Dz);
    
    fdfd.set_pml_zm(N_zm_ctrl->get_value(),
                    kmax_zm_ctrl->get_value(),
                    smax_zm_ctrl->get_value(),
                    amax_zm_ctrl->get_value());
    
    fdfd.set_pml_zp(N_zp_ctrl->get_value(),
                    kmax_zp_ctrl->get_value(),
                    smax_zp_ctrl->get_value(),
                    amax_zp_ctrl->get_value());
    
    fdfd.set_matsgrid(matsgrid);
    fdfd.set_materials(tmp_materials);
    
    #ifdef FD_ARMA_MODES_SOLVER
    arma::Col<Imdouble> n_eff;
    arma::Mat<double> tmp_E;
    
    fdfd.solve_modes_1D(mono_selector->get_lambda(),
                        1,mono_N_ctrl->get_value(),
                        n_eff,tmp_E);
    
    n_eff.print();
    #endif
}

void FDSolverFrame::evt_panels_change(wxCommandEvent &event)
{
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
}

void FDSolverFrame::rename_panels()
{
    for(unsigned int n=0;n<layers_list->get_size();n++)
    {
        wxString title="Layer ";
        title<<n;
        
        layers_list->get_panel(n)->set_title(title);
    }
}
