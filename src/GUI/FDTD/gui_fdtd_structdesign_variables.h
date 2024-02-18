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

#ifndef GUI_FDTD_STRUCTDESIGN_VAR_H
#define GUI_FDTD_STRUCTDESIGN_VAR_H

#include <gui.h>
#include <gui_panels_list.h>

class VariablesPanel: public PanelsListBase
{
    public:
        NamedTextCtrl<std::string> *name;
        NamedTextCtrl<std::string> *expression;

        VariablesPanel(wxWindow *parent,
                       std::string const &name,
                       std::string const &expression);
};

class VariablesDialog : public wxDialog
{
    public:
        std::vector<std::string> &names;
        std::vector<std::string> &expressions;

        wxScrolledWindow *controls;
        PanelsList<VariablesPanel> *variables;

        VariablesDialog(std::vector<std::string> &names,
                        std::vector<std::string> &expressions,
                        std::string const &title);
        
        void evt_add_variable(wxCommandEvent &event);
        void evt_close(wxCloseEvent &event);
};

#endif // GUI_FDTD_STRUCTDESIGN_VAR_H
