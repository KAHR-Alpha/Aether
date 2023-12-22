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

#include <gui_matspanel.h>
#include <gui_matsfitter.h>

extern const Imdouble Im;

wxDEFINE_EVENT(EVT_MODEL_NEW,wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_REMOVE,wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_RECOMP,wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_GRAPH_REFRESH,wxCommandEvent);

//####################
//   Dielec_dialog
//####################

Dielec_dialog::Dielec_dialog()
    :wxDialog(0,wxID_ANY,"Select a dielectric model"),
     selection(-1)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *model_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    dielec_type=new wxChoice(this,wxID_ANY);
    dielec_type->Append("Drude");
    dielec_type->Append("Lorentz");
    dielec_type->Append("Critical Points");
    
    wxButton *ok_button=new wxButton(this,wxID_ANY,"OK");
    wxButton *cancel_button=new wxButton(this,wxID_ANY,"Cancel");
    
    buttons_sizer->Add(ok_button,wxSizerFlags().Border(wxALL,2));
    buttons_sizer->Add(cancel_button,wxSizerFlags().Border(wxALL,2));
    
    empty_formula.AddHandler(new wxPNGHandler);
    drude_formula.AddHandler(new wxPNGHandler);
    lorentz_formula.AddHandler(new wxPNGHandler);
    critpoints_formula.AddHandler(new wxPNGHandler);
    
    empty_formula.LoadFile(PathManager::locate_resource("resources/empty_formula_transp.png").generic_string(),wxBITMAP_TYPE_PNG);
    drude_formula.LoadFile(PathManager::locate_resource("resources/drude_formula_transp.png").generic_string(),wxBITMAP_TYPE_PNG);
    lorentz_formula.LoadFile(PathManager::locate_resource("resources/lorentz_formula_transp.png").generic_string(),wxBITMAP_TYPE_PNG);
    critpoints_formula.LoadFile(PathManager::locate_resource("resources/critpoints_formula_transp.png").generic_string(),wxBITMAP_TYPE_PNG);
    
    empty_formula.Rescale(empty_formula.GetWidth()/3,
                          empty_formula.GetHeight()/3,
                          wxIMAGE_QUALITY_HIGH);
    
    drude_formula.Rescale(drude_formula.GetWidth()/3,
                          drude_formula.GetHeight()/3,
                          wxIMAGE_QUALITY_HIGH);
    
    lorentz_formula.Rescale(lorentz_formula.GetWidth()/3,
                            lorentz_formula.GetHeight()/3,
                            wxIMAGE_QUALITY_HIGH);
    
    critpoints_formula.Rescale(critpoints_formula.GetWidth()/3,
                               critpoints_formula.GetHeight()/3,
                               wxIMAGE_QUALITY_HIGH);
    
    formula_img=new wxGenericStaticBitmap(this,wxID_ANY,wxBitmap(empty_formula));
    
    wxStaticText *dielec_type_text=new wxStaticText(this,wxID_ANY,"Dielectric model: ");
    
    model_sizer->Add(dielec_type_text,wxSizerFlags().Top().Border(wxALL,2).Center());
    model_sizer->Add(dielec_type,wxSizerFlags().Top().Border(wxALL,2));
    
    wxStaticLine *sep1=new wxStaticLine(this,wxID_ANY);
    wxStaticLine *sep2=new wxStaticLine(this,wxID_ANY);
    
    top_sizer->Add(model_sizer);
    top_sizer->Add(sep1,wxSizerFlags().Expand());
    top_sizer->Add(formula_img,wxSizerFlags().Border(wxALL,2));
    top_sizer->Add(sep2,wxSizerFlags().Expand());
    top_sizer->Add(buttons_sizer,wxSizerFlags().Align(wxALIGN_RIGHT));
    
    SetSizerAndFit(top_sizer);
    
    ok_button->Bind(wxEVT_BUTTON,&Dielec_dialog::confirm_selection,this);
    cancel_button->Bind(wxEVT_BUTTON,&Dielec_dialog::close_dialog,this);
    dielec_type->Bind(wxEVT_CHOICE,&Dielec_dialog::select_model,this);
    
    ShowModal();
}

void Dielec_dialog::close_dialog(wxCommandEvent &event)
{
    Close();
}

void Dielec_dialog::confirm_selection(wxCommandEvent &event)
{
    selection=dielec_type->GetSelection();
    
    Close();
}

void Dielec_dialog::select_model(wxCommandEvent &event)
{
    if(event.GetInt()==0)
        formula_img->SetBitmap(wxBitmap(drude_formula));
    else if(event.GetInt()==1)
        formula_img->SetBitmap(wxBitmap(lorentz_formula));
    else if(event.GetInt()==2)
        formula_img->SetBitmap(wxBitmap(critpoints_formula));
    
    event.Skip();
}

//####################
//    ModelPanel
//####################

ModelPanel::ModelPanel(wxWindow *parent,int type_i,bool ctrl_only)
    :wxPanel(parent),
     type(type_i),
     hide(false), substract(false),
//     eps_inf(1.0),
//     wd(0), g(0),
//     A(0), O(0), G(0), P(0),
     move_center(false),
     move_hdl_1(false),
     move_hdl_2(false),
     move_hdl_3(false)
{
    std::string k;
    
    double rd=randp();
    double gd=randp();
    double bd=randp();
    
    unsigned char rc=100+156*rd*rd*rd;
    unsigned char gc=100+156*gd*gd*gd;
    unsigned char bc=100+156*bd*bd*bd;
    
    real_col.Set(rc,gc,bc);
    imag_col.Set(rc*0.8,gc*0.8,bc*0.8);
    
    real_pen.SetColour(real_col);
    imag_pen.SetColour(imag_col);
    real_brush.SetColour(real_col);
    imag_brush.SetColour(imag_col);
    
    if(type==FIT_CONST) k="Epsilon infinite";
    else if(type==FIT_DRUDE) k="Drude";
    else if(type==FIT_LORENTZ) k="Lorentz";
    else if(type==FIT_CRITPOINTS) k="Critical Points";
    
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,this,k);
    wxButton *kill_btn=0;
    if(type!=FIT_CONST)
    {
        kill_btn=new wxButton(this,wxID_ANY,"Kill");
        kill_btn->ToggleWindowStyle(wxBU_EXACTFIT);
    }
    wxCheckBox *subs_box=new wxCheckBox(this,wxID_ANY,"Substract");
    wxCheckBox *hide_box=new wxCheckBox(this,wxID_ANY,"Hide");
    
    real_col_btn=new wxButton(this,wxID_ANY," ");
    real_col_btn->SetBackgroundColour(real_col);
    
    if(type!=FIT_CONST)
    {
        imag_col_btn=new wxButton(this,wxID_ANY," ");
        imag_col_btn->SetBackgroundColour(imag_col);
    }
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *color_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    header_sizer->Add(subs_box,wxSizerFlags().Align(wxALIGN_LEFT));
    header_sizer->Add(hide_box,wxSizerFlags().Align(wxALIGN_LEFT));
    if(type!=FIT_CONST) header_sizer->Add(kill_btn,wxSizerFlags());
    
    color_sizer->Add(real_col_btn,wxSizerFlags(1));
    if(type!=FIT_CONST) color_sizer->Add(imag_col_btn,wxSizerFlags(1));
    
    sizer->Add(header_sizer,wxSizerFlags().Expand());
    sizer->Add(color_sizer,wxSizerFlags().Expand());
    
    wxSizerFlags flag_txt,flag_spin(1);
    
    flag_txt.Center().Border(wxALL,1);
    flag_spin.Border(wxALL,1);
    
    if(type==FIT_CONST)
    {
        eps_inf=new NamedTextCtrl<double>(this,"Eps_inf:",0);
        
        sizer->Add(eps_inf,wxSizerFlags().Expand());
                
        eps_inf->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
    }
    else if(type==FIT_DRUDE)
    {
        wd=new NamedTextCtrl<double>(this,"wd:",0);
        g=new NamedTextCtrl<double>(this,"g:",0);
        
        sizer->Add(wd,wxSizerFlags().Expand());
        sizer->Add(g,wxSizerFlags().Expand());
        
        wd->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
        g->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
    }
    else if(type==FIT_LORENTZ || type==FIT_CRITPOINTS)
    {
        A=new NamedTextCtrl<double>(this,"A:",0);
        O=new NamedTextCtrl<double>(this,"O:",0);
        G=new NamedTextCtrl<double>(this,"G:",0);
        
        sizer->Add(A,wxSizerFlags().Expand());
        sizer->Add(O,wxSizerFlags().Expand());
        sizer->Add(G,wxSizerFlags().Expand());
        
        A->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
        O->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
        G->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
                
        if(type==FIT_CRITPOINTS)
        {
            P=new NamedTextCtrl<double>(this,"P:",0);
            
            sizer->Add(P,wxSizerFlags().Expand());
            
            P->Bind(EVT_NAMEDTXTCTRL,&ModelPanel::update_widget,this);
        }
    }
    
    if(ctrl_only)
    {
        hide_box->Hide();
        subs_box->Hide();
        
        real_col_btn->Hide();
        if(type!=FIT_CONST) imag_col_btn->Hide();
    }
    
    this->SetSizer(sizer);
    
    hide_box->Bind(wxEVT_CHECKBOX,&ModelPanel::toggle_hide,this);
    subs_box->Bind(wxEVT_CHECKBOX,&ModelPanel::toggle_substract,this);
    if(type!=FIT_CONST)  kill_btn->Bind(wxEVT_BUTTON,&ModelPanel::apoptose,this);
    real_col_btn->Bind(wxEVT_BUTTON,&ModelPanel::change_real_col,this);
    if(type!=FIT_CONST) imag_col_btn->Bind(wxEVT_BUTTON,&ModelPanel::change_imag_col,this);
}

void ModelPanel::accept_mutation()
{
    *eps_inf=eps_inf_mut;
    *wd=wd_mut;
    *g=g_mut;
    *A=A_mut;
    *O=O_mut;
    *G=G_mut;
    *P=P_mut;
    
    if(type==FIT_DRUDE) drude_model.set(*wd,*g);
    else if(type==FIT_LORENTZ) lorentz_model.set(*A,*O,*G);
    else if(type==FIT_CRITPOINTS) critpoints_model.set(*A,*O,*P,*G);
}

void ModelPanel::apoptose(wxCommandEvent &event)
{
    std::cout<<"kill"<<std::endl;
    
    wxCommandEvent out_event(EVT_MODEL_REMOVE);
    out_event.SetClientData(reinterpret_cast<void*>(this));
    
    wxPostEvent(this,out_event);
    
    std::cout<<"kill end "<<this<<std::endl;
}

void ModelPanel::change_imag_col(wxCommandEvent &event)
{
    wxColourDialog dialog(this);
    dialog.ShowModal();
    
    wxColourData cdata=dialog.GetColourData();
    
    imag_col=cdata.GetColour();
    imag_pen.SetColour(imag_col);
    imag_brush.SetColour(imag_col);
    imag_col_btn->SetBackgroundColour(imag_col);
    
    wxCommandEvent test(EVT_MODEL_GRAPH_REFRESH);
    wxPostEvent(this,test);
}

void ModelPanel::change_real_col(wxCommandEvent &event)
{
    wxColourDialog dialog(this);
    dialog.ShowModal();
    
    wxColourData cdata=dialog.GetColourData();
    
    real_col=cdata.GetColour();
    real_pen.SetColour(real_col);
    real_brush.SetColour(real_col);
    real_col_btn->SetBackgroundColour(real_col);
    
    wxCommandEvent out_event(EVT_MODEL_GRAPH_REFRESH);
    wxPostEvent(this,out_event);
}

void ModelPanel::handles_to_model()
{
    if(type==FIT_CONST)
    {
        *eps_inf=y_center;
    }
    else if(type==FIT_DRUDE)
    {
        double w1,w2,f1,f2,r;
        
        if(x_center<x_hdl_1)
        {
            w1=x_center; w2=x_hdl_1;
            f1=y_center; f2=y_hdl_1;
        }
        else
        {
            w2=x_center; w1=x_hdl_1;
            f2=y_center; f1=y_hdl_1;
        }
        
        r=f2/f1;
        
        double sr=(w1*w1-r*w2*w2)/(r-1.0);
        if(sr>0) *g=std::sqrt(sr);
        else *g=0;
        
        *wd=std::sqrt(-f1*(w1*w1+(*g)*(*g)));
    }
    else if(type==FIT_LORENTZ)
    {
        double f1,f2,alp;
        
        *O=x_center;
        f1=y_center;
        
        alp=x_hdl_1/(*O);
        f2=y_hdl_1;
        
        double sr=f2*(*O)*(*O)*(1.0-alp*alp)*(1.0-alp*alp)/(alp*f1-alp*alp*f2);
        
        if(sr>0) *G=std::sqrt(sr);
        else *G=1.0;
        
        *A=(*G)*f1/(*O);
    }
    else if(type==FIT_CRITPOINTS)
    {
        double Od=*O;
        double Gd=*G;
        double Pd=*P;
        
        double r;
        Imdouble z1,z2,rat;
        
        *O=x_center;
        *G=std::abs(x_hdl_1-x_center);
        
        using std::conj;
        
        z1=1.0/(Od-x_hdl_1-Gd*Im);
        z2=1.0/(Od+x_hdl_1+Gd*Im);
        
        r=y_hdl_1/y_hdl_2;
        
        rat=-((z2+conj(z1))*Im-r*(z2-conj(z1)))/((z1+conj(z2))*Im-r*(z1-conj(z2)));
        
        *P=std::arg(rat)/2.0;
        
        Imdouble A_chk=2.0*y_hdl_1/Od/(z1*std::exp(Pd*Im)+z2*std::exp(-Pd*Im)+
                                      conj(z1*std::exp(Pd*Im)+z2*std::exp(-Pd*Im)));
        
        *A=std::real(A_chk);
    }
}

bool ModelPanel::is_selected(double x,double tol_x,double y,double tol_y)
{
    using std::abs;
    
    if(abs(x-x_center)<tol_x && abs(y-y_center)<tol_y)
    {
        move_center=true;
        return true;
    }
    else if(abs(x-x_hdl_1)<tol_x && abs(y-y_hdl_1)<tol_y)
    {
        move_hdl_1=true;
        return true;
    }
    else if(type==FIT_CRITPOINTS)
    {
        if(abs(x-x_hdl_2)<tol_x && abs(y-y_hdl_2)<tol_y)
        {
            move_hdl_2=true;
            return true;
        }
    }
    
    return false;
}

Imdouble ModelPanel::get_eps(double w)
{
    if(type==FIT_CONST) return static_cast<double>(*eps_inf);
    else if(type==FIT_DRUDE) return drude_model.eval(w);
    else if(type==FIT_LORENTZ) return lorentz_model.eval(w);
    else if(type==FIT_CRITPOINTS) return critpoints_model.eval(w);
    
    return 0;
}

Imdouble ModelPanel::get_eps_mut(double w)
{
    if(type==FIT_CONST) return eps_inf_mut;
    else if(type==FIT_DRUDE) return drude_model_mut.eval(w);
    else if(type==FIT_LORENTZ) return lorentz_model_mut.eval(w);
    else if(type==FIT_CRITPOINTS) return critpoints_model_mut.eval(w);
    
    return 0;
}

void ModelPanel::model_to_handles()
{
    if(type==FIT_CONST)
    {
        y_center=*eps_inf;
    }
    else if(type==FIT_DRUDE)
    {
        y_center=(drude_model.eval(x_center)).real();
        
        y_hdl_1=(drude_model.eval(x_hdl_1)).real();
    }
    else if(type==FIT_LORENTZ)
    {
        x_center=*O;
        y_center=(lorentz_model.eval(*O)).imag();
        
        y_hdl_1=(lorentz_model.eval(x_hdl_1)).imag();
    }
    else if(type==FIT_CRITPOINTS)
    {
        x_center=*O;
        
        x_hdl_1=x_hdl_2=static_cast<double>(*O)+static_cast<double>(*G);
        
        Imdouble eps=critpoints_model.eval(x_hdl_1);
        
        y_hdl_1=eps.real();
        y_hdl_2=eps.imag();
    }
}

void ModelPanel::move(double x,double y)
{
    if(move_center)
    {
        double dx=x-x_center;
        
        x_center=x;
        y_center=y;
        
        if(type==FIT_LORENTZ)
        {
            x_hdl_1+=dx;
        }
        else if(type==FIT_CRITPOINTS)
        {
            x_hdl_1+=dx;
            x_hdl_2+=dx;
        }
    }
    else if(move_hdl_1)
    {
        x_hdl_1=x;
        y_hdl_1=y;
        
        if(type==FIT_CRITPOINTS) x_hdl_2=x_hdl_1;
    }
    else if(move_hdl_2)
    {
        x_hdl_2=x;
        y_hdl_2=y;
        
        if(type==FIT_CRITPOINTS) x_hdl_1=x_hdl_2;
    }
    
    if(type==FIT_CONST)
    {
        if(y_center<1.0) y_center=1.0;
    }
    else if(type==FIT_LORENTZ)
    {
        if(y_center<0 && y_hdl_1>=0) y_hdl_1=-y_hdl_1;
        if(y_center>=0 && y_hdl_1<0) y_hdl_1=-y_hdl_1;
    }
    else if(type==FIT_CRITPOINTS)
    {
        if(x_hdl_1<x_center) x_hdl_1=x_center;
        if(x_hdl_2<x_center) x_hdl_2=x_center;
    }
    
    handles_to_model();
    update_panel();
    recompute();
}

void ModelPanel::mutate(double span)
{
    double p;
    
    if(type==FIT_CONST)
    {
        p=randp(-span,span);
        
        eps_inf_mut=static_cast<double>(*eps_inf)*(1.0+p);
        if(eps_inf_mut<1.0) eps_inf_mut=1.0;
    }
    else if(type==FIT_DRUDE)
    {
        p=randp(-span,span); wd_mut=static_cast<double>(*wd)*(1.0+p);
        p=randp(-span,span); g_mut=static_cast<double>(*g)*(1.0+p);
        
        drude_model_mut.set(wd_mut,g_mut);
    }
    else if(type==FIT_LORENTZ)
    {
        p=randp(-span,span); A_mut=static_cast<double>(*A)*(1.0+p);
        p=randp(-span,span); O_mut=static_cast<double>(*O)*(1.0+p);
        p=randp(-span,span); G_mut=static_cast<double>(*G)*(1.0+p);
        
        lorentz_model_mut.set(A_mut,O_mut,G_mut);
    }
    else if(type==FIT_CRITPOINTS)
    {
        p=randp(-span,span); A_mut=static_cast<double>(*A)*(1.0+p);
        p=randp(-span,span); O_mut=static_cast<double>(*O)*(1.0+p);
        p=randp(-span,span); G_mut=static_cast<double>(*G)*(1.0+p);
        p=randp(-span,span); P_mut=static_cast<double>(*P)+p;
        
        critpoints_model_mut.set(A_mut,O_mut,P_mut,G_mut);
    }
}

void ModelPanel::recompute()
{
    int l;
    
    if(type==FIT_CONST)
    {
        for(l=0;l<N_data;l++)
        {
            real_part[l]=static_cast<double>(*eps_inf);
            imag_part[l]=0;
        }
    }
    else if(type==FIT_DRUDE)
    {
        Imdouble eps;
        
        drude_model.set(*wd,*g);
        
        for(l=0;l<N_data;l++)
        {
            eps=drude_model.eval(w[l]);
            real_part[l]=eps.real();
            imag_part[l]=eps.imag();
        }
    }
    else if(type==FIT_LORENTZ)
    {
        Imdouble eps;
        
        lorentz_model.set(*A,*O,*G);
        
        for(l=0;l<N_data;l++)
        {
            eps=lorentz_model.eval(w[l]);
            real_part[l]=eps.real();
            imag_part[l]=eps.imag();
        }
    }
    else if(type==FIT_CRITPOINTS)
    {
        Imdouble eps;
        
        critpoints_model.set(*A,*O,*P,*G);
        
        for(l=0;l<N_data;l++)
        {
            eps=critpoints_model.eval(w[l]);
            real_part[l]=eps.real();
            imag_part[l]=eps.imag();
        }
    }
    
    wxCommandEvent event(EVT_MODEL_RECOMP);
    wxPostEvent(this,event);
}

void ModelPanel::reset_motion()
{
    move_center=false;
    move_hdl_1=false;
    move_hdl_2=false;
    move_hdl_3=false;
}

void ModelPanel::set_optimization_engine(OptimEngine *engine)
{
    OptimRule rule;
    
    rule.lock=false;
    rule.delta_add=0.05;
    rule.delta_grow=0.05;
    rule.limit_down=0;
    
    rule.limit_type=OptimRule::Limit::DOWN;
    rule.operation_type=OptimRule::Operation::GROW;
    
    if(type==FIT_CONST)
    {
        rule.limit_down=1.0;
        
        eps_inf->set_optimization_engine(engine,rule);
    }
    else if(type==FIT_DRUDE)
    {
        wd->set_optimization_engine(engine,rule);
        g->set_optimization_engine(engine,rule);
    }
    else if(type==FIT_LORENTZ)
    {
        A->set_optimization_engine(engine,rule);
        O->set_optimization_engine(engine,rule);
        G->set_optimization_engine(engine,rule);
    }
    else if(type==FIT_CRITPOINTS)
    {
        A->set_optimization_engine(engine,rule);
        O->set_optimization_engine(engine,rule);
        G->set_optimization_engine(engine,rule);
        
        rule.limit_type=OptimRule::Limit::NONE;
        rule.operation_type=OptimRule::Operation::ADD;
        P->set_optimization_engine(engine,rule);
    }
}

void ModelPanel::set_spectrum(int N_data_i,double w1,double w2)
{
    N_data=N_data_i;
    
    double w_span=std::abs(w2-w1);
    
    if(w1>w2) std::swap(w1,w2);
    
    double w_min=w1;
    double w_max=w2;
    
    w.resize(N_data,0);
    real_part.resize(N_data,0);
    imag_part.resize(N_data,0);
    
    for(int l=0;l<N_data;l++)
        w[l]=w_min+(w_max-w_min)*l/(N_data-1.0);
    
    if(type==FIT_CONST)
    {
        x_center=0.5*(w1+w2);
        *eps_inf=1.0;
        y_center=static_cast<double>(*eps_inf);
    }
    else if(type==FIT_DRUDE)
    {
        x_center=w1+0.1*w_span;
        y_center=-5;
        
        x_hdl_1=w2-0.1*w_span;
        y_hdl_1=-2;
    }
    else if(type==FIT_LORENTZ)
    {
        x_center=0.5*(w1+w2);
        y_center=10.0;
        
        x_hdl_1=x_center+0.2*w_span;
        y_hdl_1=3.0;
    }
    else if(type==FIT_CRITPOINTS)
    {
        x_center=0.5*(w1+w2);
        y_center=0;
        
        x_hdl_1=1.2*x_center;
        y_hdl_1=3.0;
        
        x_hdl_2=1.2*x_center;
        y_hdl_2=4.0;
    }
    
    handles_to_model();
    update_panel();
    recompute();
}

void ModelPanel::toggle_hide(wxCommandEvent &event)
{
    hide=event.GetInt();
    
    wxCommandEvent test(EVT_MODEL_GRAPH_REFRESH);
    wxPostEvent(this,test);
}

void ModelPanel::toggle_substract(wxCommandEvent &event)
{
    substract=event.GetInt();
    
    wxCommandEvent test(EVT_MODEL_RECOMP);
    wxPostEvent(this,test);
}

void ModelPanel::update_panel()
{
    if(type==FIT_CONST)
    {
        eps_inf->update_display();
    }
    else if(type==FIT_DRUDE)
    {
        wd->update_display();
        g->update_display();
    }
    else if(type==FIT_LORENTZ)
    {
        A->update_display();
        O->update_display();
        G->update_display();
    }
    else if(type==FIT_CRITPOINTS)
    {
        A->update_display();
        O->update_display();
        G->update_display();
        P->update_display();
    }
}

void ModelPanel::update_values()
{
    update_panel();
    
    recompute();
    model_to_handles();
}

void ModelPanel::update_widget(wxCommandEvent &event)
{    
    recompute();
    model_to_handles();
}

//###############
//   MatsPanel
//###############

MatsPanel::MatsPanel(wxWindow *parent,OptimEngine *engine)
    :wxPanel(parent),
     ctrl_only(false),
     Nl(100), w1(2.35e14), w2(4.7e14),
     optim_engine(engine)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    add_dielec_button=new wxButton(this,wxID_ANY,"Add dielectric model");
    add_dielec_button->Disable();
    
    mats_list=new wxScrolledWindow(this);
    mats_list->ToggleWindowStyle(wxSUNKEN_BORDER);
    
    mats_sizer=new wxBoxSizer(wxVERTICAL);
    
    mats_list->SetScrollbars(1, 1, 50, 50);
    
    mats_list->SetSizer(mats_sizer);
    mats_list->FitInside();
    mats_list->Layout();
    
    top_sizer->Add(add_dielec_button,wxSizerFlags().Expand().Border(wxALL,1));
    top_sizer->Add(mats_list,wxSizerFlags(1).Expand());
    
    SetSizer(top_sizer);
    
    // Events
    
    add_dielec_button->Bind(wxEVT_BUTTON,&MatsPanel::add_dielec_button_click,this);
    Bind(EVT_MODEL_REMOVE,&MatsPanel::delete_fitter,this);
}

void MatsPanel::add_dielec_button_click(wxCommandEvent &event)
{
    Dielec_dialog dialog{};
    
    int selection=dialog.selection;
    
    if(selection!=-1)
    {
             if(selection==0) add_dielec_model(FIT_DRUDE);
        else if(selection==1) add_dielec_model(FIT_LORENTZ);
        else if(selection==2) add_dielec_model(FIT_CRITPOINTS);
    }
}

void MatsPanel::add_dielec_model(int type)
{
    ModelPanel *panel=new ModelPanel(mats_list,type,ctrl_only);
    
    if(optim_engine!=nullptr) panel->set_optimization_engine(optim_engine);
    
    panel->set_spectrum(Nl,w1,w2);
    model_panels.push_back(panel);
    
    mats_sizer->Add(panel,wxSizerFlags().Expand().Border(wxALL,2));
    mats_list->FitInside();
    mats_list->Layout();
    
    wxCommandEvent event(EVT_MODEL_NEW);
    event.SetClientData(reinterpret_cast<void*>(panel));
    
    wxPostEvent(this,event);
}


void MatsPanel::delete_fitter(wxCommandEvent &event)
{
    ModelPanel *panel=reinterpret_cast<ModelPanel*>(event.GetClientData());
    
    //Removing the pointer to it
    #ifndef LINUX_ITERATOR_TWEAK
    std::vector<ModelPanel*>::const_iterator pos;
    #else
    std::vector<ModelPanel*>::iterator pos;
    #endif
    
    for(pos=model_panels.begin();pos<model_panels.end();pos++)
    {
        if((*pos)==panel)
        {
            model_panels.erase(pos);
            break;
        }
    }
    
    panel->Destroy();
    
    mats_list->FitInside();
    mats_list->Layout();
    
    event.Skip();
}

void MatsPanel::delete_all_fitters()
{
    for(unsigned int i=0;i<model_panels.size();i++)
        model_panels[i]->Destroy();
    
    model_panels.clear();
    
    mats_list->FitInside();
    mats_list->Layout();
}

Imdouble MatsPanel::get_eps(double w)
{
    Imdouble eps=0;
    
    for(unsigned int i=0;i<model_panels.size();i++)
        eps+=model_panels[i]->get_eps(w);
        
    return eps;
}

Imdouble MatsPanel::get_eps_mut(double w)
{
    Imdouble eps=0;
    
    for(unsigned int i=0;i<model_panels.size();i++)
        eps+=model_panels[i]->get_eps_mut(w);
        
    return eps;
}

Imdouble MatsPanel::get_eps_sub(double w)
{
    Imdouble eps=0;
    
    for(unsigned int i=0;i<model_panels.size();i++)
        if(model_panels[i]->substract)
            eps+=model_panels[i]->get_eps(w);
    
    return eps;
}

void MatsPanel::optimize(bool *optim_toggle,std::vector<double> *w,
                         std::vector<double> *eps_real,std::vector<double> *eps_imag)
{
    using std::abs;
    unsigned int i,k,l;
    
    Imdouble eps;
    double best_score=0;
    
    double span=0.05;
    int N_fail=0;
    
    for(l=0;l<w->size();l++)
    {
        eps=get_eps((*w)[l]);
        
        best_score+=abs(eps.real()-(*eps_real)[l])/abs((*eps_real)[l])+
                    abs(eps.imag()-(*eps_imag)[l])/abs((*eps_imag)[l]);
    }
            
    while(*optim_toggle)
    {
        for(k=0;k<1000;k++)
        {
            for(i=0;i<model_panels.size();i++)
            {
                model_panels[i]->mutate(span);
            }
            
            double tmp_score=0;
            
            for(l=0;l<w->size();l++)
            {
                eps=get_eps_mut((*w)[l]);
                
                tmp_score+=abs(eps.real()-(*eps_real)[l])/abs((*eps_real)[l])+
                           abs(eps.imag()-(*eps_imag)[l])/abs((*eps_imag)[l]);
                           
                if(eps.imag()<0) tmp_score+=1e4;
            }
            
            if(tmp_score>best_score)
            {
                N_fail+=1;
                
                if(N_fail>50)
                {
                    span*=0.9;
                    N_fail=0;
                }
            }
            else
            {
                N_fail=0;
                span+=0.05;
                
                best_score=tmp_score;
                for(i=0;i<model_panels.size();i++) model_panels[i]->accept_mutation();
            }
        }
        
        std::cout<<"Optim "<<best_score<<std::endl;
        
        for(i=0;i<model_panels.size();i++) model_panels[i]->update_panel();
        for(i=0;i<model_panels.size();i++) model_panels[i]->model_to_handles();
        for(i=0;i<model_panels.size();i++) model_panels[i]->recompute();
    }
    
    for(i=0;i<model_panels.size();i++) model_panels[i]->update_panel();
    for(i=0;i<model_panels.size();i++) model_panels[i]->model_to_handles();
    for(i=0;i<model_panels.size();i++) model_panels[i]->recompute();
}

void MatsPanel::set_spectrum(int Nl_i,double w1_i,double w2_i)
{
    Nl=Nl_i;
    w1=w1_i;
    w2=w2_i;
}

void MatsPanel::unlock_dielec_button()
{
    add_dielec_button->Enable();
}

void MatsPanel::update_values()
{
    for(ModelPanel *panel:model_panels)
        panel->update_values();
}
