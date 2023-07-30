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

#include <string_tools.h>
#include <enum_constants.h>

#include <iostream>
#include <iomanip>
#include <sstream>


bool alphabetical_precedence(std::string str_a,std::string str_b)
{
    unsigned int Nc=std::min(str_a.size(),str_b.size());
    
    for(unsigned int i=0;i<Nc;i++)
    {
        if(str_a[i]!=str_b[i])
        {
            if(alphabetical_score(str_b[i])<alphabetical_score(str_a[i])) return false;
            else return true;
        }
    }
    
    return true;
}

void alphabetical_reorder(std::vector<std::string> &v)
{
    for(unsigned int i=0;i<v.size();i++)
    {
        for(unsigned int j=i+1;j<v.size();j++)
        {
            if(!alphabetical_precedence(v[i],v[j]))
                std::swap(v[i],v[j]);
        }
    }
}

int alphabetical_score(unsigned char c)
{
    int score=0;
    
         if(c>=48 && c<=57) score=1000*c;
    else if(c>=65 && c<=90) score=1000*c;
    else if(c>=97 && c<=122) score=1000*(c-32)+c;
    else if(c<=127) score=c;
    else
    {
        int offset=0;
        
             if(c>=192 && c<=198) offset='A';
        else if(c==199) offset='C';
        else if(c>=200 && c<=203) offset='E';
        else if(c>=204 && c<=207) offset='I';
        else if(c==208) offset='D';
        else if(c==209) offset='N';
        else if((c>=210 && c<=214) || c==216) offset='O';
        else if(c>=217 && c<=220) offset='U';
        else if(c==221) offset='Y';
        else if(c==223) offset='B';
        else if(c>=224 && c<=230) offset='A';
        else if(c==231) offset='C';
        else if(c>=232 && c<=235) offset='E';
        else if(c>=236 && c<=239) offset='I';
        else if(c==241) offset='N';
        else if(c>=242 && c<=246) offset='O';
        else if(c>=249 && c<=252) offset='U';
        else if(c==253 || c==255) offset='Y';
        
        score=1000*offset+c;
    }
    
    return score;
}
#include <mathUT.h>
void clean_data_string(std::string &target_str,std::vector<bool> *reuseable_buffer)
{
    unsigned int i;
    unsigned int Nc=target_str.size();
    
    if(Nc==0) return;
    
    // Stardard decimal/delimiter conversion
    
    enum
    {
        DECIM_DOT,
        DECIM_COMA,
        DECIM_NONE
    };
    
    int decim=DECIM_DOT;
    
    if(target_str.find(".")==std::string::npos)
    {
        if(target_str.find(",")!=std::string::npos) decim=DECIM_COMA;
        else decim=DECIM_NONE;
    }
    
    if(decim==DECIM_DOT)
    {
        for(i=0;i<Nc;i++)
            if(target_str[i]==',' || target_str[i]==';') target_str[i]=' ';
    }
    else if(decim==DECIM_COMA)
    {
        std::string::size_type pos1=target_str.find(","),pos2;
        
        bool check=true;
        
        // Figuring out if the coma is actually used as a separator
        
        while(check)
        {
            pos2=target_str.find(",",pos1+1);
            
            if(pos2==std::string::npos) check=false;
            else
            {
                decim=DECIM_NONE;
                for(i=pos1;i<pos2;i++)
                    if(target_str[i]==' ' || target_str[i]==';') decim=DECIM_COMA;
            }
            
            if(decim==DECIM_NONE) break;
            
            pos1=pos2;
        }
        
        if(decim!=DECIM_NONE)
            for(i=0;i<Nc;i++)
            {
                if(target_str[i]==',') target_str[i]='.';
                else if(target_str[i]==';') target_str[i]=' ';
            }
    }
    
    if(decim==DECIM_NONE)
    {
        for(i=0;i<Nc;i++)
            if(target_str[i]==',' || target_str[i]==';') target_str[i]=' ';
    }
    
    // Clearing spaces and linebreaks
    
    std::vector<bool> *buffer=nullptr;
    
    if(reuseable_buffer!=nullptr) buffer=reuseable_buffer;
    else buffer=new std::vector<bool>;
    
    std::vector<bool> &chk_str=*buffer;
    
    chk_str.resize(Nc);
    for(i=0;i<Nc;i++) chk_str[i]=true;
    
    // - Replacing tabulations with spaces
    
    for(i=0;i<Nc;i++)
        if(target_str[i]=='\t') target_str[i]=' ';
        
    // Removes linebreaks
    
    for(i=0;i<Nc;i++)
    {
        char tmp=target_str[i];
        if(tmp=='\n') chk_str[i]=0;
        if(tmp=='\r') chk_str[i]=0;
    }
    
    // - Removes multiple spaces
    
    for(i=0;i<Nc-1;i++)
    {
        char tmp1=target_str[i];
        char tmp2=target_str[i+1];
        
        if((tmp1=='\n' || tmp1=='\r') && tmp2==' ') chk_str[i+1]=0;
        if(tmp1==' ' && (tmp2=='\n' || tmp2=='\r')) chk_str[i]=0;
        if(tmp1==' ' && tmp2==' ') chk_str[i]=0;
    }
    
    // - Removes initial spaces
    
    for(i=0;i<Nc;i++)
    {
        if(target_str[i]==' ') chk_str[i]=0;
        else break;
    }
    
    // - Removes trailing spaces
    
    for(i=Nc-1;i>=1;i--)
    {
        if(target_str[i]==' ') chk_str[i]=0;
        else break;
    }
    
    std::stringstream strm;
    for(i=0;i<Nc;i++)
    {
        if(chk_str[i]) strm<<target_str[i];
    }
    
    target_str=strm.str();
    
    if(reuseable_buffer==nullptr) delete buffer;
}

bool compare_caracter(char c,std::string const &pattern)
{
    for(unsigned int i=0;i<pattern.size();i++)
        if(pattern[i]==c) return true;
    
    return false;
}

bool contains_non_numerics(std::string const &str)
{
    for(unsigned int i=0;i<str.size();i++)
    {
        char const &c=str[i];
        
        if(!((c>='0' && c<='9') || c==' ' || c=='e' || c=='E' || c=='-' || c=='.' || c=='+')) return true;
    }
    
    return false;
}

int dir_to_int(std::string const &orient_str)
{
         if(is_x_pos(orient_str)) return NORMAL_X;
    else if(is_y_pos(orient_str)) return NORMAL_Y;
    else if(is_z_pos(orient_str)) return NORMAL_Z;
    else if(is_x_neg(orient_str)) return NORMAL_XM;
    else if(is_y_neg(orient_str)) return NORMAL_YM;
    else if(is_z_neg(orient_str)) return NORMAL_ZM;
    
    return NORMAL_UNDEF;
}

bool is_x_neg(std::string const &orient_str)
{
    if(orient_str=="-X" || orient_str=="-x" ||
       orient_str=="X-" || orient_str=="x-" ) return true;
    return false;
}

bool is_x_pos(std::string const &orient_str)
{
    if(orient_str=="X" || orient_str=="x" ||
       orient_str=="+X" || orient_str=="+x" ||
       orient_str=="X+" || orient_str=="x+" ) return true;
    return false;
}

bool is_y_neg(std::string const &orient_str)
{
    if(orient_str=="-Y" || orient_str=="-y" ||
       orient_str=="Y-" || orient_str=="y-" ) return true;
    return false;
}

bool is_y_pos(std::string const &orient_str)
{
    if(orient_str=="Y" || orient_str=="y" ||
       orient_str=="+Y" || orient_str=="+y" ||
       orient_str=="Y+" || orient_str=="y+" ) return true;
    return false;
}

bool is_z_neg(std::string const &orient_str)
{
    if(orient_str=="-Z" || orient_str=="-z" ||
       orient_str=="Z-" || orient_str=="z-" ) return true;
    return false;
}

bool is_z_pos(std::string const &orient_str)
{
    if(orient_str=="Z" || orient_str=="z" ||
       orient_str=="+Z" || orient_str=="+z" ||
       orient_str=="Z+" || orient_str=="z+" ) return true;
    return false;
}

std::string remove_characters(std::string const &str,char rem)
{
    std::string str_out;
    
    std::size_t i,N=0;
    
    for(i=0;i<str.size();i++)
        if(str[i]!=rem) N++;
    
    str_out.resize(N);
    
    N=0;
    
    for(i=0;i<str.size();i++)
    {
        if(str[i]!=rem)
        {
            str_out[N]=str[i];
            N++;
        }
    }
    
    return str_out;
}

std::string replace_character(std::string const &str,char M,char R)
{
    std::string str_out=str;
    
    for(unsigned int i=0;i<str_out.size();i++)
    {
        if(str_out[i]==M) str_out[i]=R;
    }
    
    return str_out;
}

std::string replace_special_characters(std::string const &str)
{
    unsigned int i,j,N=str.size();
    
    int Nsp=0;
    for(i=0;i<N;i++)
        if(compare_caracter(str[i],"\n\t\r")) Nsp++;
    
    std::string str_out;
    str_out.resize(str.size()+Nsp);
    
    for(i=0,j=0;i<N;i++,j++)
    {
             if(str[i]=='\n') { str_out[j]='\\'; str_out[j+1]='n'; j++; }
        else if(str[i]=='\r') { str_out[j]='\\'; str_out[j+1]='r'; j++; }
        else if(str[i]=='\t') { str_out[j]='\\'; str_out[j+1]='t'; j++; }
        else str_out[j]=str[i];
    }
    
    return str_out;
}

std::string seconds_to_string(double Nsec)
{
    int t_d=Nsec/86400.0;
    int t_h=(Nsec-t_d*86400)/3600.0;
    int t_m=(Nsec-t_d*86400-t_h*3600)/60.0;
    double t_s=Nsec-t_d*86400-t_h*3600-t_m*60;
    
    std::stringstream strm;
    
    if(t_d!=0) strm<<t_d<<"d"<<t_h<<"h"<<t_m<<"m"<<std::setprecision(2)<<t_s<<"s";
    else
    {
        if(t_h!=0)  strm<<t_h<<"h"<<t_m<<"m"<<std::setprecision(2)<<t_s<<"s";
        else
        {
            if(t_m!=0) strm<<t_m<<"m"<<std::setprecision(2)<<t_s<<"s";
            else strm<<t_s<<"s";
        }
    }
    
    return strm.str();
}

void split_string(std::vector<std::string> &split,std::string const &str,char delimiter)
{
    int i,N=str.size();
    int s_start=0,s_end=N-1;
    
    for(i=0;i<N;i++)
        if(!compare_caracter(str[i]," \n\r\t")) break;
    
    s_start=i;
    
    if(N==0 || s_start==s_end)
    {
        split.clear();
        return;
    }
    
    for(i=s_end;i>=s_start;i--)
        if(!compare_caracter(str[i]," \n\r\t")) break;
    
    s_end=i;
    
    int Ns=1;
    for(i=s_start;i<=s_end;i++) if(str[i]==delimiter) Ns++;
    
    split.resize(Ns);
    
    i=s_start;
    int split_bit=0;
    
    while(i<=s_end)
    {
        if(str[i]==delimiter)
        {
            split[split_bit]=str.substr(s_start,i-s_start);
            s_start=i+1;
            split_bit++;
        }
        
        i++;
    }
    
    split[split_bit]=str.substr(s_start,s_end+1-s_start);
}

std::string to_lower(std::string str)
{
    for(unsigned int i=0;i<str.size();i++) str[i]=std::tolower(str[i]);
    
    return str;
}
