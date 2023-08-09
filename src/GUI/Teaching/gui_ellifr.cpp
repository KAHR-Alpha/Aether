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

#include <gui_ellifr.h>
#include <phys_tools.h>

#include <iomanip>

extern const Imdouble Im;

//###############
// ImdoubleCtrl
//###############

ImdoubleCtrl::ImdoubleCtrl(wxWindow *parent,std::string const &name)
    :wxPanel(parent)
{
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,wxString(name));
    
    wxStaticText *amp_txt=new wxStaticText(this,wxID_ANY,"Amp: ");
    wxStaticText *phase_txt=new wxStaticText(this,wxID_ANY," Phase: ");
    
    amp_ctrl=new wxTextCtrl(this,wxID_ANY);
    phase_ctrl=new wxTextCtrl(this,wxID_ANY);
    
    amp_ctrl->SetEditable(false);
    phase_ctrl->SetEditable(false);
    
    sizer->Add(amp_txt);
    sizer->Add(amp_ctrl,wxSizerFlags(1));
    sizer->Add(phase_txt);
    sizer->Add(phase_ctrl,wxSizerFlags(1));
    
    SetSizer(sizer);
}

void ImdoubleCtrl::set_amp(double amp)
{
    std::stringstream strm;
    strm<<std::setprecision(7)<<amp;
    amp_ctrl->SetValue(strm.str());
}

void ImdoubleCtrl::set_phase(AngleRad phase)
{
    std::stringstream strm;
    strm<<std::setprecision(7)<<phase.degree();
    phase_ctrl->SetValue(strm.str());
}

//###############
//   SldCombi
//###############

SldCombi::SldCombi(wxWindow *parent,wxString const &name_,int base_val,int min_val,int max_val,double scale_,bool static_sizer)
    :wxPanel(parent), scale(scale_)
{
    if(!static_sizer)
    {
        wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *sld_sizer=new wxBoxSizer(wxHORIZONTAL);
        
        wxStaticText *name=new wxStaticText(this,wxID_ANY,name_);
        sld=new wxSlider(this,wxID_ANY,base_val,min_val,max_val);
        text_val=new wxTextCtrl(this,wxID_ANY);
        
        text_val->SetEditable(false);
        
        (*text_val)<<base_val*scale;
        
        sld_sizer->Add(sld,wxSizerFlags(1).Expand());
        sld_sizer->Add(text_val);
        
        top_sizer->Add(name);
        top_sizer->Add(sld_sizer,wxSizerFlags(1).Expand());
        
        SetSizer(top_sizer);
        
        sld->Bind(wxEVT_SLIDER,&SldCombi::update_val,this);
    }
    else
    {
        wxStaticBoxSizer *sld_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,name_);
        
        sld=new wxSlider(this,wxID_ANY,base_val,min_val,max_val);
        text_val=new wxTextCtrl(this,wxID_ANY);
        
        text_val->SetEditable(false);
        
        (*text_val)<<base_val*scale;
        
        sld_sizer->Add(sld,wxSizerFlags(1).Expand());
        sld_sizer->Add(text_val);
        
        SetSizer(sld_sizer);
        
        sld->Bind(wxEVT_SLIDER,&SldCombi::update_val,this);
    }
}

SldCombi::~SldCombi()
{
    std::cout<<"Destructing SldCombi"<<std::endl;
}

double SldCombi::get_value()
{
    double r=sld->GetValue()*scale;
    return r;
}

void SldCombi::set_range(int min_value,int max_value)
{
    sld->SetRange(min_value,max_value);
}

void SldCombi::set_value(int value)
{
    sld->SetValue(value);
    
    text_val->SetValue("");
    (*text_val)<<sld->GetValue()*scale;
}

void SldCombi::update_val(wxCommandEvent &event)
{
    text_val->SetValue("");
    (*text_val)<<sld->GetValue()*scale;
    
    event.Skip();
}

//############


ElliGraph::ElliGraph(wxWindow *parent)
    :wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(100,100)),
     t(0.0), period(1.0),
     p_amp_1(1.0), p_phase_1(0.0),
     s_amp_1(1.0), s_phase_1(0.0),
     p_amp_2(1.0), p_phase_2(0.0),
     s_amp_2(1.0), s_phase_2(0.0)
{
    SetBackgroundColour(wxColour(255,255,255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    elli_1_x.resize(361);
    elli_1_y.resize(361);
    elli_2_x.resize(361);
    elli_2_y.resize(361);
    
    rescale();
    
    field_pen=new wxPen();
    field_pen->SetColour(wxColour(255,0,0,255));
    field_pen->SetWidth(4);
    
    Bind(wxEVT_PAINT,&ElliGraph::on_paint,this);
//    #ifndef GTK_RMV_RESIZE
//    Bind(wxEVT_SIZE,&ElliGraph::on_resize,this);
//    #endif
}

ElliGraph::~ElliGraph()
{
    std::cout<<"Destructing ElliGraph"<<std::endl;
}

void ElliGraph::draw_data(wxGraphicsContext *gc)
{
    gc->SetPen(*wxBLACK_PEN);
    
    for(int i=0;i<360;i++)
    {
        gc->StrokeLine(elli_1_x[i],elli_1_y[i],
                       elli_1_x[i+1],elli_1_y[i+1]);
    }
    
    gc->StrokeLine(elli_1_x0-s_amp_1*r_max,elli_1_y0+4.5*base_unit,
                   elli_1_x0+s_amp_1*r_max,elli_1_y0+4.5*base_unit);
    
    gc->DrawText("S",elli_1_x0,elli_1_y0+5*base_unit);
    
    gc->StrokeLine(elli_1_x0-4.5*base_unit,elli_1_y0-p_amp_1*r_max,
                   elli_1_x0-4.5*base_unit,elli_1_y0+p_amp_1*r_max);
    
    gc->DrawText("P",elli_1_x0-5*base_unit,elli_1_y0);
    
    gc->SetPen(*field_pen);
    
    double x=elli_1_x0+r_max*s_amp_1*std::cos(-2.0*Pi*t/period-s_phase_1);
    double y=elli_1_y0-r_max*p_amp_1*std::cos(-2.0*Pi*t/period-p_phase_1);
    
    gc->StrokeLine(elli_1_x0,elli_1_y0,x,y);
    
    gc->StrokeLine(elli_1_x0,elli_1_y0+4.5*base_unit,
                   x,elli_1_y0+4.5*base_unit);
    
    gc->StrokeLine(elli_1_x0-4.5*base_unit,elli_1_y0,
                   elli_1_x0-4.5*base_unit,y);
    
    gc->SetPen(*wxBLACK_PEN);
    
    for(int i=0;i<360;i++)
    {
        gc->StrokeLine(elli_2_x[i],elli_2_y[i],
                       elli_2_x[i+1],elli_2_y[i+1]);
    }
    
    gc->StrokeLine(elli_2_x0-s_amp_2*r_max,elli_2_y0+4.5*base_unit,
                   elli_2_x0+s_amp_2*r_max,elli_2_y0+4.5*base_unit);
    
    gc->DrawText("S",elli_2_x0,elli_2_y0+5*base_unit);
    
    gc->StrokeLine(elli_2_x0-4.5*base_unit,elli_2_y0-p_amp_2*r_max,
                   elli_2_x0-4.5*base_unit,elli_2_y0+p_amp_2*r_max);
    
    gc->DrawText("P",elli_2_x0-5*base_unit,elli_2_y0);
    
    gc->SetPen(*field_pen);
    
    x=elli_2_x0+r_max*s_amp_2*std::cos(-2.0*Pi*t/period-s_phase_2);
    y=elli_2_y0-r_max*p_amp_2*std::cos(-2.0*Pi*t/period-p_phase_2);
    
    gc->StrokeLine(elli_2_x0,elli_2_y0,x,y);
    
    gc->StrokeLine(elli_2_x0,elli_2_y0+4.5*base_unit,
                   x,elli_2_y0+4.5*base_unit);
    
    gc->StrokeLine(elli_2_x0-4.5*base_unit,elli_2_y0,
                   elli_2_x0-4.5*base_unit,y);
}

void ElliGraph::on_paint(wxPaintEvent &event)
{
    rescale();
    wxBufferedPaintDC dc(this);
    dc.Clear();
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void ElliGraph::on_resize(wxSizeEvent &event)
{
    rescale();
    wxClientDC dc(this);
    dc.Clear();
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void ElliGraph::replot(wxGraphicsContext *gc)
{
    gc->SetFont(*wxNORMAL_FONT,wxColour(0,0,0));
        
    draw_data(gc);
}

void ElliGraph::recomp_ellipses()
{
    for(int i=0;i<=360;i++)
    {
        double ang=-2.0*Pi*i/360.0;
        
        elli_1_x[i]=elli_1_x0+r_max*s_amp_1*std::cos(-ang-s_phase_1);
        elli_1_y[i]=elli_1_y0-r_max*p_amp_1*std::cos(-ang-p_phase_1);
        
        elli_2_x[i]=elli_2_x0+r_max*s_amp_2*std::cos(-ang-s_phase_2);
        elli_2_y[i]=elli_2_y0-r_max*p_amp_2*std::cos(-ang-p_phase_2);
    }
}

void ElliGraph::rescale()
{
    GetSize(&sx,&sy);
    
    base_unit=std::min(sx/2.0,sy+0.0)/10.0;
    
    r_max=3.5*base_unit;
    
    elli_1_x0=sx/4.0+0.5*base_unit;
    elli_1_y0=sy/2.0-0.5*base_unit;
    elli_2_x0=3.0*sx/4.0+0.5*base_unit;
    elli_2_y0=sy/2.0-0.5*base_unit;
        
    recomp_ellipses();
}

void ElliGraph::set_fields(double p_amp_1_,double p_phase_1_,
                           double s_amp_1_,double s_phase_1_,
                           double p_amp_2_,double p_phase_2_,
                           double s_amp_2_,double s_phase_2_)
{
    p_amp_1=p_amp_1_; p_phase_1=p_phase_1_;
    s_amp_1=s_amp_1_; s_phase_1=s_phase_1_;
    
    p_amp_2=p_amp_2_; p_phase_2=p_phase_2_;
    s_amp_2=s_amp_2_; s_phase_2=s_phase_2_;
    
    recomp_ellipses();
}

void ElliGraph::set_time(double t_,double period_)
{
    t=t_;
    period=period_;
}

//############

ElliFrame::ElliFrame(wxString const &title)
    :BaseFrame(title),
     paused(false)
{
    wxBoxSizer *compo_sizer=new wxBoxSizer(wxVERTICAL),
               *ctrl_sizer=new wxBoxSizer(wxVERTICAL),
               *ctrl_sizer0=new wxBoxSizer(wxHORIZONTAL),
               *ctrl_sizer1=new wxBoxSizer(wxVERTICAL),
               *fresnel_sizer=new wxBoxSizer(wxHORIZONTAL),
               *k_sizer=new wxBoxSizer(wxHORIZONTAL),
               *n_sizer=new wxBoxSizer(wxHORIZONTAL),
               *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    draw_panel=new ElliGraph(this);
    
    wxPanel *ctrl_panel=new wxPanel(this);
    
    wxStaticBoxSizer *p_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"P component");
    wxStaticBoxSizer *s_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"S component");
    wxStaticBoxSizer *m_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Misc");
    
    //Compo
    
    p_amp_sld=new SldCombi(ctrl_panel,"Amplitude:",1000,0,1000,1e-3);
    p_phase_sld=new SldCombi(ctrl_panel,"Phase:",0,-180,180);
    s_amp_sld=new SldCombi(ctrl_panel,"Amplitude:",1000,0,1000,1e-3);
    s_phase_sld=new SldCombi(ctrl_panel,"Phase:",0,-180,180);
    angle_sld=new SldCombi(ctrl_panel,"Angle:",0,0,900,0.1);
    wxButton *pause_btn=new wxButton(ctrl_panel,wxID_ANY,"Pause");
    
    p_sizer->Add(p_amp_sld,wxSizerFlags(1).Expand());
    p_sizer->Add(p_phase_sld,wxSizerFlags(1).Expand());
    s_sizer->Add(s_amp_sld,wxSizerFlags(1).Expand());
    s_sizer->Add(s_phase_sld,wxSizerFlags(1).Expand());
    
    compo_sizer->Add(p_sizer,wxSizerFlags(1).Expand());
    compo_sizer->Add(s_sizer,wxSizerFlags(1).Expand());
    
    // Misc
    
    wxStaticText *n_txt=new wxStaticText(ctrl_panel,wxID_ANY,"n: ");
    n_ctrl=new wxTextCtrl(ctrl_panel,wxID_ANY,"1.5");
    wxStaticText *k_txt=new wxStaticText(ctrl_panel,wxID_ANY,"k: ");
    k_ctrl=new wxTextCtrl(ctrl_panel,wxID_ANY,"0");
    
    n_sizer->Add(n_txt);
    n_sizer->Add(n_ctrl,wxSizerFlags(1));
    k_sizer->Add(k_txt);
    k_sizer->Add(k_ctrl,wxSizerFlags(1));
    
    m_sizer->Add(n_sizer);
    m_sizer->Add(k_sizer);
    
    wxStaticText *period_txt=new wxStaticText(ctrl_panel,wxID_ANY,"Period:");
    period_ctrl=new wxTextCtrl(ctrl_panel,wxID_ANY,"10");
    
    m_sizer->Add(period_txt);
    m_sizer->Add(period_ctrl,wxSizerFlags().Expand());
    
    //ctrl_sizer0
    
    ctrl_sizer0->Add(compo_sizer,wxSizerFlags(1).Expand());
    ctrl_sizer0->Add(m_sizer,wxSizerFlags().Expand());
    ctrl_sizer0->Add(pause_btn,wxSizerFlags().Expand());
    
    //fresnel
    
    rp_ctrl=new ImdoubleCtrl(ctrl_panel,"rp");
    rs_ctrl=new ImdoubleCtrl(ctrl_panel,"rs");
    
    fresnel_sizer->Add(rp_ctrl,wxSizerFlags(1).Expand());
    fresnel_sizer->Add(rs_ctrl,wxSizerFlags(1).Expand());
    
    //ctrl_sizer1
    
    ctrl_sizer1->Add(angle_sld,wxSizerFlags(1).Expand());
    ctrl_sizer1->Add(fresnel_sizer,wxSizerFlags().Expand());
    
    //ctrl_sizer
    
    ctrl_sizer->Add(ctrl_sizer0,wxSizerFlags().Expand());
    ctrl_sizer->Add(ctrl_sizer1,wxSizerFlags().Expand());
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    top_sizer->Add(draw_panel,wxSizerFlags(1).Expand());
    top_sizer->Add(ctrl_panel,wxSizerFlags().Expand());
    
    SetSizer(top_sizer);
    
    this->Show(true);
    
    Maximize();
    
    timer=new wxTimer(this);
    watch=new wxStopWatch();
    
    Bind(wxEVT_TIMER,&ElliFrame::timed_refresh,this);
    Bind(wxEVT_BUTTON,&ElliFrame::pause_toggle,this);
    
    watch->Start(0);
    timer->Start(25.0/1000.0);
}

ElliFrame::~ElliFrame()
{
    timer->Stop();
    std::cout<<"Destructing ElliFrame"<<std::endl;
}

void ElliFrame::pause_toggle(wxCommandEvent &event)
{
    if(!paused)
    {
        watch->Pause();
        paused=true;
    }
    else
    {
        watch->Resume();
        paused=false;
    }
}

void ElliFrame::timed_refresh(wxTimerEvent &event)
{
    double t=watch->Time()/1000.0;
    double period=0.1;
    std::stringstream strm;
    strm<<period_ctrl->GetValue();
    strm>>period;
    
    double n=1.0,k=0.0;
    std::stringstream strm_n,strm_k;
    strm_n<<n_ctrl->GetValue();
    strm_k<<k_ctrl->GetValue();
    strm_n>>n;
    strm_k>>k;
    
//    double angle=0;
//    std::stringstream strm_a;
//    strm_a<<angle_ctrl->GetValue();
//    strm_a>>angle;
//    angle=angle_sld->get_value();
    
    p_amp=p_amp_sld->get_value();
    p_phase=p_phase_sld->get_value()*Pi/180.0;
    s_amp=s_amp_sld->get_value();
    s_phase=s_phase_sld->get_value()*Pi/180.0;
    
//    s_phase+=Pi;
    
    Imdouble P1=p_amp*std::exp(-p_phase*Im),
             S1=s_amp*std::exp(-s_phase*Im);
    
    Imdouble n1=1.0;
    Imdouble n2=n+k*Im;
    
    Imdouble r_te,r_tm,t_te,t_tm;
    
    fresnel_rt_te_tm(Degree(angle_sld->get_value()),
                     n1,n2,r_te,r_tm,t_te,t_tm);
    
//    double thi=angle*Pi/180.0;
//    double cos_thi=std::cos(thi);
//    Imdouble thr=std::asin(n1*std::sin(thi)/n2);
//    Imdouble cos_thr=std::cos(thr);
//    
//    Imdouble rs=(n1*cos_thi-n2*cos_thr)/(n1*cos_thi+n2*cos_thr);
//    Imdouble rp=(n2*cos_thi-n1*cos_thr)/(n2*cos_thi+n1*cos_thr);
    
    Imdouble rs=r_te;
    Imdouble rp=r_tm;
    
    Imdouble P2=rp*P1;
    Imdouble S2=rs*S1;
    
    rp_ctrl->set_amp(std::abs(rp));
    rp_ctrl->set_phase(std::arg(rp));
    rs_ctrl->set_amp(std::abs(rs));
    rs_ctrl->set_phase(std::arg(rs));
    
    draw_panel->set_fields(p_amp,p_phase,s_amp,s_phase,
                           std::abs(P2),-std::arg(P2),
                           std::abs(S2),-std::arg(S2));
    draw_panel->set_time(t,period);
    draw_panel->Refresh();
}
