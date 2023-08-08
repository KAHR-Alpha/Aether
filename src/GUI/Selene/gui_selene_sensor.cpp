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

RayCounter::RayCounter()
    :owner(false),
     object(nullptr),
     N_faces(0),
     computation_type(RC_COUNTING),
     spectral_mode(SP_FULL),
     has_lambda(false),
     has_source(false),
     has_path(false),
     has_generation(false),
     has_phase(false),
     has_polarization(false)
{
}

double RayCounter::compute_directionnal_spread()
{
    double x_avg=0,
           y_avg=0,
           z_avg=0,
           x_stddev=0,
           y_stddev=0,
           z_stddev=0;
    
    std::size_t N=obj_dir.size();
    
    for(std::size_t i=0;i<N;i++)
    {
        x_avg+=obj_dir[i].x;
        y_avg+=obj_dir[i].y;
        z_avg+=obj_dir[i].z;
    }
    
    x_avg/=N;
    y_avg/=N;
    z_avg/=N;
    
    for(std::size_t i=0;i<N;i++)
    {
        double xs=obj_dir[i].x-x_avg;
        double ys=obj_dir[i].y-y_avg;
        double zs=obj_dir[i].z-z_avg;
        
        x_stddev+=xs*xs;
        y_stddev+=ys*ys;
        z_stddev+=zs*zs;
    }
    
    x_stddev/=N;
    y_stddev/=N;
    z_stddev/=N;
    
    return std::sqrt(x_stddev)+std::sqrt(y_stddev)+std::sqrt(z_stddev);
}

double RayCounter::compute_spatial_spread()
{
    double x_avg=0,
           y_avg=0,
           z_avg=0,
           x_stddev=0,
           y_stddev=0,
           z_stddev=0;
    
    std::size_t N=obj_dir.size();
    
    for(std::size_t i=0;i<N;i++)
    {
        x_avg+=obj_inter[i].x;
        y_avg+=obj_inter[i].y;
        z_avg+=obj_inter[i].z;
    }
    
    x_avg/=N;
    y_avg/=N;
    z_avg/=N;
    
    for(std::size_t i=0;i<N;i++)
    {
        double xs=obj_inter[i].x-x_avg;
        double ys=obj_inter[i].y-y_avg;
        double zs=obj_inter[i].z-z_avg;
        
        x_stddev+=xs*xs;
        y_stddev+=ys*ys;
        z_stddev+=zs*zs;
    }
    
    x_stddev/=N;
    y_stddev/=N;
    z_stddev/=N;
    
    return std::sqrt(x_stddev)+std::sqrt(y_stddev)+std::sqrt(z_stddev);
}

void RayCounter::initialize()
{
    // Power unit
    
    std::string sensor_values=loader.header[1];
    
    std::vector<std::string> sv_split;
    split_string(sv_split,sensor_values);
    
    ray_unit=std::stod(sv_split[sv_split.size()-1]);
    chk_var(ray_unit);
    
    // Sensor type
    
    std::string sensor_content_linear=loader.header[2];
    
    std::vector<std::string> sensor_content;
    split_string(sensor_content,sensor_content_linear);
    
    int column_offset=0;
    
    if(vector_contains(sensor_content,std::string("wavelength")))
    {
        has_lambda=true;
        lambda_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("source")))
    {
        has_source=true;
        source_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("path")))
    {
        has_path=true;
        path_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("generation")))
    {
        has_generation=true;
        generation_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("length"))) column_offset++;
    if(vector_contains(sensor_content,std::string("phase")))
    {
        has_phase=true;
        phase_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("world_intersection"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("world_direction"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("world_polarization"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("obj_intersection")))
    {
        obj_inter_column=column_offset;
        column_offset+=3;
    }
    else
    {
        std::cerr<<"Invalid ray file "<<sensor_fname<<"\n";
        std::cerr<<"Missing object intersection points\nAborting...\n";
        std::exit(EXIT_FAILURE);
    }
    if(vector_contains(sensor_content,std::string("obj_direction")))
    {
        obj_dir_column=column_offset;
        column_offset+=3;
    }
    if(vector_contains(sensor_content,std::string("obj_polarization")))
    {
        has_polarization=true;
        obj_polar_column=column_offset;
        column_offset+=3;
    }
    if(vector_contains(sensor_content,std::string("obj_face")))
    {
        face_column=column_offset;
    }
    else
    {
        std::cerr<<"Invalid ray file "<<sensor_fname<<"\n";
        std::cerr<<"Missing face intersections\nAborting...\n";
        std::exit(EXIT_FAILURE);
    }
    
    N_faces=object->get_N_faces();
    
    bins.resize(N_faces);
    
    Du.resize(N_faces);
    Dv.resize(N_faces);
    
    Nu.resize(N_faces);
    Nv.resize(N_faces);
    
    for(int i=0;i<N_faces;i++)
    {
        object->default_N_uv(Nu[i],Nv[i],i);
        
        Du[i]=1.0/Nu[i];
        Dv[i]=1.0/Nv[i];
        
        bins[i].init(Nu[i],Nv[i]);
    }
    
    // Loading
    
    int Nl=loader.Nl;
    
    if(has_lambda) lambda.resize(Nl);
    if(has_source) source.resize(Nl);
    if(has_path) path.resize(Nl);
    if(has_generation) generation.resize(Nl);
    if(has_phase) phase.resize(Nl);
    if(has_polarization) obj_polarization.resize(Nl);
    
    obj_inter.resize(Nl);
    obj_dir.resize(Nl);
    face.resize(Nl);
    
    std::vector<double> data;
    
    lambda_max=0;
    lambda_min=std::numeric_limits<double>::max();
    
    for(int i=0;i<Nl;i++)
    {
        loader.load_seq(data);
        
        if(has_lambda)
        {
            lambda[i]=data[lambda_column];
            
            lambda_min=std::min(lambda_min,lambda[i]);
            lambda_max=std::max(lambda_max,lambda[i]);
        }
        
        if(has_polarization)
        {
            obj_polarization[i].x=data[obj_polar_column+0];
            obj_polarization[i].y=data[obj_polar_column+1];
            obj_polarization[i].z=data[obj_polar_column+2];
        }
        
        obj_inter[i].x=data[obj_inter_column+0];
        obj_inter[i].y=data[obj_inter_column+1];
        obj_inter[i].z=data[obj_inter_column+2];
        
        obj_dir[i].x=data[obj_dir_column+0];
        obj_dir[i].y=data[obj_dir_column+1];
        obj_dir[i].z=data[obj_dir_column+2];
        
        face[i]=data[face_column];
    }
}

void RayCounter::set_sensor(Sel::Object *object_)
{
    object=object_;
    sensor_fname=object->get_sensor_file_path();
    
    loader.initialize(sensor_fname.generic_string());
    
    initialize();
}

void RayCounter::set_sensor(std::filesystem::path const &sensor_file)
{
    sensor_fname=sensor_file;
    
    if(object==nullptr)
    {
        owner=true;
        object=new Sel::Object;
    }
    
    // Reading the object geometry from the sensor file
    
    loader.initialize(sensor_fname.generic_string());
    
    std::string object_header=loader.header[0];
    chk_var(object_header);
    
    std::vector<std::string> header_split;
    split_string(header_split,object_header);
    
    std::string object_type=header_split[0];
    std::vector<double> parameters(header_split.size()-1);
    
    for(std::size_t i=1;i<header_split.size();i++)
        parameters[i-1]=std::stod(header_split[i]);
    
    // Setting the object with the proper geometry
    
    if(owner)
    {
             if(object_type=="box") object->set_box(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="cylinder") object->set_cylinder_volume(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="disk") object->set_disk(parameters[0],parameters[1]);
        else if(object_type=="lens") object->set_lens(parameters[0],parameters[1],parameters[2],parameters[3]);
        else if(object_type=="mesh") ;
        else if(object_type=="rectangle") object->set_rectangle(parameters[0],parameters[1]);
        else if(object_type=="parabola") object->set_parabola(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="sphere") object->set_sphere(parameters[0],parameters[1]);
        else if(object_type=="spherical_patch") object->set_spherical_patch(parameters[0],parameters[1]);
        else
        {
            std::cerr<<"Ray analysis error, couldn't set up the object type "<<object_type<<"\nAborting...\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    initialize();
}

void RayCounter::reallocate()
{
    for(int i=0;i<N_faces;i++)
    {
        if(   Nu[i]!=bins[i].L1()
           || Nv[i]!=bins[i].L2())
        {
            bins[i].init(Nu[i],Nv[i]);
            
            Du[i]=1.0/Nu[i];
            Dv[i]=1.0/Nv[i];
        }
    }
}

void RayCounter::update()
{
    reallocate();
    
    for(int i=0;i<N_faces;i++)
        bins[i]=0;
    
    double unit=1.0;
    if(computation_type==RC_POWER) unit=ray_unit;
    
    for(std::size_t i=0;i<obj_inter.size();i++)
    {
        if(has_lambda && spectral_mode!=SP_FULL)
        {
            if(lambda[i]<lambda_min || lambda[i]>lambda_max) continue;
        }
        
        double u,v;
        
        object->xyz_to_uv(u,v,face[i],
                          obj_inter[i].x,
                          obj_inter[i].y,
                          obj_inter[i].z);
        
        int m=u/Du[face[i]];
        int n=v/Dv[face[i]];
        
        Grid2<double> &fbins=bins[face[i]];
        
        if(m>=0 && m<Nu[face[i]] && n>=0 && n<Nv[face[i]])
        {
            fbins(m,n)+=unit;
        }
    }
}

void RayCounter::update_from_file()
{
    reallocate();
    
    for(int i=0;i<N_faces;i++)
        bins[i]=0;
    
    int Nl=loader.Nl;
    
    std::vector<double> data;
    
    for(int i=0;i<Nl;i++)
    {
        loader.load_seq(data);
        
        double x=data[obj_inter_column+0];
        double y=data[obj_inter_column+1];
        double z=data[obj_inter_column+2];
        
        int face=data[face_column];
        
        double u,v;
        
        object->xyz_to_uv(u,v,face,x,y,z);
        
        int m=u/Du[face];
        int n=v/Dv[face];
        
        Grid2<double> &fbins=bins[face];
        
        if(m>=0 && m<Nu[face] && n>=0 && n<Nv[face])
        {
            fbins(m,n)++;
        }
    }
}

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

RayCounterFrame::RayCounterFrame(wxString const &title,RayCounter *counter_)
    :BaseFrame(title),
     linked(false),
     counter(counter_)
{
    if(counter!=nullptr) linked=true;
    else counter=new RayCounter;
    
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
        case RC_COLOR: computation_type->SetSelection(2); break;
        case RC_COUNTING: computation_type->SetSelection(0); break;
        case RC_POWER: computation_type->SetSelection(1); break;
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
            case SP_FULL: spectral_mode->SetSelection(0); break;
            case SP_MINMAX: spectral_mode->SetSelection(1); break;
            case SP_WINDOW: spectral_mode->SetSelection(2); break;
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
    
         if(selection==0) counter->computation_type=RC_COUNTING;
    else if(selection==1) counter->computation_type=RC_POWER;
    
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
    
         if(selection==0) counter->spectral_mode=SP_FULL;
    else if(selection==1)
    {
        counter->spectral_mode=SP_MINMAX;
        counter->lambda_min=lambda_min->get_lambda();
        counter->lambda_max=lambda_max->get_lambda();
    }
    else if(selection==2)
    {
        counter->spectral_mode=SP_WINDOW;
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
