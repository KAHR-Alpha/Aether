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

#include <gui_multilayers_utils.h>

wxDEFINE_EVENT(EVT_LAYER_PANEL,wxCommandEvent);

class std_dev_dialog:public wxDialog
{
    public:
        double &std_dev;
        LengthSelector *std_dev_ctrl;
        
        std_dev_dialog(double &std_dev_)
            :wxDialog(0,wxID_ANY,"Standard Deviation",wxGetApp().default_dialog_origin()),
             std_dev(std_dev_)
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
            
            std_dev_ctrl=new LengthSelector(this,"",std_dev);
            if(std_dev==0) std_dev_ctrl->set_unit("nm");
            
            wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
            wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
            
            wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
            
            buttons_sizer->Add(ok_btn);
            buttons_sizer->Add(cancel_btn);
            
            sizer->Add(std_dev_ctrl,wxSizerFlags().Expand().Border(wxALL,2));
            sizer->Add(buttons_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
            
            SetSizerAndFit(sizer);
            
            cancel_btn->Bind(wxEVT_BUTTON,&std_dev_dialog::evt_cancel,this);
            ok_btn->Bind(wxEVT_BUTTON,&std_dev_dialog::evt_ok,this);
            
            ShowModal();
        }
        
        void evt_cancel(wxCommandEvent &event)
        {
            Close();
        }
        
        void evt_ok(wxCommandEvent &event)
        {
            std_dev=std_dev_ctrl->get_length();
            Close();
        }
};

//################
//   LayerPanel
//################

LayerPanel::LayerPanel(wxWindow *parent,bool wavy)
    :LayerPanelBase(parent),
     std_dev(0)
{
    height_ctrl=new LengthSelector(this,"h: ",0);
    height_ctrl->set_unit("nm");
    
    selector=new MiniMaterialSelector(this);
    
    if(wavy)
    {
        wxButton *waviness_ctrl=new wxButton(this,wxID_ANY,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
        
        wxBoxSizer *height_sizer=new wxBoxSizer(wxHORIZONTAL);
        
        height_sizer->Add(height_ctrl,wxSizerFlags(1));
        height_sizer->Add(waviness_ctrl,wxSizerFlags().Border(wxLEFT,3).Align(wxALIGN_CENTER_VERTICAL));
        
        waviness_ctrl->Bind(wxEVT_BUTTON,&LayerPanel::evt_waviness,this);
        
        sizer->Add(height_sizer,wxSizerFlags().Expand());
    }
    else sizer->Add(height_ctrl,wxSizerFlags().Expand());
    
    sizer->Add(selector,wxSizerFlags().Expand());
}

LayerPanel::LayerPanel(wxWindow *parent,double height_,double std_dev_,GUI::Material *material,bool wavy)
    :LayerPanelBase(parent),
     std_dev(std_dev_)
{
    if(std_dev!=0) statistical=true;
    
    height_ctrl=new LengthSelector(this,"h: ",height_);
    
    selector=new MiniMaterialSelector(this,material);
    
    if(wavy)
    {
        wxButton *waviness_ctrl=new wxButton(this,wxID_ANY,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
        
        wxBoxSizer *height_sizer=new wxBoxSizer(wxHORIZONTAL);
        
        height_sizer->Add(height_ctrl,wxSizerFlags(1));
        height_sizer->Add(waviness_ctrl,wxSizerFlags().Border(wxLEFT,3).Align(wxALIGN_CENTER_VERTICAL));
        
        waviness_ctrl->Bind(wxEVT_BUTTON,&LayerPanel::evt_waviness,this);
        
        sizer->Add(height_sizer,wxSizerFlags().Expand());
    }
    else sizer->Add(height_ctrl,wxSizerFlags().Expand());
    
    sizer->Add(selector,wxSizerFlags().Expand());
}

void LayerPanel::evt_waviness(wxCommandEvent &event)
{
    std_dev_dialog dialog(std_dev);
    
    if(std_dev!=0) statistical=true;
    else statistical=false;
    
    wxCommandEvent event_out(EVT_LENGTH_SELECTOR);
    
    wxPostEvent(this,event_out);
    
    event.Skip();
}

wxString LayerPanel::get_base_name() { return "Layer"; }

double LayerPanel::get_height()
{
    if(statistical)
    {
        return std::max(0.0,randp_norm(height_ctrl->get_length(),std_dev));
    }
    else return height_ctrl->get_length();
}

void LayerPanel::get_heights(std::vector<double> &h)
{
    h.push_back(get_height());
}

Material LayerPanel::get_material() { return selector->get_material(); }

void LayerPanel::get_materials(std::vector<Material> &mats)
{
    mats.push_back(selector->get_material());
}

int LayerPanel::get_n_layers() { return 1; }

void LayerPanel::set_std_dev(double std_dev_)
{
    std_dev=std_dev_;
    if(std_dev!=0) statistical=true;
}

wxString LayerPanel::get_lua_string()
{
    wxString str;
    str<<"add_layer("<<height_ctrl->get_length()<<","
                     <<std_dev<<","
                     <<selector->get_lua()<<")";
    
    return str;
}

//################
//   BraggPanel
//################

BraggPanel::BraggPanel(wxWindow *parent,
                       double height_1_,double std_dev_1_,GUI::Material *material_1_,
                       double height_2_,double std_dev_2_,GUI::Material *material_2_,
                       double height_core_,double std_dev_core_,GUI::Material *material_core_,
                       double global_std_dev_,double g_factor_,int N_top_,int N_bottom_)
    :LayerPanelBase(parent),
     std_dev_1(std_dev_1_), std_dev_2(std_dev_2_), std_dev_core(std_dev_core_)
{
    if(std_dev_1!=0 || std_dev_2!=0 || std_dev_core!=0 || global_std_dev!=0) statistical=true;
    
    // Material 1
    
    wxStaticBoxSizer *mat_1_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Material 1");
    
    wxButton *waviness_ctrl_1=new wxButton(this,0,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    wxBoxSizer *height_sizer_1=new wxBoxSizer(wxHORIZONTAL);
    
    height_ctrl_1=new LengthSelector(this,"h: ",height_1_);
    if(height_1_==0) height_ctrl_1->set_unit("nm");
    
    height_sizer_1->Add(height_ctrl_1,wxSizerFlags(1));
    height_sizer_1->Add(waviness_ctrl_1,wxSizerFlags().Border(wxLEFT,3).Align(wxALIGN_CENTER_VERTICAL));
        
    waviness_ctrl_1->Bind(wxEVT_BUTTON,&BraggPanel::evt_waviness,this);
    
    selector_1=new MiniMaterialSelector(this,material_1_,"");
    
    mat_1_sizer->Add(height_sizer_1,wxSizerFlags().Expand());
    mat_1_sizer->Add(selector_1,wxSizerFlags().Expand());
    
    // Material 2
    
    wxStaticBoxSizer *mat_2_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Material 2");
    
    wxButton *waviness_ctrl_2=new wxButton(this,1,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    wxBoxSizer *height_sizer_2=new wxBoxSizer(wxHORIZONTAL);
    
    height_ctrl_2=new LengthSelector(this,"h: ",height_2_);
    if(height_2_==0) height_ctrl_2->set_unit("nm");
    
    height_sizer_2->Add(height_ctrl_2,wxSizerFlags(1));
    height_sizer_2->Add(waviness_ctrl_2,wxSizerFlags().Border(wxLEFT,3).Align(wxALIGN_CENTER_VERTICAL));
    
    waviness_ctrl_2->Bind(wxEVT_BUTTON,&BraggPanel::evt_waviness,this);
    
    selector_2=new MiniMaterialSelector(this,material_2_,"");
    
    mat_2_sizer->Add(height_sizer_2,wxSizerFlags().Expand());
    mat_2_sizer->Add(selector_2,wxSizerFlags().Expand());
    
    top_rep_ctrl=new NamedTextCtrl<int>(this,"Top Periods",N_top_,true);
    
    // Core
    
    wxStaticBoxSizer *core_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Core");
    
    wxButton *waviness_ctrl_core=new wxButton(this,2,"+",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    wxBoxSizer *height_sizer_core=new wxBoxSizer(wxHORIZONTAL);
    
    height_ctrl_core=new LengthSelector(this,"h: ",height_core_);
    if(height_core_==0) height_ctrl_core->set_unit("nm");
    
    height_sizer_core->Add(height_ctrl_core,wxSizerFlags(1));
    height_sizer_core->Add(waviness_ctrl_core,wxSizerFlags().Border(wxLEFT,3).Align(wxALIGN_CENTER_VERTICAL));
        
    waviness_ctrl_core->Bind(wxEVT_BUTTON,&BraggPanel::evt_waviness,this);
    
    selector_core=new MiniMaterialSelector(this,material_core_,"");
    
    core_sizer->Add(height_sizer_core,wxSizerFlags().Expand());
    core_sizer->Add(selector_core,wxSizerFlags().Expand());
    
    bottom_rep_ctrl=new NamedTextCtrl<int>(this,"Bottom Periods",N_bottom_,true);
    
    //
    
    global_std_dev=new LengthSelector(this,"Global Standard Deviation",global_std_dev_,true);
    if(global_std_dev_==0) global_std_dev->set_unit("nm");
    global_std_dev->Bind(EVT_LENGTH_SELECTOR,&BraggPanel::evt_global_std_dev,this);
    
    g_factor=new NamedTextCtrl<double>(this,"Gradient Factor",g_factor_,true);
    g_factor->Bind(EVT_NAMEDTXTCTRL,&BraggPanel::evt_g_factor,this);
    
    //
    
    sizer->Add(mat_1_sizer,wxSizerFlags().Expand());
    sizer->Add(mat_2_sizer,wxSizerFlags().Expand());
    sizer->Add(top_rep_ctrl,wxSizerFlags().Expand());
    sizer->Add(core_sizer,wxSizerFlags().Expand());
    sizer->Add(bottom_rep_ctrl,wxSizerFlags().Expand());
    sizer->Add(global_std_dev,wxSizerFlags().Expand());
    sizer->Add(g_factor,wxSizerFlags().Expand());
    
    top_rep_ctrl->Bind(EVT_NAMEDTXTCTRL,&BraggPanel::evt_periods,this);
    bottom_rep_ctrl->Bind(EVT_NAMEDTXTCTRL,&BraggPanel::evt_periods,this);
}

void BraggPanel::check_statistical()
{
    if(std_dev_1==0 &&
       std_dev_2==0 &&
       std_dev_core==0 &&
       global_std_dev->get_length()==0) statistical=false;
    else statistical=true;
}

void BraggPanel::evt_g_factor(wxCommandEvent &event)
{
    wxCommandEvent event_out(EVT_LAYER_PANEL);
    
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void BraggPanel::evt_global_std_dev(wxCommandEvent &event)
{
    check_statistical();
    
    event.Skip();
}

void BraggPanel::evt_periods(wxCommandEvent &event)
{
    int N;
    
    N=top_rep_ctrl->get_value();
    if(N<0) top_rep_ctrl->set_value(0);
    
    N=bottom_rep_ctrl->get_value();
    if(N<0) bottom_rep_ctrl->set_value(0);
    
    event.Skip();
}

void BraggPanel::evt_waviness(wxCommandEvent &event)
{
    int ID=event.GetId();
    
         if(ID==0) std_dev_dialog dialog(std_dev_1);
    else if(ID==1) std_dev_dialog dialog(std_dev_2);
    else if(ID==2) std_dev_dialog dialog(std_dev_core);
    
    check_statistical();
    
    wxCommandEvent event_out(EVT_LAYER_PANEL);
    
    wxPostEvent(this,event_out);
    
    event.Skip();
}

wxString BraggPanel::get_base_name() { return "Bragg"; }

wxString BraggPanel::get_lua_string()
{
    wxString str;
    
    str<<"add_bragg("<<height_ctrl_1->get_length()<<","<<std_dev_1<<","<<selector_1->get_lua()<<","
                     <<height_ctrl_2->get_length()<<","<<std_dev_2<<","<<selector_2->get_lua()<<","
                     <<height_ctrl_core->get_length()<<","<<std_dev_core<<","<<selector_core->get_lua()<<","
                     <<global_std_dev->get_length()<<","<<g_factor->get_value()<<","
                     <<top_rep_ctrl->get_value()<<","<<bottom_rep_ctrl->get_value()<<")";
    
    return str;
}

int BraggPanel::get_n_layers()
{
    return 1+2*(top_rep_ctrl->get_value()+bottom_rep_ctrl->get_value());
}

void BraggPanel::get_heights(std::vector<double> &h)
{
    unsigned int i;
    
    unsigned int Nt=top_rep_ctrl->get_value();
    unsigned int Nb=bottom_rep_ctrl->get_value();
    
    height_buffer.clear();
    height_buffer.reserve(1+Nt+Nb);
    
    if(statistical)
    {
        for(i=0;i<Nt;i++)
        {
            height_buffer.push_back(std::max(0.0,randp_norm(height_ctrl_1->get_length(),std_dev_1)));
            height_buffer.push_back(std::max(0.0,randp_norm(height_ctrl_2->get_length(),std_dev_2)));
        }
        
        height_buffer.push_back(std::max(0.0,randp_norm(height_ctrl_core->get_length(),std_dev_core)));
        
        for(i=0;i<Nb;i++)
        {
            height_buffer.push_back(std::max(0.0,randp_norm(height_ctrl_2->get_length(),std_dev_2)));
            height_buffer.push_back(std::max(0.0,randp_norm(height_ctrl_1->get_length(),std_dev_1)));
        }
    }
    else
    {
        for(i=0;i<Nt;i++)
        {
            height_buffer.push_back(height_ctrl_1->get_length());
            height_buffer.push_back(height_ctrl_2->get_length());
        }
        
        height_buffer.push_back(height_ctrl_core->get_length());
        
        for(i=0;i<Nb;i++)
        {
            height_buffer.push_back(height_ctrl_2->get_length());
            height_buffer.push_back(height_ctrl_1->get_length());
        }
    }
    
    double tot_size=0;
    for(i=0;i<height_buffer.size();i++) tot_size+=height_buffer[i];
        
    if(global_std_dev->get_length()!=0)
    {
        double tot_size_next=std::max(0.0,randp_norm(tot_size,global_std_dev->get_length()));
        
        double size_factor=tot_size_next/tot_size;
        
        for(i=0;i<height_buffer.size();i++) height_buffer[i]*=size_factor;
        
        tot_size=tot_size_next;
    }
    
    if(g_factor->get_value()!=0)
    {
        shift_buffer.resize(height_buffer.size());
        
        tot_size-=(height_buffer[0]+height_buffer[height_buffer.size()-1])/2.0;
        
        shift_buffer[0]=0;
        
        for(i=1;i<height_buffer.size();i++)
            shift_buffer[i]=shift_buffer[i-1]+(height_buffer[i-1]+height_buffer[i])/2.0;
        
        for(i=1;i<height_buffer.size();i++)
        {
            height_buffer[i]*=1.0+g_factor->get_value()*shift_buffer[i]/tot_size;
        }
    }
    
    for(i=0;i<height_buffer.size();i++) h.push_back(height_buffer[i]);
}

void BraggPanel::get_materials(std::vector<Material> &mats)
{
    int i,N=0;
    
    N=top_rep_ctrl->get_value();
    for(i=0;i<N;i++)
    {
        mats.push_back(selector_1->get_material());
        mats.push_back(selector_2->get_material());
    }
    
    mats.push_back(selector_core->get_material());
    
    N=bottom_rep_ctrl->get_value();
    for(i=0;i<N;i++)
    {
        mats.push_back(selector_2->get_material());
        mats.push_back(selector_1->get_material());
    }
}
