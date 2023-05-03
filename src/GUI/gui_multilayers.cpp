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

#include <phys_tools.h>
#include <string_tools.h>

#include <wx/splitter.h>

#include <gui_multilayers.h>
#include <gui_multilayers_utils.h>

extern std::ofstream plog;

wxDEFINE_EVENT(EVT_REFRESH,wxCommandEvent);

enum
{
    MENU_NEW,
    MENU_LOAD,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_EXIT
};

MultilayerFrame::MultilayerFrame(wxString const &title)
    :BaseFrame(title),
     spectral(true),
     lambda(481), angle(901),
     statistical_computation(false), thread_running(false),
     N_samples(10000), thread(nullptr),
     disp_Rs(true), disp_Rp(true), disp_R_avg(true),
     disp_Ts(true), disp_Tp(true), disp_T_avg(true),
     disp_As(true), disp_Ap(true), disp_A_avg(true)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    structure_panel=new wxScrolledWindow(splitter);
    display_panel=new wxPanel(splitter);
    
    // Structure Panel
    
    wxString computation_choices[]={"Spectral","Angular"};
    
    wxBoxSizer *structure_sizer=new wxBoxSizer(wxVERTICAL);
    
    computation_ctrl=new wxRadioBox(structure_panel,wxID_ANY,"Computation",wxDefaultPosition,wxDefaultSize,2,computation_choices);
    
    spectrum=new SpectrumSelector(structure_panel,370e-9,850e-9,481,true);
    linspace(lambda,370e-9,850e-9);
    
    angle_ctrl=new NamedTextCtrl<int>(structure_panel,"N Angles",901,true);
    for(unsigned int i=0;i<901;i++) angle[i]=90.0*i/900.0;
    
    // - Display
    
    wxStaticBoxSizer *polar_boxsizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Display");
    
    polar_all_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"All");
    polar_R_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"R");
    polar_T_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"T");
    polar_A_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"A");
    
    polar_S_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"S");
    polar_Rs_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Rs");
    polar_Rp_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Rp");
    polar_R_avg_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Ravg");
    
    polar_P_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"P");
    polar_Ts_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Ts");
    polar_Tp_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Tp");
    polar_T_avg_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Tavg");
    
    polar_avg_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"avg");
    polar_As_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"As");
    polar_Ap_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Ap");
    polar_A_avg_ctrl=new wxCheckBox(polar_boxsizer->GetStaticBox(),wxID_ANY,"Aavg");
    
    
    polar_all_ctrl->SetValue(true);
    polar_P_ctrl->SetValue(true);
    polar_S_ctrl->SetValue(true);
    polar_avg_ctrl->SetValue(true);
    
    polar_R_ctrl->SetValue(true);
    polar_Rs_ctrl->SetValue(true);
    polar_Rp_ctrl->SetValue(true);
    polar_R_avg_ctrl->SetValue(true);
    
    polar_T_ctrl->SetValue(true);
    polar_Ts_ctrl->SetValue(true);
    polar_Tp_ctrl->SetValue(true);
    polar_T_avg_ctrl->SetValue(true);
    
    polar_A_ctrl->SetValue(true);
    polar_As_ctrl->SetValue(true);
    polar_Ap_ctrl->SetValue(true);
    polar_A_avg_ctrl->SetValue(true);
    
    wxGridSizer *polar_sizer=new wxGridSizer(4);
    
    polar_sizer->Add(polar_all_ctrl);
    polar_sizer->Add(polar_R_ctrl);
    polar_sizer->Add(polar_T_ctrl);
    polar_sizer->Add(polar_A_ctrl);
    
    polar_sizer->Add(polar_S_ctrl);
    polar_sizer->Add(polar_Rs_ctrl);
    polar_sizer->Add(polar_Ts_ctrl);
    polar_sizer->Add(polar_As_ctrl);
    
    polar_sizer->Add(polar_P_ctrl);
    polar_sizer->Add(polar_Rp_ctrl);
    polar_sizer->Add(polar_Tp_ctrl);
    polar_sizer->Add(polar_Ap_ctrl);
    
    polar_sizer->Add(polar_avg_ctrl);
    polar_sizer->Add(polar_R_avg_ctrl);
    polar_sizer->Add(polar_T_avg_ctrl);
    polar_sizer->Add(polar_A_avg_ctrl);
    
    polar_boxsizer->Add(polar_sizer,wxSizerFlags().Expand());
    
    polar_all_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_all,this);
    polar_R_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_R,this);
    polar_T_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_T,this);
    polar_A_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_A,this);
    polar_P_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_P,this);
    polar_S_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_S,this);
    polar_avg_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_avg,this);
    
    polar_Rs_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_Rp_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_R_avg_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_Ts_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_Tp_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_T_avg_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_As_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_Ap_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    polar_A_avg_ctrl->Bind(wxEVT_CHECKBOX,&MultilayerFrame::evt_polar_switch,this);
    
    // - Structure
    
    wxStaticBoxSizer *superstrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Superstrate");
    wxStaticBoxSizer *layers_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Layers");
    wxStaticBoxSizer *substrate_sizer=new wxStaticBoxSizer(wxVERTICAL,structure_panel,"Substrate");
    
    superstrate_selector=new MiniMaterialSelector(structure_panel);
    
    wxBoxSizer *add_layer_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    add_layer_type=new wxChoice(structure_panel,wxID_ANY);
    add_layer_type->Append("Bragg");
    add_layer_type->Append("Layer");
    add_layer_type->SetSelection(1);
    
    wxButton *add_layer_btn=new wxButton(structure_panel,wxID_ANY,"Add");
    
    add_layer_sizer->Add(add_layer_type,wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
    add_layer_sizer->Add(add_layer_btn,wxSizerFlags());
    
    layers_list=new PanelsList<LayerPanelBase>(structure_panel);
    substrate_selector=new MiniMaterialSelector(structure_panel);
    
    sampling_ctrl=new NamedTextCtrl<int>(structure_panel,"Statistical Sampling",N_samples,true);
    
    wxButton *export_btn=new wxButton(structure_panel,wxID_ANY,"Export Data");
    
    superstrate_sizer->Add(superstrate_selector,wxSizerFlags().Expand());
    layers_sizer->Add(add_layer_sizer,wxSizerFlags().Expand());
    layers_sizer->Add(layers_list,wxSizerFlags(1).Expand());
    substrate_sizer->Add(substrate_selector,wxSizerFlags().Expand());
    
    structure_sizer->Add(computation_ctrl,std_flags);
    structure_sizer->Add(spectrum,std_flags);
    structure_sizer->Add(angle_ctrl,std_flags);
    structure_sizer->Add(polar_boxsizer,std_flags);
    structure_sizer->Add(superstrate_sizer,std_flags);
    structure_sizer->Add(layers_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    structure_sizer->Add(substrate_sizer,std_flags);
    structure_sizer->Add(sampling_ctrl,std_flags);
    structure_sizer->Add(export_btn,std_flags);
    
    structure_panel->SetSizer(structure_sizer);
    
    // Display Panel
    
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    graph=new Graph(display_panel);
    graph->spectral_data=true;
    
    ctrl_slider=new SliderDisplay(display_panel,901,0,90.0,1.0,"°");
    
    display_sizer->Add(graph,wxSizerFlags(1).Expand());
    display_sizer->Add(ctrl_slider,wxSizerFlags().Expand());
    
    display_panel->SetSizer(display_sizer);
    
    splitter->SplitVertically(structure_panel,display_panel,270);
    splitter->SetMinimumPaneSize(20);
    
    // Menu
    
    wxMenuBar *menu_bar=new wxMenuBar;
    wxMenu *file_menu=new wxMenu();
    
//    file_menu->Append(MENU_NEW,"New");
//    file_menu->AppendSeparator();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->Append(MENU_SAVE,"Save");
    file_menu->Append(MENU_SAVE_AS,"Save As...");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    
    append_help_menu(menu_bar);
    
    SetMenuBar(menu_bar);
    
    // Bindings
    
    add_layer_btn->Bind(wxEVT_BUTTON,&MultilayerFrame::evt_add_layer,this);
    angle_ctrl->Bind(EVT_NAMEDTXTCTRL,&MultilayerFrame::evt_angle,this);
    computation_ctrl->Bind(wxEVT_RADIOBOX,&MultilayerFrame::evt_mode_switch,this);
    export_btn->Bind(wxEVT_BUTTON,&MultilayerFrame::evt_export_data,this);
    
    Bind(EVT_LAYER_PANEL,&MultilayerFrame::evt_structure_change,this);
    Bind(EVT_LENGTH_SELECTOR,&MultilayerFrame::evt_structure_change,this);
    Bind(wxEVT_MENU,&MultilayerFrame::evt_menu,this);
    Bind(EVT_MINIMAT_SELECTOR,&MultilayerFrame::evt_structure_change,this);
    Bind(EVT_NAMEDTXTCTRL,&MultilayerFrame::evt_structure_change,this);
    Bind(EVT_PLIST_DOWN,&MultilayerFrame::evt_panels_change,this);
    Bind(EVT_PLIST_UP,&MultilayerFrame::evt_panels_change,this);
    Bind(EVT_PLIST_RESIZE,&MultilayerFrame::evt_panels_change,this);
    Bind(EVT_PLIST_REMOVE,&MultilayerFrame::evt_panels_change,this);
    Bind(EVT_REFRESH,&MultilayerFrame::evt_refresh,this);
    Bind(EVT_SPECTRUM_SELECTOR,&MultilayerFrame::evt_spectrum,this);
    Bind(wxEVT_SLIDER,&MultilayerFrame::evt_slider,this);
    
    structure_panel->SetScrollbars(10,10,50,50);
    structure_panel->FitInside();
}

MultilayerFrame::~MultilayerFrame()
{
    end_computation();
}

void MultilayerFrame::end_computation()
{
    statistical_computation=false;
    
    computation_mutex.Lock();
}

void MultilayerFrame::evt_add_layer(wxCommandEvent &event)
{
    end_computation();
    
    int selection=add_layer_type->GetSelection();
    
         if(selection==0) layers_list->add_panel<BraggPanel>();
    else if(selection==1) layers_list->add_panel<LayerPanel>(true);
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
    
    recompute();
}

void MultilayerFrame::evt_angle(wxCommandEvent &event)
{
    end_computation();
    
    int Na=angle_ctrl->get_value();
    
    if(Na<2)
    {
        Na=2;
        angle_ctrl->set_value(Na);
    }
    
    angle.resize(Na);
    for(int i=0;i<Na;i++) angle[i]=90.0*i/(Na-1.0);
    
    if(spectral) switch_slider_spectral();
    
    recompute();
    
    event.Skip();
}

void MultilayerFrame::evt_export_data(wxCommandEvent &event)
{
    end_computation();
    
    wxString str=wxFileSelector("Export to..",
                                wxEmptyString,
                                wxEmptyString,
                                wxEmptyString,
                                "Text File (*.txt)|*.txt",
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if(!str.empty())
    {
        std::ofstream file(str.ToStdString(),std::ios::out|std::ios::trunc);
        
        unsigned int l;
        
        if(spectral)
        {
            for(l=0;l<lambda.size();l++)
            {
                file<<lambda[l]<<" "<<curr_angle<<" "
                    <<R_TE[l]<<" "<<R_TM[l]<<" "<<R_avg[l]<<" "
                    <<T_TE[l]<<" "<<T_TM[l]<<" "<<T_avg[l]<<" "
                    <<A_TE[l]<<" "<<A_TM[l]<<" "<<A_avg[l]<<std::endl;
            }
        }
        else
        {
            for(l=0;l<angle.size();l++)
            {
                file<<curr_lambda<<" "<<angle[l]<<" "
                    <<R_TE[l]<<" "<<R_TM[l]<<" "<<R_avg[l]<<" "
                    <<T_TE[l]<<" "<<T_TM[l]<<" "<<T_avg[l]<<" "
                    <<A_TE[l]<<" "<<A_TM[l]<<" "<<A_avg[l]<<std::endl;
            }
        }
    }
}

void MultilayerFrame::evt_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
        
    if(ID==MENU_NEW) evt_reset_project();
    else if(ID==MENU_LOAD) evt_load_project("amm","Aether Mutlilayer Mode Files");
    else if(ID==MENU_SAVE) evt_save_project("amm","Aether Mutlilayer Mode Files");
    else if(ID==MENU_SAVE_AS) evt_save_project_as("amm","Aether Mutlilayer Mode Files");
    else if(ID==MENU_EXIT) Close();
    
    event.Skip();
}

void MultilayerFrame::evt_mode_switch(wxCommandEvent &event)
{
    end_computation();
    
    if(computation_ctrl->GetSelection()==0) spectral=true;
    else spectral=false;
    
    if(spectral) switch_slider_spectral();
    else switch_slider_angular();
    
    recompute();
    
    event.Skip();
}

void MultilayerFrame::evt_panels_change(wxCommandEvent &event)
{
    end_computation();
    
    structure_panel->FitInside();
    structure_panel->Layout();
    rename_panels();
    
    recompute();
}

void MultilayerFrame::evt_polar_A(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_A_ctrl->GetValue();
    
    polar_As_ctrl->SetValue(val);
    polar_Ap_ctrl->SetValue(val);
    polar_A_avg_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_all(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_all_ctrl->GetValue();
    
    polar_Rs_ctrl->SetValue(val);
    polar_Rp_ctrl->SetValue(val);
    polar_R_avg_ctrl->SetValue(val);
    
    polar_Ts_ctrl->SetValue(val);
    polar_Tp_ctrl->SetValue(val);
    polar_T_avg_ctrl->SetValue(val);
    
    polar_As_ctrl->SetValue(val);
    polar_Ap_ctrl->SetValue(val);
    polar_A_avg_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_avg(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_avg_ctrl->GetValue();
    
    polar_R_avg_ctrl->SetValue(val);
    polar_T_avg_ctrl->SetValue(val);
    polar_A_avg_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_P(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_P_ctrl->GetValue();
    
    polar_Rp_ctrl->SetValue(val);
    polar_Tp_ctrl->SetValue(val);
    polar_Ap_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_R(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_R_ctrl->GetValue();
    
    polar_Rs_ctrl->SetValue(val);
    polar_Rp_ctrl->SetValue(val);
    polar_R_avg_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_S(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_S_ctrl->GetValue();
    
    polar_Rs_ctrl->SetValue(val);
    polar_Ts_ctrl->SetValue(val);
    polar_As_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_switch(wxCommandEvent &event)
{
    end_computation();
    update_aggregate_polar_ctrl();
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_polar_T(wxCommandEvent &event)
{
    end_computation();
    
    bool val=polar_T_ctrl->GetValue();
    
    polar_Ts_ctrl->SetValue(val);
    polar_Tp_ctrl->SetValue(val);
    polar_T_avg_ctrl->SetValue(val);
    
    update_aggregate_polar_ctrl();
    
    recompute();
    event.Skip();
}

void MultilayerFrame::evt_reset_project()
{
}

void MultilayerFrame::evt_refresh(wxCommandEvent &event)
{
    if(statistical_computation)
    {
        for(unsigned int i=0;i<R_TE_sum.size();i++)
        {
            R_TE[i]=R_TE_sum[i]/static_cast<double>(Nc_samples);
            R_TM[i]=R_TM_sum[i]/static_cast<double>(Nc_samples);
            
            T_TE[i]=T_TE_sum[i]/static_cast<double>(Nc_samples);
            T_TM[i]=T_TM_sum[i]/static_cast<double>(Nc_samples);
            
            A_TE[i]=1.0-R_TE[i]-T_TE[i];
            A_TM[i]=1.0-R_TM[i]-T_TM[i];
            
            R_avg[i]=0.5*(R_TE[i]+R_TM[i]);
            T_avg[i]=0.5*(T_TE[i]+T_TM[i]);
            A_avg[i]=0.5*(A_TE[i]+A_TM[i]);
        }
            
        graph->Refresh();
    }
    
    event.Skip();
}

void MultilayerFrame::evt_slider(wxCommandEvent &event)
{
    end_computation();
    
    if(spectral) curr_angle=ctrl_slider->get_value();
    else curr_lambda=ctrl_slider->get_value();
    
    recompute();
}

void MultilayerFrame::evt_spectrum(wxCommandEvent &event)
{
    end_computation();
    
    if(spectral) switch_slider_spectral();
    else switch_slider_angular();
    
    int Nl=spectrum->get_Np();
    double lambda_min=spectrum->get_lambda_min();
    double lambda_max=spectrum->get_lambda_max();
    
    graph->clear_graph();
    
    lambda.resize(Nl);
    
    for(int l=0;l<Nl;l++)
        lambda[l]=lambda_min+(lambda_max-lambda_min)*l/(Nl-1.0);
    
    recompute();
}

void MultilayerFrame::evt_structure_change(wxCommandEvent &event)
{
    end_computation();
    
    N_samples=sampling_ctrl->get_value();
    
    recompute();
}

int lua_gui_multilayer_mode(lua_State *L)
{
    lua_getglobal(L,"bound_class");
    
    luaL_getmetatable(L,"metatable_multilayer_frame");
    lua_setmetatable(L,-2);
    
    return 1;
}

void MultilayerFrame::load_project(wxFileName const &fname_)
{
    end_computation();
    
    layers_list->clear();
    
    std::string fname=fname_.GetFullPath().ToStdString();
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    MultilayerFrame *p_frame=this;
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&p_frame));
    lua_setglobal(L,"bound_class");
    
    lua_register(L,"const_material",gen_const_material);
    lua_register(L,"gui_multilayer_mode",lua_gui_multilayer_mode);
    
    create_obj_metatable(L,"metatable_multilayer_frame");
    
    // TODO
    //lua_wrapper<0,MultilayerFrame,double,double,std::string>::bind(L,"add_layer",&MultilayerFrame::lua_add_layer);
    lua_wrapper<1,MultilayerFrame,double,double,std::string,
                                  double,double,std::string,
                                  double,double,std::string,
                                  double,double,int,int>::bind(L,"add_bragg",&MultilayerFrame::lua_add_bragg);
    lua_wrapper<2,MultilayerFrame,int>::bind(L,"angles",&MultilayerFrame::lua_set_angles);
    lua_wrapper<3,MultilayerFrame,double,double,int>::bind(L,"spectrum",&MultilayerFrame::lua_set_spectrum);
    lua_wrapper<4,MultilayerFrame,std::string>::bind(L,"substrate",&MultilayerFrame::lua_set_substrate);
    lua_wrapper<5,MultilayerFrame,std::string>::bind(L,"superstrate",&MultilayerFrame::lua_set_superstrate);
    
    int load_err = luaL_loadfile(L,fname.c_str());
    
    if(load_err!=LUA_OK)
    {
             if(load_err==LUA_ERRFILE) std::cout<<"Lua file error with "<<fname.c_str()<<std::endl;
        else if(load_err==LUA_ERRSYNTAX) std::cout<<"Lua syntax error with "<<fname.c_str()<<std::endl;
        else std::cout<<"Lua error with "<<fname.c_str()<<std::endl;
        std::cin.get();
        return;
    }
    
    lua_pcall(L,0,0,0);
    
    lua_close(L);
    
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

void MultilayerFrame::lua_add_layer(double height,double std_dev,GUI::Material *material)
{
    layers_list->add_panel<LayerPanel>(height,std_dev,material,true);
}

void MultilayerFrame::lua_add_bragg(double height_1,double std_dev_1,std::string material_1,
                                    double height_2,double std_dev_2,std::string material_2,
                                    double height_core,double std_dev_core,std::string material_core,
                                    double global_std_dev,double g_factor,int N_top,int N_bottom)
{
    layers_list->add_panel<BraggPanel>(height_1,std_dev_1,material_1,
                                       height_2,std_dev_2,material_2,
                                       height_core,std_dev_core,material_core,
                                       global_std_dev,g_factor,N_top,N_bottom);
}

void MultilayerFrame::lua_set_angles(int Na)
{
    angle_ctrl->set_value(Na);
}

void MultilayerFrame::lua_set_spectrum(double lambda_min,double lambda_max,int Nl)
{
    spectrum->set_spectrum(lambda_min,lambda_max);
    spectrum->set_Np(Nl);
}

void MultilayerFrame::lua_set_substrate(std::string material)
{
    substrate_selector->set_material(material);
}

void MultilayerFrame::lua_set_superstrate(std::string material)
{
    superstrate_selector->set_material(material);
}

void MultilayerFrame::recompute()
{
    end_computation();
    
    int l,Nlb=layers_list->get_size();
    
    statistical_computation=false;
    
    for(l=0;l<Nlb;l++)
    {
        LayerPanelBase *panel=layers_list->get_panel(l);
        
        if(panel->is_statistical())
        {
            statistical_computation=true;
            break;
        }
    }
    
    if(statistical_computation) recompute_statistical();
    else recompute_straight();
}

void MultilayerFrame::recompute_statistical()
{    
    unsigned int N_resize;
    
    if(spectral) N_resize=lambda.size();
    else N_resize=angle.size();
    
    R_TE.resize(N_resize);
    R_TM.resize(N_resize);
    R_TE_sum.resize(N_resize);
    R_TM_sum.resize(N_resize);
    R_avg.resize(N_resize);
    
    T_TE.resize(N_resize);
    T_TM.resize(N_resize);
    T_TE_sum.resize(N_resize);
    T_TM_sum.resize(N_resize);
    T_avg.resize(N_resize);
    
    A_TE.resize(N_resize);
    A_TM.resize(N_resize);
    A_avg.resize(N_resize);
    
    for(unsigned int i=0;i<N_resize;i++)
    {
        R_TE_sum[i]=0;
        R_TM_sum[i]=0;
        
        T_TE_sum[i]=0;
        T_TM_sum[i]=0;
    }
    
    graph->clear_graph();
    
    std::vector<double> *abscissa;
    
    if(spectral) abscissa=&lambda;
    else abscissa=&angle;
    
    double s=0.25;
    if(disp_Rs) graph->add_external_data(abscissa,&R_TE,0,0,s,"Rs");
    if(disp_Ts) graph->add_external_data(abscissa,&T_TE,0,s,0,"Ts");
    if(disp_As) graph->add_external_data(abscissa,&A_TE,s,0,0,"As");
    
    s=0.6;
    if(disp_Rp) graph->add_external_data(abscissa,&R_TM,0,0,s,"Rp");
    if(disp_Tp) graph->add_external_data(abscissa,&T_TM,0,s,0,"Tp");
    if(disp_Ap) graph->add_external_data(abscissa,&A_TM,s,0,0,"Ap");
    
    s=1.0;
    if(disp_R_avg) graph->add_external_data(abscissa,&R_avg,0,0,s,"R_avg");
    if(disp_T_avg) graph->add_external_data(abscissa,&T_avg,0,s,0,"T_avg");
    if(disp_A_avg) graph->add_external_data(abscissa,&A_avg,s,0,0,"A_avg");
    
    graph->set_scale((*abscissa)[0],(*abscissa)[abscissa->size()-1],-0.1,1.1);
    
    graph->Refresh();
    
    //
    
    int l;
    
    N_layers=0;
    int Nlb=layers_list->get_size();
    
    for(l=0;l<Nlb;l++)
    {
        LayerPanelBase *panel=layers_list->get_panel(l);
        N_layers+=panel->get_n_layers();
    }
    
    layers_material.clear();
    layers_material.reserve(N_layers);
    
    for(l=0;l<Nlb;l++)
    {
        LayerPanelBase *panel=layers_list->get_panel(l);
        
        panel->get_materials(layers_material);
    }
    
    layers_height_samples.clear();
    layers_height_samples.resize(N_samples);
    
    for(int i=0;i<N_samples;i++)
    {
        layers_height_samples[i].reserve(N_layers);
        
        for(l=0;l<Nlb;l++)
        {
            LayerPanelBase *panel=layers_list->get_panel(l);
            
            panel->get_heights(layers_height_samples[i]);
        }
    }
    
    //
    
    thread=new MLF_computation_thread;
    thread->frame=this;
    
    thread->Run();
    computation_mutex.Unlock();
    
    while(!thread_running) wxThread::This()->Sleep(1);
}

MLF_computation_thread::MLF_computation_thread()
    :wxThread()
{}

wxThread::ExitCode MLF_computation_thread::Entry()
{
    frame->recompute_statistical_thread();
    
    Delete();
    
    return 0;
}

void MultilayerFrame::recompute_statistical_thread()
{
    computation_mutex.Lock();
    thread_running=true;
    
    int l;
    unsigned int i;
    Angle ang_rad;
    
    ml.set_N_layers(N_layers);
    
    Nc_samples=0;
    
    double tmp_R_TE,tmp_R_TM,
           tmp_T_TE,tmp_T_TM,
           tmp_A_TE,tmp_A_TM;
    
    while(statistical_computation && Nc_samples<N_samples)
    {
        if(spectral)
        {
            ml.set_angle(Degree(curr_angle));
            
            for(i=0;i<lambda.size();i++)
            {
                double w=m_to_rad_Hz(lambda[i]);
                
                ml.set_lambda(lambda[i]);
                ml.set_environment(superstrate_selector->get_n(w),
                                   substrate_selector->get_n(w));
                
                for(l=0;l<N_layers;l++)
                    ml.set_layer(l,layers_height_samples[Nc_samples][l],layers_material[l].get_n(w));
                
                ml.compute_power(tmp_R_TE,tmp_T_TE,tmp_A_TE,
                                 tmp_R_TM,tmp_T_TM,tmp_A_TM);
                
                R_TE_sum[i]+=tmp_R_TE;
                R_TM_sum[i]+=tmp_R_TM;
                
                T_TE_sum[i]+=tmp_T_TE;
                T_TM_sum[i]+=tmp_T_TM;
            }
        }
        else
        {
            double w=m_to_rad_Hz(curr_lambda);
            
            ml.set_lambda(curr_lambda);
            ml.set_environment(superstrate_selector->get_n(w),
                               substrate_selector->get_n(w));
            
            for(l=0;l<N_layers;l++)
                ml.set_layer(l,layers_height_samples[Nc_samples][l],layers_material[l].get_n(w));
            
            for(i=0;i<angle.size();i++)
            {
                ang_rad.degree(angle[i]);
                ml.set_angle(ang_rad);
                
                ml.compute_power(tmp_R_TE,tmp_T_TE,tmp_A_TE,
                                 tmp_R_TM,tmp_T_TM,tmp_A_TM);
                
                R_TE_sum[i]+=tmp_R_TE;
                R_TM_sum[i]+=tmp_R_TM;
                
                T_TE_sum[i]+=tmp_T_TE;
                T_TM_sum[i]+=tmp_T_TM;
            }
        }
        
        Nc_samples++;
        
        if(Nc_samples%std::max(1,N_samples/100)==0)
        {
            wxCommandEvent *event=new wxCommandEvent(EVT_REFRESH);
            
            wxQueueEvent(this,event);
        }
    }
    
    computation_mutex.Unlock();
    thread_running=false;
}

void MultilayerFrame::recompute_straight()
{
    int l;
    unsigned int i;
    Angle ang_rad;
    
    int Nl=0;
    int Nlb=layers_list->get_size();
    
    for(l=0;l<Nlb;l++)
    {
        LayerPanelBase *panel=layers_list->get_panel(l);
        Nl+=panel->get_n_layers();
    }
    
    ml.set_N_layers(Nl);
    
    layers_height.clear();
    layers_material.clear();
    
    layers_height.reserve(Nl);
    layers_material.reserve(Nl);
    
    for(l=0;l<Nlb;l++)
    {
        LayerPanelBase *panel=layers_list->get_panel(l);
        
        panel->get_heights(layers_height);
        panel->get_materials(layers_material);
    }
    
    unsigned int N_resize;
    
    if(spectral) N_resize=lambda.size();
    else N_resize=angle.size();
    
    R_TE.resize(N_resize);
    R_TM.resize(N_resize);
    R_avg.resize(N_resize);
    
    T_TE.resize(N_resize);
    T_TM.resize(N_resize);
    T_avg.resize(N_resize);
    
    A_TE.resize(N_resize);
    A_TM.resize(N_resize);
    A_avg.resize(N_resize);
    
    if(spectral)
    {
        ml.set_angle(Degree(curr_angle));
        
        for(i=0;i<lambda.size();i++)
        {
            double w=m_to_rad_Hz(lambda[i]);
            
            ml.set_lambda(lambda[i]);
            ml.set_environment(superstrate_selector->get_n(w),
                               substrate_selector->get_n(w));
            
            for(l=0;l<Nl;l++)
                ml.set_layer(l,layers_height[l],layers_material[l].get_n(w));
            
            ml.compute_power(R_TE[i],T_TE[i],A_TE[i],
                             R_TM[i],T_TM[i],A_TM[i]);
            
            R_avg[i]=0.5*(R_TE[i]+R_TM[i]);
            T_avg[i]=0.5*(T_TE[i]+T_TM[i]);
            A_avg[i]=0.5*(A_TE[i]+A_TM[i]);
            
//            if(std::isnan(R_TE[i]))
//            plog<<lambda[i]<<" "<<Nl<<" "<<layers_height[0]<<" "<<layers_material[0].get_n(w)<<" "<<superstrate_selector->get_n(w)<<" "<<substrate_selector->get_n(w)<<" "
//                                <<R_avg[i]<<" "<<T_avg[i]<<" "<<A_avg[i]<<" "
//                                <<R_TE[i]<<" "<<T_TE[i]<<" "<<A_TE[i]<<" "
//                                <<R_TM[i]<<" "<<T_TM[i]<<" "<<A_TM[i]<<" "<<std::endl;
        }
    }
    else
    {
        double w=m_to_rad_Hz(curr_lambda);
        
        ml.set_lambda(curr_lambda);
        ml.set_environment(superstrate_selector->get_n(w),
                           substrate_selector->get_n(w));
        
        for(l=0;l<Nl;l++)
            ml.set_layer(l,layers_height[l],layers_material[l].get_n(w));
        
        for(i=0;i<angle.size();i++)
        {
            ang_rad.degree(angle[i]);
            ml.set_angle(ang_rad);
            
            ml.compute_power(R_TE[i],T_TE[i],A_TE[i],
                             R_TM[i],T_TM[i],A_TM[i]);
            
            R_avg[i]=0.5*(R_TE[i]+R_TM[i]);
            T_avg[i]=0.5*(T_TE[i]+T_TM[i]);
            A_avg[i]=0.5*(A_TE[i]+A_TM[i]);
        }
    }
    
    graph->clear_graph();
    
    std::vector<double> *abscissa;
    
    if(spectral) abscissa=&lambda;
    else abscissa=&angle;
    
    double s=0.25;
    if(disp_Rs) graph->add_external_data(abscissa,&R_TE,0,0,s,"Rs");
    if(disp_Ts) graph->add_external_data(abscissa,&T_TE,0,s,0,"Ts");
    if(disp_As) graph->add_external_data(abscissa,&A_TE,s,0,0,"As");
    
    s=0.6;
    if(disp_Rp) graph->add_external_data(abscissa,&R_TM,0,0,s,"Rp");
    if(disp_Tp) graph->add_external_data(abscissa,&T_TM,0,s,0,"Tp");
    if(disp_Ap) graph->add_external_data(abscissa,&A_TM,s,0,0,"Ap");
    
    s=1.0;
    if(disp_R_avg) graph->add_external_data(abscissa,&R_avg,0,0,s,"R_avg");
    if(disp_T_avg) graph->add_external_data(abscissa,&T_avg,0,s,0,"T_avg");
    if(disp_A_avg) graph->add_external_data(abscissa,&A_avg,s,0,0,"A_avg");
    
    graph->set_scale((*abscissa)[0],(*abscissa)[abscissa->size()-1],-0.1,1.1);
    
    graph->Refresh();
}

void MultilayerFrame::rename_panels()
{
    for(unsigned int n=0;n<layers_list->get_size();n++)
    {
        wxString title=layers_list->get_panel(n)->get_base_name();
        title<<" "<<n;
        
        layers_list->get_panel(n)->set_title(title);
    }
}

void MultilayerFrame::update_aggregate_polar_ctrl()
{
    disp_Rs=polar_Rs_ctrl->GetValue();
    disp_Rp=polar_Rp_ctrl->GetValue();
    disp_R_avg=polar_R_avg_ctrl->GetValue();
    
    disp_Ts=polar_Ts_ctrl->GetValue();
    disp_Tp=polar_Tp_ctrl->GetValue();
    disp_T_avg=polar_T_avg_ctrl->GetValue();
    
    disp_As=polar_As_ctrl->GetValue();
    disp_Ap=polar_Ap_ctrl->GetValue();
    disp_A_avg=polar_A_avg_ctrl->GetValue();
    
    bool R_val=disp_Rs && disp_Rp && disp_R_avg;
    bool T_val=disp_Ts && disp_Tp && disp_T_avg;
    bool A_val=disp_As && disp_Ap && disp_A_avg;
    
    bool P_val=disp_Rp && disp_Tp && disp_Ap;
    bool S_val=disp_Rs && disp_Ts && disp_As;
    bool avg_val=disp_R_avg && disp_T_avg && disp_A_avg;
    
    polar_all_ctrl->SetValue(R_val && T_val && A_val);
    polar_R_ctrl->SetValue(R_val);
    polar_T_ctrl->SetValue(T_val);
    polar_A_ctrl->SetValue(A_val);
    polar_P_ctrl->SetValue(P_val);
    polar_S_ctrl->SetValue(S_val);
    polar_avg_ctrl->SetValue(avg_val);
}

void MultilayerFrame::save_project(wxFileName const &fname_)
{
    std::string fname=fname_.GetFullPath().ToStdString();
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    file<<"mode=gui_multilayer_mode()"<<std::endl;
    file<<"mode:angles("<<angle_ctrl->get_value()<<")"<<std::endl;
    file<<"mode:spectrum("<<spectrum->get_lambda_min()<<","<<spectrum->get_lambda_max()<<","<<spectrum->get_Np()<<")"<<std::endl;
    
    file<<"mode:superstrate("<<superstrate_selector->get_lua()<<")"<<std::endl<<std::endl;
    
    for(unsigned int i=0;i<layers_list->get_size();i++)
    {
        file<<"mode:"<<layers_list->get_panel(i)->get_lua_string()<<std::endl;
    }
    file<<std::endl;
    
    file<<"mode:substrate("<<substrate_selector->get_lua()<<")";
    
    file.close();
    
    SetTitle(wxString("Multilayer : ").Append(project_fname.GetName()));
}

void MultilayerFrame::switch_slider_angular()
{
    double scale;
    std::string unit;
    
    get_unit_u(spectrum->get_lambda_min(),"m",unit,scale);
    
    ctrl_slider->set_N(spectrum->get_Np());
    ctrl_slider->set_scale(scale);
    ctrl_slider->set_unit(unit);
    ctrl_slider->set_range(spectrum->get_lambda_min(),spectrum->get_lambda_max());
    ctrl_slider->set_value(curr_lambda);
    
    curr_lambda=ctrl_slider->get_value();
}

void MultilayerFrame::switch_slider_spectral()
{
    ctrl_slider->set_N(angle_ctrl->get_value());
    ctrl_slider->set_scale(1.0);
    ctrl_slider->set_unit("°");
    ctrl_slider->set_range(0.0,90.0);
    ctrl_slider->set_value(curr_angle);
    
    curr_angle=ctrl_slider->get_value();
}
