/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <filehdl.h>
#include <logger.h>
#include <lua_sensors.h>
#include <lua_sources.h>
#include <lua_structure.h>
#include <string_tools.h>

#include <gui_fdtd.h>

#include <wx/splitter.h>

//################
//   FDTD_Frame
//################

enum
{
    FDTD_MENU_EDIT,
    HELP_MENU_ABOUT,
    HELP_MENU_BASE_WINDOW,
    PROJECT_MENU_EXIT,
    PROJECT_MENU_LOAD,
    PROJECT_MENU_NEW,
    PROJECT_MENU_RESTORE,
    PROJECT_MENU_SAVE,
    PROJECT_MENU_SAVE_AS,
    OPTIONS_MENU_VIEW,
    SOURCES_ROOT_MENU_ADD,
    SOURCES_MENU_EDIT,
    SOURCES_MENU_DELETE,
    SENSORS_ROOT_MENU_ADD,
    SENSORS_MENU_EDIT,
    SENSORS_MENU_DELETE
};

FDTD_Frame::FDTD_Frame(wxString const &title)
    :BaseFrame(title),
     Nx(60), Ny(60), Nz(60),
     Dx(5e-9), Dy(5e-9), Dz(5e-9),
     matsgrid(Nx,Ny,Nz,0),
     disp_options_dialog(nullptr)
{
    fdtd_parameters.type=FDTD_Mode::FDTD_NORMAL;
    fdtd_parameters.set_prefix("fdtd_");
    fdtd_parameters.set_directory(PathManager::tmp_path);
    fdtd_parameters.structure=new Structure();
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    wxPanel *display_panel=new wxPanel(splitter);
    
    // Controls
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - FDTD Type
    
    fdtd_type=new wxChoice(ctrl_panel,wxID_ANY);
    fdtd_type->Append("Custom");
    fdtd_type->Append("Normal Incidence");
    fdtd_type->Append("Bloch Oblique Incidence");
    fdtd_type->Append("Single Particle");
    fdtd_type->SetSelection(1);
    
    fdtd_type->Bind(wxEVT_CHOICE,&FDTD_Frame::evt_type,this);
    
    wxStaticBoxSizer *fdtd_type_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"FDTD Type");
    fdtd_type_sizer->Add(fdtd_type,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(fdtd_type_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    // - Tree
    
    wxStaticBoxSizer *tree_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Configuration");
    
    tree=new wxTreeCtrl(ctrl_panel,wxID_ANY);
    tree->SetMinSize(wxSize(1,500));
    tree_sizer->Add(tree,wxSizerFlags(1).Expand());
    
    // - Menus
    
    wxMenuBar *menu_bar=new wxMenuBar();
    
    wxMenu *project_menu=new wxMenu();
//    project_menu->Append(PROJECT_MENU_NEW,"New");
//    project_menu->AppendSeparator();
    project_menu->Append(PROJECT_MENU_LOAD,"Load");
//    project_menu->Append(PROJECT_MENU_SAVE,"Save");
    project_menu->Append(PROJECT_MENU_SAVE_AS,"Save As...");
    project_menu->AppendSeparator();
//    project_menu->Append(PROJECT_MENU_RESTORE,"Restore last session");
//    project_menu->AppendSeparator();
    project_menu->Append(PROJECT_MENU_EXIT,"Exit");
    
    wxMenu *options_menu=new wxMenu();
    options_menu->Append(OPTIONS_MENU_VIEW,"View");
    
    menu_bar->Append(project_menu,"Project");
    menu_bar->Append(options_menu,"Options");
    
    wxMenu *help_menu=new wxMenu;
    help_menu->Append(HELP_MENU_ABOUT,"About");
    help_menu->AppendSeparator();
    help_menu->Append(HELP_MENU_BASE_WINDOW,"Show main window");
    
    menu_bar->Append(help_menu,"?");
    
    menu_bar->Bind(wxEVT_MENU,&FDTD_Frame::evt_menu,this);
    
    SetMenuBar(menu_bar);
    
    fdtd_menu.Append(FDTD_MENU_EDIT,"Edit");
    
    sources_root_menu.Append(SOURCES_ROOT_MENU_ADD,"Add Source");
    sources_menu.Append(SOURCES_MENU_EDIT,"Edit");
    sources_menu.AppendSeparator();
    sources_menu.Append(SOURCES_MENU_DELETE,"Delete");
    sources_restricted_menu.Append(SOURCES_MENU_EDIT,"Edit");
    
    sensors_root_menu.Append(SENSORS_ROOT_MENU_ADD,"Add Sensor");
    sensors_menu.Append(SENSORS_MENU_EDIT,"Edit");
    sensors_menu.AppendSeparator();
    sensors_menu.Append(SENSORS_MENU_DELETE,"Delete");
    sensors_restricted_menu.Append(SENSORS_MENU_EDIT,"Edit");
    
    tree->Bind(wxEVT_TREE_ITEM_MENU,&FDTD_Frame::evt_tree,this);
    Bind(wxEVT_MENU,&FDTD_Frame::evt_popup_menu,this);
    
    ctrl_sizer->Add(tree_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    
    // - Run
    
    wxButton *run_btn=new wxButton(ctrl_panel,wxID_ANY,"Run");
    run_btn->Bind(wxEVT_BUTTON,&FDTD_Frame::evt_run,this);
    ctrl_sizer->Add(run_btn,wxSizerFlags().Expand());
    
    // - Wrapping Up
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    ctrl_panel->SetScrollbars(1,1,50,50);
    
    // Display
    
    gl=new FDTD_GL(display_panel);
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    display_sizer->Add(gl,wxSizerFlags(1).Expand());
    display_panel->SetSizer(display_sizer);
    
    // Wrapping-up
    
    splitter->SplitVertically(ctrl_panel,display_panel,300);
    splitter->SetMinimumPaneSize(250);
    
    reconstruct_tree();
}

FDTD_Frame::~FDTD_Frame()
{
//    wxFileName fname("last_session");
//    save(fname);
}

void FDTD_Frame::append_tree_pml(wxTreeItemId const &ID,std::string const &name,
                                 int N,double kappa,double sigma,double alpha,int pad)
{
    wxString str_pml,str_kappa,str_sigma,str_alpha,str_pad;
            
    wxTreeItemId pml_ID=tree->AppendItem(ID,wxString(name));
            
    str_pml<<"N: "<<N;
    str_kappa<<"kappa: "<<kappa;
    str_sigma<<"sigma: "<<sigma;
    str_alpha<<"alpha: "<<alpha;
    str_pad<<"padding: "<<pad;
    
    tree->AppendItem(pml_ID,str_pml);
    tree->AppendItem(pml_ID,str_kappa);
    tree->AppendItem(pml_ID,str_sigma);
    tree->AppendItem(pml_ID,str_alpha);
    tree->AppendItem(pml_ID,str_pad);
}

void FDTD_Frame::append_tree_sensor(wxTreeItemId const &ID,Sensor_generator &generator)
{
    wxString sensor_type,tmp_str;
    wxString tmp_str_x,tmp_str_y,tmp_str_z;
    wxTreeItemId tmp_ID;
    
         if(generator.type==Sensor_type::BOX_POYNTING) sensor_type="Poynting Box";
    else if(generator.type==Sensor_type::BOX_SPECTRAL_POYNTING) sensor_type="Spectral Poynting Box";
    else if(generator.type==Sensor_type::DIFF_ORDERS) sensor_type="Diffraction orders";
    else if(generator.type==Sensor_type::FIELDBLOCK) sensor_type="FieldBlock";
    else if(generator.type==Sensor_type::FIELDMAP) sensor_type="FieldMap";
    else if(generator.type==Sensor_type::FIELDPOINT) sensor_type="FieldPoint";
    else if(generator.type==Sensor_type::PLANAR_SPECTRAL_POYNTING) sensor_type="Spectral Poynting Plane";
    
    wxTreeItemId sensor_ID=tree->AppendItem(ID,sensor_type);
    
    wxString name("Name: ");
    name<<generator.name;
    
    tree->AppendItem(sensor_ID,name);
    
    tmp_ID=tree->AppendItem(sensor_ID,"Location");
    
    tmp_str_x="X: from ";
    tmp_str_y="Y: from ";
    tmp_str_z="Z: from ";
    
    tmp_str_x<<generator.x1r<<" to "<<generator.x2r;
    tmp_str_y<<generator.y1r<<" to "<<generator.y2r;
    tmp_str_z<<generator.z1r<<" to "<<generator.z2r;
    
    tree->AppendItem(tmp_ID,tmp_str_x);
    tree->AppendItem(tmp_ID,tmp_str_y);
    tree->AppendItem(tmp_ID,tmp_str_z);
    
    if(generator.type==Sensor_type::BOX_SPECTRAL_POYNTING ||
       generator.type==Sensor_type::DIFF_ORDERS ||
       generator.type==Sensor_type::PLANAR_SPECTRAL_POYNTING )
    {
        tmp_ID=tree->AppendItem(sensor_ID,"Spectrum");
        
        tmp_str="Lmin: "; tmp_str<<add_unit_u(generator.lambda_min);
        tree->AppendItem(tmp_ID,tmp_str);
        
        tmp_str="Lmax: "; tmp_str<<add_unit_u(generator.lambda_max);
        tree->AppendItem(tmp_ID,tmp_str);
        
        tmp_str="Analysis: "; tmp_str<<generator.Nl<<" points";
        tree->AppendItem(tmp_ID,tmp_str);
    }
    else
    {
        tmp_str="Wavelength: "; tmp_str<<add_unit_u(generator.lambda_min);
        tree->AppendItem(sensor_ID,tmp_str);
    }
    
    if(generator.type==Sensor_type::FIELDMAP ||
       generator.type==Sensor_type::DIFF_ORDERS ||
       generator.type==Sensor_type::PLANAR_SPECTRAL_POYNTING)
    {
        tmp_str="Orientation: ";
        
             if(generator.orientation==NORMAL_X) tmp_str<<"+X";
        else if(generator.orientation==NORMAL_XM) tmp_str<<"-X";
        else if(generator.orientation==NORMAL_Y) tmp_str<<"+Y";
        else if(generator.orientation==NORMAL_YM) tmp_str<<"-Y";
        else if(generator.orientation==NORMAL_Z) tmp_str<<"+Z";
        else if(generator.orientation==NORMAL_ZM) tmp_str<<"-Z";
        
        tree->AppendItem(sensor_ID,tmp_str);
    }
    
    sensors_ID.push_back(sensor_ID);
}

void FDTD_Frame::append_tree_source(wxTreeItemId const &ID,Source_generator &generator)
{
    wxString source_type,tmp_str;
    wxString tmp_str_x,tmp_str_y,tmp_str_z;
    wxTreeItemId tmp_ID;
    
         if(generator.type==Source_generator::SOURCE_GEN_OSCILLATOR) source_type="Oscillator";
    else source_type="Unknown";
    
    wxTreeItemId source_ID=tree->AppendItem(ID,source_type);
    
    tmp_ID=tree->AppendItem(source_ID,"Location");
    
    if(generator.type==Source_generator::SOURCE_GEN_OSCILLATOR)
    {
        tmp_str_x="X: "; tmp_str_y="Y: "; tmp_str_z="Z: ";
        tmp_str_x<<generator.x1r; tmp_str_y<<generator.y1r; tmp_str_z<<generator.z1r;
    }
    else
    {
        tmp_str_x="X: from ";
        tmp_str_y="Y: from ";
        tmp_str_z="Z: from ";
        
        tmp_str_x<<generator.x1r<<" to "<<generator.x2r;
        tmp_str_y<<generator.y1r<<" to "<<generator.y2r;
        tmp_str_z<<generator.z1r<<" to "<<generator.z2r;
    }
    
    tree->AppendItem(tmp_ID,tmp_str_x);
    tree->AppendItem(tmp_ID,tmp_str_y);
    tree->AppendItem(tmp_ID,tmp_str_z);
    
    tmp_ID=tree->AppendItem(source_ID,"Spectrum");
    
    tmp_str="Lmin: "; tmp_str<<add_unit_u(generator.lambda_min);
    tree->AppendItem(tmp_ID,tmp_str);
    
    tmp_str="Lmax: "; tmp_str<<add_unit_u(generator.lambda_max);
    tree->AppendItem(tmp_ID,tmp_str);
    
    tmp_str="Orientation: ";
    
         if(generator.orientation==NORMAL_X) tmp_str<<"+X";
    else if(generator.orientation==NORMAL_XM) tmp_str<<"-X";
    else if(generator.orientation==NORMAL_Y) tmp_str<<"+Y";
    else if(generator.orientation==NORMAL_YM) tmp_str<<"-Y";
    else if(generator.orientation==NORMAL_Z) tmp_str<<"+Z";
    else if(generator.orientation==NORMAL_ZM) tmp_str<<"-Z";
    
    tree->AppendItem(source_ID,tmp_str);
    
    sources_ID.push_back(source_ID);
}

void FDTD_Frame::evt_menu(wxCommandEvent &event)
{
    int menu_ID=event.GetId();
    
         if(menu_ID==HELP_MENU_ABOUT) ;
    else if(menu_ID==HELP_MENU_BASE_WINDOW) wxGetApp().restore_main_frame();
    else if(menu_ID==PROJECT_MENU_EXIT) Close();
    else if(menu_ID==PROJECT_MENU_LOAD) subevt_menu_load();
    else if(menu_ID==PROJECT_MENU_NEW) subevt_menu_new();
    else if(menu_ID==PROJECT_MENU_RESTORE) subevt_menu_restore();
    else if(menu_ID==PROJECT_MENU_SAVE) subevt_menu_save();
    else if(menu_ID==PROJECT_MENU_SAVE_AS) subevt_menu_save_as();
    else if(menu_ID==OPTIONS_MENU_VIEW)
    {
        if(disc_vao.size()>0 && disp_options_dialog==nullptr)
        {
            disp_options_dialog=new FDTD_Disp_Dialog(this,disc_vao,gl->pml_x,gl->pml_y,gl->pml_z);
            disp_options_dialog->Bind(wxEVT_CLOSE_WINDOW,&FDTD_Frame::evt_options_dialog_close,this);
            disp_options_dialog->Show();
        }
    }
    
//    event.Skip();
}

void FDTD_Frame::evt_options_dialog_close(wxCloseEvent &event)
{
    disp_options_dialog=nullptr;
    
    event.Skip();
}

void FDTD_Frame::evt_popup_menu(wxCommandEvent &event)
{
    int menu_ID=event.GetId();
    chk_var(menu_ID);
    
    if(menu_ID==FDTD_MENU_EDIT)
    {
        int target_panel=0;
        
             if(in_family_tree(tree,structure_ID,focus_ID)) target_panel=1;
        else if(in_family_tree(tree,incidence_ID,focus_ID)) target_panel=2;
        else if(in_family_tree(tree,materials_ID,focus_ID)) target_panel=3;
        else if(in_family_tree(tree,boundaries_ID,focus_ID)) target_panel=4;
        
        FDTD_Mode_Dialog dialog(&fdtd_parameters,target_panel);
        
        if(dialog.new_structure==true)
        {
            double lx,ly,lz;
            
            fdtd_parameters.structure->finalize();
            fdtd_parameters.structure->retrieve_nominal_size(lx,ly,lz);
            fdtd_parameters.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
            
            Dx=fdtd_parameters.Dx;
            Dy=fdtd_parameters.Dy;
            Dz=fdtd_parameters.Dz;
            
            fdtd_parameters.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
            gl->update_grid(Nx,Ny,Nz,Dx,Dy,Dz);
            gl->set_structure(matsgrid,disc_vao);
            
            fdtd_parameters.finalize_thight();
            update_gl_pmls();
            gl->reset_pml_display();
        }
        else if(!fdtd_parameters.structure->get_script_path().empty())
        {
            fdtd_parameters.finalize_thight();
            update_gl_pmls();
        }
    }
    else if(menu_ID==SENSORS_ROOT_MENU_ADD)
    {
        Sensor_generator generator;
        Sensor_Gen_Dialog dialog(&generator);
        
        if(dialog.selection_ok)
        {
            fdtd_parameters.add_sensor(generator);
            gl->add_sensor(generator);
            
            fdtd_parameters.finalize_thight();
            update_gl_pmls();
        }
    }
    else if(menu_ID==SENSORS_MENU_DELETE)
    {
        for(unsigned int i=0;i<sensors_ID.size();i++)
        {
            if(in_family_tree(tree,sensors_ID[i],focus_ID))
            {
                fdtd_parameters.delete_sensor(i);
                gl->delete_sensor(i);
                
                fdtd_parameters.finalize_thight();
                update_gl_pmls();

                break;
            }
        }
    }
    else if(menu_ID==SENSORS_MENU_EDIT)
    {
        for(unsigned int i=0;i<sensors_ID.size();i++)
        {
            if(in_family_tree(tree,sensors_ID[i],focus_ID))
            {
                Sensor_Gen_Dialog dialog(&fdtd_parameters.sensors[i]);
                gl->update_sensor(i,fdtd_parameters.sensors[i]);
                
                fdtd_parameters.finalize_thight();
                update_gl_pmls();
                
                break;
            }
        }
    }
    else if(menu_ID==SOURCES_ROOT_MENU_ADD)
    {
        Source_generator generator;
        Source_Gen_Dialog dialog(&generator);
        
        if(dialog.selection_ok)
        {
            fdtd_parameters.add_source(generator);
            gl->add_source(generator);
            
            fdtd_parameters.finalize_thight();
            update_gl_pmls();
        }
    }
    else if(menu_ID==SOURCES_MENU_DELETE)
    {
        for(unsigned int i=0;i<sources_ID.size();i++)
        {
            if(in_family_tree(tree,sources_ID[i],focus_ID))
            {
                fdtd_parameters.delete_source(i);
                gl->delete_source(i);
                
                fdtd_parameters.finalize_thight();
                update_gl_pmls();
                
                break;
            }
        }
    }
    else if(menu_ID==SOURCES_MENU_EDIT)
    {
        for(unsigned int i=0;i<sources_ID.size();i++)
        {
            if(in_family_tree(tree,sources_ID[i],focus_ID))
            {
                Source_Gen_Dialog dialog(&fdtd_parameters.sources[i]);
                gl->update_source(i,fdtd_parameters.sources[i]);
                
                fdtd_parameters.finalize_thight();
                update_gl_pmls();
                
                break;
            }
        }
    }
    
    reconstruct_tree();
    
    event.Skip();
}

void FDTD_Frame::evt_run(wxCommandEvent &event)
{
    fdtd_parameters.consolidate_materials();
    
    FDTD_Run_Dialog dialog(this,fdtd_parameters);
    
    event.Skip();
}

void FDTD_Frame::evt_tree(wxTreeEvent &event)
{
    focus_ID=event.GetItem();
    
    if( in_family_tree(tree,root_ID,focus_ID) &&
       !in_family_tree(tree,sources_root_ID,focus_ID) &&
       !in_family_tree(tree,sensors_root_ID,focus_ID)) PopupMenu(&fdtd_menu);
    else if(focus_ID==sources_root_ID) PopupMenu(&sources_root_menu);
    else if(focus_ID==sensors_root_ID) PopupMenu(&sensors_root_menu);
    else
    {
        for(unsigned int i=0;i<sources_ID.size();i++)
        {
                 if(focus_ID==sources_ID[i])
            {
                PopupMenu(&sources_menu);
                break;
            }
            else if(in_family_tree(tree,sources_ID[i],focus_ID))
            {
                PopupMenu(&sources_restricted_menu);
                break;
            }
        }
        
        for(unsigned int i=0;i<sensors_ID.size();i++)
        {
                 if(focus_ID==sensors_ID[i])
            {
                PopupMenu(&sensors_menu);
                break;
            }
            else if(in_family_tree(tree,sensors_ID[i],focus_ID))
            {
                PopupMenu(&sensors_restricted_menu);
                break;
            }
        }
    }
    
    event.Skip();
}

void FDTD_Frame::evt_type(wxCommandEvent &event)
{
    int selection=event.GetSelection();
    
         if(selection==0) fdtd_parameters.type=FDTD_Mode::FDTD_CUSTOM;
    else if(selection==1) fdtd_parameters.type=FDTD_Mode::FDTD_NORMAL;
    else if(selection==2) fdtd_parameters.type=FDTD_Mode::FDTD_OBLIQUE_ARS;
    else if(selection==3) fdtd_parameters.type=FDTD_Mode::FDTD_SINGLE_PARTICLE;
    
    reconstruct_tree();
    
    event.Skip();
}

int FDTD_Frame_lua_mode(lua_State *L)
{
    std::string mode=lua_tostring(L,1);
    lua_getglobal(L,"bound_class");
    
    FDTD_Mode **pp_parameters=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,-1));
    FDTD_Mode *p_parameters=*pp_parameters;
    
    luaL_getmetatable(L,"metatable_fdtd");
    lua_setmetatable(L,-2);
    
    null_function(p_parameters);
    
    if(mode=="fdtd") p_parameters->type=FDTD_Mode::FDTD_CUSTOM;
    else if(mode=="fdtd_normal") p_parameters->type=FDTD_Mode::FDTD_NORMAL;
    else if(mode=="fdtd_oblique_ARS") p_parameters->type=FDTD_Mode::FDTD_OBLIQUE_ARS;
//        else if(mode=="fdtd_planar_guided") p_parameters->type="planar_guided";
//        else if(mode=="fdtd_planar_guided_2D_ext") p_parameters->type="planar_guided_2D_ext";
    else if(mode=="fdtd_single_particle") p_parameters->type=FDTD_Mode::FDTD_SINGLE_PARTICLE;
    
    return 1;
}

namespace GUI
{
    int fdtd_compute(lua_State *L)
    {
        // Ignoring the command in GUI mode
        
        return 0;
    }
    
    
    int fdtd_set_material(lua_State *L)
    {
        GUI::FDTD_Mode *p_mode=dynamic_cast<GUI::FDTD_Mode*>(lua_get_metapointer<::FDTD_Mode>(L,1));
        
        std::size_t N=lua_tointeger(L,2);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(lua_get_metapointer<::Material>(L,3));
        
        p_mode->g_materials.resize(std::max(p_mode->g_materials.size(),N+1));
        p_mode->g_materials[N]=mat;
        
        return 0;
    }
}

int gen_const_material(lua_State* L);
int gen_complex_material(lua_State* L);

void FDTD_Frame::load(wxFileName const &fname_)
{
    std::filesystem::path fname=fname_.GetFullPath().ToStdString();
    
    // Cleaning
    
    if(fdtd_parameters.structure!=nullptr) delete fdtd_parameters.structure;
    fdtd_parameters.reset();
    
    fdtd_parameters.type=FDTD_Mode::FDTD_NORMAL;
    fdtd_parameters.set_prefix("fdtd");
    
    gl->forget_sensors();
    gl->forget_sources();
    
    // Lua
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    // - Materials
    
    lua_gui_material::Loader loader;
    loader.create_metatable(L);
    
    // - Model
    
    FDTD_Mode *p_parameters=&fdtd_parameters;
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&p_parameters));
    lua_setglobal(L,"bound_class");
    
    std::filesystem::path caller_path=fname.parent_path();
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&caller_path));
    lua_setglobal(L,"lua_caller_path");
    
    lua_register(L,"MODE",&FDTD_Frame_lua_mode);
    lua_register(L,"complex_material",gen_complex_material);
    lua_register(L,"const_material",gen_const_material);
    lua_register(L,"create_sensor",create_sensor);
    lua_register(L,"create_source",create_source);
    lua_register(L,"nearest_integer",nearest_integer);
    
    FDTD_Mode_create_metatable(L);
    metatable_add_func(L,"compute",&GUI::fdtd_compute);       // Override of the FDTD Mode computation (nothing to do in it)
    metatable_add_func(L,"material",&GUI::fdtd_set_material); // Override of the FDTD Mode Loading
    
    Source_generator_create_metatable(L);
    Sensor_generator_create_metatable(L);
    
    lua_register(L,"Structure",LuaUI::allocate_structure);
    LuaUI::create_structure_metatable(L);
    
    int load_err = luaL_loadfile(L,fname.generic_string().c_str());
    
    if(load_err!=LUA_OK)
    {
        Plog::print("Lua syntax error with ", fname, "\n");
        std::cin.get();
        return;
    }
    
    lua_pcall(L,0,0,0);
    
    lua_close(L);
    
    // Converting materials
    
    for(unsigned int i=0;i<fdtd_parameters.materials.size();i++)
    {
        if(!fdtd_parameters.materials[i].fdtd_compatible())
            fdtd_parameters.materials[i].set_const_n(1.0);
    }
    
    // Treating the new structure
    
    double lx,ly,lz;
    
    fdtd_parameters.structure->finalize();
    fdtd_parameters.structure->retrieve_nominal_size(lx,ly,lz);
    fdtd_parameters.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    fdtd_parameters.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    fdtd_parameters.finalize_thight();
    
    gl->update_grid(Nx,Ny,Nz,Dx,Dy,Dz);
    gl->set_structure(matsgrid,disc_vao);
    
    for(unsigned int i=0;i<fdtd_parameters.sensors.size();i++)
    {
        gl->add_sensor(fdtd_parameters.sensors[i]);
    }
    
    for(unsigned int i=0;i<fdtd_parameters.sources.size();i++)
        gl->add_source(fdtd_parameters.sources[i]);
    
    update_gl_pmls();
    gl->reset_pml_display();
}

void FDTD_Frame::reconstruct_tree()
{
    bool force_xy_periodic=(fdtd_parameters.type==FDTD_Mode::FDTD_NORMAL || 
                            fdtd_parameters.type==FDTD_Mode::FDTD_OBLIQUE_ARS);
    
    wxString tmp_str;
    
    tree->DeleteAllItems();
    
    root_ID=tree->AddRoot("FDTD");
    
    if(fdtd_parameters.type!=FDTD_Mode::FDTD_CUSTOM)
    {
        tmp_str="Name: "; tmp_str<<fdtd_parameters.prefix;
        tree->AppendItem(root_ID,tmp_str);
    }
    
    {
        tmp_str="Output: "; tmp_str<<fdtd_parameters.directory().generic_string();
        tree->AppendItem(root_ID,tmp_str);
    }
    
    if(fdtd_parameters.time_type==TIME_FIXED)
    {
        tmp_str="Convergence: Fixed"; tree->AppendItem(root_ID,tmp_str);
        tmp_str="Iterations: "; tmp_str<<fdtd_parameters.Nt; tree->AppendItem(root_ID,tmp_str);
    }
    else
    {
        tmp_str="Convergence: Adaptive"; wxTreeItemId tmp_ID=tree->AppendItem(root_ID,tmp_str);
        tmp_str="Max Iterations: "; tmp_str<<fdtd_parameters.Nt; tree->AppendItem(tmp_ID,tmp_str);
        tmp_str="Lmin: "; tmp_str<<add_unit_u(fdtd_parameters.cc_lmin); tree->AppendItem(tmp_ID,tmp_str);
        tmp_str="Lmax: "; tmp_str<<add_unit_u(fdtd_parameters.cc_lmax); tree->AppendItem(tmp_ID,tmp_str);
        tmp_str="Level: "; tmp_str<<-std::log10(fdtd_parameters.cc_coeff); tree->AppendItem(tmp_ID,tmp_str);
        tmp_str="N Points: "; tmp_str<<fdtd_parameters.cc_quant; tree->AppendItem(tmp_ID,tmp_str);
        tmp_str="Layout: ";
        
        if(fdtd_parameters.cc_layout.size()==3)
        {
            for(int i=0;i<3;i++)
            {
                     if(fdtd_parameters.cc_layout[i]=='n') tmp_str<<"None";
                else if(fdtd_parameters.cc_layout[i]=='d') tmp_str<<"Down";
                else if(fdtd_parameters.cc_layout[i]=='u') tmp_str<<"Up";
                else if(fdtd_parameters.cc_layout[i]=='b') tmp_str<<"Both";
                
                if(i!=2) tmp_str<<" | ";
            }
        }
        else tmp_str<<fdtd_parameters.cc_layout;
        
        tree->AppendItem(tmp_ID,tmp_str);
    }
    
    // Structure
    
    structure_ID=tree->AppendItem(root_ID,"Structure");
    if(!fdtd_parameters.structure->get_script_path().empty())
        tree->AppendItem(structure_ID,fdtd_parameters.structure->get_script_path().generic_string());
    else tree->AppendItem(structure_ID,"None");
    tree->AppendItem(structure_ID,"Dx= "+add_unit(fdtd_parameters.Dx));
    tree->AppendItem(structure_ID,"Dy= "+add_unit(fdtd_parameters.Dy));
    tree->AppendItem(structure_ID,"Dz= "+add_unit(fdtd_parameters.Dz));
    
    if(!fdtd_parameters.structure->parameter_name.empty())
    {
        wxTreeItemId tmp=tree->AppendItem(structure_ID,"Parameters");
        
        for(std::size_t i=0;i<fdtd_parameters.structure->parameter_name.size();i++)
        {
            std::stringstream strm;
            
            strm<<fdtd_parameters.structure->parameter_name[i]<<"= ";
            strm<<fdtd_parameters.structure->parameter_value[i];
            
            tree->AppendItem(tmp,strm.str());
        }
    }
    
    // Incidence
    
    if(fdtd_parameters.type!=FDTD_Mode::FDTD_CUSTOM)
    {
        incidence_ID=tree->AppendItem(root_ID,"Incidence");
        
        tmp_str="Lmin: "; tmp_str<<add_unit_u(fdtd_parameters.lambda_min);
        tree->AppendItem(incidence_ID,tmp_str);
        
        tmp_str="Lmax: "; tmp_str<<add_unit_u(fdtd_parameters.lambda_max);
        tree->AppendItem(incidence_ID,tmp_str);
        
        tmp_str="Polarization: "; tmp_str<<fdtd_parameters.polarization;
        tree->AppendItem(incidence_ID,tmp_str);
        
        tmp_str="Analysis: "; tmp_str<<fdtd_parameters.Nl<<" points";
        tree->AppendItem(incidence_ID,tmp_str);
    }
    
    // Materials
    
    materials_ID=tree->AppendItem(root_ID,"Materials");
    
    for(unsigned int i=0;i<fdtd_parameters.g_materials.size();i++)
    {
        GUI::Material *material=fdtd_parameters.g_materials[i];
        
        wxString str;
        str<<i<<": "<<material->get_short_description();
        
        tree->AppendItem(materials_ID,str);
    }
    
    // Boundaries
    
    boundaries_ID=tree->AppendItem(root_ID,"Boundary Conditions");
    
    if(!force_xy_periodic)
    {
        if(fdtd_parameters.periodic_x) tree->AppendItem(boundaries_ID,"X Periodic");
        else
        {
            append_tree_pml(boundaries_ID,"PML -X",
                            fdtd_parameters.pml_xm,
                            fdtd_parameters.kappa_xm,
                            fdtd_parameters.sigma_xm,
                            fdtd_parameters.alpha_xm,
                            fdtd_parameters.pad_xm);
                            
            append_tree_pml(boundaries_ID,"PML +X",
                            fdtd_parameters.pml_xp,
                            fdtd_parameters.kappa_xp,
                            fdtd_parameters.sigma_xp,
                            fdtd_parameters.alpha_xp,
                            fdtd_parameters.pad_xp);
        }
        
        if(fdtd_parameters.periodic_y) tree->AppendItem(boundaries_ID,"Y Periodic");
        else
        {
            append_tree_pml(boundaries_ID,"PML -Y",
                            fdtd_parameters.pml_ym,
                            fdtd_parameters.kappa_ym,
                            fdtd_parameters.sigma_ym,
                            fdtd_parameters.alpha_ym,
                            fdtd_parameters.pad_ym);
                            
            append_tree_pml(boundaries_ID,"PML +Y",
                            fdtd_parameters.pml_yp,
                            fdtd_parameters.kappa_yp,
                            fdtd_parameters.sigma_yp,
                            fdtd_parameters.alpha_yp,
                            fdtd_parameters.pad_yp);
        }
    }
    else
    {
        tree->AppendItem(boundaries_ID,"X Periodic");
        tree->AppendItem(boundaries_ID,"Y Periodic");
    }
    
    if(fdtd_parameters.periodic_z) tree->AppendItem(boundaries_ID,"Z Periodic");
    else
    {
        append_tree_pml(boundaries_ID,"PML -Z",
                        fdtd_parameters.pml_zm,
                        fdtd_parameters.kappa_zm,
                        fdtd_parameters.sigma_zm,
                        fdtd_parameters.alpha_zm,
                        fdtd_parameters.pad_zm);
                        
        append_tree_pml(boundaries_ID,"PML +Z",
                        fdtd_parameters.pml_zp,
                        fdtd_parameters.kappa_zp,
                        fdtd_parameters.sigma_zp,
                        fdtd_parameters.alpha_zp,
                        fdtd_parameters.pad_zp);
    }
    
    // Sources
    
    if(fdtd_parameters.type==FDTD_Mode::FDTD_CUSTOM)
    {
        sources_root_ID=tree->AppendItem(root_ID,"Sources");
        
        sources_ID.clear();
        for(unsigned int i=0;i<fdtd_parameters.sources.size();i++)
            append_tree_source(sources_root_ID,fdtd_parameters.sources[i]);
    }
    
    // Sensors
    
    sensors_root_ID=tree->AppendItem(root_ID,"Sensors");
    
    sensors_ID.clear();
    for(unsigned int i=0;i<fdtd_parameters.sensors.size();i++)
    {
        append_tree_sensor(sensors_root_ID,fdtd_parameters.sensors[i]);
    }
    
    tree->ExpandAll();
}

void FDTD_Frame::refresh_type_ctrl()
{
         if(fdtd_parameters.type==FDTD_Mode::FDTD_CUSTOM) fdtd_type->SetSelection(0);
    else if(fdtd_parameters.type==FDTD_Mode::FDTD_NORMAL) fdtd_type->SetSelection(1);
    else if(fdtd_parameters.type==FDTD_Mode::FDTD_OBLIQUE_ARS) fdtd_type->SetSelection(2);
    else if(fdtd_parameters.type==FDTD_Mode::FDTD_SINGLE_PARTICLE) fdtd_type->SetSelection(3);
}

void FDTD_Frame::save(wxFileName const &fname)
{
    std::filesystem::path save_path=fname.GetFullPath().ToStdString();
    std::ofstream file(save_path,std::ios::out|std::ios::trunc|std::ios::binary);
    
    GUI::FDTD_Mode &p=fdtd_parameters;
    
    // Materials
    
    lua_gui_material::Translator mtr("");
    
    for(std::size_t i=0;i<p.g_materials.size();i++)
    {
        mtr.gather(p.g_materials[i]);
    }
    
    file<<mtr.get_header()<<"\n";
    
    // Model
    
    int type=p.type;
    
    std::filesystem::path structure_path;
    structure_path=to_relative_file(p.structure->get_script_path(),save_path);
    
    file<<"structure=Structure(\""<<structure_path.generic_string()<<"\")\n";
    for(std::size_t i=0;i<p.structure->parameter_name.size();i++)
        file<<"structure:parameter(\""<<p.structure->parameter_name[i]<<"\","<<p.structure->parameter_value[i]<<")\n";
    file<<"\n";
    
         if(type==FDTD_Mode::FDTD_CUSTOM) file<<"fdtd=MODE(\"fdtd\")\n";
    else if(type==FDTD_Mode::FDTD_NORMAL) file<<"fdtd=MODE(\"fdtd_normal\")\n";
    else if(type==FDTD_Mode::FDTD_SINGLE_PARTICLE) file<<"fdtd=MODE(\"fdtd_single_particle\")\n";
    
    file<<"fdtd:prefix(\""<<p.prefix<<"\")\n";
    
    if(!p.directory().empty())
        file<<"fdtd:output_directory(\""<<p.directory().generic_string()<<"\")\n";
    
    if(type!=FDTD_Mode::FDTD_CUSTOM)
    {
        file<<"fdtd:polarization(\""<<p.polarization<<"\")\n";
        file<<"fdtd:spectrum("<<p.lambda_min<<","<<p.lambda_max<<","<<p.Nl<<")\n";
    }
    
    if(p.time_type==TIME_FIXED) file<<"fdtd:N_tsteps("<<p.Nt<<")\n";
    else if(p.time_type==TIME_FT)
    {
        file<<"fdtd:auto_tsteps("<<p.Nt<<","<<p.cc_step<<","
                                 <<p.cc_lmin<<","<<p.cc_lmax<<","
                                 <<p.cc_coeff<<","<<p.cc_quant<<",\""<<p.cc_layout<<"\")\n";
    }
    else if(p.time_type==TIME_ENERGY)
        file<<"fdtd:auto_tsteps("<<p.Nt<<","<<p.cc_step<<","<<p.cc_coeff<<")\n";
    
    file<<"fdtd:structure(structure)\n";
    file<<"fdtd:Dx("<<p.Dx<<")\n";
    file<<"fdtd:Dy("<<p.Dy<<")\n";
    file<<"fdtd:Dz("<<p.Dz<<")\n";
    file<<"fdtd:padding(";
    if(p.periodic_x) file<<"0,0,";
    else file<<p.pad_xm<<","<<p.pad_xp<<",";
    if(p.periodic_y) file<<"0,0,";
    else file<<p.pad_ym<<","<<p.pad_yp<<",";
    if(p.periodic_z) file<<"0,0";
    else file<<p.pad_zm<<","<<p.pad_zp;
    file<<")\n";
    
    if(!p.periodic_x)
    {
        file<<"fdtd:pml_xm("<<p.pml_xm<<","<<p.kappa_xm<<","<<p.sigma_xm<<","<<p.alpha_xm<<")\n";
        file<<"fdtd:pml_xp("<<p.pml_xp<<","<<p.kappa_xp<<","<<p.sigma_xp<<","<<p.alpha_xp<<")\n";
    }
    if(!p.periodic_y)
    {
        file<<"fdtd:pml_ym("<<p.pml_ym<<","<<p.kappa_ym<<","<<p.sigma_ym<<","<<p.alpha_ym<<")\n";
        file<<"fdtd:pml_yp("<<p.pml_yp<<","<<p.kappa_yp<<","<<p.sigma_yp<<","<<p.alpha_yp<<")\n";
    }
    if(!p.periodic_z)
    {
        file<<"fdtd:pml_zm("<<p.pml_zm<<","<<p.kappa_zm<<","<<p.sigma_zm<<","<<p.alpha_zm<<")\n";
        file<<"fdtd:pml_zp("<<p.pml_zp<<","<<p.kappa_zp<<","<<p.sigma_zp<<","<<p.alpha_zp<<")\n";
    }
    
    for(std::size_t i=0;i<p.g_materials.size();i++)
        file<<"fdtd:material("<<i<<","<<mtr(p.g_materials[i])<<")\n";
    
    file<<"\n";
    for(unsigned int i=0;i<p.sensors.size();i++)
    {
        Sensor_generator &gen=p.sensors[i];
        
        std::string s_name="sensor_"+std::to_string(i);
        
        file<<s_name<<"=create_sensor(\"";
        file<<from_sensor_type(gen.type);
        file<<"\")\n";
        
        file<<s_name<<":name(\""<<gen.name<<"\")\n";
        file<<s_name<<":location("<<gen.x1r<<","<<gen.x2r<<","
                                  <<gen.y1r<<","<<gen.y2r<<","
                                  <<gen.z1r<<","<<gen.z2r<<")\n";
        file<<s_name<<":orientation(\"";
             if(gen.orientation==NORMAL_XM) file<<"-X";
        else if(gen.orientation==NORMAL_X ) file<<"X";
        else if(gen.orientation==NORMAL_YM) file<<"-Y";
        else if(gen.orientation==NORMAL_Y ) file<<"Y";
        else if(gen.orientation==NORMAL_ZM) file<<"-Z";
        else if(gen.orientation==NORMAL_Z ) file<<"Z";
        file<<"\")\n";
        
        if(gen.disable_xm) file<<s_name<<":disable(\"-X\")\n";
        if(gen.disable_xp) file<<s_name<<":disable(\"X\")\n";
        if(gen.disable_ym) file<<s_name<<":disable(\"-Y\")\n";
        if(gen.disable_yp) file<<s_name<<":disable(\"Y\")\n";
        if(gen.disable_zm) file<<s_name<<":disable(\"-Z\")\n";
        if(gen.disable_zp) file<<s_name<<":disable(\"Z\")\n";
        
        if(gen.type==Sensor_type::FIELDMAP || gen.type==Sensor_type::FIELDBLOCK)
            file<<s_name<<":wavelength("<<gen.lambda_min<<")\n";
        else file<<s_name<<":spectrum("<<gen.lambda_min<<","<<gen.lambda_max<<","<<gen.Nl<<")\n";
        file<<"\n";;
    }
    
    for(unsigned int i=0;i<p.sensors.size();i++)
        file<<"fdtd:register_sensor(sensor_"<<i<<")\n";
    
    file<<"\n";
    for(unsigned int i=0;i<p.sources.size();i++)
    {
        Source_generator &gen=p.sources[i];
        
        std::string s_name="source_"+std::to_string(i);
        
        file<<s_name<<"=create_source(\"";
        if(gen.type==Source_generator::SOURCE_GEN_OSCILLATOR) file<<"oscillator";
        file<<"\")\n";
        
        if(gen.type==Source_generator::SOURCE_GEN_OSCILLATOR)
            file<<s_name<<":location("<<gen.x1r<<","<<gen.y1r<<","<<gen.z1r<<")\n";
        file<<s_name<<":orientation(\"";
             if(gen.orientation==NORMAL_XM) file<<"-X";
        else if(gen.orientation==NORMAL_X ) file<<"X";
        else if(gen.orientation==NORMAL_YM) file<<"-Y";
        else if(gen.orientation==NORMAL_Y ) file<<"Y";
        else if(gen.orientation==NORMAL_ZM) file<<"-Z";
        else if(gen.orientation==NORMAL_Z ) file<<"Z";
        file<<"\")\n";
        
        file<<s_name<<":spectrum("<<gen.lambda_min<<","<<gen.lambda_max<<")\n";
        file<<"\n";;
    }
    
    for(unsigned int i=0;i<p.sources.size();i++)
        file<<"fdtd:register_source(source_"<<i<<")\n";
    
    file<<"\nfdtd:compute()";
}

void FDTD_Frame::subevt_menu_load()
{
    wxFileName fname=wxFileSelector("Load Project File...",
                                    wxEmptyString,wxEmptyString,
                                    "afdtd","Aether FDTD script (*.afdtd)|*.afdtd|Lua Script (*.lua)|*.lua",
                                    wxFD_OPEN);
    
    if(fname.Exists())
    {
        project_fname=fname;
        load(fname);
    }
    
    refresh_type_ctrl();
    reconstruct_tree();
}

void FDTD_Frame::subevt_menu_new()
{
    reconstruct_tree();
}

void FDTD_Frame::subevt_menu_restore()
{
    wxFileName fname("last_session");
    load(fname);
    project_fname="";
}

void FDTD_Frame::subevt_menu_save()
{
    save(project_fname);
}

void FDTD_Frame::subevt_menu_save_as()
{
    wxFileName fname=wxFileSelector("Save Project File As...",
                                    wxEmptyString,wxEmptyString,
                                    "afdtd","Aether FDTD script (*.afdtd)|*.afdtd|Lua Script (*.lua)|*.lua",
                                    wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    project_fname=fname;
    project_fname.MakeRelativeTo();
    save(project_fname);
}

void FDTD_Frame::update_gl_pmls()
{
    if(fdtd_parameters.type==FDTD_Mode::FDTD_SINGLE_PARTICLE || fdtd_parameters.type==FDTD_Mode::FDTD_CUSTOM)
    {
        gl->set_pml(fdtd_parameters.pml_xm,fdtd_parameters.pml_xp,
                    fdtd_parameters.pml_ym,fdtd_parameters.pml_yp,
                    fdtd_parameters.pml_zm,fdtd_parameters.pml_zp,
                    fdtd_parameters.pad_xm,fdtd_parameters.pad_xp,
                    fdtd_parameters.pad_ym,fdtd_parameters.pad_yp,
                    fdtd_parameters.pad_zm,fdtd_parameters.pad_zp);
    }
    else
    {
        gl->set_pml(0,0,0,0,fdtd_parameters.pml_zm,fdtd_parameters.pml_zp,
                    0,0,0,0,fdtd_parameters.pad_zm,fdtd_parameters.pad_zp);
    }
}
