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

#include <aether.h>
#include <gui_fitter.h>
#include <gui_fitter_functions.h>

//#######################
//   FitterModelDialog
//#######################

FitterModelDialog::FitterModelDialog()
    :wxDialog(NULL,-1,"Select a model",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     choice_ok(false), choice_type(F_CONST),
     Np(101), x_data(Np), y_data(Np), params(15)
{
    int i;
    
    for(i=0;i<Np;i++)
    {
        x_data[i]=-4.0+8.0*i/(Np-1.0);
        y_data[i]=1.0;
    }
    
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxString type_choices[]={"Constant",
                             "Exponential",
                             "Gaussian",
                             "Inverse",
                             "Logistic",
                             "Lorenztian",
                             "Lorentzian - generalized",
                             "Planck - frequency-like",
                             "Planck - wavelength-like",
                             "Sine"};
    wxStaticText *type_txt=new wxStaticText(this,wxID_ANY,"Model: ");
    model_choice=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,10,type_choices);
    model_choice->SetSelection(0);
    
    type_sizer->Add(type_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    type_sizer->Add(model_choice);
    
    top_sizer->Add(type_sizer,wxSizerFlags().Border(wxALL,2));
    
    graph=new Graph(this);
    graph->add_data(&x_data,&y_data,0,0,1.0);
    graph->set_scale(-5.0,5.0,0.0,2.0);
    
    top_sizer->Add(graph,wxSizerFlags(1).Expand());
    
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Border(wxALL,2).Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&FitterModelDialog::evt_cancel,this);
    model_choice->Bind(wxEVT_CHOICE,&FitterModelDialog::evt_choice,this);
    ok_btn->Bind(wxEVT_BUTTON,&FitterModelDialog::evt_ok,this);
    
    SetSizer(top_sizer);
    ShowModal();
}

void FitterModelDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void FitterModelDialog::evt_choice(wxCommandEvent &event)
{
    int i;
    int n=model_choice->GetSelection();
    
    for(i=0;i<Np;i++) x_data[i]=-4.0+8.0*i/(Np-1.0);
    
         if(n==0)
    {
        choice_type=F_CONST;
        
        params[0]=1;
        f_const(x_data,params,y_data);
    }
    else if(n==1)
    {
        choice_type=F_EXP;
        
        params[0]=1;
        params[1]=-0.5;
        f_exp(x_data,params,y_data);
    }
    else if(n==2)
    {
        choice_type=F_GAUSSIAN;
        
        params[0]=1;
        params[1]=0.5;
        f_gaussian(x_data,params,y_data);
    }
    else if(n==3)
    {
        choice_type=F_INV_POW;
        
        for(i=0;i<Np;i++) x_data[i]=1.0+2.0*i/(Np-1.0);
        params[0]=1.0;
        params[1]=4.5;
        f_inv_pow(x_data,params,y_data);
    }
    else if(n==4)
    {
        choice_type=F_LOGISTIC;
        
        params[0]=1.0;
        params[1]=1.0;
        params[2]=0.0;
        f_logistic(x_data,params,y_data);
    }
    else if(n==5)
    {
        choice_type=F_LORENTZIAN;
        
        params[0]=1.0;
        params[1]=1.0;
        params[2]=0.0;
        f_lorentz(x_data,params,y_data);
    }
    else if(n==6)
    {
        choice_type=F_LORENTZIAN_GEN;
        
        params[0]=1.0;
        params[1]=1.0;
        params[2]=0.0;
        params[3]=3.0;
        f_lorentz_gen(x_data,params,y_data);
    }
    else if(n==7)
    {
        choice_type=F_PLANCK_FREQ;
        
        for(i=0;i<Np;i++) x_data[i]=0.1+10.9*i/(Np-1.0);
        params[0]=1.0;
        params[1]=1.0;
        f_planck_freq(x_data,params,y_data);
    }
    else if(n==8)
    {
        choice_type=F_PLANCK_WVL;
        
        for(i=0;i<Np;i++) x_data[i]=0.1+0.9*i/(Np-1.0);
        params[0]=1.0;
        params[1]=1.0;
        f_planck_wvl(x_data,params,y_data);
    }
    else if(n==9)
    {
        choice_type=F_SINE;
        
        for(i=0;i<Np;i++) y_data[i]=std::sin(x_data[i]);
    }
    
    if(n==0)
    {
        graph->set_scale(-5.0,5.0,0.0,2.0);
        graph->Refresh();
    }
    else graph->autoscale();
    
    event.Skip();
}

void FitterModelDialog::evt_ok(wxCommandEvent &event)
{
    choice_ok=true;
    Close();
}

//##################
//   FitterHandle
//##################

FitterHandle::FitterHandle()
    :x(0), y(0), model(nullptr)
{
}

FitterHandle::FitterHandle(FitterHandle const &handle)
    :x(handle.x),
     y(handle.y),
     model(handle.model)
{
}

void FitterHandle::operator = (FitterHandle const &handle)
{
    x=handle.x;
    y=handle.y;
    model=handle.model;
}

void FitterHandle::set_position(double x_,double y_)
{
    x=x_;
    y=y_;
    
    model->recompute_from_handles();
}

void FitterHandle::set_position_strict(double x_,double y_)
{
    x=x_;
    y=y_;
}

//#################
//   GraphFitter
//#################

wxDEFINE_EVENT(EVT_GRAPH_CROP_VALUE,wxCommandEvent);

GraphFitter::GraphFitter(wxWindow *parent)
    :Graph(parent),
     show_cropping(false),
     target_crop(0),
     target_handle(nullptr),
     target_group(nullptr)
{
    cropping_pen.SetColour(wxColour(255,0,0,180));
    cropping_brush.SetColour(wxColour(255,0,0,120));
}

GraphFitter::~GraphFitter()
{
}

void GraphFitter::add_handles(std::vector<FitterHandle> &handles_)
{
    unsigned int i;
    std::vector<FitterHandle*> p_handles;
    
    p_handles.resize(handles_.size());
    
    for(i=0;i<handles_.size();i++) p_handles[i]=&handles_[i];
    
    handles.push_back(p_handles);
    show_handles_v.push_back(true);
    handles_pens.push_back(wxPen(*wxBLACK));
}

void GraphFitter::autoscale()
{
    if(x_data.size()<=0) return;
    if(x_data[0]->size()<=0) return;
    
    xmin=xmax=(*x_data[0])[0];
    ymin=ymax=(*y_data[0])[0];
    
    if(std::isnan(xmin) || std::isinf(xmin))
    {
        xmin=-4;
        xmax=4;
    }
    
    if(std::isnan(ymin) || std::isinf(ymin))
    {
        ymin=-4;
        ymax=4;
    }
    
    for(unsigned int i=0;i<x_data[0]->size();i++)
    {
        double tmp_x=(*x_data[0])[i];
        double tmp_y=(*y_data[0])[i];
        
        if(!std::isinf(tmp_x))
        {
            xmin=std::min(xmin,tmp_x);
            xmax=std::max(xmax,tmp_x);
        }
        
        if(!std::isinf(tmp_y))
        {
            ymin=std::min(ymin,tmp_y);
            ymax=std::max(ymax,tmp_y);
        }
    }
    
    double spanx=xmax-xmin;
    double spany=ymax-ymin;
    
    xmin-=0.05*spanx; xmax+=0.05*spanx;
    ymin-=0.05*spany; ymax+=0.05*spany;
    
    Refresh();
}

void GraphFitter::clear_graph()
{
    handles.clear();
    handles_pens.clear();
    show_handles_v.clear();
    
    Graph::clear_graph();
}

bool GraphFitter::evt_mouse_left_down_spec(wxMouseEvent &event)
{
    unsigned int i,j;
    
    wxPoint loc=event.GetPosition();
    
    int px,py;
    
    px=d2px(x_min_crop);
    
    if(px-10<=loc.x && loc.x<=px+10)
    {
        target_crop=1;
        return true;
    }
    
    px=d2px(x_max_crop);
    
    if(px-10<=loc.x && loc.x<=px+10)
    {
        target_crop=2;
        return true;
    }
    
    for(i=0;i<handles.size();i++)
    {
        if(show_handles_v[i])
        {
            for(j=0;j<handles[i].size();j++)
            {
                px=d2px(handles[i][j]->x);
                py=d2py(handles[i][j]->y);
                
                if(px-10<=loc.x && loc.x<=px+10 &&
                   py-10<=loc.y && loc.y<=py+10)
                {
                    target_handle=handles[i][j];
                    target_group=&handles[i];
                    
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool GraphFitter::evt_mouse_left_up_spec(wxMouseEvent &event)
{
    target_handle=nullptr;
    target_group=nullptr;
    target_crop=0;
    
    return false;
}

bool GraphFitter::evt_mouse_motion_spec(wxMouseEvent &event)
{
    if(target_crop==1)
    {
        x_min_crop=px2dx(mouse_x);
        wxCommandEvent event_out(EVT_GRAPH_CROP_VALUE);
        wxPostEvent(this,event_out);
        return true;
    }
    else if(target_crop==2)
    {
        x_max_crop=px2dx(mouse_x);
        wxCommandEvent event_out(EVT_GRAPH_CROP_VALUE);
        wxPostEvent(this,event_out);
        return true;
    }
    else if(target_handle!=nullptr &&
       event.Dragging() &&
       event.LeftIsDown())
    {
        target_handle->set_position(px2dx(mouse_x),py2dy(mouse_y));
        return true;
    }
    else return false;
}

void GraphFitter::forget_handles(std::vector<FitterHandle> &handles_)
{
    unsigned int i,j;
    
    int pos=-1;
    
    for(i=0;i<handles.size();i++)
    {
        bool handles_check=true;
        
        if(handles_.size()==handles[i].size())
        {
            for(j=0;j<handles[i].size();j++)
                if(handles[i][j]!=&handles_[j]) handles_check=false;
        }
        else handles_check=false;
        
        if(handles_check)
        {
            pos=i;
            break;
        }
    }
    
    if(pos>=0)
    {
        std::vector<std::vector<FitterHandle*>>::const_iterator it=handles.begin()+pos;
        std::vector<wxPen>::const_iterator itp=handles_pens.begin()+pos;
        std::vector<bool>::const_iterator its=show_handles_v.begin()+pos;
        
        handles.erase(it);
        handles_pens.erase(itp);
        show_handles_v.erase(its);
    }
}

void GraphFitter::hide_cropping()
{
    show_cropping=false;
}

void GraphFitter::postplot(wxGraphicsContext *gc)
{
    unsigned int i,j;
    
    for(i=0;i<handles.size();i++)
    {
        if(show_handles_v[i])
        {
            gc->SetPen(handles_pens[i]);
            
            for(j=0;j<handles[i].size();j++)
            {
                int px=d2px(handles[i][j]->x);
                int py=d2py(handles[i][j]->y);
                    
                gc->StrokeLine(px-10,py,px+10,py);
                gc->StrokeLine(px,py-10,px,py+10);
                
                gc->DrawEllipse(px-6,py-6,12,12);
            }
        }
    }
    
    if(show_cropping)
    {
        gc->SetPen(cropping_pen);
        gc->SetBrush(cropping_brush);
        
        int px=d2px(x_min_crop);
        px=std::min(px,sx-padxp);
        
        if(px>=padxm) gc->DrawRectangle(padxm,padym,px-padxm,sy-padym-padyp);
        
        px=d2px(x_max_crop);
        px=std::max(px,padxm);
        
        if(px<=sx-padxp) gc->DrawRectangle(px,padym,sx-px-padxp,sy-padym-padyp);
    }
}

void GraphFitter::set_cropping(double x_min_crop_,double x_max_crop_)
{
    show_cropping=true;
    x_min_crop=x_min_crop_;
    x_max_crop=x_max_crop_;
    
    Refresh();
}

void GraphFitter::show_handles(std::vector<FitterHandle> &handles_,bool val)
{
    unsigned int i,j;
    
    for(i=0;i<handles.size();i++)
    {
        bool handles_check=true;
        
        if(handles_.size()==handles[i].size())
        {
            for(j=0;j<handles[i].size();j++)
                if(handles[i][j]!=&handles_[j]) handles_check=false;
        }
        else handles_check=false;
        
        if(handles_check)
        {
            show_handles_v[i]=val;
            break;
        }
    }
}

void GraphFitter::set_handles_color(std::vector<FitterHandle> &handles_,
                                    double r,double g,double b)
{
    unsigned int i,j;
    wxColour col(r*255,g*255,b*255);
    
    for(i=0;i<handles.size();i++)
    {
        bool handles_check=true;
        
        if(handles_.size()==handles[i].size())
        {
            for(j=0;j<handles[i].size();j++)
                if(handles[i][j]!=&handles_[j]) handles_check=false;
        }
        else handles_check=false;
        
        if(handles_check)
        {
            handles_pens[i]=wxPen(col);
            return;
        }
    }
}

//#################
//   FitterModel
//#################

wxDEFINE_EVENT(EVT_FITTER_CHANGE,wxCommandEvent);

FitterModel::FitterModel(wxWindow *parent,int type_,GraphFitter *graph_,
                         std::vector<double> *x_data_,
                         std::vector<double> const &params_,
                         std::vector<std::string> const &param_names,
                         std::vector<FitterHandle> const &handles_,
                         void (*f_)(std::vector<double> const&,std::vector<double> const&,std::vector<double> &),
                         void (*h_to_p_)(std::vector<FitterHandle> const&,std::vector<double> &),
                         void (*p_to_h_)(std::vector<double> const&,std::vector<FitterHandle> &))
    :PanelsListBase(parent),
     type(type_),
     Np(params_.size()),
     hide(false), substract(false),
     x_data(x_data_),
     y_data(x_data->size()),
     y_data_mutation(x_data->size()),
     params(params_),
     params_mutation(params_),
     handles(handles_),
     params_ctrl(Np),
     graph(graph_),
     f(f_), h_to_p(h_to_p_), p_to_h(p_to_h_)
{
    double rd=0.4+0.6*std::pow(randp(),3.0);
    double gd=0.4+0.6*std::pow(randp(),3.0);
    double bd=0.4+0.6*std::pow(randp(),3.0);
    
    unsigned char r=255*rd;
    unsigned char g=255*gd;
    unsigned char b=255*bd;
    
    wxColour btn_col(r,g,b);
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    color_btn=new wxButton(this,wxID_ANY," ",wxDefaultPosition,wxDefaultSize,wxBORDER_NONE|wxBU_EXACTFIT);
    color_btn->SetBackgroundColour(btn_col);
    hide_ctrl=new wxCheckBox(this,wxID_ANY,"Hide");
    substract_ctrl=new wxCheckBox(this,wxID_ANY,"Substract");
    
    btn_sizer->Add(color_btn,wxSizerFlags().Border(wxRIGHT,2));
    btn_sizer->Add(hide_ctrl,wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL));
    btn_sizer->Add(substract_ctrl,wxSizerFlags().Align(wxALIGN_CENTRE_VERTICAL));
    
    sizer->Add(btn_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    for(int i=0;i<Np;i++)
    {
        params_ctrl[i]=new NamedTextCtrl<double>(this,param_names[i],params[i]);
        params_ctrl[i]->Bind(EVT_NAMEDTXTCTRL,&FitterModel::evt_value,this);
        sizer->Add(params_ctrl[i],wxSizerFlags().Expand());
    }
    
    update_data();
    
    for(unsigned int i=0;i<handles.size();i++) handles[i].model=this;
    
    graph->add_data(x_data,&y_data,rd,gd,bd);
    graph->add_handles(handles);
    graph->set_handles_color(handles,rd,gd,bd);
    graph->Refresh();
    
    color_btn->Bind(wxEVT_BUTTON,&FitterModel::evt_color,this);
    hide_ctrl->Bind(wxEVT_CHECKBOX,&FitterModel::evt_hide,this);
    substract_ctrl->Bind(wxEVT_CHECKBOX,&FitterModel::evt_substract,this);
}

FitterModel::~FitterModel()
{
}

void FitterModel::accept_mutation()
{
    params=params_mutation;
}

void FitterModel::apoptose(wxCommandEvent &event)
{
    graph->forget_data(x_data,&y_data);
    graph->forget_handles(handles);
    graph->Refresh();
    
    PanelsListBase::apoptose(event);
}

void FitterModel::evt_color(wxCommandEvent &event)
{
    wxColourData data;
    data.SetColour(color_btn->GetBackgroundColour());
    
    wxColourDialog dialog(this,&data);
    dialog.ShowModal();
    
    data=dialog.GetColourData();
    wxColour color=data.GetColour();
    
    color_btn->SetBackgroundColour(color);
    
    double r=color.Red()/255.0;
    double g=color.Green()/255.0;
    double b=color.Blue()/255.0;
    
    graph->set_data_color(x_data,&y_data,r,g,b);
    graph->set_handles_color(handles,r,g,b);
    graph->Refresh();
    
    event.Skip();
}

void FitterModel::evt_hide(wxCommandEvent &event)
{
    hide=hide_ctrl->GetValue();
    
    graph->show_data(x_data,&y_data,!hide);
    graph->show_handles(handles,!hide);
    graph->Refresh();
    
    event.Skip();
}

void FitterModel::evt_substract(wxCommandEvent &event)
{
    substract=substract_ctrl->GetValue();
    
    wxCommandEvent event_out(EVT_FITTER_CHANGE);
    
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void FitterModel::evt_value(wxCommandEvent &event)
{
    for(int i=0;i<Np;i++) params[i]=params_ctrl[i]->get_value();
    
    update_handles();
    update_data();
    
    wxCommandEvent event_out(EVT_FITTER_CHANGE);
    
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void FitterModel::get_latex(int id,std::string &formula_str, std::string &params_str)
{
    std::stringstream formula_strm,
                      params_strm;
    std::string alpha, beta, gamma, delta;
    
    alpha="\\alpha_{" + std::to_string(id) + "}";
    beta="\\beta_{" + std::to_string(id) + "}";
    gamma="\\gamma_{" + std::to_string(id) + "}";
    delta="\\delta_{" + std::to_string(id) + "}";
    
    std::vector<std::string> tmp(4);
    tmp[0]=alpha; tmp[1]=beta; tmp[2]=gamma; tmp[3]=delta;
    
         if(type==F_CONST) formula_strm<<alpha;
    else if(type==F_EXP) formula_strm<<alpha<<" exp^{"<<beta<<" x}";
    else if(type==F_GAUSSIAN) formula_strm<<alpha<<" exp^{-"<<beta<<"(x-"<<gamma<<")^2}";
    else if(type==F_INV_POW) formula_strm<<"\\frac{"<<alpha<<"}{x^{"<<beta<<"}}";
    else if(type==F_LOGISTIC) formula_strm<<"\\frac{"<<alpha<<"}{1+e^{ -"<<beta<<"(x-"<<gamma<<")}};";
    else if(type==F_LORENTZIAN) formula_strm<<"\\frac{"<<alpha<<"}{1+"<<beta<<"(x-"<<gamma<<")^2}";
    else if(type==F_LORENTZIAN_GEN) formula_strm<<"\\frac{"<<alpha<<"}{1+"<<beta<<"|x-"<<gamma<<"|^{"<<delta<<"}}";
    else if(type==F_PLANCK_FREQ) formula_strm<<"\\frac{"<<alpha<<" x^3}{e^{ "<<beta<<" x}-1}";
    else if(type==F_PLANCK_WVL) formula_strm<<"\\frac{"<<alpha<<"}{x^5}\\frac{1}{e^{ "<<beta<<"/x}-1}";
    else if(type==F_SINE) formula_strm<<alpha<<"+"<<beta<<"\\sin("<<gamma<<"x+"<<delta<<")";
    
    params_strm<<"\\begin{array}{l}"<<std::endl;
    for(int i=0;i<Np;i++)
    {
        params_strm<<tmp[i]<<"="<<params[i]<<" ";
        
        if(i!=Np-1)params_strm<<"\\\\";
        params_strm<<std::endl;
    }
    params_strm<<"\\end{array}"<<std::endl;
    
    formula_str=formula_strm.str();
    params_str=params_strm.str();
}

void FitterModel::get_matlab(int id,std::string &formula_str, std::string &params_str)
{
    std::stringstream formula_strm,
                      params_strm;
    std::string var_a, var_b, var_c, var_d, var_e;
    
    var_a="a"+std::to_string(id);
    var_b="b"+std::to_string(id);
    var_c="c"+std::to_string(id);
    var_d="d"+std::to_string(id);
    var_e="e"+std::to_string(id);
    
    std::vector<std::string> tmp(5);
    tmp[0]=var_a; tmp[1]=var_b; tmp[2]=var_c; tmp[3]=var_d; tmp[4]=var_e;
    
    formula_strm<<"f_"<<id<<"=";
    
         if(type==F_CONST) formula_strm<<var_a<<"+0*x;";
    else if(type==F_EXP) formula_strm<<var_a<<"*exp("<<var_b<<"*x);";
    else if(type==F_GAUSSIAN) formula_strm<<var_a<<"*exp(-"<<var_b<<"*(x-"<<var_c<<").^2);";
    else if(type==F_INV_POW) formula_strm<<var_a<<"./x.^"<<var_b<<";";
    else if(type==F_LOGISTIC) formula_strm<<var_a<<"./(1.0+exp(-"<<var_b<<"*(x-"<<var_c<<")));";
    else if(type==F_LORENTZIAN) formula_strm<<var_a<<"./(1+"<<var_b<<"*(x-"<<var_c<<").^2);";
    else if(type==F_LORENTZIAN_GEN) formula_strm<<var_a<<"./(1+"<<var_b<<"*abs(x-"<<var_c<<").^"<<var_d<<");";
    else if(type==F_PLANCK_FREQ) formula_strm<<var_a<<"*x.^3./(exp("<<var_b<<"*x)-1);";
    else if(type==F_PLANCK_WVL) formula_strm<<var_a<<"./x.^5./(exp( "<<var_b<<"./x)-1);";
    else if(type==F_SINE) formula_strm<<var_a<<"+"<<var_b<<".*sin("<<var_c<<"*x+"<<var_d<<");";
    
    for(int i=0;i<Np;i++)
    {
        params_strm<<tmp[i]<<"="<<params[i]<<";"<<std::endl;
    }
    
    formula_str=formula_strm.str();
    params_str=params_strm.str();
}

void FitterModel::lock()
{
    for(int i=0;i<Np;i++) params_ctrl[i]->lock();
    
    PanelsListBase::lock();
}

void FitterModel::mutate(double eps)
{ 
    for(int i=0;i<Np;i++)
        params_mutation[i]=params[i]*(1+randp(-eps,+eps));
    
    (*f)(*x_data,params_mutation,y_data_mutation);
}

void FitterModel::reallocate()
{
    int Nx=x_data->size();
    y_data.resize(Nx);
    y_data_mutation.resize(Nx);
    
    update_data();
}

void FitterModel::recompute_from_handles()
{
    static int k;
    k++;
    
    update_params();
    
    for(int i=0;i<Np;i++) params_ctrl[i]->set_value(params[i]);
    
    update_data();
    
    wxCommandEvent event_out(EVT_FITTER_CHANGE);
    
    wxPostEvent(this,event_out);
}

void FitterModel::unlock()
{
    for(int i=0;i<Np;i++) params_ctrl[i]->unlock();
    PanelsListBase::unlock();
}

void FitterModel::update_controls()
{
    for(int i=0;i<Np;i++) params_ctrl[i]->set_value(params[i]);
}

void FitterModel::update_data() { (*f)(*x_data,params,y_data); }
void FitterModel::update_handles() { (*p_to_h)(params,handles); }
void FitterModel::update_params() { (*h_to_p)(handles,params); }

//########################
//   FitterExportDialog
//########################

FitterExportDialog::FitterExportDialog()
    :wxDialog(NULL,-1,""),
     choice_ok(false)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxString choices[]={"Partial MatLab script","Full MatLab script","Partial LaTeX code","Text File"};
    type_ctrl=new wxRadioBox(this,wxID_ANY,"Export type",wxDefaultPosition,wxDefaultSize,4,choices,1);
    type_ctrl->SetSelection(0);
    
    top_sizer->Add(type_ctrl,wxSizerFlags().Expand().Border(wxALL,3));
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    btn_sizer->Add(ok_btn);
    btn_sizer->Add(cancel_btn);
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Border(wxALL,2).Align(wxALIGN_RIGHT));
    
    cancel_btn->Bind(wxEVT_BUTTON,&FitterExportDialog::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&FitterExportDialog::evt_ok,this);
    
    SetSizerAndFit(top_sizer);
    ShowModal();
}

void FitterExportDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void FitterExportDialog::evt_ok(wxCommandEvent &event)
{
    export_type=type_ctrl->GetSelection();
    
    wxString wildcard="MatLab script (*.m)|*.m";
    
         if(export_type==2) wildcard="LaTeX file (*.tex)|*.tex";
    else if(export_type==3) wildcard="Text file (*.txt)|*.txt";
    
    fname=wxFileSelector("Select the file to save to:",
                         wxEmptyString,
                         wxEmptyString,
                         wxEmptyString,
                         wildcard,
                         wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if(fname.size()==0) return;
    
    choice_ok=true;
    Close();
}

//#######################
//   FitterXDataDialog
//#######################

wxDEFINE_EVENT(EVT_X_DATA_APPLY,wxCommandEvent);
wxDEFINE_EVENT(EVT_X_DATA_CLOSE,wxCommandEvent);
wxDEFINE_EVENT(EVT_X_DATA_RESET,wxCommandEvent);
wxDEFINE_EVENT(EVT_X_DATA_VALUE,wxCommandEvent);

FitterXDataDialog::FitterXDataDialog(wxWindow *parent,
                                     double x_min_,double x_max_,
                                     int Np_,int Nstep_)
    :wxDialog(parent,0,"X Data Manipulation")
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Cropping
    
    wxStaticBoxSizer *crop_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Crop");
    
    x_min=new NamedTextCtrl<double>(this,"X Min: ",x_min_);
    x_max=new NamedTextCtrl<double>(this,"X Max: ",x_max_);
    
    crop_sizer->Add(x_min,wxSizerFlags().Expand());
    crop_sizer->Add(x_max,wxSizerFlags().Expand());
    
    top_sizer->Add(crop_sizer,wxSizerFlags().Expand());
    
    // Points
    
    wxStaticBoxSizer *points_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"Points");
    
    Np=new NamedTextCtrl<int>(this,"N Points: ",Np_);
    Np->lock();
    Nstep=new NamedTextCtrl<int>(this,"   Stepping: ",Nstep_);
    
    points_sizer->Add(Np);
    points_sizer->Add(Nstep);
    
    top_sizer->Add(points_sizer,wxSizerFlags().Expand());
    
    // Buttons
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *btn_apply=new wxButton(this,wxID_ANY,"Apply");
    wxButton *btn_reset=new wxButton(this,wxID_ANY,"Reset");
    
    btn_sizer->Add(btn_apply,wxSizerFlags(1));
    btn_sizer->Add(btn_reset,wxSizerFlags(1));
    
    top_sizer->Add(btn_sizer,wxSizerFlags().Expand());
    
    btn_apply->Bind(wxEVT_BUTTON,&FitterXDataDialog::evt_apply,this);
    btn_reset->Bind(wxEVT_BUTTON,&FitterXDataDialog::evt_reset,this);
    Nstep->Bind(EVT_NAMEDTXTCTRL,&FitterXDataDialog::evt_stepping,this);
    x_min->Bind(EVT_NAMEDTXTCTRL,&FitterXDataDialog::evt_value,this);
    x_max->Bind(EVT_NAMEDTXTCTRL,&FitterXDataDialog::evt_value,this);
    
    Bind(wxEVT_CLOSE_WINDOW,&FitterXDataDialog::evt_close,this);
    
    SetSizerAndFit(top_sizer);
    Show();
}

void FitterXDataDialog::evt_apply(wxCommandEvent &event)
{
    wxCommandEvent event_out(EVT_X_DATA_APPLY);
    wxPostEvent(GetParent(),event_out);
}

void FitterXDataDialog::evt_close(wxCloseEvent &event)
{
    wxCommandEvent event_out(EVT_X_DATA_CLOSE);
    wxPostEvent(GetParent(),event_out);
    
    event.Skip();
}

void FitterXDataDialog::evt_reset(wxCommandEvent &event)
{
    Nstep->set_value(1);
    
    wxCommandEvent event_out(EVT_X_DATA_RESET);
    wxPostEvent(GetParent(),event_out);
}

void FitterXDataDialog::evt_stepping(wxCommandEvent &event)
{
    if(Nstep->get_value()<1) Nstep->set_value(1);
}

void FitterXDataDialog::evt_value(wxCommandEvent &event)
{
    wxCommandEvent event_out(EVT_X_DATA_VALUE);
    wxPostEvent(GetParent(),event_out);
}

double FitterXDataDialog::get_min() { return x_min->get_value(); }
double FitterXDataDialog::get_max() { return x_max->get_value(); }
int FitterXDataDialog::get_stepping() { return Nstep->get_value(); }

void FitterXDataDialog::set_cropping(double x_min_,double x_max_)
{
    x_min->set_value(x_min_);
    x_max->set_value(x_max_);
}

void FitterXDataDialog::set_points(int Np_)
{
    Np->set_value(Np_);
}

//#################
//   FitterFrame
//#################

wxDEFINE_EVENT(EVT_PURE_REFRESH,wxCommandEvent);
wxDEFINE_EVENT(EVT_OPTIM_COMPLETE,wxCommandEvent);

FitterFrame::FitterFrame(wxString const &title)
    :BaseFrame(title),
     first_load(true),
     optim_toggle(false),
     Nl(0), Nd(0), Nstep(1),
     N_models(0), N_models_tot(0),
     optim_condition(optim_mutex),
     refresh_condition(refresh_mutex),
     x_min_crop(0), x_max_crop(0), x_data_dialog(nullptr)
{
    refresh_mutex.Lock();
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - Data
    
    wxStaticBoxSizer *data_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Data");
    
    wxBoxSizer *data_load_sizer=new wxBoxSizer(wxHORIZONTAL);
    data_name=new wxTextCtrl(ctrl_panel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    data_load_btn=new wxButton(ctrl_panel,wxID_ANY,"Load",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    data_load_sizer->Add(data_name,wxSizerFlags(1));
    data_load_sizer->Add(data_load_btn,wxSizerFlags());
    
    x_data_btn=new wxButton(ctrl_panel,wxID_ANY,"X Data Manip");
    
    data_sizer->Add(data_load_sizer,wxSizerFlags().Expand());
    data_sizer->Add(x_data_btn,wxSizerFlags().Expand());
    
    // - Models
    
    wxStaticBoxSizer *models_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Models");
    
    add_model_btn=new wxButton(ctrl_panel,wxID_ANY,"Add Model");
    add_model_btn->Disable();
    models_panel=new PanelsList<>(ctrl_panel);
    
    models_sizer->Add(add_model_btn,wxSizerFlags().Expand());
    models_sizer->Add(models_panel,wxSizerFlags(1).Expand());
    
    wxStaticBoxSizer *optim_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Optimization");
    
    // - Optimization
    
    optim_btn=new wxToggleButton(ctrl_panel,wxID_ANY,"Optimize");
    optim_btn->Disable();
    optim_sizer->Add(optim_btn,wxSizerFlags().Expand());
    
    error_ctrl=new NamedTextCtrl<double>(ctrl_panel,"Error",0,true);
    error_ctrl->lock();
    export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    export_btn->Disable();
    
    ctrl_sizer->Add(data_sizer,std_flags);
    ctrl_sizer->Add(models_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    ctrl_sizer->Add(optim_sizer,std_flags);
    ctrl_sizer->Add(error_ctrl,std_flags);
    ctrl_sizer->Add(export_btn,std_flags);
    
    // Display
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    fitter=new GraphFitter(display_panel);
    fitter_error=new GraphFitter(display_panel);
    
    display_sizer->Add(fitter,wxSizerFlags(3).Expand());
    display_sizer->Add(fitter_error,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    // Menus
    
    // Bindings
    
    add_model_btn->Bind(wxEVT_BUTTON,&FitterFrame::evt_add_model,this);
    data_load_btn->Bind(wxEVT_BUTTON,&FitterFrame::evt_load,this);
    export_btn->Bind(wxEVT_BUTTON,&FitterFrame::evt_export,this);
    optim_btn->Bind(wxEVT_TOGGLEBUTTON,&FitterFrame::evt_optimize,this);
    x_data_btn->Bind(wxEVT_BUTTON,&FitterFrame::evt_x_data_dialog,this);
    
    Bind(EVT_FITTER_CHANGE,&FitterFrame::evt_recompute,this);
    Bind(EVT_GRAPH_CROP_VALUE,&FitterFrame::evt_x_data_graph_value,this);
    Bind(EVT_PLIST_REMOVE,&FitterFrame::evt_model_delete,this);
    Bind(EVT_PURE_REFRESH,&FitterFrame::evt_pure_refresh,this);
    Bind(EVT_OPTIM_COMPLETE,&FitterFrame::evt_optimization_cleanup,this);
    Bind(EVT_X_DATA_APPLY,&FitterFrame::evt_x_data_dialog_apply,this);
    Bind(EVT_X_DATA_CLOSE,&FitterFrame::evt_x_data_dialog_close,this);
    Bind(EVT_X_DATA_RESET,&FitterFrame::evt_x_data_dialog_reset,this);
    Bind(EVT_X_DATA_VALUE,&FitterFrame::evt_x_data_dialog_value,this);
    //
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
}

FitterFrame::~FitterFrame()
{
    if(x_data_dialog!=nullptr) x_data_dialog->Close();
}

void FitterFrame::evt_add_model(wxCommandEvent &event)
{
    std::vector<double> params;
    std::vector<std::string> param_names;
    std::vector<FitterHandle> handles;
    
    void (*f)(std::vector<double> const&,std::vector<double> const&,std::vector<double> &)=nullptr;
    void (*h_to_p)(std::vector<FitterHandle> const&,std::vector<double> &)=nullptr;
    void (*p_to_h)(std::vector<double> const&,std::vector<FitterHandle> &)=nullptr;
    void (*base_h)(double,double,double,double,std::vector<FitterHandle> &)=nullptr;
    
    FitterModelDialog dialog;
    
    if(!dialog.choice_ok) return;
    
    int type=dialog.choice_type;
    
    wxString panel_title;
    
    double x_min=0,x_max=0,y_min=0,y_max=0;
    
    fitter->get_scale(x_min,x_max,y_min,y_max);
    
    if(type==F_CONST)
    {
        params.resize(1); param_names.resize(1); handles.resize(1);
        param_names[0]="a: ";
        
        f=&f_const;
        h_to_p=&f_const_h_to_p;
        p_to_h=&f_const_p_to_h;
        base_h=&f_const_base_h;
        
        panel_title="Const";
    }
    else if(type==F_EXP)
    {
        params.resize(2); param_names.resize(2); handles.resize(2);
        param_names[0]="a: ";
        param_names[1]="b: ";
        
        f=&f_exp;
        h_to_p=&f_exp_h_to_p;
        p_to_h=&f_exp_p_to_h;
        base_h=&f_exp_base_h;
        
        panel_title="Exp";
    }
    else if(type==F_GAUSSIAN)
    {
        params.resize(3); param_names.resize(3); handles.resize(2);
        param_names[0]="a: ";
        param_names[1]="b: ";
        param_names[2]="c: ";
        
        f=&f_gaussian;
        h_to_p=&f_gaussian_h_to_p;
        p_to_h=&f_gaussian_p_to_h;
        base_h=&f_gaussian_base_h;
        
        panel_title="Gaussian";
    }
    else if(type==F_INV_POW)
    {
        params.resize(2); param_names.resize(2); handles.resize(2);
        param_names[0]="a: ";
        param_names[1]="b: ";
        
        f=&f_inv_pow;
        h_to_p=&f_inv_pow_h_to_p;
        p_to_h=&f_inv_pow_p_to_h;
        base_h=&f_inv_pow_base_h;
        
        panel_title="Inverse";
    }
    else if(type==F_LOGISTIC)
    {
        params.resize(3); param_names.resize(3); handles.resize(2);
        param_names[0]="a: ";
        param_names[1]="b: ";
        param_names[2]="c: ";
        
        f=&f_logistic;
        h_to_p=&f_logistic_h_to_p;
        p_to_h=&f_logistic_p_to_h;
        base_h=&f_logistic_base_h;
        
        panel_title="Logistic";
    }
    else if(type==F_LORENTZIAN)
    {
        params.resize(3); param_names.resize(3); handles.resize(2);
        param_names[0]="a: ";
        param_names[1]="b: ";
        param_names[2]="c: ";
        
        f=&f_lorentz;
        h_to_p=&f_lorentz_h_to_p;
        p_to_h=&f_lorentz_p_to_h;
        base_h=&f_lorentz_base_h;
        
        panel_title="Lorentian";
    }
    else if(type==F_LORENTZIAN_GEN)
    {
        params.resize(4); param_names.resize(4); handles.resize(3);
        param_names[0]="a: ";
        param_names[1]="b: ";
        param_names[2]="c: ";
        param_names[3]="d: ";
        
        f=&f_lorentz_gen;
        h_to_p=&f_lorentz_gen_h_to_p;
        p_to_h=&f_lorentz_gen_p_to_h;
        base_h=&f_lorentz_gen_base_h;
        
        panel_title="Lorentian Gen";
    }
    else if(type==F_PLANCK_FREQ)
    {
        params.resize(2); param_names.resize(2); handles.resize(1);
        param_names[0]="a: ";
        param_names[1]="b: ";
        
        f=&f_planck_freq;
        h_to_p=&f_planck_freq_h_to_p;
        p_to_h=&f_planck_freq_p_to_h;
        base_h=&f_planck_freq_base_h;
        
        panel_title="Planck - Freq";
    }
    else if(type==F_PLANCK_WVL)
    {
        params.resize(2); param_names.resize(2); handles.resize(1);
        param_names[0]="a: ";
        param_names[1]="b: ";
        
        f=&f_planck_wvl;
        h_to_p=&f_planck_wvl_h_to_p;
        p_to_h=&f_planck_wvl_p_to_h;
        base_h=&f_planck_wvl_base_h;
        
        panel_title="Planck - Wvl";
    }
    else if(type==F_SINE)
    {
        params.resize(5); param_names.resize(5); handles.resize(2);
        params[4]=0.5;
        param_names[0]="y0: ";
        param_names[1]="A: ";
        param_names[2]="w: ";
        param_names[3]="phi: ";
        param_names[4]="D: ";
        
        f=&f_sine;
        h_to_p=&f_sine_h_to_p;
        p_to_h=&f_sine_p_to_h;
        base_h=&f_sine_base_h;
        
        panel_title="Sine";
    }
    
    (*base_h)(x_min,x_max,y_min,y_max,handles);
    (*h_to_p)(handles,params);
    
    FitterModel *panel=dynamic_cast<FitterModel*>(models_panel->add_panel<FitterModel>(type,fitter,&x_data_display,
                                                                                      params,param_names,handles,
                                                                                      f,h_to_p,p_to_h));
    
    N_models=models_panel->get_size();
    models_list.push_back(panel);
    
    std::stringstream strm;
    strm<<" "<<N_models_tot;
    N_models_tot++;
    
    panel_title.append(strm.str());
    panel->set_title(panel_title);
    
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
    
    evt_recompute_sub();
    
    fitter->Refresh();
    fitter_error->autoscale();
    
    optim_btn->Enable();
    
    event.Skip();
}

void FitterFrame::evt_export(wxCommandEvent &event)
{
    rebuild_models_list();
    
    int i;
    
    FitterExportDialog dialog;
    
    if(!dialog.choice_ok) return;
    
    int export_type=dialog.export_type;
    
    wxFileName fname(dialog.fname);
    wxFile file(fname.GetFullPath(),wxFile::write);
    
    if(!file.IsOpened())
    {
        wxMessageBox("Error: could not open file");
        return;
    }
    
    std::vector<std::string> formula_str(N_models),
                             params_str(N_models);
    
    if(export_type==0 || export_type==1)
    {
        for(i=0;i<N_models;i++)
            models_list[i]->get_matlab(i+1,formula_str[i],params_str[i]);
        
        std::stringstream script_strm;
        
        for(i=0;i<N_models;i++) script_strm<<params_str[i]<<std::endl;
        for(i=0;i<N_models;i++) script_strm<<formula_str[i]<<std::endl;
        script_strm<<std::endl<<"f_fit=";
        
        for(i=1;i<=N_models;i++)
        {
            script_strm<<"f_"<<i;
            if(i!=N_models) script_strm<<"+";
            else script_strm<<";";
        }
        script_strm<<std::endl;
        
        if(export_type==0) 
        {
            file.Write(wxString(script_strm.str()));
            file.Close();
        }
        else
        {
            std::stringstream full_script;
            
            file.Write(wxString("function out="));
            file.Write(fname.GetName());
            file.Write(wxString("(varargin)\n"));
            
            full_script<<"x_data=[";
            for(i=0;i<Nd;i++)
            {
                full_script<<x_data_display[i];
                if(i!=Nd-1) full_script<<";";
                else full_script<<"];"<<std::endl;
            }
            full_script<<"y_data=[";
            for(i=0;i<Nd;i++)
            {
                full_script<<y_data_work[i];
                if(i!=Nd-1) full_script<<";";
                else full_script<<"];"<<std::endl;
            }
            
            full_script<<std::endl<<"x=x_data;"<<std::endl<<std::endl;
            full_script<<script_strm.str()<<std::endl;
            full_script<<"if nargin>0"<<std::endl;
            full_script<<"if strcmp('display',varargin{1})==1"<<std::endl;
            full_script<<"    plot(x_data,y_data,'k',x,f_fit','r')"<<std::endl;
            full_script<<"    legend('data','fit')"<<std::endl;
            full_script<<"end"<<std::endl<<std::endl;
            
            full_script<<"if strcmp('display_ext',varargin{1})==1"<<std::endl;
            full_script<<"    plot(x_data,y_data,'k',x,f_fit','r',";
            for(i=1;i<=N_models;i++)
            {
                full_script<<"x,f_"<<i;
                if(i!=N_models) full_script<<",";
                else full_script<<")"<<std::endl;
            }
            full_script<<"    legend('data','fit',";
            for(i=1;i<=N_models;i++)
            {
                full_script<<"'f"<<i<<"'";
                if(i!=N_models) full_script<<",";
                else full_script<<")"<<std::endl;
            }
            full_script<<"end"<<std::endl<<"end"<<std::endl<<std::endl;
            full_script<<"out=[x_data,y_data,f_fit];"<<std::endl;
            full_script<<"end";
            
            file.Write(wxString(full_script.str()));
            file.Close();
        }
    }
    else if(export_type==2)
    {
        for(i=0;i<N_models;i++)
            models_list[i]->get_latex(i+1,formula_str[i],params_str[i]);
        
        std::stringstream strm;
        
        strm<<"\\begin{split}"<<std::endl;
        strm<<"    f(x)=";
        
        for(i=0;i<N_models;i++)
        {
            if(i==0) strm<<"& ";
            else strm<<"        & +";
            
            strm<<formula_str[i];
            
            if(i!=N_models-1) strm<<"\\\\";
            strm<<std::endl;
        }
        strm<<"\\end{split}"<<std::endl<<std::endl;
        
        strm<<"\\begin{equation}"<<std::endl;
        for(i=0;i<N_models;i++) strm<<params_str[i]<<std::endl;
        strm<<"\\end{equation}"<<std::endl;
        
        file.Write(wxString(strm.str()));
        file.Close();
    }
    else if(export_type==3)
    {
        std::stringstream strm;
        
        for(i=0;i<Nd;i++)
        {
            strm<<x_data_display[i]<<" "<<y_data_work[i];
            
            double S=0;
            for(int j=0;j<N_models;j++) S+=models_list[j]->y_data[i];
            strm<<" "<<S;
            
            for(int j=0;j<N_models;j++)
                strm<<" "<<models_list[j]->y_data[i];
                
            
            if(i!=Nd-1) strm<<std::endl;
        }
        
        file.Write(wxString(strm.str()));
        file.Close();
    }
    
    event.Skip();
}

void FitterFrame::evt_load(wxCommandEvent &event)
{
    int i;
    
    wxString fname_tmp=wxFileSelector("Select the data file");
    if(fname_tmp.size()==0) return;
    
    add_model_btn->Enable();
    export_btn->Enable();
    first_load=false;
    
//    if(!first_load)
//    {
//        models_panel->clear();
//        models_list.clear();
//        fitter->clear_graph();
//        fitter_error->clear_graph();
//    }
    
    wxFileName fname(fname_tmp);
    data_name->ChangeValue(fname.GetFullName());
    
    std::string fname_std=fname_tmp.ToStdString();
    
    // Data loading
    
    std::vector<std::vector<double>> data_holder;
    ascii_data_loader(fname_std,data_holder);
    
    if(data_holder.size()<2)
    {
        wxMessageBox("Error: failed opening the data file");
        return;
    }
    
    Nl=data_holder[0].size();
    Nstep=1;
    
    x_data.resize(Nl);
    y_data.resize(Nl);
    
    x_data=data_holder[0];
    y_data=data_holder[1];
    
    if(Nl>1)
    {
        if(x_data[0]>x_data[1])
        {
            for(i=0;i<Nl/2;i++)
            {
                std::swap(x_data[i],x_data[Nl-1-i]);
                std::swap(y_data[i],y_data[Nl-1-i]);
            }
        }
    }
    
    x_min_crop=x_data[0];
    x_max_crop=x_data[0];
    
    for(i=0;i<Nl;i++)
    {
        x_min_crop=std::min(x_data[i],x_min_crop);
        x_max_crop=std::max(x_data[i],x_max_crop);
    }
    
    // Data renormalization
    
    rebuild_data();
//    Nd=(Nl-1)/Nstep+1;
//    
//    x_data_display.resize(Nd);
//    y_data_display.resize(Nd);
//    error_display.resize(Nd);
//    
//    for(int i=0;i<Nd;i++)
//    {
//        x_data_display[i]=x_data[i*Nstep];
//        y_data_display[i]=y_data[i*Nstep];
//        error_display[i]=1.0;
//    }
    
    fitter->add_data(&x_data_display,&y_data_display,1.0,0,0);
    fitter->autoscale();
    
    fitter_error->add_data(&x_data_display,&error_display,1.0,0,0);
    fitter_error->autoscale();
    
    event.Skip();
}

void FitterFrame::evt_model_delete(wxCommandEvent &event)
{
    rebuild_models_list();
    
    evt_recompute_sub();
    
    error_ctrl->set_value(error_value);
    
    fitter->Refresh();
    fitter_error->autoscale();
}

class FitterOptimThread: public wxThread
{
    public:
        FitterFrame *frame;
        
        ExitCode Entry()
        {
            frame->optimize();
            
            Delete();
            
            return 0;
        }
};

void FitterFrame::evt_optimize(wxCommandEvent &event)
{
    if(optim_toggle==false)
    {
        optim_toggle=true;
        
        add_model_btn->Disable();
        data_load_btn->Disable();
        export_btn->Disable();
        models_panel->lock();
        x_data_btn->Disable();
        
        FitterOptimThread *optim_thread=new FitterOptimThread;
        optim_thread->frame=this;
        optim_thread->Run();
    }
    else optim_toggle=false;
}

void FitterFrame::evt_optimization_cleanup(wxCommandEvent &event)
{
    for(int l=0;l<N_models;l++)
        models_list[l]->update_controls();
    
    add_model_btn->Enable();
    data_load_btn->Enable();
    export_btn->Enable();
    models_panel->unlock();
    x_data_btn->Enable();
    
    error_ctrl->set_value(error_value);
    
    fitter->Refresh();
    fitter_error->autoscale();
    
    refresh_mutex.Lock();
}

void FitterFrame::evt_pure_refresh(wxCommandEvent &event)
{
    refresh_condition.Wait();
    optim_mutex.Lock();
    
    int l;
    
    for(l=0;l<N_models;l++)
    {
        models_list[l]->update_data();
        models_list[l]->update_handles();
        models_list[l]->update_controls();
    }
    
    evt_recompute_sub();
    
    error_ctrl->set_value(error_value);
    
    fitter->Refresh();
    fitter_error->autoscale();
    
    Refresh();
    
    optim_condition.Broadcast();
    optim_mutex.Unlock();
}

void FitterFrame::evt_recompute(wxCommandEvent &event)
{
    evt_recompute_sub();
    
    error_ctrl->set_value(error_value);
    
    fitter->Refresh();
    fitter_error->autoscale();
}

void FitterFrame::evt_recompute_sub()
{
    int i,l;
    
    for(i=0;i<Nd;i++)
    {
        y_data_display[i]=y_data_work[i];
        error_display[i]=0;
    }
    
    int Np=models_panel->get_size();
    
    for(l=0;l<Np;l++)
    {
        FitterModel *panel=dynamic_cast<FitterModel*>(models_panel->get_panel(l));
        
        for(i=0;i<Nd;i++)
            error_display[i]+=panel->y_data[i];
    }
    
    error_value=0;
    
    for(i=0;i<Nd;i++)
    {
        error_value+=std::abs(error_display[i]-y_data_display[i]);
        error_display[i]=std::log10(std::abs((error_display[i]-y_data_display[i])/y_data_display[i]));
    }
    
    for(l=0;l<Np;l++)
    {
        FitterModel *panel=dynamic_cast<FitterModel*>(models_panel->get_panel(l));
        
        if(panel->substract==true)
        {
            for(i=0;i<Nd;i++)
                y_data_display[i]-=panel->y_data[i];
        }
    }
}

void FitterFrame::evt_x_data_dialog(wxCommandEvent &event)
{
    if(x_data_dialog==nullptr)
    {
        x_data_dialog=new FitterXDataDialog(this,x_min_crop,x_max_crop,Nd,Nstep);
        fitter->set_cropping(x_min_crop,x_max_crop);
        
        add_model_btn->Disable();
        data_load_btn->Disable();
        optim_btn->Disable();
    }
    else x_data_dialog->SetFocus();
    
    event.Skip();
}

void FitterFrame::evt_x_data_dialog_close(wxCommandEvent &event)
{
    x_data_dialog=nullptr;
    
    add_model_btn->Enable();
    data_load_btn->Enable();
    if(models_list.size()>0) optim_btn->Enable();
    
    fitter->hide_cropping();
    fitter->Refresh();
}

void FitterFrame::evt_x_data_dialog_apply(wxCommandEvent &event)
{
    x_min_crop=x_data_dialog->get_min();
    x_max_crop=x_data_dialog->get_max();
    Nstep=x_data_dialog->get_stepping();
    
    rebuild_data();
    
    x_data_dialog->set_points(Nd);
    
    for(int l=0;l<N_models;l++)
        models_list[l]->reallocate();
    
    evt_recompute_sub();
    
    fitter->Refresh();
    fitter_error->autoscale();
}

void FitterFrame::evt_x_data_dialog_reset(wxCommandEvent &event)
{
    if(Nl==0) return;
    
    x_min_crop=x_data[0];
    x_max_crop=x_data[0];
    
    for(int i=0;i<Nl;i++)
    {
        x_min_crop=std::min(x_data[i],x_min_crop);
        x_max_crop=std::max(x_data[i],x_max_crop);
    }
    Nstep=1;
    
    rebuild_data();
    
    x_data_dialog->set_points(Nd);
    
    for(int l=0;l<N_models;l++)
        models_list[l]->reallocate();
    
    evt_recompute_sub();
    
    x_data_dialog->set_cropping(x_min_crop,x_max_crop);
    fitter->set_cropping(x_min_crop,x_max_crop);
    
    fitter->Refresh();
    fitter_error->autoscale();
}

void FitterFrame::evt_x_data_dialog_value(wxCommandEvent &event)
{
    if(x_data_dialog==nullptr) return;
    
    double tmp_x_min_crop=x_data_dialog->get_min();
    double tmp_x_max_crop=x_data_dialog->get_max();
    
    fitter->set_cropping(tmp_x_min_crop,tmp_x_max_crop);
    
    fitter->Refresh();
}

void FitterFrame::evt_x_data_graph_value(wxCommandEvent &event)
{
    if(x_data_dialog==nullptr) return;
    
    double tmp_x_min_crop=fitter->x_min_crop;
    double tmp_x_max_crop=fitter->x_max_crop;
    
    x_data_dialog->set_cropping(tmp_x_min_crop,tmp_x_max_crop);
    
    fitter->Refresh();
}

void FitterFrame::optimize()
{
    int i,l;
    
    std::vector<double> diff_reconst(Nd);
    
    for(i=0;i<Nd;i++) diff_reconst[i]=y_data_work[i];
    
    for(l=0;l<N_models;l++)
    {
        for(i=0;i<Nd;i++)
            diff_reconst[i]-=models_list[l]->y_data[i];
    }
    
    double best_error=0;
    for(i=0;i<Nd;i++) best_error+=std::abs(diff_reconst[i]);
    
    int Nfail=0;
    double eps_base=0.01;
    double eps=0.01;
    
    while(optim_toggle)
    {
        optim_mutex.Lock();
        
        for(l=0;l<N_models;l++) models_list[l]->mutate(eps);
        
        for(i=0;i<Nd;i++) diff_reconst[i]=y_data_work[i];
        for(l=0;l<N_models;l++)
        {
            for(i=0;i<Nd;i++) diff_reconst[i]-=models_list[l]->y_data_mutation[i];
        }
        
        double tmp_error=0;
        for(i=0;i<Nd;i++) tmp_error+=std::abs(diff_reconst[i]);
        
        if(tmp_error<best_error)
        {
            Nfail=0;
            eps=eps_base;
            
            best_error=tmp_error;
            for(l=0;l<N_models;l++) models_list[l]->accept_mutation();
            
            wxCommandEvent *event=new wxCommandEvent(EVT_PURE_REFRESH);
            
            wxQueueEvent(this,event);
            
            refresh_mutex.Lock();
            refresh_condition.Broadcast();
            refresh_mutex.Unlock();
            
            optim_condition.Wait();
        }
        else Nfail++;
        
        if(Nfail>1000) eps*=0.99;
        
        optim_mutex.Unlock();
    }
    
    wxCommandEvent *event=new wxCommandEvent(EVT_OPTIM_COMPLETE);
    
    wxQueueEvent(this,event);
}

void FitterFrame::rebuild_models_list()
{
    models_list.clear();
    
    N_models=models_panel->get_size();
    
    for(int l=0;l<N_models;l++)
    {
        FitterModel *panel=dynamic_cast<FitterModel*>(models_panel->get_panel(l));
        
        models_list.push_back(panel);
    }
}

void FitterFrame::rebuild_data()
{
    int i;
    
    int l1=0,l2=Nl;
    
    for(i=1;i<Nl;i++)
    {
        if(x_data[i]>=x_min_crop && x_data[i-1]<x_min_crop)
        {
            l1=i;
            break;
        }
    }
    
    for(i=0;i<Nl-1;i++)
    {
        if(x_data[i]<=x_max_crop && x_data[i+1]>x_max_crop)
        {
            l2=i+1;
            break;
        }
    }
    
    Nd=l2-l1;
    Nd/=Nstep;
        
    x_data_display.resize(Nd);
    y_data_display.resize(Nd);
    y_data_work.resize(Nd);
    error_display.resize(Nd);
    
    for(i=0;i<Nd;i++)
    {
        x_data_display[i]=x_data[l1+i*Nstep];
        y_data_display[i]=y_data[l1+i*Nstep];
        y_data_work[i]=y_data[l1+i*Nstep];
        error_display[i]=1.0;
    }
}
