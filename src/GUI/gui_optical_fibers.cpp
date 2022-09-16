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

#include <gui_optical_fibers.h>


OptFibersFrame::OptFibersFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    wxScrolledWindow *ctrl_panel=new wxScrolledWindow(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    spectrum=new SpectrumSelector(ctrl_panel,400e-9,800e-9,81,true);
    
    ctrl_sizer->Add(spectrum,wxSizerFlags().Expand());
    
    wxStaticBoxSizer *radius_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Radius");
    radius=new LengthSelector(ctrl_panel,"",300e-9);
    
    radius_sizer->Add(radius,wxSizerFlags().Expand());
    ctrl_sizer->Add(radius_sizer,std_flags);
    
    wxStaticBoxSizer *core_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Core");
    wxStaticBoxSizer *cladding_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Cladding");
    
    core_mat=new MiniMaterialSelector(ctrl_panel);
    cladding_mat=new MiniMaterialSelector(ctrl_panel);
    
    core_sizer->Add(core_mat,wxSizerFlags().Expand());
    cladding_sizer->Add(cladding_mat,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(core_sizer,std_flags);
    ctrl_sizer->Add(cladding_sizer,std_flags);
    
    wxStaticBoxSizer *computation_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Computation");
    
    min_order=new NamedTextCtrl<int>(ctrl_panel,"Min Order: ",0);
    max_order=new NamedTextCtrl<int>(ctrl_panel,"Max Order: ",1);
    precision=new NamedTextCtrl<double>(ctrl_panel,"Precision: ",2);
    
    wxButton *compute_btn=new wxButton(ctrl_panel,wxID_ANY,"Compute");
    report_btn=new wxButton(ctrl_panel,wxID_ANY,"Report");
    export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    
    report_btn->Disable();
    export_btn->Disable();
    export_btn->Hide();
    
    computation_sizer->Add(min_order,wxSizerFlags().Expand());
    computation_sizer->Add(max_order,wxSizerFlags().Expand());
    computation_sizer->Add(precision,wxSizerFlags().Expand());
    
    computation_sizer->Add(compute_btn,wxSizerFlags().Expand());
    computation_sizer->Add(report_btn,wxSizerFlags().Expand());
    computation_sizer->Add(export_btn,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(computation_sizer,std_flags);
    
    // Display Controls
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    graph=new Graph(display_panel);
    display_sizer->Add(graph,wxSizerFlags(1).Expand());
    
    display_panel->SetSizer(display_sizer);
    
    // Bindings
    
    compute_btn->Bind(wxEVT_BUTTON,&OptFibersFrame::evt_compute,this);
    export_btn->Bind(wxEVT_BUTTON,&OptFibersFrame::evt_export,this);
    report_btn->Bind(wxEVT_BUTTON,&OptFibersFrame::evt_report,this);
    
    //
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
}

void OptFibersFrame::clear_data()
{
    for(unsigned int i=0;i<modes_raw.size();i++) modes_raw[i].clear();
    modes_raw.clear();
    
    
}

void OptFibersFrame::evt_compute(wxCommandEvent &event)
{
    int l;
    
    report_btn->Enable();
    export_btn->Enable();
    graph->clear_graph();
    
    OpticalFiber fiber(radius->get_length());
    
    int min_order_value=min_order->get_value();
    int max_order_value=max_order->get_value();
    
    if(min_order_value<0)
    {
        min_order_value=0;
        min_order->set_value(0);
    }
    if(max_order_value<0)
    {
        max_order_value=0;
        max_order->set_value(0);
    }
    
    double precision_value=precision->get_value();
    
    clear_data();
    
    int Nl=spectrum->get_Np();
    double lambda_min=spectrum->get_lambda_min();
    double lambda_max=spectrum->get_lambda_max();
    
    modes_raw.resize(Nl);
    
    for(l=0;l<Nl;l++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
        double w=m_to_rad_Hz(lambda);
        
        double n1=std::real(core_mat->get_n(w));
        double n2=std::real(cladding_mat->get_n(w));
        
        fiber.compute_modes(lambda,n1,n2,
                            min_order_value,max_order_value,
                            precision_value,modes_raw[l]);
    }
    
    int N_TE=optical_fibers_reorders_TE(modes_raw,modes_TE,modes_TE_l);
    int N_TM=optical_fibers_reorders_TM(modes_raw,modes_TM,modes_TM_l);
    optical_fibers_reorders_TEM(modes_raw,modes_TEM,modes_TEM_l,modes_TEM_orders);
    
    graph->clear_graph();
    
    for(l=0;l<=N_TE;l++)
    {
        if(l==0) graph->add_external_data(&modes_TE_l[l],&modes_TE[l],0,0,1,"TE");
        else graph->add_external_data(&modes_TE_l[l],&modes_TE[l],0,0,1);
    }
    
    for(l=0;l<=N_TM;l++)
    {
        if(l==0) graph->add_external_data(&modes_TM_l[l],&modes_TM[l],1,0,0,"TM");
        else graph->add_external_data(&modes_TM_l[l],&modes_TM[l],1,0,0);
    }
        
    for(unsigned int l=0;l<modes_TEM.size();l++)
    {
        if(l==0) graph->add_external_data(&modes_TEM_l[l],&modes_TEM[l],0,0.5,0,"TEM");
        else graph->add_external_data(&modes_TEM_l[l],&modes_TEM[l],0,0.5,0);
    }
    
    graph->autoscale();
    
    event.Skip();
}

void OptFibersFrame::evt_export(wxCommandEvent &event)
{
    event.Skip();
}

void OptFibersFrame::evt_report(wxCommandEvent &event)
{
    unsigned int l,Nl=modes_raw.size();
    
    std::stringstream strm;
    
    for(l=0;l<Nl;l++)
        strm<<optical_fiber_generate_report(modes_raw[l]);
    
    wxString report(strm.str());
    
    int sx,sy;
    GetSize(&sx,&sy);
    
    OptFibersReportDialog dialog(wxPoint(0.1*sx,0.1*sy),
                                 wxSize(0.8*sx,0.8*sy),report);
    
    event.Skip();
}

//###########################
//   OptFibersReportDialog
//###########################

OptFibersReportDialog::OptFibersReportDialog(wxPoint const &coord,wxSize const &size,wxString const &report)
    :wxDialog(NULL,wxID_ANY,"Report",coord,size)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    wxTextCtrl *test=new wxTextCtrl(this,wxID_ANY,report,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
    
    sizer->Add(test,wxSizerFlags(1).Expand());
    
    SetSizer(sizer);
    
    ShowModal();
}
