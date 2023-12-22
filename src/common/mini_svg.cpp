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

#include <mini_svg.h>

extern std::ofstream plog;

namespace SvgLite
{

    Svg::Svg(std::string fname)
    {
        std::string str=fname;
        str.append(".svg");
        
        f_out.open(str,std::ios::out|std::ios::trunc);
        
        f_out<<"<?xml version=\"1.0\" standalone=\"no\"?>"<<std::endl;
        f_out<<"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\""<<std::endl;
        f_out<<"\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"<<std::endl;
        
        f_out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"<<std::endl;
        f_out<<"<circle cx=\"100\" cy=\"50\" r=\"40\" stroke=\"black\""<<std::endl;
        f_out<<"stroke-width=\"2\" fill=\"red\" />"<<std::endl;
        f_out<<"</svg> "<<std::endl;
    }
    
    Svg::~Svg()
    {
        f_out.close();
    }
}
    
void relat_to_mbezier(std::string fname,
                      double xmin,double xmin_ref,
                      double xmax,double xmax_ref,
                      double ymin,double ymin_ref,
                      double ymax,double ymax_ref,
                      MultiBezier<double,double> &mb)
{
    unsigned int i;
    double x,y;
    
    std::ifstream file(fname,std::ios::in);
    std::vector<std::string> str_base;
    std::vector<double> x_base,y_base;
    
    while(!file.eof())
    {
        std::string str;
        file>>str;
        
        int vp=str.find(',');
        str[vp]=' ';
        
        std::stringstream strm(str);
        
        strm>>x;
        strm>>y;
        
        x_base.push_back(x);
        y_base.push_back(y);
    }
    
    unsigned int Ns=(x_base.size()-1)/3;
    mb.set(Ns);
    
    for(i=0;i<Ns;i++)
    {
        x=x_base[3*i];
        y=y_base[3*i];
        
        x_base[3*i+1]+=x;
        x_base[3*i+2]+=x;
        x_base[3*i+3]+=x;
        
        y_base[3*i+1]+=y;
        y_base[3*i+2]+=y;
        y_base[3*i+3]+=y;
    }
    
    for(i=0;i<x_base.size();i++)
    {
        x_base[i]=xmin+(xmax-xmin)*(x_base[i]-xmin_ref)/(xmax_ref-xmin_ref);
        y_base[i]=ymin+(ymax-ymin)*(y_base[i]-ymin_ref)/(ymax_ref-ymin_ref);
    }
    
    Grid1<double> x_tmp(4),y_tmp(4);
    
    for(i=0;i<Ns;i++)
    {
        x_tmp[0]=x_base[3*i+0];
        x_tmp[1]=x_base[3*i+1];
        x_tmp[2]=x_base[3*i+2];
        x_tmp[3]=x_base[3*i+3];
        
        y_tmp[0]=y_base[3*i+0];
        y_tmp[1]=y_base[3*i+1];
        y_tmp[2]=y_base[3*i+2];
        y_tmp[3]=y_base[3*i+3];
        
        mb.set(i,x_tmp,y_tmp);
    }
}
