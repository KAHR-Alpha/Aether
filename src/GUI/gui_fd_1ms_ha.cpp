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

#include <wx/splitter.h>

#include <gui_fd_ms.h>
#include <gui_rsc.h>

extern const Imdouble Im;
extern std::ofstream plog;

wxDEFINE_EVENT(EVT_MAP_REQUEST,MSMapEvent);
wxDEFINE_EVENT(EVT_MAP_READY,wxCommandEvent);
wxDEFINE_EVENT(EVT_NEW_TARGET,wxCommandEvent);

class MapThread: public wxThread
{
    public:
        HAPSolverFrame *frame;
        
        ExitCode Entry()
        {
            frame->compute_map();
            
            Delete();
            
            return 0;
        }
};

//#######################
//   MSTargetInfoPanel
//#######################

MSTargetInfoPanel::MSTargetInfoPanel(wxWindow *parent,
                                     int polarization_,double lambda_,
                                     MSTarget const &target_,Multilayer_TMM const &ml_)
    :wxPanel(parent),
     display_E(true),
     display_Ex(true),
     display_Ey(true),
     display_Ez(true),
     display_complex(true),
     polarization(polarization_),
     lambda(lambda_),
     target(target_),
     ml(ml_)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *data_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Data");
    
    // Data
    
    wxBoxSizer *real_n_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *imag_n_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *wvl_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *propag_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    std::stringstream real_n_strm;
    std::stringstream imag_n_strm;
    std::stringstream wvl_strm;
    std::stringstream propag_strm;
    
    real_n_strm<<target.nr;
    imag_n_strm<<target.ni;
    wvl_strm<<add_unit_u(lambda/target.nr);
    propag_strm<<add_unit_u(lambda/(2.0*Pi*target.ni));
    
    wxStaticText *real_n_txt=new wxStaticText(this,wxID_ANY,"Re(n_eff) :");
    wxStaticText *imag_n_txt=new wxStaticText(this,wxID_ANY,"Im(n_eff) :");
    wxStaticText *wvl_txt=new wxStaticText(this,wxID_ANY,"Eff Wavelength:");
    wxStaticText *propag_txt=new wxStaticText(this,wxID_ANY,"Eff Propagation (E):");
    
    real_n_disp=new wxTextCtrl(this,wxID_ANY,wxString(real_n_strm.str()),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    imag_n_disp=new wxTextCtrl(this,wxID_ANY,wxString(imag_n_strm.str()),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    wvl_disp=new wxTextCtrl(this,wxID_ANY,wxString(wvl_strm.str()),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    propag_disp=new wxTextCtrl(this,wxID_ANY,wxString(propag_strm.str()),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    
    real_n_sizer->Add(real_n_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    real_n_sizer->Add(real_n_disp,wxSizerFlags(1));
    
    imag_n_sizer->Add(imag_n_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    imag_n_sizer->Add(imag_n_disp,wxSizerFlags(1));
    
    wvl_sizer->Add(wvl_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    wvl_sizer->Add(wvl_disp,wxSizerFlags(1));
    
    propag_sizer->Add(propag_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    propag_sizer->Add(propag_disp,wxSizerFlags(1));
    
    data_sizer->Add(real_n_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    data_sizer->Add(imag_n_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    data_sizer->Add(wvl_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    data_sizer->Add(propag_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    
    top_sizer->Add(data_sizer);
    
    // Graph
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *display_E_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"Display");
    wxBoxSizer *display_ctrl_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    field_graph=new Graph(this);
    field_graph->switch_dark();
    
    wxString cpl_choices[]={"Complex","Abs"};
    
    ex_ctrl=new wxCheckBox(this,wxID_ANY,"Ex");
    ey_ctrl=new wxCheckBox(this,wxID_ANY,"Ey");
    ez_ctrl=new wxCheckBox(this,wxID_ANY,"Ez");
    e_ctrl=new wxCheckBox(this,wxID_ANY,"E");
    cpl_ctrl=new wxRadioBox(this,wxID_ANY,"Amplitude",wxDefaultPosition,wxDefaultSize,2,cpl_choices);
    
    if(polarization==0)
    {
        ex_ctrl->Disable();
        ey_ctrl->SetValue(true);
        ez_ctrl->Disable();
    }
    else
    {
        ex_ctrl->SetValue(true);
        ey_ctrl->Disable();
        ez_ctrl->SetValue(true);
    }
    e_ctrl->SetValue(true);
    
    display_E_sizer->Add(ex_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    display_E_sizer->Add(ey_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    display_E_sizer->Add(ez_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    display_E_sizer->Add(e_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    display_ctrl_sizer->Add(display_E_sizer,wxSizerFlags().Expand());
    display_ctrl_sizer->Add(cpl_ctrl,wxSizerFlags().Expand());
    
    display_sizer->Add(field_graph,wxSizerFlags(1).Expand());
    display_sizer->Add(display_ctrl_sizer,wxSizerFlags().Expand());
    
    top_sizer->Add(display_sizer,wxSizerFlags(1).Expand());
    
    SetSizer(top_sizer);
    
    Bind(wxEVT_RADIOBOX,&MSTargetInfoPanel::evt_cpl_switch,this);
    Bind(wxEVT_CHECKBOX,&MSTargetInfoPanel::evt_field_check,this);
    
    compute_field();
}

void MSTargetInfoPanel::compute_field()
{
    Imdouble n_eff=Imdouble(target.nr,target.ni);
    
         if(polarization==0) ml.compute_mode_TE(n_eff,holder);
    else if(polarization==1) ml.compute_mode_TM(n_eff,holder);
    
    holder.normalize();
    holder.invert_z();
    holder.complex_split();
    
    update_graph();
}

void MSTargetInfoPanel::change_target(MSTarget const &target_)
{
    target=target_;
    
    std::stringstream real_n_strm;
    std::stringstream imag_n_strm;
    std::stringstream wvl_strm;
    std::stringstream propag_strm;
    
    real_n_strm<<target.nr;
    imag_n_strm<<target.ni;
    wvl_strm<<add_unit_u(lambda/target.nr);
    propag_strm<<add_unit_u(lambda/(2.0*Pi*target.ni));
    
    real_n_disp->ChangeValue(wxString(real_n_strm.str()));
    imag_n_disp->ChangeValue(wxString(imag_n_strm.str()));
    wvl_disp->ChangeValue(wxString(wvl_strm.str()));
    propag_disp->ChangeValue(wxString(propag_strm.str()));
    
    compute_field();
    update_graph();
}

void MSTargetInfoPanel::evt_cpl_switch(wxCommandEvent &event)
{
    if(cpl_ctrl->GetSelection()==0) display_complex=true;
    else display_complex=false;
    
    update_graph();
}

void MSTargetInfoPanel::evt_field_check(wxCommandEvent &event)
{
    display_E=e_ctrl->GetValue();
    display_Ex=ex_ctrl->GetValue();
    display_Ey=ey_ctrl->GetValue();
    display_Ez=ez_ctrl->GetValue();
    
    update_graph();
}

void MSTargetInfoPanel::export_field(std::string fname)
{
    int l,k;
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc|std::ios::binary);
    
    for(l=0;l<holder.N_layers();l++)
    {
        for(k=0;k<holder.Nz(l);k++)
        {
            file<<holder.z(l,k)<<" "
                <<std::real(holder.Ex(l,k))<<" "
                <<std::imag(holder.Ex(l,k))<<" "
                <<std::abs(holder.Ex(l,k))<<" "
                <<std::real(holder.Ey(l,k))<<" "
                <<std::imag(holder.Ey(l,k))<<" "
                <<std::abs(holder.Ey(l,k))<<" "
                <<std::real(holder.Ez(l,k))<<" "
                <<std::imag(holder.Ez(l,k))<<" "
                <<std::abs(holder.Ez(l,k))<<" "
                <<std::abs(holder.E(l,k))<<" "<<std::endl;
        }
    }
}

void MSTargetInfoPanel::update_graph()
{
    int l;
    int Nl=holder.N_layers();
    
    field_graph->clear_graph();
    
    for(l=0;l<Nl;l++)
    {
        if(l==0)
        {
            if(display_E) field_graph->add_external_data(&holder.get_E(l),&holder.get_z(l),1,1,1,"abs(E)");
            if(display_complex)
            {
                if(polarization==0)
                {
                    if(display_Ey)
                    {
                        field_graph->add_external_data(&holder.get_Ey_real(l),&holder.get_z(l),0,1,0,"real(Ey)");
                        field_graph->add_external_data(&holder.get_Ey_imag(l),&holder.get_z(l),1,0,1,"imag(Ey)");
                    }
                }
                else
                {
                    if(display_Ex)
                    {
                        field_graph->add_external_data(&holder.get_Ex_real(l),&holder.get_z(l),1,0,0,"real(Ex)");
                        field_graph->add_external_data(&holder.get_Ex_imag(l),&holder.get_z(l),0,1,1,"imag(Ex)");
                    }
                    if(display_Ez)
                    {
                        field_graph->add_external_data(&holder.get_Ez_real(l),&holder.get_z(l),0,0,1,"real(Ez)");
                        field_graph->add_external_data(&holder.get_Ez_imag(l),&holder.get_z(l),1,1,0,"imag(Ez)");
                    }
                }
            }
            else
            {
                if(display_Ey && polarization==0) field_graph->add_external_data(&holder.get_Ey_abs(l),&holder.get_z(l),0,1,0,"abs(Ey)");
                if(polarization==1)
                {
                    if(display_Ex) field_graph->add_external_data(&holder.get_Ex_abs(l),&holder.get_z(l),1,0,0,"abs(Ex)");
                    if(display_Ez) field_graph->add_external_data(&holder.get_Ez_abs(l),&holder.get_z(l),0,0,1,"abs(Ez)");
                }
            }
        }
        else
        {
            if(display_E) field_graph->add_external_data(&holder.get_E(l),&holder.get_z(l),1,1,1);
            if(display_complex)
            {
                if(polarization==0)
                {
                    if(display_Ey)
                    {
                        field_graph->add_external_data(&holder.get_Ey_real(l),&holder.get_z(l),0,1,0);
                        field_graph->add_external_data(&holder.get_Ey_imag(l),&holder.get_z(l),1,0,1);
                    }
                }
                else
                {
                    if(display_Ex)
                    {
                        field_graph->add_external_data(&holder.get_Ex_real(l),&holder.get_z(l),1,0,0);
                        field_graph->add_external_data(&holder.get_Ex_imag(l),&holder.get_z(l),0,1,1);
                    }
                    if(display_Ez)
                    {
                        field_graph->add_external_data(&holder.get_Ez_real(l),&holder.get_z(l),0,0,1);
                        field_graph->add_external_data(&holder.get_Ez_imag(l),&holder.get_z(l),1,1,0);
                    }
                }
            }
            else
            {
                if(display_Ey && polarization==0) field_graph->add_external_data(&holder.get_Ey_abs(l),&holder.get_z(l),0,1,0);
                if(polarization==1)
                {
                    if(display_Ex) field_graph->add_external_data(&holder.get_Ex_abs(l),&holder.get_z(l),1,0,0);
                    if(display_Ez) field_graph->add_external_data(&holder.get_Ez_abs(l),&holder.get_z(l),0,0,1);
                }
            }
        }
    }
    
    field_graph->autoscale();
}

//##########################
//   MSSingleTargetDialog
//##########################

MSSingleTargetDialog::MSSingleTargetDialog(int px,int py,int sx,int sy,
                                           int polarization,double lambda,
                                           MSTarget const &target,Multilayer_TMM const &ml)
    :wxDialog(NULL,wxID_ANY,"Target Information",wxPoint(px,py),wxSize(sx,sy),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX),
     confirm_target(false)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    target_panel=new MSTargetInfoPanel(this,polarization,lambda,target,ml);
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Delete");
    ok_btn->SetFocus();
    
    btn_sizer->Add(ok_btn,wxSizerFlags().Expand());
    btn_sizer->Add(cancel_btn,wxSizerFlags().Expand());
    
    top_sizer->Add(target_panel,wxSizerFlags(1).Expand());
    top_sizer->Add(btn_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&MSSingleTargetDialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&MSSingleTargetDialog::evt_ok,this);
    
    SetSizer(top_sizer);
    
    ShowModal();
}

void MSSingleTargetDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void MSSingleTargetDialog::evt_ok(wxCommandEvent &event)
{
    confirm_target=true;
    Close();
}

//#############################
//   MSMultipleTargetsDialog
//#############################

wxDEFINE_EVENT(EVT_TARGETS_DELETION,MSTargetDeletionEvent);

MSMultipleTargetsDialog::MSMultipleTargetsDialog(wxWindow *parent,
                                                 int px,int py,int sx,int sy,
                                                 int polarization,double lambda_,
                                                 std::vector<MSTarget> const &targets_,
                                                 Multilayer_TMM const &ml)
    :wxDialog(parent,wxID_ANY,"Target Information",wxPoint(px,py),wxSize(sx,sy),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX),
     current_target(0),
     lambda(lambda_),
     targets(targets_),
     deletion_flag(targets.size(),false)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *targets_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxStaticText *target_text=new wxStaticText(this,wxID_ANY,"Target: ");
    wxButton *prev_btn=new wxButton(this,wxID_ANY,"<",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    current_target_ctrl=new wxTextCtrl(this,wxID_ANY,"0",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    wxButton *next_btn=new wxButton(this,wxID_ANY,">",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    forget_target_ctrl=new wxCheckBox(this,wxID_ANY,"Forget Target");
    wxPanel *buffer=new wxPanel(this);
    wxButton *export_data_btn=new wxButton(this,wxID_ANY,"Export Data");
    wxButton *export_field_btn=new wxButton(this,wxID_ANY,"Export Field");
    
    targets_sizer->Add(target_text,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    targets_sizer->Add(prev_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    targets_sizer->Add(current_target_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    targets_sizer->Add(next_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    targets_sizer->Add(forget_target_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxLEFT,30)); // to be replaced with CenterVertical
    targets_sizer->Add(buffer,wxSizerFlags(1));
    targets_sizer->Add(export_data_btn,wxSizerFlags());
    targets_sizer->Add(export_field_btn,wxSizerFlags());
    
    target_panel=new MSTargetInfoPanel(this,polarization,lambda,targets[0],ml);
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    btn_sizer->Add(ok_btn,wxSizerFlags().Expand());
    btn_sizer->Add(cancel_btn,wxSizerFlags().Expand());
    
    top_sizer->Add(targets_sizer,wxSizerFlags().Expand().Border(wxALL,3));
    top_sizer->Add(target_panel,wxSizerFlags(1).Expand());
    top_sizer->Add(btn_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_cancel,this);
    current_target_ctrl->Bind(wxEVT_TEXT_ENTER,&MSMultipleTargetsDialog::evt_text_enter,this);
    current_target_ctrl->Bind(wxEVT_KILL_FOCUS,&MSMultipleTargetsDialog::evt_text_focus,this);
    export_data_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_export_data,this);
    export_field_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_export_field,this);
    forget_target_ctrl->Bind(wxEVT_CHECKBOX,&MSMultipleTargetsDialog::evt_forget_switch,this);
    next_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_next,this);
    ok_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_ok,this);
    prev_btn->Bind(wxEVT_BUTTON,&MSMultipleTargetsDialog::evt_previous,this);
    
    SetSizer(top_sizer);
    
    Show();
}

void MSMultipleTargetsDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void MSMultipleTargetsDialog::evt_export_data(wxCommandEvent &event)
{
    wxFileName data_tmp=wxFileSelector("Save data as",
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxFileSelectorDefaultWildcardStr,
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(data_tmp.IsOk()==false) return;
    
    std::ofstream file(data_tmp.GetFullPath().ToStdString(),
                       std::ios::out|std::ios::trunc|std::ios::binary);
    
    for(unsigned int i=0;i<targets.size();i++)
    {
        if(!deletion_flag[i])
        {
            file<<lambda<<" "<<targets[i].nr
                        <<" "<<targets[i].ni
                        <<" "<<lambda/targets[i].nr
                        <<" "<<lambda/(2.0*Pi*targets[i].ni)<<std::endl;
        }
    }
}

void MSMultipleTargetsDialog::evt_export_field(wxCommandEvent &event)
{
    wxFileName data_tmp=wxFileSelector("Save data as",
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxEmptyString,
                                       wxFileSelectorDefaultWildcardStr,
                                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(data_tmp.IsOk()==false) return;
    
    target_panel->export_field(data_tmp.GetFullPath().ToStdString());
}

void MSMultipleTargetsDialog::evt_forget_switch(wxCommandEvent &event)
{
    deletion_flag[current_target]=forget_target_ctrl->GetValue();
}

void MSMultipleTargetsDialog::evt_next(wxCommandEvent &event)
{
    current_target++;
    
    if(current_target>=targets.size()) current_target=0;
    
    update_target();
}

void MSMultipleTargetsDialog::evt_ok(wxCommandEvent &event)
{
    unsigned int i;
    
    bool c=false;
    for(i=0;i<deletion_flag.size();i++) if(deletion_flag[i])
    {
        c=true;
        break;
    }
        
    if(c)
    {
        MSTargetDeletionEvent event_out(EVT_TARGETS_DELETION);
        
        for(i=0;i<deletion_flag.size();i++)
        {
            if(deletion_flag[i])
                event_out.targets.push_back(targets[i]);
        }
        
        wxPostEvent(GetParent(),event_out);
    }
    
    Close();
}

void MSMultipleTargetsDialog::evt_previous(wxCommandEvent &event)
{
    if(current_target==0) current_target=targets.size()-1;
    else current_target--;
    
    update_target();
}

void MSMultipleTargetsDialog::evt_text_enter(wxCommandEvent &event)
{
    subevent_text();
}

void MSMultipleTargetsDialog::evt_text_focus(wxFocusEvent &event)
{
    subevent_text();
}

void MSMultipleTargetsDialog::subevent_text()
{
    std::stringstream strm;
    strm<<current_target_ctrl->GetValue().ToStdString();
    
    strm>>current_target;
    
    if(current_target>=targets.size()) current_target=targets.size()-1;
    
    update_target();
}

void MSMultipleTargetsDialog::update_target()
{
    std::stringstream strm;
    strm<<current_target;
    
    current_target_ctrl->ChangeValue(strm.str());
    target_panel->change_target(targets[current_target]);
    forget_target_ctrl->SetValue(deletion_flag[current_target]);
}

//#####################
//   HAPSolverFrame
//#####################

enum
{
    MENU_NEW,
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT
};

HAPSolverFrame::HAPSolverFrame(wxString const &title)
    :BaseFrame(title),
     autorefresh(true), auto_exposure(true),
     polarization(0),
     computing_map(false), reallocate_map(false),
     map_Nr(1024), map_Ni(1024),
     map_nr_min(0), map_nr_max(1),
     map_ni_min(-1), map_ni_max(1),
     map_data(nullptr)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    structure_panel=new wxScrolledWindow(splitter);
    display_panel=new wxPanel(splitter);
    
    // Structure Panel
    
    wxString polar_choices[]={"TE","TM"};
    
    wxBoxSizer *structure_sizer=new wxBoxSizer(wxVERTICAL);
    
    polar_ctrl=new wxRadioBox(structure_panel,wxID_ANY,"Polarization",wxDefaultPosition,wxDefaultSize,2,polar_choices);
    
    wxStaticBoxSizer *superstrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Superstrate");
    wxStaticBoxSizer *layers_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Layers");
    wxStaticBoxSizer *substrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Substrate");
    
    superstrate_selector=new MiniMaterialSelector(structure_panel);
    wxButton *add_layer_btn=new wxButton(structure_panel,wxID_ANY,"Add Layer");
    layers_list=new PanelsList<LayerPanelBase>(structure_panel);
    substrate_selector=new MiniMaterialSelector(structure_panel);
    
    superstrate_sizer->Add(superstrate_selector,wxSizerFlags().Expand());
    layers_sizer->Add(add_layer_btn,wxSizerFlags().Expand());
    layers_sizer->Add(layers_list,wxSizerFlags(1).Expand());
    substrate_sizer->Add(substrate_selector,wxSizerFlags().Expand());
    
    structure_sizer->Add(polar_ctrl,std_flags);
    structure_sizer->Add(superstrate_sizer,std_flags);
    structure_sizer->Add(layers_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    structure_sizer->Add(substrate_sizer,std_flags);
    
    structure_panel->SetSizer(structure_sizer);
    
    // Display Panel
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxString mode_choices[]={"Monochromatic","Polychromatic"};
    
    wxRadioBox *mode_ctrl=new wxRadioBox(display_panel,wxID_ANY,"Mode",wxDefaultPosition,wxDefaultSize,2,mode_choices);
    mode_ctrl->Disable();
    
    target_panel=new wxPanel(display_panel);
    multi_panel=new wxPanel(display_panel);
    
    // Target
    
    wxBoxSizer *target_panel_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *target_data_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *target_exposure_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *target_tools_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *target_ctrl_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxStaticBoxSizer *target_nr_sizer=new wxStaticBoxSizer(wxHORIZONTAL,target_panel,"Real n_eff");
    wxStaticBoxSizer *target_ni_sizer=new wxStaticBoxSizer(wxHORIZONTAL,target_panel,"Imag n_eff");
    
    // Target - Data
    
    wxPanel *target_tools_panel=new wxPanel(target_panel);
    wxPanel *target_exposure_panel=new wxPanel(target_panel);
    
    wxButton *target_new_btn=new wxButton(target_tools_panel,wxID_ANY,"",
                                          wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    target_refresh_btn=new wxToggleButton(target_tools_panel,wxID_ANY,"",
                                          wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
//    wxButton *target_gauss_btn=new wxButton(target_tools_panel,wxID_ANY,"G",
//                                            wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    target_new_btn->SetBitmap(ImagesManager::get_bitmap(PathManager::locate_resource("resources/target_2_32.png").generic_string()));
    target_refresh_btn->SetBitmap(ImagesManager::get_bitmap(PathManager::locate_resource("resources/refresh_32.png").generic_string()));
    target_refresh_btn->SetValue(true);
    
    target_tools_sizer->Add(target_new_btn);
    target_tools_sizer->Add(target_refresh_btn);
//    target_tools_sizer->Add(target_gauss_btn);
    
    target_tools_panel->SetSizer(target_tools_sizer);
    
    #ifndef WX30_RESTRICT
    wxGLAttributes attrib;
    attrib.PlatformDefaults().MinRGBA(8,8,8,8).DoubleBuffer().Depth(24).EndList();
    target_graph=new HAMS_GL(target_panel,attrib);
    #else
    target_graph=new HAMS_GL(target_panel);
    #endif
    
    exposure_slider=new wxSlider(target_exposure_panel,wxID_ANY,0,-400,400,wxDefaultPosition,wxDefaultSize,wxSL_VERTICAL);
    auto_exposure_btn=new wxToggleButton(target_exposure_panel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    auto_exposure_btn->SetBitmap(ImagesManager::get_bitmap(PathManager::locate_resource("resources/refresh_32.png").generic_string()));
    auto_exposure_btn->SetValue(true);
    
    target_exposure_sizer->Add(exposure_slider,wxSizerFlags(1).Align(wxALIGN_CENTER_HORIZONTAL)); // to be replaced with CenterHorizontal
    target_exposure_sizer->Add(auto_exposure_btn,wxSizerFlags().Align(wxALIGN_CENTER_HORIZONTAL)); // to be replaced with CenterHorizontal
    target_exposure_panel->SetSizer(target_exposure_sizer);
    
    target_data_sizer->Add(target_tools_panel,wxSizerFlags().Expand());
    target_data_sizer->Add(target_graph,wxSizerFlags(1).Expand());
    target_data_sizer->Add(target_exposure_panel,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Target - Controls
    
    lambda_t_ctrl=new WavelengthSelector(target_panel,"Wavelength",500e-9);
    
    nr_t_min_ctrl=new NamedTextCtrl<double>(target_panel,"",0,false,5);
    nr_t_max_ctrl=new NamedTextCtrl<double>(target_panel,"",1,false,5);
    wxStaticText *nr_t_pad=new wxStaticText(target_panel,wxID_ANY," : ");
    
    ni_t_min_ctrl=new NamedTextCtrl<double>(target_panel,"",-1,false,5);
    ni_t_max_ctrl=new NamedTextCtrl<double>(target_panel,"",1,false,5);
    wxStaticText *ni_t_pad=new wxStaticText(target_panel,wxID_ANY," : ");
    
    target_nr_sizer->Add(nr_t_min_ctrl);
    target_nr_sizer->Add(nr_t_pad,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    target_nr_sizer->Add(nr_t_max_ctrl);
    
    target_ni_sizer->Add(ni_t_min_ctrl);
    target_ni_sizer->Add(ni_t_pad,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    target_ni_sizer->Add(ni_t_max_ctrl);
    
    wxButton *t_reset_zoom_btn=new wxButton(target_panel,wxID_ANY,"Reset Zoom");
    wxButton *t_target_info_btn=new wxButton(target_panel,wxID_ANY,"Targets Info");
    wxButton *t_clear_targets_btn=new wxButton(target_panel,wxID_ANY,"Clear Targets");
    
    target_ctrl_sizer->Add(lambda_t_ctrl,wxSizerFlags().Expand());
    target_ctrl_sizer->Add(target_nr_sizer,wxSizerFlags().Expand());
    target_ctrl_sizer->Add(target_ni_sizer,wxSizerFlags().Expand());
    target_ctrl_sizer->Add(t_reset_zoom_btn,wxSizerFlags().Expand());
    target_ctrl_sizer->Add(t_target_info_btn,wxSizerFlags().Expand());
    target_ctrl_sizer->Add(t_clear_targets_btn,wxSizerFlags().Expand());
    
    target_panel_sizer->Add(target_data_sizer,wxSizerFlags(1).Expand());
    target_panel_sizer->Add(target_ctrl_sizer,wxSizerFlags().Expand());
    
    target_panel->SetSizer(target_panel_sizer);
    
    // Multi
    
    multi_panel->Hide();
    
    // Display Panel Wrapping
    
    display_sizer->Add(mode_ctrl,wxSizerFlags().Border(wxALL,4));
    display_sizer->Add(target_panel,wxSizerFlags(1).Expand());
    display_sizer->Add(multi_panel,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    splitter->SplitVertically(structure_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    // Menu
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    // Bindings
    
    add_layer_btn->Bind(wxEVT_BUTTON,&HAPSolverFrame::evt_add_layer,this);
    auto_exposure_btn->Bind(wxEVT_TOGGLEBUTTON,&HAPSolverFrame::evt_exposure_auto,this);
    exposure_slider->Bind(wxEVT_SLIDER,&HAPSolverFrame::evt_exposure,this);
    polar_ctrl->Bind(wxEVT_RADIOBOX,&HAPSolverFrame::evt_structure_change,this);
    target_refresh_btn->Bind(wxEVT_TOGGLEBUTTON,&HAPSolverFrame::evt_refresh_toggle,this);
    t_clear_targets_btn->Bind(wxEVT_BUTTON,&HAPSolverFrame::evt_clear_targets,this);
    t_reset_zoom_btn->Bind(wxEVT_BUTTON,&HAPSolverFrame::evt_reset_zoom,this);
    t_target_info_btn->Bind(wxEVT_BUTTON,&HAPSolverFrame::evt_targets_info,this);
    target_new_btn->Bind(wxEVT_BUTTON,&HAPSolverFrame::evt_target_button,this);
    
    Bind(EVT_LENGTH_SELECTOR,&HAPSolverFrame::evt_structure_change,this);
    Bind(EVT_MAP_READY,&HAPSolverFrame::evt_map_ready,this);
    Bind(EVT_MAP_REQUEST,&HAPSolverFrame::evt_map_request,this);
    Bind(wxEVT_MENU,&HAPSolverFrame::evt_menu,this);
    Bind(EVT_MINIMAT_SELECTOR,&HAPSolverFrame::evt_structure_change,this);
    Bind(EVT_NEW_TARGET,&HAPSolverFrame::evt_new_target,this);
    Bind(EVT_PLIST_DOWN,&HAPSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_UP,&HAPSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_RESIZE,&HAPSolverFrame::evt_panels_change,this);
    Bind(EVT_PLIST_REMOVE,&HAPSolverFrame::evt_panels_change,this);
    Bind(EVT_TARGETS_DELETION,&HAPSolverFrame::evt_targets_deletion,this);
    Bind(EVT_WAVELENGTH_SELECTOR,&HAPSolverFrame::evt_structure_change,this);
    
    structure_panel->SetScrollbars(10,10,50,50);
    structure_panel->FitInside();
    
    compute_map();
}

void HAPSolverFrame::compute_map()
{
    if(map_data==nullptr) map_data=new GLfloat[map_Nr*map_Ni];
    else if(reallocate_map)
    {
        delete[] map_data;
        map_data=new GLfloat[map_Nr*map_Ni];
    }
    
    int l;
    int Nl=layers_list->get_size();
    
    double lambda=lambda_t_ctrl->get_lambda();
    double w=m_to_rad_Hz(lambda);
    
    Multilayer_TMM ml(Nl);
    ml.set_lambda(lambda);
    
    ml.set_environment(superstrate_selector->get_n(w),
                       substrate_selector->get_n(w));
    
    for(l=0;l<Nl;l++)
    {
        LayerPanel *panel=reinterpret_cast<LayerPanel*>(layers_list->get_panel(l));
        ml.set_layer(l,panel->get_height(),panel->get_material()->get_n(w));
    }
    
    double nr=1.0,ni=0;
    Imdouble n_eff=1.0,charac;
    
    polarization=polar_ctrl->GetSelection();
    
    for(int i=0;i<map_Nr;i++)
    {
        for(int j=0;j<map_Ni;j++)
        {
            nr=map_nr_min+(map_nr_max-map_nr_min)*i/(map_Nr-1.0);
            ni=map_ni_min+(map_ni_max-map_ni_min)*j/(map_Ni-1.0);
            
            n_eff=nr+ni*Im;
            
                 if(polarization==0) charac=ml.compute_chara_TE(n_eff);
            else if(polarization==1) charac=ml.compute_chara_TM(n_eff);
            
            map_data[i+j*map_Nr]=1.0/std::abs(charac);
        }
    }
    
    wxCommandEvent event(EVT_MAP_READY);
    
    wxPostEvent(this,event);
}

void HAPSolverFrame::compute_exposure()
{
    double map_max=0;
    double map_avg=0;
    
    for(int i=0;i<map_Nr*map_Ni;i++)
    {
        map_max=std::max(map_max,static_cast<double>(map_data[i]));
        map_avg+=static_cast<double>(map_data[i]);
    }
    
    map_avg/=map_Nr*map_Ni;
    
    int p_max=static_cast<int>(std::log10(map_avg)*20.0)+20;
    exposure_slider->SetValue(static_cast<int>(p_max));
    target_graph->set_exposure(p_max/20.0);
}

void HAPSolverFrame::construct_multilayer(Multilayer_TMM &ml)
{
    int l;
    int Nl=layers_list->get_size();
    
    double lambda=lambda_t_ctrl->get_lambda();
    double w=m_to_rad_Hz(lambda);
    
    ml.set_N_layers(Nl);
    ml.set_lambda(lambda);
    
    ml.set_environment(superstrate_selector->get_n(w),
                       substrate_selector->get_n(w));
    
    for(l=0;l<Nl;l++)
    {
        LayerPanel *panel=dynamic_cast<LayerPanel*>(layers_list->get_panel(l));
        ml.set_layer(l,panel->get_height(),panel->get_material()->get_n(w));
    }
}

void HAPSolverFrame::evt_add_layer(wxCommandEvent &event)
{
    layers_list->add_panel<LayerPanel>();
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
}

void HAPSolverFrame::evt_clear_targets(wxCommandEvent &event)
{
    target_graph->clear_targets();
}

void HAPSolverFrame::evt_exposure(wxCommandEvent &event)
{
    int expo_i=exposure_slider->GetValue();
    
    target_graph->set_exposure(expo_i/20.0);
}

void HAPSolverFrame::evt_exposure_auto(wxCommandEvent &event)
{
    auto_exposure=auto_exposure_btn->GetValue();
    auto_exposure_btn->Refresh();
    
    if(auto_exposure) compute_exposure();
}

void HAPSolverFrame::evt_map_ready(wxCommandEvent &event)
{
    if(auto_exposure) compute_exposure();
    
    target_graph->set_map(map_Nr,map_Ni,
                        map_nr_min,map_nr_max,
                        map_ni_min,map_ni_max,map_data);
    
    
    computing_map=false;
}

void HAPSolverFrame::evt_map_request(MSMapEvent &event)
{
    if(!computing_map)
    {
        computing_map=true;
        
        MapThread *computation_thread=new MapThread;
        
        if(map_Nr!=event.Nr || map_Ni!=event.Ni) reallocate_map=true;
        else reallocate_map=false;
        
        map_Nr=event.Nr;
        map_Ni=event.Ni;
        map_nr_min=event.nr_min;
        map_nr_max=event.nr_max;
        map_ni_min=event.ni_min;
        map_ni_max=event.ni_max;
        
        computation_thread->frame=this;
        
        computation_thread->Run();
    }
}

void HAPSolverFrame::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
        
    if(ID==MENU_LOAD) evt_load_project("ahapms","Aether HA Modes Solver Files");
    else if(ID==MENU_SAVE) evt_save_project("ahapms","Aether HA Modes Solver Files");
    else if(ID==MENU_SAVE_AS) evt_save_project_as("ahapms","Aether HA Modes Solver Files");
    else if(ID==MENU_EXIT) Close();
    
    event.Skip();
}


void HAPSolverFrame::evt_new_target(wxCommandEvent &event)
{
    MSTarget *target=reinterpret_cast<MSTarget*>(event.GetClientData());
    
    refine_target(*target);
    
    Multilayer_TMM ml;
    construct_multilayer(ml);
    
    int Sx,Sy;
    GetSize(&Sx,&Sy);
    
    MSSingleTargetDialog dialog(static_cast<int>(0.25*Sx),
                                static_cast<int>(0.25*Sy),
                                static_cast<int>(0.5*Sx),
                                static_cast<int>(0.5*Sy),
                                polarization,lambda_t_ctrl->get_lambda(),*target,ml);
    
    if(!dialog.confirm_target) target_graph->forget_target(*target);
}

void HAPSolverFrame::evt_panels_change(wxCommandEvent &event)
{
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
    
    if(autorefresh) request_map_computation();
}

void HAPSolverFrame::evt_refresh_toggle(wxCommandEvent &event)
{
    autorefresh=target_refresh_btn->GetValue();
    target_refresh_btn->Refresh();
    
    if(autorefresh) request_map_computation();
    
    event.Skip();
}

void HAPSolverFrame::evt_reset_zoom(wxCommandEvent &event)
{
    map_nr_min=nr_t_min_ctrl->get_value();
    map_nr_max=nr_t_max_ctrl->get_value();
    map_ni_min=ni_t_min_ctrl->get_value();
    map_ni_max=ni_t_max_ctrl->get_value();
    
    request_map_computation();
    
    event.Skip();
}

void HAPSolverFrame::evt_structure_change(wxCommandEvent &event)
{
    double w=m_to_rad_Hz(lambda_t_ctrl->get_lambda());
    target_graph->set_substrate(std::abs(substrate_selector->get_n(w)));
    target_graph->set_superstrate(std::abs(superstrate_selector->get_n(w)));
    
    if(autorefresh) request_map_computation();
}

void HAPSolverFrame::evt_target_button(wxCommandEvent &event)
{
    target_graph->enable_target_mode();
}

void HAPSolverFrame::evt_targets_deletion(MSTargetDeletionEvent &event)
{
    std::vector<MSTarget> &targets=event.targets;
    
    for(unsigned int i=0;i<targets.size();i++)
        target_graph->forget_target(targets[i]);
}

void HAPSolverFrame::evt_targets_info(wxCommandEvent &event)
{
    std::vector<MSTarget> targets=target_graph->get_targets();
    
    if(targets.size()<1) return;
    
    Multilayer_TMM ml;
    construct_multilayer(ml);
    
    int Sx,Sy;
    GetSize(&Sx,&Sy);
    
    MSMultipleTargetsDialog *dialog=nullptr;
    
    dialog=new MSMultipleTargetsDialog(this,
                                       static_cast<int>(0.25*Sx),static_cast<int>(0.25*Sy),
                                       static_cast<int>(0.5*Sx),static_cast<int>(0.5*Sy),
                                       polarization,lambda_t_ctrl->get_lambda(),targets,ml);
    
    null_function(dialog); // Warning removal
}


int lua_gui_hapms_mode(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    
    luaL_getmetatable(L,"metatable_hapms_frame");
    lua_setmetatable(L,-2);
    
    return 1;
}

int add_layer(lua_State *L)
{
    HAPSolverFrame *p_frame=lua_get_metapointer<HAPSolverFrame>(L,1);
    
    double height=lua_tonumber(L,2);
    double std_dev=lua_tonumber(L,3);
    GUI::Material *material=dynamic_cast<GUI::Material*>(lua_get_metapointer<Material>(L,4));
    
    p_frame->layers_list->add_panel<LayerPanel>(height,std_dev,material,true);
    
    return 0;
}

int set_substrate(lua_State *L)
{
    HAPSolverFrame *p_frame=lua_get_metapointer<HAPSolverFrame>(L,1);
    Material *p_material=lua_get_metapointer<Material>(L,2);
    
    p_frame->substrate_selector->set_material(dynamic_cast<GUI::Material*>(p_material));
    
    return 0;
}

int set_superstrate(lua_State *L)
{
    HAPSolverFrame *p_frame=lua_get_metapointer<HAPSolverFrame>(L,1);
    Material *p_material=lua_get_metapointer<Material>(L,2);
    
    p_frame->superstrate_selector->set_material(dynamic_cast<GUI::Material*>(p_material));
    
    return 0;
}

void HAPSolverFrame::load_project(wxFileName const &fname_)
{
    layers_list->clear();
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    // Materials
    
    lua_gui_material::Loader loader;
    loader.create_metatable(L);
    
    // Model
    
    lua_register(L,"gui_hapms_mode",lua_gui_hapms_mode);
    
    HAPSolverFrame *p_frame=this;
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&p_frame));
    lua_setglobal(L,"bound_class");
    
    create_obj_metatable(L,"metatable_hapms_frame");
    
    metatable_add_func(L,"add_layer",&add_layer);
    metatable_add_func(L,"substrate",&set_substrate);
    metatable_add_func(L,"superstrate",&set_superstrate);
    
    int load_err = luaL_loadfile(L,fname.c_str());
    
    if(load_err!=LUA_OK)
    {
             if(load_err==LUA_ERRFILE) std::cout<<"Lua file error with "<<fname.c_str()<<std::endl;
        else if(load_err==LUA_ERRSYNTAX) std::cout<<"Lua syntax error with "<<fname.c_str()<<std::endl;
        else std::cout<<"Lua error with "<<fname.c_str()<<std::endl;
        std::cin.get();
        return;
    }
    
    lua_pcall(L,0,0,0);
    
    lua_close(L);
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
    
    SetTitle(wxString("Humman Assisted Mode Solver : ").Append(project_fname.GetName()));
    
    if(autorefresh) request_map_computation();
}

void HAPSolverFrame::lua_add_layer(double height,double std_dev,GUI::Material *material)
{
    layers_list->add_panel<LayerPanel>(height,std_dev,material,true);
}

void HAPSolverFrame::lua_set_substrate(GUI::Material *material)
{
    substrate_selector->set_material(material);
}

void HAPSolverFrame::lua_set_superstrate(GUI::Material *material)
{
    superstrate_selector->set_material(material);
}

void HAPSolverFrame::refine_target(MSTarget &target)
{
    int l;
    int Nl=layers_list->get_size();
    
    double lambda=lambda_t_ctrl->get_lambda();
    double w=m_to_rad_Hz(lambda);
    
    Multilayer_TMM ml(Nl);
    ml.set_lambda(lambda);
    
    ml.set_environment(superstrate_selector->get_n(w),
                       substrate_selector->get_n(w));
    
    for(l=0;l<Nl;l++)
    {
        LayerPanel *panel=reinterpret_cast<LayerPanel*>(layers_list->get_panel(l));
        ml.set_layer(l,panel->get_height(),panel->get_material()->get_n(w));
    }
    
    double span=0.1;
    double lim=1e-20;
    int max_fail=100;
    
    Imdouble zero_index=0;
    if(polarization==0)
        zero_index=ml.nearest_zero_chara_TE_MC(target.nr+target.ni*Im,
                                               span,span,lim,max_fail);
    else
        zero_index=ml.nearest_zero_chara_TM_MC(target.nr+target.ni*Im,
                                               span,span,lim,max_fail);
    
    target.nr=zero_index.real();
    target.ni=zero_index.imag();
}

void HAPSolverFrame::rename_panels()
{
    for(unsigned int n=0;n<layers_list->get_size();n++)
    {
        wxString title="Layer ";
        title<<n;
        
        layers_list->get_panel(n)->set_title(title);
    }
}

void HAPSolverFrame::request_map_computation()
{
    MSMapEvent event_out(EVT_MAP_REQUEST);
    
    event_out.nr_min=map_nr_min;
    event_out.nr_max=map_nr_max;
    
    event_out.ni_min=map_ni_min;
    event_out.ni_max=map_ni_max;
    
    event_out.Nr=map_Nr;
    event_out.Ni=map_Ni;
    
    target_graph->set_zoom(map_nr_min,map_nr_max,map_ni_min,map_ni_max);
    
    wxPostEvent(this,event_out);
}

void HAPSolverFrame::save_project(wxFileName const &fname_)
{
    // Materials
    
    lua_gui_material::Translator mtr("");
    
    mtr.gather(superstrate_selector->get_material());
    mtr.gather(substrate_selector->get_material());
    
    for(std::size_t i=0;i<layers_list->get_size();i++)
    {
        std::vector<GUI::Material*> layer_mats;
        layers_list->get_panel(i)->get_materials(layer_mats);
        
        for(GUI::Material *mat:layer_mats)
            mtr.gather(mat);
    }
    
    // Model
        
    std::string fname=fname_.GetFullPath().ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    file<<mtr.get_header()<<"\n";
    
    file<<"mode=gui_hapms_mode()"<<std::endl;
    
    file<<"mode:superstrate("<<mtr(superstrate_selector->get_material())<<")"<<std::endl<<std::endl;
    
    for(unsigned int i=0;i<layers_list->get_size();i++)
    {
        file<<"mode:"<<layers_list->get_panel(i)->get_lua_string(mtr)<<std::endl;
    }
    file<<std::endl;
    
    file<<"mode:substrate("<<mtr(substrate_selector->get_material())<<")";
    
    file.close();
    
    SetTitle(wxString("Human Assisted Mode Solver : ").Append(project_fname.GetName()));
}
