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

#ifndef GUI_PANELS_LIST_H_INCLUDED
#define GUI_PANELS_LIST_H_INCLUDED

#include <gui.h>
#include <gui_rsc.h>

wxDECLARE_EVENT(EVT_PLIST_DOWN,wxCommandEvent);
wxDECLARE_EVENT(EVT_PLIST_REMOVE,wxCommandEvent);
wxDECLARE_EVENT(EVT_PLIST_RESIZE,wxCommandEvent);
wxDECLARE_EVENT(EVT_PLIST_UP,wxCommandEvent);

class PanelsListBase: public wxPanel
{
    public:
        wxBoxSizer *sizer,*header_sizer;
        
        wxToggleButton *collapse_btn;
        wxButton *down_btn,*up_btn;
        wxButton *kill;
        
        wxStaticText *title;
        
        PanelsListBase(wxWindow *parent);
        
        virtual void apoptose(wxCommandEvent &event);
        virtual void collapse();
        void evt_collapse(wxCommandEvent &event);
        void evt_down(wxCommandEvent &event);
        void evt_up(wxCommandEvent &event);
        virtual void expand();
        void header_insert(int pos,wxWindow *window,wxSizerFlags const &flags);
        virtual void lock();
        void set_title(wxString title);
        void throw_resize_event();
        virtual void unlock();
};


template<class T_Base=PanelsListBase>
class PanelsList: public wxPanel
{
    public:
        wxBoxSizer *sizer;
        std::vector<T_Base*> panels;
        
        PanelsList(wxWindow *parent)
            :wxPanel(parent)
        {
            sizer=new wxBoxSizer(wxVERTICAL);
            
            SetSizer(sizer);
            
            SetWindowStyle(wxBORDER_SUNKEN);
            
            Bind(EVT_PLIST_DOWN,&PanelsList::evt_panel_down,this);
            Bind(EVT_PLIST_REMOVE,&PanelsList::evt_panel_remove,this);
            Bind(EVT_PLIST_RESIZE,&PanelsList::evt_panel_resize,this);
            Bind(EVT_PLIST_UP,&PanelsList::evt_panel_up,this);
        }
        
        template<typename T,typename... Varg>
        T* add_panel(Varg... arg)
        {
            T *panel=new T(this,arg...);
            
            sizer->Add(panel,wxSizerFlags().Expand().Border(wxALL,1));
            panels.push_back(panel);
            
            Layout();
            
            return panel;
        }
        
        void clear()
        {
            for(unsigned int i=0;i<panels.size();i++)
            {
                panels[i]->Destroy();
            }
            
            panels.clear();
            
            Layout();
        }
        
        void evt_panel_down(wxCommandEvent &event)
        {
            for(unsigned int i=0;i<panels.size()-1;i++)
            {
                if(panels[i]==event.GetClientData())
                {
                    std::swap(panels[i],panels[i+1]);
                    break;
                }
            }
            
            sizer->Clear();
            for(unsigned int i=0;i<panels.size();i++)
                sizer->Add(panels[i],wxSizerFlags().Expand().Border(wxALL,1));
            
            Layout();
            
            event.Skip();
        }
        
        void evt_panel_remove(wxCommandEvent &event)
        {
            T_Base *panel=reinterpret_cast<T_Base*>(event.GetClientData());
            bool known_panel=false;
            
            for(unsigned int i=0;i<panels.size();i++)
            {
                if(panels[i]==panel)
                {
                    for(unsigned int j=i+1;j<panels.size();j++) panels[j-1]=panels[j];
                    panels.pop_back();
                    known_panel=true;
                    break;
                }
            }
            
            if(known_panel) panel->Destroy();
            
            Layout();
            
            event.Skip();
        }
        
        void evt_panel_resize(wxCommandEvent &event)
        {
            event.Skip();
        }
        
        void evt_panel_up(wxCommandEvent &event)
        {
            for(unsigned int i=1;i<panels.size();i++)
            {
                if(panels[i]==event.GetClientData())
                {
                    std::swap(panels[i],panels[i-1]);
                    break;
                }
            }
            
            sizer->Clear();
            for(unsigned int i=0;i<panels.size();i++)
                sizer->Add(panels[i],wxSizerFlags().Expand().Border(wxALL,1));
            
            Layout();
            
            event.Skip();
        }
        
        std::size_t get_size() { return panels.size(); }
        
        T_Base* get_panel(std::size_t n)
        {
            if(n>=panels.size()) return nullptr;
            else return panels[n];
        }
        
        void pop_back()
        {
            if(!panels.empty())
            {
                panels[panels.size()-1]->Destroy();
                panels.pop_back();
                
                Layout();
            }
        }
        
        void lock()
        {
            for(unsigned int i=0;i<panels.size();i++) panels[i]->lock();
        }
        
        void unlock()
        {
            for(unsigned int i=0;i<panels.size();i++) panels[i]->unlock();
        }
};

#endif // GUI_PANELS_LIST_H_INCLUDED
