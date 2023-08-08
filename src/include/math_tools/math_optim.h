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

class OptimTarget
{
    public:
        double weight;
        
        virtual double evaluate() const=0;
};

class OptimRule
{
    public:
        enum Operation { ADD, GROW };
        enum Limit { UP, DOWN, BOTH, NONE };
        
        bool lock;
        double delta_add,delta_grow,limit_down,limit_up;
        Operation operation_type;
        Limit limit_type;
        
        OptimRule();
        OptimRule(OptimRule const &rule) = default;
        
        OptimRule& operator = (OptimRule const &rule) = default;
};

class OptimEngine
{
    public:
        std::vector<double*> variables;
        std::vector<double> previous_values;
        std::vector<OptimRule> rules;
        
        void evolve(double factor);
        void forget_variable(double *target);
        bool get_rule(double *target,OptimRule &rule) const;
        int locate_variable(double *target) const;
        void register_variable(double *target,OptimRule const &rule);
        void revert_variables();
        void set_rule(double *target,OptimRule const &rule);
};

#endif // MATH_OPTIM_H_INCLUDED
