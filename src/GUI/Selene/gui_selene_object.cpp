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

#include <gui_selene.h>
#include <aether.h>

#include <wx/notebook.h>

namespace SelGUI
{
    
//########################
//   ObjectDialog
//########################

ObjectDialog::ObjectDialog(Sel::Object *object_,
                           std::vector<Sel::Frame*> const &frames_,
                           std::vector<GUI::Material*> const &materials_,
                           std::vector<Sel::IRF*> const &irfs_,
                           OptimEngine &optim_engine_)
    :FrameDialog(dynamic_cast<Sel::Frame*>(object_),frames_,optim_engine_),
     object(object_),
     materials(materials_),
     irfs(irfs_)
{
        
    // - - Prism
    
//    ang_A=frame->prism_a1;
//    ang_B=frame->prism_a2;
//    
//    prism_length=new LengthSelector(ctrl_panel,"Length",frame->prism_length,true);
//    prism_height=new LengthSelector(ctrl_panel,"Height",frame->prism_height,true);
//    prism_a1=new NamedTextCtrl<double>(ctrl_panel,"Angle 1",ang_A.degree(),true);
//    prism_a2=new NamedTextCtrl<double>(ctrl_panel,"Angle 2",ang_B.degree(),true);
//    prism_width=new LengthSelector(ctrl_panel,"Max Width",frame->prism_width,true);
//    
//    prism_length->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
//    prism_height->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
//    prism_a1->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
//    prism_a2->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
//    prism_width->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
//    
//    geometry_sizer->Add(prism_length,wxSizerFlags().Expand().Border(wxALL,1));
//    geometry_sizer->Add(prism_height,wxSizerFlags().Expand().Border(wxALL,1));
//    geometry_sizer->Add(prism_a1,wxSizerFlags().Expand().Border(wxALL,1));
//    geometry_sizer->Add(prism_a2,wxSizerFlags().Expand().Border(wxALL,1));
//    geometry_sizer->Add(prism_width,wxSizerFlags().Expand().Border(wxALL,1));
    
    //
    
    
    // Display
    
    wxPanel *display_panel=new wxPanel(panels_book);
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    gl=new GL_Selene_Minimal(display_panel);
    
    display_sizer->Add(gl,wxSizerFlags(1).Expand());
    display_panel->SetSizer(display_sizer);
    
    panels_book->AddPage(display_panel,"Geometry");
    
    // Interfaces
        
    for(std::size_t i=0;i<materials.size();i++) materials_str.Add(materials[i]->name);
    for(std::size_t i=0;i<irfs.size();i++) irfs_str.Add(irfs[i]->name);
    
    if(object->type!=Sel::OBJ_MESH)
    {
        wxScrolledWindow *interf_panel=new wxScrolledWindow(panels_book);
        wxBoxSizer *interf_sizer=new wxBoxSizer(wxVERTICAL);
        
        // - Defaults
        
        wxStaticBoxSizer *default_sizer=new wxStaticBoxSizer(wxVERTICAL,interf_panel,"Defaults");
        
        wxGridSizer *btn_sizer=new wxGridSizer(3);
        wxButton *btn_out_mat=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"Out Material");
        wxButton *btn_in_mat=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"In Material");
        wxButton *btn_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"IRF");
        wxButton *btn_out_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"Out IRF");
        wxButton *btn_in_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"In IRF");
        
        btn_out_mat->Bind(wxEVT_BUTTON,&ObjectDialog::evt_out_material,this);
        btn_in_mat->Bind(wxEVT_BUTTON,&ObjectDialog::evt_in_material,this);
        btn_irf->Bind(wxEVT_BUTTON,&ObjectDialog::evt_IRF,this);
        btn_out_irf->Bind(wxEVT_BUTTON,&ObjectDialog::evt_out_IRF,this);
        btn_in_irf->Bind(wxEVT_BUTTON,&ObjectDialog::evt_in_IRF,this);
        
        btn_sizer->Add(new wxPanel(default_sizer->GetStaticBox()));
        btn_sizer->Add(btn_out_mat,wxSizerFlags().Expand());
        btn_sizer->Add(btn_in_mat,wxSizerFlags().Expand());
        btn_sizer->Add(btn_irf,wxSizerFlags().Expand());
        btn_sizer->Add(btn_out_irf,wxSizerFlags().Expand());
        btn_sizer->Add(btn_in_irf,wxSizerFlags().Expand());
        
        default_sizer->Add(btn_sizer,wxSizerFlags());
        interf_sizer->Add(default_sizer,wxSizerFlags().Expand());
        
        // - Faces
        
        faces=object->F_arr;
        
        int Ni=faces.size();
        
        up_mat.reserve(Ni);
        down_mat.reserve(Ni);
        up_irf.reserve(Ni);
        down_irf.reserve(Ni);
        
        tangent_up.reserve(Ni);
        tangent_up_x.reserve(Ni);
        tangent_up_y.reserve(Ni);
        tangent_up_z.reserve(Ni);
        
        tangent_down.reserve(Ni);
        tangent_down_x.reserve(Ni);
        tangent_down_y.reserve(Ni);
        tangent_down_z.reserve(Ni);
        
        wxArrayString tangent_type_str;
        tangent_type_str.Add("Unset");
        tangent_type_str.Add("Fixed");
        tangent_type_str.Add("Expand");
        tangent_type_str.Add("Expand Inverted");
        tangent_type_str.Add("Polar");
        tangent_type_str.Add("Polar Inverted");
        
        for(int i=0;i<Ni;i++)
        {
            wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxVERTICAL,interf_panel,object->face_name(i));
            wxStaticBox *panel=sizer->GetStaticBox();
            wxFlexGridSizer *grid=new wxFlexGridSizer(2);
            
            Vector3 color;
            color.rand_sph();
            
            color.x=std::abs(color.x);
            color.y=std::abs(color.y);
            color.z=std::abs(color.z);
            
            double min_val=color.least_dominant_val();
            double max_val=color.most_dominant_val();
            
            color.x=0.8+0.2*(color.x-min_val)/(max_val-min_val);
            color.y=0.8+0.2*(color.y-min_val)/(max_val-min_val);
            color.z=0.8+0.2*(color.z-min_val)/(max_val-min_val);
            
            color*=255.0;
            
            panel->SetBackgroundColour(wxColor(color.x,color.y,color.z));
            
            grid->AddGrowableCol(1,1);
            
            std::string ud[2]={"Up ","Down "};
            Material *mats_ud[2]={faces[i].up_mat,faces[i].down_mat};
            Sel::IRF *irfs_ud[2]={faces[i].up_irf,faces[i].down_irf};
            int tangents_ud[2]={faces[i].tangent_up,faces[i].tangent_down};
            Vector3 tangents_fixed_ud[2]={faces[i].fixed_tangent_up,faces[i].fixed_tangent_down};
            
            for(int j=0;j<2;j++)
            {
                if(j==1)
                {
                    grid->Add(new wxStaticLine(panel),wxSizerFlags().Border(wxUP|wxDOWN,5).Expand());
                    grid->Add(new wxStaticLine(panel),wxSizerFlags().Border(wxUP|wxDOWN,5).Expand());
                }
                
                int k;
                bool found;
                
                // Material
                            
                wxStaticText *mat_text=new wxStaticText(panel,wxID_ANY,ud[j]+"Material: ");
                wxChoice *mat=new wxChoice(panel,wxID_ANY);
                
                mat->Set(materials_str);
                mat->SetSelection(0);
                
                k=vector_locate(found,materials,dynamic_cast<GUI::Material*>(mats_ud[j]));
                
                if(found) mat->SetSelection(k);
                
                grid->Add(mat_text);
                grid->Add(mat,wxSizerFlags().Expand());
                
                // IRF
                
                wxStaticText *irf_text=new wxStaticText(panel,wxID_ANY,ud[j]+"IRF: ");
                wxChoice *irf=new wxChoice(panel,wxID_ANY);
                
                irf->Set(irfs_str);
                irf->SetSelection(0);
                
                k=vector_locate(found,irfs,irfs_ud[j]);
                
                if(found) irf->SetSelection(k);
                
                grid->Add(irf_text);
                grid->Add(irf,wxSizerFlags().Expand());
                
                // Tangent
                
                wxStaticText *tangent_text=new wxStaticText(panel,wxID_ANY,ud[j]+"Tangent: ");
                grid->Add(tangent_text);
                
                wxBoxSizer *tangent_sizer=new wxBoxSizer(wxHORIZONTAL);
                
                wxChoice *ntangent_type=new wxChoice(panel,wxID_ANY);
                ntangent_type->Set(tangent_type_str);
                
                switch(tangents_ud[j])
                {
                    case Sel::TANGENT_UNSET: ntangent_type->SetSelection(0); break;
                    case Sel::TANGENT_FIXED: ntangent_type->SetSelection(1); break;
                    case Sel::TANGENT_EXPAND: ntangent_type->SetSelection(2); break;
                    case Sel::TANGENT_EXPAND_NEG: ntangent_type->SetSelection(3); break;
                    case Sel::TANGENT_POLAR: ntangent_type->SetSelection(4); break;
                    case Sel::TANGENT_POLAR_NEG: ntangent_type->SetSelection(5); break;
                    default: ntangent_type->SetSelection(0);
                }
                
                tangent_sizer->Add(ntangent_type);
                
                NamedTextCtrl<double> *ntangent_x=new NamedTextCtrl<double>(panel,"X: ",tangents_fixed_ud[j].x);
                NamedTextCtrl<double> *ntangent_y=new NamedTextCtrl<double>(panel,"Y: ",tangents_fixed_ud[j].y);
                NamedTextCtrl<double> *ntangent_z=new NamedTextCtrl<double>(panel,"Z: ",tangents_fixed_ud[j].z);
                
                tangent_sizer->Add(ntangent_x);
                tangent_sizer->Add(ntangent_y);
                tangent_sizer->Add(ntangent_z);
                
                grid->Add(tangent_sizer,wxSizerFlags().Expand());
                
                // Storing
                
                if(j==0)
                {
                    up_mat.push_back(mat);
                    up_irf.push_back(irf);
                    
                    tangent_up.push_back(ntangent_type);
                    
                    tangent_up_x.push_back(ntangent_x);
                    tangent_up_y.push_back(ntangent_y);
                    tangent_up_z.push_back(ntangent_z);
                }
                else
                {
                    down_mat.push_back(mat);
                    down_irf.push_back(irf);
                    
                    tangent_down.push_back(ntangent_type);
                    
                    tangent_down_x.push_back(ntangent_x);
                    tangent_down_y.push_back(ntangent_y);
                    tangent_down_z.push_back(ntangent_z);
                }
            }
            
            sizer->Add(grid,wxSizerFlags().Expand());
            interf_sizer->Add(sizer,wxSizerFlags().Expand());
            
        }
        
        interf_panel->SetSizer(interf_sizer);
        interf_panel->SetScrollRate(10,10);
        interf_panel->FitInside();
        
        panels_book->AddPage(interf_panel,"Interfaces");
    }
    
    // Sensor
    
    ObjectDialogSensor();
    
    // OpenGl check timer
    
    gl_check=new wxTimer(this);
    Bind(wxEVT_TIMER,&ObjectDialog::evt_gl_check,this);
}

void ObjectDialog::ObjectDialogSensor()
{
    wxScrolledWindow *sensor_panel=new wxScrolledWindow(panels_book);
    wxBoxSizer *sensor_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Type
    
    wxString sensor_choice[3]={"None","Absorbing","Transparent"};
    sensor_type=new wxRadioBox(sensor_panel,wxID_ANY,"Sensor Type",wxDefaultPosition,wxDefaultSize,3,sensor_choice,3);
    
    switch(object->sensor_type)
    {
        case Sel::Sensor::NONE: sensor_type->SetSelection(0); break;
        case Sel::Sensor::ABS: sensor_type->SetSelection(1); break;
        case Sel::Sensor::TRANSP: sensor_type->SetSelection(2); break;
    }
    
    sensor_sizer->Add(sensor_type);
    
    // Choice
    
    wxStaticBoxSizer *record_sizer=new wxStaticBoxSizer(wxVERTICAL,sensor_panel,"Record");
    
    ray_wavelength=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Wavelength");
    ray_source=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Source ID");
    ray_path=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Path ID");
    ray_generation=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Generation");
    ray_length=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Optical Path Length");
    ray_phase=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Phase");
    ray_world_intersection=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"World Intersection");
    ray_world_direction=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"World Direction");
    ray_world_polarization=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"World Polarization");
    ray_obj_intersection=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Object Intersection");
    ray_obj_direction=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Object Direction");
    ray_obj_polarization=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Object Polarization");
    ray_obj_face=new wxCheckBox(record_sizer->GetStaticBox(),wxID_ANY,"Face Hit");
    
    ray_wavelength->SetValue(object->sens_wavelength);
    ray_source->SetValue(object->sens_source);
    ray_generation->SetValue(object->sens_generation);
    ray_path->SetValue(object->sens_path);
    ray_length->SetValue(object->sens_length);
    ray_phase->SetValue(object->sens_phase);
    ray_world_intersection->SetValue(object->sens_ray_world_intersection);
    ray_world_direction->SetValue(object->sens_ray_world_direction);
    ray_world_polarization->SetValue(object->sens_ray_world_polar);
    ray_obj_intersection->SetValue(object->sens_ray_obj_intersection);
    ray_obj_direction->SetValue(object->sens_ray_obj_direction);
    ray_obj_polarization->SetValue(object->sens_ray_obj_polar);
    ray_obj_face->SetValue(object->sens_ray_obj_face);
    
    record_sizer->Add(ray_wavelength,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_source,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_path,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_generation,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_length,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_phase,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_world_intersection,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_world_direction,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_world_polarization,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_obj_intersection,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_obj_direction,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_obj_polarization,wxSizerFlags().Border(wxALL,3));
    record_sizer->Add(ray_obj_face,wxSizerFlags().Border(wxALL,3));
    
    sensor_sizer->Add(record_sizer);
    
    sensor_panel->SetSizer(sensor_sizer);
    sensor_panel->SetScrollRate(10,10);
    
    panels_book->AddPage(sensor_panel,"Sensor");
}

void ObjectDialog::evt_gl_check(wxTimerEvent &event)
{
    if(gl->gl_ok)
    {
        refresh_mesh();
        gl_check->Stop();
    }
}

void ObjectDialog::evt_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the default IRF for both sides of every face","Default IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<up_irf.size();i++)
        {
            up_irf[i]->SetSelection(selection);
            down_irf[i]->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void ObjectDialog::evt_in_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the IRF of the object's inside, for every face","Default In IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<up_irf.size();i++)
        {
            down_irf[i]->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void ObjectDialog::evt_in_material(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the material of the object","Default In Material",materials_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<up_irf.size();i++)
        {
            down_mat[i]->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void ObjectDialog::evt_geometry(wxCommandEvent &event)
{
    refresh_mesh();
    
    event.Skip();
}

void ObjectDialog::evt_out_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the IRF of the object's outside, for every face","Default Out IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<up_irf.size();i++)
        {
            up_irf[i]->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void ObjectDialog::evt_out_material(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the ambient material of the object","Default Out Material",materials_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<up_irf.size();i++)
        {
            up_mat[i]->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void ObjectDialog::refresh_mesh()
{
    double eff_r_max=1.0;
    
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
    eff_r_max=mesh(V_arr,F_arr);
    
    gl->focus();
    gl->set_camera_radius(2.0*eff_r_max);
    
    SeleneVAO *vao=gl->get_vao();
    vao->set_mesh_wireframe(V_arr,F_arr);
    vao->set_shading(Vector3(1.0,1.0,1.0),true,true);
    
    gl->set_camera_direction(Vector3(1.0,1.0,-1.0));
    gl->set_camera_radius(2.0*eff_r_max);
}

double ObjectDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    return 1.0;
    
//    else if(type==Sel::OBJ_PRISM) eff_r_max=mesh_prism(V_arr,F_arr);
//    else if(type==Sel::OBJ_SQUARE_MIRROR ||
//            type==Sel::OBJ_BOX) eff_r_max=mesh_plate(V_arr,F_arr);
    
}

//double ObjectDialog::mesh_prism(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
//{
//    prism_mesh_wireframe(V_arr,F_arr,
//                         prism_length->get_length(),prism_height->get_length(),
//                         Degree(prism_a1->get_value()),Degree(prism_a2->get_value()),
//                         prism_width->get_length());
//    
//    return 2*prism_length->get_length();
//}
//

void ObjectDialog::save_object()
{
    if(object->type!=Sel::OBJ_MESH)
    {
        for(std::size_t i=0;i<faces.size();i++)
        {
            // Material
            
            if(materials.size()>0)
            {
                faces[i].up_mat=materials[up_mat[i]->GetSelection()];
                faces[i].down_mat=materials[down_mat[i]->GetSelection()];
            }
            else
            {
                faces[i].up_mat=nullptr;
                faces[i].down_mat=nullptr;
            }
            
            // Irf
            
            faces[i].up_irf=irfs[up_irf[i]->GetSelection()];
            faces[i].down_irf=irfs[down_irf[i]->GetSelection()];
            
            // Tangent
            
            int j=tangent_up[i]->GetSelection();
            
            switch(j)
            {
                case 0: faces[i].tangent_up=Sel::TANGENT_UNSET; break;
                case 1: faces[i].tangent_up=Sel::TANGENT_FIXED; break;
                case 2: faces[i].tangent_up=Sel::TANGENT_EXPAND; break;
                case 3: faces[i].tangent_up=Sel::TANGENT_EXPAND_NEG; break;
                case 4: faces[i].tangent_up=Sel::TANGENT_POLAR; break;
                case 5: faces[i].tangent_up=Sel::TANGENT_POLAR_NEG; break;
            }
            
            faces[i].fixed_tangent_up.x=tangent_up_x[i]->get_value();
            faces[i].fixed_tangent_up.y=tangent_up_y[i]->get_value();
            faces[i].fixed_tangent_up.z=tangent_up_z[i]->get_value();
            
            j=tangent_down[i]->GetSelection();
            
            switch(j)
            {
                case 0: faces[i].tangent_down=Sel::TANGENT_UNSET; break;
                case 1: faces[i].tangent_down=Sel::TANGENT_FIXED; break;
                case 2: faces[i].tangent_down=Sel::TANGENT_EXPAND; break;
                case 3: faces[i].tangent_down=Sel::TANGENT_EXPAND_NEG; break;
                case 4: faces[i].tangent_down=Sel::TANGENT_POLAR; break;
                case 5: faces[i].tangent_down=Sel::TANGENT_POLAR_NEG; break;
            }
            
            faces[i].fixed_tangent_down.x=tangent_down_x[i]->get_value();
            faces[i].fixed_tangent_down.y=tangent_down_y[i]->get_value();
            faces[i].fixed_tangent_down.z=tangent_down_z[i]->get_value();
        }
        
        object->F_arr=faces;
    }
    
    save_object_geometry();
    
    switch(sensor_type->GetSelection())
    {
        case 0: object->sensor_type=Sel::Sensor::NONE; break;
        case 1: object->sensor_type=Sel::Sensor::ABS; break;
        case 2: object->sensor_type=Sel::Sensor::TRANSP; break;
    }
    
    object->sens_wavelength=ray_wavelength->GetValue();
    object->sens_source=ray_source->GetValue();
    object->sens_generation=ray_generation->GetValue();
    object->sens_path=ray_path->GetValue();
    object->sens_length=ray_length->GetValue();
    object->sens_phase=ray_phase->GetValue();
    object->sens_ray_world_intersection=ray_world_intersection->GetValue();
    object->sens_ray_world_direction=ray_world_direction->GetValue();
    object->sens_ray_world_polar=ray_world_polarization->GetValue();
    object->sens_ray_obj_intersection=ray_obj_intersection->GetValue();
    object->sens_ray_obj_direction=ray_obj_direction->GetValue();
    object->sens_ray_obj_polar=ray_obj_polarization->GetValue();
    object->sens_ray_obj_face=ray_obj_face->GetValue();
}

//#####################
//   BoxDialog
//#####################

BoxDialog::BoxDialog(Sel::Object *object_,std::vector<Sel::Frame*> const &frames_,
                     std::vector<GUI::Material*> const &materials_,
                     std::vector<Sel::IRF*> const &irfs_,
                     OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{    
    box_lx=new LengthSelector(ctrl_panel,"X Length",object->box.get_lx(),true);
    box_lx->handle_external_optimization(&object->box.ref_lx(),optim_engine);
    box_lx->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(box_lx,wxSizerFlags().Expand().Border(wxALL,1));
    
    box_ly=new LengthSelector(ctrl_panel,"Y Length",object->box.get_ly(),true);
    box_ly->handle_external_optimization(&object->box.ref_ly(),optim_engine);
    box_ly->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(box_ly,wxSizerFlags().Expand().Border(wxALL,1));
    
    box_lz=new LengthSelector(ctrl_panel,"Z Length",object->box.get_lz(),true);
    box_lz->handle_external_optimization(&object->box.ref_lz(),optim_engine);
    box_lz->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(box_lz,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double BoxDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    double eff_r_max;
    
    eff_r_max=var_max(box_lx->get_length()/2.0,
                      box_ly->get_length()/2.0,
                      box_lz->get_length()/2.0);
    
    Glite::make_block_wires(V_arr,F_arr,
                            box_lx->get_length(),
                            box_ly->get_length(),
                            box_lz->get_length(),
                            -0.5,-0.5,-0.5);
    
    return eff_r_max;
}

template<typename T>
void register_optimization(T *ctrl,double *var,OptimEngine &engine)
{
    if(ctrl->optimize) engine.register_variable(var,ctrl->optim_rule);
    else engine.forget_variable(var);
}

void BoxDialog::save_object_geometry()
{
    object->box.set_parameters(box_lx->get_length(),
                               box_ly->get_length(),
                               box_lz->get_length());
    
    register_optimization(box_lx,&object->box.ref_lx(),optim_engine);
    register_optimization(box_ly,&object->box.ref_ly(),optim_engine);
    register_optimization(box_lz,&object->box.ref_lz(),optim_engine);
    
    object->set_box();
}

//########################
//   ConicSectionDialog
//########################

ConicSectionDialog::ConicSectionDialog(Sel::Object *object_,
                                       std::vector<Sel::Frame*> const &frames_,
                                       std::vector<GUI::Material*> const &materials_,
                                       std::vector<Sel::IRF*> const &irfs_,
                                       OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    conic_R=new LengthSelector(ctrl_panel,"Conic Radius",object->conic.R_factor,true);
    conic_R->handle_external_optimization(&object->conic.R_factor,optim_engine);
    conic_R->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(conic_R,wxSizerFlags().Expand().Border(wxALL,1));
    
    conic_K=new NamedTextCtrl<double>(ctrl_panel,"Conic Constant",object->conic.K_factor,true);
    conic_K->handle_external_optimization(&object->conic.K_factor,optim_engine);
    conic_K->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(conic_K,wxSizerFlags().Expand().Border(wxALL,1));
    
    conic_in_radius=new LengthSelector(ctrl_panel,"In Radius",object->conic.in_radius,true,"mm");
    conic_in_radius->handle_external_optimization(&object->conic.in_radius,optim_engine);
    conic_in_radius->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(conic_in_radius,wxSizerFlags().Expand().Border(wxALL,1));
    
    conic_out_radius=new LengthSelector(ctrl_panel,"Out Radius",object->conic.out_radius,true);
    conic_out_radius->handle_external_optimization(&object->conic.out_radius,optim_engine);
    conic_out_radius->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(conic_out_radius,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double ConicSectionDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    conic_section_mesh_wireframe(V_arr,F_arr,128,
                                 conic_R->get_length(),conic_K->get_value(),
                                 conic_in_radius->get_length(),conic_out_radius->get_length());
    
    return 1.5*conic_R->get_length();
}

void ConicSectionDialog::save_object_geometry()
{
    object->conic.R_factor=conic_R->get_length();
    object->conic.K_factor=conic_K->get_value();
    object->conic.in_radius=conic_in_radius->get_length();
    object->conic.out_radius=conic_out_radius->get_length();
    
    register_optimization(conic_R,&object->conic.R_factor,optim_engine);
    register_optimization(conic_K,&object->conic.K_factor,optim_engine);
    register_optimization(conic_in_radius,&object->conic.in_radius,optim_engine);
    register_optimization(conic_out_radius,&object->conic.out_radius,optim_engine);
    
    object->set_conic_section();
}

//####################
//   CylinderDialog
//####################

CylinderDialog::CylinderDialog(Sel::Object *object_,
                               std::vector<Sel::Frame*> const &frames_,
                               std::vector<GUI::Material*> const &materials_,
                               std::vector<Sel::IRF*> const &irfs_,
                               OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    cyl_r=new LengthSelector(ctrl_panel,"Radius",object->cylinder.radius,true);
    cyl_r->handle_external_optimization(&object->cylinder.radius,optim_engine);
    cyl_r->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(cyl_r,wxSizerFlags().Expand().Border(wxALL,1));
    
    cyl_l=new LengthSelector(ctrl_panel,"Length",object->cylinder.length,true);
    cyl_l->handle_external_optimization(&object->cylinder.length,optim_engine);
    cyl_l->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(cyl_l,wxSizerFlags().Expand().Border(wxALL,1));
    
    cyl_cut=new NamedTextCtrl<double>(ctrl_panel,"Cut Factor",object->cylinder.cut_factor,true);
    cyl_cut->handle_external_optimization(&object->cylinder.cut_factor,optim_engine);
    cyl_cut->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(cyl_cut,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double CylinderDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    cylinder_cut_mesh_wireframe(V_arr,F_arr,
                                cyl_l->get_length(),
                                cyl_r->get_length(),
                                cyl_cut->get_value());
    
    return cyl_l->get_length();
}

void CylinderDialog::save_object_geometry()
{
    object->cylinder.length=cyl_l->get_length();
    object->cylinder.radius=cyl_r->get_length();
    object->cylinder.cut_factor=cyl_cut->get_value();
    
    register_optimization(cyl_l,&object->cylinder.length,optim_engine);
    register_optimization(cyl_r,&object->cylinder.radius,optim_engine);
    register_optimization(cyl_cut,&object->cylinder.cut_factor,optim_engine);
    
    object->set_cylinder_volume();
}

//################
//   DiskDialog
//################

DiskDialog::DiskDialog(Sel::Object *object_,
                       std::vector<Sel::Frame*> const &frames_,
                       std::vector<GUI::Material*> const &materials_,
                       std::vector<Sel::IRF*> const &irfs_,
                       OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    dsk_r=new LengthSelector(ctrl_panel,"Radius",object->disk.radius,true);
    dsk_r->handle_external_optimization(&object->disk.radius,optim_engine);
    dsk_r->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(dsk_r,wxSizerFlags().Expand().Border(wxALL,1));
    
    dsk_r_in=new LengthSelector(ctrl_panel,"In Radius",object->disk.in_radius,true,"mm");
    dsk_r_in->handle_external_optimization(&object->disk.in_radius,optim_engine);
    dsk_r_in->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(dsk_r_in,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double DiskDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    disk_mesh_wireframe(V_arr,F_arr,dsk_r->get_length(),dsk_r_in->get_length());
    
    return 1.2*dsk_r->get_length();
}

void DiskDialog::save_object_geometry()
{
    object->disk.radius=dsk_r->get_length();
    object->disk.in_radius=dsk_r_in->get_length();
    
    register_optimization(dsk_r,&object->disk.radius,optim_engine);
    register_optimization(dsk_r_in,&object->disk.in_radius,optim_engine);
    
    object->set_disk();
}

//######################
//   LensDialog
//######################

LensDialog::LensDialog(Sel::Object *object_,
                       std::vector<Sel::Frame*> const &frames_,
                       std::vector<GUI::Material*> const &materials_,
                       std::vector<Sel::IRF*> const &irfs_,
                       OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    ls_thickness=new LengthSelector(ctrl_panel,"Thickness",object->lens.thickness,true);
    ls_thickness->handle_external_optimization(&object->lens.thickness,optim_engine);
    ls_thickness->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(ls_thickness,wxSizerFlags().Expand().Border(wxALL,1));
    
    ls_r1=new LengthSelector(ctrl_panel,"In Radius",object->lens.radius_front,true);
    ls_r1->handle_external_optimization(&object->lens.radius_front,optim_engine);
    ls_r1->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(ls_r1,wxSizerFlags().Expand().Border(wxALL,1));
    
    ls_r2=new LengthSelector(ctrl_panel,"Out Radius",object->lens.radius_back,true);
    ls_r2->handle_external_optimization(&object->lens.radius_back,optim_engine);
    ls_r2->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(ls_r2,wxSizerFlags().Expand().Border(wxALL,1));
    
    ls_r_max=new LengthSelector(ctrl_panel,"Max Radius",object->lens.max_outer_radius,true);
    ls_r_max->handle_external_optimization(&object->lens.max_outer_radius,optim_engine);
    ls_r_max->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(ls_r_max,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double LensDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    double tmp1,tmp2,tmp3,tmp4,eff_r_max;
    
    Sel::lens_geometry(tmp1,tmp2,eff_r_max,tmp3,tmp4,
                       ls_thickness->get_length(),
                       ls_r_max->get_length(),
                       ls_r1->get_length(),
                       ls_r2->get_length());
    
    lens_mesh_wireframe(V_arr,F_arr,128,
                        ls_thickness->get_length(),
                        ls_r_max->get_length(),
                        ls_r1->get_length(),
                        ls_r2->get_length());
    
    return eff_r_max;
}

void LensDialog::save_object_geometry()
{
    object->lens.thickness=ls_thickness->get_length();
    object->lens.radius_front=ls_r1->get_length();
    object->lens.radius_back=ls_r2->get_length();
    object->lens.max_outer_radius=ls_r_max->get_length();
    
    register_optimization(ls_thickness,&object->lens.thickness,optim_engine);
    register_optimization(ls_r1,&object->lens.radius_front,optim_engine);
    register_optimization(ls_r2,&object->lens.radius_back,optim_engine);
    register_optimization(ls_r_max,&object->lens.max_outer_radius,optim_engine);
    
    object->set_lens();
}

//##########################
//   ParabolaDialog
//##########################

ParabolaDialog::ParabolaDialog(Sel::Object *object_,
                               std::vector<Sel::Frame*> const &frames_,
                               std::vector<GUI::Material*> const &materials_,
                               std::vector<Sel::IRF*> const &irfs_,
                               OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    pr_focal=new LengthSelector(ctrl_panel,"Focal Point",object->parabola.focal,true);
    pr_focal->handle_external_optimization(&object->parabola.focal,optim_engine);
    pr_focal->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(pr_focal,wxSizerFlags().Expand().Border(wxALL,1));
    
    pr_in_radius=new LengthSelector(ctrl_panel,"In Radius",object->parabola.inner_radius,true);
    pr_in_radius->handle_external_optimization(&object->parabola.inner_radius,optim_engine);
    pr_in_radius->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(pr_in_radius,wxSizerFlags().Expand().Border(wxALL,1));
    
    pr_height=new LengthSelector(ctrl_panel,"Height",object->parabola.length,true);
    pr_height->handle_external_optimization(&object->parabola.length,optim_engine);
    pr_height->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(pr_height,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double ParabolaDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    parabola_mesh_wireframe(V_arr,F_arr,128,
                            pr_focal->get_length(),
                            pr_in_radius->get_length(),
                            pr_height->get_length());
    
    return std::sqrt(pr_height->get_length()*4.0*pr_focal->get_length());
}

void ParabolaDialog::save_object_geometry()
{
    object->parabola.focal=pr_focal->get_length();
    object->parabola.inner_radius=pr_in_radius->get_length();
    object->parabola.length=pr_height->get_length();
    
    register_optimization(pr_focal,&object->parabola.focal,optim_engine);
    register_optimization(pr_in_radius,&object->parabola.inner_radius,optim_engine);
    register_optimization(pr_height,&object->parabola.length,optim_engine);
    
    object->set_parabola();
}

//#####################
//   RectangleDialog
//#####################

RectangleDialog::RectangleDialog(Sel::Object *object_,std::vector<Sel::Frame*> const &frames_,
                     std::vector<GUI::Material*> const &materials_,
                     std::vector<Sel::IRF*> const &irfs_,
                     OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    box_ly=new LengthSelector(ctrl_panel,"Y Length",object->rectangle.get_ly(),true,"mm");
    box_ly->handle_external_optimization(&object->rectangle.ref_ly(),optim_engine);
    box_ly->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    box_lz=new LengthSelector(ctrl_panel,"Z Length",object->rectangle.get_lz(),true,"mm");
    box_lz->handle_external_optimization(&object->rectangle.ref_lz(),optim_engine);
    box_lz->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(box_ly,wxSizerFlags().Expand().Border(wxALL,1));
    ctrl_sizer->Add(box_lz,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double RectangleDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    rectangle_mesh_wireframe(V_arr,F_arr,box_ly->get_length(),box_lz->get_length());
    
    return std::max(box_ly->get_length(),box_lz->get_length());
}

void RectangleDialog::save_object_geometry()
{
    object->rectangle.set_parameters(box_ly->get_length(),
                                     box_lz->get_length());
    
    register_optimization(box_ly,&object->rectangle.ref_ly(),optim_engine);
    register_optimization(box_lz,&object->rectangle.ref_lz(),optim_engine);
    
    object->set_rectangle();
}

//##########################
//   SphereDialog
//##########################

SphereDialog::SphereDialog(Sel::Object *object_,
                               std::vector<Sel::Frame*> const &frames_,
                               std::vector<GUI::Material*> const &materials_,
                               std::vector<Sel::IRF*> const &irfs_,
                               OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    sph_r=new LengthSelector(ctrl_panel,"Radius",object->sphere.get_radius(),true);
    sph_r->handle_external_optimization(&object->sphere.ref_radius(),optim_engine);
    sph_r->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    sph_cut=new NamedTextCtrl(ctrl_panel,"Slice Factor",object->sphere.get_cut_factor(),true);
    sph_cut->handle_external_optimization(&object->sphere.ref_cut_factor(),optim_engine);
    sph_cut->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(sph_r,wxSizerFlags().Expand().Border(wxALL,1));
    ctrl_sizer->Add(sph_cut,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double SphereDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    sphere_cut_mesh_wireframe(V_arr,F_arr,sph_r->get_length(),sph_cut->get_value(),true);
    
    return 1.5*sph_r->get_length();
}

void SphereDialog::save_object_geometry()
{
    object->sphere.set_parameters(sph_r->get_length(),
                                  sph_cut->get_value());
    
    register_optimization(sph_r,&object->sphere.ref_radius(),optim_engine);
    register_optimization(sph_cut,&object->sphere.ref_cut_factor(),optim_engine);
    
    object->set_sphere();
}

//#######################
//   SpherePatchDialog
//#######################

SpherePatchDialog::SpherePatchDialog(Sel::Object *object_,
                                     std::vector<Sel::Frame*> const &frames_,
                                     std::vector<GUI::Material*> const &materials_,
                                     std::vector<Sel::IRF*> const &irfs_,
                                     OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    sph_r=new LengthSelector(ctrl_panel,"Radius",object->sphere_patch.get_radius(),true);
    sph_r->handle_external_optimization(&object->sphere_patch.ref_radius(),optim_engine);
    sph_r->Bind(EVT_LENGTH_SELECTOR,&ObjectDialog::evt_geometry,this);
    
    sph_cut=new NamedTextCtrl(ctrl_panel,"Slice Factor",object->sphere_patch.get_cut_factor(),true);
    sph_cut->handle_external_optimization(&object->sphere_patch.ref_cut_factor(),optim_engine);
    sph_cut->Bind(EVT_NAMEDTXTCTRL,&ObjectDialog::evt_geometry,this);
    
    ctrl_sizer->Add(sph_r,wxSizerFlags().Expand().Border(wxALL,1));
    ctrl_sizer->Add(sph_cut,wxSizerFlags().Expand().Border(wxALL,1));
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
}

double SpherePatchDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    sphere_cut_mesh_wireframe(V_arr,F_arr,sph_r->get_length(),sph_cut->get_value(),false);
    
    return 1.5*sph_r->get_length();
}

void SpherePatchDialog::save_object_geometry()
{
    object->sphere_patch.set_parameters(sph_r->get_length(),
                                        sph_cut->get_value());
    
    register_optimization(sph_r,&object->sphere_patch.ref_radius(),optim_engine);
    register_optimization(sph_cut,&object->sphere_patch.ref_cut_factor(),optim_engine);
    
    object->set_spherical_patch();
}

}
