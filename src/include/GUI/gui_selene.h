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

#ifndef GUI_SELENE_H_INCLUDED
#define GUI_SELENE_H_INCLUDED

#include <gui.h>
#include <gui_material.h>
#include <gui_multilayers_utils.h>
#include <gui_selene_gl.h>
#include <lua_base.h>
#include <phys_tools.h>
#include <selene.h>

#include <wx/splitter.h>
#include <wx/treectrl.h>

namespace SelGUI
{

//##############
//   Elements
//##############

class FrameDialog: public wxDialog
{
    public:
        bool cancel_check;
        OptimEngine &optim_engine;
        
        Sel::Frame *frame;
        std::vector<Sel::Frame*> frames;
        
        wxScrolledWindow *ctrl_panel;
        wxBoxSizer *ctrl_sizer;
        wxNotebook *panels_book;
        
        NamedTextCtrl<std::string> *name_ctrl;
        LengthSelector *x_ctrl,*y_ctrl,*z_ctrl;
        NamedTextCtrl<double> *a_ctrl,*b_ctrl,*c_ctrl;
        
        wxChoice *origin,
                 *relative_origin,*relative_anchor,
                 *translation_frame,*rotation_frame;
        
        FrameDialog(Sel::Frame *frame,
                    std::vector<Sel::Frame*> const &frames,
                    OptimEngine &optim_engine);
        
        void evt_cancel(wxCommandEvent &event);
        void evt_close(wxCloseEvent &event);
        void evt_ok(wxCommandEvent &event);
        void evt_relative_origin(wxCommandEvent &event);
        
        void refresh_relative_anchors();
        virtual void save_object() {}
};

//##################
//   ObjectDialog
//##################

class ObjectDialog: public FrameDialog
{
    public:
        Sel::Object *object;
        std::vector<GUI::Material*> const &materials;
        std::vector<Sel::IRF*> const &irfs;
        
        wxArrayString materials_str;
        wxArrayString irfs_str;
        
//        LengthSelector *prism_length,*prism_height,*prism_width; // Prism
//        NamedTextCtrl<double> *prism_a1,*prism_a2;
        
        GL_Selene_Minimal *gl;
        
        std::vector<Sel::SelFace> faces;
        std::vector<wxChoice*> up_mat,down_mat,up_irf,down_irf;
        
        std::vector<wxChoice*> tangent_up,tangent_down;
        std::vector<NamedTextCtrl<double>*> tangent_up_x,tangent_up_y,tangent_up_z,
                                            tangent_down_x,tangent_down_y,tangent_down_z;
        
        wxRadioBox *sensor_type;
        wxCheckBox *ray_wavelength,
                   *ray_source,
                   *ray_generation,
                   *ray_path,
                   *ray_length,
                   *ray_phase,
                   *ray_world_intersection,
                   *ray_world_direction,
                   *ray_world_polarization,
                   *ray_obj_intersection,
                   *ray_obj_direction,
                   *ray_obj_polarization,
                   *ray_obj_face;
        
        wxTimer *gl_check;
        
        ObjectDialog(Sel::Object *object,
                     std::vector<Sel::Frame*> const &frames,
                     std::vector<GUI::Material*> const &materials,
                     std::vector<Sel::IRF*> const &irfs,
                     OptimEngine &optim_engine);
        void ObjectDialogSensor();
        
        void evt_geometry(wxCommandEvent &event);
        void evt_gl_check(wxTimerEvent &event);
        void evt_IRF(wxCommandEvent &event);
        void evt_in_IRF(wxCommandEvent &event);
        void evt_in_material(wxCommandEvent &event);
        void evt_out_IRF(wxCommandEvent &event);
        void evt_out_material(wxCommandEvent &event);
        void evt_sensor_type(wxCommandEvent &event);
        virtual double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void refresh_mesh();
        void save_object();
        virtual void save_object_geometry() {}
};

class BoxDialog: public ObjectDialog
{
    public:
        LengthSelector *box_lx,*box_ly,*box_lz;
        
        BoxDialog(Sel::Object *data,
                  std::vector<Sel::Frame*> const &frames,
                  std::vector<GUI::Material*> const &materials,
                  std::vector<Sel::IRF*> const &irfs,
                  OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class ConicSectionDialog: public ObjectDialog
{
    public:
        LengthSelector *conic_R,*conic_in_radius,*conic_out_radius;
        NamedTextCtrl<double> *conic_K;
        
        ConicSectionDialog(Sel::Object *data,
                           std::vector<Sel::Frame*> const &frames,
                           std::vector<GUI::Material*> const &materials,
                           std::vector<Sel::IRF*> const &irfs,
                           OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class CylinderDialog: public ObjectDialog
{
    public:
        LengthSelector *cyl_r,*cyl_l; // Cylinder
        NamedTextCtrl<double> *cyl_cut;
        
        CylinderDialog(Sel::Object *data,
                       std::vector<Sel::Frame*> const &frames,
                       std::vector<GUI::Material*> const &materials,
                       std::vector<Sel::IRF*> const &irfs,
                       OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class DiskDialog: public ObjectDialog
{
    public:
        LengthSelector *dsk_r,*dsk_r_in;
        
        DiskDialog(Sel::Object *data,
                   std::vector<Sel::Frame*> const &frames,
                   std::vector<GUI::Material*> const &materials,
                   std::vector<Sel::IRF*> const &irfs,
                   OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class LensDialog: public ObjectDialog
{
    public:
        LengthSelector *ls_thickness,*ls_r1,*ls_r2,*ls_r_max;
        
        LensDialog(Sel::Object *data,
                   std::vector<Sel::Frame*> const &frames,
                   std::vector<GUI::Material*> const &materials,
                   std::vector<Sel::IRF*> const &irfs,
                   OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class FacesGroupPanel: public PanelsListBase
{
    public:
        NamedTextCtrl<int> *group_start,*group_end;
        wxChoice *up_mat,*down_mat,
                 *up_irf,*down_irf;
        
        wxChoice *tangent_up,
                 *tangent_down;
        NamedTextCtrl<double> *tangent_up_x,*tangent_up_y,*tangent_up_z,
                              *tangent_down_x,*tangent_down_y,*tangent_down_z;
        
        FacesGroupPanel(wxWindow *parent,
                        Sel::SelFace const &face,int group_start,int group_end,
                        std::vector<GUI::Material*> const &materials,wxArrayString const &materials_str,
                        std::vector<Sel::IRF*> const &irfs,wxArrayString const &irfs_str);
        
};

class MeshDialog: public ObjectDialog
{
    public:
        
        std::vector<Sel::Vertex> mesh_V_arr;
        std::vector<Sel::SelFace> mesh_F_arr;
        
        wxTextCtrl *mesh_file;
        NamedTextCtrl<double> *scaling_factor;
        wxCheckBox *mesh_scaled;
        
        wxScrolledWindow *groups_panel;
        PanelsList<FacesGroupPanel> *groups_list;
        std::vector<int> groups_start,groups_end;
        
        NamedTextCtrl<std::string> *dim_x,*dim_y,*dim_z;
        
        MeshDialog(Sel::Object *data,
                   std::vector<Sel::Frame*> const &frames,
                   std::vector<GUI::Material*> const &materials,
                   std::vector<Sel::IRF*> const &irfs,
                   OptimEngine &optim_engine);
        
        void evt_add_group(wxCommandEvent &event);
        void evt_groups_reorder(wxCommandEvent &event);
        void evt_IRF(wxCommandEvent &event);
        void evt_in_IRF(wxCommandEvent &event);
        void evt_in_material(wxCommandEvent &event);
        void evt_out_IRF(wxCommandEvent &event);
        void evt_out_material(wxCommandEvent &event);
        void evt_mesh_load(wxCommandEvent &event);
        void evt_scale(wxCommandEvent &event);
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
        void update_dimensions();
        void update_faces();
};

class ParabolaDialog: public ObjectDialog
{
    public:
        LengthSelector *pr_focal,*pr_in_radius,*pr_height;
        
        ParabolaDialog(Sel::Object *data,
                       std::vector<Sel::Frame*> const &frames,
                       std::vector<GUI::Material*> const &materials,
                       std::vector<Sel::IRF*> const &irfs,
                       OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class RectangleDialog: public ObjectDialog
{
    public:
        LengthSelector *box_ly,*box_lz;
        
        RectangleDialog(Sel::Object *data,
                        std::vector<Sel::Frame*> const &frames,
                        std::vector<GUI::Material*> const &materials,
                        std::vector<Sel::IRF*> const &irfs,
                        OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class SphereDialog: public ObjectDialog
{
    public:
        LengthSelector *sph_r;
        NamedTextCtrl<double> *sph_cut;
        
        SphereDialog(Sel::Object *data,
                     std::vector<Sel::Frame*> const &frames,
                     std::vector<GUI::Material*> const &materials,
                     std::vector<Sel::IRF*> const &irfs,
                     OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

class SpherePatchDialog: public ObjectDialog
{
    public:
        LengthSelector *sph_r;
        NamedTextCtrl<double> *sph_cut;
        
        SpherePatchDialog(Sel::Object *data,
                          std::vector<Sel::Frame*> const &frames,
                          std::vector<GUI::Material*> const &materials,
                          std::vector<Sel::IRF*> const &irfs,
                          OptimEngine &optim_engine);
        
        double mesh(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr);
        void save_object_geometry();
};

//#############
//   Sources
//#############

class SourceDialog: public FrameDialog
{
    public:
        std::vector<GUI::Material*> &materials;
        Sel::Light *light;
        
        NamedTextCtrl<double> *power;
        wxChoice *ambient_material;
        
        // Cone
        
        NamedTextCtrl<double> *angle;
        
        // Gaussian beam
        
        NamedTextCtrl<double> *numerical_aperture;
        LengthSelector *waist_distance;
        
        // Polarization
        
        wxChoice *polarization;
        NamedTextCtrl<double> *polar_x,*polar_y,*polar_z;
        
        // Extent
        
        wxPanel *extent_panel;
        wxChoice *extent;
        LengthSelector *extent_x,*extent_y,*extent_z,*extent_d;
        NamedTextCtrl<double> *opening;
        
        wxScrolledWindow *spectral_panel;
        
        wxRadioBox *spectrum_type;
        
        // Monochromatic
        
        WavelengthSelector *mono_lambda;
        
        // Polymonochromatic
        
        wxPanel *polymono_panel;
        PanelsList<> *polymono_list;
        
        // Polychromatic
        
        wxPanel *polychromatic_panel;
        wxChoice  *spectrum_shape;
        WavelengthSelector *lambda_min,*lambda_max;
        NamedTextCtrl<double> *temperature;
        wxPanel *file_panel;
        wxTextCtrl *file_ctrl;
        std::vector<double> file_x,file_y;
        
        // Display
        
        Graph *sp_graph;
        std::vector<double> spectrum_x,spectrum_y;
        
        SourceDialog(Sel::Light *light,
                     std::vector<Sel::Frame*> const &frames,
                     std::vector<GUI::Material*> &materials,
                     OptimEngine &optim_engine);
        
        void evt_add_polymono(wxCommandEvent &event);
        void evt_delete_polymono(wxCommandEvent &event);
        void evt_extent(wxCommandEvent &event);
        void evt_graph(wxCommandEvent &event);
        void evt_polarization_type(wxCommandEvent &event);
        void evt_polychromatic_file(wxCommandEvent &event);
        void evt_polychromatic_type(wxCommandEvent &event);
        void evt_spectrum_type(wxCommandEvent &event);
        
        void layout_extent();
        void layout_polarization();
        
        void layout_monochromatic();
        void layout_poly_monochromatic();
        void layout_polychromatic();
        void layout_polychromatic_parameters();
        
        void layout_cone();
        void layout_gaussian_beam();
        void layout_lambertian();
        void layout_perfect_beam();
        void layout_point();
        void layout_point_planar();
        
        void load_spectrum_file();
        
        void save_object();
        void update_graph();
};

//###############
//   Materials
//###############

class MaterialPanel: public PanelsListBase
{
    public:
        GUI::Material *original_material;
        
        MaterialSelector *selector;
        
        MaterialPanel(wxWindow *parent,
                      GUI::Material *material);
};

class MaterialsDialog: public wxDialog
{
    public:
        std::vector<GUI::Material*> &materials;

        wxScrolledWindow *panel;
        PanelsList<MaterialPanel> *materials_panels;
        
        MaterialsDialog(std::vector<GUI::Material*> &materials);
        
        void evt_add_material(wxCommandEvent &event);
        void evt_close(wxCloseEvent &event);
        void evt_list(wxCommandEvent &event);
};

//#########
//   IRF
//#########

class IRF_Panel: public PanelsListBase
{
    public:
        Sel::IRF *original_irf;
        
        NamedTextCtrl<std::string> *name;
        wxChoice *type;
        
        // Grating
        
        wxPanel *grating_panel;
        
        wxTextCtrl *grating_ref_file;
        wxTextCtrl *grating_tra_file;
        
        // Multilayer
        
        wxPanel *multilayer_panel;
        PanelsList<LayerPanel> *layers;
        
        // Splitter
        
        NamedTextCtrl<double> *splitter;
        
        //
        
        IRF_Panel(wxWindow *parent,Sel::IRF const &irf);
        IRF_Panel(wxWindow *parent,Sel::IRF *irf);
        
        void evt_add_layer(wxCommandEvent &event);
        void evt_grating_file(wxCommandEvent &event);
        void evt_irf_type(wxCommandEvent &event);
        
        void layout();
};

class IRF_Dialog: public wxDialog
{
    public:
        std::vector<Sel::IRF*> &irfs;
        
        wxScrolledWindow *panel;
        PanelsList<IRF_Panel> *irfs_panels;
        
        IRF_Dialog(std::vector<Sel::IRF*> &irfs);
        
        void evt_add_irf(wxCommandEvent &event);
        void evt_list(wxCommandEvent &event);
        void evt_ok(wxCloseEvent &event);
};

//##################
//   Optimization
//##################

enum class OptimTreatment
{
    MINIMIZE_SPATIAL_SPREAD,
    MINIMIZE_DIRECTION_SPREAD
};

class OptimTarget
{
    public:
        Sel::Object *sensor;
        OptimTreatment treatment;
        double weight;
};

class OptimTargetPanel: public PanelsListBase
{
    public:
        wxChoice *sensors;
        wxChoice *treatment;
        NamedTextCtrl<double> *weight;
        
        OptimTargetPanel(wxWindow *parent,std::vector<std::string> const &sensor_names);
};

class OptimizationDialog: public wxDialog
{
    public:
        std::vector<OptimTarget> &targets;
        std::vector<Sel::Object*> const &sensors;
        std::vector<std::string> sensor_names;
        
        PanelsList<OptimTargetPanel> *targets_ctrl;
        
        OptimizationDialog(std::vector<OptimTarget> &targets,
                           std::vector<Sel::Object*> const &sensors);
        
        void evt_add_target(wxCommandEvent &event);
        void evt_close(wxCloseEvent &event);
        void evt_delete_target(wxCommandEvent &event);
};

//################
//   Main Frame
//################

class SeleneFrame: public BaseFrame
{
    public:
        int item_count;
        int focus_ID;
        
        wxScrolledWindow *ctrl_panel;
        
        // Display
        
        wxChoice *ray_disp_type;
        NamedTextCtrl<int> *gen_min,*gen_max;
        WavelengthSelector *lambda_min,*lambda_max;
        LengthSelector *lost_length;
        
        NamedTextCtrl<int> *nr_disp,*nr_tot;
        
        std::filesystem::path output_directory_std;
        wxTextCtrl *output_directory;
        
        GL_Selene *gl;
        
        wxChoice *add_element_list;
        
        wxTreeItemId root_ID;
        
        wxMenu default_menu,
               object_menu,
               frame_menu,
               root_menu,
               source_menu;
        
        wxTreeCtrl *objects_tree;
        wxImageList *tree_icons;
        
        std::vector<Sel::Frame*> frames;
        std::vector<bool> render_element;
        std::vector<wxTreeItemId> frames_ID;
        std::vector<SeleneVAO*> frames_vao;
        
        std::vector<GUI::Material*> materials;
        
        std::vector<Sel::IRF> irfs;
        std::vector<Sel::IRF*> user_irfs;
        
        // Optimization
        
        OptimEngine optim_engine;
        bool optimize;
        std::vector<OptimTarget> optimization_targets;
        
        SeleneFrame(wxString const &title);
        void SeleneFrame_RayDisp(wxWindow *parent,wxBoxSizer *ctrl_sizer);
        
        void check_objects_irfs();
        void check_objects_materials();
        void clear_state();
        void delete_element(Sel::Frame *element);
        void delete_irf(Sel::IRF *irf);
        void evt_add_element(wxCommandEvent &event);
        void evt_forget_object(wxCommandEvent &event);
        void evt_generation_display(wxCommandEvent &event);
        void evt_generation_display_auto(wxCommandEvent &event);
        void evt_lost_length(wxCommandEvent &event);
        void evt_menu(wxCommandEvent &event);
        void evt_object_menu(wxTreeEvent &event);
        void evt_output_directory(wxCommandEvent &event);
        void evt_popup_menu(wxCommandEvent &event);
        void evt_ray_display_type(wxCommandEvent &event);
        void evt_trace(wxCommandEvent &event);
        void evt_tree_right_click(wxMouseEvent &event);
        void gather_materials();
        std::string get_IRF_script_name(Sel::IRF *irf);
        void load_project(wxFileName const &fname);
        void rebuild_tree();
        void save_project(wxFileName const &fname);
        void update_vao(SeleneVAO *vao,Sel::Frame *frame);
};

// Lua bindings

int lua_allocate_SeleneFrame_pointer(lua_State *L);
int lua_allocate_selene_object(lua_State *L);
int lua_allocate_selene_IRF(lua_State *L);
int lua_allocate_selene_light(lua_State *L);

int lua_selene_add_light(lua_State *L);
int lua_selene_add_object(lua_State *L);
int lua_selene_set_N_rays_disp(lua_State *L);
int lua_selene_set_N_rays_total(lua_State *L);
int lua_selene_output_directory(lua_State *L);
int lua_selene_optimization_engine(lua_State *L);

}

#endif // GUI_SELENE_H_INCLUDED
