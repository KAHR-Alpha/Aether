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

#include <gui_fdtd.h>
#include <string_tools.h>

//#######################
//   Sensor_Gen_Dialog
//#######################

Sensor_Gen_Dialog::Sensor_Gen_Dialog(Sensor_generator *generator_)
    :Sensor_generator(*generator_),
     wxDialog(0,wxID_ANY,"Sensor Parameters",
              wxGetApp().default_dialog_origin()),
     generator_holder(generator_),
     selection_ok(false),
     disable_plane_ctrl(6)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    // Type
    
    type_ctrl=new wxChoice(this,wxID_ANY);
    type_ctrl->Append("Spectral Poynting Box");
    type_ctrl->Append("Spectral Poynting Plane");
    type_ctrl->Append("Diffractive Orders");
    type_ctrl->Append("FieldBlock");
    type_ctrl->Append("FieldMap");
    
         if(type==Sensor_generator::BOX_SPECTRAL_POYNTING) type_ctrl->SetSelection(0);
    else if(type==Sensor_generator::PLANAR_SPECTRAL_POYNTING) type_ctrl->SetSelection(1);
    else if(type==Sensor_generator::DIFF_ORDERS) type_ctrl->SetSelection(2);
    else if(type==Sensor_generator::FIELDBLOCK) type_ctrl->SetSelection(3);
    else if(type==Sensor_generator::FIELDMAP) type_ctrl->SetSelection(4);
    
    type_ctrl->Bind(wxEVT_CHOICE,&Sensor_Gen_Dialog::evt_type,this);
    
    wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(new wxStaticText(this,wxID_ANY,"Sensor Type: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    type_sizer->Add(type_ctrl);
    
    sizer->Add(type_sizer,wxSizerFlags().Border(wxALL,2));
    
    // Name
    
    name_ctrl=new NamedTextCtrl<std::string>(this,"Sensor Name: ",name);
    
    sizer->Add(name_ctrl,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Spectrum
    
    spectrum_ctrl=new SpectrumSelector(this,lambda_min,lambda_max,Nl);
    
    sizer->Add(spectrum_ctrl);
    
    // Location
    
    // - Position
    
    x1_ctrl=new NamedTextCtrl<double>(this,"X Min",x1r,true);
    x2_ctrl=new NamedTextCtrl<double>(this,"X Max",x2r,true);
    
    y1_ctrl=new NamedTextCtrl<double>(this,"Y Min",y1r,true);
    y2_ctrl=new NamedTextCtrl<double>(this,"Y Max",y2r,true);
    
    z1_ctrl=new NamedTextCtrl<double>(this,"Z Min",z1r,true);
    z2_ctrl=new NamedTextCtrl<double>(this,"Z Max",z2r,true);
    
    // - Orientation
    
    orientation_ctrl=new wxChoice(this,wxID_ANY);
    orientation_ctrl->Append("-X");
    orientation_ctrl->Append("+X");
    orientation_ctrl->Append("-Y");
    orientation_ctrl->Append("+Y");
    orientation_ctrl->Append("-Z");
    orientation_ctrl->Append("+Z");
    
         if(orientation==NORMAL_XM) orientation_ctrl->SetSelection(0);
    else if(orientation==NORMAL_X ) orientation_ctrl->SetSelection(1);
    else if(orientation==NORMAL_YM) orientation_ctrl->SetSelection(2);
    else if(orientation==NORMAL_Y ) orientation_ctrl->SetSelection(3);
    else if(orientation==NORMAL_ZM) orientation_ctrl->SetSelection(4);
    else if(orientation==NORMAL_Z ) orientation_ctrl->SetSelection(5);
    
    orientation_ctrl->Bind(wxEVT_CHOICE,&Sensor_Gen_Dialog::evt_orientation,this);
    
    wxBoxSizer *orientation_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    orientation_sizer->Add(new wxStaticText(this,wxID_ANY,"Normal Vector: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    orientation_sizer->Add(orientation_ctrl);
    
    // - Sizers
    
    wxStaticBoxSizer *location_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Location");
    
    wxBoxSizer *location_x_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_x_sizer->Add(x1_ctrl,wxSizerFlags(1));
    location_x_sizer->Add(x2_ctrl,wxSizerFlags(1));
    
    wxBoxSizer *location_y_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_y_sizer->Add(y1_ctrl,wxSizerFlags(1));
    location_y_sizer->Add(y2_ctrl,wxSizerFlags(1));
    
    wxBoxSizer *location_z_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_z_sizer->Add(z1_ctrl,wxSizerFlags(1));
    location_z_sizer->Add(z2_ctrl,wxSizerFlags(1));
    
    location_sizer->Add(location_x_sizer,wxSizerFlags().Expand());
    location_sizer->Add(location_y_sizer,wxSizerFlags().Expand());
    location_sizer->Add(location_z_sizer,wxSizerFlags().Expand());
    location_sizer->Add(orientation_sizer,wxSizerFlags().Border(wxALL,2));
    
    sizer->Add(location_sizer,wxSizerFlags().Expand());
    
    wxStaticBoxSizer *disable_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"Disable planes");
    
    std::string disable_ctrl_names[6]={"-X","+X","-Y","+Y","-Z","+Z"};
    int disable_ctrl_values[6]={disable_xm,disable_xp,disable_ym,disable_yp,disable_zm,disable_zp};
    
    for(int i=0;i<6;i++)
    {
        disable_plane_ctrl[i]=new wxCheckBox(this,wxID_ANY,disable_ctrl_names[i]);
        disable_plane_ctrl[i]->SetValue(disable_ctrl_values[i]);
        disable_sizer->Add(disable_plane_ctrl[i],wxSizerFlags().Border(wxALL,2));
    }
    
    sizer->Add(disable_sizer);
    
    refresh_layout();
    
    // Buttons
    
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    ok_btn->Bind(wxEVT_BUTTON,&Sensor_Gen_Dialog::evt_ok,this);
    cancel_btn->Bind(wxEVT_BUTTON,&Sensor_Gen_Dialog::evt_cancel,this);
    
    buttons_sizer->Add(ok_btn);
    buttons_sizer->Add(cancel_btn);
    
    sizer->Add(buttons_sizer,wxSizerFlags().Border(wxALL,3).Align(wxALIGN_RIGHT));
    
    // Wrapping Up
    
    SetSizerAndFit(sizer);
    ShowModal();
}

void Sensor_Gen_Dialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void Sensor_Gen_Dialog::evt_ok(wxCommandEvent &event)
{
    selection_ok=true;
    
    save();
    Close();
}

void Sensor_Gen_Dialog::evt_orientation(wxCommandEvent &event)
{
    refresh_layout();
    
    event.Skip();
}

void Sensor_Gen_Dialog::evt_type(wxCommandEvent &event)
{
    int selection=type_ctrl->GetSelection();
    
    if(selection==0) type=Sensor_generator::BOX_SPECTRAL_POYNTING;
    else if(selection==1) type=Sensor_generator::PLANAR_SPECTRAL_POYNTING;
    else if(selection==2) type=Sensor_generator::DIFF_ORDERS;
    else if(selection==3) type=Sensor_generator::FIELDBLOCK;
    else if(selection==4) type=Sensor_generator::FIELDMAP;
    
    refresh_layout();
    
    event.Skip();
}

bool Sensor_Gen_Dialog::is_planar()
{
    bool plane=true;
    
    if(type==Sensor_generator::BOX_POYNTING ||
       type==Sensor_generator::BOX_SPECTRAL_POYNTING ||
       type==Sensor_generator::FIELDBLOCK) plane=false;
    
    return plane;
}

void Sensor_Gen_Dialog::refresh_layout()
{
    bool plane=is_planar();
    
    int orientation=orientation_ctrl->GetSelection();
    
    x2_ctrl->Enable(); y2_ctrl->Enable(); z2_ctrl->Enable();
    
    if(plane)
    {
             if(orientation==0 || orientation==1) x2_ctrl->Disable();
        else if(orientation==2 || orientation==3) y2_ctrl->Disable();
        else if(orientation==4 || orientation==5) z2_ctrl->Disable();
    }
    
    if(type==Sensor_generator::FIELDBLOCK ||
       type==Sensor_generator::FIELDMAP) spectrum_ctrl->set_monochromatic();
    else spectrum_ctrl->set_polychromatic();
}

void Sensor_Gen_Dialog::save()
{
    name=name_ctrl->get_value();
    
    lambda_min=spectrum_ctrl->get_lambda_min();
    lambda_max=spectrum_ctrl->get_lambda_max();
    Nl=spectrum_ctrl->get_Np();
    
    x1r=x1_ctrl->get_value(); x2r=x2_ctrl->get_value();
    y1r=y1_ctrl->get_value(); y2r=y2_ctrl->get_value();
    z1r=z1_ctrl->get_value(); z2r=z2_ctrl->get_value();
    
    int selection=orientation_ctrl->GetSelection();
    
         if(selection==0) orientation=NORMAL_XM;
    else if(selection==1) orientation=NORMAL_X ;
    else if(selection==2) orientation=NORMAL_YM;
    else if(selection==3) orientation=NORMAL_Y ;
    else if(selection==4) orientation=NORMAL_ZM;
    else if(selection==5) orientation=NORMAL_Z ;
    
    if(is_planar())
    {
             if(selection==0 || selection==1) x2r=x1r;
        else if(selection==2 || selection==3) y2r=y1r;
        else if(selection==4 || selection==5) z2r=z1r;
    }
    
    disable_xm=disable_xp=disable_ym=disable_yp=disable_zm=disable_zp=false;
    if(type==Sensor_generator::BOX_POYNTING || type==Sensor_generator::BOX_SPECTRAL_POYNTING)
    {
        bool *disable_X[6]={&disable_xm,&disable_xp,&disable_ym,&disable_yp,&disable_zm,&disable_zp};
        
        for(int i=0;i<6;i++)
            *(disable_X[i])=disable_plane_ctrl[i]->GetValue();
    }
    
    Sensor_generator *base_p=this;
    
    (*generator_holder)=*base_p;
}

//#######################
//   Source_Gen_Dialog
//#######################

Source_Gen_Dialog::Source_Gen_Dialog(Source_generator *generator_)
    :Source_generator(*generator_),
     wxDialog(0,wxID_ANY,"Source Parameters",
              wxGetApp().default_dialog_origin()),
     generator_holder(generator_),
     selection_ok(false)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    // Type
    
    type_ctrl=new wxChoice(this,wxID_ANY);
    type_ctrl->Append("Oscillator");
    
    type_ctrl->SetSelection(0);
    
    type_ctrl->Bind(wxEVT_CHOICE,&Source_Gen_Dialog::evt_type,this);
    
    wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(new wxStaticText(this,wxID_ANY,"Source Type: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    type_sizer->Add(type_ctrl);
    
    sizer->Add(type_sizer,wxSizerFlags().Border(wxALL,2));
    
    // Spectrum
    
    wxBoxSizer *spectrum_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    lambda_min_ctrl=new WavelengthSelector(this,"Lambda Min",lambda_min);
    lambda_max_ctrl=new WavelengthSelector(this,"Lambda Max",lambda_max);
    
    spectrum_sizer->Add(lambda_min_ctrl);
    spectrum_sizer->Add(lambda_max_ctrl);
    
    sizer->Add(spectrum_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    
    // Location
    
    // - Position
    
    x1_ctrl=new NamedTextCtrl<double>(this,"X Min",x1r,true);
    x2_ctrl=new NamedTextCtrl<double>(this,"X Max",x2r,true);
    
    y1_ctrl=new NamedTextCtrl<double>(this,"Y Min",y1r,true);
    y2_ctrl=new NamedTextCtrl<double>(this,"Y Max",y2r,true);
    
    z1_ctrl=new NamedTextCtrl<double>(this,"Z Min",z1r,true);
    z2_ctrl=new NamedTextCtrl<double>(this,"Z Max",z2r,true);
    
    // - Orientation
    
    orientation_ctrl=new wxChoice(this,wxID_ANY);
    orientation_ctrl->Append("-X");
    orientation_ctrl->Append("+X");
    orientation_ctrl->Append("-Y");
    orientation_ctrl->Append("+Y");
    orientation_ctrl->Append("-Z");
    orientation_ctrl->Append("+Z");
    
         if(orientation==NORMAL_XM) orientation_ctrl->SetSelection(0);
    else if(orientation==NORMAL_X ) orientation_ctrl->SetSelection(1);
    else if(orientation==NORMAL_YM) orientation_ctrl->SetSelection(2);
    else if(orientation==NORMAL_Y ) orientation_ctrl->SetSelection(3);
    else if(orientation==NORMAL_ZM) orientation_ctrl->SetSelection(4);
    else if(orientation==NORMAL_Z ) orientation_ctrl->SetSelection(5);
    
    wxBoxSizer *orientation_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    orientation_sizer->Add(new wxStaticText(this,wxID_ANY,"Orientation: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    orientation_sizer->Add(orientation_ctrl);
    
    // - Sizers
    
    wxStaticBoxSizer *location_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Location");
    
    wxBoxSizer *location_x_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_x_sizer->Add(x1_ctrl,wxSizerFlags(1));
    location_x_sizer->Add(x2_ctrl,wxSizerFlags(1));
    
    wxBoxSizer *location_y_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_y_sizer->Add(y1_ctrl,wxSizerFlags(1));
    location_y_sizer->Add(y2_ctrl,wxSizerFlags(1));
    
    wxBoxSizer *location_z_sizer=new wxBoxSizer(wxHORIZONTAL);
    location_z_sizer->Add(z1_ctrl,wxSizerFlags(1));
    location_z_sizer->Add(z2_ctrl,wxSizerFlags(1));
    
    location_sizer->Add(location_x_sizer,wxSizerFlags().Expand());
    location_sizer->Add(location_y_sizer,wxSizerFlags().Expand());
    location_sizer->Add(location_z_sizer,wxSizerFlags().Expand());
    location_sizer->Add(orientation_sizer);
    
    sizer->Add(location_sizer,wxSizerFlags().Expand());
    
    refresh_layout();
    
    // Buttons
    
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    ok_btn->Bind(wxEVT_BUTTON,&Source_Gen_Dialog::evt_ok,this);
    cancel_btn->Bind(wxEVT_BUTTON,&Source_Gen_Dialog::evt_cancel,this);
    
    buttons_sizer->Add(ok_btn);
    buttons_sizer->Add(cancel_btn);
    
    sizer->Add(buttons_sizer,wxSizerFlags().Border(wxALL,3).Align(wxALIGN_RIGHT));
    
    // Wrapping Up
    
    SetSizerAndFit(sizer);
    ShowModal();
}

void Source_Gen_Dialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void Source_Gen_Dialog::evt_ok(wxCommandEvent &event)
{
    selection_ok=true;
    
    save();
    Close();
}

void Source_Gen_Dialog::evt_type(wxCommandEvent &event)
{
    int selection=type_ctrl->GetSelection();
    
    if(selection==0) type=Source_generator::SOURCE_GEN_OSCILLATOR;
    
    refresh_layout();
    
    event.Skip();
}

void Source_Gen_Dialog::refresh_layout()
{
    if(type==Source_generator::SOURCE_GEN_OSCILLATOR)
    {
        x2_ctrl->Disable();
        y2_ctrl->Disable();
        z2_ctrl->Disable();
    }
    else
    {
        x2_ctrl->Enable();
        y2_ctrl->Enable();
        z2_ctrl->Enable();
    }
}

void Source_Gen_Dialog::save()
{
    int selection=type_ctrl->GetSelection();
    
    if(selection==0) type=SOURCE_GEN_OSCILLATOR;
    
    lambda_min=lambda_min_ctrl->get_lambda();
    lambda_max=lambda_max_ctrl->get_lambda();
    
    x1r=x1_ctrl->get_value(); x2r=x2_ctrl->get_value();
    y1r=y1_ctrl->get_value(); y2r=y2_ctrl->get_value();
    z1r=z1_ctrl->get_value(); z2r=z2_ctrl->get_value();
    
    selection=orientation_ctrl->GetSelection();
    
         if(selection==0) orientation=NORMAL_XM;
    else if(selection==1) orientation=NORMAL_X ;
    else if(selection==2) orientation=NORMAL_YM;
    else if(selection==3) orientation=NORMAL_Y ;
    else if(selection==4) orientation=NORMAL_ZM;
    else if(selection==5) orientation=NORMAL_Z ;
    
    Source_generator *base_p=this;
    
    (*generator_holder)=*base_p;
}

//######################
//   FDTD_Disc_Dialog
//######################

FDTD_Disp_Dialog::FDTD_Disp_Dialog(wxFrame *parent,std::vector<Basic_VAO*> &vao_,
                                   bool &pml_x_,bool &pml_y_,bool &pml_z_)
    :wxDialog(parent,wxID_ANY,"Display Options"),
     vao(vao_), pml_x(pml_x_), pml_y(pml_y_), pml_z(pml_z_)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    wxScrolledWindow *panel=new wxScrolledWindow(this);
    wxBoxSizer *panel_sizer=new wxBoxSizer(wxVERTICAL);
    
    // PMLs
    
    wxStaticBoxSizer *pml_sizer=new wxStaticBoxSizer(wxHORIZONTAL,panel,"PMLs/Padding");
    
    pml_x_ctrl=new wxCheckBox(panel,wxID_ANY,"X ");
    pml_y_ctrl=new wxCheckBox(panel,wxID_ANY,"Y ");
    pml_z_ctrl=new wxCheckBox(panel,wxID_ANY,"Z");
    
    pml_x_ctrl->SetValue(pml_x);
    pml_y_ctrl->SetValue(pml_y);
    pml_z_ctrl->SetValue(pml_z);
    
    pml_x_ctrl->Bind(wxEVT_CHECKBOX,&FDTD_Disp_Dialog::evt_pml,this);
    pml_y_ctrl->Bind(wxEVT_CHECKBOX,&FDTD_Disp_Dialog::evt_pml,this);
    pml_z_ctrl->Bind(wxEVT_CHECKBOX,&FDTD_Disp_Dialog::evt_pml,this);
    
    pml_sizer->Add(pml_x_ctrl,wxSizerFlags().Border(wxALL,2));
    pml_sizer->Add(pml_y_ctrl,wxSizerFlags().Border(wxALL,2));
    pml_sizer->Add(pml_z_ctrl,wxSizerFlags().Border(wxALL,2));
    
    panel_sizer->Add(pml_sizer,wxSizerFlags().Expand());
    
    // VAOs
    
    for(unsigned int i=0;i<vao.size();i++)
    {
        wxCheckBox *tmp_show=new wxCheckBox(panel,i,"Show");
        wxCheckBox *tmp_wireframe=new wxCheckBox(panel,i,"Wireframe");
        wxButton *tmp_color=new wxButton(panel,i,"");
        
        tmp_show->Bind(wxEVT_CHECKBOX,&FDTD_Disp_Dialog::evt_show,this);
        tmp_wireframe->Bind(wxEVT_CHECKBOX,&FDTD_Disp_Dialog::evt_wireframe,this);
        tmp_color->Bind(wxEVT_BUTTON,&FDTD_Disp_Dialog::evt_color,this);
        tmp_color->SetBackgroundColour(wxColour(vao[i]->color.x*255,
                                                vao[i]->color.y*255,
                                                vao[i]->color.z*255));
        
        if(vao[i]->display) tmp_show->SetValue(true);
        if(vao[i]->wireframe) tmp_wireframe->SetValue(true);
        
        wxString str("Material ");
        str<<vao[i]->ID;
        
        wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,str);
        sizer->Add(tmp_show,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        sizer->Add(tmp_wireframe,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        sizer->Add(tmp_color,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        
        panel_sizer->Add(sizer,wxSizerFlags().Expand());
        
        show.push_back(tmp_show);
        wireframe.push_back(tmp_wireframe);
        color.push_back(tmp_color);
    }
    
    panel->SetSizer(panel_sizer);
    panel->SetScrollbars(50,50,1,1);
    
    sizer->Add(panel,wxSizerFlags(1).Expand());
    
    // Wrapping up
    
    SetSizer(sizer);
//    SetSizerAndFit(sizer);
    Show();
}

void FDTD_Disp_Dialog::evt_color(wxCommandEvent &event)
{
    int i=event.GetId();
    
    wxColourData color_data;
    color_data.SetColour(wxColour(vao[i]->color.x*255,
                                  vao[i]->color.y*255,
                                  vao[i]->color.z*255));
    
    wxColourDialog dialog(this,&color_data);
    dialog.ShowModal();
    color_data=dialog.GetColourData();
    
    wxColour tmp_color=color_data.GetColour();
    vao[i]->color=Vector3(tmp_color.Red()/255.0,
                          tmp_color.Green()/255.0,
                          tmp_color.Blue()/255.0);
    
    color[i]->SetBackgroundColour(tmp_color);
    
    event.Skip();
}

void FDTD_Disp_Dialog::evt_pml(wxCommandEvent &event)
{
    pml_x=pml_x_ctrl->GetValue();
    pml_y=pml_y_ctrl->GetValue();
    pml_z=pml_z_ctrl->GetValue();
    
    event.Skip();
}

void FDTD_Disp_Dialog::evt_show(wxCommandEvent &event)
{
    int i=event.GetId();
    vao[i]->display=static_cast<bool>(event.GetInt());
    
    event.Skip();
}

void FDTD_Disp_Dialog::evt_wireframe(wxCommandEvent &event)
{
    int i=event.GetId();
    vao[i]->wireframe=static_cast<bool>(event.GetInt());
    
    event.Skip();
}
