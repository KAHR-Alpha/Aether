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

#ifndef SPECTRAL_COLOR_H_INCLUDED
#define SPECTRAL_COLOR_H_INCLUDED


#include <mathUT.h>
class DistinctColorsSampler
{
    public:
        std::vector<double> L,a,b;
        
        DistinctColorsSampler(double R,double G,double B);
        void operator() (double &R,double &G,double &B);
};

void spectrum_to_XYZ(std::vector<double> const &lambda,std::vector<double> const &spectrum,std::string const &source,
                     double &X,double &Y,double &Z);
void spectrum_to_sRGB(std::vector<double> const &lambda,std::vector<double> const &spectrum,std::string const &source,
                     double &R,double &G,double &B);
double sRGB_inverse_linear_transform(double C);
double sRGB_linear_transform(double C);
void sRGB_to_XYZ(double &X,double &Y,double &Z,double R,double G,double B);
void sRGB_to_Lab(double &L,double &a,double &b,double R,double G,double B);
void XYZ_to_Lab(double &L,double &a,double &b,double X,double Y,double Z);
void XYZ_to_sRGB(double X,double Y,double Z,double &R,double &G,double &B);


#endif // SPECTRAL_COLOR_H_INCLUDED
