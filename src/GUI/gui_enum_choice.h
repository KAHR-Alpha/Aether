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

#include <wx/choice.h>

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

#endif
