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
#include <lua_selene.h>

#include <gui_selene.h>

extern std::ofstream plog;

namespace SelGUI
{

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
    MENU_IRF
};

SeleneFrame::SeleneFrame(wxString const &title)
    :BaseFrame(title),
     item_count(0)
{
    irfs.resize(4);
    
    irfs[0].set_type_fresnel(); irfs[0].name="Fresnel";
    irfs[1].set_type(Sel::IRF_PERF_ABS); irfs[1].name="Perfect Absorber";
    irfs[2].set_type(Sel::IRF_PERF_ANTIREF); irfs[2].name="Perfect Antireflector";
    irfs[3].set_type(Sel::IRF_PERF_MIRROR); irfs[3].name="Perfect Mirror";
    
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
    
    wxButton *trace_btn=new wxButton(ctrl_panel,wxID_ANY,"Trace");
    ctrl_sizer->Add(trace_btn,std_flags);
    
    // Tree initialization
    
    root_ID=objects_tree->AddRoot("Elements");
    
    // Display
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    wxMenu *materials_menu=new wxMenu();
    
    file_menu->Append(MENU_NEW,"New");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    materials_menu->Append(MENU_REF_IND,"Refractive Index");
    materials_menu->Append(MENU_IRF,"Interface Responce Functions");
    
    menu_bar->Append(file_menu,"File");
    menu_bar->Append(materials_menu,"Materials");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    default_menu.Append(MENU_PROPERTIES,"Properties");
    default_menu.AppendSeparator();
    default_menu.Append(MENU_DELETE,"Delete");
    
    // Bindings
    
    objects_tree->Bind(wxEVT_TREE_ITEM_MENU,&SeleneFrame::evt_object_menu,this);
    trace_btn->Bind(wxEVT_BUTTON,&SeleneFrame::evt_trace,this);
    
    Bind(wxEVT_MENU,&SeleneFrame::evt_menu,this);
    
    //
    
//    splitter->SplitVertically(ctrl_panel,display_panel,250);
    splitter->SplitVertically(ctrl_panel,gl,250);
    splitter->SetMinimumPaneSize(20);
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollRate(10,10);
    ctrl_panel->FitInside();
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
    
    gen_min=new NamedTextCtrl<int>(box,"Min Gen: ",0,5);
    gen_max=new NamedTextCtrl<int>(box,"Max Gen: ",10,5);
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
                dialog=new BoxDialog(object,frames,materials,aggregator);
                break;
            case 1: object->set_cone_volume(); break;
            case 2:
                object->set_cylinder_volume();
                object->name="Cylinder_" + std::to_string(item_count);
                dialog=new CylinderDialog(object,frames,materials,aggregator);
                break;
            case 3:
                object->set_lens();
                object->name="Lens_" + std::to_string(item_count);
                dialog=new LensDialog(object,frames,materials,aggregator);
                break;
            case 4:
                object->set_mesh();
                object->name="Mesh_" + std::to_string(item_count);
                dialog=new MeshDialog(object,frames,materials,aggregator);
                break;
            case 5:
                // Prism
                break;
            case 6:
                object->set_sphere();
                object->name="Sphere_" + std::to_string(item_count);
                dialog=new SphereDialog(object,frames,materials,aggregator);
                break;
            // Case 7 is the separator
            case 8:
                object->set_conic_section();
                object->name="Conic_" + std::to_string(item_count);
                dialog=new ConicSectionDialog(object,frames,materials,aggregator);
                break;
            case 9:
                object->set_disk();
                object->name="Disk_" + std::to_string(item_count);
                dialog=new DiskDialog(object,frames,materials,aggregator);
                break;
            case 10:
                object->set_parabola();
                object->name="Parabola_" + std::to_string(item_count);
                dialog=new ParabolaDialog(object,frames,materials,aggregator);
                break;
            case 11:
                object->set_rectangle();
                object->name="Rectangle_" + std::to_string(item_count);
                dialog=new RectangleDialog(object,frames,materials,aggregator);
                break;
            case 12:
                object->set_spherical_patch();
                object->name="Spherical_Patch_" + std::to_string(item_count);
                dialog=new SpherePatchDialog(object,frames,materials,aggregator);
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
        
        dialog=new SourceDialog(light,frames,materials);
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
    int gen_max_gl=gl->gen_max;
    
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
        case MENU_DELETE: evt_popup_menu(event); break;
        case MENU_PROPERTIES: evt_popup_menu(event); break;
    }
    
    event.Skip();
}

void SeleneFrame::evt_object_menu(wxTreeEvent &event)
{
    wxTreeItemId item_ID=event.GetItem();
        
    bool found;
    focus_ID=vector_locate(found,frames_ID,item_ID);
    
    if(found) PopupMenu(&default_menu);
    
    event.Skip();
}

void SeleneFrame::evt_output_directory(wxCommandEvent &event)
{
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
                    dialog=new BoxDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_CONIC:
                    dialog=new ConicSectionDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_DISK:
                    dialog=new DiskDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_PARABOLA:
                    dialog=new ParabolaDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_LENS:
                    dialog=new LensDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_MESH:
                    dialog=new MeshDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_RECTANGLE:
                    dialog=new RectangleDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_SPHERE:
                    dialog=new SphereDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_SPHERE_PATCH:
                    dialog=new SpherePatchDialog(object,frames,materials,aggregator);
                    break;
                case Sel::OBJ_VOL_CYLINDER:
                    dialog=new CylinderDialog(object,frames,materials,aggregator);
                    break;
            }
        }
        else if(light!=nullptr)
            dialog=new SourceDialog(light,frames,materials);
            
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
        Sel::Selene selene;
        
        for(std::size_t i=0;i<frames.size();i++)
        {
            Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
            Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
            
            if(object!=nullptr) selene.add_object(object);
            else if(light!=nullptr) selene.add_light(light);
        }
        
        selene.set_output_directory(output_directory_std);
        
        selene.render(nr_disp->get_value(),
                      nr_tot->get_value());
        
        gl->set_rays(selene.xs_ftc,selene.xe_ftc,
                     selene.ys_ftc,selene.ye_ftc,
                     selene.zs_ftc,selene.ze_ftc,
                     selene.gen_ftc,selene.lambda_ftc,
                     selene.lost_ftc);
    }
    
    event.Skip();
}

/*int lua_allocate_material(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    Material *mat=lua_allocate_metapointer<Material>(L,"metatable_material");
    
    frame->materials.push_back(mat);
    
    return 1;
}*/

int lua_allocate_selene_IRF(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    Sel::IRF *irf=lua_allocate_metapointer<Sel::IRF>(L,"metatable_selene_irf");
    
    frame->user_irfs.push_back(irf);
    
    return 1;
}

int lua_allocate_selene_light(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Light
    
    Sel::Frame **ppf=reinterpret_cast<Sel::Frame**>(lua_newuserdata(L,sizeof(Sel::Frame*)));
    
    luaL_getmetatable(L,"metatable_selene_light");
    lua_setmetatable(L,-2);
    
    Sel::Light *p_light=new Sel::Light;
    frame->frames.push_back(p_light);
    *ppf=p_light;
    
    LuaUI::create_selene_light_type(L,p_light);
    
    return 1;
}

int lua_allocate_selene_object(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Object
    
    Sel::Frame **ppf=reinterpret_cast<Sel::Frame**>(lua_newuserdata(L,sizeof(Sel::Frame*)));
    
    luaL_getmetatable(L,"metatable_selene_object");
    lua_setmetatable(L,-2);
    
    Sel::Object *p_object=new Sel::Object;
    frame->frames.push_back(p_object);
    *ppf=p_object;
    
    LuaUI::create_selene_object_type(L,p_object);
    
    return 1;
}

int lua_allocate_SeleneFrame_pointer(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    SeleneFrame *frame=reinterpret_cast<SeleneFrame*>(lua_touserdata(L,-1));
    
    // Object
    
    SeleneFrame **ppf=reinterpret_cast<SeleneFrame**>(lua_newuserdata(L,sizeof(SeleneFrame*)));
    
    *ppf=frame;
    
    luaL_getmetatable(L,"metatable_selene");
    lua_setmetatable(L,-2);
        
    return 1;
}

int lua_selene_add_light(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    return 0;
}

int lua_selene_add_object(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    return 0;
}

int lua_selene_set_N_rays_disp(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    frame->nr_disp->set_value(lua_tointeger(L,2));
    
    return 0;
}

int lua_selene_set_N_rays_total(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    frame->nr_tot->set_value(lua_tointeger(L,2));
    
    return 0;
}

int lua_selene_output_directory(lua_State *L)
{
    SeleneFrame *frame=lua_get_metapointer<SeleneFrame>(L,1);
    
    std::string directory=lua_tostring(L,2);
    proper_dirname_convertion(directory);
    chk_var(directory);
    chk_var(frame);
    frame->output_directory_std=directory;
    frame->output_directory->ChangeValue(directory);
    
    return 0;
}

void SeleneFrame::load_project(wxFileName const &fname_)
{
    std::cout<<"\n\nThis: "<<this<<"\n\n";
    
    clear_state();
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    // Lua
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    // Globals allocation
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"bound_class");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[0]));
    lua_setglobal(L,"SEL_IRF_FRESNEL");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[1]));
    lua_setglobal(L,"SEL_IRF_PERFECT_ABSORBER");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[2]));
    lua_setglobal(L,"SEL_IRF_PERFECT_ANTIREFLECTOR");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&irfs[3]));
    lua_setglobal(L,"SEL_IRF_PERFECT_MIRROR");
    
    // Allocation functions
    
    //lua_register(L,"Material",&GUI::lua_create_material);
    lua_register(L,"Selene_IRF",&lua_allocate_selene_IRF);
    lua_register(L,"Selene_light",&lua_allocate_selene_light);
    lua_register(L,"Selene_object",&lua_allocate_selene_object);
    
    // Metatables
    
    // - Materials
    
    GUI::create_material_metatable(L);
    
    // - Simulations parameters
    
    lua_register(L,"MODE",&lua_allocate_SeleneFrame_pointer);
    
    create_obj_metatable(L,"metatable_selene");
    
    metatable_add_func(L,"add_object",&SelGUI::lua_selene_add_object);
    metatable_add_func(L,"add_light",&SelGUI::lua_selene_add_light);
    metatable_add_func(L,"N_rays_disp",&SelGUI::lua_selene_set_N_rays_disp);
    metatable_add_func(L,"N_rays_total",&SelGUI::lua_selene_set_N_rays_total);
    metatable_add_func(L,"render",&null_lua);
    metatable_add_func(L,"output_directory",&SelGUI::lua_selene_output_directory);
    
    // - Selene elements
    
    LuaUI::Selene_create_light_metatable(L);
    LuaUI::Selene_create_object_metatable(L);
    
//    int load_err = luaL_loadfile(L,fname.c_str());
//    
//    if(load_err!=LUA_OK)
//    {
//        std::cout<<"Lua syntax error with "<<fname<<std::endl;
//        std::cin.get();
//        return;
//    }
    
//    lua_pcall(L,0,0,0);
    
    if(luaL_loadfile(L,fname.c_str()) || docall(L, 0, 0))
    {
        std::cerr << lua_tostring(L, -1) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    lua_close(L);
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        frames[i]->consolidate_position();
        
        SeleneVAO *vao=gl->request_vao();
        update_vao(vao,frames[i]);
        
        frames_vao.push_back(vao);
        
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            if(object->name.size()==0)
            {
                object->name="Object_"+std::to_string(item_count);
                item_count++;
            }
        }
        else if(frames[i]->name.size()==0)
        {
            frames[i]->name="Light_"+std::to_string(item_count);
            item_count++;
        }
    }
    
    item_count=frames.size();
    
    rebuild_tree();
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

void SeleneFrame::save_project(wxFileName const &fname_)
{
    std::string fname=fname_.GetFullPath().ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    // Materials
    
    for(std::size_t i=0;i<materials.size();i++)
    {
        std::string name="material_"+std::to_string(i);
        
        file<<name<<"=Material()\n";
        
        file<<name<<":";
        if(materials[i]->is_const())
        {
            file<<"refractive_index("<<std::real(materials[i]->get_n(0))<<")\n";
        }
        else file<<"load_script(\""<<materials[i]->script_path.generic_string()<<"\")\n";
        file<<name<<":name(\""<<materials[i]->name<<"\")\n";
        file<<"\n";
    }
    
    // IRFs
    
    for(std::size_t i=0;i<user_irfs.size();i++)
    {
        std::string name="irf_"+std::to_string(i);
        
        file<<name<<"=Selene_IRF()\n";
        
        file<<name<<":type(";
        switch(user_irfs[i]->type)
        {
            case Sel::IRF_GRATING:
                file<<"\"grating\","<<user_irfs[i]->grat_ref_path.generic_string()<<","
                                    <<user_irfs[i]->grat_ref_path.generic_string()<<")\n";
                break;
            case Sel::IRF_MULTILAYER:
                file<<"\"multilayer\")\n";
                
                for(std::size_t j=0;j<user_irfs[i]->ml_heights.size();j++)
                {
                    if(user_irfs[i]->ml_materials[j].is_const())
                        file<<name<<":add_layer("<<user_irfs[i]->ml_heights[j]<<","<<user_irfs[i]->ml_materials[j].get_n(0).real()<<")\n";
                    else
                        file<<name<<":add_layer("<<user_irfs[i]->ml_heights[j]<<","<<user_irfs[i]->ml_materials[j].script_path.generic_string()<<")\n";
                }
                
                break;
            case Sel::IRF_SNELL_SPLITTER:
                file<<"\"snell_splitter\","<<user_irfs[i]->splitting_factor<<")\n";
                break;
            default:
                file<<"\"fresnel\")\n";
                break;
        }
        file<<name<<":name(\""<<user_irfs[i]->name<<"\")\n";
        file<<"\n";
    }
    
    // Objects definition
    file<<"-- Objects definition\n\n";
    
    std::vector<std::string> ID(frames.size());
    
    int obj_counter=0;
    int light_counter=0;
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        if(dynamic_cast<Sel::Object*>(frames[i])!=nullptr)
        {
            ID[i]="object_"+std::to_string(obj_counter);
            obj_counter++;
        }
        else
        {
            ID[i]="light_"+std::to_string(light_counter);
            light_counter++;
        }
    }
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            file<<ID[i]<<"=Selene_object(";
            
            switch(object->type)
            {
                case Sel::OBJ_BOX:
                    file<<"\"box\","<<object->box_lx<<","<<object->box_ly<<","<<object->box_lz;
                    break;
                case Sel::OBJ_VOL_CONE:
                    break;
                case Sel::OBJ_CONIC:
                    file<<"\"conic_section\","<<object->conic_R<<","<<object->conic_K<<","<<object->conic_in_radius<<","<<object->conic_out_radius;
                    break;
                case Sel::OBJ_VOL_CYLINDER:
                    file<<"\"cylinder\","<<object->cyl_l<<","<<object->cyl_r<<","<<object->cyl_cut;
                    break;
                case Sel::OBJ_DISK:
                    file<<"\"disk\","<<object->dsk_r<<","<<object->dsk_r_in;
                    break;
                case Sel::OBJ_LENS:
                    file<<"\"lens\","<<object->ls_thickness<<","<<object->ls_r_max_nominal<<","<<object->ls_r1<<","<<object->ls_r2;
                    break;
                case Sel::OBJ_MESH:
                    file<<"\"mesh\",\""<<object->mesh_fname<<"\"";
                    break;
                case Sel::OBJ_RECTANGLE:
                    file<<"\"rectangle\","<<object->box_ly<<","<<object->box_lz;
                    break;
                case Sel::OBJ_PARABOLA:
                    file<<"\"parabola\","<<object->pr_f<<","<<object->pr_in_radius<<","<<object->pr_length;
                    break;
                case Sel::OBJ_SPHERE:
                    file<<"\"sphere\","<<object->sph_r<<","<<object->sph_cut;
                    break;
                case Sel::OBJ_SPHERE_PATCH:
                    file<<"\"spherical_patch\","<<object->sph_r<<","<<object->sph_cut;
                    break;
            }
            
            file<<")\n";
        }
    }
    
    file<<"\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr)
        {
            file<<ID[i]<<"=Selene_light(";
            
            switch(light->type)
            {
                case Sel::SRC_CONE: file<<"\"cone\""; break;
                case Sel::SRC_GAUSSIAN_BEAM: file<<"\"gaussian_beam\""; break;
                case Sel::SRC_LAMBERTIAN: file<<"\"lambertian\""; break;
                case Sel::SRC_POINT: file<<"\"point\""; break;
                case Sel::SRC_POINT_PLANAR: file<<"\"point_planar\""; break;
                case Sel::SRC_PERFECT_BEAM: file<<"\"perfect_beam\""; break;
                case Sel::SRC_USER_DEFINED: file<<"\"user_defined\""; break;
            }
            
            file<<")\n";
        }
    }
    
    file<<"\n";
    
    // Frame properties
    file<<"-- Frame properties\n\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        file<<ID[i]<<":name(\""<<frames[i]->name<<"\")\n";
        
        bool found;
        int k;
        
        if(frames[i]->origin_anchor!=0)
            file<<ID[i]<<":origin("<<frames[i]->get_anchor_script_name(frames[i]->origin_anchor)<<")\n";
        
        if(frames[i]->relative_origin!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->relative_origin);
            file<<ID[i]<<":relative_origin("<<ID[k]<<","
                <<frames[k]->get_anchor_script_name(frames[i]->relative_anchor)<<")\n";
        }
        
        if(frames[i]->translation_frame!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->translation_frame);
            file<<ID[i]<<":translation_frame("<<ID[k]<<")\n";
        }
        
        if(frames[i]->rotation_frame!=nullptr)
        {
            k=vector_locate(found,frames,frames[i]->rotation_frame);
            file<<ID[i]<<":rotation_frame("<<ID[k]<<")\n";
        }
        
        file<<ID[i]<<":location("<<frames[i]->in_displacement.x<<","
                                 <<frames[i]->in_displacement.y<<","
                                 <<frames[i]->in_displacement.z<<")\n";
        file<<ID[i]<<":rotation("<<frames[i]->in_A.degree()<<","
                                 <<frames[i]->in_B.degree()<<","
                                 <<frames[i]->in_C.degree()<<")\n";
        
        file<<"\n";
    }
    
    // Specific object properties
    file<<"-- Specific object properties\n\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr)
        {
            int j;
            
            int N=object->get_N_faces();
            Sel::SelFace& (Sel::Object::*face_access)(int)=&Sel::Object::face;
            std::string face_name="face";
            
            if(object->type==Sel::OBJ_MESH)
            {
                N=object->get_N_faces_groups();
                face_access=&Sel::Object::faces_group;
                face_name="faces_group";
            }
            
            // Mesh Specifics
            
            if(object->scaled_mesh)
                file<<ID[i]<<":rescale_mesh("<<object->scaling_factor<<")\n";
            for(std::size_t k=0;k<object->Fg_arr.size();k++)
                file<<ID[i]<<":define_faces_group("<<k<<","<<object->Fg_start[k]<<","<<object->Fg_end[k]<<")\n";
                        
            //#####################
            //   Materials check
            //#####################
            
            // Up Material
            
            Material *mat=object->face(0).up_mat;
            bool out_material_valid=true;
            
            for(j=1;j<object->get_N_faces();j++) if(mat!=object->face(j).up_mat)
                { out_material_valid=false; break; }
            
            int k;
            bool found;
            
            if(out_material_valid)
            {
                k=vector_locate(found,materials,dynamic_cast<GUI::Material*>(mat));
                file<<ID[i]<<":default_out_mat(material_"<<k<<")\n";
            }
            else
            {
                for(j=0;j<N;j++)
                {
                    k=vector_locate(found,materials,dynamic_cast<GUI::Material*>((object->*face_access)(j).up_mat));
                    file<<ID[i]<<":"<<face_name<<"_up_mat("<<j<<",material_"<<k<<")\n";
                }
            }
            
            // Down Material
            
            mat=object->face(0).down_mat;
            bool in_material_valid=true;
            
            for(int j=1;j<object->get_N_faces();j++) if(mat!=object->face(j).down_mat)
                { in_material_valid=false; break; }
            
            if(in_material_valid)
            {
                k=vector_locate(found,materials,dynamic_cast<GUI::Material*>(mat));
                file<<ID[i]<<":default_in_mat(material_"<<k<<")\n";
            }
            else
            {
                for(j=0;j<N;j++)
                {
                    k=vector_locate(found,materials,dynamic_cast<GUI::Material*>((object->*face_access)(j).down_mat));
                    file<<ID[i]<<":"<<face_name<<"_down_mat("<<j<<",material_"<<k<<")\n";
                }
            }
            
            //################
            //   IRFs check
            //################
            
            Sel::IRF *irf1=object->face(0).up_irf;
            Sel::IRF *irf2=object->face(0).down_irf;
            
            bool default_IRF_valid=true;
            bool out_IRF_valid=true;
            bool in_IRF_valid=true;
            
            for(j=1;j<object->get_N_faces();j++)
            {
                if(irf1!=object->face(j).up_irf ||
                   irf1!=object->face(j).down_irf)
                {
                    default_IRF_valid=false;
                }
                
                if(irf1!=object->face(j).up_irf) out_IRF_valid=false;
                if(irf2!=object->face(j).down_irf) in_IRF_valid=false;
            }
            
            std::string IRF_ID;
            
            if(default_IRF_valid)
            {
                IRF_ID=get_IRF_script_name(irf1);
                file<<ID[i]<<":default_IRF("<<IRF_ID<<")\n";
            }
            else
            {
                if(out_IRF_valid)
                {
                    IRF_ID=get_IRF_script_name(irf1);
                    file<<ID[i]<<":default_out_IRF("<<IRF_ID<<")\n";
                }
                else
                {
                    for(int j=0;j<N;j++)
                    {
                        IRF_ID=get_IRF_script_name((object->*face_access)(j).up_irf);
                        file<<ID[i]<<":"<<face_name<<"_up_IRF("<<j<<","<<IRF_ID<<")\n";
                    }
                }
                
                if(in_IRF_valid)
                {
                    IRF_ID=get_IRF_script_name(irf2);
                    file<<ID[i]<<":default_in_IRF("<<IRF_ID<<")\n";
                }
                else
                {
                    for(int j=0;j<N;j++)
                    {
                        IRF_ID=get_IRF_script_name((object->*face_access)(j).down_irf);
                        file<<ID[i]<<":"<<face_name<<"_down_IRF("<<j<<","<<IRF_ID<<")\n";
                    }
                }
            }
            
            // Tangents
            
            for(j=0;j<N;j++)
            {
                switch((object->*face_access)(j).tangent_up)
                {
                    case Sel::TANGENT_FIXED:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.x<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.y<<","
                                                     <<(object->*face_access)(j).fixed_tangent_up.z<<")\n";
                        break;
                    case Sel::TANGENT_EXPAND:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"expand\")\n";
                        break;
                    case Sel::TANGENT_EXPAND_NEG:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"expand_neg\")\n";
                        break;
                    case Sel::TANGENT_POLAR:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"polar\")\n";
                        break;
                    case Sel::TANGENT_POLAR_NEG:
                        file<<ID[i]<<":"<<face_name<<"_up_tangent("<<j<<",\"polar_neg\")\n";
                        break;
                }
                switch((object->*face_access)(j).tangent_down)
                {
                    case Sel::TANGENT_FIXED:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.x<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.y<<","
                                                     <<(object->*face_access)(j).fixed_tangent_down.z<<")\n";
                        break;
                    case Sel::TANGENT_EXPAND:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"expand\")\n";
                        break;
                    case Sel::TANGENT_EXPAND_NEG:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"expand_neg\")\n";
                        break;
                    case Sel::TANGENT_POLAR:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"polar\")\n";
                        break;
                    case Sel::TANGENT_POLAR_NEG:
                        file<<ID[i]<<":"<<face_name<<"_down_tangent("<<j<<",\"polar_neg\")\n";
                        break;
                }
            }
            
            // Sensor properties
            
            if(object->sensor_type!=Sel::SENS_NONE)
            {
                file<<ID[i]<<":sensor(";
                     if(object->sensor_type==Sel::SENS_ABS) file<<"\"abs\"";
                else if(object->sensor_type==Sel::SENS_TRANSP) file<<"\"transp\"";
                
                if(object->sens_wavelength) file<<",\"wavelength\"";
                if(object->sens_source) file<<",\"source\"";
                if(object->sens_generation) file<<",\"generation\"";
                if(object->sens_path) file<<",\"path\"";
                if(object->sens_phase) file<<",\"phase\"";
                if(object->sens_ray_world_intersection) file<<",\"world_intersection\"";
                if(object->sens_ray_world_direction) file<<",\"world_direction\"";
                if(object->sens_ray_obj_intersection) file<<",\"obj_intersection\"";
                if(object->sens_ray_obj_direction) file<<",\"obj_direction\"";
                if(object->sens_ray_obj_face) file<<",\"obj_face\"";
                
                file<<")\n";
            }
            
            file<<"\n";
        }
    }
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr)
        {
            if(light->power!=1.0)
                file<<ID[i]<<":power("<<light->power<<")\n";
            
            // Geometric properties
            
            if(light->type==Sel::SRC_CONE)
            {
                file<<ID[i]<<":full_angle("<<light->cone_angle*180.0/Pi<<")\n";
            }
            else if(light->type==Sel::SRC_GAUSSIAN_BEAM)
            {
                file<<ID[i]<<":numerical_aperture("<<light->beam_numerical_aperture<<")\n";
                file<<ID[i]<<":waist_distance("<<light->beam_waist_distance<<")\n";
            }
            
            // Materials check
            
            Material *mat=light->amb_mat;
            
            int k;
            bool found;
            
            k=vector_locate(found,materials,dynamic_cast<GUI::Material*>(mat));
            file<<ID[i]<<":material(material_"<<k<<")\n";
            
            // Spectral Properties
            
            if(light->spectrum_type==Sel::SPECTRUM_MONO)
            {
                file<<ID[i]<<":wavelength("<<light->lambda_mono<<")\n";
            }
            else if(light->spectrum_type==Sel::SPECTRUM_POLYMONO)
            {
                file<<ID[i]<<":discrete_spectrum({";
                
                for(std::size_t j=0;j<light->polymono_lambda.size();j++)
                {
                    file<<light->polymono_lambda[j];
                    if(j+1!=light->polymono_lambda.size())
                        file<<",";
                }
                
                file<<"},{";
                
                for(std::size_t j=0;j<light->polymono_weight.size();j++)
                {
                    file<<light->polymono_weight[j];
                    if(j+1!=light->polymono_weight.size())
                        file<<",";
                }
                
                file<<"})\n";
            }
            else if(light->spectrum_type==Sel::SPECTRUM_POLY)
            {
                switch(light->spectrum_shape)
                {
                    case Sel::SPECTRUM_FILE:
                        file<<ID[i]<<":spectrum(\"file\",\""<<light->spectrum_file<<"\")\n";
                        break;
                    case Sel::SPECTRUM_FLAT:
                        file<<ID[i]<<":spectrum(\"flat\","<<light->lambda_min<<","<<light->lambda_max<<")\n";
                        break;
                    case Sel::SPECTRUM_PLANCK:
                        file<<ID[i]<<":spectrum(\"planck\","
                                              <<light->lambda_min<<","
                                              <<light->lambda_max<<","
                                              <<light->planck_temperature<<")\n";
                        break;
                }
            }
            
            // Polarization Properties
            
            switch(light->polar_type)
            {
                case Sel::POLAR_ALONG:
                    file<<ID[i]<<":polar_along("<<light->polar_vector.x<<","
                                                <<light->polar_vector.y<<","
                                                <<light->polar_vector.z<<")\n";
                    break;
                case Sel::POLAR_NOT:
                    file<<ID[i]<<":polar_not("<<light->polar_vector.x<<","
                                              <<light->polar_vector.y<<","
                                              <<light->polar_vector.z<<")\n";
                    break;
            }
            
            // Geometric Properties
            
            switch(light->extent)
            {
                case Sel::EXTENT_CIRCLE:
                    file<<ID[i]<<":extent(\"circle\","<<light->extent_d<<")\n";
                    break;
                case Sel::EXTENT_ELLIPSE:
                    file<<ID[i]<<":extent(\"ellipse\","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_ELLIPSOID:
                    file<<ID[i]<<":extent(\"ellipsoid\","<<light->extent_x<<","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_RECTANGLE:
                    file<<ID[i]<<":extent(\"rectangle\","<<light->extent_y<<","<<light->extent_z<<")\n";
                    break;
                case Sel::EXTENT_SPHERE:
                    file<<ID[i]<<":extent(\"sphere\","<<light->extent_d<<")\n";
                    break;
            }
                
            file<<"\n";
            
        }
    }
    
    file<<"\n";
    
    // Simulation properties
    file<<"-- Simulation properties\n\n";
    
    file<<"selene=MODE(\"selene\")\n\n";
    file<<"selene:N_rays_total("<<nr_tot->get_value()<<")\n";
    file<<"selene:N_rays_disp("<<nr_disp->get_value()<<")\n\n";
    file<<"selene:output_directory(\""<<output_directory_std.generic_string()<<"\")\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
        
        if(object!=nullptr) file<<"selene:add_object("<<ID[i]<<")\n";
    }
    
    file<<"\n";
    
    for(std::size_t i=0;i<frames.size();i++)
    {
        Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
        
        if(light!=nullptr) file<<"selene:add_light("<<ID[i]<<")\n";
    }
    
    file<<"\nselene:render()";
}

std::string SeleneFrame::get_IRF_script_name(Sel::IRF *irf)
{
    int k;
    bool found;
    
    k=vector_locate(found,user_irfs,irf);
                
    if(found) return "irf_"+std::to_string(k);
    else
    {
             if(irf==&irfs[0]) return "SEL_IRF_FRESNEL";
        else if(irf==&irfs[1]) return "SEL_IRF_PERFECT_ABSORBER";
        else if(irf==&irfs[2]) return "SEL_IRF_PERFECT_ANTIREFLECTOR";
        else if(irf==&irfs[3]) return "SEL_IRF_PERFECT_MIRROR";
    }
    
    return "";
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
                                    object->box_lx,object->box_ly,object->box_lz,
                                    -0.5,-0.5,-0.5);
        }
        else if(object->type==Sel::OBJ_VOL_CYLINDER)
        {
            cylinder_cut_mesh_wireframe(V_arr,F_arr,object->cyl_l,object->cyl_r,object->cyl_cut);
        }
        else if(object->type==Sel::OBJ_CONIC)
        {
            conic_section_mesh_wireframe(V_arr,F_arr,128,
                                         object->conic_R,object->conic_K,
                                         object->conic_in_radius,object->conic_out_radius);
    
        }
        else if(object->type==Sel::OBJ_DISK)
        {
            disk_mesh_wireframe(V_arr,F_arr,object->dsk_r,object->dsk_r_in);
        }
        else if(object->type==Sel::OBJ_LENS)
        {
            lens_mesh_wireframe(V_arr,F_arr,128,
                                object->ls_thickness,object->ls_r_max_nominal,object->ls_r1,object->ls_r2);
        }
        else if(object->type==Sel::OBJ_PARABOLA)
        {
            parabola_mesh_wireframe(V_arr,F_arr,128,
                                    object->pr_f,
                                    object->pr_in_radius,
                                    object->pr_length);
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
            rectangle_mesh_wireframe(V_arr,F_arr,object->box_ly,object->box_lz);
        }
        else if(object->type==Sel::OBJ_SPHERE)
        {
            sphere_cut_mesh_wireframe(V_arr,F_arr,object->sph_r,object->sph_cut,true);
        }
        else if(object->type==Sel::OBJ_SPHERE_PATCH)
        {
            sphere_cut_mesh_wireframe(V_arr,F_arr,object->sph_r,object->sph_cut,false);
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
            double s2=0.05/2.0;
            double s3=0.05*std::sqrt(2.0)/2.0;
            
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
    
    Vector3 O=frame->loc,
            A=frame->local_x,
            B=frame->local_y,
            C=frame->local_z;
    
    vao->set_mesh_wireframe(V_arr,F_arr);
    vao->set_shading(color,true,true);
    vao->set_matrix(O,A,B,C);
}

}
