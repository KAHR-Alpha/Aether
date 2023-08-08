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

#include <gui_selene_sensor.h>

#include <wx/splitter.h>

namespace SelGUI
{

//######################
//   RayCounterFrame
//######################

enum
{
    MENU_LOAD,
    MENU_EXIT,
    MENU_GRADIENT_BW,
    MENU_GRADIENT_JET,
    MENU_GRADIENT_JET_EXT,
};

RayCounterFrame::RayCounterFrame(wxString const &title,Sel::RayCounter *counter_)
    :BaseFrame(title),
     linked(false),
     counter(counter_)
{
    if(counter!=nullptr) linked=true;
    else counter=new Sel::RayCounter;
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    // Left Panel
    
    left_panel=new wxPanel(splitter);
    wxBoxSizer *left_sizer=new wxBoxSizer(wxVERTICAL);
    left_panel->SetSizer(left_sizer);
    
    // - Faces switch
    
    wxStaticBoxSizer *face_sizer=new wxStaticBoxSizer(wxHORIZONTAL,left_panel,"Face");
    
    wxButton *face_prev_btn=new wxButton(face_sizer->GetStaticBox(),0,"<",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    face_number=new NamedTextCtrl<int>(face_sizer->GetStaticBox(),"",0);
    wxButton *face_next_btn=new wxButton(face_sizer->GetStaticBox(),1,">",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    face_prev_btn->Bind(wxEVT_BUTTON,&RayCounterFrame::evt_face_switch,this);
    face_next_btn->Bind(wxEVT_BUTTON,&RayCounterFrame::evt_face_switch,this);
    
    face_number->Bind(EVT_NAMEDTXTCTRL,&RayCounterFrame::evt_face_switch_text,this);
    
    face_sizer->Add(face_prev_btn);
    face_sizer->Add(face_number,wxSizerFlags(1));
    face_sizer->Add(face_next_btn);
    
    left_sizer->Add(face_sizer,wxSizerFlags().Expand());
    
    // - Faces Controls
    
    ctrl_panel=new wxScrolledWindow(left_panel);
    
    ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    ctrl_panel->SetSizer(ctrl_sizer);
    
    if(linked) allocate_face_controls();
    
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
    
    left_sizer->Add(ctrl_panel,wxSizerFlags(1).Expand());
    
    // - Refresh options
    
//    wxButton *update_btn=new wxButton(left_panel,wxID_ANY,"Refresh");
//    update_btn->Bind(wxEVT_BUTTON,&RayCounterFrame::evt_update,this);
//    left_sizer->Add(update_btn,wxSizerFlags().Expand());
    
    // - Export
    
    wxButton *export_btn=new wxButton(left_panel,wxID_ANY,"Export");
    export_btn->Bind(wxEVT_BUTTON,&RayCounterFrame::evt_export,this);
    left_sizer->Add(export_btn,wxSizerFlags().Expand());
    
    // Display
    
    display=new GL_2D_graph(splitter);
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    wxMenu *display_menu=new wxMenu();
    
    if(!linked)
    {
        file_menu->Append(MENU_LOAD,"Load");
        file_menu->AppendSeparator();
    }
    file_menu->Append(MENU_EXIT,"Exit");
    
    display_menu->AppendRadioItem(MENU_GRADIENT_BW,"BW Gradient");
    display_menu->AppendRadioItem(MENU_GRADIENT_JET,"Jet Gradient");
    display_menu->AppendRadioItem(MENU_GRADIENT_JET_EXT,"Extended Jet Gradient");
    
    menu_bar->Append(file_menu,"File");
    menu_bar->Append(display_menu,"Display");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    Bind(wxEVT_MENU,&RayCounterFrame::evt_menu,this);
    
    //
    
    splitter->SplitVertically(left_panel,display,250);
    splitter->SetMinimumPaneSize(20);
}

RayCounterFrame::~RayCounterFrame()
{
    if(linked) delete counter;
}

void RayCounterFrame::allocate_face_controls()
{
    // Cleaning up
    
    ctrl_sizer->Clear(true);
    
    x_resolution.clear();
    y_resolution.clear();
    
    // Allocation
    
    wxSizerFlags default_flags=wxSizerFlags().Expand();
    
    int N_faces=counter->N_faces;
    
    for(int i=0;i<N_faces;i++)
    {
        NamedTextCtrl<int> *f_x_resolution=new NamedTextCtrl<int>(ctrl_panel,"X Resolution",counter->Nu[i],true);
        NamedTextCtrl<int> *f_y_resolution=new NamedTextCtrl<int>(ctrl_panel,"Y Resolution",counter->Nv[i],true);
        
        f_x_resolution->Bind(EVT_NAMEDTXTCTRL,&RayCounterFrame::evt_resolution,this);
        f_y_resolution->Bind(EVT_NAMEDTXTCTRL,&RayCounterFrame::evt_resolution,this);
        
        ctrl_sizer->Add(f_x_resolution,wxSizerFlags().Expand());
        ctrl_sizer->Add(f_y_resolution,wxSizerFlags().Expand());
        
        x_resolution.push_back(f_x_resolution);
        y_resolution.push_back(f_y_resolution);
    }
    
    // Computation type
    
    wxStaticBoxSizer *computation_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Computation type");
    
    computation_type=new wxChoice(computation_sizer->GetStaticBox(),wxID_ANY);
    
    computation_type->Append("Hit counts");
    computation_type->Append("Power");
//    if(counter->has_lambda) computation_type->Append("Color");
    
    switch(counter->computation_type)
    {
        case Sel::RC_COLOR: computation_type->SetSelection(2); break;
        case Sel::RC_COUNTING: computation_type->SetSelection(0); break;
        case Sel::RC_POWER: computation_type->SetSelection(1); break;
    }
    
    computation_type->Bind(wxEVT_CHOICE,&RayCounterFrame::evt_computation_type,this);
    
    computation_sizer->Add(computation_type,wxSizerFlags().Expand());
    ctrl_sizer->Add(computation_sizer,wxSizerFlags().Expand());
    
    // Wavelength filter
        
    if(counter->has_lambda)
    {
        spectral_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Spectral filter");
        
        spectral_mode=new wxChoice(spectral_sizer->GetStaticBox(),wxID_ANY);
        spectral_mode->Append("Full");
        spectral_mode->Append("Min/Max");
        spectral_mode->Append("Window");
        
        switch(counter->spectral_mode)
        {
            case Sel::SP_FULL: spectral_mode->SetSelection(0); break;
            case Sel::SP_MINMAX: spectral_mode->SetSelection(1); break;
            case Sel::SP_WINDOW: spectral_mode->SetSelection(2); break;
        }
        
        spectral_mode->Bind(wxEVT_CHOICE,&RayCounterFrame::evt_spectral_filter,this);
        spectral_sizer->Add(spectral_mode,default_flags);
        
        double l1=counter->lambda_min;
        double l2=counter->lambda_max;
        chk_var(l1);
        chk_var(l2);
        lambda_min=new WavelengthSelector(spectral_sizer->GetStaticBox(),"Lambda Min",l1);
        lambda_max=new WavelengthSelector(spectral_sizer->GetStaticBox(),"Lambda Max",l2);
        lambda_center=new WavelengthSelector(spectral_sizer->GetStaticBox(),"Lambda Center",0.5*(l1+l2));
        lambda_width=new WavelengthSelector(spectral_sizer->GetStaticBox(),"Lambda Width",l2-l1);
        
        lambda_min->Bind(EVT_WAVELENGTH_SELECTOR,&RayCounterFrame::evt_spectral_filter,this);
        lambda_max->Bind(EVT_WAVELENGTH_SELECTOR,&RayCounterFrame::evt_spectral_filter,this);
        lambda_center->Bind(EVT_WAVELENGTH_SELECTOR,&RayCounterFrame::evt_spectral_filter,this);
        lambda_width->Bind(EVT_WAVELENGTH_SELECTOR,&RayCounterFrame::evt_spectral_filter,this);
        
        spectral_sizer->Add(lambda_min,default_flags);
        spectral_sizer->Add(lambda_max,default_flags);
        spectral_sizer->Add(lambda_center,default_flags);
        spectral_sizer->Add(lambda_width,default_flags);
        
        ctrl_sizer->Add(spectral_sizer,default_flags);
    }
    
    // Source filter
    
    update_face_controls();
}

void RayCounterFrame::evt_computation_type(wxCommandEvent &event)
{
    int selection=computation_type->GetSelection();
    
         if(selection==0) counter->computation_type=Sel::RC_COUNTING;
    else if(selection==1) counter->computation_type=Sel::RC_POWER;
    
    counter->update();
    update_graph();
}

void RayCounterFrame::evt_export(wxCommandEvent &event)
{
    std::vector<wxString> choices(3),file_types(3);
    
    choices[0]="Text file"; file_types[0]="Text file (*.txt)|*.txt";
    choices[1]="PNG image"; file_types[1]="PNG image (*.png)|*.png";
    choices[2]="Matlab script"; file_types[2]="MatLab script (*.m)|*.m";
    
    ExportChoiceDialog dialog("Choose an export format",choices,file_types);
    
    if(dialog.choice_ok)
    {
        int choice=dialog.export_type;
        std::filesystem::path fname=dialog.fname;
        
        int face=face_number->get_value();
        Grid2<double> &bins=counter->bins[face];
        
        int Nx=bins.L1();
        int Ny=bins.L2();
        
        switch(choice)
        {
            case 0:
                {
                    std::ofstream file(fname,std::ios::out|std::ios::trunc);
                                        
                    for(int j=0;j<Ny;j++)
                    {
                        for(int i=0;i<Nx;i++)
                        {
                            file<<bins(i,j);
                            if(i!=Nx-1) file<<" ";
                        }
                        if(j!=Ny-1) file<<"\n";
                    }
                }
                
                break;
            case 1:
                G2_to_degra(bins,fname.generic_string());
                break;
            case 2:
                {
                    std::ofstream file(fname,std::ios::out|std::ios::trunc);
                    
                    file<<"function data="<<fname.stem().generic_string()<<"()\n";
                    file<<"\n";
                    file<<"data=[";
                    
                    for(int j=0;j<Ny;j++)
                    {
                        for(int i=0;i<Nx;i++)
                        {
                            file<<bins(i,j);
                            if(i!=Nx-1) file<<",";
                        }
                        if(j!=Ny-1) file<<";";
                    }
                    
                    file<<"];\n";
                    file<<"\n";
                    file<<"pcolor(data)\n";
                    file<<"shading flat\n";
                    file<<"colorbar\n";
                    file<<"\n";
                    file<<"end";
                }
        }
    }
}

void RayCounterFrame::evt_face_switch(wxCommandEvent &event)
{
    int N_faces=counter->N_faces;
    
    if(N_faces==0) return;
    
    int current_face=face_number->get_value();
    
    if(event.GetId()==0)
    {
        if(current_face-1<0) face_number->set_value(N_faces-1);
        else face_number->set_value(current_face-1);
    }
    else
    {
        if(current_face+1>=N_faces) face_number->set_value(0);
        else face_number->set_value(current_face+1);
    }
    
    update_face_controls();
    update_graph();
}

void RayCounterFrame::evt_face_switch_text(wxCommandEvent &event)
{
    if(counter->N_faces==0)
    {
        face_number->set_value(0);
        return;
    }
    
    if(face_number->get_value()>=counter->N_faces)
    {
        face_number->set_value(counter->N_faces-1);
    }
    else if(face_number->get_value()<0)
    {
        face_number->set_value(0);
    }
    
    update_face_controls();
    update_graph();
}

void RayCounterFrame::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
         if(ID==MENU_LOAD) evt_load_project("*","Select a Ray file");
    else if(ID==MENU_EXIT) Close();
    else if(ID==MENU_GRADIENT_BW) display->set_gradient(0);
    else if(ID==MENU_GRADIENT_JET) display->set_gradient(1);
    else if(ID==MENU_GRADIENT_JET_EXT) display->set_gradient(2);
    
    update_graph();
}

void RayCounterFrame::evt_resolution(wxCommandEvent &event)
{
    int face=face_number->get_value();
    
    int Nx=x_resolution[face]->get_value();
    int Ny=y_resolution[face]->get_value();
    
    if(Nx<=0)
    {
        Nx=1;
        x_resolution[face]->set_value(1);
    }
    if(Ny<=0)
    {
        Ny=1;
        y_resolution[face]->set_value(1);
    }
    
    counter->Nu[face]=Nx;
    counter->Nv[face]=Ny;
    
    counter->update();
    update_graph();
}

void RayCounterFrame::evt_spectral_filter(wxCommandEvent &event)
{
    int selection=spectral_mode->GetSelection();
    
         if(selection==0) counter->spectral_mode=Sel::SP_FULL;
    else if(selection==1)
    {
        counter->spectral_mode=Sel::SP_MINMAX;
        counter->lambda_min=lambda_min->get_lambda();
        counter->lambda_max=lambda_max->get_lambda();
    }
    else if(selection==2)
    {
        counter->spectral_mode=Sel::SP_WINDOW;
        counter->lambda_min=lambda_center->get_lambda()-lambda_width->get_lambda()/2.0;
        counter->lambda_max=lambda_center->get_lambda()+lambda_width->get_lambda()/2.0;
    }
    
    update_face_controls();
    counter->update();
    update_graph();
}

void RayCounterFrame::evt_update(wxCommandEvent &event)
{
    chk_var(ctrl_panel->GetSize().x);
    chk_var(ctrl_panel->GetSize().y);
    chk_var(display->GetSize().x);
    chk_var(display->GetSize().y);
    chk_var(GetSize().x);
    chk_var(GetSize().y);
}

void RayCounterFrame::load_project(wxFileName const &fname_)
{
    std::filesystem::path fname=fname_.GetFullPath().ToStdString();
    
    counter->set_sensor(fname);
    counter->update();
    
    allocate_face_controls();
    update_graph();
}

void RayCounterFrame::update_face_controls()
{
    for(std::size_t i=0;i<x_resolution.size();i++)
    {
        x_resolution[i]->Hide();
        y_resolution[i]->Hide();
    }
    
    int current_face=face_number->get_value();
    
    x_resolution[current_face]->Show();
    y_resolution[current_face]->Show();
    
    // Spectral Filter
    
    lambda_min->Hide();
    lambda_max->Hide();
    lambda_center->Hide();
    lambda_width->Hide();
    
    switch(spectral_mode->GetSelection())
    {
        case 1:
            lambda_min->Show();
            lambda_max->Show();
            break;
        case 2:
            lambda_center->Show();
            lambda_width->Show();
            break;
    }
    
    ctrl_panel->Layout();
    ctrl_panel->FitInside();
    left_panel->Layout();
}

void RayCounterFrame::update_graph()
{
    int face=face_number->get_value();
    
    display->data=counter->bins[face];
    
    display->refresh_from_bitmap();
}

}
