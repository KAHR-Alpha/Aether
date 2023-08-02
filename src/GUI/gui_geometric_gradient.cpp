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

#include <phys_tools.h>
#include <string_tools.h>

#include <gui_geometric_gradient.h>

GeoGradientHolder::GeoGradientHolder()
    :lattice(LAT_HEXA),
     profile(PROF_LINEAR),
     height(100e-9),
     periodicity(150e-9),
     top_radius(0), bottom_radius(75e-9)
{
}

double GeoGradientHolder::get_ratio(double z)
{
    double r=0,radius=0;
    double &r1=top_radius;
    double &r2=bottom_radius;
    
    
         if(profile==PROF_LINEAR) radius=r1+(r2-r1)*(1.0-z/height);
    else if(profile==PROF_S_CURVE)
    {
        radius=r1+dichotomy<double,double,double>(0,r2-r1,value_s_curve,r2-r1,0,z/height,1e-6);
    }
    else if(profile==PROF_GAUSSIAN)
    {
        if(z<=0) radius=periodicity;
        else radius=r1+(r2-r1)*std::sqrt(-std::log(z/height));
    }
    
    if(z>=height) radius=0;
    
    double a_base=1.0;
    double a_circ=Pi*radius*radius;
    
    if(lattice==LAT_SQUARE) a_base=periodicity*periodicity;
    else if(lattice==LAT_HEXA) a_base=periodicity*periodicity*std::sqrt(3.0)/4.0;
    
    if(radius<=periodicity/2.0)
    {
             if(lattice==LAT_SQUARE) r=a_circ/a_base;
        else if(lattice==LAT_HEXA) r=a_circ/2.0/a_base;
    }
    else
    {
        double p2=periodicity/2.0;
        double y=std::sqrt(radius*radius-p2*p2);
        
        double a_over=2.0*radius*radius*std::atan(y/p2)-periodicity*y;
        
             if(lattice==LAT_SQUARE)
        {
            if(radius>=periodicity/std::sqrt(2.0)) return 1.0;
            else r=(a_circ-4.0*a_over/2.0)/a_base;
        }
        else if(lattice==LAT_HEXA)
        {
            if(radius>=periodicity/std::sqrt(3.0)) return 1.0;
            else r=(a_circ/2.0-3.0*a_over/2.0)/a_base;
        }
    }
        
    return r;
}

//#####################
//   ProfileDesigner
//#####################

GeoGradientDesigner::GeoGradientDesigner(GeoGradientHolder *holder_,wxPoint const &pos,wxSize const &size)
    :wxDialog(NULL,-1,"ProfileDesigner",pos,size,wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER),
     holder(holder_)
{
    wxPanel *top_panel=new wxPanel(this);
    
    wxSplitterWindow *splitter=new wxSplitterWindow(top_panel);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    wxScrolledWindow *ctrl_panel=new wxScrolledWindow(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    height_ctrl=new LengthSelector(ctrl_panel,"Height: ",holder->height);
    ctrl_sizer->Add(height_ctrl,std_flags);
    
    periodicity_ctrl=new LengthSelector(ctrl_panel,"Periodicity: ",holder->periodicity);
    ctrl_sizer->Add(periodicity_ctrl,std_flags);
    
    wxString lat_choices[]={"Square","Hexagonal"};
    lattice_ctrl=new wxRadioBox(ctrl_panel,wxID_ANY,"Lattice",wxDefaultPosition,wxDefaultSize,2,lat_choices,1);
         if(holder->lattice==LAT_SQUARE) lattice_ctrl->SetSelection(0);
    else if(holder->lattice==LAT_HEXA) lattice_ctrl->SetSelection(1);
    ctrl_sizer->Add(lattice_ctrl,std_flags);
    
    wxString prof_choices[]={"Linear","S-Curve","Gaussian","Bezier"};
    prof_ctrl=new wxRadioBox(ctrl_panel,wxID_ANY,"Profile",wxDefaultPosition,wxDefaultSize,3,prof_choices,1);
         if(holder->profile==PROF_LINEAR) prof_ctrl->SetSelection(0);
    else if(holder->profile==PROF_S_CURVE) prof_ctrl->SetSelection(1);
    else if(holder->profile==PROF_GAUSSIAN) prof_ctrl->SetSelection(2);
    else if(holder->profile==PROF_BEZIER) prof_ctrl->SetSelection(3);
    ctrl_sizer->Add(prof_ctrl,std_flags);
    
    top_radius_ctrl=new LengthSelector(ctrl_panel,"Top Radius",holder->top_radius,true);
    bottom_radius_ctrl=new LengthSelector(ctrl_panel,"Bottom Radius",holder->bottom_radius,true);
    
    top_radius_ctrl->set_unit("nm");
    bottom_radius_ctrl->set_unit("nm");
    
    ctrl_sizer->Add(top_radius_ctrl,std_flags);
    ctrl_sizer->Add(bottom_radius_ctrl,std_flags);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display
    
    graph=new Graph(splitter);
    
    // Ok/Cancel
    
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    top_sizer->Add(splitter,wxSizerFlags(1).Expand());
    
    wxBoxSizer *close_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *ok_btn=new wxButton(top_panel,wxID_ANY,"Ok");
    wxButton *cancel_btn=new wxButton(top_panel,wxID_ANY,"Cancel");
    
    close_sizer->Add(ok_btn);
    close_sizer->Add(cancel_btn);
    
    top_sizer->Add(close_sizer,wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL,3));
    
    top_panel->SetSizer(top_sizer);
    
    // Bindings
    
    height_ctrl->Bind(EVT_LENGTH_SELECTOR,&GeoGradientDesigner::evt_profile,this);
    periodicity_ctrl->Bind(EVT_LENGTH_SELECTOR,&GeoGradientDesigner::evt_profile,this);
    top_radius_ctrl->Bind(EVT_LENGTH_SELECTOR,&GeoGradientDesigner::evt_profile,this);
    bottom_radius_ctrl->Bind(EVT_LENGTH_SELECTOR,&GeoGradientDesigner::evt_profile,this);
    cancel_btn->Bind(wxEVT_BUTTON,&GeoGradientDesigner::evt_cancel,this);
    ok_btn->Bind(wxEVT_BUTTON,&GeoGradientDesigner::evt_ok,this);
    prof_ctrl->Bind(wxEVT_RADIOBOX,&GeoGradientDesigner::evt_profile,this);
    
    // Wrapping up
    
    splitter->SplitVertically(ctrl_panel,graph,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
    
    recompute();
    
    ShowModal();
}

GeoGradientDesigner::~GeoGradientDesigner()
{
}

void GeoGradientDesigner::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void GeoGradientDesigner::evt_ok(wxCommandEvent &event)
{
    int lat_selection=lattice_ctrl->GetSelection();
    int prof_selection=prof_ctrl->GetSelection();
    
         if(lat_selection==0) holder->lattice=LAT_SQUARE;
    else if(lat_selection==1) holder->lattice=LAT_HEXA;
    
         if(prof_selection==0) holder->profile=PROF_LINEAR;
    else if(prof_selection==1) holder->profile=PROF_S_CURVE;
    else if(prof_selection==2) holder->profile=PROF_GAUSSIAN;
    else if(prof_selection==3) holder->profile=PROF_BEZIER;
    
    holder->height=height_ctrl->get_length();
    holder->periodicity=periodicity_ctrl->get_length();
    holder->top_radius=top_radius_ctrl->get_length();
    holder->bottom_radius=bottom_radius_ctrl->get_length();
    
    Close();
}

void GeoGradientDesigner::evt_profile(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void GeoGradientDesigner::recompute()
{
    int l,Nl=2001;
    double x=0;
    double w=periodicity_ctrl->get_length();
    double h=height_ctrl->get_length();
    double r1=top_radius_ctrl->get_length();
    double r2=bottom_radius_ctrl->get_length();
    
    double Dx=w/(Nl-1.0);
    
    prof_x.resize(Nl);
    prof_val.resize(Nl);
    
    for(l=0;l<Nl;l++)
    {
        x=l*Dx;
        prof_x[l]=x;
    }
    
    double v1,v2;
    int prof_select=prof_ctrl->GetSelection();
    
         if(prof_select==0)
    {
        for(l=0;l<Nl;l++)
        {
            double &x=prof_x[l];
            
            if(x<r1) v1=h;
            else v1=h*(1.0-(x-r1)/(r2-r1));
            if(w-x<r1) v2=h;
            else v2=h*(1.0-(w-x-r1)/(r2-r1));
            
            prof_val[l]=var_max(0.0,v1,v2);
        }
    }
    else if(prof_select==1)
    {
        for(l=0;l<Nl;l++)
        {
            v1=h*s_curve(prof_x[l],r2,r1);
            v2=h*s_curve(prof_x[l],w-r2,w-r1);
            
            prof_val[l]=std::max(v1,v2);
        }
    }
    else if(prof_select==2)
    {
        for(l=0;l<Nl;l++)
        {
            double &x=prof_x[l];
            
            double a1=(x-r1)/(r2-r1);
            double a2=(w-x-r1)/(r2-r1);
            
            if(x<r1) v1=h;
            else v1=h*std::exp(-a1*a1);
            if(w-x<r1) v2=h;
            else v2=h*std::exp(-a2*a2);
            
            prof_val[l]=std::max(v1,v2);
        }
    }
    else if(prof_select==3)
    {
    }
    
    graph->clear_graph();
    graph->add_external_data(&prof_x,&prof_val,1.0,0,0);
    
    graph->set_scale(0,w,0,h);
    graph->Refresh();
}

//######################
//   GeoGradientFrame
//######################

GeoGradientFrame::GeoGradientFrame(wxString const &title)
    :BaseFrame(title),
     spectral(true),
     curr_lambda(400e-9),
     curr_angle(0)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    wxScrolledWindow *ctrl_panel=new wxScrolledWindow(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxString computation_choices[]={"Spectral","Angular"};
    computation_ctrl=new wxRadioBox(ctrl_panel,wxID_ANY,"Computation",wxDefaultPosition,wxDefaultSize,2,computation_choices);
    ctrl_sizer->Add(computation_ctrl,std_flags);
    
    spectrum=new SpectrumSelector(ctrl_panel,400e-9,800e-9,401,true);
    ctrl_sizer->Add(spectrum,std_flags);
    
    angles_ctrl=new NamedTextCtrl<int>(ctrl_panel,"N Angles",901,true);
    ctrl_sizer->Add(angles_ctrl,std_flags);
    
    superstrate=new MiniMaterialSelector(ctrl_panel,nullptr,"Superstrate");
    substrate=new MiniMaterialSelector(ctrl_panel,nullptr,"Substrate");
    
    ctrl_sizer->Add(superstrate,std_flags);
    ctrl_sizer->Add(substrate,std_flags);
    
    wxButton *profile_btn=new wxButton(ctrl_panel,wxID_ANY,"Set Profile");
    ctrl_sizer->Add(profile_btn,std_flags);
    
    layers_ctrl=new NamedTextCtrl<int>(ctrl_panel,"N Layers",15,true);
    ctrl_sizer->Add(layers_ctrl,std_flags);
    
    wxStaticBoxSizer *models_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Models");
    
    max_gar_1_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Maxwell-Garnett 1");
    max_gar_2_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Maxwell-Garnett 2");
    bruggeman_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Bruggeman");
    looyenga_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Looyenga");
    sum_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Weighted Sum");
    sum_inv_ctrl=new wxCheckBox(ctrl_panel,wxID_ANY,"Weighted Sum Inv");
    
    max_gar_1_ctrl->SetValue(true);
    max_gar_2_ctrl->SetValue(true);
    bruggeman_ctrl->SetValue(true);
    looyenga_ctrl->SetValue(true);
    sum_ctrl->SetValue(true);
    sum_inv_ctrl->SetValue(true);
    
    models_sizer->Add(max_gar_1_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    models_sizer->Add(max_gar_2_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    models_sizer->Add(bruggeman_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    models_sizer->Add(looyenga_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    models_sizer->Add(sum_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    models_sizer->Add(sum_inv_ctrl,wxSizerFlags().Expand().Border(wxBOTTOM,2));
    
    ctrl_sizer->Add(models_sizer,std_flags);
    
    wxButton *export_btn=new wxButton(ctrl_panel,wxID_ANY,"Export");
    wxButton *export_index_btn=new wxButton(ctrl_panel,wxID_ANY,"Export Index");
    
    ctrl_sizer->Add(export_btn,std_flags);
    ctrl_sizer->Add(export_index_btn,std_flags);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    graph=new Graph(display_panel);
    display_sizer->Add(graph,wxSizerFlags(1).Expand());
//    graph->switch_dark();
    
    ctrl_slider=new SliderDisplay(display_panel,901,0,90.0,1.0,"°");
    display_sizer->Add(ctrl_slider,wxSizerFlags().Expand().Border(wxALL,3));
    
    display_panel->SetSizer(display_sizer);
    
    // Bindings
    
    angles_ctrl->Bind(EVT_NAMEDTXTCTRL,&GeoGradientFrame::evt_angles,this);
    bruggeman_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    computation_ctrl->Bind(wxEVT_RADIOBOX,&GeoGradientFrame::evt_mode_switch,this);
    ctrl_slider->Bind(wxEVT_SLIDER,&GeoGradientFrame::evt_slider,this);
    export_btn->Bind(wxEVT_BUTTON,&GeoGradientFrame::evt_export,this);
    export_index_btn->Bind(wxEVT_BUTTON,&GeoGradientFrame::evt_export_index,this);
    layers_ctrl->Bind(EVT_NAMEDTXTCTRL,&GeoGradientFrame::evt_layers,this);
    looyenga_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    max_gar_1_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    max_gar_2_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    profile_btn->Bind(wxEVT_BUTTON,&GeoGradientFrame::evt_set_profile,this);
    spectrum->Bind(EVT_SPECTRUM_SELECTOR,&GeoGradientFrame::evt_spectrum,this);
    sum_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    sum_inv_ctrl->Bind(wxEVT_CHECKBOX,&GeoGradientFrame::evt_models,this);
    substrate->Bind(EVT_MINIMAT_SELECTOR,&GeoGradientFrame::evt_structure_change,this);
    superstrate->Bind(EVT_MINIMAT_SELECTOR,&GeoGradientFrame::evt_structure_change,this);
    
    // Wrapping up
    
    spectrum->get_spectrum(lambda);
    angle.resize(901);
    angle_rad.resize(901);
    for(int a=0;a<901;a++)
    {
        angle[a]=0.1*a;
        angle_rad[a]=Degree(angle[a]);
    }
    
    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetScrollbars(10,10,50,50);
    ctrl_panel->FitInside();
}

void GeoGradientFrame::evt_angles(wxCommandEvent &event)
{
    int Na=angles_ctrl->get_value();
    
    if(Na<2)
    {
        Na=2;
        angles_ctrl->set_value(Na);
    }
    
    angle.resize(Na);
    angle_rad.resize(Na);
    for(int i=0;i<Na;i++)
    {
        angle[i]=90.0*i/(Na-1.0);
        angle_rad[i]=Degree(angle[i]);
    }
    
    if(spectral) switch_slider_spectral();
    
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_export(wxCommandEvent &event)
{
    
    wxString wxfname=wxFileSelector("Select the file to save to:",
                                    wxEmptyString,
                                    wxEmptyString,
                                    wxEmptyString, 
                                    wxFileSelectorDefaultWildcardStr,
                                    wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if(wxfname.size()==0) return;
    
    std::string fname=wxfname.ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    unsigned int i;
    
    for(i=0;i<min_R_TE.size();i++)
    {
        if(spectral) file<<lambda[i]<<" ";
        else file<<angle[i]<<" ";
        
        file<<min_R_TE[i]<<" "
            <<max_R_TE[i]<<" "
            <<min_R_TM[i]<<" "
            <<max_R_TM[i]<<" "
            <<min_T_TE[i]<<" "
            <<max_T_TE[i]<<" "
            <<min_T_TM[i]<<" "
            <<max_T_TM[i]<<" "<<std::endl;
    }
    
    event.Skip();
}

void GeoGradientFrame::evt_export_index(wxCommandEvent &event)
{
    wxString wxfname=wxFileSelector("Select the file to save to:",
                                    wxEmptyString,
                                    wxEmptyString,
                                    wxEmptyString, 
                                    wxFileSelectorDefaultWildcardStr,
                                    wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if(wxfname.size()==0) return;
    
    int m,l;
    unsigned int i;
    AngleOld ang_rad;
    
    int Nl=layers_ctrl->get_value();
    
    double height=profile_holder.height;
    double Dz=height/Nl;
    
    std::vector<double> ratio(Nl),layers_height(Nl);
    
    for(l=0;l<Nl;l++)
    {
        double z=Dz*(l+0.5);
        ratio[Nl-l-1]=profile_holder.get_ratio(z);
        layers_height[l]=Dz;
    }
    
    Imdouble eps_sub,eps_sup,eps_eff,n_sub,n_sup,n_eff;
    
    bool comp_max_gar_1=max_gar_1_ctrl->GetValue();
    bool comp_max_gar_2=max_gar_2_ctrl->GetValue();
    bool comp_bruggeman=bruggeman_ctrl->GetValue();
    bool comp_looyenga=looyenga_ctrl->GetValue();
    bool comp_sum=sum_ctrl->GetValue();
    bool comp_sum_inv=sum_inv_ctrl->GetValue();
    
    std::string fname_=wxfname.ToStdString();
    
    for(m=0;m<6;m++)
    {
        Imdouble (*tmp_model)(Imdouble,Imdouble,double,double)=&effmodel_maxwell_garnett_1;
        std::string fname(fname_);
        
             if(m==0)
        {
            tmp_model=&effmodel_maxwell_garnett_1;
            fname.append("_mg1");
            
            if(!comp_max_gar_1) continue;
        }
        else if(m==1)
        {
            tmp_model=&effmodel_maxwell_garnett_2;
            fname.append("_mg2");
            
            if(!comp_max_gar_2) continue;
        }
        else if(m==2)
        {
            tmp_model=&effmodel_bruggeman;
            fname.append("_bg");
            
            if(!comp_bruggeman) continue;
        }
        else if(m==3)
        {
            tmp_model=&effmodel_looyenga;
            fname.append("_lo");
            
            if(!comp_looyenga) continue;
        }
        else if(m==4)
        {
            tmp_model=&effmodel_sum;
            fname.append("_sum");
            
            if(!comp_sum) continue;
        }
        else if(m==5)
        {
            tmp_model=&effmodel_sum_inv;
            fname.append("_isum");
            
            if(!comp_sum_inv) continue;
        }
        
        std::ofstream file(fname,std::ios::out|std::ios::trunc|std::ios::binary);
        
        file<<0<<" ";
        for(i=0;i<lambda.size();i++)
        {
            file<<lambda[i]<<" "<<lambda[i];
            if(i!=lambda.size()-1) file<<" ";
        }
        file<<std::endl;
        
        for(l=0;l<Nl;l++)
        {
            for(i=0;i<lambda.size();i++)
            {
                double w=m_to_rad_Hz(lambda[i]);
                
                n_sup=superstrate->get_n(w);
                n_sub=substrate->get_n(w);
                
                eps_sup=n_sup*n_sup;
                eps_sub=n_sub*n_sub;
                
                eps_eff=(*tmp_model)(eps_sup,eps_sub,1-ratio[l],ratio[l]);
                
                n_eff=std::sqrt(eps_eff);
                
                if(i==0) file<<Dz*(l+0.5)<<" ";
                file<<n_eff.real()<<" "<<n_eff.imag();
                if(i!=lambda.size()-1) file<<" ";
            }
            
            if(l!=Nl-1) file<<std::endl;
        }
        
        file.close();
        
    }
    
    event.Skip();
}

void GeoGradientFrame::evt_layers(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_mode_switch(wxCommandEvent &event)
{
    if(computation_ctrl->GetSelection()==0) spectral=true;
    else spectral=false;
    
    if(spectral) switch_slider_spectral();
    else switch_slider_angular();
    
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_models(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_set_profile(wxCommandEvent &event)
{
    int sx,sy;
    
    GetSize(&sx,&sy);
    
    GeoGradientDesigner dialog(&profile_holder,wxPoint(0.1*sx,0.1*sy),wxSize(0.8*sx,0.8*sy));
        
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_slider(wxCommandEvent &event)
{
    if(spectral) curr_angle=ctrl_slider->get_value();
    else curr_lambda=ctrl_slider->get_value();
    
    recompute();
}

void GeoGradientFrame::evt_spectrum(wxCommandEvent &event)
{
    if(spectral) switch_slider_spectral();
    else switch_slider_angular();
    
    int Nl=spectrum->get_Np();
    double lambda_min=spectrum->get_lambda_min();
    double lambda_max=spectrum->get_lambda_max();
    
    lambda.resize(Nl);
    
    for(int l=0;l<Nl;l++)
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
    
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::evt_structure_change(wxCommandEvent &event)
{
    recompute();
    
    event.Skip();
}

void GeoGradientFrame::switch_slider_angular()
{
    double scale;
    std::string unit;
    
    get_unit_u(spectrum->get_lambda_min(),"m",unit,scale);
    
    ctrl_slider->set_N(spectrum->get_Np());
    ctrl_slider->set_scale(scale);
    ctrl_slider->set_unit(unit);
    ctrl_slider->set_range(spectrum->get_lambda_min(),spectrum->get_lambda_max());
    ctrl_slider->set_value(curr_lambda);
    
    curr_lambda=ctrl_slider->get_value();
}

void GeoGradientFrame::switch_slider_spectral()
{
    ctrl_slider->set_N(angles_ctrl->get_value());
    ctrl_slider->set_scale(1.0);
    ctrl_slider->set_unit("°");
    ctrl_slider->set_range(0.0,90.0);
    ctrl_slider->set_value(curr_angle);
    
    curr_angle=ctrl_slider->get_value();
}

void GeoGradientFrame::recompute()
{
    int m,l;
    unsigned int i;
    AngleOld ang_rad;
    
    int Nl=layers_ctrl->get_value();
    
    ml.set_N_layers(Nl);
    
    double height=profile_holder.height;
    double Dz=height/Nl;
    
    std::vector<double> ratio(Nl),layers_height(Nl);
    
    for(l=0;l<Nl;l++)
    {
        double z=Dz*(l+0.5);
        ratio[Nl-l-1]=profile_holder.get_ratio(z);
        layers_height[l]=Dz;
    }
    
    Imdouble eps_sub,eps_sup,eps_eff,n_sub,n_sup;
    
    bool comp_max_gar_1=max_gar_1_ctrl->GetValue();
    bool comp_max_gar_2=max_gar_2_ctrl->GetValue();
    bool comp_bruggeman=bruggeman_ctrl->GetValue();
    bool comp_looyenga=looyenga_ctrl->GetValue();
    bool comp_sum=sum_ctrl->GetValue();
    bool comp_sum_inv=sum_inv_ctrl->GetValue();
    
    double R_TE,T_TE,A_TE;
    double R_TM,T_TM,A_TM;
        
    if(spectral)
    {
        min_R_TE.resize(lambda.size());
        max_R_TE.resize(lambda.size());
        min_T_TE.resize(lambda.size());
        max_T_TE.resize(lambda.size());
        
        min_R_TM.resize(lambda.size());
        max_R_TM.resize(lambda.size());
        min_T_TM.resize(lambda.size());
        max_T_TM.resize(lambda.size());
        
        ml.set_angle(Degree(curr_angle));
        
        for(i=0;i<lambda.size();i++)
        {
            double w=m_to_rad_Hz(lambda[i]);
            
            n_sup=superstrate->get_n(w);
            n_sub=substrate->get_n(w);
            
            eps_sup=n_sup*n_sup;
            eps_sub=n_sub*n_sub;
            
            ml.set_lambda(lambda[i]);
            ml.set_environment(n_sup,n_sub);
            
            bool init_set=false;
            
            for(m=0;m<6;m++)
            {
                Imdouble (*tmp_model)(Imdouble,Imdouble,double,double)=&effmodel_maxwell_garnett_1;
                
                     if(m==0)
                {
                    tmp_model=&effmodel_maxwell_garnett_1;
                    if(!comp_max_gar_1) continue;
                }
                else if(m==1)
                {
                    tmp_model=&effmodel_maxwell_garnett_2;
                    if(!comp_max_gar_2) continue;
                }
                else if(m==2)
                {
                    tmp_model=&effmodel_bruggeman;
                    if(!comp_bruggeman) continue;
                }
                else if(m==3)
                {
                    tmp_model=&effmodel_looyenga;
                    if(!comp_looyenga) continue;
                }
                else if(m==4)
                {
                    tmp_model=&effmodel_sum;
                    if(!comp_sum) continue;
                }
                else if(m==5)
                {
                    tmp_model=&effmodel_sum_inv;
                    if(!comp_sum_inv) continue;
                }
                
                for(l=0;l<Nl;l++)
                {
                    eps_eff=(*tmp_model)(eps_sup,eps_sub,1-ratio[l],ratio[l]);
                    ml.set_layer(l,layers_height[l],std::sqrt(eps_eff));
                }
                
                ml.compute_power(R_TE,T_TE,A_TE,
                                 R_TM,T_TM,A_TM);
                
                if(!init_set)
                {
                    max_R_TE[i]=min_R_TE[i]=R_TE;
                    max_T_TE[i]=min_T_TE[i]=T_TE;
                    
                    max_R_TM[i]=min_R_TM[i]=R_TM;
                    max_T_TM[i]=min_T_TM[i]=T_TM;
                    
                    init_set=true;
                }
                else
                {
                    max_R_TE[i]=std::max(max_R_TE[i],R_TE);
                    min_R_TE[i]=std::min(min_R_TE[i],R_TE);
                    
                    max_T_TE[i]=std::max(max_T_TE[i],T_TE);
                    min_T_TE[i]=std::min(min_T_TE[i],T_TE);
                    
                    max_R_TM[i]=std::max(max_R_TM[i],R_TM);
                    min_R_TM[i]=std::min(min_R_TM[i],R_TM);
                    
                    max_T_TM[i]=std::max(max_T_TM[i],T_TM);
                    min_T_TM[i]=std::min(min_T_TM[i],T_TM);
                }
            }
        }
        
        graph->clear_graph();
        
        graph->add_external_data(&lambda,&min_R_TE,1.0,0,0,"min/max R_TE");
        graph->add_external_data(&lambda,&max_R_TE,1.0,0,0);
        
        graph->add_external_data(&lambda,&min_T_TE,0,0,1.0,"min/max T_TE");
        graph->add_external_data(&lambda,&max_T_TE,0,0,1.0);
        
        graph->add_external_data(&lambda,&min_R_TM,1.0,0.5,0,"min/max R_TM");
        graph->add_external_data(&lambda,&max_R_TM,1.0,0.5,0);
        
        graph->add_external_data(&lambda,&min_T_TM,0,0.7,1.0,"min/max T_TM");
        graph->add_external_data(&lambda,&max_T_TM,0,0.7,1.0);
        
        graph->set_scale(spectrum->get_lambda_min(),spectrum->get_lambda_max(),-0.05,1.05);
        graph->Refresh();
    }
    else
    {
        unsigned int Na=angle.size();
        
        min_R_TE.resize(Na); max_R_TE.resize(Na);
        min_T_TE.resize(Na); max_T_TE.resize(Na);
        
        min_R_TM.resize(Na); max_R_TM.resize(Na);
        min_T_TM.resize(Na); max_T_TM.resize(Na);
        
        double w=m_to_rad_Hz(curr_lambda);
        
        ml.set_lambda(curr_lambda);
        
        n_sup=superstrate->get_n(w);
        n_sub=substrate->get_n(w);
        
        eps_sup=n_sup*n_sup;
        eps_sub=n_sub*n_sub;
        
        ml.set_environment(n_sup,n_sub);
        
        std::vector<double> R_TE_buf(Na),T_TE_buf(Na),
                            R_TM_buf(Na),T_TM_buf(Na);
        
        bool init_set=false;
        
        for(m=0;m<6;m++)
        {
            Imdouble (*tmp_model)(Imdouble,Imdouble,double,double)=&effmodel_maxwell_garnett_1;
            
                 if(m==0)
            {
                tmp_model=&effmodel_maxwell_garnett_1;
                if(!comp_max_gar_1) continue;
            }
            else if(m==1)
            {
                tmp_model=&effmodel_maxwell_garnett_2;
                if(!comp_max_gar_2) continue;
            }
            else if(m==2)
            {
                tmp_model=&effmodel_bruggeman;
                if(!comp_bruggeman) continue;
            }
            else if(m==3)
            {
                tmp_model=&effmodel_looyenga;
                if(!comp_looyenga) continue;
            }
            else if(m==4)
            {
                tmp_model=&effmodel_sum;
                if(!comp_sum) continue;
            }
            else if(m==5)
            {
                tmp_model=&effmodel_sum_inv;
                if(!comp_sum_inv) continue;
            }
            
            for(l=0;l<Nl;l++)
            {
                eps_eff=(*tmp_model)(eps_sup,eps_sub,1-ratio[l],ratio[l]);
                ml.set_layer(l,layers_height[l],std::sqrt(eps_eff));
            }
            
            for(i=0;i<Na;i++)
            {
                ml.set_angle(angle_rad[i]);
            
                ml.compute_power(R_TE_buf[i],T_TE_buf[i],A_TE,
                                 R_TM_buf[i],T_TM_buf[i],A_TM);
            }
            
            if(!init_set)
            {
                for(i=0;i<Na;i++)
                {
                    min_R_TE[i]=max_R_TE[i]=R_TE_buf[i];
                    min_R_TM[i]=max_R_TM[i]=R_TM_buf[i];
                    
                    min_T_TE[i]=max_T_TE[i]=T_TE_buf[i];
                    min_T_TM[i]=max_T_TM[i]=T_TM_buf[i];
                }
                
                init_set=true;
            }
            else
            {
                for(i=0;i<Na;i++)
                {
                    min_R_TE[i]=std::min(min_R_TE[i],R_TE_buf[i]);
                    max_R_TE[i]=std::max(max_R_TE[i],R_TE_buf[i]);
                    
                    min_T_TE[i]=std::min(min_T_TE[i],T_TE_buf[i]);
                    max_T_TE[i]=std::max(max_T_TE[i],T_TE_buf[i]);
                    
                    min_R_TM[i]=std::min(min_R_TM[i],R_TM_buf[i]);
                    max_R_TM[i]=std::max(max_R_TM[i],R_TM_buf[i]);
                    
                    min_T_TM[i]=std::min(min_T_TM[i],T_TM_buf[i]);
                    max_T_TM[i]=std::max(max_T_TM[i],T_TM_buf[i]);
                }
            }
        }
        
        graph->clear_graph();
        
        graph->add_external_data(&angle,&min_R_TE,1.0,0,0,"min/max R_TE");
        graph->add_external_data(&angle,&max_R_TE,1.0,0,0);
        
        graph->add_external_data(&angle,&min_T_TE,0,0,1.0,"min/max T_TE");
        graph->add_external_data(&angle,&max_T_TE,0,0,1.0);
        
        graph->add_external_data(&angle,&min_R_TM,1.0,0.5,0,"min/max R_TM");
        graph->add_external_data(&angle,&max_R_TM,1.0,0.5,0);
        
        graph->add_external_data(&angle,&min_T_TM,0,0.7,1.0,"min/max T_TM");
        graph->add_external_data(&angle,&max_T_TM,0,0.7,1.0);
        
        graph->set_scale(0,90,-0.05,1.05);
        graph->Refresh();
    }
}
