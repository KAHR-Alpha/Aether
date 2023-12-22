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

#include <iostream>
#include <fstream>

#include <mathUT.h>
#include <math_approx.h>

namespace SvgLite
{
    class Svg_element
    {
        private:
            double x;
            double y;
            
        public:
            void set_x(double);
            void set_y(double);
        
    };

    class Svg_line :public Svg_element
    {
        
    };

    class Svg
    {
        public:
            std::ofstream f_out;
            
            Svg(std::string);
            ~Svg();
            
            void add_rect(double x,double y,double w,double h);
    };
}
    
void relat_to_mbezier(std::string fname,
                      double xmin,double xmin_ref,
                      double xmax,double xmax_ref,
                      double ymin,double ymin_ref,
                      double ymax,double ymax_ref,
                      MultiBezier<double,double> &mb);
