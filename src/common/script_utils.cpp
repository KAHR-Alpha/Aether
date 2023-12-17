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

#include <script_utils.h>
#include<string_tools.h>

int get_cmdcode(std::string K)
{
    if(K=="MODE") return CMD_MODE;
    else if(K=="add_file") return CMD_ADD_FILE;
    else if(K=="geo_subdivide") return CMD_SUBDIVIDE;
    else if(K=="index_thickness") return CMD_IND_THICK;
    else if(K=="index_sub_index") return CMD_IND_SUB_INDEX;
    else if(K=="index_sup_index") return CMD_IND_SUP_INDEX;
    else if(K=="separate_data") return CMD_SEPARATE;
    else if(K=="set_angle") return CMD_ANGLE;
    else if(K=="set_base") return CMD_SET_BASE;
    else if(K=="set_directory") return CMD_WRKDIR;
    else if(K=="set_file") return CMD_FILE;
    else if(K=="set_kx") return CMD_KX;
    else if(K=="set_N_wavelengths") return CMD_SPECTRUM_DENS;
    else if(K=="set_output") return CMD_OUTPUT;
    else if(K=="set_param_file") return CMD_PARAMFILE;
    else if(K=="set_phase") return CMD_PHASE_COMP;
    else if(K=="set_pml") return CMD_SETPML;
    else if(K=="set_polarization") return CMD_POLARIZATION;
    else if(K=="set_prefix") return CMD_PREFIX;
    else if(K=="set_spectrum") return CMD_SPECTRUM;
    else if(K=="set_working_dir") return CMD_WRKDIR;
    else if(K=="time_decimate") return CMD_DECIMATE;
    else
    {
        return CMD_UNKNOWN;
    }
}

void script_add_file(std::stringstream &strm,std::vector<std::string> &data_fname)
{
    std::string tfname;
    strm>>tfname;    
    data_fname.push_back(tfname);
}

void script_add_prefix(std::stringstream &strm,std::string &stro)
{
    std::cout<<"Adding a new prefix: ";
    strm>>stro;
    std::cout<<stro<<std::endl;
}

void script_enable_autocompletion(std::stringstream &strm,bool &auto_complete,double &completion_coeff)
{
    auto_complete=true;
    strm>>completion_coeff;
    std::cout<<"Enabling autocompletion at level "<<completion_coeff<<std::endl;
}

void script_set_Dx(std::stringstream &strm,double &Dx)
{
    std::cout<<"Changing Dx from "<<add_unit_u(Dx)<<" to ";
    strm>>Dx;
    std::cout<<add_unit_u(Dx)<<std::endl;
}

void script_set_Dy(std::stringstream &strm,double &Dy)
{
    std::cout<<"Changing Dy from "<<add_unit_u(Dy)<<" to ";
    strm>>Dy;
    std::cout<<add_unit_u(Dy)<<std::endl;
}

void script_set_Dz(std::stringstream &strm,double &Dz)
{
    std::cout<<"Changing Dz from "<<add_unit_u(Dz)<<" to ";
    strm>>Dz;
    std::cout<<add_unit_u(Dz)<<std::endl;
}

void script_geo_subdivide(std::stringstream &strm,Grid3<unsigned int> &matgrid,int &Nx,int &Ny,int &Nz,int &Nt,double &Dx,double &Dy,double &Dz)
{
    int factorX=1,factorY=1,factorZ=1;
    
    strm>>factorX;
    strm>>factorY;
    strm>>factorZ;
    
    if(factorX<1) factorX=1;
    if(factorY<1) factorY=1;
    if(factorZ<1) factorZ=1;
    
    Grid3<unsigned int> tmp_grid=matgrid;
    
    matgrid.init(factorX*Nx,factorY*Ny,factorZ*Nz,0);
    
    int i,j,k;
    int l,m,n;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        int ib=i*factorX;
        int jb=j*factorY;
        int kb=k*factorZ;
        
        int ic=(i+1)*factorX;
        int jc=(j+1)*factorY;
        int kc=(k+1)*factorZ;
        
        for(l=ib;l<ic;l++){ for(m=jb;m<jc;m++){ for(n=kb;n<kc;n++)
        {
            matgrid(l,m,n)=tmp_grid(i,j,k);
        }}}
    }}}
    
    Nx*=factorX; Dx/=factorX;
    Ny*=factorY; Dy/=factorY;
    Nz*=factorZ; Dz/=factorZ;
    
    Nt*=factorZ;
}

void script_set_polarization(std::stringstream &strm,std::string &polar_mode)
{
    strm>>polar_mode;
    std::cout<<"Setting the polarisation mode to "<<polar_mode<<std::endl;
}

void script_set_slab_thickness(std::stringstream &strm,double &hslab)
{
    strm>>hslab;
    std::cout<<"Setting slab thickness to "<<add_unit_u(hslab)<<std::endl;
}

void script_set_spectrum(std::stringstream &strm,double &li,double &lf)
{
    std::cout<<"Changing the covered spectrum from ["<<add_unit_u(li)<<" ; "<<add_unit_u(lf)<<"] to [";
    strm>>li;
    strm>>lf;
    std::cout<<add_unit_u(li)<<" ; "<<add_unit_u(lf)<<"]"<<std::endl;
}

void script_set_substrate_ind(std::stringstream &strm,double &n3)
{
    std::cout<<"Setting substrate index from "<<n3<<" to ";
    strm>>n3;
    std::cout<<n3<<std::endl;
}

void script_set_superstrate_ind(std::stringstream &strm,double &n1)
{
    std::cout<<"Setting superstrate index from "<<n1<<" to ";
    strm>>n1;
    std::cout<<n1<<std::endl;
}

void script_set_timesteps(std::stringstream &strm,int &Nt)
{
    strm>>Nt;
    std::cout<<"Setting the number of timesteps to "<<Nt<<std::endl;
}

void script_set_working_dir(std::stringstream &strm,std::string &working_dir)
{
    std::string tmp_str;
    strm>>tmp_str;
    char end_char=tmp_str[tmp_str.size()-1];
    if(end_char!='/' || end_char!='\\') tmp_str.append("/");
    
    working_dir=tmp_str;
}

/// DUPLI


void script_add_file(std::string options,std::vector<std::string> &data_fname)
{
    std::stringstream strm(options); std::string tfname;
    strm>>tfname;    
    data_fname.push_back(tfname);
}

void script_add_prefix(std::string options,std::string &stro)
{
    std::stringstream strm(options);
    std::cout<<"Adding a new prefix: ";
    strm>>stro;
    std::cout<<stro<<std::endl;
}

void script_enable_autocompletion(std::string options,bool &auto_complete,double &completion_coeff)
{
    std::stringstream strm(options); 
    auto_complete=true;
    strm>>completion_coeff;
    std::cout<<"Enabling autocompletion at level "<<completion_coeff<<std::endl;
}

void script_set_Dx(std::string options,double &Dx)
{
    std::stringstream strm(options);
    std::cout<<"Changing Dx from "<<add_unit_u(Dx)<<" to ";
    strm>>Dx;
    std::cout<<add_unit_u(Dx)<<std::endl;
}

void script_set_Dy(std::string options,double &Dy)
{
    std::stringstream strm(options);
    std::cout<<"Changing Dy from "<<add_unit_u(Dy)<<" to ";
    strm>>Dy;
    std::cout<<add_unit_u(Dy)<<std::endl;
}

void script_set_Dz(std::string options,double &Dz)
{
    std::stringstream strm(options);
    std::cout<<"Changing Dz from "<<add_unit_u(Dz)<<" to ";
    strm>>Dz;
    std::cout<<add_unit_u(Dz)<<std::endl;
}

void script_geo_subdivide(std::string options,Grid3<unsigned int> &matgrid,int &Nx,int &Ny,int &Nz,int &Nt,double &Dx,double &Dy,double &Dz)
{
    std::stringstream strm(options);
    int factorX=1,factorY=1,factorZ=1;
    
    strm>>factorX;
    strm>>factorY;
    strm>>factorZ;
    
    if(factorX<1) factorX=1;
    if(factorY<1) factorY=1;
    if(factorZ<1) factorZ=1;
    
    Grid3<unsigned int> tmp_grid=matgrid;
    
    matgrid.init(factorX*Nx,factorY*Ny,factorZ*Nz,0);
    
    int i,j,k;
    int l,m,n;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
    {
        int ib=i*factorX;
        int jb=j*factorY;
        int kb=k*factorZ;
        
        int ic=(i+1)*factorX;
        int jc=(j+1)*factorY;
        int kc=(k+1)*factorZ;
        
        for(l=ib;l<ic;l++){ for(m=jb;m<jc;m++){ for(n=kb;n<kc;n++)
        {
            matgrid(l,m,n)=tmp_grid(i,j,k);
        }}}
    }}}
    
    Nx*=factorX; Dx/=factorX;
    Ny*=factorY; Dy/=factorY;
    Nz*=factorZ; Dz/=factorZ;
    
    Nt*=factorZ;
}

void script_set_polarization(std::string options,std::string &polar_mode)
{
    std::stringstream strm(options);
    strm>>polar_mode;
    std::cout<<"Setting the polarisation mode to "<<polar_mode<<std::endl;
}

void script_set_slab_thickness(std::string options,double &hslab)
{
    std::stringstream strm(options);
    strm>>hslab;
    std::cout<<"Setting slab thickness to "<<add_unit_u(hslab)<<std::endl;
}

void script_set_spectrum(std::string options,int &Nl,double &li,double &lf)
{
    std::stringstream strm(options);
    std::cout<<"Changing the covered spectrum from ["<<add_unit_u(li)<<" ; "<<add_unit_u(lf)<<"] to [";
    strm>>Nl;
    strm>>li;
    strm>>lf;
    std::cout<<add_unit_u(li)<<" ; "<<add_unit_u(lf)<<"] with "<<Nl<<" points"<<std::endl;
}

void script_set_substrate_ind(std::string options,double &n3)
{
    std::stringstream strm(options);
    std::cout<<"Setting substrate index from "<<n3<<" to ";
    strm>>n3;
    std::cout<<n3<<std::endl;
}

void script_set_superstrate_ind(std::string options,double &n1)
{
    std::stringstream strm(options);
    std::cout<<"Setting superstrate index from "<<n1<<" to ";
    strm>>n1;
    std::cout<<n1<<std::endl;
}

void script_set_timesteps(std::string options,int &Nt)
{
    std::stringstream strm(options);
    strm>>Nt;
    std::cout<<"Setting the number of timesteps to "<<Nt<<std::endl;
}

void script_set_working_dir(std::string options,std::string &working_dir)
{
    std::stringstream strm(options);
    std::string tmp_str;
    strm>>tmp_str;
    char end_char=tmp_str[tmp_str.size()-1];
    if(end_char!='/' || end_char!='\\') tmp_str.append("/");
    
    working_dir=tmp_str;
}

//###############
//  ScriptInstr
//###############

ScriptInstr::ScriptInstr()
    :cmd(""), options(""), options_strm("")
{
}

ScriptInstr::ScriptInstr(std::string const &cmd_i,std::string const &opt_i)
    :cmd(cmd_i), options(opt_i), options_strm(opt_i)
{
}

ScriptInstr::ScriptInstr(ScriptInstr const &inst)
    :cmd(inst.cmd), options(inst.options), options_strm(inst.options)
{
}

void ScriptInstr::operator = (ScriptInstr const &inst)
{
    cmd=inst.cmd;
    options=inst.options;
    options_strm.str(inst.options);
}

void ScriptInstr::show()
{
    std::cout<<cmd<<"|"<<options<<std::endl;
}

//###############
// ScriptHandler
//###############

ScriptHandler::ScriptHandler()
    :ordered(false), ord_curr_cmd(0),
     cmd_sep(':'), instruc_sep(';'), base_string(""),
     Ncmd(0)
{
}

ScriptHandler::ScriptHandler(std::string bstring,char cmdS,char intS)
    :ordered(false), ord_curr_cmd(0),
     cmd_sep(cmdS), instruc_sep(intS), Ncmd(0)
{
    analyze(bstring);
}

ScriptHandler::ScriptHandler(ScriptHandler const &spth)
    :ordered(spth.ordered), ord_curr_cmd(spth.ord_curr_cmd),
     cmd_sep(spth.cmd_sep),
     instruc_sep(spth.instruc_sep),
     base_string(spth.base_string),
     Ncmd(spth.Ncmd), curr_instr(spth.curr_instr),
     instr_arr(spth.instr_arr),
     instr_used(spth.instr_used)
{
}

void ScriptHandler::add_cmd(std::string n_cmd,std::string n_options)
{
    clean_str(n_cmd);
    clean_str(n_options);
    
    instr_arr.push_back(ScriptInstr(n_cmd,n_options));
    instr_used.push_back(0);
    Ncmd++;
}

void ScriptHandler::analyze(std::string in_str)
{
    int i;
    
    base_string=in_str;
    clean_str(base_string);
    
    int Nc=base_string.size();
    
    std::vector<int> inst_loc;
    std::vector<std::string> str_list;
    
    inst_loc.push_back(-1);
    
    for(i=0;i<Nc;i++)
    {
        if(base_string[i]==instruc_sep)
        {
            inst_loc.push_back(i);
            Ncmd++;
        }
    }
    
    if(Ncmd>0)
    {
        for(i=0;i<Ncmd;i++)
        {
            int pos1=inst_loc[i]+1;
            int pos2=inst_loc[i+1];
            str_list.push_back(base_string.substr(pos1,pos2-pos1));
        }
    }
    
    if(Ncmd>0)
    {
        for(i=0;i<Ncmd;i++)
        {
            std::string &loc_inst=str_list[i];
            
            if(loc_inst.find(cmd_sep)!=std::string::npos)
            {
                unsigned int pos=loc_inst.find(cmd_sep);
                if(pos!=0 && pos!=loc_inst.size()-1)
                {
                    std::string tmpstr1=loc_inst.substr(0,pos);
                    std::string tmpstr2=loc_inst.substr(pos+1);
                    
                    clean_str(tmpstr1);
                    clean_str(tmpstr2);
                    
                    instr_arr.push_back(ScriptInstr(tmpstr1,tmpstr2));
                    instr_used.push_back(0);
                }
            }
        }
        Ncmd=instr_arr.size();
    }
}

void ScriptHandler::clean_str(std::string &target_str)
{
    int i;
    int Nc=target_str.size();
    
    Grid1<bool> chk_str(Nc,1);
    
    //Removes linebreaks
    
    for(i=0;i<Nc;i++)
    {
        char tmp=target_str[i];
        if(tmp=='\n') chk_str[i]=0;
        if(tmp=='\r') chk_str[i]=0;
    }
    
    //Removes multiple spaces
    
    for(i=0;i<Nc-1;i++)
    {
        char tmp1=target_str[i];
        char tmp2=target_str[i+1];
        
        if((tmp1=='\n' || tmp1=='\r') && tmp2==' ') chk_str[i+1]=0;
        if(tmp1==' ' && (tmp2=='\n' || tmp2=='\r')) chk_str[i]=0;
        if(tmp1==' ' && tmp2==' ') chk_str[i]=0;
    }
    
    i=0;
    while(target_str[i]==' ')
    {
        chk_str[i]=0;
        i++;
    }
    
    i=Nc-1;
    while(target_str[i]==' ')
    {
        chk_str[i]=0;
        i--;
    }
    
    std::stringstream strm;
    for(i=0;i<Nc;i++)
    {
        if(chk_str[i]) strm<<target_str[i];
    }
    target_str=strm.str();
}

void ScriptHandler::clear()
{
    int i;
    for(i=0;i<Ncmd;i++)
    {
        if(instr_used[i]) instr_used[i]=0;
    }
}

std::string ScriptHandler::get_options()
{
    instr_used[curr_instr]=1;
    if(ordered) ord_curr_cmd+=1;
    
    return instr_arr[curr_instr].options;
}

bool ScriptHandler::has(std::string str_query)
{
    int i;
    
    if(ordered)
    {
        if(ord_curr_cmd>=Ncmd) return 0;
        
        i=ord_curr_cmd;
        if(instr_used[i]==0 && instr_arr[i].cmd==str_query)
        {
            curr_instr=i;
            return 1;
        }
    }
    else
    {
        for(i=0;i<Ncmd;i++)
        {
            if(instr_used[i]==0 && instr_arr[i].cmd==str_query)
            {
                curr_instr=i;
                return 1;
            }
        }
    }
    
    return 0;
}

std::vector<ScriptHandler> ScriptHandler::make_subscripts(std::string cmd_cut)
{
    int i;
    
    int Nsub=0; // Number of subscripts
    
    for(i=0;i<Ncmd;i++)
        if(instr_arr[i].cmd==cmd_cut) Nsub++;
        
    if(Nsub==0)
        chk_msg("Scripts critical error");
    
    std::vector<int> cmd_loc(Nsub,0);
    std::vector<std::string> sub_bstring(Nsub);
    std::vector<ScriptHandler> sub_spt(Nsub);
        
    std::string search_cmd=cmd_cut;
    search_cmd+=cmd_sep;
    
    //Finds the locations of cmd_cut
        
    cmd_loc[0]=base_string.find(search_cmd);
    
    for(i=1;i<Nsub;i++)
        cmd_loc[i]=base_string.find(search_cmd,cmd_loc[i-1]+1);
    
    //Cuts the base string into substrings
    
    for(i=0;i<Nsub-1;i++)
    {
        sub_bstring[i]=base_string.substr(cmd_loc[i],cmd_loc[i+1]-cmd_loc[i]);
    }
    
    sub_bstring[Nsub-1]=base_string.substr(cmd_loc[Nsub-1]);
    
    //Makes the subscripts
    
    for(i=0;i<Nsub;i++) sub_spt[i].set(sub_bstring[i],cmd_sep,instruc_sep);
    
    return sub_spt;
}

void ScriptHandler::load(std::string fname)
{
    std::string tmpstr;
    
    std::ifstream file(fname.c_str(),std::ios::in);
    std::getline(file,tmpstr,'\0');
    
    analyze(tmpstr);
}

void ScriptHandler::print(std::string fname)
{
    int i;
    std::ofstream file(fname.c_str(),std::ios::out|std::ios::trunc);
    
    for(i=0;i<Ncmd;i++)
    {
        file<<instr_arr[i].cmd<<cmd_sep<<" "<<instr_arr[i].options<<instruc_sep<<std::endl;
    }
}

void ScriptHandler::set(std::string bstring,char cmdS,char intS)
{
    cmd_sep=cmdS;
    instruc_sep=intS;
    Ncmd=0;
    
    instr_arr.clear();
    instr_used.clear();
    
    analyze(bstring);
}

void ScriptHandler::set_ordered(bool ord)
{
    ordered=ord;
    ord_curr_cmd=0;
}

void ScriptHandler::set_separators(char cmdS,char instS)
{
    cmd_sep=cmdS;
    instruc_sep=instS;
}

void ScriptHandler::show()
{
    if(Ncmd>0)
    {
        int i;
        for(i=0;i<Ncmd;i++) instr_arr[i].show();
    }
}

void ScriptHandler::show_unused()
{
    std::cout<<"Unused script commands:"<<std::endl;
    if(Ncmd>0)
    {
        int i;
        for(i=0;i<Ncmd;i++)
            if(instr_used[i]==false) instr_arr[i].show();
    }
}

void ScriptHandler::operator = (ScriptHandler const &spth)
{
    ordered=spth.ordered;
    ord_curr_cmd=spth.ord_curr_cmd;
    cmd_sep=spth.cmd_sep;
    instruc_sep=spth.instruc_sep;
    base_string=spth.base_string;
    Ncmd=spth.Ncmd;
    curr_instr=spth.curr_instr;
    instr_arr=spth.instr_arr;
    instr_used=spth.instr_used;
}
