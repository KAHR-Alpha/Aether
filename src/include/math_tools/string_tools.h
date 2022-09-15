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

#ifndef STRING_TOOLS_H_INCLUDED
#define STRING_TOOLS_H_INCLUDED

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

template<typename T>
std::string add_unit(T val,std::string bunit="m")
{
    std::stringstream strm;
    std::string unit;
    
    double aval=std::abs(val);
    double pval=std::log10(aval);
    int rank=static_cast<int>(pval/3.0);
    if(pval<0) rank-=1;
    
    if(rank==0) unit="";
    else if(rank==1) unit="k";
    else if(rank==2) unit="M";
    else if(rank==3) unit="G";
    else if(rank==4) unit="T";
    else if(rank==5) unit="P";
    else if(rank==6) unit="E";
    else if(rank==7) unit="Z";
    else if(rank==8) unit="Y";
    else if(rank==-1) unit="m";
    else if(rank==-2) unit=char(230);
    else if(rank==-3) unit="n";
    else if(rank==-4) unit="p";
    else if(rank==-5) unit="f";
    else if(rank==-6) unit="a";
    else if(rank==-7) unit="z";
    else if(rank==-8) unit="y";
    
    if(val<0) strm<<"-";
    
    if(val!=0)
    {
        strm<<aval/std::pow(10,3*rank);
        strm<<" "<<unit<<bunit;
    }
    else
    {
        strm<<0<<" "<<bunit;
    }
    
    return strm.str();
}

template<typename T>
std::string add_unit_u(T val,std::string bunit="m")
{
    std::stringstream strm;
    std::string unit;
    
    double aval=std::abs(val);
    double pval=std::log10(aval);
    int rank=static_cast<int>(pval/3.0);
    if(pval<0) rank-=1;
    
    if(rank==0) unit="";
    else if(rank==1) unit="k";
    else if(rank==2) unit="M";
    else if(rank==3) unit="G";
    else if(rank==4) unit="T";
    else if(rank==5) unit="P";
    else if(rank==6) unit="E";
    else if(rank==7) unit="Z";
    else if(rank==8) unit="Y";
    else if(rank==-1) unit="m";
    else if(rank==-2) unit="u";
    else if(rank==-3) unit="n";
    else if(rank==-4) unit="p";
    else if(rank==-5) unit="f";
    else if(rank==-6) unit="a";
    else if(rank==-7) unit="z";
    else if(rank==-8) unit="y";
    
    if(val<0) strm<<"-";
    
    if(val!=0)
    {
        strm<<aval/std::pow(10,3*rank);
        strm<<" "<<unit<<bunit;
    }
    else
    {
        strm<<0<<" "<<bunit;
    }
    
    return strm.str();
}

template<typename T>
void get_unit_u(T val,std::string const &base_unit,std::string &unit,double &scale)
{
    double aval=std::abs(val);
    double pval=std::log10(aval);
    int rank=static_cast<int>(pval/3.0);
    if(pval<0) rank-=1;
    
    if(rank==0) unit="";
    else if(rank==1) unit="k";
    else if(rank==2) unit="M";
    else if(rank==3) unit="G";
    else if(rank==4) unit="T";
    else if(rank==5) unit="P";
    else if(rank==6) unit="E";
    else if(rank==7) unit="Z";
    else if(rank==8) unit="Y";
    else if(rank==-1) unit="m";
    else if(rank==-2) unit="u";
    else if(rank==-3) unit="n";
    else if(rank==-4) unit="p";
    else if(rank==-5) unit="f";
    else if(rank==-6) unit="a";
    else if(rank==-7) unit="z";
    else if(rank==-8) unit="y";
    
    scale=std::pow(10,3*rank);
    unit.append(base_unit);
}


bool alphabetical_precedence(std::string str_a,std::string str_b);
void alphabetical_reorder(std::vector<std::string> &v);
int alphabetical_score(unsigned char c);
void clean_data_string(std::string &target_str,
                       std::vector<bool> *reuseable_buffer=nullptr);
bool compare_caracter(char c,std::string const &pattern);
bool contains_non_numerics(std::string const &str);
int dir_to_int(std::string const &orient_str);
bool is_x_neg(std::string const &orient_str);
bool is_x_pos(std::string const &orient_str);
bool is_y_neg(std::string const &orient_str);
bool is_y_pos(std::string const &orient_str);
bool is_z_neg(std::string const &orient_str);
bool is_z_pos(std::string const &orient_str);
std::string remove_characters(std::string const &str,char rem);
std::string replace_character(std::string const &str,char M,char R);
std::string replace_special_characters(std::string const &str);
std::string seconds_to_string(double Nsec);
void split_string(std::vector<std::string> &split,std::string const &str,char delimiter=' ');
[[deprecated]] std::string to_generic_path(std::string path);
std::string to_lower(std::string str);

#endif // STRING_TOOLS_H_INCLUDED
