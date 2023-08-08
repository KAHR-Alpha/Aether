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

#include <gui_selene.h>
#include <aether.h>

#include <wx/notebook.h>

namespace SelGUI
{

//#########################
//   FrameDialog
//#########################

FrameDialog::FrameDialog(Sel::Frame *frame_,
                         std::vector<Sel::Frame*> const &frames_,
                         OptimEngine &optim_engine_)
    :wxDialog(NULL,-1,"Object Properties",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     cancel_check(true),
     optim_engine(optim_engine_),
     frame(frame_)
{
    for(std::size_t i=0;i<frames_.size();i++)
    {
        if(!frames_[i]->cyclic_check(frame))
            frames.push_back(frames_[i]);
    }
    
    wxArrayString frame_names;
    
    frame_names.Add("World");
    for(std::size_t i=0;i<frames.size();i++)
        frame_names.Add(frames[i]->name);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    panels_book=new wxNotebook(splitter,wxID_ANY);
        
    // Controls
    
    ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - Name
    
    name_ctrl=new NamedTextCtrl<std::string>(ctrl_panel,"Name: ",frame->name,0,true);
    
    ctrl_sizer->Add(name_ctrl,std_flags);
    
    // - Location
    
    // -- Origin
    
    wxStaticBoxSizer *origin_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Origin");
    
    origin=new wxChoice(ctrl_panel,wxID_ANY);
    
    for(int i=0;i<frame->get_anchors_number();i++)
        origin->Append(frame->get_anchor_name(i));
    
    origin->SetSelection(frame->origin_anchor);
    
    origin_sizer->Add(origin,wxSizerFlags().Expand());
    ctrl_sizer->Add(origin_sizer,std_flags);
    
    // -- Relative Origin
    
    wxStaticBoxSizer *rel_origin_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Relative Origin");
    
    relative_origin=new wxChoice(ctrl_panel,wxID_ANY);
    relative_anchor=new wxChoice(ctrl_panel,wxID_ANY);
    relative_origin->Set(frame_names);
    
    int index;
    bool found;
    
    index=vector_locate(found,frames,frame->relative_origin);
    
    if(found) relative_origin->SetSelection(index+1);
    else relative_origin->SetSelection(0);
    
    refresh_relative_anchors();
    
    relative_origin->Bind(wxEVT_CHOICE,&FrameDialog::evt_relative_origin,this);
    
    
    rel_origin_sizer->Add(relative_origin,wxSizerFlags().Expand());
    rel_origin_sizer->Add(relative_anchor,wxSizerFlags().Expand());
    ctrl_sizer->Add(rel_origin_sizer,std_flags);
    
    // - Translation Frame 
    
    wxStaticBoxSizer *translation_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Translation direction");
    
    translation_frame=new wxChoice(ctrl_panel,wxID_ANY);
    translation_frame->Set(frame_names);
    
    index=vector_locate(found,frames,frame->translation_frame);
    
    if(found) translation_frame->SetSelection(index+1);
    else translation_frame->SetSelection(0);
    
    translation_sizer->Add(translation_frame,wxSizerFlags().Expand());
    ctrl_sizer->Add(translation_sizer,std_flags);
    
    // - Translation coordinates
    
    wxStaticBoxSizer *loc_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Location");
    wxStaticBoxSizer *ang_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Rotation");
    
    x_ctrl=new LengthSelector(ctrl_panel,"x: ",frame->in_displacement.x,false,"mm");
    y_ctrl=new LengthSelector(ctrl_panel,"y: ",frame->in_displacement.y,false,"mm");
    z_ctrl=new LengthSelector(ctrl_panel,"z: ",frame->in_displacement.z,false,"mm");
    
    x_ctrl->handle_external_optimization(&frame->in_displacement.x,optim_engine);
    y_ctrl->handle_external_optimization(&frame->in_displacement.y,optim_engine);
    z_ctrl->handle_external_optimization(&frame->in_displacement.z,optim_engine);
    
    loc_sizer->Add(x_ctrl,wxSizerFlags().Expand());
    loc_sizer->Add(y_ctrl,wxSizerFlags().Expand());
    loc_sizer->Add(z_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(loc_sizer,std_flags);
    
    // - Rotation Frame
    
    wxStaticBoxSizer *rotation_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Relative Rotation");
    
    rotation_frame=new wxChoice(ctrl_panel,wxID_ANY);
    rotation_frame->Set(frame_names);
    
    index=vector_locate(found,frames,frame->rotation_frame);
    
    if(found) rotation_frame->SetSelection(index+1);
    else rotation_frame->SetSelection(0);
    
    rotation_sizer->Add(rotation_frame,wxSizerFlags().Expand());
    ctrl_sizer->Add(rotation_sizer,std_flags);
    
    // - Rotation coordinates
    
    a_ctrl=new NamedTextCtrl<double>(ctrl_panel,"a: ",frame->in_A.degree());
    b_ctrl=new NamedTextCtrl<double>(ctrl_panel,"b: ",frame->in_B.degree());
    c_ctrl=new NamedTextCtrl<double>(ctrl_panel,"c: ",frame->in_C.degree());
    
    a_ctrl->handle_external_optimization(&frame->in_A.val,optim_engine);
    b_ctrl->handle_external_optimization(&frame->in_B.val,optim_engine);
    c_ctrl->handle_external_optimization(&frame->in_C.val,optim_engine);
    
    ang_sizer->Add(a_ctrl,wxSizerFlags().Expand());
    ang_sizer->Add(b_ctrl,wxSizerFlags().Expand());
    ang_sizer->Add(c_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(ang_sizer,std_flags);
    
    // - Controls wrap-up
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
    
    // Display
    
    splitter->SplitVertically(ctrl_panel,panels_book,250);
    splitter->SetMinimumPaneSize(20);
    
    // Splitter wrapping up
    
    top_sizer->Add(splitter,wxSizerFlags(1).Expand());
    
    // Buttons
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL,3));
    
    // Bindings
    
    cancel_btn->Bind(wxEVT_BUTTON,&FrameDialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&FrameDialog::evt_ok,this);
    
    Bind(wxEVT_CLOSE_WINDOW,&FrameDialog::evt_close,this);
    
    //  Final Layout
    
    SetSizer(top_sizer);
}

void FrameDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void FrameDialog::evt_close(wxCloseEvent &event)
{
    event.Skip();
}

void FrameDialog::evt_ok(wxCommandEvent &event)
{
    cancel_check=false;
    
    frame->origin_anchor=origin->GetSelection();
    
    if(relative_origin->GetSelection()==0)
    {
        frame->relative_origin=nullptr;
        frame->relative_anchor=0;
    }
    else
    {
        frame->relative_origin=frames[relative_origin->GetSelection()-1];
        frame->relative_anchor=relative_anchor->GetSelection();
    }
    
    if(translation_frame->GetSelection()==0) frame->translation_frame=nullptr;
    else frame->translation_frame=frames[translation_frame->GetSelection()-1];
    
    if(rotation_frame->GetSelection()==0) frame->rotation_frame=nullptr;
    else frame->rotation_frame=frames[rotation_frame->GetSelection()-1];
    
    // Location
    
    frame->in_displacement(x_ctrl->get_length(),
                           y_ctrl->get_length(),
                           z_ctrl->get_length());
    
    if(x_ctrl->optimize) optim_engine.register_variable(&frame->in_displacement.x,x_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_displacement.x);
    
    if(y_ctrl->optimize) optim_engine.register_variable(&frame->in_displacement.y,y_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_displacement.y);
    
    if(z_ctrl->optimize) optim_engine.register_variable(&frame->in_displacement.z,z_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_displacement.z);
    
    // Rotation
    
    frame->in_A.degree(a_ctrl->get_value());
    frame->in_B.degree(b_ctrl->get_value());
    frame->in_C.degree(c_ctrl->get_value());
    
    if(a_ctrl->optimize) optim_engine.register_variable(&frame->in_A.val,a_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_A.val);
    
    if(b_ctrl->optimize) optim_engine.register_variable(&frame->in_B.val,b_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_B.val);
    
    if(c_ctrl->optimize) optim_engine.register_variable(&frame->in_C.val,c_ctrl->optim_rule);
    else optim_engine.forget_variable(&frame->in_C.val);
    
    frame->name=name_ctrl->get_value();
    
    save_object();
    
//    frame->prism_length=prism_length->get_length();
//    frame->prism_height=prism_height->get_length();
//    frame->prism_a1=prism_a1->get_value()*Pi/180.0;
//    frame->prism_a2=prism_a2->get_value()*Pi/180.0;
//    frame->prism_width=prism_width->get_length();
    
//    frame->cyl_mat=cyl_mat->get_material();
//    frame->ls_mat=ls_mat->get_material();
//    frame->pl_mat=pl_mat->get_material();
//    frame->prm_mat=prm_mat->get_material();
//    frame->prism_mat=prism_mat->get_material();
    
//    Destroy();
    Close();
}

void FrameDialog::evt_relative_origin(wxCommandEvent &event)
{
    refresh_relative_anchors();
    ctrl_panel->FitInside();
    
    event.Skip();
}

void FrameDialog::refresh_relative_anchors()
{
    wxArrayString str_arr;
    
    int selection=relative_origin->GetSelection();
    
    if(selection==0)
    {
        str_arr.Add("Center");
        relative_anchor->Set(str_arr);
        relative_anchor->SetSelection(0);
    }
    else
    {
        selection--;
        
        int N=frames[selection]->get_anchors_number();
        
        for(int i=0;i<N;i++)
            str_arr.Add(frames[selection]->get_anchor_name(i));
        
        relative_anchor->Set(str_arr);
        
        if(frames[selection]==frame->relative_origin)
            relative_anchor->SetSelection(frame->relative_anchor);
        else relative_anchor->SetSelection(0);
    }
}

}
