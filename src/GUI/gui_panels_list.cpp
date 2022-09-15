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
#include <gui_panels_list.h>

wxDEFINE_EVENT(EVT_PLIST_DOWN,wxCommandEvent);
wxDEFINE_EVENT(EVT_PLIST_REMOVE,wxCommandEvent);
wxDEFINE_EVENT(EVT_PLIST_RESIZE,wxCommandEvent);
wxDEFINE_EVENT(EVT_PLIST_UP,wxCommandEvent);

PanelsListBase::PanelsListBase(wxWindow *parent)
    :wxPanel(parent)
{
    sizer=new wxBoxSizer(wxVERTICAL);
    header_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    collapse_btn=new wxToggleButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    down_btn=new wxButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    up_btn=new wxButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    kill=new wxButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT|wxBORDER_NONE);
    
    std::string collapse_tri_16=PathManager::locate_resource("resources/collapse_tri_16.png").generic_string();
    std::string expand_tri_16=PathManager::locate_resource("resources/expand_tri_16.png").generic_string();
    std::string down_arrow_16=PathManager::locate_resource("resources/down_arrow_16.png").generic_string();
    std::string up_arrow_16=PathManager::locate_resource("resources/up_arrow_16.png").generic_string();
    std::string x_button_14=PathManager::locate_resource("resources/x_button_14.png").generic_string();
    
    wxBitmap collapse_bitmap_up=ImagesManager::get_bitmap(collapse_tri_16);
    wxBitmap collapse_bitmap_down=ImagesManager::get_bitmap(expand_tri_16);
    wxBitmap down_bitmap=ImagesManager::get_bitmap(down_arrow_16);
    wxBitmap up_bitmap=ImagesManager::get_bitmap(up_arrow_16);
    wxBitmap kill_bitmap=ImagesManager::get_bitmap(x_button_14);
    
    collapse_btn->SetBitmap(collapse_bitmap_up);
    collapse_btn->SetBitmapPressed(collapse_bitmap_down);
    collapse_btn->SetValue(true);
    
    down_btn->SetBitmap(down_bitmap);
    up_btn->SetBitmap(up_bitmap);
    
    kill->SetBitmap(kill_bitmap);
    
    wxPanel *buffer1=new wxPanel(this);
    wxPanel *buffer2=new wxPanel(this);
    
    header_sizer->Add(collapse_btn,wxSizerFlags());
    header_sizer->Add(buffer1,wxSizerFlags(1));
    header_sizer->Add(down_btn,wxSizerFlags());
    header_sizer->Add(up_btn,wxSizerFlags());
    header_sizer->Add(buffer2,wxSizerFlags(1));
    header_sizer->Add(kill,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    title=new wxStaticText(this,wxID_ANY,"");
    
    sizer->Add(header_sizer,wxSizerFlags().Expand());
    sizer->Add(title,wxSizerFlags().Expand());
    
    collapse_btn->Bind(wxEVT_TOGGLEBUTTON,&PanelsListBase::evt_collapse,this);
    down_btn->Bind(wxEVT_BUTTON,&PanelsListBase::evt_down,this);
    up_btn->Bind(wxEVT_BUTTON,&PanelsListBase::evt_up,this);
    kill->Bind(wxEVT_BUTTON,&PanelsListBase::apoptose,this);
    
    SetWindowStyle(wxBORDER_RAISED);
    
    SetSizer(sizer);
}

void PanelsListBase::apoptose(wxCommandEvent &event)
{
    wxCommandEvent k_event(EVT_PLIST_REMOVE);
    k_event.SetClientData(this);
        
    wxPostEvent(this,k_event);
}

void PanelsListBase::collapse() {}
void PanelsListBase::evt_collapse(wxCommandEvent &event_)
{
    if(!collapse_btn->GetValue()) collapse();
    else expand();
    
    wxCommandEvent event(EVT_PLIST_RESIZE);
    event.SetClientData(this);
    
    wxPostEvent(this,event);
}

void PanelsListBase::evt_down(wxCommandEvent &event_)
{
    wxCommandEvent event(EVT_PLIST_DOWN);
    event.SetClientData(this);
    
    wxPostEvent(this,event);
}

void PanelsListBase::evt_up(wxCommandEvent &event_)
{
    wxCommandEvent event(EVT_PLIST_UP);
    event.SetClientData(this);
    
    wxPostEvent(this,event);
}

void PanelsListBase::expand() { }

void PanelsListBase::header_insert(int pos,wxWindow *window,wxSizerFlags const &flags)
{
    header_sizer->Insert(pos,window,flags);
    Refresh();
}

void PanelsListBase::lock()
{
    down_btn->Disable();
    kill->Disable();
    up_btn->Disable();
}

void PanelsListBase::set_title(wxString title_)
{
    title->SetLabel(title_);
}

void PanelsListBase::throw_resize_event()
{
    wxCommandEvent event(EVT_PLIST_RESIZE);
    event.SetClientData(this);
    
    wxPostEvent(this,event);
}

void PanelsListBase::unlock()
{
    down_btn->Enable();
    kill->Enable();
    up_btn->Enable();
}

//####################
//     PanelsList
//####################

//PanelsList::PanelsList(wxWindow *parent)
//    :wxPanel(parent)
//{
//    sizer=new wxBoxSizer(wxVERTICAL);
//    
//    SetSizer(sizer);
//    
//    SetWindowStyle(wxBORDER_SUNKEN);
//    
//    Bind(EVT_PLIST_DOWN,&PanelsList::evt_panel_down,this);
//    Bind(EVT_PLIST_REMOVE,&PanelsList::evt_panel_remove,this);
//    Bind(EVT_PLIST_RESIZE,&PanelsList::evt_panel_resize,this);
//    Bind(EVT_PLIST_UP,&PanelsList::evt_panel_up,this);
//}

//void PanelsList::clear()
//{
//    for(unsigned int i=0;i<panels.size();i++)
//    {
//        panels[i]->Destroy();
//    }
//    
//    panels.clear();
//    
//    Layout();
//}

//void PanelsList::evt_panel_down(wxCommandEvent &event)
//{
//    for(unsigned int i=0;i<panels.size()-1;i++)
//    {
//        if(panels[i]==event.GetClientData())
//        {
//            std::swap(panels[i],panels[i+1]);
//            break;
//        }
//    }
//    
//    sizer->Clear();
//    for(unsigned int i=0;i<panels.size();i++)
//        sizer->Add(panels[i],wxSizerFlags().Expand().Border(wxALL,1));
//    
//    Layout();
//    
//    event.Skip();
//}

//void PanelsList::evt_panel_remove(wxCommandEvent &event)
//{
//    PanelsListBase *panel=reinterpret_cast<PanelsListBase*>(event.GetClientData());
//    
//    for(unsigned int i=0;i<panels.size();i++)
//    {
//        if(panels[i]==panel)
//        {
//            for(unsigned int j=i+1;j<panels.size();j++) panels[j-1]=panels[j];
//            panels.pop_back();
//            break;
//        }
//    }
//    
//    panel->Destroy();
//    
//    Layout();
//    
//    event.Skip();
//}

//void PanelsList::evt_panel_resize(wxCommandEvent &event)
//{
//    event.Skip();
//}

//void PanelsList::evt_panel_up(wxCommandEvent &event)
//{
//    for(unsigned int i=1;i<panels.size();i++)
//    {
//        if(panels[i]==event.GetClientData())
//        {
//            std::swap(panels[i],panels[i-1]);
//            break;
//        }
//    }
//    
//    sizer->Clear();
//    for(unsigned int i=0;i<panels.size();i++)
//        sizer->Add(panels[i],wxSizerFlags().Expand().Border(wxALL,1));
//    
//    Layout();
//    
//    event.Skip();
//}
//
//int PanelsList::get_size() { return panels.size(); }

//PanelsListBase* PanelsList::get_panel(unsigned int n)
//{
//    if(n>=panels.size()) return nullptr;
//    else return panels[n];
//}

//void PanelsList::lock()
//{
//    for(unsigned int i=0;i<panels.size();i++) panels[i]->lock();
//}
//
//void PanelsList::unlock()
//{
//    for(unsigned int i=0;i<panels.size();i++) panels[i]->unlock();
//}
