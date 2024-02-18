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

#ifndef MATH_SYM_H_INCLUDED
#define MATH_SYM_H_INCLUDED

#include <mathUT.h>
#include <list>

class SymNode;

enum class SymType
{
    FUNC,
    NUM,
    EXPR,
    VAR
};

enum class SymOp
{
    ADD,
    DIV,
    MULT,
    POW,
    SUBS
};

enum class SymFunc
{
    ACOS,
    ASIN,
    ATAN,
    COS,
    EXP,
    ID,
    SIN,
    TAN
};

class SymLib
{
    private:
        double evaluate_specials(bool &known,std::string const &var);
        
    public:
        std::vector<std::string> keys;
        std::vector<SymNode*> nodes;
        std::vector<bool> persistent;
        
        void add(std::string const &var,SymNode *node,bool persistent=false);
        double evaluate(std::string const &var);
        double evaluate(std::string const &var,std::list<SymNode*> &backtrace);
        void forget(std::string const &key,bool force=false);
        void forget(SymNode *node,bool force=false);
};

class SymNode
{
    private:
        int sign;
        SymType type;
        SymFunc func_type;

        double val;
        std::string var;
        
        std::string base_expression;
        
        SymLib *lib;
        
        std::vector<SymOp> op_arr;
        std::vector<SymNode*> nodes_arr;
        
        std::vector<bool> eval_flags;
        std::vector<double> eval_blocks;
        
        SymNode(std::string const &frm,
                SymType type,
                SymFunc func);
        
        void clean();
        void parse(std::string const &frm);
        void parse_block(std::string const &frm);
        
    public:
        SymNode(SymLib *lib=nullptr);
        SymNode(std::string const &frm,SymLib *lib=nullptr);
        
        ~SymNode();
        
        double evaluate();
        double evaluate(std::list<SymNode*> &backtrace);
        std::string get_expression();
        bool requires_one(std::string const &var);
        bool requires_any(std::vector<std::string> const &var);
        void set_expression(std::string const &frm);
        void set_lib(SymLib *lib);
};

bool is_add(char c);
bool is_numeric(char c);
bool is_mul(char c);
bool is_operator(char c);

#endif // MATH_SYM_H_INCLUDED
