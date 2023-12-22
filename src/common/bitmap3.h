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

#ifndef BITMAP_H
#define BITMAP_H

#include <mathUT.h>

#include <png.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <cmath>

void G2_to_BW(Grid2<double>&,std::string);
void G2_to_degra(Grid2<double>&,std::string);
void G2_to_degra_fixed(Grid2<double>&,std::string);
void G2_to_WB(Grid2<double>&,std::string);
void G3_to_col(Grid3<double>&,std::string);

class Bitmap
{
      public:
             char mag[2];
             int size;  //calc
             int app;
             int start;
             int sizeh;
             int width;  //def
             int height;  //def
             int col;
             int bpp;
             int comp;
             int sizeraw; //calc
             int resh;
             int resv;
             int colp;
             int colimp;
             int rank;
             
             //unsigned char ***M;
             Grid3<unsigned char> M;
             
             std::mutex modification_mutex;
             
             Bitmap();
             Bitmap(int x,int y);
             
             void operator()(int,int,double,double,double);
             
             void degra(int i,int j,double pos,double min,double max);
             void degra_bw(int i,int j,double pos,double min,double max);
             void degra_circ(int i,int j,double pos,double min,double max);
             void draw_circle(double x,double y,double R,double r=0,double g=0,double b=0);
             void draw_line(double x1,double y1,double x2,double y2,double r=0,double g=0,double b=0);
             void draw_line_radial(double i1,double j1,double rad,double th,double r=0,double g=0,double b=0);
             void G2degra(Grid2<double> &,std::string);
             void G2degra(Grid2<double> &,std::string,int,std::string);
             void G2degraM(Grid2<double> &,std::string,int,std::string,double,double);
             void G2degra_fixed(Grid2<double> &,std::string);
             void G2BW(Grid2<double> &,std::string);
             void G2BW(Grid2<double> &,std::string,int,std::string);
             void G2WB(Grid2<double> &,std::string);
             void G2WB(Grid2<double> &,std::string,int,std::string);
             void G3col(Grid3<double> &,std::string);
             std::mutex& get_mutex();
             void set(int,int,double,double,double);
             void set_full(double r,double g,double b);
             void set_size(int w,int h);
             void write(std::string const &fname);
             
             
             //color: B,G,R
};

#endif

