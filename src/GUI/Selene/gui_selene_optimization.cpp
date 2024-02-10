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
#include <gui_selene_sensor.h>

namespace SelGUI
{
wxDEFINE_EVENT(EVT_REFRESH_GEOMETRY,wxCommandEvent);


//######################
//   OptimTargetPanel
//######################


OptimTargetPanel::OptimTargetPanel(wxWindow *parent,std::vector<std::string> const &sensor_names)
    :PanelsListBase(parent)
{
    wxPanel *panel=new wxPanel(this);
    
    wxBoxSizer *panel_sizer=new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(panel_sizer);
    
    panel_sizer->Add(new wxStaticText(panel,wxID_ANY,"Sensor: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    
    sensors=new wxChoice(panel,wxID_ANY);
    for(std::string const &name:sensor_names) sensors->Append(name);
    sensors->SetSelection(0);
        
    panel_sizer->Add(sensors);
    
    panel_sizer->Add(new wxStaticText(panel,wxID_ANY," Goal: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    
    goal=new wxChoice(panel,wxID_ANY);
    goal->Append("Maximize hit count");
    goal->Append("Minimize spatial dispersion");
    goal->Append("Minimize angular dispersion");
    goal->Append("Target hit count");
    goal->SetSelection(0);
    
    goal->Bind(wxEVT_CHOICE,&OptimTargetPanel::evt_goal,this);
    
    panel_sizer->Add(goal);
    
    target_value=new NamedTextCtrl<double>(panel," Target: ",0);
    target_value->Hide();
    panel_sizer->Add(target_value);
    
    weight=new NamedTextCtrl<double>(panel," Weight: ",1.0);
    panel_sizer->Add(weight);
    
    sizer->Add(panel);
}


void OptimTargetPanel::evt_goal(wxCommandEvent &event)
{
    if(goal->GetSelection()==3)
    {
        target_value->Show();
    }
    else target_value->Hide();
    
    Layout();
}


//########################
//   OptimizationDialog
//########################


OptimizationDialog::OptimizationDialog(std::vector<Sel::OptimTarget> &targets_,
                                       std::vector<Sel::Object*> const &sensors_)
    :wxDialog(0,wxID_ANY,"Optimization targets",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     targets(targets_),
     sensors(sensors_)
{
    for(Sel::Object *obj : sensors)
        sensor_names.push_back(obj->name);
        
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    targets_ctrl=new PanelsList<OptimTargetPanel>(this);
    
    for(Sel::OptimTarget target : targets)
    {
        
        bool found;
        std::size_t i=vector_locate(found,sensors,target.sensor);
        
        if(!found) continue;
            
        OptimTargetPanel *panel=targets_ctrl->add_panel<OptimTargetPanel>(sensor_names);
        
        panel->sensors->SetSelection(i);
        
        switch(target.goal)
        {
            case Sel::OptimGoal::MAXIMIZE_HIT_COUNT:
                panel->goal->SetSelection(0);
                break;
            case Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD:
                panel->goal->SetSelection(1);
                break;
            case Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD:
                panel->goal->SetSelection(2);
                break;
            case Sel::OptimGoal::TARGET_HIT_COUNT:
                panel->goal->SetSelection(3);
                break;
        }
        
        if(target.goal==Sel::OptimGoal::TARGET_HIT_COUNT)
        {
            panel->target_value->Show();
            panel->target_value->set_value(target.target_value);
        }
        
        panel->weight->set_value(target.weight);
    }
    
    sizer->Add(targets_ctrl,wxSizerFlags(1).Expand());
    
    wxButton *add_target_btn=new wxButton(this,wxID_ANY,"Add Target");
    add_target_btn->Bind(wxEVT_BUTTON,&OptimizationDialog::evt_add_target,this);
    
    sizer->Add(add_target_btn,wxSizerFlags().Border(wxALL,3));
    
    Bind(wxEVT_CLOSE_WINDOW,&OptimizationDialog::evt_close,this);
    
    SetSizer(sizer);
    
    Layout();
    ShowModal();
}

void OptimizationDialog::evt_add_target(wxCommandEvent &event)
{
    targets_ctrl->add_panel<OptimTargetPanel>(sensor_names);
    
    Layout();
}

void OptimizationDialog::evt_close(wxCloseEvent &event)
{
    targets.resize(targets_ctrl->get_size());
    
    for(std::size_t i=0;i<targets.size();i++)
    {
        OptimTargetPanel *panel=targets_ctrl->get_panel(i);
        
        int sensor_ID=panel->sensors->GetSelection();
        int treatment=panel->goal->GetSelection();
        
        targets[i].sensor=sensors[sensor_ID];
        
        switch(treatment)
        {
            case 0:
                targets[i].goal=Sel::OptimGoal::MAXIMIZE_HIT_COUNT;
                break;
            case 1:
                targets[i].goal=Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD;
                break;
            case 2:
                targets[i].goal=Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD;
                break;
            case 3:
                targets[i].goal=Sel::OptimGoal::TARGET_HIT_COUNT;
                break;
        }
        
        targets[i].target_value=panel->target_value->get_value();
        targets[i].weight=panel->weight->get_value();
    }
    
    Destroy();
}


//#################
//   SeleneFrame
//#################


void SeleneFrame::optimization_trace()
{
    bool first_run=true;
    double best_score=std::numeric_limits<double>::max();
    
    std::chrono::time_point<std::chrono::high_resolution_clock> start,end;
    
    start=std::chrono::high_resolution_clock::now();
    
    optim_engine.clear_targets();
    
    for(Sel::OptimTarget &target : optimization_targets)
    {
        target.sensor->sens_ray_obj_intersection=true;
        target.sensor->sens_ray_obj_direction=true;
        target.sensor->sens_ray_obj_face=true;

        optim_engine.add_target(&target);
    }
    
    while(optimization_running)
    {
        if(!first_run) optim_engine.evolve(1.0);
            
        std::cout<<"Best score: "<<best_score<<std::endl;
        Sel::Selene selene;
            
        for(std::size_t i=0;i<frames.size();i++)
        {
            Sel::Object *object=dynamic_cast<Sel::Object*>(frames[i]);
            Sel::Light *light=dynamic_cast<Sel::Light*>(frames[i]);
            
            if(object!=nullptr)
            {
                object->update_geometry();
                selene.add_object(object);
            }
            else if(light!=nullptr) selene.add_light(light);
        }
        
        selene.set_output_directory(output_directory_std);
        
        selene.render(nr_disp->get_value(),
                      nr_tot->get_value());
        
        // Score evaluation
        
        double current_score=optim_engine.evaluate_targets();
        
        if(current_score<best_score)
        {
            best_score=current_score;
            
            for(std::size_t i=0;i<Nrays;i++)
            {
                if(i<selene.xs_ftc.size())
                {
                    rays_x1[i]=selene.xs_ftc[i]; rays_x2[i]=selene.xe_ftc[i];
                    rays_y1[i]=selene.ys_ftc[i]; rays_y2[i]=selene.ye_ftc[i];
                    rays_z1[i]=selene.zs_ftc[i]; rays_z2[i]=selene.ze_ftc[i];
                    
                    rays_gen[i]=selene.gen_ftc[i];
                    rays_lambda[i]=selene.lambda_ftc[i];
                    rays_lost[i]=selene.lost_ftc[i];
                }
                else
                {
                    rays_x1[i]=0; rays_x2[i]=0;
                    rays_y1[i]=0; rays_y2[i]=0;
                    rays_z1[i]=0; rays_z2[i]=0;
                    
                    rays_gen[i]=0;
                    rays_lambda[i]=0;
                    rays_lost[i]=0;
                }
            }
        }
        else
        {
            optim_engine.revert_variables();
        }
        
        // Graphical updates
        
        end=std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> d=end-start;
        
        if(d>std::chrono::milliseconds(250))
        {
            start=end;
            
            wxCommandEvent event(EVT_REFRESH_GEOMETRY);
            wxPostEvent(this,event);
            
            pause_optimization=true;
        }
        
        while(optimization_running && pause_optimization)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        first_run=false;
    }
    
    optim_engine.revert_variables();
}

}
