/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <enum_tools.h>
#include <logger.h>
#include <math_sym.h>

bool is_add(char c)
{
    if(c=='+') return true;
    if(c=='-') return true;
    
    return false;
}

bool is_numeric(char c)
{
    if(c=='0') return true;
    if(c=='1') return true;
    if(c=='2') return true;
    if(c=='3') return true;
    if(c=='4') return true;
    if(c=='5') return true;
    if(c=='6') return true;
    if(c=='7') return true;
    if(c=='8') return true;
    if(c=='9') return true;
    if(c=='.') return true;
    
    return false;
}

bool is_mul(char c)
{
    if(c=='*') return true;
    if(c=='/') return true;
    
    return false;
}

bool is_operator(char c)
{
    if(c=='+') return true;
    if(c=='-') return true;
    if(c=='*') return true;
    if(c=='/') return true;
    if(c=='^') return true;
    
    return false;
}

//###############
//    SymLib
//###############

void SymLib::add(std::string const &var,SymNode *node,bool persistent_)
{
    bool check=true;
    
    while(check)
    {
        int pos=-1;
        
        for(unsigned int i=0;i<keys.size();i++)
        {
            if(keys[i]==var)
            {
                pos=i;
                break;
            }
        }
        
        if(pos<0)
        {
            check=false;
            break;
        }
        
        #ifndef LINUX_ITERATOR_TWEAK
        std::vector<std::string>::const_iterator it_keys=keys.begin()+pos;
        std::vector<SymNode*>::const_iterator it_nodes=nodes.begin()+pos;
        std::vector<bool>::const_iterator it_lock=persistent.begin()+pos;
        #else
        std::vector<std::string>::iterator it_keys=keys.begin()+pos;
        std::vector<SymNode*>::iterator it_nodes=nodes.begin()+pos;
        std::vector<bool>::iterator it_lock=persistent.begin()+pos;
        #endif
        
        keys.erase(it_keys);
        nodes.erase(it_nodes);
        persistent.erase(it_lock);
    }
    
    keys.push_back(var);
    nodes.push_back(node);
    persistent.push_back(persistent_);
}

double SymLib::evaluate(std::string const &var)
{
    bool known_special=false;
    double special_val=evaluate_specials(known_special,var);
    
    if(known_special) return special_val;
    
    // Lib variables
    
    std::list<SymNode*> backtrace;
    
    for(unsigned int i=0;i<keys.size();i++)
    {
        if(var==keys[i])
        {
            double out=nodes[i]->evaluate(backtrace);
            backtrace.push_back(nodes[i]);
            return out;
        }
    }
    
    return 0;
}

double SymLib::evaluate(std::string const &var,std::list<SymNode*> &backtrace)
{
    bool known_special=false;
    double special_val=evaluate_specials(known_special,var);
    
    if(known_special) return special_val;
    
    // Lib variables
    
    for(unsigned int i=0;i<keys.size();i++)
    {
        if(var==keys[i])
        {
            double out=nodes[i]->evaluate(backtrace);
            backtrace.push_back(nodes[i]);
            return out;
        }
    }
    
    return 0;
}


double SymLib::evaluate_specials(bool &known,std::string const &var)
{
    if(var=="pi")
    {
        known=true;
        return Pi;
    }
    
    known=false;
    return 0;
}


void SymLib::forget(std::string const &key,bool force)
{
    int pos=-1;
    
    for(unsigned int i=0;i<nodes.size();i++)
    {
        if(keys[i]==key)
        {
            pos=i;
            break;
        }
    }
    
    if(pos<0) return;
    if(!force && persistent[pos]) return;
    
    keys.erase(keys.begin()+pos);
    nodes.erase(nodes.begin()+pos);
    persistent.erase(persistent.begin()+pos);
}


void SymLib::forget(SymNode *node,bool force)
{
    int pos=-1;
    
    for(unsigned int i=0;i<nodes.size();i++)
    {
        if(nodes[i]==node)
        {
            pos=i;
            break;
        }
    }
    
    if(pos<0) return;
    if(!force && persistent[pos]) return;
    
    #ifndef LINUX_ITERATOR_TWEAK
    std::vector<std::string>::const_iterator it_keys=keys.begin()+pos;
    std::vector<SymNode*>::const_iterator it_nodes=nodes.begin()+pos;
    std::vector<bool>::const_iterator it_lock=persistent.begin()+pos;
    #else
    std::vector<std::string>::iterator it_keys=keys.begin()+pos;
    std::vector<SymNode*>::iterator it_nodes=nodes.begin()+pos;
    std::vector<bool>::iterator it_lock=persistent.begin()+pos;
    #endif
        
    keys.erase(it_keys);
    nodes.erase(it_nodes);
    persistent.erase(it_lock);
}

//###############
//    SymNode
//###############

SymNode::SymNode(SymLib *lib_)
    :sign(1), type(SymType::EXPR),
     val(0), var(""),
     base_expression(""),
     lib(lib_)
{
}

SymNode::SymNode(std::string const &frm,SymLib *lib_)
    :sign(1), type(SymType::EXPR),
     val(0), var(""),
     base_expression(frm),
     lib(lib_)
{
    if(frm.find('=')==std::string::npos) parse(frm);
    else
    {
        int Ne=frm.find('=');
        
        std::string a_var=frm.substr(0,Ne);
        std::string a_frm=frm.substr(Ne+1);
        
        if(lib!=nullptr) lib->add(a_var,this);
        
        parse(a_frm);
    }
}

SymNode::SymNode(std::string const &frm,
                 SymType type_,
                 SymFunc func_)
    :sign(1), type(type_), func_type(func_),
     val(0), var(""),
     base_expression(frm),
     lib(nullptr)
{
    parse_block(frm);
}

SymNode::~SymNode()
{
    clean();
    if(lib!=nullptr) lib->forget(this,true);
}

void SymNode::clean()
{
    sign=1;
    type=SymType::EXPR;
    val=0;
    var="";
    
    for(unsigned int i=0;i<nodes_arr.size();i++) delete nodes_arr[i];
    
    op_arr.clear();
    nodes_arr.clear();
    eval_flags.clear();
    eval_blocks.clear();
    
    if(lib!=nullptr) lib->forget(this);
}

double SymNode::evaluate()
{
         if(type==SymType::NUM) return val;
    else if(type==SymType::VAR)
    {
        if(lib!=nullptr)
            return sign*lib->evaluate(var);
    }
    else if(type==AnyOf(SymType::EXPR,SymType::FUNC))
    {
        int i;
        
        int N_op=op_arr.size();
        int N_blocks=nodes_arr.size();
        
        for(i=0;i<N_blocks;i++)
        {
            eval_blocks[i]=nodes_arr[i]->evaluate();
            eval_flags[i]=false;
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::POW)
            {
                eval_blocks[i+1]=std::pow(eval_blocks[i],eval_blocks[i+1]);
                eval_flags[i]=true;
            }
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::MULT)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]*eval_blocks[p];
                eval_flags[i]=true;
            }
            else if(op_arr[i]==SymOp::DIV)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]/eval_blocks[p];
                eval_flags[i]=true;
            }
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::ADD)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]+eval_blocks[p];
            }
            else if(op_arr[i]==SymOp::SUBS)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]-eval_blocks[p];
            }
        }
        
        double expr_val=sign*eval_blocks[N_blocks-1];

        if(type==SymType::EXPR) return expr_val;
        else
        {
            switch(func_type)
            {
                case SymFunc::ACOS: return std::acos(expr_val);
                case SymFunc::ASIN: return std::asin(expr_val);
                case SymFunc::ATAN: return std::atan(expr_val);
                case SymFunc::COS: return std::cos(expr_val);
                case SymFunc::EXP: return std::exp(expr_val);
                case SymFunc::SIN: return std::sin(expr_val);
                case SymFunc::TAN: return std::tan(expr_val);
            }
        }
    }
    
    return 0;
}

double SymNode::evaluate(std::list<SymNode*> &backtrace)
{
    std::list<SymNode*>::const_iterator it;
    
    for(it=backtrace.begin();it!=backtrace.end();it++)
        if(*it==this)
        {
            Plog::print("Bump\n");
            chk_var(this);
            return 0;
        }
    
         if(type==SymType::NUM) return val;
    else if(type==SymType::VAR)
    {
        if(lib!=nullptr)
            return sign*lib->evaluate(var,backtrace);
    }
    else if(type==AnyOf(SymType::EXPR,SymType::FUNC))
    {
        int i;
        
        int N_op=op_arr.size();
        int N_blocks=nodes_arr.size();
        
        for(i=0;i<N_blocks;i++)
        {
            eval_blocks[i]=nodes_arr[i]->evaluate(backtrace);
            eval_flags[i]=false;
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::POW)
            {
                eval_blocks[i+1]=std::pow(eval_blocks[i],eval_blocks[i+1]);
                eval_flags[i]=true;
            }
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::MULT)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]*eval_blocks[p];
                eval_flags[i]=true;
            }
            else if(op_arr[i]==SymOp::DIV)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]/eval_blocks[p];
                eval_flags[i]=true;
            }
        }
        
        for(i=0;i<N_op;i++)
        {
                 if(op_arr[i]==SymOp::ADD)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]+eval_blocks[p];
            }
            else if(op_arr[i]==SymOp::SUBS)
            {
                int p=i+1;
                while(eval_flags[p]) p++;
                
                eval_blocks[p]=eval_blocks[i]-eval_blocks[p];
            }
        }
        
        double expr_val=sign*eval_blocks[N_blocks-1];

        if(type==SymType::EXPR) return expr_val;
        else
        {
            switch(func_type)
            {
                case SymFunc::ACOS: return std::acos(expr_val);
                case SymFunc::ASIN: return std::asin(expr_val);
                case SymFunc::ATAN: return std::atan(expr_val);
                case SymFunc::COS: return std::cos(expr_val);
                case SymFunc::EXP: return std::exp(expr_val);
                case SymFunc::SIN: return std::sin(expr_val);
                case SymFunc::TAN: return std::tan(expr_val);
            }
        }
    }
    
    return 0;
}

std::string SymNode::get_expression() { return base_expression; }

void SymNode::parse(std::string const &frm)
{
    int i;
    
    int N=frm.size();
    
    std::vector<int> block_start, block_end;
    std::vector<SymType> block_type;
    std::vector<SymFunc> block_func;
    
    int p=0;
    int curr_block=0;
    SymType curr_type=SymType::NUM;
    
    block_start.push_back(p);
    
    while(p<N)
    {
        //Checking the type at the start of a new block
        
        if(p==block_start[curr_block])
        {
            int func_offset=0;
            SymFunc curr_func=SymFunc::ID;

            //Checking for sign
            if(is_add(frm[p])) p++;
            
                 if(is_numeric(frm[p])) curr_type=SymType::NUM;
            else if(frm[p]=='(') curr_type=SymType::EXPR;
            else
            {
                for(int p2=p+1;p2<N;p2++)
                {
                    if(frm[p2]=='(')
                    {
                        curr_type=SymType::FUNC;
                        func_offset=p2-p;
                        break;
                    }
                    else if(is_operator(frm[p2]))
                    {
                        break;
                    }
                }

                if(curr_type!=SymType::FUNC)
                {
                    curr_type = SymType::VAR;
                }
            }

            if(curr_type==SymType::FUNC) // Figuring out the function
            {
                std::string func_str=frm.substr(p,func_offset);

                     if(func_str=="acos") curr_func=SymFunc::ACOS;
                else if(func_str=="asin") curr_func=SymFunc::ASIN;
                else if(func_str=="atan") curr_func=SymFunc::ATAN;
                else if(func_str=="exp") curr_func=SymFunc::EXP;
                else if(func_str=="cos") curr_func=SymFunc::COS;
                else if(func_str=="sin") curr_func=SymFunc::SIN;
                else if(func_str=="tan") curr_func=SymFunc::TAN;

                p=p+func_offset; // Jumping to opening '('
                block_start[block_start.size()-1]=p;
            }
            
            if(curr_type==AnyOf(SymType::EXPR,SymType::FUNC)) //Jumping to closing ')'
            {
                int p_counter=1;
                
                while(p_counter>0 && p<N)
                {
                    p++;
                    
                    if(frm[p]=='(') p_counter++;
                    if(frm[p]==')') p_counter--;
                }
            }
            
            block_type.push_back(curr_type);
            block_func.push_back(curr_func);
        }
        
        if(is_operator(frm[p]))
        {
            //Taking potential exponents into account
            if(curr_type==SymType::NUM && (frm[p-1]=='e' || frm[p-1]=='E')) p++;
            else
            {
                block_end.push_back(p-1);
                block_start.push_back(p+1);
                
                     if(frm[p]=='+') op_arr.push_back(SymOp::ADD);
                else if(frm[p]=='-') op_arr.push_back(SymOp::SUBS);
                else if(frm[p]=='*') op_arr.push_back(SymOp::MULT);
                else if(frm[p]=='^') op_arr.push_back(SymOp::POW);
                else if(frm[p]=='/') op_arr.push_back(SymOp::DIV);
                else op_arr.push_back(SymOp::ADD);
                
                curr_block++;
            }
        }
        
        p++;
    }
    
    block_end.push_back(p-1);
    
    int N_blocks=block_start.size();
    
    
    eval_flags.resize(N_blocks);
    eval_blocks.resize(N_blocks);
    
    nodes_arr.resize(N_blocks);
    
    for(i=0;i<N_blocks;i++)
    {
        std::string sub_frm=frm.substr(block_start[i],block_end[i]-block_start[i]+1);
        nodes_arr[i]=new SymNode(sub_frm,block_type[i],block_func[i]);
        nodes_arr[i]->set_lib(lib);
    }
}

void SymNode::parse_block(std::string const &frm)
{
         if(type==SymType::NUM)
    {
        val=std::stod(frm);
    }
    else if(type==AnyOf(SymType::EXPR,SymType::FUNC))
    {
        int N=frm.size();
        
        if(frm[0]=='-')
        {
            sign=-1;
            
            parse(frm.substr(2,N-3));
        }
        else parse(frm.substr(1,N-2));
    }
    else if(type==SymType::VAR)
    {
        if(frm[0]=='-')
        {
            sign=-1;
            var=frm.substr(1);
        }
        else var=frm;
    }
}

bool SymNode::requires_one(std::string const &var_)
{
         if(type==SymType::VAR && var==var_) return true;
    else if(type==SymType::EXPR)
    {
        for(unsigned int i=0;i<nodes_arr.size();i++)
            if(nodes_arr[i]->requires_one(var_)) return true;
    }
        
    return false;
}

bool SymNode::requires_any(std::vector<std::string> const &var_)
{
    for(unsigned int i=0;i<var_.size();i++)
        if(requires_one(var_[i])) return true;
        
    return false;
}

void SymNode::set_expression(std::string const &frm)
{
    clean();
    
    base_expression=frm;
    
    if(frm.find('=')==std::string::npos) parse(frm);
    else
    {
        int Ne=frm.find('=');
        
        std::string a_var=frm.substr(0,Ne);
        std::string a_frm=frm.substr(Ne+1);
        
        if(lib!=nullptr)
        {
            lib->forget(this);
            lib->add(a_var,this);
        }
        
        parse(a_frm);
    }
}

void SymNode::set_lib(SymLib *lib_)
{
    lib=lib_;
    for(unsigned int i=0;i<nodes_arr.size();i++)
        nodes_arr[i]->set_lib(lib);
}
