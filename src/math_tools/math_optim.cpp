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
     delta(1.0), limit_down(0), limit_up(0),
     operation_type(Operation::GROWTH),
     limit_type(Limit::NONE)
{
}

//#################
//   OptimEngine
//#################

void OptimEngine::evolve(double factor)
{
    for(unsigned int i=0;i<targets.size();i++)
    {
        double &val=*(targets[i]);
        OptimRule &rule=rules[i];
        
        previous_values[i]=val;
        
        if(!rule.lock)
        {
            double p=randp(-rule.delta,rule.delta)*factor;
            
                 if(rule.operation_type==OptimRule::Operation::ADD) val+=p;
            else if(rule.operation_type==OptimRule::Operation::GROWTH) val*=1.0+p;
            
                 if(rule.limit_type==OptimRule::Limit::UP) val=std::min(rule.limit_up,val);
            else if(rule.limit_type==OptimRule::Limit::DOWN) val=std::max(rule.limit_down,val);
            else if(rule.limit_type==OptimRule::Limit::BOTH) val=std::clamp(val,rule.limit_down,rule.limit_up);
        }
    }
}

void OptimEngine::forget_target(int *target)
{
}

void OptimEngine::forget_target(unsigned int *target)
{
}

void OptimEngine::forget_target(double *target)
{
    int i=locate_target(target);
    
    if(i==-1) return;
    
    std::vector<double*>::const_iterator it1=targets.begin()+i;
    std::vector<double>::const_iterator it2=previous_values.begin()+i;
    std::vector<OptimRule>::const_iterator it3=rules.begin()+i;
    
    targets.erase(it1);
    previous_values.erase(it2);
    rules.erase(it3);
}

void OptimEngine::forget_target(std::string *target)
{
}

bool OptimEngine::get_rule(int *target,OptimRule &rule)
{
    return false;
}

bool OptimEngine::get_rule(unsigned int *target,OptimRule &rule)
{
    return false;
}

bool OptimEngine::get_rule(double *target,OptimRule &rule)
{
    int i=locate_target(target);
    
    if(i==-1) return false;
    else
    {
        rule=rules[i];
        return true;
    }
}

bool OptimEngine::get_rule(std::string *target,OptimRule &rule)
{
    return false;
}

int OptimEngine::locate_target(double *target)
{
    for(int i=0;i<static_cast<int>(targets.size());i++)
        if(target==targets[i]) return i;
    
    return -1;
}

void OptimEngine::register_target(double *target,OptimRule const &rule)
{
    if(locate_target(target)!=-1) return;
    
    targets.push_back(target);
    previous_values.push_back(0);
    rules.push_back(rule);
}

void OptimEngine::revert_targets()
{
    for(unsigned int i=0;i<targets.size();i++)
    {
        if(!rules[i].lock) *(targets[i])=previous_values[i];
    }
}

void OptimEngine::set_rule(int *target,OptimRule const &rule)
{
}

void OptimEngine::set_rule(unsigned int *target,OptimRule const &rule)
{
}

void OptimEngine::set_rule(double *target,OptimRule const &rule)
{
    int i=locate_target(target);
    
    if(i!=-1) rules[i]=rule;
}

void OptimEngine::set_rule(std::string *target,OptimRule const &rule)
{
}

