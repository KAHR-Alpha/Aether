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

#include <filehdl.h>
#include <gui_fdtd_structdesign.h>
#include <gui_rsc.h>
#include <lua_base.h>

bool glew_ok=false;

extern std::ofstream plog;
extern const double Pi;

//###############
//  EMGeometry_GL
//###############

EMGeometry_GL::EMGeometry_GL(wxWindow *parent)
    :GL_3D_Base(parent),
     lx(1.0), ly(1.0), lz(1.0),scale(1.0)
{
}

void EMGeometry_GL::forget_all_vaos()
{
    for(unsigned int i=0;i<vao.size();i++) delete vao[i];
    vao.clear();
}

void EMGeometry_GL::forget_vao(std::vector<Basic_VAO*> const &vao_)
{
    for(unsigned int i=0;i<vao_.size();i++) forget_vao(vao_[i]);
}

void EMGeometry_GL::forget_vao(Basic_VAO *vao_)
{
    if(vao.size()==0) return;
    
    unsigned int i,j;
    
    for(i=0;i<vao.size();i++)
    {
        if(vao[i]==vao_)
        {
            delete vao[i];
            for(j=i+1;j<vao.size();j++) vao[j-1]=vao[j];
            vao.pop_back();
            return;
        }
    }
}

void EMGeometry_GL::init_opengl()
{
    chk_var("init start");
    focus();
    
    gxd=new Glite::LineGrid_VAO;
    gxu=new Glite::LineGrid_VAO;
    gyd=new Glite::LineGrid_VAO;
    gyu=new Glite::LineGrid_VAO;
    gzd=new Glite::LineGrid_VAO;
    gzu=new Glite::LineGrid_VAO;
    
    update_grid(lx,ly,lz);
    
    prog_grid=Glite::create_program(PathManager::locate_resource("resources/glsl/grid_vshader.glsl"),
                                    PathManager::locate_resource("resources/glsl/grid_fshader.glsl"));
    
    prog_solid=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_solid_fshader.glsl"));
    
    prog_wires=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_wires_fshader.glsl"));
    
//    glBindAttribLocation(prog_solid,3,"transf_matrix");
    
    uni_proj_grid=glGetUniformLocation(prog_grid,"proj_mat");
    chk_var("init end");
}

void EMGeometry_GL::render()
{
    glUseProgram(prog_grid);
    glUniformMatrix4fv(uni_proj_grid,1,0,camera.proj_gl);
    
    glLineWidth(3.0);
    
    if(camera.Co.x>-0.5*lx) gxd->draw();
    if(camera.Co.x<+0.5*lx) gxu->draw();
    if(camera.Co.y>-0.5*ly) gyd->draw();
    if(camera.Co.y<+0.5*ly) gyu->draw();
    if(camera.Co.z>-0.5*lz) gzd->draw();
    if(camera.Co.z<+0.5*lz) gzu->draw();
    
    glLineWidth(1.0);
    
    glUseProgram(prog_wires);
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    
    glUseProgram(prog_solid);
    
    Vector3 sun_dir(1.0,1.0,1.0);
//    sun_dir.normalize();
    sun_dir.set_spherical(1.0,Pi/4.0,-Pi/3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    glUniform4f(11,sun_dir.x,
                   sun_dir.y,
                   sun_dir.z,
                   0);
    
    for(unsigned int i=0;i<vao.size();i++) vao[i]->draw();
    
    glUseProgram(prog_wires);
    
    glLineWidth(3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    for(unsigned int i=0;i<vao.size();i++) vao[i]->draw_wireframe();
    
    glLineWidth(1.0);
}

Basic_VAO* EMGeometry_GL::request_vao()
{
    SetCurrent(*glcnt);
    
    Basic_VAO *new_vao=new Basic_VAO;
    new_vao->init_opengl();
    
    new_vao->set_world(lx,ly,lz,scale);
    
    vao.push_back(new_vao);
    
    return new_vao;
}

void EMGeometry_GL::update_grid(double lx_,double ly_,double lz_)
{
    SetCurrent(*glcnt);
    
    lx=lx_;
    ly=ly_;
    lz=lz_;
    
    double l_max=var_max(lx,ly,lz);
    scale=1.0/l_max;
    
    lx*=scale;
    ly*=scale;
    lz*=scale;
    
    gxd->set_grid(1,1,Vector3(-0.5*lx,0,0),Vector3(0,ly,0),Vector3(0,0,lz));
    gxu->set_grid(1,1,Vector3(+0.5*lx,0,0),Vector3(0,ly,0),Vector3(0,0,lz));
    gyd->set_grid(1,1,Vector3(0,-0.5*ly,0),Vector3(lx,0,0),Vector3(0,0,lz));
    gyu->set_grid(1,1,Vector3(0,+0.5*ly,0),Vector3(lx,0,0),Vector3(0,0,lz));
    gzd->set_grid(1,1,Vector3(0,0,-0.5*lz),Vector3(lx,0,0),Vector3(0,ly,0));
    gzu->set_grid(1,1,Vector3(0,0,+0.5*lz),Vector3(lx,0,0),Vector3(0,ly,0));
    
    for(unsigned int i=0;i<vao.size();i++) vao[i]->set_world(lx,ly,lz,scale);
}

// GeomOP_Panel

GeomOP_Panel::GeomOP_Panel(wxWindow *parent,EMGeometry_GL *engine_)
    :PanelsListBase(parent),
     A(1.0,0,0), B(0,1.0,0), C(0,0,1.0), O(0,0,0), color(1.0,1.0,1.0),
     engine(engine_),
     vao(nullptr)
{
    color_btn=new wxButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT|wxBORDER_NONE);
    wires_btn=new wxToggleButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    hide_btn=new wxToggleButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    color_btn->SetBackgroundColour(wxColour(255.0,255.0,255.0));
    
    std::string empty_16=PathManager::locate_resource("resources/empty_16.png").generic_string();
    std::string wireframe_on_16=PathManager::locate_resource("resources/wireframe_on_16.png").generic_string();
    std::string wireframe_off_16=PathManager::locate_resource("resources/wireframe_off_16.png").generic_string();
    std::string sun_on_16=PathManager::locate_resource("resources/sun_on_16.png").generic_string();
    std::string sun_off_16=PathManager::locate_resource("resources/sun_off_16.png").generic_string();
    
    wxBitmap color_bitmap=ImagesManager::get_bitmap(empty_16);
    wxBitmap wireframe_on_bitmap=ImagesManager::get_bitmap(wireframe_on_16);
    wxBitmap wireframe_off_bitmap=ImagesManager::get_bitmap(wireframe_off_16);
    wxBitmap eye_on_bitmap=ImagesManager::get_bitmap(sun_on_16);
    wxBitmap eye_off_bitmap=ImagesManager::get_bitmap(sun_off_16);
    
    color_btn->SetBitmap(color_bitmap);
    
    wires_btn->SetBitmap(wireframe_off_bitmap);
    wires_btn->SetBitmapPressed(wireframe_on_bitmap);
    
    hide_btn->SetBitmap(eye_on_bitmap);
    hide_btn->SetBitmapPressed(eye_off_bitmap);
    
    wxPanel *buffer3=new wxPanel(this);
    
    header_insert(5,color_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    header_insert(6,wires_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    header_insert(7,hide_btn,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    header_insert(8,buffer3,wxSizerFlags(1));
    
    color_btn->Bind(wxEVT_BUTTON,&GeomOP_Panel::vao_color_event,this);
    wires_btn->Bind(wxEVT_TOGGLEBUTTON,&GeomOP_Panel::vao_wires_event,this);
    hide_btn->Bind(wxEVT_TOGGLEBUTTON,&GeomOP_Panel::vao_hide_event,this);
    
    SetWindowStyle(wxBORDER_RAISED);
}

GeomOP_Panel::~GeomOP_Panel()
{
    if(vao!=nullptr) engine->forget_vao(vao);
}

void GeomOP_Panel::apoptose(wxCommandEvent &event)
{
    if(vao!=nullptr)
    {
        engine->forget_vao(vao);
        vao=nullptr;
    }
    
    PanelsListBase::apoptose(event);
}

void GeomOP_Panel::collapse() {}
void GeomOP_Panel::expand() { }
std::string GeomOP_Panel::get_lua() { return ""; }
int GeomOP_Panel::get_material() { return -1; }

void GeomOP_Panel::set(std::vector<std::string> const &args)
{
}

void GeomOP_Panel::set_color(Vector3 const &color_)
{
    color=color_;
    color_btn->SetBackgroundColour(wxColour(255*color.x,
                                            255*color.y,
                                            255*color.z));
    update_vao_display();
}

void GeomOP_Panel::update_vao_display()
{
    if(vao!=nullptr) vao->set_shading(color,wires_btn->GetValue(),!hide_btn->GetValue());
}

void GeomOP_Panel::vao_color_event(wxCommandEvent &event)
{
    wxColourDialog dialog(this);
    dialog.ShowModal();
    
    wxColourData cdata=dialog.GetColourData();
    wxColour tmp=cdata.GetColour();
    
    color=Vector3(tmp.Red()/255.0,tmp.Green()/255.0,tmp.Blue()/255.0);
    
    color_btn->SetBackgroundColour(tmp);
    
    update_vao_display();
    
    event.Skip();
}

void GeomOP_Panel::vao_hide_event(wxCommandEvent &event) { update_vao_display(); hide_btn->Refresh(); event.Skip(); }
void GeomOP_Panel::vao_wires_event(wxCommandEvent &event) { update_vao_display(); wires_btn->Refresh(); event.Skip(); }

void GeomOP_Panel::update_world(double lx_,double ly_,double lz_)
{
    lx=lx_;
    ly=ly_;
    lz=lz_;
}

//###############
//###############

enum
{
    MENU_NEW,
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT
};

EMGeometry_Frame::EMGeometry_Frame(wxString const &title,wxFileName const &fname)
    :BaseFrame(title),
     lx(300e-9), ly(300e-9), lz(300e-9)
{
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    
    ctrl_panel=new wxScrolledWindow(splitter);
    gl=new EMGeometry_GL(splitter);
    
    // Controls Panel
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // - Window size
    
    wxStaticBoxSizer *window_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Computational Window");
    
    lx_ctrl=new NamedTextCtrl<double>(window_sizer->GetStaticBox(),"lx: ",lx);
    ly_ctrl=new NamedTextCtrl<double>(window_sizer->GetStaticBox(),"ly: ",ly);
    lz_ctrl=new NamedTextCtrl<double>(window_sizer->GetStaticBox(),"lz: ",lz);
    
    window_sizer->Add(lx_ctrl,wxSizerFlags().Expand());
    window_sizer->Add(ly_ctrl,wxSizerFlags().Expand());
    window_sizer->Add(lz_ctrl,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(window_sizer,wxSizerFlags().Expand());
    
    // - Base material
    
    def_mat_ctrl=new NamedTextCtrl<int>(ctrl_panel,"Default Material: ",0);
    ctrl_sizer->Add(def_mat_ctrl,wxSizerFlags().Expand());
    
    // - Geometry
    
    geom_top_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Geometry");
    
    // New Operation
    
    wxBoxSizer *add_op_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxString choices[]={"Block","Cone","Cylinder","Layer","Mesh","Sphere"};
    
    op_add_choice=new wxChoice(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,6,choices);
    op_add_choice->SetSelection(0);
    wxButton *add_op_btn=new wxButton(ctrl_panel,wxID_ANY,"Add Operation");
    wxButton *autocolor_btn=new wxButton(ctrl_panel,wxID_ANY,"AC",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    
    add_op_sizer->Add(op_add_choice,wxSizerFlags(1).Expand());
    add_op_sizer->Add(add_op_btn,wxSizerFlags().Expand());
    add_op_sizer->Add(autocolor_btn,wxSizerFlags().Expand());
    
    geom_top_sizer->Add(add_op_sizer,wxSizerFlags().Expand());
    
    op=new PanelsList<GeomOP_Panel>(ctrl_panel);
    
    geom_top_sizer->Add(op,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(geom_top_sizer,wxSizerFlags(1).Expand());
    
    splitter->SetMinimumPaneSize(300);
    splitter->SplitVertically(ctrl_panel,gl,300);
    
    top_sizer->Add(splitter,wxSizerFlags(1).Expand());
    
    SetSizer(top_sizer);
    
//    if(fname.IsOk())
//    {
//        load_project(fname);
//    }
//    
    ctrl_panel->SetScrollRate(50,50);
    ctrl_panel->Layout();
    ctrl_panel->FitInside();
    
    // Menus
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    
    file_menu->Append(MENU_NEW,"New");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    add_op_btn->Bind(wxEVT_BUTTON,&EMGeometry_Frame::evt_add_operation,this);
    autocolor_btn->Bind(wxEVT_BUTTON,&EMGeometry_Frame::evt_autocolor,this);
    lx_ctrl->Bind(EVT_NAMEDTXTCTRL,&EMGeometry_Frame::evt_update_grid,this);
    ly_ctrl->Bind(EVT_NAMEDTXTCTRL,&EMGeometry_Frame::evt_update_grid,this);
    lz_ctrl->Bind(EVT_NAMEDTXTCTRL,&EMGeometry_Frame::evt_update_grid,this);
    
    Bind(wxEVT_MENU,&EMGeometry_Frame::evt_menu,this);
    Bind(EVT_PLIST_REMOVE,&EMGeometry_Frame::evt_remove_operation,this);
    Bind(EVT_PLIST_RESIZE,&EMGeometry_Frame::evt_refit,this);
    Bind(EVT_PLIST_DOWN,&EMGeometry_Frame::evt_operation_down,this);
    Bind(EVT_PLIST_UP,&EMGeometry_Frame::evt_operation_up,this);
}

EMGeometry_Frame::~EMGeometry_Frame()
{
    op->clear();
}

void EMGeometry_Frame::autocolor()
{
    unsigned int i;
    
    double min_mat=def_mat_ctrl->get_value();
    double max_mat=min_mat;
    
    for(i=0;i<op->get_size();i++)
    {
        double m=op->get_panel(i)->get_material();
        
        if(m>=0)
        {
            min_mat=std::min(m,min_mat);
            max_mat=std::max(m,max_mat);
        }
    }
    
    if(min_mat==max_mat) return;
    double span=max_mat-min_mat;
    
    for(i=0;i<op->get_size();i++)
    {
        GeomOP_Panel *panel=op->get_panel(i);
        int m=panel->get_material();
        
        if(m>=0)
        {
            double r=0,g=0,b=0;
            
            degra((m-min_mat)/span,1.0,r,g,b);
            panel->set_color(Vector3(r,g,b));
        }
    }
}

void EMGeometry_Frame::evt_add_operation(wxCommandEvent &event)
{
    int selection=op_add_choice->GetSelection();
        
    GeomOP_Panel *panel=nullptr;
    
    chk_var(selection);
    
         if(selection==0 || selection==4) panel=op->add_panel<GOP_Block>(gl);
    else if(selection==1) panel=op->add_panel<GOP_Cone>(gl);
    else if(selection==2) panel=op->add_panel<GOP_Cylinder>(gl);
    else if(selection==3) panel=op->add_panel<GOP_Layer>(gl);
    else if(selection==5) panel=op->add_panel<GOP_Sphere>(gl);
    else panel=op->add_panel<GeomOP_Panel>(gl);
    
    panel->update_world(lx,ly,lz);
    
    ctrl_panel->FitInside();
    Layout();
    
    ctrl_panel->FitInside();
    Refresh();
    
    refit();
    
    event.Skip();
}

void EMGeometry_Frame::evt_autocolor(wxCommandEvent &event)
{
    autocolor();
    
    event.Skip();
}

void EMGeometry_Frame::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    switch(ID)
    {
        case MENU_NEW:
            break;
        case MENU_LOAD: evt_load_project("ageom","Load Finite-Difference Geometry"); break;
        case MENU_SAVE: evt_save_project("ageom","Save Finite-Difference Geometry"); break;
        case MENU_SAVE_AS: evt_save_project_as("ageom","Save Finite-Difference Geometry As"); break;
        case MENU_EXIT: Close();
    }
    
    event.Skip();
}

void EMGeometry_Frame::save_project(wxFileName const &fname)
{
    std::ofstream file(fname.GetFullPath().ToStdString(),std::ios::out|std::ios::trunc);
    
    file<<"lx="<<lx<<std::endl;
    file<<"ly="<<ly<<std::endl;
    file<<"lz="<<lz<<std::endl<<std::endl;
    
    file<<"default_material("<<def_mat_ctrl->get_value()<<")"<<std::endl<<std::endl;
    
    for(unsigned int i=0;i<op->get_size();i++)
        file<<op->get_panel(i)->get_lua()<<std::endl;
}

void EMGeometry_Frame::evt_operation_down(wxCommandEvent &event)
{
    refit();
    Layout();
    Refresh();
}

void EMGeometry_Frame::evt_operation_up(wxCommandEvent &event)
{
    refit();
    Layout();
    Refresh();
}

void EMGeometry_Frame::evt_refit(wxCommandEvent &event)
{
    refit();
}

void EMGeometry_Frame::evt_remove_operation(wxCommandEvent &event)
{
    refit();
    
    event.Skip();
}

void EMGeometry_Frame::evt_update_grid(wxCommandEvent &event)
{
    lx=lx_ctrl->get_value();
    ly=ly_ctrl->get_value();
    lz=lz_ctrl->get_value();
    
    update_grid();
    
    event.Skip();
}

int lua_set_full(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    EMGeometry_Frame *frame=reinterpret_cast<EMGeometry_Frame*>(lua_touserdata(L,-1));
    
    frame->def_mat_ctrl->set_value(lua_tointeger(L,1));
    
    return 0;
}

template<class T>
int lua_add_command(lua_State *L)
{
    int i,N=lua_gettop(L);
    
    std::vector<std::string> args(N);
    for(i=0;i<N;i++) args[i]=lua_tostring(L,i+1);
    
    lua_getglobal(L,"bound_class");
    EMGeometry_Frame *frame=reinterpret_cast<EMGeometry_Frame*>(lua_touserdata(L,-1));
    
    frame->add_command<T>(args);
    
    return 0;
}

void EMGeometry_Frame::load_project(wxFileName const &fname)
{
    // Cleaning
    
    op->clear();
    
    //
    
    /*std::string full_script=script;
    
    if(append_cmd.size()>0 || parameter_cmd.size()>0)
    {
        std::stringstream strm;
        
        struct_id_lock.lock();
        strm<<"buf/struct_script_"<<struct_id<<".lua";
        struct_id++;
        struct_id_lock.unlock();
        
        full_script=strm.str();
        
        std::string script_data;
        std::ifstream f_in(script,std::ios::in|std::ios::binary);
        std::getline(f_in,script_data,'\0');
        f_in.close();
        
        std::ofstream f_out(full_script,std::ios::out|std::ios::trunc|std::ios::binary);
        
        for(unsigned int l=0;l<parameter_cmd.size();l++)
        {
            f_out<<parameter_cmd[l]<<std::endl;
        }
        
        f_out<<std::endl<<script_data<<std::endl<<std::endl;
        
        for(unsigned int l=0;l<append_cmd.size();l++)
        {
            f_out<<append_cmd[l]<<std::endl;
        }
        
        f_out.close();
    }*/
    
    std::cout<<"Lua"<<std::endl;
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"bound_class");
    
    /*lua_pushlightuserdata(L,reinterpret_cast<void*>(L));
    lua_setglobal(L,"mother_lua_state");*/
    
    lua_register(L,"add_block",lua_add_command<GOP_Block>);
    lua_register(L,"add_cone",lua_add_command<GOP_Cone>);
//    lua_register(L,"add_cone_o",lop_add_cone_o);
    lua_register(L,"add_cylinder",lua_add_command<GOP_Cylinder>);
//    lua_register(L,"add_cylinder_o",lop_add_cylinder_o);
//    lua_register(L,"add_ellipsoid",lop_add_ellipsoid);
//    lua_register(L,"add_height_map",lop_add_height_map);
    lua_register(L,"add_layer",lua_add_command<GOP_Layer>);
//    lua_register(L,"add_lua_def",lop_add_lua_def);
//    lua_register(L,"add_mesh",lop_add_mesh);
//    lua_register(L,"add_sin_layer",lop_add_sin_layer);
    lua_register(L,"add_sphere",lua_add_command<GOP_Sphere>);
//    lua_register(L,"add_triangle_g",lop_add_triangle_g);
//    lua_register(L,"add_vect_block",lop_add_vect_block);
//    lua_register(L,"add_vect_tri",lop_add_vect_tri);
//    lua_register(L,"coat",lop_coat);
//    lua_register(L,"declare_parameter",declare_parameter_cli);
//    lua_register(L,"erode",lop_erode);
//    lua_register(L,"flip",lop_flip);
//    lua_register(L,"loop",lop_loop_modifier);
//    lua_register(L,"loop_modifier",lop_loop_modifier);
//    lua_register(L,"random_packing",random_packing);
    lua_register(L,"default_material",lua_set_full);
    
    /*lua_register(L,"nearest_integer",nearest_integer);*/
    
    std::string full_script=fname.GetFullPath().ToStdString();
    
    int load_err = luaL_loadfile(L,full_script.c_str());
    
    if(load_err!=LUA_OK)
    {
             if(load_err==LUA_ERRFILE) std::cout<<"Lua file error with "<<full_script<<std::endl;
        else if(load_err==LUA_ERRSYNTAX) std::cout<<"Lua syntax error with "<<full_script<<std::endl;
        else std::cout<<"Lua error with "<<full_script<<std::endl;
        std::cin.get();
        return;
    }
    
    lua_pcall(L, 0, 0, 0);
    
    lua_getglobal(L,"lx");
    lua_getglobal(L,"ly");
    lua_getglobal(L,"lz");
    
    lx=lua_tonumber(L,1);
    ly=lua_tonumber(L,2);
    lz=lua_tonumber(L,3);
    
    lua_close(L);
    
    lx_ctrl->set_value(lx);
    ly_ctrl->set_value(ly);
    lz_ctrl->set_value(lz);
    
//    lua_lst.apply_operations(Nx,Ny,Nz,Dx,Dy,Dz,matgrid);
    update_grid();
    
    autocolor();
}

void EMGeometry_Frame::refit()
{
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
}

void EMGeometry_Frame::update_grid()
{
    if(gl->gl_ok)
    {
        gl->update_grid(lx_ctrl->get_value(),
                        ly_ctrl->get_value(),
                        lz_ctrl->get_value());
        
        for(unsigned int i=0;i<op->get_size();i++)
            op->get_panel(i)->update_world(lx,ly,lz);
    }
}
