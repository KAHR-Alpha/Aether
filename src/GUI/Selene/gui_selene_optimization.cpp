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
    treatment->Append("Minimize direction dispersion");
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
    double best_score=std::numeric_limits<double>::max();
    
    while(optimization_running)
    {
        optim_engine.evolve(1.0);
            
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
        
        double current_score=0;
        
        for(std::size_t i=0;i<optimization_targets.size();i++)
        {
            RayCounter counter;
            counter.set_sensor(optimization_targets[i].sensor);
            current_score=counter.compute_spatial_spread();
        }
        
        if(current_score<best_score)
        {
            best_score=current_score;
            
            for(std::size_t i=0;i<frames.size();i++)
                update_vao_location(frames_vao[i],frames[i]);
        }
        else
        {
            optim_engine.revert_targets();
        }
        
        gl->set_rays(selene.xs_ftc,selene.xe_ftc,
                     selene.ys_ftc,selene.ye_ftc,
                     selene.zs_ftc,selene.ze_ftc,
                     selene.gen_ftc,selene.lambda_ftc,
                     selene.lost_ftc);
    }
}

}
