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

#ifndef GUI_H
#define GUI_H

#include <mathUT.h>
#include <math_optim.h>
#include <math_sym.h>
#include <phys_constants.h>

#include <aether.h>

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/generic/statbmpg.h>
//#include <wx/glcanvas.h>
#include <wx/graphics.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/treectrl.h>

#include <filesystem>
#include <string>

wxDECLARE_EVENT(EVT_NAMEDTXTCTRL,wxCommandEvent);

template<typename T>
void textctrl_to_value(wxTextCtrl *ctrl,T &target)
{
    std::stringstream stream_in,stream_out;
    
    if constexpr(std::is_same_v<T,std::string>)
    {
        target=ctrl->GetValue().ToStdString();
    }
    else
    {
        stream_in<<ctrl->GetValue();
        stream_in>>target;
        
        stream_out<<target;
        ctrl->ChangeValue(stream_out.str());
    }
}

template<typename T> class NamedTextCtrl;

class NamedTextCtrlDialog: public OptimRule, public wxDialog
{
    public:
        OptimRule *rule_holder;
        bool selection_ok;
        
        wxCheckBox *lock;
        wxChoice *operation_type,*limit_type;
        NamedTextCtrl<double> *delta,*limit_down,*limit_up;
        
        NamedTextCtrlDialog(OptimRule *rule);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void save();
};

template<typename T>
class NamedTextCtrl: public wxPanel
{
    public:
        T val;
        OptimEngine *optim_engine;
        
        wxStaticText *name_ctrl;
        wxTextCtrl *txt;
        wxButton *adv_ctrl;
                
        NamedTextCtrl(wxWindow *parent,std::string name,T const &x,bool static_style=false,int Nc=0)
            :wxPanel(parent),
             val(x),
             optim_engine(nullptr)
        {
            wxBoxSizer *sizer;
            wxStaticText *name_ctrl=nullptr;
            
            if(static_style) sizer=new wxStaticBoxSizer(wxVERTICAL,this,wxString(name));
            else sizer=new wxBoxSizer(wxHORIZONTAL);
            
            std::stringstream strm;
            strm<<x;
            
            if(!static_style) name_ctrl=new wxStaticText(this,wxID_ANY,wxString(name));
            txt=new wxTextCtrl(this,wxID_ANY,wxString(strm.str()),
                               wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
            
            if(Nc>0)
            {
                std::stringstream strm;
                for(int i=0;i<Nc;i++) strm<<"0";
                
                txt->SetInitialSize(txt->GetSizeFromTextSize(txt->GetTextExtent(strm.str())));
            }
            
            if(!static_style) sizer->Add(name_ctrl,wxSizerFlags().Align(wxCENTER));
            sizer->Add(txt,wxSizerFlags(1).Expand());
            
            adv_ctrl=new wxButton(this,wxID_ANY,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
            adv_ctrl->Hide();
            
            adv_ctrl->Bind(wxEVT_BUTTON,&NamedTextCtrl<T>::evt_advanced,this);
            
            sizer->Add(adv_ctrl,wxSizerFlags().Expand());
            
            txt->Bind(wxEVT_TEXT_ENTER,&NamedTextCtrl<T>::value_enter,this);
            txt->Bind(wxEVT_KILL_FOCUS,&NamedTextCtrl<T>::value_focus,this);
            
            SetSizer(sizer);
        }
        
        ~NamedTextCtrl()
        {
            if(optim_engine!=nullptr)
                optim_engine->forget_target(&val);
        }
        
        void evt_advanced(wxCommandEvent &event)
        {
            if(optim_engine!=nullptr)
            {
                OptimRule rule;
                
                bool known=optim_engine->get_rule(&val,rule);
                
                if(!known) return;
                
                NamedTextCtrlDialog dialog(&rule);
                
                if(dialog.selection_ok) optim_engine->set_rule(&val,rule);
            }
        }
        
        T get_value() { return val; }
                
        void lock()
        {
            txt->SetEditable(false);
        }
        
        T& operator = (T const &val_) { val=val_; return val; }
        
        void update_display()
        {
            std::stringstream strm;
            strm<<val;
            
            txt->ChangeValue(wxString(strm.str()));
        }
        
        operator T() const { return val; }
        
        void set_optimization_engine(OptimEngine *engine,OptimRule const &rule)
        {
            optim_engine=engine;
            
            optim_engine->register_target(&val,rule);
            
            adv_ctrl->Show();
            Layout();
        }
        
        void set_value(T const &x)
        {
            val=x;
            
            std::stringstream strm;
            strm<<x;
            
            txt->ChangeValue(wxString(strm.str()));
        }
        
        void value_enter(wxCommandEvent &event)
        {
            textctrl_to_value(txt,val);
            
            wxCommandEvent event_out(EVT_NAMEDTXTCTRL);
            wxPostEvent(this,event_out);
        }
        
        void value_focus(wxFocusEvent &event)
        {
            textctrl_to_value(txt,val);
            
            wxCommandEvent event_out(EVT_NAMEDTXTCTRL);
            wxPostEvent(this,event_out);
            
            event.Skip();
        }
        
        void unlock()
        {
            txt->SetEditable(true);
        }
};

class NamedSymCtrl: public wxPanel
{
    public:
        bool owned;
        SymNode *val;
        
        wxStaticText *name_ctrl;
        wxTextCtrl *txt;
        
        NamedSymCtrl(wxWindow *parent,std::string name,double const &x,bool static_style=false);
        NamedSymCtrl(wxWindow *parent,std::string name,std::string const &x,bool static_style=false);
        NamedSymCtrl(wxWindow *parent,std::string name,SymNode *val,bool static_style=false);
        
        ~NamedSymCtrl();
        
        SymNode* get_node();
        std::string get_text();
        double get_value();
        int get_value_integer();
        void hide();
        bool requires_any(std::vector<std::string> const &var);
        void set_expression(std::string const &expression);
        void set_lib(SymLib *lib);
        void set_node(SymNode *val);
        void show();
        void value_enter(wxCommandEvent &event);
        void value_focus(wxFocusEvent &event);
};

class BaseFrame: public wxFrame
{
    public:
        bool help_menu_added;
        wxFileName project_fname;
        
        BaseFrame(wxString const &title);
        virtual ~BaseFrame();
        
        #ifndef LIMULE_MODE
        void append_help_menu(wxMenuBar *menu_bar);
        void evt_help_menu(wxCommandEvent &event);
        void evt_load_project(std::string const &extension,std::string const &description);
        void evt_save_project(std::string const &extension,std::string const &description);
        void evt_save_project_as(std::string const &extension,std::string const &description);
        virtual void load_project(wxFileName const &fname);
        virtual void save_project(wxFileName const &fname);
        #endif
};

class ChoiceDialog: public wxDialog
{
    public:
        bool choice_ok;
        int choice;
        
        wxRadioBox *choice_ctrl;
        
        ChoiceDialog(wxString const &title,
                     std::vector<wxString> const &labels);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
};

class ExportChoiceDialog: public wxDialog
{
    public:
        bool choice_ok;
        int export_type;
        std::filesystem::path fname;
        
        std::vector<wxString> file_types;
        wxRadioBox *type_ctrl;
        
        ExportChoiceDialog(wxString const &title,
                           std::vector<wxString> const &labels,
                           std::vector<wxString> const &file_types);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
};

class ColorPanel: public wxPanel
{
    public:
        
        NamedTextCtrl<double> *X,*Y,*Z,*R,*G,*B,*Rn,*Gn,*Bn;
        wxButton *rgb_btn,*rgbn_btn;
        
        ColorPanel(wxWindow *parent,wxString const &name,
                   double X_,double Y_,double Z_);
                   
        void set_XYZ(double X_,double Y_,double Z_);
};

class SldCombi: public wxPanel
{
    public:
        wxSlider *sld;
        wxTextCtrl *text_val;
        double scale;
        
        SldCombi(wxWindow *parent,wxString const &name,
                 int base_val,int min_val,int max_val,double scale=1.0,bool static_sizer=false);
        ~SldCombi();
        
        double get_value();
        void set_range(int min_value,int max_value);
        void set_value(int value);
        void update_val(wxCommandEvent &event);
};

wxDECLARE_EVENT(EVT_SLIDERDISPLAY,wxCommandEvent);

class SliderDisplay: public wxPanel
{
    public:
        int N;
        double min_val,max_val,scale;
        std::string unit;
        
        wxSlider *sld;
        wxTextCtrl *text_val;
        
        SliderDisplay(wxWindow *parent,int N_val,double min_val,double max_val,double scale=1.0,std::string const &unit="",bool vertical=false);
        ~SliderDisplay();
        
        void evt_sld(wxCommandEvent &event);
        double get_value();
        void set_N(int N);
        void set_range(double min_val,double max_val);
        void set_scale(double scale);
        void set_unit(std::string const &unit);
        void set_value(double value);
        void update_display();
};

class PMLPanel: public wxPanel
{
    public:
        NamedTextCtrl<int> *N_ctrl;
        NamedTextCtrl<double> *k_max_ctrl,
                              *s_max_ctrl,
                              *a_max_ctrl;
        
        PMLPanel(wxWindow *parent,std::string const &name);
        PMLPanel(wxWindow *parent,std::string const &name,
                 int N_pml,double k_max,double s_max,double a_max);
        
        void get_parameters(int &N_pml,double &k_max,double &s_max,double &a_max);
};

//class ImgDsp: public wxPanel
//{
//    public:
//        ImgDsp(wxWindow *parent);
//        
//        void on_paint(wxPaintEvent &);
//        void on_resize(wxSizeEvent &);
//};

class MainPanel: public wxPanel
{
    public:
        MainPanel();
};

class MainFrame: public wxFrame
{
    public:
        MainFrame(std::string title,wxPoint const &pos,wxSize const &size);
        ~MainFrame();
        
        template<class T,const char *title>
        void open_frame(wxCommandEvent &event)
        {
            std::cout<<title<<std::endl;
            T *frame=new T(wxString(title));
            frame->Show(true);
            frame->Maximize();
            
            Raise();
        }
        
        void evt_about_help(wxCommandEvent &event);
        void evt_open_materials_manager(wxCommandEvent &event);
};

wxDECLARE_EVENT(EVT_FILE_SELECTOR,wxCommandEvent);
wxDECLARE_EVENT(EVT_LENGTH_SELECTOR,wxCommandEvent);
wxDECLARE_EVENT(EVT_SPECTRUM_SELECTOR,wxCommandEvent);
wxDECLARE_EVENT(EVT_WAVELENGTH_SELECTOR,wxCommandEvent);

class FileSelector: public wxPanel
{
    public:
        std::filesystem::path path;
        wxTextCtrl *path_ctrl;
        
        wxString default_path,
                 default_file,
                 default_extension,
                 default_wildcards;
        
        FileSelector(wxWindow *parent,std::string const &name);
        
        void evt_file(wxCommandEvent &event);
};

class LengthSelector: public wxPanel
{
    public:
        double L;
        
        bool optimize;
        OptimRule optim_rule;
        
        wxTextCtrl *length_ctrl;
        wxChoice *unit_ctrl;
        wxButton *extension_button;
        
        LengthSelector(wxWindow *parent,std::string name,double L,bool static_style=false,std::string const &zero_unit="m");
        
        void auto_unit();
        void evt_advanced(wxCommandEvent &event);
        double get_length();
        void handle_external_optimization(double *target,OptimEngine *engine);
        void set_length(double L);
        void set_unit(std::string const &unit);
        void unit_event(wxCommandEvent &event);
        void value_change();
        void value_enter_event(wxCommandEvent &event);
        void value_focus_event(wxFocusEvent &event);
};

class LengthSelectorDialog: public wxDialog
{
    public:
        OptimRule rule;
        bool optimize;
        bool selection_ok;
        
        wxCheckBox *optimize_ctrl,*lock;
        wxChoice *operation_type,*limit_type;
        LengthSelector *delta,*limit_down,*limit_up;
        
        LengthSelectorDialog(bool optimize, OptimRule const &rule);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_optimize(wxCommandEvent &event);
        void save();
};

class WavelengthSelector: public wxPanel
{
    public:
        double lambda;
        
        wxTextCtrl *lambda_ctrl;
        wxChoice *unit_ctrl;
        
        WavelengthSelector(wxWindow *parent,std::string name,double lambda);
        
        void change_unit(std::string const &unit);
        double get_lambda();
        void lock();
        void set_lambda(double lambda);
        void unit_event(wxCommandEvent &event);
        void unlock();
        void update_value_display();
        void value_change();
        void value_enter_event(wxCommandEvent &event);
        void value_focus_event(wxFocusEvent &event);
};

class SpectrumSelector: public wxPanel
{
    public:
        bool monochromatic;
        int Np;
        double lambda_min,lambda_max;
        
        WavelengthSelector *lambda_min_ctrl,*lambda_max_ctrl;
        wxTextCtrl *Np_ctrl;
        
        SpectrumSelector(wxWindow *parent,double lambda_min,double lambda_max,int Np,bool vertical=false);
        
        int get_closest(double lambda);
        double get_lambda(int N);
        double get_lambda_max();
        double get_lambda_min();
        int get_Np();
        void get_spectrum(std::vector<double> &lambda);
        
        void np_enter_event(wxCommandEvent &event);
        void np_focus_event(wxFocusEvent &event);
        void set_Np(int Np);
        void set_monochromatic();
        void set_polychromatic();
        void set_spectrum(double lambda_min,double lambda_max);
        void spectrum_update();
        void spectrum_event(wxCommandEvent &event);
};

bool in_family_tree(wxTreeCtrl *tree,wxTreeItemId const &parent,wxTreeItemId const&child,bool recursive=true,bool strict=false);

#endif
