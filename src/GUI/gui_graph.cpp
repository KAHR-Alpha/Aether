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

#include <spectral_color.h>

#include <gui.h>
#include <gui_graph.h>

#include <sstream>

extern std::ofstream plog;

enum
{
    MEM_NONE=0,
    MEM_ALL,
    MEM_SLIDER_SINGLE,
    MEM_SLIDER_ALL
};

//###################
//   BackDropPanel
//###################

BackdropPanel::BackdropPanel(wxWindow *parent,Graph *graph_,int ID_)
    :PanelsListBase(parent),
     ID(ID_), graph(graph_)
{
    up_btn->Hide();
    down_btn->Hide();
    
    set_title(graph->fname_self[ID]);
    
    wxBoxSizer *self_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    display=new wxCheckBox(this,wxID_ANY,"Display ");
    display->SetValue(graph->show_self[ID]);
    display->Bind(wxEVT_CHECKBOX,&BackdropPanel::evt_display,this);
    
    color_btn=new wxButton(this,wxID_ANY);
    color_btn->SetBackgroundColour(graph->pen_self[ID].GetColour());
    color_btn->Bind(wxEVT_BUTTON,&BackdropPanel::evt_color,this);
    
    legend=new wxTextCtrl(this,wxID_ANY,graph->legend_self[ID]);
    legend->Bind(wxEVT_TEXT,&BackdropPanel::evt_legend,this);
    
    self_sizer->Add(display);
    self_sizer->Add(color_btn);
    self_sizer->Add(legend);
    
    sizer->Add(self_sizer,wxSizerFlags().Expand());
}

void BackdropPanel::evt_color(wxCommandEvent &event)
{
    wxColourData data;
    
    data.SetColour(color_btn->GetBackgroundColour());
    
    wxColourDialog dialog(this,&data);
    dialog.ShowModal();
    
    wxColour tmp=dialog.GetColourData().GetColour();
    color_btn->SetBackgroundColour(tmp);
    graph->pen_self[ID].SetColour(tmp);
    
    graph->Refresh();
}

void BackdropPanel::evt_display(wxCommandEvent &event)
{
    graph->show_self[ID]=display->GetValue();
    graph->Refresh();
}

void BackdropPanel::evt_legend(wxCommandEvent &event)
{
    graph->legend_self[ID]=legend->GetValue().ToStdString();
}

//########################
//   GraphOptionsDialog
//########################

GraphOptionsDialog::GraphOptionsDialog(Graph *parent)
    :wxDialog(parent,wxID_ANY,"Graph Options"),
     graph(parent),
     N_data(graph->N_data),
     checkbox(N_data),
     color(N_data),
     legend(N_data)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxNotebook *book=new wxNotebook(this,wxID_ANY);
    
    // Curves
    
    wxPanel *curves_panel=new wxPanel(book);
    wxBoxSizer *curves_sizer=new wxBoxSizer(wxVERTICAL);
    curves_panel->SetSizer(curves_sizer);
    
    // - X Pointer
    
    pointer_x=new NamedTextCtrl<double>(curves_panel,"X: ",0);
    pointer_x->lock();
    
    curves_sizer->Add(pointer_x);
    
    // - Individual Curves
    
    pointer_y.resize(N_data);
    
    for(int i=0;i<N_data;i++)
    {
        wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *sub_sizer=new wxBoxSizer(wxVERTICAL);
        
        wxStaticText *text=new wxStaticText(curves_panel,wxID_ANY,std::to_string(i+1)+": ");
        
        checkbox[i]=new wxCheckBox(curves_panel,i,"");
        checkbox[i]->SetValue(graph->show_data_v[i]);
        
        checkbox[i]->Bind(wxEVT_CHECKBOX,&GraphOptionsDialog::evt_checkbox,this);
        
        color[i]=new wxButton(curves_panel,i," ",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
        color[i]->SetBackgroundColour(graph->data_pen[i].GetColour());
        
        color[i]->Bind(wxEVT_BUTTON,&GraphOptionsDialog::evt_color,this);
        
        legend[i]=new wxTextCtrl(curves_panel,i,graph->legend_str[i]);
        legend[i]->Bind(wxEVT_TEXT,&GraphOptionsDialog::evt_legend,this);
        
        pointer_y[i]=new NamedTextCtrl<double>(curves_panel,"Y: ",0);
        pointer_y[i]->lock();
        
//        sizer->Add(text,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
//        sizer->Add(checkbox[i],wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
//        sizer->Add(color[i],wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2).Expand());
//        
//        sub_sizer->Add(legend[i],wxSizerFlags().Expand());
//        sub_sizer->Add(pointer_y[i],wxSizerFlags().Expand());
//        sizer->Add(sub_sizer,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
        
        sizer->Add(text,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
        sizer->Add(checkbox[i],wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
        sizer->Add(color[i],wxSizerFlags().Border(wxALL,2).Expand());
        
        sub_sizer->Add(legend[i],wxSizerFlags().Expand());
        sub_sizer->Add(pointer_y[i],wxSizerFlags().Expand());
        sizer->Add(sub_sizer,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL).Border(wxALL,2));
        
        curves_sizer->Add(sizer,wxSizerFlags().Expand());
    }
    
    book->AddPage(curves_panel,"Curves");
    
    // Spectral Colors
    
    if(graph->spectral_data)
    {
        wxScrolledWindow *spcol_panel=new wxScrolledWindow(book);
        
        GraphOptionsDialog_SpColors(spcol_panel);
        
        spcol_panel->SetScrollRate(0,50);
        spcol_panel->FitInside();
        
        book->AddPage(spcol_panel,"Spectral Colors");
    }
    
    // Scale
    
    wxPanel *scale_panel=new wxPanel(book);
    wxBoxSizer *scale_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *x_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *y_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    x_min=new NamedTextCtrl<double>(scale_panel,"X Min",graph->xmin,true);
    x_max=new NamedTextCtrl<double>(scale_panel,"X Max",graph->xmax,true);
    
    x_min->Bind(EVT_NAMEDTXTCTRL,&GraphOptionsDialog::evt_scale,this);
    x_max->Bind(EVT_NAMEDTXTCTRL,&GraphOptionsDialog::evt_scale,this);
    
    y_min=new NamedTextCtrl<double>(scale_panel,"Y Min",graph->ymin,true);
    y_max=new NamedTextCtrl<double>(scale_panel,"Y Max",graph->ymax,true);
    
    y_min->Bind(EVT_NAMEDTXTCTRL,&GraphOptionsDialog::evt_scale,this);
    y_max->Bind(EVT_NAMEDTXTCTRL,&GraphOptionsDialog::evt_scale,this);
    
    lock_scale=new wxCheckBox(scale_panel,wxID_ANY,"Lock Scale");
    lock_scale->SetValue(graph->lock_scale);
    
    lock_scale->Bind(wxEVT_CHECKBOX,&GraphOptionsDialog::evt_scale,this);
    
    x_sizer->Add(x_min); x_sizer->Add(x_max);
    y_sizer->Add(y_min); y_sizer->Add(y_max);
    
    scale_sizer->Add(x_sizer);
    scale_sizer->Add(y_sizer);
    scale_sizer->Add(lock_scale,wxSizerFlags().Border(wxALL,2));
    
    scale_panel->SetSizer(scale_sizer);
    
    book->AddPage(scale_panel,"Scale");
    
    // Memory
    
    wxPanel *memory_panel=new wxPanel(book);
    wxBoxSizer *memory_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxButton *memory_btn=new wxButton(memory_panel,wxID_ANY,"Memorise Data");
    memory_btn->Bind(wxEVT_BUTTON,&GraphOptionsDialog::evt_memorize_data,this);
    
    wxStaticBoxSizer *memory_display_sizer=new wxStaticBoxSizer(wxVERTICAL,memory_panel,"Display Mode");
    
    // - Memory Mode
    
    memory_mode=new wxChoice(memory_panel,wxID_ANY);
    memory_mode->Append("None");
    memory_mode->Append("All");
    memory_mode->Append("Slider - Single");
    memory_mode->Append("Slider - All");
    
         if(graph->show_memory==MEM_NONE) memory_mode->SetSelection(0);
    else if(graph->show_memory==MEM_ALL) memory_mode->SetSelection(1);
    else if(graph->show_memory==MEM_SLIDER_SINGLE) memory_mode->SetSelection(2);
    else if(graph->show_memory==MEM_SLIDER_ALL) memory_mode->SetSelection(3);
    
    memory_mode->Bind(wxEVT_CHOICE,&GraphOptionsDialog::evt_memory_mode,this);
    
    // - Memory Slider
    
    int slider_max_value=graph->x_mem.size();
    if(slider_max_value==0) slider_max_value=1; // wx doesn't accept 0 to 0
    
    int slider_init_value=graph->show_memory_target;
    if(slider_init_value==-1) slider_init_value= slider_max_value;
    
    memory_slider=new wxSlider(memory_panel,wxID_ANY,slider_init_value,0,slider_max_value);
    
    if(graph->show_memory==MEM_NONE || graph->show_memory==MEM_ALL)
        memory_slider->Disable();
        
    memory_slider->Bind(wxEVT_SLIDER,&GraphOptionsDialog::evt_memory_slider,this);
    
    memory_display_sizer->Add(memory_mode,wxSizerFlags().Border(wxALL,2).Expand());
    memory_display_sizer->Add(memory_slider,wxSizerFlags().Border(wxALL,2).Expand());
    
    wxButton *clear_btn=new wxButton(memory_panel,wxID_ANY,"Clear Memory");
    clear_btn->Bind(wxEVT_BUTTON,&GraphOptionsDialog::evt_clear_memory,this);
    
    memory_sizer->Add(memory_btn,wxSizerFlags().Expand().Border(wxALL,2));
    memory_sizer->Add(memory_display_sizer,wxSizerFlags().Border(wxALL,2).Expand());
    memory_sizer->Add(clear_btn,wxSizerFlags().Expand().Border(wxALL,2));
    
    memory_panel->SetSizer(memory_sizer);
    
    book->AddPage(memory_panel,"Memory");
    
    // Backdrop
    
    backdrop_panel=new wxScrolledWindow(book);
    wxBoxSizer *backdrop_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxButton *add_backdrop_btn=new wxButton(backdrop_panel,wxID_ANY,"Add Backdrop Data");
    add_backdrop_btn->Bind(wxEVT_BUTTON,&GraphOptionsDialog::evt_add_backdrop,this);
    backdrop_sizer->Add(add_backdrop_btn);
    
    backdrops=new PanelsList<BackdropPanel>(backdrop_panel);
    
    for(std::size_t i=0;i<graph->x_self.size();i++)
    {
        backdrops->add_panel<BackdropPanel>(graph,i);
    }
    backdrop_panel->Bind(EVT_PLIST_REMOVE,&GraphOptionsDialog::evt_delete_backdrop,this);
    
    backdrop_sizer->Add(backdrops,wxSizerFlags().Expand());
    
    backdrop_panel->SetSizer(backdrop_sizer);
    
    book->AddPage(backdrop_panel,"Backdrop");
    
    //
    
    
    top_sizer->Add(book,wxSizerFlags().Expand());
    
    SetSizerAndFit(top_sizer);
}

void GraphOptionsDialog::GraphOptionsDialog_SpColors(wxWindow *panel)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    panel->SetSizer(sizer);
    
    for(int i=0;i<N_data;i++)
    {
        wxBoxSizer *sub_sizer=new wxBoxSizer(wxHORIZONTAL);
        
        wxStaticText *cn=new wxStaticText(panel,wxID_ANY,std::to_string(i+1)+": ");
        
        sub_sizer->Add(cn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        
        NamedTextCtrl<double> *X=new NamedTextCtrl<double>(panel,"X: ",0);
        NamedTextCtrl<double> *Y=new NamedTextCtrl<double>(panel,"Y: ",0);
        NamedTextCtrl<double> *Z=new NamedTextCtrl<double>(panel,"Z: ",0);
        
        X->lock();
        Y->lock();
        Z->lock();
        
        wxBoxSizer *XYZ_sizer=new wxBoxSizer(wxVERTICAL);
        
        XYZ_sizer->Add(X);
        XYZ_sizer->Add(Y);
        XYZ_sizer->Add(Z);
        
        spcol_X.push_back(X);
        spcol_Y.push_back(Y);
        spcol_Z.push_back(Z);
        
        sub_sizer->Add(XYZ_sizer);
        
        wxButton *col=new wxButton(panel,wxID_ANY,"       ");
        wxButton *col_norm=new wxButton(panel,wxID_ANY,"       ");
        
        col->SetBackgroundColour(wxColour(0,0,0));
        col_norm->SetBackgroundColour(wxColour(0,0,0));
        
        sub_sizer->Add(col,wxSizerFlags(1).Expand());
        sub_sizer->Add(col_norm,wxSizerFlags(1).Expand());
        
        spcol.push_back(col);
        spcol_norm.push_back(col_norm);
        
        sizer->Add(sub_sizer,wxSizerFlags().Expand());
        
        if(i!=N_data-1)
            sizer->Add(new wxStaticLine(panel),wxSizerFlags().Expand().Border(wxALL,3));
    }
    
    update_spectral_colors();
}

void GraphOptionsDialog::evt_add_backdrop(wxCommandEvent &event)
{
    wxString fname=wxFileSelectorEx("Select a data file");
    
    if(fname.size()>0)
    {
        std::vector<std::vector<double>> data;
        ascii_data_loader(fname.ToStdString(),data);
        
        graph->x_self.push_back(data[0]);
        graph->y_self.push_back(data[1]);
        graph->show_self.push_back(true);
        graph->pen_self.push_back(*wxBLACK_PEN);
        graph->fname_self.push_back(fname.ToStdString());
        graph->legend_self.push_back("");
    
        rebuild_backdrop_list();
        graph->Refresh();
    }
}

void GraphOptionsDialog::evt_delete_backdrop(wxCommandEvent &event)
{
    int N=backdrops->get_size();
    std::vector<bool> present(N+1);
    
    for(int i=0;i<N;i++)
    {
        BackdropPanel *panel=backdrops->get_panel(i);
        
        chk_var(panel->ID);
        present[panel->ID]=true;
    }
    
    for(int i=0;i<=N;i++)
    {
        if(present[i]==false)
        {
            graph->forget_self_data(i);
            break;
        }
    }
    
    graph->Refresh();
    rebuild_backdrop_list();
}

void GraphOptionsDialog::evt_checkbox(wxCommandEvent &event)
{
    int i=event.GetId();
    
    graph->show_data_v[i]=checkbox[i]->GetValue();
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_clear_memory(wxCommandEvent &event)
{
    memory_mode->SetSelection(0);
    memory_slider->Disable();
    
    graph->clear_memorized_data();
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_color(wxCommandEvent &event)
{
    int i=event.GetId();
    
    wxColourData data;
    
    data.SetColour(color[i]->GetBackgroundColour());
    
    wxColourDialog dialog(this,&data);
    dialog.ShowModal();
    
    wxColour tmp=dialog.GetColourData().GetColour();
    color[i]->SetBackgroundColour(tmp);
    graph->data_pen[i].SetColour(tmp);
    
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_legend(wxCommandEvent &event)
{
    int i=event.GetId();
    
    graph->legend_str[i]=legend[i]->GetValue().ToStdString();
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_memorize_data(wxCommandEvent &event)
{
    graph->memorize_data();
    
    if(graph->x_mem.size()!=0)
    {
        int smax=graph->x_mem.size();
        
        memory_slider->SetMax(smax);
        
        if(graph->show_memory_target==-1)
            memory_slider->SetValue(smax);
        
        graph->Refresh();
    }
    
    event.Skip();
}

void GraphOptionsDialog::evt_memory_mode(wxCommandEvent &event)
{
    int choice=memory_mode->GetSelection();
    
         if(choice==0) { graph->show_memory=MEM_NONE; memory_slider->Disable(); }
    else if(choice==1) { graph->show_memory=MEM_ALL; memory_slider->Disable(); }
    else if(choice==2) { graph->show_memory=MEM_SLIDER_SINGLE; memory_slider->Enable(); }
    else if(choice==3) { graph->show_memory=MEM_SLIDER_ALL; memory_slider->Enable(); }
    
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_memory_slider(wxCommandEvent &event)
{
    int value=memory_slider->GetValue();
    
    if(value==memory_slider->GetMax()) graph->show_memory_target=-1;
    else graph->show_memory_target=value;
    
    graph->Refresh();
    
    event.Skip();
}

void GraphOptionsDialog::evt_scale(wxCommandEvent &event)
{
    graph->xmin=std::min(x_min->get_value(),x_max->get_value());
    graph->xmax=std::max(x_max->get_value(),x_max->get_value());
    
    graph->ymin=std::min(y_min->get_value(),y_max->get_value());
    graph->ymax=std::max(y_max->get_value(),y_max->get_value());
    
    graph->lock_scale=lock_scale->GetValue();
    
    graph->Refresh();
}

void GraphOptionsDialog::rebuild_backdrop_list()
{
    backdrops->clear();
    
    for(std::size_t i=0;i<graph->x_self.size();i++)
    {
        backdrops->add_panel<BackdropPanel>(graph,i);
    }
    
    backdrop_panel->Layout();
    backdrop_panel->FitInside();
    
    Layout();
    Fit();
}

void GraphOptionsDialog::update_spectral_colors()
{
    for(int i=0;i<N_data;i++)
    {
        double X,Y,Z;
        
        spectrum_to_XYZ(*(graph->x_data[i]),*(graph->y_data[i]),"D65",X,Y,Z);
        
        spcol_X[i]->set_value(X);
        spcol_Y[i]->set_value(Y);
        spcol_Z[i]->set_value(Z);
        
        double R,G,B;
        
        XYZ_to_sRGB(X,Y,Z,R,G,B);
        
        spcol[i]->SetBackgroundColour(wxColour(R*255,G*255,B*255));
        
        double cmax=var_max(R,G,B);
        
        R/=cmax;
        G/=cmax;
        B/=cmax;
        
        spcol_norm[i]->SetBackgroundColour(wxColour(R*255,G*255,B*255));
    }
}

void GraphOptionsDialog::update_scale()
{
    x_min->set_value(graph->xmin);
    x_max->set_value(graph->xmax);
    
    y_min->set_value(graph->ymin);
    y_max->set_value(graph->ymax);
}

//###########
//   Graph
//###########

Graph::Graph(wxWindow *parent)
    :wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(100,100)),
     spectral_data(false),
     draw_grid(true), graph_drag(false), legend(false), black(false), lock_scale(false),
     padxm(50), padxp(25),
     padym(25), padyp(25),
     xmin(-4), xmax(4),
     ymin(-4), ymax(4),
     fixed_ratio(false),
     x_ratio(1.0), y_ratio(1.0),
     right_zoom(false),
     right_zoom_xs(0), right_zoom_ys(0),
     N_data(0),
     show_memory(MEM_NONE),
     show_memory_target(-1),
     options_dialog(nullptr)
{
    SetBackgroundColour(wxColour(255,255,255));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    GetSize(&sx,&sy);
    
    select_pen.SetColour(wxColour(80,80,230,180));
    select_brush.SetColour(wxColour(80,80,230,120));
    
    grid_pen.SetColour(wxColour(220,220,220,255));
    
    Bind(wxEVT_MOUSEWHEEL,&Graph::evt_mouse_zoom,this);
    Bind(wxEVT_KEY_DOWN,&Graph::evt_keyboard,this);
    Bind(wxEVT_PAINT,&Graph::on_paint,this);
    #ifndef GTK_RMV_RESIZE
    Bind(wxEVT_SIZE,&Graph::on_resize,this);
    #endif
    Bind(wxEVT_LEFT_DOWN,&Graph::evt_mouse_left_down,this);
    Bind(wxEVT_LEFT_UP,&Graph::evt_mouse_left_up,this);
    Bind(wxEVT_RIGHT_DOWN,&Graph::evt_mouse_right_down,this);
    Bind(wxEVT_RIGHT_DCLICK,&Graph::evt_mouse_right_down,this);
    Bind(wxEVT_LEAVE_WINDOW,&Graph::evt_mouse_hold_reset,this);
    Bind(wxEVT_MOTION,&Graph::evt_mouse_motion,this);
}

void Graph::add_data(std::vector<double> *x_data_in,std::vector<double> *y_data_in,double r,double g,double b,std::string legend_str_)
{
    add_external_data(x_data_in,y_data_in,r,g,b,legend_str_);
}

void Graph::add_external_data(std::vector<double> *x_data_in,std::vector<double> *y_data_in,double r,double g,double b,std::string legend_str_)
{
    close_options_dialog();
    
    wxColour col(r*255,g*255,b*255);
    
    if(x_data_in->size()==y_data_in->size())
    {
        N_data++;
        x_data.push_back(x_data_in);
        y_data.push_back(y_data_in);
        show_data_v.push_back(true);
        data_pen.push_back(wxPen(col));
        legend_str.push_back(legend_str_);
        if(legend_str_.size()>0) legend=true;
    }
}

void Graph::autoscale()
{
    if(x_data.size()<=0) return;
    if(x_data[0]->size()<=0) return;
    
    double xmin_,xmax_,ymin_,ymax_;
    
    xmin_=xmax_=(*x_data[0])[0];
    ymin_=ymax_=(*y_data[0])[0];
    
    if(std::isnan(xmin_) || std::isinf(xmin_))
    {
        xmin_=-4;
        xmax_=4;
    }
    
    if(std::isnan(ymin_) || std::isinf(ymin_))
    {
        ymin_=-4;
        ymax_=4;
    }
    
    for(int l=0;l<N_data;l++)
    {
        std::vector<double> &tmp_x=*(x_data[l]);
        std::vector<double> &tmp_y=*(y_data[l]);
        
        for(unsigned int i=0;i<tmp_x.size();i++)
        {
            if(!std::isinf(tmp_x[i]))
            {
                xmin_=std::min(xmin_,tmp_x[i]);
                xmax_=std::max(xmax_,tmp_x[i]);
            }
            
            if(!std::isinf(tmp_y[i]))
            {
                ymin_=std::min(ymin_,tmp_y[i]);
                ymax_=std::max(ymax_,tmp_y[i]);
            }
        }
    }
    
    double spanx=xmax_-xmin_;
    double spany=ymax_-ymin_;
    
    xmin_-=0.05*spanx; xmax_+=0.05*spanx;
    ymin_-=0.05*spany; ymax_+=0.05*spany;
    
    if(spanx==0)
    {
        xmin_-=0.1*std::abs(xmin_);
        xmax_+=0.1*std::abs(xmax_);
        
        if(xmin_==0 && xmax_==0)
        {
            xmin_=-1;
            xmax_=+1;
        }
    }
    if(spany==0)
    {
        ymin_-=0.1*std::abs(ymin_);
        ymax_+=0.1*std::abs(ymax_);
        
        if(ymin_==0 && ymax_==0)
        {
            ymin_=-1;
            ymax_=+1;
        }
    }
    
    set_scale(xmin_,xmax_,ymin_,ymax_);
    
    Refresh();
}

void Graph::clear_graph()
{
    N_data=0;
    legend=false;
    x_data.clear();
    y_data.clear();
    show_data_v.clear();
    data_pen.clear();
    legend_str.clear();
}

void Graph::clear_memorized_data()
{
    show_memory=MEM_NONE;
    show_memory_target=-1;
    
    x_mem.clear();
    y_mem.clear();
    pen_mem.clear();
    
    Refresh();
}

void Graph::close_options_dialog()
{
    if(options_dialog!=nullptr)
    {
        options_dialog->Destroy();
        options_dialog=nullptr;
    }
}

int Graph::d2px(double x)
{
    return padxm+static_cast<int>((sx-padxm-padxp)*(x-xmin)/(xmax-xmin));
}

int Graph::d2py(double y)
{
    return sy-padym-static_cast<int>((sy-padym-padyp)*(y-ymin)/(ymax-ymin));
}

void Graph::draw_border(wxGraphicsContext *gc)
{
    if(black) gc->SetPen(*wxWHITE_PEN);
    else gc->SetPen(*wxBLACK_PEN);
    
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

void Graph::draw_data(wxGraphicsContext *gc)
{
    if(show_memory!=MEM_NONE)
    {
        for(unsigned int l=0;l<x_mem.size();l++)
            draw_memory_data(gc,l);
    }
    
    for(int l=0;l<N_data;l++)
    {
        if(show_data_v[l])
        {
            if(show_memory==MEM_NONE || show_memory==MEM_ALL || show_memory_target==-1)
            {
                draw_data(gc,*(x_data[l]),*(y_data[l]),data_pen[l]);
            }
            else if(show_memory==MEM_SLIDER_ALL && show_memory_target!=-1)
            {
                draw_data(gc,*(x_data[l]),*(y_data[l]),*wxLIGHT_GREY_PEN);
            }
        }
    }
    
    for(std::size_t l=0;l<x_self.size();l++)
    {
        if(show_self[l])
        {
            draw_data(gc,x_self[l],y_self[l],pen_self[l]);
        }
    }
    
    gc->SetPen(*wxBLACK_PEN);
}


void Graph::draw_data(wxGraphicsContext *gc,
                      std::vector<double> const &x_data_,
                      std::vector<double> const &y_data_,
                      wxPen const &pen_)
{
    gc->SetPen(pen_);
    
    for(unsigned int i=0;i<x_data_.size()-1;i++)
    {
        if(!std::isinf(x_data_[i])   &&
           !std::isinf(x_data_[i+1]) &&
           !std::isinf(y_data_[i])   &&
           !std::isinf(y_data_[i+1]))
        {
            gc->StrokeLine(d2px(x_data_[i]),d2py(y_data_[i]),
                           d2px(x_data_[i+1]),d2py(y_data_[i+1]));
        }
    }
}

void Graph::draw_memory_data(wxGraphicsContext *gc,int level)
{
    if(show_memory==MEM_SLIDER_SINGLE && show_memory_target!=level) return;
    
    std::vector<std::vector<double>> &tmp_x=x_mem[level];
    std::vector<std::vector<double>> &tmp_y=y_mem[level];
    
    if(show_memory==MEM_SLIDER_ALL && show_memory_target!=level)
    {
        for(unsigned int i=0;i<tmp_x.size();i++)
            draw_data(gc,tmp_x[i],tmp_y[i],*wxLIGHT_GREY_PEN);
    }
    else
    {
        for(unsigned int i=0;i<tmp_x.size();i++)
            draw_data(gc,tmp_x[i],tmp_y[i],pen_mem[level][i]);
    }
}

void Graph::draw_grid_coord(wxGraphicsContext *gc)
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

void Graph::draw_legend(wxGraphicsContext *gc)
{
    int l,Ns=0;
    
    double sw,sh,sd,sext;
    double sw_max=0,sh_max=0;
    
    for(l=0;l<N_data;l++)
    {
        if(legend_str[l].size()>0)
        {
            gc->GetTextExtent(legend_str[l],&sw,&sh,&sd,&sext);
            sw_max=std::max(sw_max,sw);
            sh_max=std::max(sh_max,sh);
            Ns++;
        }
    }
    
    
    Ns=0;
    
    for(int l=0;l<N_data;l++)
    {
        if(legend_str[l].size()>0)
        {
            double x_text=sx-padxp-10-sw_max;
            double y_text=10+padym+Ns*(sh_max+3);
            
            if(black) gc->SetPen(*wxWHITE_PEN);
            else gc->SetPen(*wxBLACK_PEN);
            gc->DrawText(legend_str[l],x_text,y_text);
            
            gc->SetPen(data_pen[l]);
            gc->StrokeLine(x_text-10,y_text+sh_max/2.0,x_text-60,y_text+sh_max/2.0);
                           
            Ns++;
        }
    }
    
    if(black) gc->SetPen(*wxWHITE_PEN);
    else gc->SetPen(*wxBLACK_PEN);
}

void Graph::evt_dialog_closed(wxCloseEvent &event)
{
    options_dialog=nullptr;
    
    event.Skip();
}

void Graph::evt_keyboard(wxKeyEvent &event)
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
            
                Refresh();
            }
            else autoscale();
        }
        else if(event.GetUnicodeKey()=='G') 
        {
            draw_grid=!draw_grid;
            
            Refresh();
        }
        else if(event.GetUnicodeKey()=='M')
        {
            close_options_dialog();
            memorize_data();
        }
        else if(event.GetUnicodeKey()=='O') 
        {
            if(options_dialog==nullptr)
            {
                options_dialog=new GraphOptionsDialog(this);
                
                options_dialog->Show();
                
                options_dialog->Bind(wxEVT_CLOSE_WINDOW,&Graph::evt_dialog_closed,this);
            }
        }
        else if(event.GetUnicodeKey()=='P') 
        {
            print();
        }
    }
}

void Graph::evt_mouse_hold_reset(wxMouseEvent &event)
{
    graph_drag=false;
}

void Graph::evt_mouse_left_down(wxMouseEvent &event)
{
    if(!evt_mouse_left_down_spec(event))
    {
        if(right_zoom) process_right_zoom(event);
        else
        {
            graph_drag=true;
        }
    }
    
    event.Skip();
}

bool Graph::evt_mouse_left_down_spec(wxMouseEvent &event)
{
    return false;
}

void Graph::evt_mouse_left_up(wxMouseEvent &event)
{
    if(!evt_mouse_left_up_spec(event))
    {
        graph_drag=false;
    }
    
    event.Skip();
}

bool Graph::evt_mouse_left_up_spec(wxMouseEvent &event)
{
    return false;
}

void Graph::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(!evt_mouse_motion_spec(event))
    {
        if(graph_drag)
        {
            double dx=(xmax-xmin)/(sx-padxm-padxp)*(mouse_x-mouse_xp);
            double dy=(ymax-ymin)/(sy-padym-padyp)*(mouse_y-mouse_yp);
            
            set_scale_shift(-dx,-dx,dy,dy);
            
            Refresh();
        }
        else if(right_zoom) Refresh();
    }
    
    if(options_dialog!=nullptr)
    {
        double x=px2dx(mouse_x);
        options_dialog->pointer_x->set_value(x);
        
        for(int i=0;i<N_data;i++)
        {
            double y=vector_interp_linear(*x_data[i],*y_data[i],x);
            options_dialog->pointer_y[i]->set_value(y);
        }
    }
    
    event.Skip();
}

bool Graph::evt_mouse_motion_spec(wxMouseEvent &event)
{
    return false;
}

void Graph::evt_mouse_right_down(wxMouseEvent &event)
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

void Graph::evt_mouse_zoom(wxMouseEvent &event)
{
    int rot=event.GetWheelRotation();
    
    double c_x=px2dx(event.GetX());
    double c_y=py2dy(event.GetY());
    
    double xm=c_x-xmin;
    double xp=xmax-c_x;
    
    double ym=c_y-ymin;
    double yp=ymax-c_y;
    
    if(rot>=0)
        set_scale(c_x-0.95*xm,c_x+0.95*xp,c_y-0.95*ym,c_y+0.95*yp);
    else
        set_scale(c_x-1.05*xm,c_x+1.05*xp,c_y-1.05*ym,c_y+1.05*yp);
    
    Refresh();
}

void Graph::force_ratio(double x_ratio_,double y_ratio_)
{
    fixed_ratio=true;
    x_ratio=x_ratio_;
    y_ratio=y_ratio_;
}

void Graph::forget_data(std::vector<double> *x_data_,std::vector<double> *y_data_)
{
    forget_external_data(x_data_,y_data_);
}

void Graph::forget_external_data(std::vector<double> *x_data_,std::vector<double> *y_data_)
{
    int pos=-1;
    unsigned int i;
    
    for(i=0;i<x_data.size();i++)
    {
        if(x_data[i]==x_data_ && y_data[i]==y_data_)
        {
            pos=i;
            break;
        }
    }
    
    if(pos>=0)
    {
        std::vector<std::vector<double>*>::const_iterator itx=x_data.begin()+pos;
        std::vector<std::vector<double>*>::const_iterator ity=y_data.begin()+pos;
        std::vector<bool>::const_iterator its=show_data_v.begin()+pos;
        std::vector<wxPen>::const_iterator itp=data_pen.begin()+pos;
        std::vector<std::string>::const_iterator itl=legend_str.begin()+pos;
        
        x_data.erase(itx);
        y_data.erase(ity);
        show_data_v.erase(its);
        data_pen.erase(itp);
        legend_str.erase(itl);
        
        legend=false;
        for(i=0;i<legend_str.size();i++)
        {
            if(legend_str[i].size()>0)
            {
                legend=true;
                break;
            }
        }
        
        N_data--;
    }
}

void Graph::forget_self_data(int i)
{
    x_self.erase(x_self.begin()+i);
    y_self.erase(y_self.begin()+i);
    show_self.erase(show_self.begin()+i);
    pen_self.erase(pen_self.begin()+i);
    fname_self.erase(fname_self.begin()+i);
    legend_self.erase(legend_self.begin()+i);
}

void Graph::get_scale(double &xmin_,double &xmax_,double &ymin_,double &ymax_)
{
    xmin_=xmin; xmax_=xmax;
    ymin_=ymin; ymax_=ymax;
}

void Graph::memorize_data()
{
    if(x_data.size()==0) return;
    
    if(show_memory==MEM_NONE) show_memory=MEM_ALL;
    
    int N_mem=0;
    for(int i=0;i<N_data;i++)
        if(show_data_v[i]) N_mem++;
    
    std::vector<std::vector<double>> x_tmp(N_mem),y_tmp(N_mem);
    std::vector<wxPen> pen_tmp(N_mem);
    
    N_mem=0;
    
    for(int i=0;i<N_data;i++)
    {
        if(show_data_v[i])
        {
            x_tmp[N_mem]=(*x_data[i]);
            y_tmp[N_mem]=(*y_data[i]);
            
            pen_tmp[N_mem]=data_pen[i];
            
            N_mem++;
        }
    }
    
    x_mem.push_back(x_tmp);
    y_mem.push_back(y_tmp);
    
    pen_mem.push_back(pen_tmp);
    
}

void Graph::on_paint(wxPaintEvent &event)
{
    rescale();
    wxBufferedPaintDC dc(this);
    dc.Clear();
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void Graph::on_resize(wxSizeEvent &event)
{
    rescale();
    wxClientDC dc(this);
    dc.Clear();
    
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    delete gc;
}

void Graph::print()
{
    std::string fname("");
    
    fname=wxFileSelector("Select the file to save to:",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         wxEmptyString,
                         "PNG image (*.png)|*.png",
                         wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
                         
    if(fname.size()==0) return;
    
    wxBitmap bitmap(GetSize().x,GetSize().y);
    
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.Clear();
    
    wxGraphicsContext *gc=wxGraphicsContext::Create(dc);
    replot(gc);
    
    wxImage image=bitmap.ConvertToImage();
    image.SaveFile(fname,wxBITMAP_TYPE_PNG);
}

void Graph::postplot(wxGraphicsContext *gc)
{
}

void Graph::process_right_zoom(wxMouseEvent &event)
{
    int i=event.GetX();
    int j=event.GetY();
    
    right_zoom=false;
    
    double xmin_b=px2dx(std::min(i,right_zoom_xs));
    double xmax_b=px2dx(std::max(i,right_zoom_xs));
    double ymin_b=py2dy(std::max(j,right_zoom_ys));
    double ymax_b=py2dy(std::min(j,right_zoom_ys));
    
    set_scale(xmin_b,xmax_b,ymin_b,ymax_b);
    
    Refresh();
}

double Graph::px2dx(int i)
{
    return xmin+(xmax-xmin)*(i-padxm)/(sx-padxm-padxp+0.0);
}

double Graph::py2dy(int j)
{
    return ymax+(ymin-ymax)*(j-padyp)/(sy-padym-padyp+0.0);
}

void Graph::replot(wxGraphicsContext *gc)
{
    if(black) gc->SetFont(*wxNORMAL_FONT,wxColour(255,255,255));
    else gc->SetFont(*wxNORMAL_FONT,wxColour(0,0,0));
    
    draw_grid_coord(gc);
    
    draw_data(gc);
    
    draw_border(gc);
    
    if(legend) draw_legend(gc);
    
    if(spectral_data && options_dialog!=nullptr)
        options_dialog->update_spectral_colors();
    
    postplot(gc);
}

void Graph::rescale()
{
    GetSize(&sx,&sy);
}

void Graph::set_data_color(std::vector<double> *x_data_,std::vector<double> *y_data_,
                           double r,double g,double b)
{
    wxColour col(r*255,g*255,b*255);
    
    for(unsigned int i=0;i<x_data.size();i++)
    {
        if(x_data[i]==x_data_ && y_data[i]==y_data_)
        {
            data_pen[i]=wxPen(col);
            return;
        }
    }
}

void Graph::show_data(std::vector<double> *x_data_,std::vector<double> *y_data_,bool val)
{
    for(unsigned int i=0;i<x_data.size();i++)
    {
        if(x_data[i]==x_data_ && y_data[i]==y_data_)
        {
            show_data_v[i]=val;
            return;
        }
    }
}

void Graph::set_scale(double xmin_,double xmax_,double ymin_,double ymax_)
{
    if(!lock_scale)
    {
        xmin=std::min(xmin_,xmax_);
        xmax=std::max(xmin_,xmax_);
        
        ymin=std::min(ymin_,ymax_);
        ymax=std::max(ymin_,ymax_);
        
        if(options_dialog!=nullptr)
            options_dialog->update_scale();
    }
}

void Graph::set_scale_shift(double xmin_,double xmax_,double ymin_,double ymax_)
{
    if(!lock_scale)
    {
        xmin+=xmin_;
        xmax+=xmax_;
        
        ymin+=ymin_;
        ymax+=ymax_;
        
        if(options_dialog!=nullptr)
            options_dialog->update_scale();
    }
}

void Graph::switch_dark()
{
    black=!black;
    
    if(black)
    {
        SetBackgroundColour(wxColour(0,0,0));
        grid_pen.SetColour(wxColour(35,35,35,255));
    }
    else
    {
        SetBackgroundColour(wxColour(255,255,255));
        grid_pen.SetColour(wxColour(220,220,220,255));
    }
    
    Refresh();
}


//#####################

//ImgDsp::ImgDsp(wxWindow *parent)
//    :wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(1000,1000))
//{
//    SetBackgroundColour(wxColour(255,255,255));
//    
//    Bind(wxEVT_PAINT,&ImgDsp::on_paint,this);
//    Bind(wxEVT_SIZE,&ImgDsp::on_resize,this);
//}
//
//void ImgDsp::on_paint(wxPaintEvent &event)
//{
////    ClearBackground();
//    int sx,sy;
//    GetSize(&sx,&sy);
//    SetBackgroundColour(wxColour(255,0,0));
//    std::cout<<sx<<" "<<sy<<std::endl;
//    
//    wxImage imag;
//    //imag.AddHandler(wxBMPHandler);
//    imag.InitStandardHandlers();
//    imag.LoadFile("Brods_nTE_2_Fmap.bmp");
//    imag.Rescale(sx,sy);
//    wxBitmap bmp(imag);
//    
//    wxPaintDC dc(this);
//    dc.Clear();
//    dc.DrawLine(20,sy-20,sx-20,sy-20);
//    dc.DrawLine(20,sy-20,20,20);
//    dc.DrawBitmap(bmp,0,0);
//    dc.DrawText("blah",100,100);
//}
//
//void ImgDsp::on_resize(wxSizeEvent &event)
//{
//    int sx,sy;
//    GetSize(&sx,&sy);
//    SetBackgroundColour(wxColour(255,0,0));
//    std::cout<<sx<<" "<<sy<<std::endl;
//    
//    wxImage imag;
//    //imag.AddHandler(wxBMPHandler);
//    imag.InitStandardHandlers();
//    imag.LoadFile("Brods_nTE_2_Fmap.bmp");
//    imag.Rescale(sx,sy);
//    wxBitmap bmp(imag);
//    
//    wxClientDC dc(this);
//    dc.Clear();
//    dc.DrawLine(20,sy-20,sx-20,sy-20);
//    dc.DrawLine(20,sy-20,20,20);
////    dc.DrawBitmap(bmp,0,0);
//    dc.DrawText("blah",100,100);
//
//    std::cout<<"size"<<std::endl;
//}
