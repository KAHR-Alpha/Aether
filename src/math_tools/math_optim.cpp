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

#include <algorithm>
#include <math_optim.h>

OptimRule::OptimRule()
    :lock(false), 
     delta_add(1.0), delta_grow(0.05), limit_down(0), limit_up(0),
     operation_type(Operation::GROW),
     limit_type(Limit::NONE)
{
}

//#################
//   OptimEngine
//#################

void OptimEngine::add_target(OptimTarget *target)
{
    targets.push_back(target);
}


void OptimEngine::clear_targets()
{
    targets.clear();
}


double OptimEngine::evaluate_targets()
{
    double score=0;
    
    for(OptimTarget *target : targets)
    {
        score+=target->evaluate();
    }
    
    return score;
}


void OptimEngine::evolve(double factor)
{
    for(unsigned int i=0;i<variables.size();i++)
    {
        double &val=*(variables[i]);
        OptimRule &rule=rules[i];
        
        previous_values[i]=val;
        
        if(!rule.lock)
        {
            if(rule.operation_type==OptimRule::Operation::ADD)
            {
                double p=randp(-rule.delta_add,rule.delta_add)*factor;
                val+=p;
            }
            else if(rule.operation_type==OptimRule::Operation::GROW)
            {
                double p=randp(-rule.delta_grow,rule.delta_grow)*factor;
                val*=1.0+p;
            }
            
                 if(rule.limit_type==OptimRule::Limit::UP) val=std::min(rule.limit_up,val);
            else if(rule.limit_type==OptimRule::Limit::DOWN) val=std::max(rule.limit_down,val);
            else if(rule.limit_type==OptimRule::Limit::BOTH) val=std::clamp(val,rule.limit_down,rule.limit_up);
        }
    }
}


void OptimEngine::forget_variable(double *target)
{
    int i=locate_variable(target);
    
    if(i==-1) return;
    
    std::vector<double*>::const_iterator it1=variables.begin()+i;
    std::vector<double>::const_iterator it2=previous_values.begin()+i;
    std::vector<OptimRule>::const_iterator it3=rules.begin()+i;
    
    variables.erase(it1);
    previous_values.erase(it2);
    rules.erase(it3);
}


bool OptimEngine::get_rule(double *target,OptimRule &rule) const
{
    int i=locate_variable(target);
    
    if(i==-1) return false;
    else
    {
        rule=rules[i];
        return true;
    }
}


int OptimEngine::locate_variable(double *target) const
{
    for(int i=0;i<static_cast<int>(variables.size());i++)
        if(target==variables[i]) return i;
    
    return -1;
}


void OptimEngine::register_variable(double *target,OptimRule const &rule)
{
    int target_index=locate_variable(target);
    
    if(target_index!=-1) 
    {
        rules[target_index]=rule;
    }
    else
    {
        variables.push_back(target);
        previous_values.push_back(0);
        rules.push_back(rule);
    }
}


void OptimEngine::revert_variables()
{
    for(unsigned int i=0;i<variables.size();i++)
    {
        if(!rules[i].lock) *(variables[i])=previous_values[i];
    }
}


void OptimEngine::set_rule(double *target,OptimRule const &rule)
{
    int i=locate_variable(target);
    
    if(i!=-1) rules[i]=rule;
}
