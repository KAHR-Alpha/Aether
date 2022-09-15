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

#ifndef SCRIPT_UTILS_H
#define SCRIPT_UTILS_H

#include <mathUT.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

enum
{
    CMD_ADD_FILE,
    CMD_ANGLE,
    CMD_SET_BASE,
    CMD_DECIMATE,
    CMD_FILE,
    CMD_IND_SUB_INDEX,
    CMD_IND_SUP_INDEX,
    CMD_IND_THICK,
    CMD_KX,
    CMD_MODE,
    CMD_OUTPUT,
    CMD_PREFIX,
    CMD_PARAMFILE,
    CMD_PHASE_COMP,
    CMD_POLARIZATION,
    CMD_SEPARATE,
    CMD_SETPML,
    CMD_SPECTRUM,
    CMD_SPECTRUM_DENS,
    CMD_SUBDIVIDE,
    CMD_UNKNOWN,
    CMD_WRKDIR
};

int get_cmdcode(std::string);

void script_add_file(std::stringstream &,std::vector<std::string> &);
void script_add_prefix(std::stringstream &,std::string &);
void script_enable_autocompletion(std::stringstream &,bool &,double &);
void script_geo_subdivide(std::stringstream &,Grid3<unsigned int>&,int&,int&,int&,int&,double&,double&,double&);
void script_set_Dx(std::stringstream &,double &);
void script_set_Dy(std::stringstream &,double &);
void script_set_Dz(std::stringstream &,double &);
void script_set_polarization(std::stringstream &,std::string &);
void script_set_slab_thickness(std::stringstream &,double &);
void script_set_spectrum(std::stringstream &,double &,double &);
void script_set_substrate_ind(std::stringstream &,double &);
void script_set_superstrate_ind(std::stringstream &,double &);
void script_set_timesteps(std::stringstream &,int &);
void script_set_working_dir(std::stringstream &,std::string &);

void script_add_file(std::string,std::vector<std::string> &);
void script_add_prefix(std::string,std::string &);
void script_enable_autocompletion(std::string,bool &,double &);
void script_geo_subdivide(std::string,Grid3<unsigned int>&,int&,int&,int&,int&,double&,double&,double&);
void script_set_Dx(std::string,double &);
void script_set_Dy(std::string,double &);
void script_set_Dz(std::string,double &);
void script_set_polarization(std::string,std::string &);
void script_set_slab_thickness(std::string,double &);
void script_set_spectrum(std::string,int&,double &,double &);
void script_set_substrate_ind(std::string,double &);
void script_set_superstrate_ind(std::string,double &);
void script_set_timesteps(std::string,int &);
void script_set_working_dir(std::string,std::string &);


class ScriptInstr
{
    public:
        std::string cmd;
        std::string options;
        std::stringstream options_strm;
        
        ScriptInstr();
        ScriptInstr(std::string const &cmd_i,std::string const &opt_i);
        ScriptInstr(ScriptInstr const &inst);
        
        template<typename T>
        bool get_options(T &target)
        {
            options_strm>>target;
            
            return options_strm.eof();
        }
        
        void show();
        void operator = (ScriptInstr const &inst);
};

class ScriptHandler
{
    private:
        bool ordered;
        int ord_curr_cmd;
        
        char cmd_sep;
        char instruc_sep;
        std::string base_string;
        
        int Ncmd,curr_instr;
        std::vector<ScriptInstr> instr_arr;
        std::vector<bool> instr_used;
        
        void analyze(std::string in_str);
        void clean_str(std::string &target_str);
        
    public:
        ScriptHandler();
        ScriptHandler(std::string bstring,char cmdS,char intS);
        ScriptHandler(ScriptHandler const &spth);
        
        void add_cmd(std::string n_cmd,std::string n_options);
        void clear();
        std::string get_options();
        
        template<typename T>
        void get_options(T &target)
        {
            bool c=instr_arr[curr_instr].get_options(target);
            if(c) { instr_used[curr_instr]=true; ord_curr_cmd+=1; }
        }
        
        bool has(std::string str_query);
        void load(std::string fname);
        std::vector<ScriptHandler> make_subscripts(std::string cmd);
        void print(std::string fname);
        void set(std::string bstring,char cmdS,char intS);
        void set_ordered(bool ord=true);
        void set_separators(char cmdS,char instS);
        void show();
        void show_unused();
        
        
        void operator = (ScriptHandler const &spth);
};

#endif // SCRIPT_UTILS_H
