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

#include <gui_vector3ctrl.h>

wxDEFINE_EVENT(EVT_V3CTRL,wxCommandEvent);

Vector3Ctrl::Vector3Ctrl(wxWindow *parent,std::string name)
    :wxPanel(parent),
     data(0,0,0)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    x_ctrl=new wxTextCtrl(this,wxID_ANY,"0",
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxTE_PROCESS_ENTER|wxTE_RIGHT);
    
    y_ctrl=new wxTextCtrl(this,wxID_ANY,"0",
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxTE_PROCESS_ENTER|wxTE_RIGHT);
    
    z_ctrl=new wxTextCtrl(this,wxID_ANY,"0",
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxTE_PROCESS_ENTER|wxTE_RIGHT);
    
    if(name.size()>0)
    {
        wxStaticText *name_ctrl=new wxStaticText(this,wxID_ANY,name);
        sizer->Add(name_ctrl);
    }
    
    sizer->Add(x_ctrl,wxSizerFlags().Expand());
    sizer->Add(y_ctrl,wxSizerFlags().Expand());
    sizer->Add(z_ctrl,wxSizerFlags().Expand());
    
    SetSizer(sizer);
    
    x_ctrl->Bind(wxEVT_TEXT,&Vector3Ctrl::update_x,this);
    y_ctrl->Bind(wxEVT_TEXT,&Vector3Ctrl::update_y,this);
    z_ctrl->Bind(wxEVT_TEXT,&Vector3Ctrl::update_z,this);
}

Vector3 Vector3Ctrl::GetValue()
{
    return data;
}

void Vector3Ctrl::SetValue(Vector3 const &V)
{
    data=V;
    update_ctrl();
}

void Vector3Ctrl::update_ctrl()
{
    std::stringstream strm_x,strm_y,strm_z;
    
    strm_x<<data.x;
    strm_y<<data.y;
    strm_z<<data.z;
    
    x_ctrl->ChangeValue(strm_x.str());
    z_ctrl->ChangeValue(strm_y.str());
    y_ctrl->ChangeValue(strm_z.str());
}

void Vector3Ctrl::update_x(wxCommandEvent &event)
{
    std::stringstream strm;
    strm.str(std::string(x_ctrl->GetValue()));
    strm>>data.x;
    
    chk_msg_sc(data);
    
    wxPostEvent(this,wxCommandEvent(EVT_V3CTRL));
}

void Vector3Ctrl::update_y(wxCommandEvent &event)
{
    std::stringstream strm;
    strm.str(std::string(y_ctrl->GetValue()));
    strm>>data.y;
    
    chk_msg_sc(data);
    
    wxPostEvent(this,wxCommandEvent(EVT_V3CTRL));
}

void Vector3Ctrl::update_z(wxCommandEvent &event)
{
    std::stringstream strm;
    strm.str(std::string(z_ctrl->GetValue()));
    strm>>data.z;
    
    chk_msg_sc(data);
    
    wxPostEvent(this,wxCommandEvent(EVT_V3CTRL));
}
