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

#include <logger.h>
#include <multilayers.h>
#include <phys_tools.h>

#include <gui_layerfitter.h>

#include <wx/splitter.h>

extern std::ofstream plog;

//########################
//   Dielec_Panel_Const
//########################

Dielec_Panel_Const::Dielec_Panel_Const(wxWindow *parent,double eps_inf_)
    :ModelPanel2(parent)
{
    set_title("Const Eps");
    
    eps_inf=new NamedTextCtrl<double>(this,"Eps: ",eps_inf_);
    
    sizer->Add(eps_inf,wxSizerFlags().Expand());
}

Imdouble Dielec_Panel_Const::get_eps(double w)
{
    return eps_inf_val;
}

std::string Dielec_Panel_Const::get_matlab()
{
    return "0*w+("+std::to_string(eps_inf->get_value())+")";
}

void Dielec_Panel_Const::set_optimization_engine_deep()
{
    OptimRule rule;
    
    rule.lock=false;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.limit_down=1.0;
    
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.operation_type=OptimRule::Operation::GROW;
        
    eps_inf->set_optimization_engine(engine,rule);
}

void Dielec_Panel_Const::update_display()
{
    eps_inf->update_display();
}

void Dielec_Panel_Const::update_model()
{
    eps_inf_val=*eps_inf;
}

//########################
//   Dielec_Panel_Drude
//########################

Dielec_Panel_Drude::Dielec_Panel_Drude(wxWindow *parent,double wd_,double g_)
    :ModelPanel2(parent)
{
    set_title("Drude");
    
    wd=new NamedTextCtrl<double>(this,"Wd: ",wd_);
    g=new NamedTextCtrl<double>(this,"g: ",g_);
    
    update_model();
    
    sizer->Add(wd,wxSizerFlags().Expand());
    sizer->Add(g,wxSizerFlags().Expand());
}

Imdouble Dielec_Panel_Drude::get_eps(double w)
{
    return model.eval(w);
}

std::string Dielec_Panel_Drude::get_matlab()
{
    return "(-(" + std::to_string(wd->get_value()) + ")^2./(w.^2+i*(" +std::to_string(g->get_value()) + ")*w))";
}

void Dielec_Panel_Drude::set_optimization_engine_deep()
{
    OptimRule rule;
    
    rule.lock=false;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.limit_down=0.0;
    
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.operation_type=OptimRule::Operation::GROW;
        
    wd->set_optimization_engine(engine,rule);
    g->set_optimization_engine(engine,rule);
}

void Dielec_Panel_Drude::update_display()
{
    wd->update_display();
    g->update_display();
}

void Dielec_Panel_Drude::update_model()
{
    model.set(*wd,*g);
}

//##########################
//   Dielec_Panel_Lorentz
//##########################

Dielec_Panel_Lorentz::Dielec_Panel_Lorentz(wxWindow *parent,double A_,double O_,double G_)
    :ModelPanel2(parent)
{
    set_title("Lorentz");
    
    A=new NamedTextCtrl<double>(this,"A: ",A_);
    O=new NamedTextCtrl<double>(this,"O: ",O_);
    G=new NamedTextCtrl<double>(this,"G: ",G_);
    
    update_model();
    
    sizer->Add(A,wxSizerFlags().Expand());
    sizer->Add(O,wxSizerFlags().Expand());
    sizer->Add(G,wxSizerFlags().Expand());
}

std::string Dielec_Panel_Lorentz::get_matlab()
{
    return "(" + std::to_string(A->get_value()) + ")*(" + std::to_string(O->get_value()) + ")^2./(("+ std::to_string(O->get_value()) +")^2-w.^2-i*w*(" + std::to_string(G->get_value()) +"))";
}

Imdouble Dielec_Panel_Lorentz::get_eps(double w)
{
    return model.eval(w);
}

void Dielec_Panel_Lorentz::set_optimization_engine_deep()
{
    OptimRule rule;
    
    rule.lock=false;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.limit_down=0.0;
    
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.operation_type=OptimRule::Operation::GROW;
        
    A->set_optimization_engine(engine,rule);
    O->set_optimization_engine(engine,rule);
    G->set_optimization_engine(engine,rule);
}

void Dielec_Panel_Lorentz::update_display()
{
    A->update_display();
    O->update_display();
    G->update_display();
}

void Dielec_Panel_Lorentz::update_model()
{
    model.set(*A,*O,*G);
}

//############################
//   Dielec_Panel_CritPoint
//############################

Dielec_Panel_CritPoint::Dielec_Panel_CritPoint(wxWindow *parent,double A_,double O_,double G_,double P_)
    :ModelPanel2(parent)
{
    set_title("Critical Points");
    
    A=new NamedTextCtrl<double>(this,"A: ",A_);
    O=new NamedTextCtrl<double>(this,"O: ",O_);
    G=new NamedTextCtrl<double>(this,"G: ",G_);
    P=new NamedTextCtrl<double>(this,"P: ",P_);
    
    update_model();
    
    sizer->Add(A,wxSizerFlags().Expand());
    sizer->Add(O,wxSizerFlags().Expand());
    sizer->Add(G,wxSizerFlags().Expand());
    sizer->Add(P,wxSizerFlags().Expand());
}

Imdouble Dielec_Panel_CritPoint::get_eps(double w)
{
    return model.eval(w);
}

void Dielec_Panel_CritPoint::set_optimization_engine_deep()
{
    OptimRule rule;
    
    rule.lock=false;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.limit_down=0.0;
    
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.operation_type=OptimRule::Operation::GROW;
        
    A->set_optimization_engine(engine,rule);
    O->set_optimization_engine(engine,rule);
    G->set_optimization_engine(engine,rule);
    
    rule.limit_type=OptimRule::Limit::NONE;
    rule.operation_type=OptimRule::Operation::ADD;
    
    P->set_optimization_engine(engine,rule);
}

void Dielec_Panel_CritPoint::update_display()
{
    A->update_display();
    O->update_display();
    G->update_display();
    P->update_display();
}

void Dielec_Panel_CritPoint::update_model()
{
    model.set(*A,*O,*P,*G);
}

//#################
//   LayerFitter
//#################

LayerFitter::LayerFitter(wxString const &title)
    :BaseFrame(title),
     optimizing(false)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    graph=new Graph(splitter);
    
    wxSizerFlags flag_expand; flag_expand.Expand();
    
    wxBoxSizer *controls_sizer=new wxBoxSizer(wxVERTICAL);
    ctrl_panel->SetSizer(controls_sizer);
    
    // Data
    
    wxStaticBoxSizer *data_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Data");
    wxBoxSizer *data_header=new wxBoxSizer(wxHORIZONTAL);
    
    data_load=new wxButton(ctrl_panel,wxID_ANY,"Load data");
    data_type=new wxChoice(ctrl_panel,wxID_ANY);
    data_type->Append("Transmission");
    data_type->Append("Reflection");
    data_type->SetSelection(0);
    
    data_header->Add(data_load);
    data_header->Add(data_type);
    
    data_fname_disp=new wxTextCtrl(ctrl_panel,wxID_ANY);
    
    data_load->Bind(wxEVT_BUTTON,&LayerFitter::get_data_fname,this);
    data_fname_disp->SetEditable(false);
    
    data_sizer->Add(data_header,flag_expand);
    data_sizer->Add(data_fname_disp,flag_expand);
    
    controls_sizer->Add(data_sizer,flag_expand);
    
    // Superstrate
    
    wxStaticBoxSizer *sup_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Surrounding medium");
    sup_sizer->GetStaticBox()->SetBackgroundColour(wxColour(210,210,255));
    
    wxButton *env_btn=new wxButton(ctrl_panel,wxID_ANY,"Add Dielectric Model");
    env_mats=new PanelsList<ModelPanel2>(ctrl_panel);
    
    env_btn->Bind(wxEVT_BUTTON,&LayerFitter::evt_add_dielec_environment,this);

    sup_sizer->Add(env_btn,wxSizerFlags().Expand().Border(wxALL,2));
    sup_sizer->Add(env_mats,wxSizerFlags().Expand().Border(wxALL,2));

    controls_sizer->Add(sup_sizer,wxSizerFlags().Expand());

    // Substrate

    wxStaticBoxSizer *sub_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Substrate");
    sub_sizer->GetStaticBox()->SetBackgroundColour(wxColour(255,255,255));
    
    OptimRule rule;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.operation_type=OptimRule::Operation::GROW;
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.limit_down=0;
    
    sub_th_ctrl=new NamedTextCtrl<double>(ctrl_panel,"h (mm):",0);
    sub_th_ctrl->set_optimization_engine(&optim_engine,rule);
    
    sub_scatt_ctrl=new NamedTextCtrl<double>(ctrl_panel,"RScatt:",0);
    sub_scatt_ctrl->set_optimization_engine(&optim_engine,rule);

    wxButton *sub_btn=new wxButton(ctrl_panel,wxID_ANY,"Add Dielectric Model");
    sub_mats=new PanelsList<ModelPanel2>(ctrl_panel);

    sub_btn->Bind(wxEVT_BUTTON,&LayerFitter::evt_add_dielec_substrate,this);

    sub_sizer->Add(sub_th_ctrl,flag_expand);
    sub_sizer->Add(sub_scatt_ctrl,flag_expand);
    sub_sizer->Add(sub_btn,wxSizerFlags().Expand().Border(wxALL,2));
    sub_sizer->Add(sub_mats,wxSizerFlags().Expand().Border(wxALL,2));

    controls_sizer->Add(sub_sizer,wxSizerFlags().Expand());

    // Layer

    wxStaticBoxSizer *layer_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Layer");
    layer_sizer->GetStaticBox()->SetBackgroundColour(wxColour(255,210,210));

    thickness_ctrl=new NamedTextCtrl<double>(layer_sizer->GetStaticBox(),"h (nm):",0);
    thickness_ctrl->set_optimization_engine(&optim_engine,rule);

    wxButton *layer_btn=new wxButton(layer_sizer->GetStaticBox(),wxID_ANY,"Add Dielectric Model");
    layer_mats=new PanelsList<ModelPanel2>(layer_sizer->GetStaticBox());

    layer_btn->Bind(wxEVT_BUTTON,&LayerFitter::evt_add_dielec_layer,this);

    layer_sizer->Add(thickness_ctrl,flag_expand);
    layer_sizer->Add(layer_btn,wxSizerFlags().Expand().Border(wxALL,2));
    layer_sizer->Add(layer_mats,wxSizerFlags().Expand().Border(wxALL,2));

    controls_sizer->Add(layer_sizer,wxSizerFlags().Expand());

    // Optimization

    timer=new wxTimer(this);
    Bind(wxEVT_TIMER,&LayerFitter::evt_timed_refresh,this);

    wxStaticBoxSizer *optim_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Optimization");

    wxBoxSizer *optim_limits_sizer=new wxBoxSizer(wxHORIZONTAL);

    x_min=new NamedTextCtrl<double>(ctrl_panel,"X Min: ",0);
    x_max=new NamedTextCtrl<double>(ctrl_panel," X Max: ",0);

    optim_limits_sizer->Add(x_min,wxSizerFlags(1));
    optim_limits_sizer->Add(x_max,wxSizerFlags(1));

    wxToggleButton *optim_btn=new wxToggleButton(ctrl_panel,wxID_ANY,"Optimize");
    optim_btn->SetValue(false);

    optim_btn->Bind(wxEVT_TOGGLEBUTTON,&LayerFitter::evt_optimize,this);

    optim_sizer->Add(optim_limits_sizer,wxSizerFlags().Expand());
    optim_sizer->Add(optim_btn,wxSizerFlags().Expand());

    controls_sizer->Add(optim_sizer,wxSizerFlags().Expand());
    
    // Export
    
    wxButton *export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    export_btn->Bind(wxEVT_BUTTON,&LayerFitter::evt_export,this);
    
    controls_sizer->Add(export_btn,wxSizerFlags().Expand());
    
    //Graphs
    
    Bind(EVT_NAMEDTXTCTRL,&LayerFitter::evt_update,this);
    Bind(EVT_PLIST_RESIZE,&LayerFitter::evt_update_ui,this);
    Bind(EVT_PLIST_REMOVE,&LayerFitter::evt_update_ui,this);
    
    ctrl_panel->SetScrollRate(50,50);
    
    splitter->SplitVertically(ctrl_panel,graph,350);
    splitter->SetMinimumPaneSize(250);
    
    ctrl_panel->FitInside();
}

LayerFitter::~LayerFitter()
{
    timer->Stop();
}

void LayerFitter::add_dielec_element(PanelsList<ModelPanel2> *panel)
{
    Dielec_dialog dialog;
    
    int selection=dialog.selection;
    
    ModelPanel2 *model_panel=nullptr;
    
    if(selection!=-1)
    {
             if(selection==0) model_panel=panel->add_panel<Dielec_Panel_Drude>(1e15,1e14);
        else if(selection==1) model_panel=panel->add_panel<Dielec_Panel_Lorentz>(1.0,1e15,1e14);
        else if(selection==2) model_panel=panel->add_panel<Dielec_Panel_CritPoint>(1.0,1e15,1e14,0);
    }
    
    if(model_panel==nullptr) return;
    
    model_panel->set_optimization_engine(&optim_engine);
    
    ctrl_panel->FitInside();
}

void LayerFitter::compute_response()
{
    for(unsigned int i=0;i<layer_eps.size();i++)
    {
        env_eps[i]=0;
        layer_eps[i]=0;
        sub_eps[i]=0;
    }
    
    for(unsigned int i=0;i<env_mats->get_size();i++)
        env_mats->get_panel(i)->add_eps(env_eps,r_freq);
    
    for(unsigned int i=0;i<layer_mats->get_size();i++)
        layer_mats->get_panel(i)->add_eps(layer_eps,r_freq);
    
    for(unsigned int i=0;i<sub_mats->get_size();i++)
        sub_mats->get_panel(i)->add_eps(sub_eps,r_freq);
    
    Imdouble n_sup,n_sub;

    double h=thickness_ctrl->get_value()*1e-9;
    double h_sub=sub_th_ctrl->get_value()*1e-3;
    double sub_scatt=sub_scatt_ctrl->get_value();

    double R_TE,R_TM;
    double T_TE,T_TM;
    double A_TE,A_TM;
    
    Multilayer_TMM lm(1);
    
    for(int l=0;l<N_data;l++)
    {
        n_sup=std::sqrt(env_eps[l]);
        n_sub=std::sqrt(sub_eps[l]);
        
        lm.set_environment(n_sup,n_sub);
        lm.set_lambda(lambda[l]);
        lm.set_layer(0,h,std::sqrt(layer_eps[l]));
        
        lm.compute_power(R_TE,T_TE,A_TE,R_TM,T_TM,A_TM);
        
//        double R_12=R_TE;
        double T_12=T_TE;
        
        lm.set_environment(n_sub,n_sup);
        lm.compute_power(R_TE,T_TE,A_TE,R_TM,T_TM,A_TM);
        
        double R_21=R_TE;
//        double T_21=T_TE;
        
        double R_23=std::norm((n_sub-n_sup)/(n_sub+n_sup));
        double T_23=1.0-R_23;
        
//        double P=std::exp(-4.0*Pi*h_sub/lambda[l]*n_sub.imag());
        double P=std::exp(-4.0*Pi*h_sub/lambda[l]*n_sub.imag()-h_sub*sub_scatt/std::pow(lambda[l],4));

        data_comp[l]=-T_12*P*T_23/(R_21*P*P*R_23-1.0);
//        data_comp[l]=R_21-P*P*T_12*R_23*T_12/(R_21*P*P*R_23-1.0);
            
//            if(l==N_data-1)
//            Plog::print(n_sub<<" "<<exp_decay<<" "<<sec_ref<<" "
//                     <<(1.0-std::real(sec_ref*std::conj(sec_ref)))<<" "
//                     <<t_base<<" "
//                     <<t_TE<<" "
//                     <<data_[l]<<std::endl;
//        }
//        else // Reflection
//        {
//            data_comp[l]=std::real(r_TE*std::conj(r_TE));
//        }
    }

//    graph->Refresh();
}

void LayerFitter::evt_add_dielec_environment(wxCommandEvent &event)
{
    add_dielec_element(env_mats);
    event.Skip();
}

void LayerFitter::evt_add_dielec_layer(wxCommandEvent &event)
{
    add_dielec_element(layer_mats);
    event.Skip();
}

void LayerFitter::evt_add_dielec_substrate(wxCommandEvent &event)
{
    add_dielec_element(sub_mats);
    event.Skip();
}

void LayerFitter::evt_export(wxCommandEvent &event)
{
    ExportChoiceDialog dialog("Please choose a format to export to",
                              {"MatLab", "ASCII"},
                              {"MatLab script (*.m)|*.m", "ASCII file (*.txt)|*.txt"});

    if(!dialog.choice_ok) return;

    if(dialog.export_type == 0)
    {
        export_as_matlab(dialog.fname);
    }
    else
    {
        export_as_ascii(dialog.fname);
    }
}

void LayerFitter::evt_update(wxCommandEvent &event)
{
    compute_response();
    graph->Refresh();
    
    event.Skip();
}

void LayerFitter::evt_update_ui(wxCommandEvent &event)
{
    ctrl_panel->FitInside();
    Layout();
    
    evt_update(event);
}


void LayerFitter::export_as_ascii(std::filesystem::path const &fname)
{
    compute_response();

    std::ofstream file(fname, std::ios::out|std::ios::trunc);

    if(!file.is_open())
    {
        Plog::print(LogType::FATAL, "Couldn't write to ", fname.generic_string());
        return;
    }

    for(std::size_t i=0; i<lambda.size(); i++)
    {
        Imdouble n_env = std::sqrt(env_eps[i]);
        Imdouble n_layer = std::sqrt(layer_eps[i]);
        Imdouble n_sub = std::sqrt(sub_eps[i]);

        file<<lambda[i]<<" ";
        file<<n_env.real()<<" "<<n_env.imag()<<" ";
        file<<n_layer.real()<<" "<<n_layer.imag()<<" ";
        file<<n_sub.real()<<" "<<n_sub.imag();

        if(i < lambda.size()-1) file<<"\n";
    }
}


void LayerFitter::export_as_matlab(std::filesystem::path const &fname_) const
{
    wxString fname = fname_.generic_string();
    
    if(fname.size()==0) return;
    
    wxFile file(fname,wxFile::write);
    
    wxFileName descr(fname);
    
    wxString strm;
    
    strm<<"function out="<<descr.GetName()<<"()\n\n";
    strm<<"lambda=[";
    for(int i=0;i<N_data;i++)
    {
        strm<<lambda[i];
        
        if(i==N_data-1) strm<<"];\n";
        else strm<<",";
    }
    strm<<"w=2*pi*299792458./lambda;\n\n";
    strm<<"data=[";
    for(int i=0;i<N_data;i++)
    {
        strm<<data[i];
        
        if(i==N_data-1) strm<<"];\n";
        else strm<<",";
    }
    strm<<"data_comp=[";
    for(int i=0;i<N_data;i++)
    {
        strm<<data_comp[i];
        
        if(i==N_data-1) strm<<"];\n\n";
        else strm<<",";
    }
    strm<<"eps_env=";
    for(unsigned int i=0;i<env_mats->get_size();i++)
    {
        strm<<env_mats->get_panel(i)->get_matlab();
        
        if(i==env_mats->get_size()-1) strm<<";\n";
        else strm<<"+";
    }
    strm<<"eps_layer=";
    for(unsigned int i=0;i<layer_mats->get_size();i++)
    {
        strm<<layer_mats->get_panel(i)->get_matlab();
        
        if(i==layer_mats->get_size()-1) strm<<";\n";
        else strm<<"+";
    }
    strm<<"eps_sub=";
    for(unsigned int i=0;i<sub_mats->get_size();i++)
    {
        strm<<sub_mats->get_panel(i)->get_matlab();
        
        if(i==sub_mats->get_size()-1) strm<<";\n\n";
        else strm<<"+";
    }
    strm<<"n_env=sqrt(eps_env);\n";
    strm<<"n_layer=sqrt(eps_layer);\n";
    strm<<"n_sub=sqrt(eps_sub);\n\n";
    strm<<"figure(1)\n\n";
    strm<<"plot(lambda,real(n_env),lambda,imag(n_env))\n";
    strm<<"xlabel('Lambda (m)')\n";
    strm<<"ylabel('n environment')\n";
    strm<<"legend('real','imag')\n";
    strm<<"figure(2)\n";
    strm<<"plot(lambda,real(n_layer),lambda,imag(n_layer))\n";
    strm<<"xlabel('Lambda (m)')\n";
    strm<<"ylabel('n layer')\n";
    strm<<"legend('real','imag')\n";
    strm<<"figure(3)\n";
    strm<<"plot(lambda,real(n_sub),lambda,imag(n_sub))\n";
    strm<<"xlabel('Lambda (m)')\n";
    strm<<"ylabel('n substrate')\n";
    strm<<"legend('real','imag')\n\n";
    strm<<"figure(4)\n\n";
    strm<<"plot(lambda,data,lambda,data_comp)\n";
    strm<<"xlabel('Lambda (m)')\n";
    strm<<"ylabel('T')\n";
    strm<<"legend('data','fit')\n\n";
    strm<<"out.data=data;\n";
    strm<<"out.data_comp=data_comp;\n";
    strm<<"out.n_env=n_env;\n";
    strm<<"out.n_layer=n_layer;\n";
    strm<<"out.n_sub=n_sub;\n\n";
    
    strm<<"end\n";
    
    
    file.Write(strm);
}


void LayerFitter::get_data_fname(wxCommandEvent &event)
{
    int l;
    
    graph->clear_graph();
    
    env_mats->clear();
    layer_mats->clear();
    sub_mats->clear();
    
    std::string data_tmp;
    data_tmp=wxFileSelector("Select the data file");
    if(data_tmp.size()==0) return;
    
    data_fname=data_tmp;
    
    data_fname_disp->SetValue(data_fname);
    
    N_data=fcountlines(data_fname);
    
    lambda.resize(N_data);
    r_freq.resize(N_data);
    data.resize(N_data);
    data_comp.resize(N_data);
    
    env_eps.resize(N_data);
    layer_eps.resize(N_data);
    sub_eps.resize(N_data);
    buffer_eps.resize(N_data);
    
    std::ifstream file(data_fname,std::ios::in);
    
    for(l=0;l<N_data;l++)
    {
        file>>lambda[l];
        file>>data[l];
        
        r_freq[l]=m_to_rad_Hz(lambda[l]);
    }
    
    file.close();
    
    lambda_min=std::min(lambda[0],lambda[N_data-1]);
    lambda_max=std::max(lambda[0],lambda[N_data-1]);
    
    for(l=0;l<N_data;l++) data_comp[l]=0;
    
    graph->add_external_data(&lambda,&data);
    
    x_min->set_value(lambda_min);
    x_max->set_value(lambda_max);
    
    ModelPanel2 *base_env=env_mats->add_panel<Dielec_Panel_Const>(1.0);
    ModelPanel2 *base_layer=layer_mats->add_panel<Dielec_Panel_Const>(1.0);
    ModelPanel2 *base_sub=sub_mats->add_panel<Dielec_Panel_Const>(1.0);
    
    base_env->set_optimization_engine(&optim_engine);
    base_layer->set_optimization_engine(&optim_engine);
    base_sub->set_optimization_engine(&optim_engine);
    
    ctrl_panel->FitInside();
    Layout();
    
    compute_response();
    
    graph->add_external_data(&lambda,&data_comp,1.0,0,0);
    graph->autoscale();
    graph->Refresh();
}

void LayerFitter::evt_optimize(wxCommandEvent &event)
{
    if(event.IsChecked())
    {
        optimizing=true;
        optim_thread=new std::thread(&LayerFitter::threaded_optimization,this);

        timer->Start(500);
    }
    else
    {
        optimizing=false;
        chk_var("joining");
        optim_thread->join();
        chk_var("joined");

        delete optim_thread;

        timer->Stop();
        
        sub_th_ctrl->update_display();
        thickness_ctrl->update_display();
        
        for(unsigned int i=0;i<env_mats->get_size();i++)
            env_mats->get_panel(i)->update_display();
        
        for(unsigned int i=0;i<layer_mats->get_size();i++)
            layer_mats->get_panel(i)->update_display();
        
        for(unsigned int i=0;i<sub_mats->get_size();i++)
            sub_mats->get_panel(i)->update_display();

        compute_response();

        graph->Refresh();

        chk_var("end");
    }
}

void LayerFitter::evt_timed_refresh(wxTimerEvent &event)
{
    sub_th_ctrl->update_display();
    sub_scatt_ctrl->update_display();
    thickness_ctrl->update_display();
        
    for(unsigned int i=0;i<env_mats->get_size();i++)
        env_mats->get_panel(i)->update_display();
    
    for(unsigned int i=0;i<layer_mats->get_size();i++)
        layer_mats->get_panel(i)->update_display();
    
    for(unsigned int i=0;i<sub_mats->get_size();i++)
        sub_mats->get_panel(i)->update_display();

    compute_response();

    graph->Refresh();
}

void LayerFitter::threaded_optimization()
{
    chk_var("Optim");
    int i;
    
    double x_min_d=x_min->get_value();
    double x_max_d=x_max->get_value();
    
    compute_response();
    
    double best_score=0;
    
    for(i=0;i<N_data;i++)
    {
        if(x_min_d<=lambda[i] && x_max_d>=lambda[i])
        {
            double diff=data_comp[i]-data[i];

            best_score+=diff*diff;
        }
//        plog<<i<<" "<<limited_data[i]<<" "<<data[i]<<" "<<best_score<<" "<<x_min_d<<" "<<x_max_d<<std::endl;
    }

    int k=0;
    int N_fail=100;
    double factor=1.0;
    
    while(optimizing)
    {
        Plog::print(k, " ", best_score, "\n");
        k++;

        optim_engine.evolve(factor);
        
        compute_response();
        
        double tmp_score=0;

        for(i=0;i<N_data;i++)
        {
            if(x_min_d<=lambda[i] && x_max_d>=lambda[i])
            {
                double diff=data_comp[i]-data[i];
                
                tmp_score+=diff*diff;
            }
        }
        
////        chk_var("E");
        if(tmp_score>best_score)
        {
            N_fail++;
            optim_engine.revert_variables();
            
            if(N_fail>100) factor*=0.9;
        }
        else
        {
            N_fail=0;
            factor*=1.1;
            
            best_score=tmp_score;
        }
////        chk_var("F");
//
    }
    
    chk_var("o end");
}
