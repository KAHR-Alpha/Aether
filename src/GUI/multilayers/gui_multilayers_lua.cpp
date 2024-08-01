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

#include <gui_multilayers.h>

#include <logger.h>

namespace lua_gui_multilayer
{

int allocate(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    
    MultilayerFrame *p_frame=static_cast<MultilayerFrame*>(lua_touserdata(L,-1));
    lua_set_metapointer(L,"metatable_multilayer_frame",p_frame);
    
    return 1;
}

int add_layer(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    
    double height=lua_tonumber(L,2);
    double std_dev=lua_tonumber(L,3);
    GUI::Material *material=dynamic_cast<GUI::Material*>(lua_get_metapointer<Material>(L,4));
    
    p_frame->layers_list->add_panel<LayerPanel>(height,std_dev,material,true);
    
    return 0;
}

int add_bragg(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    
    double height_1=lua_tonumber(L,2);
    double std_dev_1=lua_tonumber(L,3);
    Material *material_1=lua_get_metapointer<Material>(L,4);
    
    double height_2=lua_tonumber(L,5);
    double std_dev_2=lua_tonumber(L,6);
    Material *material_2=lua_get_metapointer<Material>(L,7);
    
    double height_core=lua_tonumber(L,8);
    double std_dev_core=lua_tonumber(L,9);
    Material *material_core=lua_get_metapointer<Material>(L,10);
    
    double global_std_dev=lua_tonumber(L,11);
    double g_factor=lua_tonumber(L,12);
    int N_top=lua_tointeger(L,13);
    int N_bottom=lua_tointeger(L,14);
    
    p_frame->layers_list->add_panel<BraggPanel>(height_1,std_dev_1,dynamic_cast<GUI::Material*>(material_1),
                                                height_2,std_dev_2,dynamic_cast<GUI::Material*>(material_2),
                                                height_core,std_dev_core,dynamic_cast<GUI::Material*>(material_core),
                                                global_std_dev,g_factor,N_top,N_bottom);
    
    return 0;
}

int set_angles(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    int Na=lua_tointeger(L,2);
    
    p_frame->angle_ctrl->set_value(Na);
    
    return 0;
}

int set_spectrum(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    
    double lambda_min=lua_tonumber(L,2);
    double lambda_max=lua_tonumber(L,3);
    int Nl=lua_tointeger(L,4);
    
    p_frame->spectrum->set_spectrum(lambda_min,lambda_max);
    p_frame->spectrum->set_Np(Nl);
    
    return 0;
}

int set_substrate(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    Material *p_material=lua_get_metapointer<Material>(L,2);
    
    p_frame->substrate_selector->set_material(dynamic_cast<GUI::Material*>(p_material));
    
    return 0;
}

int set_superstrate(lua_State *L)
{
    MultilayerFrame *p_frame=lua_get_metapointer<MultilayerFrame>(L,1);
    Material *p_material=lua_get_metapointer<Material>(L,2);
    
    p_frame->superstrate_selector->set_material(dynamic_cast<GUI::Material*>(p_material));
    
    return 0;
}

}

void MultilayerFrame::load_project(wxFileName const &fname_)
{
    end_computation();
    
    layers_list->clear();
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    // Materials
    
    lua_gui_material::Loader loader;
    loader.create_metatable(L);
    
    // MultilayerFrame
    
    lua_pushlightuserdata(L,static_cast<void*>(this));
    lua_setglobal(L,"bound_class");
    
    lua_register(L,"Multilayer",lua_gui_multilayer::allocate);
    
    create_obj_metatable(L,"metatable_multilayer_frame");
    
    metatable_add_func(L,"add_bragg",&lua_gui_multilayer::add_bragg);
    metatable_add_func(L,"add_layer",&lua_gui_multilayer::add_layer);
    metatable_add_func(L,"angles",&lua_gui_multilayer::set_angles);
    metatable_add_func(L,"spectrum",&lua_gui_multilayer::set_spectrum);
    metatable_add_func(L,"substrate",&lua_gui_multilayer::set_substrate);
    metatable_add_func(L,"superstrate",&lua_gui_multilayer::set_superstrate);
    
    int load_err = luaL_loadfile(L,fname.c_str());
    
    if(load_err!=LUA_OK)
    {
             if(load_err==LUA_ERRFILE) Plog::print("Lua file error with ", fname, "\n");
        else if(load_err==LUA_ERRSYNTAX) Plog::print("Lua syntax error with ", fname, "\n");
        else Plog::print("Lua error with ", fname, "\n");
        std::cin.get();
        return;
    }
    
    lua_pcall(L,0,0,0);
    
    lua_close(L);
    
    //MaterialsLib::consolidate();
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
    
    int Nl=spectrum->get_Np();
    double lambda_min=spectrum->get_lambda_min();
    double lambda_max=spectrum->get_lambda_max();
    
    lambda.resize(Nl);
    linspace(lambda,lambda_min,lambda_max);
    
    int Na=angle_ctrl->get_value();
    
    angle.resize(Na);
    linspace(angle,0.0,90.0);
    
    SetTitle(wxString("Multilayer : ").Append(project_fname.GetName()));
        
    recompute();
}

void MultilayerFrame::save_project(wxFileName const &fname_)
{
    // Materials
    
    lua_gui_material::Translator mtr("");
    
    mtr.gather(superstrate_selector->get_material());
    mtr.gather(substrate_selector->get_material());
    
    for(std::size_t i=0;i<layers_list->get_size();i++)
    {
        std::vector<GUI::Material*> layer_mats;
        layers_list->get_panel(i)->get_materials(layer_mats);
        
        for(GUI::Material *mat:layer_mats)
            mtr.gather(mat);
    }
        
    // Model
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    file<<mtr.get_header()<<"\n";
    
    file<<"mode=Multilayer()\n";
    file<<"mode:angles("<<angle_ctrl->get_value()<<")\n";
    file<<"mode:spectrum("<<spectrum->get_lambda_min()<<","<<spectrum->get_lambda_max()<<","<<spectrum->get_Np()<<")\n";
    
    file<<"mode:superstrate("<<mtr(superstrate_selector->get_material())<<")\n";
    
    for(unsigned int i=0;i<layers_list->get_size();i++)
    {
        file<<"mode:"<<layers_list->get_panel(i)->get_lua_string(mtr)<<std::endl;
    }
    file<<std::endl;
    
    file<<"mode:substrate("<<mtr(substrate_selector->get_material())<<")";
    
    file.close();
    
    SetTitle(wxString("Multilayer : ").Append(project_fname.GetName()));
}
