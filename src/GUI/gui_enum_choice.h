/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GUI_ENUM_CHOICE_H
#define GUI_ENUM_CHOICE_H

#include <aether.h>

#include <wx/choice.h>
#include <wx/dialog.h>

#include <optional>
#include <vector>

template<typename T>
class EnumChoice: public wxChoice
{
    public:
        std::vector<T> items;

        EnumChoice(wxWindow *parent, wxWindowID ID)
            :wxChoice(parent,ID)
        {
        }

        int append(wxString const &item, T const &E)
        {
            items.push_back(E);
            return Append(item);
        }

        T get_selection() const
        {
            int ID=wxChoice::GetSelection();
            return items[ID];
        }

        void set_selection(T const &E)
        {
            for(std::size_t i=0; i<items.size(); i++)
            {
                if(items[i]==E)
                {
                    wxChoice::SetSelection(i);
                    return;
                }
            }
        }
};


template<typename T>
class EnumRadioDialog: public wxDialog
{
    public:
        std::vector<T> items;
        std::optional<T> selection;
        
        wxRadioBox *selector;
        
        EnumRadioDialog(wxString const &title,
                        std::vector<wxString> const &labels,
                        std::vector<T> const &items_)
            :wxDialog(0,wxID_ANY,"",
                      wxGetApp().default_dialog_origin()),
             items(items_)
        {
            wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
            
            // Selector
            
            const int N=labels.size();
    
            wxArrayString choices;
            choices.Alloc(N);
            
            for(int i=0;i<N;i++)
            choices.Add(labels[i]);
            
            selector = new wxRadioBox(this, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, choices, 0, wxVERTICAL);
            
            sizer->Add(selector);
            
            // Buttons
            
            wxBoxSizer *btn_sizer = new wxBoxSizer(wxHORIZONTAL);

            wxButton *ok_btn = new wxButton(this, wxID_ANY, "Ok");
            ok_btn->Bind(wxEVT_BUTTON, &EnumRadioDialog<T>::evt_ok, this);
            
            wxButton *cancel_btn = new wxButton(this, wxID_ANY, "Cancel");
            cancel_btn->Bind(wxEVT_BUTTON, &EnumRadioDialog::evt_cancel, this);
            
            btn_sizer->Add(ok_btn);
            btn_sizer->Add(cancel_btn);
            
            sizer->Add(btn_sizer, wxSizerFlags().Align(wxALIGN_RIGHT));
            
            SetSizerAndFit(sizer);
        }
            
        void evt_cancel(wxCommandEvent &event)
        {
            Close();
        }
        
        void evt_ok(wxCommandEvent &event)
        {
            int ID=selector->GetSelection();
            selection = items[ID];
            
            Close();
        }
        
        std::optional<T> get_selection() const
        {
            return selection;
        }
};

#endif
