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
#include <filehdl.h>
#include <lua_base.h>
#include <lua_interface.h>
#include <lua_optim.h>
#include <lua_selene.h>

#include <gui_selene.h>

#include <map>

extern std::ofstream plog;

namespace SelGUI
{
//########################
//   Ray Bounces Dialog
//########################

RayBouncesDialog::RayBouncesDialog(int max_ray_bounces)
    :wxDialog(nullptr,wxID_ANY,"Choose a value",wxGetApp().default_dialog_origin())
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    ray_bounces=new NamedTextCtrl<int>(this,"Max number of ray bounces: ",max_ray_bounces);
    
    sizer->Add(ray_bounces,wxSizerFlags().Expand().Border(wxALL,3));
    
    SetSizerAndFit(sizer);
    
    ShowModal();
}


//#################
//   SeleneFrame
//#################

enum
{
    MENU_DELETE,
    MENU_PROPERTIES,
    MENU_NEW,
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT,
    MENU_REF_IND,
    MENU_IRF,
    MENU_OPTIMIZE,
    MENU_OPTIMIZATION_TARGETS,
    MENU_BOUNCES
};

SeleneFrame::SeleneFrame(wxString const &title)
    :BaseFrame(title),
     item_count(0),
     max_ray_bounces(200),
     optimize(false),
     optimization_running(false),
     pause_optimization(false),
     optimization_thread(nullptr)
{
    irfs.resize(4);
    
    irfs[0].set_type_fresnel(); irfs[0].name="Fresnel";
    irfs[1].set_type(Sel::IRF_Type::PERF_ABS); irfs[1].name="Perfect Absorber";
    irfs[2].set_type(Sel::IRF_Type::PERF_ANTIREF); irfs[2].name="Perfect Antireflector";
    irfs[3].set_type(Sel::IRF_Type::PERF_MIRROR); irfs[3].name="Perfect Mirror";
    
    // UI
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    gl=new GL_Selene(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - Display
    
    SeleneFrame_RayDisp(ctrl_panel,ctrl_sizer);
        
    // Elements addition
    
    wxBoxSizer *add_sizer=new wxBoxSizer(wxHORIZONTAL);
    add_element_list=new wxChoice(ctrl_panel,wxID_ANY);
    wxButton *add_element_btn=new wxButton(ctrl_panel,wxID_ANY,"Add",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    add_element_list->Append("Box");
    add_element_list->Append("(todo) Cone");
    add_element_list->Append("Cylinder");
    add_element_list->Append("Lens");
    add_element_list->Append("Mesh");
    add_element_list->Append("(todo) Prism");
    add_element_list->Append("Sphere");
    add_element_list->Append("---------------");
    add_element_list->Append("Conic Section");
    add_element_list->Append("Disk");
    add_element_list->Append("Parabola");
    add_element_list->Append("Rectangle");
    add_element_list->Append("Spherical patch");
    add_element_list->Append("---------------");
    add_element_list->Append("Cone");
    add_element_list->Append("Gaussian Beam");
    add_element_list->Append("Lambertian Source");
    add_element_list->Append("Perfect Beam");
    add_element_list->Append("Planar Point Source");
    add_element_list->Append("Point Source");
    
    add_element_list->SetSelection(0);
    add_element_btn->Bind(wxEVT_BUTTON,&SeleneFrame::evt_add_element,this);
    
    add_sizer->Add(add_element_list,wxSizerFlags(1));
    add_sizer->Add(add_element_btn);
    
    ctrl_sizer->Add(add_sizer,wxSizerFlags().Expand());
    
    // - Scene
    
    wxStaticBoxSizer *scene_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Scene");
    
    objects_tree=new wxTreeCtrl(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_HIDE_ROOT);
    
    wxBitmap sun_bmp=ImagesManager::get_bitmap(PathManager::locate_resource("resources/sun_on_16.png").generic_string());
    wxBitmap mesh_bmp=ImagesManager::get_bitmap(PathManager::locate_resource("resources/wireframe_on_16.png").generic_string());
    
    tree_icons=new wxImageList(16,16);
    tree_icons->Add(sun_bmp);
    tree_icons->Add(mesh_bmp);
    
    objects_tree->AssignImageList(tree_icons);
    
    scene_sizer->Add(objects_tree,wxSizerFlags(1).Expand());
    
    ctrl_sizer->Add(scene_sizer,wxSizerFlags(1).Expand());
    
    // - Tracing Parameters
    
    wxStaticBoxSizer *trace_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Trace Parameters");
    
    nr_disp=new NamedTextCtrl<int>(trace_sizer->GetStaticBox(),"N Disp: ",1000);
    nr_tot=new NamedTextCtrl<int>(trace_sizer->GetStaticBox(),"N Tot: ",10000);
    
    trace_sizer->Add(nr_disp,wxSizerFlags().Expand());
    trace_sizer->Add(nr_tot,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(trace_sizer,wxSizerFlags().Expand());
    
    wxStaticBoxSizer *output_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Output Directory");
    
    output_directory=new wxTextCtrl(output_sizer->GetStaticBox(),wxID_ANY);
    wxButton *output_btn=new wxButton(output_sizer->GetStaticBox(),wxID_ANY,"...",
                                      wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    output_directory->SetEditable(false);
    output_btn->Bind(wxEVT_BUTTON,&SeleneFrame::evt_output_directory,this);
    
    output_sizer->Add(output_directory,wxSizerFlags(1).Expand());
    output_sizer->Add(output_btn,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(output_sizer,std_flags);
    
    trace_btn=new wxToggleButton(ctrl_panel,wxID_ANY,"Trace");
    trace_btn->Bind(wxEVT_TOGGLEBUTTON,&SeleneFrame::evt_trace,this);
    ctrl_sizer->Add(trace_btn,std_flags);
    
    // Tree initialization
    
    root_ID=objects_tree->AddRoot("Elements");
    
    // Display
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    wxMenu *materials_menu=new wxMenu();
    wxMenu *optimization_menu=new wxMenu();
    wxMenu *parameters_menu=new wxMenu();
    
    file_menu->Append(MENU_NEW,"New");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    materials_menu->Append(MENU_REF_IND,"Refractive Index");
    materials_menu->Append(MENU_IRF,"Interface Responce Functions");
    
    optimize_ctrl=optimization_menu->AppendCheckItem(MENU_OPTIMIZE,"Enable");
    optimization_menu->Append(MENU_OPTIMIZATION_TARGETS,"Target");
    
    parameters_menu->Append(MENU_BOUNCES,"Max ray bounces");
    
    menu_bar->Append(file_menu,"File");
    menu_bar->Append(materials_menu,"Materials");
    menu_bar->Append(optimization_menu,"Optimization");
    menu_bar->Append(parameters_menu,"Parameters");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    default_menu.Append(MENU_PROPERTIES,"Properties");
    default_menu.AppendSeparator();
    default_menu.Append(MENU_DELETE,"Delete");
    
    // Bindings
    
    objects_tree->Bind(wxEVT_TREE_ITEM_MENU,&SeleneFrame::evt_object_menu,this);
    
    Bind(wxEVT_MENU,&SeleneFrame::evt_menu,this);
    
    Bind(EVT_REFRESH_GEOMETRY,&SeleneFrame::evt_refresh_geometry,this);
    
    //
    
    splitter->SplitVertically(ctrl_panel,gl,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
}

SeleneFrame::~SeleneFrame()
{
    if(optimization_thread!=nullptr)
    {
        optimization_thread->join();
        delete optimization_thread;
    }
}

void SeleneFrame::SeleneFrame_RayDisp(wxWindow *parent,wxBoxSizer *ctrl_sizer)
{
    wxSizerFlags default_expand=wxSizerFlags().Expand();
    
    wxStaticBoxSizer *ray_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Rays Display");
    wxStaticBox *box=ray_sizer->GetStaticBox();
    
    // Display type
    
    ray_disp_type=new wxChoice(box,wxID_ANY);
    ray_disp_type->Bind(wxEVT_CHOICE,&SeleneFrame::evt_ray_display_type,this);
    
    ray_disp_type->Append("Dispersion");
    ray_disp_type->Append("Generation");
    ray_disp_type->SetSelection(1);
    
    ray_sizer->Add(ray_disp_type,default_expand);
    
    // Min Max
    
    wxBoxSizer *auto_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // - Gen/Spectrum
    
    wxBoxSizer *minmax_sizer=new wxBoxSizer(wxVERTICAL);
    
    gen_min=new NamedTextCtrl<int>(box,"Min Gen: ",0,true);
    gen_max=new NamedTextCtrl<int>(box,"Max Gen: ",10,true);
    gen_min->Bind(EVT_NAMEDTXTCTRL,&SeleneFrame::evt_generation_display,this);
    gen_max->Bind(EVT_NAMEDTXTCTRL,&SeleneFrame::evt_generation_display,this);
    
    minmax_sizer->Add(gen_min,wxSizerFlags().Expand());
    minmax_sizer->Add(gen_max,wxSizerFlags().Expand());
    
    lambda_min=new WavelengthSelector(box,"Min Wavelength: ",370e-9);
    lambda_max=new WavelengthSelector(box,"Max Wavelength: ",850e-9);
    
    lambda_min->Bind(EVT_WAVELENGTH_SELECTOR,&SeleneFrame::evt_generation_display,this);
    lambda_max->Bind(EVT_WAVELENGTH_SELECTOR,&SeleneFrame::evt_generation_display,this);
    
    minmax_sizer->Add(lambda_min,wxSizerFlags().Expand());
    minmax_sizer->Add(lambda_max,wxSizerFlags().Expand());
    
    lambda_min->Hide();
    lambda_max->Hide();
    
    // - Auto
    
    wxButton *auto_btn=new wxButton(box,wxID_ANY,"Auto",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    auto_btn->Bind(wxEVT_BUTTON,&SeleneFrame::evt_generation_display_auto,this);
    
    auto_sizer->Add(minmax_sizer,wxSizerFlags(1));
    auto_sizer->Add(auto_btn,wxSizerFlags().Expand());
    
    ray_sizer->Add(auto_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Lost length
    
    lost_length=new LengthSelector(box,"Losses: ",0.1);
    lost_length->Bind(EVT_LENGTH_SELECTOR,&SeleneFrame::evt_lost_length,this);
    
    ray_sizer->Add(lost_length,wxSizerFlags().Expand().Border(wxALL,2));
    
    // Wrapping-up
    
    ctrl_sizer->Add(ray_sizer,wxSizerFlags().Expand());
}

void SeleneFrame::evt_ray_display_type(wxCommandEvent &event)
{
    gen_min->Hide();
    gen_max->Hide();
    
    lambda_min->Hide();
    lambda_max->Hide();
    
    if(ray_disp_type->GetSelection()==0)
    {
        lambda_min->Show();
        lambda_max->Show();
        gl->display_type=0;
    }
    else
    {
        gen_min->Show();
        gen_max->Show();
        gl->display_type=1;
    }
    
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
}

void SeleneFrame::check_objects_irfs()
{
    if(materials.size()==0) return;
    
    std::vector<std::string> modified_objects;
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            int N=object->get_N_faces();
            bool valid_object=true;
            
            for(int j=0;j<N;j++)
            {
                Sel::SelFace &face=object->face(j);
                
                if(!vector_contains(materials,dynamic_cast<GUI::Material*>(face.down_mat)))
                {
                    face.down_mat=materials[0];
                    valid_object=false;
                }
                
                if(!vector_contains(materials,dynamic_cast<GUI::Material*>(face.up_mat)))
                {
                    face.up_mat=materials[0];
                    valid_object=false;
                }
            }
            
            if(!valid_object) modified_objects.push_back(object->name);
        }
    }
    
    if(modified_objects.size()>0)
    {
        std::stringstream msg;
        msg<<"Warning, the following elements had invalid materials and have been arbitrarily modified: ";
        for(std::size_t i=0;i<modified_objects.size();i++)
        {
            msg<<modified_objects[i];
            if(i+1!=modified_objects.size()) msg<<" | ";
        }
        
        wxMessageDialog dialog(this,msg.str(),"Materials Warning",wxOK|wxCENTRE|wxICON_EXCLAMATION);
        dialog.ShowModal();
    }
}

void SeleneFrame::check_objects_materials()
{
    if(materials.size()==0) return;
    
    std::vector<std::string> modified_objects;
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(object!=nullptr)
        {
            int N=object->get_N_faces();
            bool valid_object=true;
            
            for(int j=0;j<N;j++)
            {
                Sel::SelFace &face=object->face(j);
                
                if(!vector_contains(materials,dynamic_cast<GUI::Material*>(face.down_mat)))
                {
                    face.down_mat=materials[0];
                    valid_object=false;
                }
                
                if(!vector_contains(materials,dynamic_cast<GUI::Material*>(face.up_mat)))
                {
                    face.up_mat=materials[0];
                    valid_object=false;
                }
            }
            
            if(!valid_object) modified_objects.push_back(object->name);
        }
        else if(light!=nullptr)
        {
            if(!vector_contains(materials,dynamic_cast<GUI::Material*>(light->amb_mat)))
            {
                light->amb_mat=materials[0];
                modified_objects.push_back(light->name);
            }
        }
    }
    
    if(modified_objects.size()>0)
    {
        std::stringstream msg;
        msg<<"Warning, the following elements had invalid materials and have been arbitrarily modified: ";
        for(std::size_t i=0;i<modified_objects.size();i++)
        {
            msg<<modified_objects[i];
            if(i+1!=modified_objects.size()) msg<<" | ";
        }
        
        wxMessageDialog dialog(this,msg.str(),"Materials Warning",wxOK|wxCENTRE|wxICON_EXCLAMATION);
        dialog.ShowModal();
    }
}

void SeleneFrame::clear_state()
{
    gen_min->set_value(0);
    gen_max->set_value(10);
    
    lost_length->set_length(0.1);
    
    nr_disp->set_value(1000);
    nr_tot->set_value(10000);
    
    // Materials
    
    for(std::size_t i=0;i<materials.size();i++) delete materials[i];
    materials.clear();
    
    // IRF
    
    for(std::size_t i=0;i<user_irfs.size();i++) delete user_irfs[i];
    user_irfs.clear();
    
    // Objects
    
    for(std::size_t i=0;i<frames_vao.size();i++) gl->delete_vao(frames_vao[i]);
    frames_vao.clear();
    
    for(std::size_t i=0;i<frames.size();i++) delete frames[i];
    frames.clear();
        
    rebuild_tree();
}

void SeleneFrame::evt_add_element(wxCommandEvent &event)
{
    if(optimization_running) return;
    
    int selection=add_element_list->GetSelection();
    
    bool cancel_check=true;
    Sel::Frame *frame=nullptr;
    
//    add_element_list->Append("Box");
//    add_element_list->Append("(todo) Cone");
//    add_element_list->Append("Cylinder");
//    add_element_list->Append("Lens");
//    add_element_list->Append("Mesh");
//    add_element_list->Append("(todo) Prism");
//    add_element_list->Append("Sphere");
//    add_element_list->Append("---------------");
//    add_element_list->Append("Conic Section");
//    add_element_list->Append("Disk");
//    add_element_list->Append("Parabola");
//    add_element_list->Append("Rectangle");
//    add_element_list->Append("Spherical patch");
//    add_element_list->Append("---------------");
//    add_element_list->Append("(todo) Beam");
//    add_element_list->Append("(todo) Cone");
//    add_element_list->Append("Lambertian Source");
//    add_element_list->Append("Perfect Beam");
//    add_element_list->Append("Planar Point Source");
//    add_element_list->Append("Point Source");
    
    int Nl=14;
    
    FrameDialog *dialog=nullptr;
    
    std::vector<Sel::IRF*> aggregator(irfs.size()+user_irfs.size());
    
    for(std::size_t i=0;i<irfs.size();i++)
        aggregator[i]=&irfs[i];
    
    for(std::size_t i=0;i<user_irfs.size();i++)
        aggregator[i+irfs.size()]=user_irfs[i];
            
    if(selection<=Nl-2)
    {
        Sel::Object *object=new Sel::Object;
        frame=object;
        
        switch(selection)
        {
            case 0:
                object->set_box();
                object->name="Box_" + std::to_string(item_count);
                dialog=new BoxDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 1: object->set_cone_volume(); break;
            case 2:
                object->set_cylinder_volume();
                object->name="Cylinder_" + std::to_string(item_count);
                dialog=new CylinderDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 3:
                object->set_lens();
                object->name="Lens_" + std::to_string(item_count);
                dialog=new LensDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 4:
                object->set_mesh();
                object->name="Mesh_" + std::to_string(item_count);
                dialog=new MeshDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 5:
                // Prism
                break;
            case 6:
                object->set_sphere();
                object->name="Sphere_" + std::to_string(item_count);
                dialog=new SphereDialog(object,frames,materials,aggregator,optim_engine);
                break;
            // Case 7 is the separator
            case 8:
                object->set_conic_section();
                object->name="Conic_" + std::to_string(item_count);
                dialog=new ConicSectionDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 9:
                object->set_disk();
                object->name="Disk_" + std::to_string(item_count);
                dialog=new DiskDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 10:
                object->set_parabola();
                object->name="Parabola_" + std::to_string(item_count);
                dialog=new ParabolaDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 11:
                object->set_rectangle();
                object->name="Rectangle_" + std::to_string(item_count);
                dialog=new RectangleDialog(object,frames,materials,aggregator,optim_engine);
                break;
            case 12:
                object->set_spherical_patch();
                object->name="Spherical_Patch_" + std::to_string(item_count);
                dialog=new SpherePatchDialog(object,frames,materials,aggregator,optim_engine);
                break;
        }
    }
    else if(selection>=Nl)
    {
        selection-=Nl;
        
        Sel::Light *light=new Sel::Light;
        frame=light;
        
        switch(selection)
        {
            case 0:
                light->type=Sel::SRC_CONE;
                light->name="Cone_Source_" + std::to_string(item_count);
                break;
            case 1:
                light->type=Sel::SRC_GAUSSIAN_BEAM;
                light->name="Gaussian_Beam_" + std::to_string(item_count);
                break;
            case 2:
                light->type=Sel::SRC_LAMBERTIAN;
                light->name="Lambertian_Source_" + std::to_string(item_count);
                break;
            case 3:
                light->type=Sel::SRC_PERFECT_BEAM;
                light->name="Perfect_Beam_" + std::to_string(item_count);
                break;
            case 4:
                light->type=Sel::SRC_POINT_PLANAR;
                light->name="Planar_Point_Source_" + std::to_string(item_count);
                break;
            case 5:
                light->name="Point_Source_" + std::to_string(item_count);
                break;
        }
        
        dialog=new SourceDialog(light,frames,materials,optim_engine);
    }
    
    if(dialog!=nullptr)
    {
        dialog->ShowModal();
        cancel_check=dialog->cancel_check;
        dialog->Destroy();
    }
    
    if(!cancel_check)
    {
        frame->consolidate_position();
        
        SeleneVAO *vao=gl->request_vao();
        update_vao(vao,frame);
        
        frames.push_back(frame);
        frames_vao.push_back(vao);
        
        item_count++;
    }
    else delete frame;
    
    rebuild_tree();
    
    // Recentering camera
    
    if(frames.size()>=2)
    {
        double x_min=std::numeric_limits<double>::max();
        double x_max=std::numeric_limits<double>::min();
        double y_min=std::numeric_limits<double>::max();
        double y_max=std::numeric_limits<double>::min();
        double z_min=std::numeric_limits<double>::max();
        double z_max=std::numeric_limits<double>::min();
        
        for(std::size_t i=0;i<frames.size();i++)
        {
            x_min=std::min(x_min,frames[i]->loc.x);
            x_max=std::max(x_max,frames[i]->loc.x);
            
            y_min=std::min(y_min,frames[i]->loc.y);
            y_max=std::max(y_max,frames[i]->loc.y);
            
            z_min=std::min(z_min,frames[i]->loc.z);
            z_max=std::max(z_max,frames[i]->loc.z);
        }
        
        double x_center=0.5*(x_max-x_min),
               y_center=0.5*(y_max-y_min),
               z_center=0.5*(z_max-z_min);
        
        double radius=(x_center-x_min)*(x_center-x_min)+
                      (y_center-y_min)*(y_center-y_min)+
                      (z_center-z_min)*(z_center-z_min);
        
        radius=1.5*std::sqrt(radius);
        
        gl->set_camera_target(Vector3(x_center,y_center,z_center));
        if(radius>0) gl->set_camera_radius(radius);
    }
    
    event.Skip();
}

void SeleneFrame::evt_generation_display(wxCommandEvent &event)
{
    gl->gen_min_disp=gen_min->get_value();
    gl->gen_max_disp=gen_max->get_value();
    
    gl->lambda_min_disp=lambda_min->get_lambda();
    gl->lambda_max_disp=lambda_max->get_lambda();
    
    event.Skip();
}

void SeleneFrame::evt_generation_display_auto(wxCommandEvent &event)
{
//    int gen_max_gl=gl->gen_max;
    
    // Generation
    
    gl->gen_min_disp=0;
    gl->gen_max_disp=gl->gen_max;
    
    gen_min->set_value(0);
    gen_max->set_value(gl->gen_max);
    
    // Wavelength
    
    gl->lambda_min_disp=gl->lambda_min;
    gl->lambda_max_disp=gl->lambda_max;
    
    lambda_min->set_lambda(gl->lambda_min);
    lambda_max->set_lambda(gl->lambda_max);
    
    event.Skip();
}

void SeleneFrame::evt_lost_length(wxCommandEvent &event)
{
    gl->lost_length=lost_length->get_length();
    
    event.Skip();
}

void SeleneFrame::evt_menu(wxCommandEvent &event)
{
    if(optimization_running) return;
    
    int menu_ID=event.GetId();
    
    switch(menu_ID)
    {
        case MENU_NEW: clear_state(); break;
        case MENU_SAVE: evt_save_project("asln","Save Selene Project"); break;
        case MENU_SAVE_AS: evt_save_project_as("asln","Save Selene Project As"); break;
        case MENU_LOAD: evt_load_project("asln","Load Selene Project"); break;
        case MENU_EXIT: Close(); break;
        case MENU_REF_IND:
            { MaterialsDialog dialog{materials}; }
            check_objects_materials();
            break;
        case MENU_IRF:
            { IRF_Dialog dialog{user_irfs}; }
            check_objects_irfs();
            break;
        case MENU_OPTIMIZE:optimize=event.IsChecked(); break;
        case MENU_OPTIMIZATION_TARGETS:
            {
                std::vector<Sel::Object*> sensors;
                
                for(Sel::Frame *frame: frames)
                {
                    if(Sel::Object *object=dynamic_cast<Sel::Object*>(frame);
                          object!=nullptr
                       && object->sensor_type!=Sel::Sensor::NONE)
                    {
                        sensors.push_back(object);
                    }
                }
                
                if(!sensors.empty()) OptimizationDialog dialog(optimization_targets,sensors);
                else
                {
                    wxMessageBox("Error: no sensor to optimize for");
                }
            }
            break;
        case MENU_BOUNCES: 
            {
                RayBouncesDialog dialog(max_ray_bounces);
                
                max_ray_bounces=std::max(2,dialog.ray_bounces->get_value());
            }
            break;
        case MENU_DELETE: evt_popup_menu(event); break;
        case MENU_PROPERTIES: evt_popup_menu(event); break;
    }
    
    event.Skip();
}

void SeleneFrame::evt_object_menu(wxTreeEvent &event)
{
    if(optimization_running) return;
    
    wxTreeItemId item_ID=event.GetItem();
        
    bool found;
    focus_ID=vector_locate(found,frames_ID,item_ID);
    
    if(found) PopupMenu(&default_menu);
    
    event.Skip();
}

void SeleneFrame::evt_output_directory(wxCommandEvent &event)
{
    if(optimization_running) return;
    
    wxDirDialog dialog(0,"Select the output directory");
    dialog.ShowModal();
    
    wxString dir=dialog.GetPath();
    
    if(dir.size()==0) return;
    
    output_directory_std=dir.ToStdString();
    output_directory->ChangeValue(output_directory_std.generic_string());
}

void SeleneFrame::evt_popup_menu(wxCommandEvent &event)
{
    int menu_ID=event.GetId();
    
    if(menu_ID==MENU_PROPERTIES)
    {
        FrameDialog *dialog=nullptr;
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[focus_ID]);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[focus_ID]);
        
        std::vector<Sel::IRF*> aggregator(irfs.size()+user_irfs.size());
        
        for(std::size_t i=0;i<irfs.size();i++)
            aggregator[i]=&irfs[i];
        
        for(std::size_t i=0;i<user_irfs.size();i++)
            aggregator[i+irfs.size()]=user_irfs[i];
        
        if(object!=nullptr)
        {
            switch(object->type)
            {
                case Sel::OBJ_BOX:
                    dialog=new BoxDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_CONIC:
                    dialog=new ConicSectionDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_DISK:
                    dialog=new DiskDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_PARABOLA:
                    dialog=new ParabolaDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_LENS:
                    dialog=new LensDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_MESH:
                    dialog=new MeshDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_RECTANGLE:
                    dialog=new RectangleDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_SPHERE:
                    dialog=new SphereDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_SPHERE_PATCH:
                    dialog=new SpherePatchDialog(object,frames,materials,aggregator,optim_engine);
                    break;
                case Sel::OBJ_VOL_CYLINDER:
                    dialog=new CylinderDialog(object,frames,materials,aggregator,optim_engine);
                    break;
            }
        }
        else if(light!=nullptr)
            dialog=new SourceDialog(light,frames,materials,optim_engine);
            
        if(dialog!=nullptr)
        {
            dialog->ShowModal();
            dialog->Destroy();
        }
    }
    else if(menu_ID==MENU_DELETE)
    {
        for(std::size_t i=0;i<frames.size();i++)
            frames[i]->forget_frame(frames[focus_ID]);
        
        gl->delete_vao(frames_vao[focus_ID]);
        
        std::map<std::string,double*> &variables_map=frames[focus_ID]->variables_map;
        std::map<std::string,double*>::const_iterator it;
        
        for(it=variables_map.begin();it!=variables_map.end();++it)
        {
            optim_engine.forget_variable(it->second);
        }
        
        std::vector<Sel::Frame*>::const_iterator it_frame=frames.begin()+focus_ID;
        std::vector<wxTreeItemId>::const_iterator it_tree=frames_ID.begin()+focus_ID;
        std::vector<SeleneVAO*>::const_iterator it_vao=frames_vao.begin()+focus_ID;
        
        frames.erase(it_frame);
        frames_ID.erase(it_tree);
        frames_vao.erase(it_vao);
    }
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        frames[i]->consolidate_position();
        update_vao(frames_vao[i],frames[i]);
    }
    
    rebuild_tree();
    
    event.Skip();
}

void SeleneFrame::evt_refresh_geometry(wxCommandEvent &event)
{
    update_rays();
        
    for(std::size_t i=0;i<frames.size();i++)
        update_vao(frames_vao[i],frames[i]);
        
    pause_optimization=false;
}

void SeleneFrame::evt_trace(wxCommandEvent &event)
{
    bool scene_check=true;
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *obj=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(obj!=nullptr)
        {
            bool obj_check=true;
            
            for(int j=0;j<obj->get_N_faces();j++)
            {
                if(   obj->face(j).down_mat==nullptr
                   || obj->face(j).up_mat==nullptr)
                {
                    obj_check=false;
                    break;
                }
            }
            
            if(!obj_check)
            {
                wxMessageBox("Error: missing material on object: "+obj->name);
                scene_check=false;
            }
        }
    }
    
    if(scene_check)
    {
        if(optimize)
        {
            optimization_running=trace_btn->GetValue();
            
            if(optimization_running)
            {
                rays_x1.resize(Nrays); rays_x2.resize(Nrays);
                rays_y1.resize(Nrays); rays_y2.resize(Nrays);
                rays_z1.resize(Nrays); rays_z2.resize(Nrays);
                
                rays_gen.resize(Nrays);
                rays_lambda.resize(Nrays);
                rays_lost.resize(Nrays);
                
                optimization_thread=new std::thread(&SeleneFrame::optimization_trace,this);
            }
            else
            {
                if(optimization_thread!=nullptr)
                {
                    optimization_thread->join();
                    delete optimization_thread;
                    optimization_thread=nullptr;
                }
            }
        }
        else
        {
            Sel::Selene selene;
            
            for(std::size_t i=0;i<frames.size();i++)
            {
                Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
                Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
                
                if(object!=nullptr) selene.add_object(object);
                else if(light!=nullptr) selene.add_light(light);
            }
            
            selene.set_output_directory(output_directory_std);
            selene.set_max_ray_bounces(max_ray_bounces);
            
            selene.render(nr_disp->get_value(),
                          nr_tot->get_value());
            
            gl->set_rays(selene.xs_ftc,selene.xe_ftc,
                         selene.ys_ftc,selene.ye_ftc,
                         selene.zs_ftc,selene.ze_ftc,
                         selene.gen_ftc,selene.lambda_ftc,
                         selene.lost_ftc);
            
            trace_btn->SetValue(false);
        }
    }
    else trace_btn->SetValue(false);
    
    event.Skip();
}

void SeleneFrame::gather_materials()
{
    materials.clear();
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(object!=nullptr)
        {
            int N=object->get_N_faces();
            
            for(int j=0;j<N;j++)
            {
                Sel::SelFace &face=object->face(j);
                
                GUI::Material *down_mat=dynamic_cast<GUI::Material*>(face.down_mat);
                GUI::Material *up_mat=dynamic_cast<GUI::Material*>(face.up_mat);
                
                if(!vector_contains(materials,down_mat))
                    materials.push_back(down_mat);
                
                if(!vector_contains(materials,up_mat))
                    materials.push_back(up_mat);
            }
        }
        else if(light!=nullptr)
        {
            GUI::Material *mat=dynamic_cast<GUI::Material*>(light->amb_mat);
            
            if(!vector_contains(materials,mat))
                materials.push_back(mat);
        }
    }
}

void SeleneFrame::rebuild_tree()
{
    int N=frames.size();
    frames_ID.resize(N);
    
    std::vector<bool> placed_element(N);
    for(int i=0;i<N;i++) placed_element[i]=false;
    
    objects_tree->DeleteAllItems();
    root_ID=objects_tree->AddRoot("Elements");
    
    while(vector_contains(placed_element,false))
    {
        for(int i=0;i<N;i++) if(!placed_element[i])
        {
            std::string name=frames[i]->name;
            int image_ID=0;
            if(dynamic_cast<Sel::Light*>(frames[i])==nullptr) image_ID=1;
            
            if(frames[i]->relative_origin==nullptr)
            {
                frames_ID[i]=objects_tree->AppendItem(root_ID,name,image_ID);
                placed_element[i]=true;
            }
            else
            {
                bool found;
                int index=vector_locate(found,frames,frames[i]->relative_origin);
                
                if(placed_element[index])
                {
                    frames_ID[i]=objects_tree->AppendItem(frames_ID[index],name,image_ID);
                    placed_element[i]=true;
                }
            }
        }
    }
    
    objects_tree->ExpandAll();
}

void SeleneFrame::update_rays()
{
    gl->set_rays(rays_x1,rays_x2,
                 rays_y1,rays_y2,
                 rays_z1,rays_z2,
                 rays_gen,rays_lambda,
                 rays_lost);
}

void SeleneFrame::update_vao(SeleneVAO *vao,Sel::Frame *frame)
{
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
    Sel::Object *object=dynamic_cast<Sel::Object*>(frame);
    Sel::Light *light=dynamic_cast<Sel::Light*>(frame);
    
    Vector3 color(1.0,1.0,1.0);
    
    if(object!=nullptr)
    {
        if(object->type==Sel::OBJ_BOX)
        {
            Glite::make_block_wires(V_arr,F_arr,
                                    object->box.get_lx(),object->box.get_ly(),object->box.get_lz(),
                                    -0.5,-0.5,-0.5);
        }
        else if(object->type==Sel::OBJ_VOL_CYLINDER)
        {
            cylinder_cut_mesh_wireframe(V_arr,F_arr,object->cylinder.length,object->cylinder.radius,object->cylinder.cut_factor);
        }
        else if(object->type==Sel::OBJ_CONIC)
        {
            conic_section_mesh_wireframe(V_arr,F_arr,128,
                                         object->conic.R_factor,object->conic.K_factor,
                                         object->conic.in_radius,object->conic.out_radius);
    
        }
        else if(object->type==Sel::OBJ_DISK)
        {
            disk_mesh_wireframe(V_arr,F_arr,object->disk.radius,object->disk.in_radius);
        }
        else if(object->type==Sel::OBJ_LENS)
        {
            lens_mesh_wireframe(V_arr,F_arr,128,
                                object->lens.thickness,object->lens.max_outer_radius,object->lens.radius_front,object->lens.radius_back);
        }
        else if(object->type==Sel::OBJ_PARABOLA)
        {
            parabola_mesh_wireframe(V_arr,F_arr,128,
                                    object->parabola.focal,
                                    object->parabola.inner_radius,
                                    object->parabola.length);
        }
        else if(object->type==Sel::OBJ_MESH)
        {
            V_arr.resize(object->V_arr.size());
            F_arr.resize(object->F_arr.size());
            
            for(unsigned int i=0;i<V_arr.size();i++)
            {
                V_arr[i].loc=object->V_arr[i].loc;
                V_arr[i].norm=object->V_arr[i].norm;
            }
            
            for(unsigned int i=0;i<F_arr.size();i++)
            {
                F_arr[i].V1=object->F_arr[i].V1;
                F_arr[i].V2=object->F_arr[i].V2;
                F_arr[i].V3=object->F_arr[i].V3;
            }
        }
        else if(object->type==Sel::OBJ_PRISM)
        {
    //        prism_mesh_wireframe(V_arr,F_arr,
    //                             object->prism_length,object->prism_height,
    //                             object->prism_a1,object->prism_a2,object->prism_width);
        }
        else if(object->type==Sel::OBJ_RECTANGLE)
        {
            rectangle_mesh_wireframe(V_arr,F_arr,object->rectangle.ly,object->rectangle.lz);
        }
        else if(object->type==Sel::OBJ_SPHERE)
        {
            sphere_cut_mesh_wireframe(V_arr,F_arr,object->sphere.get_radius(),object->sphere.get_cut_factor(),true);
        }
        else if(object->type==Sel::OBJ_SPHERE_PATCH)
        {
            sphere_cut_mesh_wireframe(V_arr,F_arr,object->sphere_patch.get_radius(),object->sphere_patch.get_cut_factor(),false);
        }
    }
    else if(light!=nullptr)
    {
        color=Vector3(1.0,1.0,0);
        int i;
            
        if(light->type==Sel::SRC_POINT || light->type==Sel::SRC_POINT_PLANAR)
        {
            V_arr.resize(14);
            F_arr.resize(7);
            
            double s1=0.05;
            double s2=0.05/2.0;
            double s3=0.05*std::sqrt(2.0)/2.0;
            
            V_arr[ 0].loc(0,0,-s1);
            V_arr[ 1].loc(0,0,+s1);
            V_arr[ 2].loc(0,-s1,0);
            V_arr[ 3].loc(0,+s1,0);
            V_arr[ 4].loc(-s1,0,0);
            V_arr[ 5].loc(+s1,0,0);
            
            V_arr[ 6].loc(-s2,-s2,-s3);
            V_arr[ 7].loc(+s2,+s2,+s3);
            V_arr[ 8].loc(+s2,-s2,-s3);
            V_arr[ 9].loc(-s2,+s2,+s3);
            V_arr[10].loc(+s2,+s2,-s3);
            V_arr[11].loc(-s2,-s2,+s3);
            V_arr[12].loc(-s2,+s2,-s3);
            V_arr[13].loc(+s2,-s2,+s3);
            
            for(i=0;i<7;i++)
            {
                F_arr[i].V1=2*i;
                F_arr[i].V2=2*i+1;
            }
        }
        else if(light->type==Sel::SRC_CONE)
        {
            double r=0.05;
            double ang=light->cone_angle/2.0;
            
            double ca=std::cos(ang);
            double sa=std::sin(ang);
            
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,0,-r),Vector3(0,0,+r));
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,-r,0),Vector3(0,+r,0));
            
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,0,0),r*Vector3(ca,+sa,0));
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,0,0),r*Vector3(ca,-sa,0));
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,0,0),r*Vector3(ca,0,+sa));
            wireframe_mesh_add_line(V_arr,F_arr,Vector3(0,0,0),r*Vector3(ca,0,-sa));
            
            wireframe_mesh_add_arc(V_arr,F_arr,128,Vector3(r,0,0),Vector3(0,r,0),0,ang);
            wireframe_mesh_add_arc(V_arr,F_arr,128,Vector3(r,0,0),Vector3(0,r,0),0,-ang);
            wireframe_mesh_add_arc(V_arr,F_arr,128,Vector3(r,0,0),Vector3(0,0,r),0,ang);
            wireframe_mesh_add_arc(V_arr,F_arr,128,Vector3(r,0,0),Vector3(0,0,r),0,-ang);
            
            wireframe_mesh_add_circle(V_arr,F_arr,128,r*ca,r*sa);
        }
        else if(light->type==Sel::SRC_LAMBERTIAN)
        {
            V_arr.resize(14+128);
            F_arr.resize(7+128);
            
            double s1=0.05;
            double s2=0.05/2.0;
            double s3=0.05*std::sqrt(2.0)/2.0;
            
            V_arr[ 0].loc(0,0,-s1);
            V_arr[ 1].loc(0,0,+s1);
            V_arr[ 2].loc(0,-s1,0);
            V_arr[ 3].loc(0,+s1,0);
            V_arr[ 4].loc(0,0,0);
            V_arr[ 5].loc(+s1,0,0);
            
            V_arr[ 6].loc(0,0,0);
            V_arr[ 7].loc(+s2,+s2,+s3);
            V_arr[ 8].loc(0,0,0);
            V_arr[ 9].loc(+s2,-s2,+s3);
            V_arr[10].loc(0,0,0);
            V_arr[11].loc(+s2,+s2,-s3);
            V_arr[12].loc(0,0,0);
            V_arr[13].loc(+s2,-s2,-s3);
            
            for(i=0;i<7;i++)
            {
                F_arr[i].V1=2*i;
                F_arr[i].V2=2*i+1;
            }
            
            int v_offset=14;
            int f_offset=7;
            
            for(i=0;i<128;i++)
            {
                double ang=2.0*Pi*i/127.0;
                
                V_arr[i+v_offset].loc(0,0*std::cos(ang),
                                        0*std::sin(ang));
            }
            
            for(i=0;i<128;i++)
            {
                F_arr[i+f_offset].V1=i+v_offset;
                F_arr[i+f_offset].V2=i+1+v_offset;
            }
            
            F_arr[134].V2=v_offset;
        }
        else if(light->type==Sel::SRC_GAUSSIAN_BEAM ||light->type==Sel::SRC_PERFECT_BEAM)
        {
            V_arr.resize(6+128);
            F_arr.resize(3+128);
            
            double s1=0.05;
//            double s2=0.05/2.0;
//            double s3=0.05*std::sqrt(2.0)/2.0;
            
            V_arr[ 0].loc(0,0,-s1);
            V_arr[ 1].loc(0,0,+s1);
            V_arr[ 2].loc(0,-s1,0);
            V_arr[ 3].loc(0,+s1,0);
            V_arr[ 4].loc(0,0,0);
            V_arr[ 5].loc(+s1,0,0);
                        
            for(i=0;i<3;i++)
            {
                F_arr[i].V1=2*i;
                F_arr[i].V2=2*i+1;
            }
            
            int v_offset=6;
            int f_offset=3;
            
            for(i=0;i<128;i++)
            {
                double ang=2.0*Pi*i/127.0;
                double r=light->extent_d/2.0;
                
                V_arr[i+v_offset].loc(0,r*std::cos(ang),
                                        r*std::sin(ang));
            }
            
            for(i=0;i<128;i++)
            {
                F_arr[i+f_offset].V1=i+v_offset;
                F_arr[i+f_offset].V2=i+1+v_offset;
            }
            
            F_arr[130].V2=v_offset;
        }
        
        for(unsigned int i=0;i<F_arr.size();i++)
            F_arr[i].V3=F_arr[i].V2;
    }
    
    gl->focus();
    
    vao->set_mesh_wireframe(V_arr,F_arr);
    vao->set_shading(color,true,true);
    
    update_vao_location(vao,frame);
}

void SeleneFrame::update_vao_location(SeleneVAO *vao,Sel::Frame *frame)
{
    gl->focus();
    frame->consolidate_position();
    
    Vector3 O=frame->loc,
            A=frame->local_x,
            B=frame->local_y,
            C=frame->local_z;
            
    vao->set_matrix(O,A,B,C);
}

}
