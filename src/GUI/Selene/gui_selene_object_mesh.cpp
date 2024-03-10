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

//################
//   MeshDialog
//################

FacesGroupPanel::FacesGroupPanel(wxWindow *parent,
                                 Sel::SelFace const &face,int group_start_,int group_end_,
                                 std::vector<GUI::Material*> const &materials,wxArrayString const &materials_str,
                                 std::vector<Sel::IRF*> const &irfs,wxArrayString const &irfs_str)
    :PanelsListBase(parent)
{
    // Boundaries
    
    wxBoxSizer *group_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    group_start=new NamedTextCtrl<int>(this,"Start: ",group_start_);
    group_end=new NamedTextCtrl<int>(this," End :",group_end_);
    
    group_sizer->Add(group_start);
    group_sizer->Add(group_end);
    
    sizer->Add(group_sizer);
    
    // Controls
    
//    wxArrayString tangent_type_str;
//    tangent_type_str.Add("Unset");
//    tangent_type_str.Add("Fixed");
//    tangent_type_str.Add("Expand");
//    tangent_type_str.Add("Expand Inverted");
//    tangent_type_str.Add("Polar");
//    tangent_type_str.Add("Polar Inverted");
    
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
    
    this->SetBackgroundColour(wxColor(color.x,color.y,color.z));
    
    grid->AddGrowableCol(1,1);
    
    std::string ud[2]={"Up ","Down "};
    Material *mats_ud[2]={face.up_mat,face.down_mat};
    Sel::IRF *irfs_ud[2]={face.up_irf,face.down_irf};
//    int tangents_ud[2]={face.tangent_up,face.tangent_down};
//    Vector3 tangents_fixed_ud[2]={face.fixed_tangent_up,face.fixed_tangent_down};
    
    for(int j=0;j<2;j++)
    {
        if(j==1)
        {
            grid->Add(new wxStaticLine(this),wxSizerFlags().Border(wxUP|wxDOWN,5).Expand());
            grid->Add(new wxStaticLine(this),wxSizerFlags().Border(wxUP|wxDOWN,5).Expand());
        }
        
        int k;
        bool found;
        
        // Material
                    
        wxStaticText *mat_text=new wxStaticText(this,wxID_ANY,ud[j]+"Material: ");
        wxChoice *mat=new wxChoice(this,wxID_ANY);
        
        mat->Set(materials_str);
        mat->SetSelection(0);
        
        k=vector_locate(found,materials,dynamic_cast<GUI::Material*>(mats_ud[j]));
        
        if(found) mat->SetSelection(k);
        
        grid->Add(mat_text);
        grid->Add(mat,wxSizerFlags().Expand());
        
        // IRF
        
        wxStaticText *irf_text=new wxStaticText(this,wxID_ANY,ud[j]+"IRF: ");
        wxChoice *irf=new wxChoice(this,wxID_ANY);
        
        irf->Set(irfs_str);
        irf->SetSelection(0);
        
        k=vector_locate(found,irfs,irfs_ud[j]);
        
        if(found) irf->SetSelection(k);
        
        grid->Add(irf_text);
        grid->Add(irf,wxSizerFlags().Expand());
        
        if(j==0)
        {
            up_mat=mat;
            up_irf=irf;
        }
        else
        {
            down_mat=mat;
            down_irf=irf;
        }
    }
    
    sizer->Add(grid,wxSizerFlags().Expand());
}

MeshDialog::MeshDialog(Sel::Object *object_,
                       std::vector<Sel::Frame*> const &frames_,
                       std::vector<GUI::Material*> const &materials_,
                       std::vector<Sel::IRF*> const &irfs_,
                       OptimEngine &optim_engine_)
    :ObjectDialog(object_,frames_,materials_,irfs_,optim_engine_)
{
    object->mesh.get_faces_groups(faces,
                                  groups_start,
                                  groups_end);
    
    mesh_V_arr=object->mesh.get_vertex_array();
    mesh_F_arr=object->mesh.get_faces_array();
    
    // Mesh File
    
    wxStaticBoxSizer *file_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Mesh File");
    
    mesh_file=new wxTextCtrl(file_sizer->GetStaticBox(),wxID_ANY,object->mesh.get_mesh_path().generic_string());
    mesh_file->SetEditable(false);
    
    wxButton *mesh_load_btn=new wxButton(file_sizer->GetStaticBox(),wxID_ANY,"...",
                                         wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    mesh_load_btn->Bind(wxEVT_BUTTON,&MeshDialog::evt_mesh_load,this);
    
    file_sizer->Add(mesh_file,wxSizerFlags(1));
    file_sizer->Add(mesh_load_btn);
    
    ctrl_sizer->Add(file_sizer,wxSizerFlags().Expand().Border(wxALL,1));
    
    // Mesh scaling
    
    wxStaticBoxSizer *scale_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Scaling");
    wxBoxSizer *scale_subsizer=new wxBoxSizer(wxHORIZONTAL);
    
    scaling_factor=new NamedTextCtrl<double>(scale_sizer->GetStaticBox(),"Factor: ",object->mesh.get_scaling_factor());
    
    wxButton *scaling_apply_btn=new wxButton(scale_sizer->GetStaticBox(),wxID_ANY,"Apply");
    scaling_apply_btn->Bind(wxEVT_BUTTON,&MeshDialog::evt_scale,this);
    
    mesh_scaled=new wxCheckBox(scale_sizer->GetStaticBox(),wxID_ANY,"Scaling applied");
    mesh_scaled->SetValue(object->mesh.get_scaling_status());
    mesh_scaled->Disable();
    
    scale_subsizer->Add(scaling_apply_btn);
    scale_subsizer->Add(new wxPanel(scale_sizer->GetStaticBox()),wxSizerFlags(1));
    scale_subsizer->Add(mesh_scaled);
    
    scale_sizer->Add(scaling_factor,wxSizerFlags().Expand());
    scale_sizer->Add(scale_subsizer,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(scale_sizer,wxSizerFlags().Expand().Border(wxALL,1));
    
    groups_panel=new wxScrolledWindow(panels_book);
    wxBoxSizer *groups_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Dimensions
    
    wxStaticBoxSizer *dimensions_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Dimensions");
    
    dim_x=new NamedTextCtrl<std::string>(dimensions_sizer->GetStaticBox(),"X: ",add_unit(0));
    dim_y=new NamedTextCtrl<std::string>(dimensions_sizer->GetStaticBox(),"Y: ",add_unit(0));
    dim_z=new NamedTextCtrl<std::string>(dimensions_sizer->GetStaticBox(),"Z: ",add_unit(0));
    
    dim_x->lock();
    dim_y->lock();
    dim_z->lock();
    
    dimensions_sizer->Add(dim_x,wxSizerFlags().Expand());
    dimensions_sizer->Add(dim_y,wxSizerFlags().Expand());
    dimensions_sizer->Add(dim_z,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(dimensions_sizer,wxSizerFlags().Expand());
    
    // Faces Groups
    
    // - Defaults
    
    wxStaticBoxSizer *default_sizer=new wxStaticBoxSizer(wxVERTICAL,groups_panel,"Defaults");
    
    wxGridSizer *btn_sizer=new wxGridSizer(3);
    wxButton *btn_out_mat=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"Out Material");
    wxButton *btn_in_mat=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"In Material");
    wxButton *btn_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"IRF");
    wxButton *btn_out_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"Out IRF");
    wxButton *btn_in_irf=new wxButton(default_sizer->GetStaticBox(),wxID_ANY,"In IRF");
    
    btn_out_mat->Bind(wxEVT_BUTTON,&MeshDialog::evt_out_material,this);
    btn_in_mat->Bind(wxEVT_BUTTON,&MeshDialog::evt_in_material,this);
    btn_irf->Bind(wxEVT_BUTTON,&MeshDialog::evt_IRF,this);
    btn_out_irf->Bind(wxEVT_BUTTON,&MeshDialog::evt_out_IRF,this);
    btn_in_irf->Bind(wxEVT_BUTTON,&MeshDialog::evt_in_IRF,this);
    
    btn_sizer->Add(new wxPanel(default_sizer->GetStaticBox()));
    btn_sizer->Add(btn_out_mat,wxSizerFlags().Expand());
    btn_sizer->Add(btn_in_mat,wxSizerFlags().Expand());
    btn_sizer->Add(btn_irf,wxSizerFlags().Expand());
    btn_sizer->Add(btn_out_irf,wxSizerFlags().Expand());
    btn_sizer->Add(btn_in_irf,wxSizerFlags().Expand());
    
    default_sizer->Add(btn_sizer,wxSizerFlags());
    groups_sizer->Add(default_sizer,wxSizerFlags().Expand());
    
    // - Panels
    
    wxBoxSizer *groups_subsizer=new wxBoxSizer(wxHORIZONTAL);
    
    groups_list=new PanelsList<FacesGroupPanel>(groups_panel);
    wxButton *add_group_btn=new wxButton(groups_panel,wxID_ANY,"Add Group");
    add_group_btn->Bind(wxEVT_BUTTON,&MeshDialog::evt_add_group,this);
    
    for(unsigned int i=0;i<groups_start.size();i++)
        groups_list->add_panel<FacesGroupPanel>(faces[i],groups_start[i],groups_end[i],materials,materials_str,irfs,irfs_str);
    
    groups_subsizer->Add(groups_list,wxSizerFlags(1).Expand());
    groups_subsizer->Add(add_group_btn);
    
    groups_sizer->Add(groups_subsizer,wxSizerFlags(1).Expand());
    
    groups_panel->SetSizer(groups_sizer);
    groups_panel->SetScrollRate(10,10);
    groups_panel->Layout();
    groups_panel->FitInside();
    
    panels_book->InsertPage(1,groups_panel,"Faces Groups");
    
    gl_check->Start(100);
    ctrl_panel->FitInside();
    
    Bind(EVT_PLIST_DOWN,&MeshDialog::evt_groups_reorder,this);
    Bind(EVT_PLIST_UP,&MeshDialog::evt_groups_reorder,this);
    Bind(EVT_PLIST_RESIZE,&MeshDialog::evt_groups_reorder,this);
    Bind(EVT_PLIST_REMOVE,&MeshDialog::evt_groups_reorder,this);
}

void MeshDialog::evt_add_group(wxCommandEvent &event)
{
    if(mesh_F_arr.size()==0) return;
    
    Sel::SelFace tmp_face;
    
    tmp_face.up_irf=irfs[0];
    tmp_face.down_irf=irfs[0];
    
    tmp_face.up_mat=nullptr;
    tmp_face.down_mat=nullptr;
    
    if(materials.size()>0)
    {
        tmp_face.up_mat=materials[0];
        tmp_face.down_mat=materials[0];
    }
    
    int tmp_start=0;
    int tmp_end=mesh_F_arr.size()-1;
        
    groups_list->add_panel<FacesGroupPanel>(tmp_face,tmp_start,tmp_end,materials,materials_str,irfs,irfs_str);
    
    evt_groups_reorder(event);
}

void MeshDialog::evt_groups_reorder(wxCommandEvent &event)
{
    std::size_t i,N=groups_list->get_size();
    
    for(i=0;i<N;i++)
    {
        FacesGroupPanel *panel=groups_list->get_panel(i);
        
        panel->set_title("Faces group "+std::to_string(i));
    }
    
    groups_panel->FitInside();
    groups_panel->Layout();
}

void MeshDialog::evt_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the default IRF for both sides of every face","Default IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<groups_list->get_size();i++)
        {
            groups_list->get_panel(i)->up_irf->SetSelection(selection);
            groups_list->get_panel(i)->down_irf->SetSelection(selection);
        }
    }
    
    event.Skip();
}

void MeshDialog::evt_in_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the IRF of the object's inside, for every face","Default In IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<groups_list->get_size();i++)
            groups_list->get_panel(i)->down_irf->SetSelection(selection);
    }
    
    event.Skip();
}

void MeshDialog::evt_in_material(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the material of the object","Default In Material",materials_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<groups_list->get_size();i++)
            groups_list->get_panel(i)->down_mat->SetSelection(selection);
    }
    
    event.Skip();
}

void MeshDialog::evt_out_IRF(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the IRF of the object's outside, for every face","Default Out IRF",irfs_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<groups_list->get_size();i++)
            groups_list->get_panel(i)->up_irf->SetSelection(selection);
    }
    
    event.Skip();
}

void MeshDialog::evt_out_material(wxCommandEvent &event)
{
    int selection=wxGetSingleChoiceIndex("Select the ambient material of the object","Default Out Material",materials_str);
    
    if(selection!=-1)
    {
        for(std::size_t i=0;i<groups_list->get_size();i++)
            groups_list->get_panel(i)->up_mat->SetSelection(selection);
    }
    
    event.Skip();
}

void MeshDialog::evt_mesh_load(wxCommandEvent &event)
{
    wxFileName fname=wxFileSelector("Load Mesh File...",
                                    wxEmptyString,wxEmptyString,
                                    "obj","Object file (*.obj)|*.obj",
                                    wxFD_OPEN);
                                    
    if(fname.Exists())
    {
        std::filesystem::path fname_str=fname.GetFullPath().ToStdString();
        
        mesh_file->ChangeValue(fname_str.generic_string());
        scaling_factor->set_value(1.0);
        mesh_scaled->SetValue(false);
        
        faces.resize(1);
        groups_start.resize(1);
        groups_end.resize(1);
        
        faces[0].up_irf=irfs[0];
        faces[0].down_irf=irfs[0];
        
        faces[0].up_mat=nullptr;
        faces[0].down_mat=nullptr;
        
        obj_file_load(fname_str,mesh_V_arr,mesh_F_arr);
        
        groups_start[0]=0;
        groups_end[0]=mesh_F_arr.size()-1;
        
        groups_list->clear();
        groups_list->add_panel<FacesGroupPanel>(faces[0],groups_start[0],groups_end[0],materials,materials_str,irfs,irfs_str);
        
        groups_panel->Layout();
        groups_panel->FitInside();
        
        evt_geometry(event);
    }
}

void MeshDialog::evt_scale(wxCommandEvent &event)
{
    double S=scaling_factor->get_value();
    
    mesh_scaled->SetValue(true);
    
    for(std::size_t i=0;i<mesh_V_arr.size();i++)
    {
        mesh_V_arr[i].loc*=S;
    }
    
    evt_geometry(event);
}

double MeshDialog::mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
{
    double r_max=1.0;
    
    if(mesh_F_arr.size()==0)
    {
        V_arr.resize(2);
        V_arr[0].loc=Vector3(0);
        V_arr[1].loc=Vector3(0);
        
        F_arr.resize(1);
        F_arr[0].V1=0;
        F_arr[0].V2=1;
    }
    else
    {
        r_max=0;
        
        V_arr.resize(mesh_V_arr.size());
        F_arr.resize(mesh_F_arr.size());
        
        for(unsigned int i=0;i<V_arr.size();i++)
        {
            V_arr[i].loc=mesh_V_arr[i].loc;
            V_arr[i].norm=mesh_V_arr[i].norm;
            
            r_max=var_max(r_max,
                          std::abs(V_arr[i].loc.x),
                          std::abs(V_arr[i].loc.y),
                          std::abs(V_arr[i].loc.z));
        }
        
        for(unsigned int i=0;i<F_arr.size();i++)
        {
            F_arr[i].V1=mesh_F_arr[i].V1;
            F_arr[i].V2=mesh_F_arr[i].V2;
            F_arr[i].V3=mesh_F_arr[i].V3;
        }
    }
    
    update_dimensions();
    
    return r_max;
}

void MeshDialog::save_object_geometry()
{
    object->mesh.set_mesh_path(mesh_file->GetValue().ToStdString());
    object->mesh.set_scaling_factor(scaling_factor->get_value());
    object->mesh.set_scaling_status(mesh_scaled->GetValue());
    
    object->mesh.set_mesh(mesh_V_arr, mesh_F_arr);
    
    update_faces();
    
    object->mesh.set_faces_groups(faces, groups_start, groups_end);
    object->mesh.propagate_faces_groups();
    
    object->set_mesh();
}

void MeshDialog::update_dimensions()
{
    if(mesh_V_arr.size()>0)
    {
        double x_min=mesh_V_arr[0].loc.x;
        double x_max=mesh_V_arr[0].loc.x;
        
        double y_min=mesh_V_arr[0].loc.y;
        double y_max=mesh_V_arr[0].loc.y;
        
        double z_min=mesh_V_arr[0].loc.z;
        double z_max=mesh_V_arr[0].loc.z;
        
        for(unsigned int i=0;i<mesh_V_arr.size();i++)
        {
            x_min=std::min(x_min,mesh_V_arr[i].loc.x);
            y_min=std::min(y_min,mesh_V_arr[i].loc.y);
            z_min=std::min(z_min,mesh_V_arr[i].loc.z);
            
            x_max=std::max(x_max,mesh_V_arr[i].loc.x);
            y_max=std::max(y_max,mesh_V_arr[i].loc.y);
            z_max=std::max(z_max,mesh_V_arr[i].loc.z);
        }
        
        dim_x->set_value(add_unit(x_max-x_min));
        dim_y->set_value(add_unit(y_max-y_min));
        dim_z->set_value(add_unit(z_max-z_min));
    }
    else
    {
        dim_x->set_value(add_unit(0));
        dim_y->set_value(add_unit(0));
        dim_z->set_value(add_unit(0));
    }
}

void MeshDialog::update_faces()
{
    std::size_t N=groups_list->get_size();
    
    chk_var(N);
    
    faces.resize(N);
    groups_start.resize(N);
    groups_end.resize(N);
    
    for(unsigned int i=0;i<N;i++)
    {
        FacesGroupPanel *panel=groups_list->get_panel(i);
        
        groups_start[i]=panel->group_start->get_value();
        groups_end[i]=panel->group_end->get_value();
        
        // Material
        
        if(materials.size()>0)
        {
            faces[i].up_mat=materials[panel->up_mat->GetSelection()];
            faces[i].down_mat=materials[panel->down_mat->GetSelection()];
        }
        else
        {
            faces[i].up_mat=nullptr;
            faces[i].down_mat=nullptr;
        }
        
        // Irf
        
        faces[i].up_irf=irfs[panel->up_irf->GetSelection()];
        faces[i].down_irf=irfs[panel->down_irf->GetSelection()];
    }
}

}
