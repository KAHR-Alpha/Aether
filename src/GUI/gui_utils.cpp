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
#include <spectral_color.h>

#include <gui.h>
#include <gui_rsc.h>

wxDEFINE_EVENT(EVT_NAMEDTXTCTRL,wxCommandEvent);

//##################
//   ChoiceDialog
//##################

ChoiceDialog::ChoiceDialog(wxString const &title,
                           std::vector<wxString> const &labels)
    :wxDialog(0,wxID_ANY,"",
              wxGetApp().default_dialog_origin()),
     choice_ok(false)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    const int N=labels.size();
    
    wxArrayString choices;
    choices.Alloc(N);
    
    for(int i=0;i<N;i++)
        choices.Add(labels[i]);
    
    choice_ctrl=new wxRadioBox(this,wxID_ANY,title,wxDefaultPosition,wxDefaultSize,choices,1);
    
    choice_ctrl->SetSelection(0);
    
    top_sizer->Add(choice_ctrl,wxSizerFlags().Expand().Border(wxALL,3));
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Border(wxALL,2).Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&ChoiceDialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&ChoiceDialog::evt_ok,this);
    
    SetSizerAndFit(top_sizer);
    ShowModal();
}

void ChoiceDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void ChoiceDialog::evt_ok(wxCommandEvent &event)
{
    choice=choice_ctrl->GetSelection();
    choice_ok=true;
    
    Close();
}

//########################
//   ExportChoiceDialog
//########################

ExportChoiceDialog::ExportChoiceDialog(wxString const &title,
                                       std::vector<wxString> const &labels,
                                       std::vector<wxString> const &file_types_)
    :wxDialog(0,wxID_ANY,"",
              wxGetApp().default_dialog_origin()),
     choice_ok(false),
     export_type(-1),
     file_types(file_types_)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    const int N=labels.size();
    
    wxArrayString choices;
    choices.Alloc(N);
    
    for(int i=0;i<N;i++)
        choices.Add(labels[i]);
    
    type_ctrl=new wxRadioBox(this,wxID_ANY,title,wxDefaultPosition,wxDefaultSize,choices,1);
    
    type_ctrl->SetSelection(0);
    
    top_sizer->Add(type_ctrl,wxSizerFlags().Expand().Border(wxALL,3));
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Border(wxALL,2).Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&ExportChoiceDialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&ExportChoiceDialog::evt_ok,this);
    
    SetSizerAndFit(top_sizer);
    ShowModal();
}

void ExportChoiceDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void ExportChoiceDialog::evt_ok(wxCommandEvent &event)
{
    export_type=type_ctrl->GetSelection();
    
    wxString wildcard=file_types[export_type];
        
    wxFileName fname_=wxFileSelector("Select the file to save to:",
                                     wxEmptyString,wxEmptyString,wxEmptyString,
                                     wildcard,
                                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    fname=fname_.GetFullPath().ToStdString();
    
    if(fname.empty()) return;
    
    choice_ok=true;
    Close();
}

//################
//   ColorPanel
//################

ColorPanel::ColorPanel(wxWindow *parent,wxString const &name,
                       double X_,double Y_,double Z_)
    :wxPanel(parent)
{
    wxStaticBoxSizer *main_sizer=new wxStaticBoxSizer(wxVERTICAL,this,name);
    
    wxBoxSizer *xyz_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *rgb_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *rgbn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    X=new NamedTextCtrl<double>(this,"X: ",X_);
    Y=new NamedTextCtrl<double>(this,"Y: ",Y_);
    Z=new NamedTextCtrl<double>(this,"Z: ",Z_);
    
    X->lock();
    Y->lock();
    Z->lock();
    
    xyz_sizer->Add(X,wxSizerFlags(1));
    xyz_sizer->Add(Y,wxSizerFlags(1).Border(wxLEFT,5));
    xyz_sizer->Add(Z,wxSizerFlags(1).Border(wxLEFT,5));
    
    double R_,G_,B_;
    
    XYZ_to_sRGB(X_,Y_,Z_,R_,G_,B_);
    
    R=new NamedTextCtrl<double>(this,"R: ",R_);
    G=new NamedTextCtrl<double>(this,"G: ",G_);
    B=new NamedTextCtrl<double>(this,"B: ",B_);
    
    R->lock();
    G->lock();
    B->lock();
    
    rgb_sizer->Add(R,wxSizerFlags(1));
    rgb_sizer->Add(G,wxSizerFlags(1).Border(wxLEFT,5));
    rgb_sizer->Add(B,wxSizerFlags(1).Border(wxLEFT,5));
    
    double c_max=var_max(R_,G_,B_);
    
    Rn=new NamedTextCtrl<double>(this,"Rn: ",R_/c_max);
    Gn=new NamedTextCtrl<double>(this,"Gn: ",G_/c_max);
    Bn=new NamedTextCtrl<double>(this,"Bn: ",B_/c_max);
    
    Rn->lock();
    Gn->lock();
    Bn->lock();
    
    rgbn_sizer->Add(Rn,wxSizerFlags(1));
    rgbn_sizer->Add(Gn,wxSizerFlags(1).Border(wxLEFT,5));
    rgbn_sizer->Add(Bn,wxSizerFlags(1).Border(wxLEFT,5));
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    rgb_btn=new wxButton(this,wxID_ANY,"sRGB");
    rgbn_btn=new wxButton(this,wxID_ANY,"Normalized sRGB");
    
    rgb_btn->SetBackgroundColour(wxColour(255*R_,255*G_,255*B_));
    rgb_btn->SetMinSize(wxSize(1,100));
    rgbn_btn->SetBackgroundColour(wxColour(255*R_/c_max,255*G_/c_max,255*B_/c_max));
    rgbn_btn->SetMinSize(wxSize(1,100));
    
    btn_sizer->Add(rgb_btn,wxSizerFlags(1));
    btn_sizer->Add(rgbn_btn,wxSizerFlags(1));
    
    main_sizer->Add(xyz_sizer,wxSizerFlags().Expand());
    main_sizer->Add(rgb_sizer,wxSizerFlags().Expand());
    main_sizer->Add(rgbn_sizer,wxSizerFlags().Expand());
    main_sizer->Add(btn_sizer,wxSizerFlags().Expand());
    
    SetSizer(main_sizer);
}

void ColorPanel::set_XYZ(double X_,double Y_,double Z_)
{
    X->set_value(X_);
    Y->set_value(Y_);
    Z->set_value(Z_);
     
    Vector3 color;
    
    XYZ_to_sRGB(X_,Y_,Z_,color.x,color.y,color.z);
    
    R->set_value(color.x);
    G->set_value(color.y);
    B->set_value(color.z);
    
    color.normalize();
    
    Rn->set_value(color.x);
    Gn->set_value(color.y);
    Bn->set_value(color.z);
}

//###################
//   NamedTextCtrl
//###################

NamedTextCtrlDialog::NamedTextCtrlDialog(OptimRule *rule_)
    :OptimRule(*rule_),
     wxDialog(0,wxID_ANY,"Parameters",
              wxGetApp().default_dialog_origin()),
     rule_holder(rule_),
     selection_ok(false)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    // 
    
    lock=new wxCheckBox(this,wxID_ANY,"Lock optimization");
    lock->SetValue(rule_holder->lock);
    
    sizer->Add(lock);
    
    // Type
    
    wxStaticBoxSizer *op_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Operation type");
    
    operation_type=new wxChoice(this,wxID_ANY);
    operation_type->Append("Add");
    operation_type->Append("Growth");
    
         if(rule_holder->operation_type==OptimRule::Operation::ADD) operation_type->SetSelection(0);
    else if(rule_holder->operation_type==OptimRule::Operation::GROWTH) operation_type->SetSelection(1);
    
    op_sizer->Add(operation_type,wxSizerFlags().Expand());
    sizer->Add(op_sizer,wxSizerFlags().Expand());
    
    // Delta
    
    delta=new NamedTextCtrl<double>(this,"Delta",rule_holder->delta,true);
    sizer->Add(delta,wxSizerFlags().Expand());
    
    // Limits
    
    wxStaticBoxSizer *limit_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Limits");
    
    wxBoxSizer *limit_type_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxStaticText *limit_type_txt=new wxStaticText(this,wxID_ANY,"Type: ");
    
    limit_type=new wxChoice(this,wxID_ANY);
    limit_type->Append("Up");
    limit_type->Append("Down");
    limit_type->Append("Both");
    limit_type->Append("None");
    
         if(rule_holder->limit_type==OptimRule::Limit::UP) limit_type->SetSelection(0);
    else if(rule_holder->limit_type==OptimRule::Limit::DOWN) limit_type->SetSelection(1);
    else if(rule_holder->limit_type==OptimRule::Limit::BOTH) limit_type->SetSelection(2);
    else if(rule_holder->limit_type==OptimRule::Limit::NONE) limit_type->SetSelection(3);
    
    #ifndef WX30_RESTRICT
    limit_type_sizer->Add(limit_type_txt,wxSizerFlags().CenterVertical());
    limit_type_sizer->Add(limit_type,wxSizerFlags(1).CenterVertical());
    #else
    limit_type_sizer->Add(limit_type_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    limit_type_sizer->Add(limit_type,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
    #endif
    
    limit_sizer->Add(limit_type_sizer,wxSizerFlags().Expand());
    
    limit_down=new NamedTextCtrl<double>(this,"Down: ",rule_holder->limit_down);
    limit_up=new NamedTextCtrl<double>(this,"Up: ",rule_holder->limit_up);
    
    limit_sizer->Add(limit_down,wxSizerFlags().Expand());
    limit_sizer->Add(limit_up,wxSizerFlags().Expand());
    
    sizer->Add(limit_sizer,wxSizerFlags().Expand());
    
    // Buttons
    
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    ok_btn->Bind(wxEVT_BUTTON,&NamedTextCtrlDialog::evt_ok,this);
    cancel_btn->Bind(wxEVT_BUTTON,&NamedTextCtrlDialog::evt_cancel,this);
    
    buttons_sizer->Add(ok_btn);
    buttons_sizer->Add(cancel_btn);
    
    sizer->Add(buttons_sizer,wxSizerFlags().Border(wxALL,3).Align(wxALIGN_RIGHT));
    
    // Wrapping Up
    
    SetSizerAndFit(sizer);
    ShowModal();
}

void NamedTextCtrlDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void NamedTextCtrlDialog::evt_ok(wxCommandEvent &event)
{
    selection_ok=true;
    
    save();
    Close();
}

void NamedTextCtrlDialog::save()
{
    OptimRule::lock=lock->GetValue();
    
    int op_selection=operation_type->GetSelection();
    
         if(op_selection==0) OptimRule::operation_type=OptimRule::Operation::ADD;
    else if(op_selection==1) OptimRule::operation_type=OptimRule::Operation::GROWTH;
    
    OptimRule::delta=delta->get_value();
    
    int limit_selection=limit_type->GetSelection();
    
         if(limit_selection==0) OptimRule::limit_type=OptimRule::Limit::UP;
    else if(limit_selection==1) OptimRule::limit_type=OptimRule::Limit::DOWN;
    else if(limit_selection==2) OptimRule::limit_type=OptimRule::Limit::BOTH;
    else if(limit_selection==3) OptimRule::limit_type=OptimRule::Limit::NONE;
    
    OptimRule::limit_down=limit_down->get_value();
    OptimRule::limit_up=limit_up->get_value();
    
    OptimRule *base_p=this;
    
    (*rule_holder)=*base_p;
}


//####################
//   NamedSymCtrl
//####################

NamedSymCtrl::NamedSymCtrl(wxWindow *parent,std::string name,double const &x,bool static_style)
    :wxPanel(parent), owned(true)
{
    val=new SymNode;
    
    wxSizer *sizer=nullptr;
    if(static_style) sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    else sizer=new wxBoxSizer(wxHORIZONTAL);
    
    std::stringstream strm;
    strm<<x;
    
    val->set_expression(strm.str());
    
    if(!static_style)
    {
        name_ctrl=new wxStaticText(this,wxID_ANY,wxString(name));
        sizer->Add(name_ctrl,wxSizerFlags().Align(wxCENTER));
    }
    
    txt=new wxTextCtrl(this,wxID_ANY,wxString(strm.str()),
                       wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    sizer->Add(txt,wxSizerFlags(1).Expand());
    
    txt->Bind(wxEVT_TEXT_ENTER,&NamedSymCtrl::value_enter,this);
    txt->Bind(wxEVT_KILL_FOCUS,&NamedSymCtrl::value_focus,this);
    
    SetSizer(sizer);
}

NamedSymCtrl::NamedSymCtrl(wxWindow *parent,std::string name,std::string const &x,bool static_style)
    :wxPanel(parent), owned(true)
{
    val=new SymNode;
    
    wxSizer *sizer=nullptr;
    if(static_style) sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    else sizer=new wxBoxSizer(wxHORIZONTAL);
    
    val->set_expression(x);
    
    if(!static_style)
    {
        name_ctrl=new wxStaticText(this,wxID_ANY,wxString(name));
        sizer->Add(name_ctrl,wxSizerFlags().Align(wxCENTER));
    }
    
    txt=new wxTextCtrl(this,wxID_ANY,wxString(x),
                       wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    sizer->Add(txt,wxSizerFlags(1).Expand());
    
    txt->Bind(wxEVT_TEXT_ENTER,&NamedSymCtrl::value_enter,this);
    txt->Bind(wxEVT_KILL_FOCUS,&NamedSymCtrl::value_focus,this);
    
    SetSizer(sizer);
}

NamedSymCtrl::NamedSymCtrl(wxWindow *parent,std::string name,SymNode *val_,bool static_style)
    :wxPanel(parent), owned(false), val(val_)
{
    wxSizer *sizer=nullptr;
    if(static_style) sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    else sizer=new wxBoxSizer(wxHORIZONTAL);
    
    if(!static_style)
    {
        name_ctrl=new wxStaticText(this,wxID_ANY,wxString(name));
        sizer->Add(name_ctrl,wxSizerFlags().Align(wxCENTER));
    }
    
    txt=new wxTextCtrl(this,wxID_ANY,wxString(val->get_expression()),
                       wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    sizer->Add(txt,wxSizerFlags(1).Expand());
    
    txt->Bind(wxEVT_TEXT_ENTER,&NamedSymCtrl::value_enter,this);
    txt->Bind(wxEVT_KILL_FOCUS,&NamedSymCtrl::value_focus,this);
    
    SetSizer(sizer);
}

NamedSymCtrl::~NamedSymCtrl()
{
    if(owned) delete val;
}

SymNode* NamedSymCtrl::get_node() { return val; }

double NamedSymCtrl::get_value()
{
    return val->evaluate();
}

std::string NamedSymCtrl::get_text()
{
    return txt->GetValue().ToStdString();
}

int NamedSymCtrl::get_value_integer()
{
    return static_cast<int>(val->evaluate()+0.5);
}

void NamedSymCtrl::hide()
{
    name_ctrl->Hide();
    txt->Hide();
    
    Layout();
}

void NamedSymCtrl::set_expression(std::string const &expression)
{
    val->set_expression(expression);
    txt->ChangeValue(wxString(expression));
}

void NamedSymCtrl::set_lib(SymLib *lib)
{
    val->set_lib(lib);
}

bool NamedSymCtrl::requires_any(std::vector<std::string> const &var)
{
    return val->requires_any(var);
}

void NamedSymCtrl::set_node(SymNode *val_)
{
    if(owned) delete val;
    
    owned=false;
    val=val_;
    
    txt->ChangeValue(wxString(val->get_expression()));
}

void NamedSymCtrl::show()
{
    name_ctrl->Show();
    txt->Show();
    
    Layout();
}

void NamedSymCtrl::value_enter(wxCommandEvent &event)
{
    std::string frm=txt->GetValue().ToStdString();
    
    if(frm.size()==0)
    {
        frm="0";
        txt->ChangeValue("0");
    }
    
    val->set_expression(frm);
    
    wxCommandEvent event_out(EVT_NAMEDTXTCTRL);
    wxPostEvent(this,event_out);
}

void NamedSymCtrl::value_focus(wxFocusEvent &event)
{
    std::string frm=txt->GetValue().ToStdString();
    
    if(frm.size()==0)
    {
        frm="0";
        txt->ChangeValue("0");
    }
    
    val->set_expression(frm);
    
    wxCommandEvent event_out(EVT_NAMEDTXTCTRL);
    wxPostEvent(this,event_out);
    
    event.Skip();
}



//###################
//   SliderDisplay
//###################

wxDEFINE_EVENT(EVT_SLIDERDISPLAY,wxCommandEvent);

SliderDisplay::SliderDisplay(wxWindow *parent,int N_,double min_val_,double max_val_,double scale_,std::string const &unit_,bool vertical)
    :wxPanel(parent),
     N(N_-1), min_val(min_val_), max_val(max_val_), scale(scale_), unit(unit_)
{
    wxBoxSizer *sizer;
    
    if(!vertical) sizer=new wxBoxSizer(wxHORIZONTAL);
    else sizer=new wxBoxSizer(wxVERTICAL);
    
    sld=new wxSlider(this,wxID_ANY,0,0,N);
    text_val=new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    
    update_display();
    
    sizer->Add(sld,wxSizerFlags(1).Expand());
    if(!vertical) sizer->Add(text_val);
    else sizer->Add(text_val,wxSizerFlags().Align(wxALIGN_CENTER_HORIZONTAL)); // to be replaced with CenterHorizontal
    
    SetSizer(sizer);
    
    sld->Bind(wxEVT_SLIDER,&SliderDisplay::evt_sld,this);
}

SliderDisplay::~SliderDisplay()
{
}

void SliderDisplay::evt_sld(wxCommandEvent &event)
{
    update_display();
    
    wxCommandEvent event_out(EVT_SLIDERDISPLAY);
    wxPostEvent(this,event_out);
    
    event.Skip();
}

double SliderDisplay::get_value()
{
    return min_val+(max_val-min_val)*sld->GetValue()/N;
}

void SliderDisplay::set_N(int N_)
{
    double curr_val=get_value();
    
    N=N_-1;
    sld->SetRange(0,N);
    
    set_value(curr_val);
}

void SliderDisplay::set_range(double min_val_,double max_val_)
{
    double curr_val=get_value();
    
    min_val=min_val_;
    max_val=max_val_;
    
    set_value(curr_val);
}

void SliderDisplay::set_scale(double scale_) { scale=scale_; }

void SliderDisplay::set_unit(std::string const &unit_) { unit=unit_; }

void SliderDisplay::set_value(double value)
{
    double u=N*(value-min_val)/(max_val-min_val);
    sld->SetValue(static_cast<int>(u+0.5));
    update_display();
}

void SliderDisplay::update_display()
{
    std::stringstream strm;
    strm<<get_value()/scale<<" "<<unit;
    text_val->ChangeValue(strm.str());
    
}

//##############
//   PMLPanel
//##############

PMLPanel::PMLPanel(wxWindow *parent,std::string const &name)
    :PMLPanel(parent,name,25,25,1,0.2)
{
}

PMLPanel::PMLPanel(wxWindow *parent,std::string const &name,
                   int N_pml,double k_max,double s_max,double a_max)
    :wxPanel(parent)
{
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,wxString(name));
    N_ctrl=new NamedTextCtrl<int>(this,"N: ",N_pml);
    k_max_ctrl=new NamedTextCtrl<double>(this,"",k_max);
    s_max_ctrl=new NamedTextCtrl<double>(this,"",s_max);
    a_max_ctrl=new NamedTextCtrl<double>(this,"",a_max);
    
    wxBoxSizer *k_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *s_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *a_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    std::string kappa_16=PathManager::locate_resource("resources/kappa_16.png").generic_string();
    std::string sigma_16=PathManager::locate_resource("resources/sigma_16.png").generic_string();
    std::string alpha_16=PathManager::locate_resource("resources/alpha_16.png").generic_string();
    
    wxGenericStaticBitmap *k_bmp=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(kappa_16));
    wxGenericStaticBitmap *s_bmp=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(sigma_16));
    wxGenericStaticBitmap *a_bmp=new wxGenericStaticBitmap(this,wxID_ANY,ImagesManager::get_bitmap(alpha_16));
    
    k_sizer->Add(k_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    s_sizer->Add(s_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    a_sizer->Add(a_bmp,wxSizerFlags().Border(wxLEFT,2).Border(wxRIGHT,2).Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    
    k_sizer->Add(k_max_ctrl,wxSizerFlags(1).Expand());
    s_sizer->Add(s_max_ctrl,wxSizerFlags(1).Expand());
    a_sizer->Add(a_max_ctrl,wxSizerFlags(1).Expand());
    
    sizer->Add(N_ctrl,wxSizerFlags().Expand());
    sizer->Add(k_sizer,wxSizerFlags().Expand());
    sizer->Add(s_sizer,wxSizerFlags().Expand());
    sizer->Add(a_sizer,wxSizerFlags().Expand());
    
    SetSizer(sizer);
}

void PMLPanel::get_parameters(int &N_pml,double &k_max,double &s_max,double &a_max)
{
    N_pml=N_ctrl->get_value();
    k_max=k_max_ctrl->get_value();
    s_max=s_max_ctrl->get_value();
    a_max=a_max_ctrl->get_value();
}

//#########################
//   WavelengthSelector
//#########################

wxDEFINE_EVENT(EVT_SPECTRUM_UPDATE,wxCommandEvent);

wxDEFINE_EVENT(EVT_LENGTH_SELECTOR,wxCommandEvent);
wxDEFINE_EVENT(EVT_SPECTRUM_SELECTOR,wxCommandEvent);
wxDEFINE_EVENT(EVT_WAVELENGTH_SELECTOR,wxCommandEvent);

LengthSelector::LengthSelector(wxWindow *parent,std::string name,double L_,bool static_style,std::string const &zero_unit)
    :wxPanel(parent),
     L(L_)
{
    wxSizer *sizer=nullptr;
    
    std::stringstream strm;
    strm<<L*1e9;
    
    wxString choices[]={"nm","microns","mm","m"};
    
    length_ctrl=new wxTextCtrl(this,wxID_ANY,strm.str(),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    unit_ctrl=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,4,choices);
    unit_ctrl->SetSelection(0);
    
    auto_unit();
    if(L==0) set_unit(zero_unit);
    
    if(static_style)
    {
        sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
        
        sizer->Add(length_ctrl,wxSizerFlags(1));
        sizer->Add(unit_ctrl);
    }
    else
    {
        sizer=new wxBoxSizer(wxHORIZONTAL);
        
        wxStaticText *name_ctrl=new wxStaticText(this,wxID_ANY,wxString(name));
        
        sizer->Add(name_ctrl,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
        sizer->Add(length_ctrl,wxSizerFlags(1));
        sizer->Add(unit_ctrl);
    }
    
    length_ctrl->Bind(wxEVT_TEXT_ENTER,&LengthSelector::value_enter_event,this);
    length_ctrl->Bind(wxEVT_KILL_FOCUS,&LengthSelector::value_focus_event,this);
    unit_ctrl->Bind(wxEVT_CHOICE,&LengthSelector::unit_event,this);
    
    SetSizer(sizer);
}

void LengthSelector::auto_unit()
{
    double disp_value=0;
    
    if(L==0)
    {
        unit_ctrl->SetSelection(3);
    }
    else
    {
        double ln_abs=std::log10(std::abs(L));
        
             if(ln_abs>=0)
        {
            disp_value=L;
            unit_ctrl->SetSelection(3);
        }
        else if(ln_abs>=-3.0)
        {
            disp_value=m_to_mm(L);
            unit_ctrl->SetSelection(2);
        }
        else if(ln_abs>=-6.0)
        {
            disp_value=m_to_microns(L);
            unit_ctrl->SetSelection(1);
        }
        else if(ln_abs>=-9.0)
        {
            disp_value=m_to_nm(L);
            unit_ctrl->SetSelection(0);
        }
    }
    
    std::stringstream strm;
    strm<<disp_value;
    
    length_ctrl->SetValue(strm.str());
}

double LengthSelector::get_length() { return L; }

void LengthSelector::set_length(double L_)
{
    L=L_;
    auto_unit();
}

void LengthSelector::set_unit(std::string const &unit)
{
    double disp_value=0;

         if(unit=="mm")
    {
        disp_value=m_to_mm(L);
        unit_ctrl->SetSelection(2);
    }
    else if(unit=="um")
    {
        disp_value=m_to_microns(L);
        unit_ctrl->SetSelection(1);
    }
    else if(unit=="nm")
    {
        disp_value=m_to_nm(L);
        unit_ctrl->SetSelection(0);
    }
    else
    {
        disp_value=L;
        unit_ctrl->SetSelection(3);
    }
    
    std::stringstream strm;
    strm<<disp_value;
    
    length_ctrl->SetValue(strm.str());
}

void LengthSelector::unit_event(wxCommandEvent &event)
{
    double disp_value=0;
    
    int selection=unit_ctrl->GetSelection();
    
         if(selection==0) disp_value=m_to_nm(L);
    else if(selection==1) disp_value=m_to_microns(L);
    else if(selection==2) disp_value=m_to_mm(L);
    else if(selection==3) disp_value=L;
    
    std::stringstream strm;
    strm<<disp_value;
    
    length_ctrl->SetValue(strm.str());
}

void LengthSelector::value_change()
{
    double input_value=0;
    
    textctrl_to_T(length_ctrl,input_value);
    
    int selection=unit_ctrl->GetSelection();
    
         if(selection==0) L=nm_to_m(input_value);
    else if(selection==1) L=microns_to_m(input_value);
    else if(selection==2) L=mm_to_m(input_value);
    else if(selection==3) L=input_value;
    
    wxCommandEvent event(EVT_LENGTH_SELECTOR);
    
    wxPostEvent(this,event);
}

void LengthSelector::value_enter_event(wxCommandEvent &event)
{
    value_change();
}

void LengthSelector::value_focus_event(wxFocusEvent &event)
{
    value_change();
    
    event.Skip();
}

//#########################
//   WavelengthSelector
//#########################

WavelengthSelector::WavelengthSelector(wxWindow *parent,std::string name,double lambda_)
    :wxPanel(parent),
     lambda(lambda_)
{
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    std::stringstream strm;
    strm<<lambda*1e9;
    
    wxString choices[]={"nm","microns","m","THz","Hz","eV","1/cm"};
    
    lambda_ctrl=new wxTextCtrl(this,wxID_ANY,strm.str(),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    unit_ctrl=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,7,choices);
    unit_ctrl->SetSelection(0);
    
    sizer->Add(lambda_ctrl,wxSizerFlags(1));
    sizer->Add(unit_ctrl);
    
    lambda_ctrl->Bind(wxEVT_TEXT_ENTER,&WavelengthSelector::value_enter_event,this);
    lambda_ctrl->Bind(wxEVT_KILL_FOCUS,&WavelengthSelector::value_focus_event,this);
    unit_ctrl->Bind(wxEVT_CHOICE,&WavelengthSelector::unit_event,this);
    
    SetSizer(sizer);
}

double WavelengthSelector::get_lambda() { return lambda; }

void WavelengthSelector::lock()
{
    lambda_ctrl->SetEditable(false);
}

void WavelengthSelector::set_lambda(double lambda_)
{
    lambda=lambda_;
    
    double disp_value=0;
    
    int selection=unit_ctrl->GetSelection();
    
         if(selection==0) disp_value=m_to_nm(lambda);
    else if(selection==1) disp_value=m_to_microns(lambda);
    else if(selection==2) disp_value=lambda;
    else if(selection==3) disp_value=m_to_THz(lambda);
    else if(selection==4) disp_value=m_to_Hz(lambda);
    else if(selection==5) disp_value=m_to_eV(lambda);
    else if(selection==6) disp_value=m_to_inv_cm(lambda);
    
    std::stringstream strm;
    strm<<disp_value;
    
    lambda_ctrl->SetValue(strm.str());
}

void WavelengthSelector::unit_event(wxCommandEvent &event)
{
    double disp_value=0;
    
    int selection=unit_ctrl->GetSelection();
    
         if(selection==0) disp_value=m_to_nm(lambda);
    else if(selection==1) disp_value=m_to_microns(lambda);
    else if(selection==2) disp_value=lambda;
    else if(selection==3) disp_value=m_to_THz(lambda);
    else if(selection==4) disp_value=m_to_Hz(lambda);
    else if(selection==5) disp_value=m_to_eV(lambda);
    else if(selection==6) disp_value=m_to_inv_cm(lambda);
    
    std::stringstream strm;
    strm<<disp_value;
    
    lambda_ctrl->SetValue(strm.str());
}

void WavelengthSelector::unlock()
{
    lambda_ctrl->SetEditable(true);
}

void WavelengthSelector::value_change()
{
    double input_value=0;
    
    textctrl_to_T(lambda_ctrl,input_value);
    
    int selection=unit_ctrl->GetSelection();
    
         if(selection==0) lambda=nm_to_m(input_value);
    else if(selection==1) lambda=microns_to_m(input_value);
    else if(selection==2) lambda=input_value;
    else if(selection==3) lambda=THz_to_m(input_value);
    else if(selection==4) lambda=Hz_to_m(input_value);
    else if(selection==5) lambda=eV_to_m(input_value);
    else if(selection==6) lambda=inv_cm_to_m(input_value);
    
    wxCommandEvent event(EVT_WAVELENGTH_SELECTOR);
    
    wxPostEvent(this,event);
}

void WavelengthSelector::value_enter_event(wxCommandEvent &event)
{
    value_change();
}

void WavelengthSelector::value_focus_event(wxFocusEvent &event)
{
    value_change();
    
    event.Skip();
}

//######################
//   SpectrumSelector
//######################

SpectrumSelector::SpectrumSelector(wxWindow *parent,double lambda_min_,double lambda_max_,int Np_,bool vertical)
    :wxPanel(parent),
     monochromatic(false), Np(Np_),
     lambda_min(lambda_min_),
     lambda_max(lambda_max_)
{
    wxBoxSizer *sizer;
    
    if(vertical) sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Spectrum");
    else sizer=new wxBoxSizer(wxHORIZONTAL);
    
    lambda_min_ctrl=new WavelengthSelector(this,"Lambda min",lambda_min);
    lambda_max_ctrl=new WavelengthSelector(this,"Lambda max",lambda_max);
    
    if(vertical)
    {
        sizer->Add(lambda_min_ctrl,wxSizerFlags(1).Expand());
        sizer->Add(lambda_max_ctrl,wxSizerFlags(1).Expand());
    }
    else
    {
        sizer->Add(lambda_min_ctrl,wxSizerFlags(1));
        sizer->Add(lambda_max_ctrl,wxSizerFlags(1));
    }
    
    std::stringstream strm;
    strm<<Np;
    
    wxStaticBoxSizer *Np_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"N Points");
    Np_ctrl=new wxTextCtrl(this,wxID_ANY,strm.str(),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
    Np_sizer->Add(Np_ctrl);
    
    sizer->Add(Np_sizer);
    
    lambda_min_ctrl->Bind(EVT_WAVELENGTH_SELECTOR,&SpectrumSelector::spectrum_event,this);
    lambda_max_ctrl->Bind(EVT_WAVELENGTH_SELECTOR,&SpectrumSelector::spectrum_event,this);
    Np_ctrl->Bind(wxEVT_TEXT_ENTER,&SpectrumSelector::np_enter_event,this);
    Np_ctrl->Bind(wxEVT_KILL_FOCUS,&SpectrumSelector::np_focus_event,this);
    
    SetSizer(sizer);
}

int SpectrumSelector::get_closest(double lambda)
{
    if(lambda<=lambda_min) return 0;
    else if(lambda>=lambda_max) return Np-1;
    else
    {
        double u=(Np-1.0)*(lambda-lambda_min)/(lambda_max-lambda_min);
        return static_cast<int>(u+0.5);
    }
}

double SpectrumSelector::get_lambda(int N)
{
    if(N<=0) return lambda_min;
    else if(N>=Np-1) return lambda_max;
    else return lambda_min+(lambda_max-lambda_min)*N/(Np-1.0);
}

double SpectrumSelector::get_lambda_max() { return lambda_max; }
double SpectrumSelector::get_lambda_min() { return lambda_min; }
int SpectrumSelector::get_Np() { return Np; }

void SpectrumSelector::get_spectrum(std::vector<double> &lambda)
{
    lambda.resize(Np);
    
    for(int l=0;l<Np;l++) lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Np-1.0);
}

void SpectrumSelector::np_enter_event(wxCommandEvent &event)
{
    spectrum_update();
}

void SpectrumSelector::np_focus_event(wxFocusEvent &event)
{
    spectrum_update();
    
    event.Skip();
}

void SpectrumSelector::set_Np(int Np_)
{
    Np=Np_;
    std::stringstream strm;
    strm<<Np;
    
    Np_ctrl->ChangeValue(strm.str());
}

void SpectrumSelector::set_monochromatic()
{
    monochromatic=true;
    lambda_max_ctrl->Disable();
    Np_ctrl->Disable();
}

void SpectrumSelector::set_polychromatic()
{
    monochromatic=false;
    lambda_max_ctrl->Enable();
    Np_ctrl->Enable();
}

void SpectrumSelector::set_spectrum(double lambda_min_,double lambda_max_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    
    lambda_min_ctrl->set_lambda(lambda_min);
    lambda_max_ctrl->set_lambda(lambda_max);
}

void SpectrumSelector::spectrum_update()
{
    bool last_update_min=true;
    if(lambda_min==lambda_min_ctrl->get_lambda()) last_update_min=false;
    
    lambda_min=lambda_min_ctrl->get_lambda();
    lambda_max=lambda_max_ctrl->get_lambda();
    textctrl_to_T(Np_ctrl,Np);
    
    if(lambda_min>lambda_max)
    {
        if(last_update_min) lambda_max=lambda_min;
        else lambda_min=lambda_max;
        lambda_min_ctrl->set_lambda(lambda_min);
        lambda_max_ctrl->set_lambda(lambda_max);
    }
    
    wxCommandEvent event(EVT_SPECTRUM_SELECTOR);
    
    wxPostEvent(this,event);
}

void SpectrumSelector::spectrum_event(wxCommandEvent &event)
{
    spectrum_update();
}

bool in_family_tree(wxTreeCtrl *tree,wxTreeItemId const &parent,wxTreeItemId const&child,bool recursive,bool strict)
{
    if(!parent.IsOk()) return false;
    
    if(!strict && parent==child) return true;
    else if(tree->HasChildren(parent))
    {
        wxTreeItemIdValue cookie;
        
        wxTreeItemId first=tree->GetFirstChild(parent,cookie);
        
        if(first==child) return true;
        if(recursive && tree->HasChildren(first))
        {
           if(in_family_tree(tree,first,child,true,true)) return true;
        }
        
        wxTreeItemId next=tree->GetNextChild(parent,cookie);
        
        while(next.IsOk())
        {
            if(next==child) return true;
            if(recursive && tree->HasChildren(next))
            {
               if(in_family_tree(tree,next,child,true,true)) return true;
            }
            
            next=tree->GetNextChild(parent,cookie);
        }
    }
    
    return false;
}
