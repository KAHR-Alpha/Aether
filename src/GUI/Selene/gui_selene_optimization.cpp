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

OptimTargetPanel::OptimTargetPanel(wxWindow *parent,std::vector<std::string> const &sensor_names)
    :PanelsListBase(parent)
{
    wxPanel *panel=new wxPanel(this);
    
    wxBoxSizer *panel_sizer=new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(panel_sizer);
    
    panel_sizer->Add(new wxStaticText(panel,wxID_ANY,"Sensor: "));
    
    sensors=new wxChoice(panel,wxID_ANY);
    for(std::string const &name:sensor_names) sensors->Append(name);
    sensors->SetSelection(0);
        
    panel_sizer->Add(sensors);
    
    panel_sizer->Add(new wxStaticText(panel,wxID_ANY," Treatment: "));
    
    treatment=new wxChoice(panel,wxID_ANY);
    treatment->Append("Minimize spatial dispersion");
    treatment->Append("Minimize angular dispersion");
    treatment->SetSelection(0);
    
    panel_sizer->Add(treatment);
    
    weight=new NamedTextCtrl<double>(panel," weight: ",1.0);
    
    panel_sizer->Add(weight);
    
    sizer->Add(panel);
}

OptimizationDialog::OptimizationDialog(std::vector<OptimTarget> &targets_,
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
    
    for(OptimTarget target : targets)
    {
        
        bool found;
        std::size_t i=vector_locate(found,sensors,target.sensor);
        
        if(!found) continue;
            
        OptimTargetPanel *panel=targets_ctrl->add_panel<OptimTargetPanel>(sensor_names);
        
        panel->sensors->SetSelection(i);
        
        switch(target.treatment)
        {
            case OptimTreatment::MINIMIZE_SPATIAL_SPREAD:
                panel->treatment->SetSelection(0);
                break;
            case OptimTreatment::MINIMIZE_DIRECTION_SPREAD:
                panel->treatment->SetSelection(1);
                break;
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
        int treatment=panel->treatment->GetSelection();
        
        targets[i].sensor=sensors[sensor_ID];
        
        switch(treatment)
        {
            case 0:
                targets[i].treatment=OptimTreatment::MINIMIZE_SPATIAL_SPREAD;
                break;
            case 1:
                targets[i].treatment=OptimTreatment::MINIMIZE_DIRECTION_SPREAD;
                break;
        }
        
        targets[i].weight=panel->weight->get_value();
    }
    
    Destroy();
}

void SeleneFrame::optimization_trace()
{
    bool first_run=true;
    double best_score=std::numeric_limits<double>::max();
    
    std::chrono::time_point<std::chrono::high_resolution_clock> start,end;
    
    start=std::chrono::high_resolution_clock::now();
    
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
        
        double current_score=0;
        
        for(OptimTarget const &target : optimization_targets)
        {
            RayCounter counter;
            counter.set_sensor(target.sensor);
            
            if(target.treatment==OptimTreatment::MINIMIZE_SPATIAL_SPREAD)
            {
                current_score+=counter.compute_spatial_spread();
            }
            else
            {
                current_score+=counter.compute_angular_spread();
            }
        }
        
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
            optim_engine.revert_targets();
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
    
    optim_engine.revert_targets();
}

}
