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

#ifndef MATH_OPTIM_H_INCLUDED
#define MATH_OPTIM_H_INCLUDED

#include <mathUT.h>

class OptimRule
{
    public:
        enum Operation { ADD, GROWTH };
        enum Limit { UP, DOWN, BOTH, NONE };
        
        bool lock;
        double delta,limit_down,limit_up;
        Operation operation_type;
        Limit limit_type;
        
        OptimRule();
        OptimRule(OptimRule const &rule) = default;
        
        OptimRule& operator = (OptimRule const &rule) = default;
};

class OptimEngine
{
    public:
        std::vector<double*> targets;
        std::vector<double> previous_values;
        std::vector<OptimRule> rules;
        
        void evolve(double factor);
        void forget_target(int *target);
        void forget_target(unsigned int *target);
        void forget_target(double *target);
        void forget_target(std::string *target);
        bool get_rule(int *target,OptimRule &rule);
        bool get_rule(unsigned int *target,OptimRule &rule);
        bool get_rule(double *target,OptimRule &rule);
        bool get_rule(std::string *target,OptimRule &rule);
        int locate_target(double *target);
        void register_target(double *target,OptimRule const &rule);
        void revert_targets();
        void set_rule(int *target,OptimRule const &rule);
        void set_rule(unsigned int *target,OptimRule const &rule);
        void set_rule(double *target,OptimRule const &rule);
        void set_rule(std::string *target,OptimRule const &rule);
};

#endif // MATH_OPTIM_H_INCLUDED
