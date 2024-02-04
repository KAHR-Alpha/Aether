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

#include <gui_fdtd_structdesign_variables.h>

//#####################
//   VariablesPanel
//#####################

VariablesPanel::VariablesPanel(wxWindow *parent,
                               std::string const &name_,
                               std::string const &expression_)
    :PanelsListBase(parent)
{
    name=new NamedTextCtrl<std::string>(this,"Name: ",name_);
    expression=new NamedTextCtrl<std::string>(this,"Value: ",expression_);

    wxBoxSizer *subsizer=new wxBoxSizer(wxHORIZONTAL);

    subsizer->Add(name,wxSizerFlags(1));
    subsizer->Add(expression,wxSizerFlags(3));

    sizer->Add(subsizer,wxSizerFlags().Expand());
}

//#####################
//   VariablesDialog
//#####################

VariablesDialog::VariablesDialog(std::vector<std::string> &names_,
                                 std::vector<std::string> &expressions_)
    :wxDialog(nullptr,wxID_ANY,
              "Variables",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     names(names_),
     expressions(expressions_)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    wxButton *add_btn=new wxButton(this,wxID_ANY,"Add Variable");
    add_btn->Bind(wxEVT_BUTTON,&VariablesDialog::evt_add_variable,this);
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);

    btn_sizer->Add(new wxPanel(this),wxSizerFlags(1));
    btn_sizer->Add(add_btn);
    sizer->Add(btn_sizer,wxSizerFlags().Expand());

    controls=new wxScrolledWindow(this);
    variables=new PanelsList<VariablesPanel>(controls);

    for(std::size_t i=0;i<names.size();i++)
    {
        variables->add_panel<VariablesPanel>(names[i],expressions[i]);
    }

    wxBoxSizer *controls_sizer=new wxBoxSizer(wxVERTICAL);
    controls_sizer->Add(variables,wxSizerFlags(1).Expand());
    controls->SetSizer(controls_sizer);
    controls->SetScrollRate(0,50);
    controls->FitInside();

    sizer->Add(controls,wxSizerFlags(1).Expand());

    SetSizer(sizer);

    Bind(wxEVT_CLOSE_WINDOW,&VariablesDialog::evt_close,this);

    ShowModal();
}

void VariablesDialog::evt_add_variable(wxCommandEvent &event)
{
    variables->add_panel<VariablesPanel>("var","0");
    controls->FitInside();
    Layout();
}

void VariablesDialog::evt_close(wxCloseEvent &event)
{
    names.clear();
    expressions.clear();

    for(std::size_t i=0;i<variables->get_size();i++)
    {
        names.push_back(variables->get_panel(i)->name->get_value());
        expressions.push_back(variables->get_panel(i)->expression->get_value());
    }

    event.Skip();
}