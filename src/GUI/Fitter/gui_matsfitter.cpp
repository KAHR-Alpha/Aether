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
#include <phys_tools.h>
#include <gui_matsfitter.h>

extern const Imdouble Im;

//####################
//     MatsFitter
//####################

MatsFitter::MatsFitter(wxString const &title)
    :BaseFrame(title),
     optim_toggle(false)
{
    wxPanel *base_panel=new wxPanel(this);
    
    wxBoxSizer *a0_sizer=new wxBoxSizer(wxHORIZONTAL); // Topmost sizer
    
    wxBoxSizer *controls_sizer_0=new wxBoxSizer(wxVERTICAL); //Left side
    wxBoxSizer *graphs_sizer_0=new wxBoxSizer(wxVERTICAL); //Right side
    
    // Data Controls
    
    wxStaticBoxSizer *data_sizer_0=new wxStaticBoxSizer(wxVERTICAL,base_panel,"Data");
    wxBoxSizer *data_sizer_00=new wxBoxSizer(wxHORIZONTAL);
    
    data_type_choice=new wxChoice(base_panel,wxID_ANY);
    data_type_choice->Append("nm n k");
    data_type_choice->Append("m n k");
    data_type_choice->Append("Hz n k");
    data_type_choice->Append("eV n k");
    data_type_choice->SetSelection(1);
    
    data_load_btn=new wxButton(base_panel,wxID_ANY,"Load data");
    data_load_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,&MatsFitter::get_data_fname,this);
    
    data_sizer_00->Add(data_load_btn,1);
    data_sizer_00->Add(data_type_choice,1);
    data_type_choice->Disable();
    
    data_name_disp=new wxTextCtrl(base_panel,wxID_ANY);
    data_name_disp->SetEditable(false);
    data_name_disp->SetValue("");
    
    data_sizer_0->Add(data_sizer_00);
    data_sizer_0->Add(data_name_disp,0,wxEXPAND);
    
    controls_sizer_0->Add(data_sizer_0,0,wxEXPAND);
    
    // Remaining Controls
    
    wxStaticBoxSizer *display_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,"Display");
    
    disp_real_box=new wxCheckBox(base_panel,wxID_ANY,"Real part");
    disp_imag_box=new wxCheckBox(base_panel,wxID_ANY,"Imaginary part");
    disp_real_box->SetValue(true);
    disp_imag_box->SetValue(true);
    
    display_sizer->Add(disp_real_box);
    display_sizer->Add(disp_imag_box);
    
    wxStaticBoxSizer *optim_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,"Optimization");
    
//    wxCheckBox *optim_rt=new wxCheckBox(base_panel,wxID_ANY,"Real time");
    optim_button=new wxToggleButton(base_panel,wxID_ANY,"Optimize");
//    optim_rt->Disable();
    optim_button->Disable();
    
//    optim_sizer->Add(optim_rt);
    optim_sizer->Add(optim_button,wxSizerFlags().Expand());
    
    // Models panel
    
    mats_panel=new MatsPanel(base_panel);
    
    // Optimization
    
    controls_sizer_0->Add(display_sizer,wxSizerFlags().Expand().Border(wxALL,1));
    controls_sizer_0->Add(optim_sizer,wxSizerFlags().Expand().Border(wxALL,1));
    controls_sizer_0->Add(mats_panel,wxSizerFlags(1).Expand());
    
    // Graphs
    fitter=new GraphMatsFitter(base_panel);
    error_graph=new Graph(base_panel);
    
    graphs_sizer_0->Add(fitter,wxSizerFlags(2).Expand().Border(wxALL,1));
    graphs_sizer_0->Add(error_graph,wxSizerFlags(1).Expand().Border(wxALL,1));
    
    a0_sizer->Add(graphs_sizer_0,4,wxEXPAND|wxALL);
    a0_sizer->Add(controls_sizer_0,1,wxEXPAND|wxALL);
    
    base_panel->SetSizer(a0_sizer);
    
    // Events
    
    Bind(EVT_MODEL_NEW,&MatsFitter::new_fitter,this);
    Bind(EVT_MODEL_REMOVE,&MatsFitter::delete_fitter,this);
    Bind(EVT_MODEL_GRAPH_REFRESH,&MatsFitter::needs_refresh,this);
    Bind(EVT_MODEL_RECOMP,&MatsFitter::top_computations,this);
    optim_button->Bind(wxEVT_TOGGLEBUTTON,&MatsFitter::optimize,this);
    
    Show(true);
    
    Maximize();
}

void MatsFitter::delete_fitter(wxCommandEvent &event)
{
    ModelPanel *panel=reinterpret_cast<ModelPanel*>(event.GetClientData());
    
    //Removing the handle on the graph
    fitter->delete_widget(panel);
    
    wxCommandEvent recomp_event(EVT_MODEL_RECOMP);
    wxPostEvent(this,recomp_event);
}

void MatsFitter::delete_all_fitters()
{
    mats_panel->delete_all_fitters();
    fitter->delete_all_widgets();
}

void MatsFitter::get_data_fname(wxCommandEvent &event)
{
    if(mats_panel->model_panels.size()>0)
    {
        delete_all_fitters();
        error_graph->clear_graph();
    }
    
    std::string data_tmp;
    data_tmp=wxFileSelector("Select the data file");
    if(data_tmp.size()==0) return;
    
    data_fname=data_tmp;
    
    Plog::print(data_fname, "\n");
    data_name_disp->SetValue(data_fname);
    
    N_data=fcountlines(data_fname);
    Plog::print(N_data, "\n");
    
    w_base.resize(N_data,0);
    real_base.resize(N_data,0);
    imag_base.resize(N_data,0);
    
    real_disp.resize(N_data,0);
    imag_disp.resize(N_data,0);
    
    real_error.resize(N_data,0);
    imag_error.resize(N_data,0);
    
    std::ifstream file(data_fname,std::ios::in);
    
    double tmp1,tmp2,tmp3;
    Imdouble n_index,tmp_eps;
    
    for(int l=0;l<N_data;l++)
    {
        file>>tmp1;
        file>>tmp2;
        file>>tmp3;
        
        n_index=tmp2+tmp3*Im;
        tmp_eps=n_index*n_index;
        
        w_base[l]=m_to_rad_Hz(tmp1);
        real_base[l]=tmp_eps.real();
        imag_base[l]=tmp_eps.imag();
        real_disp[l]=real_base[l];
        imag_disp[l]=imag_base[l];
    }
    
    fitter->set_base_data(&w_base,&real_disp,&imag_disp);
    error_graph->add_external_data(&w_base,&real_error,0,0,1.0);
    error_graph->add_external_data(&w_base,&imag_error,1.0,0,0);
    
    double w1=std::min(w_base[0],w_base[N_data-1]);
    double w2=std::max(w_base[0],w_base[N_data-1]);
    
    mats_panel->set_spectrum(100,w1,w2);
    mats_panel->add_dielec_model(FIT_CONST);
    mats_panel->add_dielec_button->Enable();
    optim_button->Enable();
    
    error_graph->set_scale(w1-0.05*(w2-w1),w2+0.05*(w2-w1),-0.1,1.5);
}

void MatsFitter::needs_refresh(wxCommandEvent &event)
{
    fitter->Refresh();
}

void MatsFitter::new_fitter(wxCommandEvent &event)
{
    ModelPanel *panel=reinterpret_cast<ModelPanel*>(event.GetClientData());
    
    fitter->add_widget(panel);
    fitter->Refresh();
}

void MatsFitter::optimize(wxCommandEvent &event)
{
    if(optim_toggle==false)
    {
        optim_toggle=true;
        
        optim_thread=new OptimThread;
        
        optim_thread->optim_toggle=&optim_toggle;
        optim_thread->w=&w_base;
        optim_thread->eps_real=&real_base;
        optim_thread->eps_imag=&imag_base;
        optim_thread->mats_panel=mats_panel;
        
        optim_thread->Run();
    }
    else optim_toggle=false;
}

void MatsFitter::top_computations(wxCommandEvent &event)
{
    Imdouble eps,eps_sub;
    
    for(int l=0;l<N_data;l++)
    {
        double w=w_base[l];
        
        real_disp[l]=real_error[l]=real_base[l];
        imag_disp[l]=imag_error[l]=imag_base[l];
        
        eps_sub=mats_panel->get_eps_sub(w);
        eps=mats_panel->get_eps(w);
        
        real_disp[l]-=eps_sub.real();
        imag_disp[l]-=eps_sub.imag();
        
        real_error[l]-=eps.real();
        imag_error[l]-=eps.imag();
        
        real_error[l]=std::abs(real_error[l]/(real_base[l]));
        imag_error[l]=std::abs(imag_error[l]/(imag_base[l]));
        
        if(real_error[l]>1e100) real_error[l]=-1;
        if(imag_error[l]>1e100) imag_error[l]=-1;
    }
    
    fitter->Refresh();
    error_graph->Refresh();
}

//####################
//    GraphMatsFitter
//####################

GraphMatsFitter::GraphMatsFitter(wxWindow *parent)
    :wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(100,100)),
     draw_grid(true), graph_drag(false),
     padxm(50), padxp(25),
     padym(25), padyp(25),
     xmin(-4), xmax(4),
     ymin(-4), ymax(4),
     right_zoom(false),
     right_zoom_xs(0), right_zoom_ys(0),
     N_data(0), draw_real(true), draw_imag(true),
     widget_drag(-1)
{
    SetBackgroundColour(wxColour(255,255,255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    GetSize(&sx,&sy);
    
    select_pen.SetColour(wxColour(80,80,230,180));
    select_brush.SetColour(wxColour(80,80,230,120));
    
    grid_pen.SetColour(wxColour(220,220,220,255));
    
    Bind(wxEVT_MOUSEWHEEL,&GraphMatsFitter::evt_mouse_zoom,this);
    Bind(wxEVT_KEY_DOWN,&GraphMatsFitter::evt_keyboard,this);
    Bind(wxEVT_PAINT,&GraphMatsFitter::on_paint,this);
    #ifndef GTK_RMV_RESIZE
    Bind(wxEVT_SIZE,&GraphMatsFitter::on_resize,this);
    #endif
    Bind(wxEVT_LEFT_DOWN,&GraphMatsFitter::evt_mouse_left_down,this);
    Bind(wxEVT_LEFT_UP,&GraphMatsFitter::evt_mouse_left_up,this);
    Bind(wxEVT_RIGHT_DOWN,&GraphMatsFitter::evt_mouse_right_down,this);
    Bind(wxEVT_LEAVE_WINDOW,&GraphMatsFitter::evt_mouse_hold_reset,this);
    Bind(wxEVT_MOTION,&GraphMatsFitter::evt_mouse_motion,this);
}

void GraphMatsFitter::add_widget(ModelPanel *panel)
{
    model_widgets.push_back(panel);
}

void GraphMatsFitter::autoscale()
{
    xmin=xmax=(*w_base)[0];
    ymin=ymax=(*real_disp)[0];
    
    for(int l=0;l<N_data;l++)
    {
        xmin=std::min(xmin,(*w_base)[l]);
        xmax=std::max(xmax,(*w_base)[l]);
        
        ymin=std::min(ymin,(*real_disp)[l]);
        ymin=std::min(ymin,(*imag_disp)[l]);
        
        ymax=std::max(ymax,(*real_disp)[l]);
        ymax=std::max(ymax,(*imag_disp)[l]);
    }
    
    double spanx=xmax-xmin;
    double spany=ymax-ymin;
    
    xmin-=0.05*spanx; xmax+=0.05*spanx;
    ymin-=0.05*spany; ymax+=0.05*spany;
}

int GraphMatsFitter::d2px(double x)
{
    return padxm+static_cast<int>((sx-padxm-padxp)*(x-xmin)/(xmax-xmin));
}

int GraphMatsFitter::d2py(double y)
{
    return sy-padym-static_cast<int>((sy-padym-padyp)*(y-ymin)/(ymax-ymin));
}

void GraphMatsFitter::delete_widget(ModelPanel *panel)
{
    #ifndef LINUX_ITERATOR_TWEAK
    std::vector<ModelPanel*>::const_iterator pos;
    #else
    std::vector<ModelPanel*>::iterator pos;
    #endif
    
    for(pos=model_widgets.begin();pos<model_widgets.end();pos++)
    {
        if((*pos)==panel)
        {
            model_widgets.erase(pos);
            
            break;
        }
    }
    Plog::print("size " ,model_widgets.size(), "\n");
    
    Refresh();
}

void GraphMatsFitter::delete_all_widgets()
{
    model_widgets.clear();
    
    Refresh();
}

void GraphMatsFitter::draw_border(wxGraphicsContext *gc)
{
    gc->SetPen(*wxBLACK_PEN);
    
    gc->DrawRectangle(padxm,padyp,sx-padxm-padxp,sy-padym-padyp);
    
    if(right_zoom)
    {
        gc->SetPen(select_pen);
        gc->SetBrush(select_brush);
        
        gc->DrawRectangle(std::min(right_zoom_xs,mouse_x),
                          std::min(right_zoom_ys,mouse_y),
                          std::abs(mouse_x-right_zoom_xs),
                          std::abs(mouse_y-right_zoom_ys));
    }
}

void GraphMatsFitter::draw_data(wxGraphicsContext *gc)
{
    gc->SetPen(*wxBLACK_PEN);
    gc->SetPen(*wxBLUE_PEN);
    
    for(int l=0;l<N_data-1;l++)
    {
        gc->StrokeLine(d2px((*w_base)[l]),d2py((*real_disp)[l]),
                       d2px((*w_base)[l+1]),d2py((*real_disp)[l+1]));
    }
    gc->SetPen(*wxRED_PEN);
    
    for(int l=0;l<N_data-1;l++)
    {
        gc->StrokeLine(d2px((*w_base)[l]),d2py((*imag_disp)[l]),
                       d2px((*w_base)[l+1]),d2py((*imag_disp)[l+1]));
    }
    
    for(unsigned int l=0;l<model_widgets.size();l++)
    {
        ModelPanel &model=*model_widgets[l];
        
        if(!model.hide)
        {
            int px=d2px(model_widgets[l]->x_center);
            int py=d2py(model_widgets[l]->y_center);
            
            if(model.type==FIT_CONST)
            {
                gc->SetPen(model.real_pen);
                gc->SetBrush(model.real_brush);
                
                gc->StrokeLine(px-10,py,px+10,py);
                gc->StrokeLine(px,py-10,px,py+10);
                
                gc->DrawEllipse(px-6,py-6,12,12);
            }
            else if(model.type==FIT_DRUDE)
            {
                gc->SetPen(model.real_pen);
                gc->SetBrush(model.real_brush);
                
                gc->DrawEllipse(px-6,py-6,12,12);
                
                gc->StrokeLine(px-10,py,px+10,py);
                gc->StrokeLine(px,py-10,px,py+10);
                
                px=d2px(model_widgets[l]->x_hdl_1);
                py=d2py(model_widgets[l]->y_hdl_1);
                
                gc->DrawEllipse(px-6,py-6,12,12);
            }
            else if(model.type==FIT_LORENTZ)
            {
                gc->SetPen(model.imag_pen);
                gc->SetBrush(model.imag_brush);
                
                gc->DrawEllipse(px-6,py-6,12,12);
                
                gc->StrokeLine(px-10,py,px+10,py);
                gc->StrokeLine(px,py-10,px,py+10);
                
                px=d2px(model_widgets[l]->x_hdl_1);
                py=d2py(model_widgets[l]->y_hdl_1);
                
                gc->DrawEllipse(px-6,py-6,12,12);
            }
            else if(model.type==FIT_CRITPOINTS)
            {
                gc->SetPen(model.real_pen);
                gc->SetBrush(model.real_brush);
                
                gc->DrawEllipse(px-6,py-6,12,12);
                
                gc->StrokeLine(px-10,py,px+10,py);
                gc->StrokeLine(px,py-10,px,py+10);
                
                px=d2px(model_widgets[l]->x_hdl_1);
                py=d2py(model_widgets[l]->y_hdl_1);
                
                gc->DrawEllipse(px-6,py-6,12,12);
                
                gc->SetPen(model.imag_pen);
                gc->SetBrush(model.imag_brush);
                
                px=d2px(model_widgets[l]->x_hdl_2);
                py=d2py(model_widgets[l]->y_hdl_2);
                
                gc->DrawEllipse(px-6,py-6,12,12);
            }
        }
    }
    
    for(unsigned int l=0;l<model_widgets.size();l++)
    {
        ModelPanel &model=*model_widgets[l];
        
        if(!model.hide)
        {
            gc->SetPen(model.real_pen);
            
            for(int i=0;i<model.N_data-1;i++)
            {
                gc->StrokeLine(d2px(model.w[i]),d2py(model.real_part[i]),
                               d2px(model.w[i+1]),d2py(model.real_part[i+1]));
            }
            
            gc->SetPen(model.imag_pen);
            
            if(model.type!=FIT_CONST)
            {
                for(int i=0;i<model.N_data-1;i++)
                {
                    gc->StrokeLine(d2px(model.w[i]),d2py(model.imag_part[i]),
                                   d2px(model.w[i+1]),d2py(model.imag_part[i+1]));
                }
            }
        }
    }
}

void GraphMatsFitter::draw_grid_coord(wxGraphicsContext *gc)
{
    // Stuff to determine the grid to draw
    
    int px,py;
    
    std::stringstream strm;
    
    double spanx=xmax-xmin;
    double spany=ymax-ymin;
    
    double logxd=std::log10(spanx);
    double logyd=std::log10(spany);
    
    int logx=static_cast<int>(std::floor(logxd));
    int logy=static_cast<int>(std::floor(logyd));
    
    if(logxd-logx<0.31) logx-=1;
    if(logyd-logy<0.31) logy-=1;
    
    double ldx=std::pow(10.0,logx);
    double ldy=std::pow(10.0,logy);
    
    int64_t bx=static_cast<int64_t>(xmin/ldx);
    int64_t by=static_cast<int64_t>(ymin/ldy);
    
    double dbx=bx*ldx;
    double dby=by*ldy;
    
    while(dbx<xmin) dbx+=ldx;
    while(dby<ymin) dby+=ldy;
    
    double dbx0=dbx;
    double dby0=dby;
    
    gc->SetPen(grid_pen);
    
    // Grid and Coordinates
    
    double sw,sh,sd,sext;
    
    dbx=dbx0;
    dby=dby0;
    
    while(dbx<=xmax)
    {
        px=d2px(dbx);
        if(draw_grid) gc->StrokeLine(px,padyp,px,sy-padym);
        
        strm.str(""); strm<<dbx;
        gc->GetTextExtent(strm.str(),&sw,&sh,&sd,&sext);
        gc->DrawText(strm.str(),px-sw/2.0,sy-padym+5);
        
        dbx+=ldx;
    }
    
    while(dby<=ymax)
    {
        py=d2py(dby);
        if(draw_grid) gc->StrokeLine(padxm,py,sx-padxp,py);
        
        strm.str(""); strm<<dby;
        gc->GetTextExtent(strm.str(),&sw,&sh,&sd,&sext);
        gc->DrawText(strm.str(),3,py-sh/2.0);
        
        dby+=ldy;
    }
}

void GraphMatsFitter::evt_keyboard(wxKeyEvent &event)
{
    if(right_zoom)
    {
        if(event.GetKeyCode()==WXK_ESCAPE)
        {
            right_zoom=false;
            Refresh();
        }
    }
    else
    {
        if(event.GetUnicodeKey()=='A')
        {
            if(N_data==0)
            {
                xmin=ymin=-4;
                xmax=ymax=4;
            }
            else autoscale();
            
            Refresh();
        }
        else if(event.GetUnicodeKey()=='G') 
        {
            draw_grid=!draw_grid;
            
            Refresh();
        }
    }
}

void GraphMatsFitter::evt_mouse_hold_reset(wxMouseEvent &event)
{
    graph_drag=false;
    
    widget_drag=-1;
    for(unsigned int l=0;l<model_widgets.size();l++)
        model_widgets[l]->reset_motion();
}

void GraphMatsFitter::evt_mouse_left_down(wxMouseEvent &event)
{
    double tol_x=std::abs(px2dx(7)-px2dx(1));
    double tol_y=std::abs(py2dy(7)-py2dy(1));
    
    for(unsigned int l=0;l<model_widgets.size();l++)
    {
        if(model_widgets[l]->is_selected(px2dx(mouse_x),tol_x,
                                         py2dy(mouse_y),tol_y))
        {
            widget_drag=l;
            break;
        }
    }
    
    if(widget_drag<0)
    {
        if(right_zoom) process_right_zoom(event);
        else
        {
            graph_drag=true;
        }
    }
    
    event.Skip();
}

void GraphMatsFitter::evt_mouse_left_up(wxMouseEvent &event)
{
    graph_drag=false;
    
    widget_drag=-1;
    for(unsigned int l=0;l<model_widgets.size();l++)
        model_widgets[l]->reset_motion();
}

void GraphMatsFitter::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(widget_drag>-1)
    {
        model_widgets[widget_drag]->move(px2dx(mouse_x),py2dy(mouse_y));
        
        Refresh();
    }
    else if(graph_drag)
    {
        double dx=(xmax-xmin)/(sx-padxm-padxp)*(mouse_x-mouse_xp);
        double dy=(ymax-ymin)/(sy-padym-padyp)*(mouse_y-mouse_yp);
        
        xmax-=dx; xmin-=dx;
        ymax+=dy; ymin+=dy;
        
        Refresh();
    }
    else if(right_zoom) Refresh();
    
    event.Skip();
}

void GraphMatsFitter::evt_mouse_right_down(wxMouseEvent &event)
{
    if(!graph_drag)
    {
        if(!right_zoom)
        {
            right_zoom=true;
            right_zoom_xs=event.GetX();
            right_zoom_ys=event.GetY();
        }
        else
        {
            process_right_zoom(event);
        }
    }
    
    event.Skip();
}

void GraphMatsFitter::evt_mouse_zoom(wxMouseEvent &event)
{
    int rot=event.GetWheelRotation();
    
    double c_x=px2dx(event.GetX());
    double c_y=py2dy(event.GetY());
    
    double xm=c_x-xmin;
    double xp=xmax-c_x;
    
    double ym=c_y-ymin;
    double yp=ymax-c_y;
    
    if(rot>=0)
    {
        xmin=c_x-0.95*xm; xmax=c_x+0.95*xp;
        ymin=c_y-0.95*ym; ymax=c_y+0.95*yp;
    }
    else
    {
        xmin=c_x-1.05*xm; xmax=c_x+1.05*xp;
        ymin=c_y-1.05*ym; ymax=c_y+1.05*yp;
    }
    
    Refresh();
}

void GraphMatsFitter::on_paint(wxPaintEvent &event)
{
    rescale();
    wxBufferedPaintDC dc(this);
    dc.Clear();
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void GraphMatsFitter::on_resize(wxSizeEvent &event)
{
    rescale();
    wxClientDC dc(this);
    dc.Clear();
    
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void GraphMatsFitter::process_right_zoom(wxMouseEvent &event)
{
    int i=event.GetX();
    int j=event.GetY();
    
    right_zoom=false;
    
    double xmin_b=px2dx(std::min(i,right_zoom_xs));
    double xmax_b=px2dx(std::max(i,right_zoom_xs));
    double ymin_b=py2dy(std::max(j,right_zoom_ys));
    double ymax_b=py2dy(std::min(j,right_zoom_ys));
    
    xmin=xmin_b; xmax=xmax_b;
    ymin=ymin_b; ymax=ymax_b;
    
    Refresh();
}

double GraphMatsFitter::px2dx(int i)
{
    return xmin+(xmax-xmin)*(i-padxm)/(sx-padxm-padxp+0.0);
}

double GraphMatsFitter::py2dy(int j)
{
    return ymax+(ymin-ymax)*(j-padyp)/(sy-padym-padyp+0.0);
}

void GraphMatsFitter::replot(wxGraphicsContext *gc)
{
    gc->SetFont(*wxNORMAL_FONT,wxColour(0,0,0));
    gc->SetBrush(wxNullBrush);
    
    draw_grid_coord(gc);
    
    draw_data(gc);
    
    gc->SetBrush(wxNullBrush);
    draw_border(gc);
}

void GraphMatsFitter::rescale()
{
    GetSize(&sx,&sy);
}

void GraphMatsFitter::set_base_data(std::vector<double> *w_base_i,
                                std::vector<double> *real_disp_i,
                                std::vector<double> *imag_disp_i)
{
    w_base=w_base_i;
    real_disp=real_disp_i;
    imag_disp=imag_disp_i;
    
    N_data=w_base->size();
    
    autoscale();
    Refresh();
}
