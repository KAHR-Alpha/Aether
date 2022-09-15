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

#ifndef NOISE_H
#define NOISE_H

#include <mathUT.h>

class Perlin2
{
    private:
        int gsize;
        Grid2<Vector2> data;
    
    public:
        Perlin2();
        
        double antiblobby(double x,double y,double per,double amp,int rec);
        double antilightning(double x,double y,double per,double amp,int rec);
        double blobby(double x,double y,double per,double amp,int rec);
        double cloud(double x,double y,double per,double amp,int rec);
        double lightning(double x,double y,double per,double amp,int rec);
        double operator() (double x,double y,double sc,double amp);
        
};

class Perlin3
{
    private:
        int Nthr,gsize;
        std::vector<std::vector<int>> xc,yc,zc;
        std::vector<std::vector<Vector3>> vb;
        Grid3<Vector3> data;
        
    public:
        Perlin3(bool init_noise=true);
        
        double antilightning(double x,double y,double z,double per,double amp,int rec,int thID=0); // x,y,z location in cartesian space, "per" average distance between perlin bumps, amp-litude, rec-ursion level
        double cloud(double x,double y,double z,double per,double amp,int rec,int thID=0);
        void init();
        double lightning(double x,double y,double z,double per,double amp,int rec,int thID=0);
        double operator() (double x,double y,double z,double per,double amp,int thID=0);
        void set_threads_number(int Nthr);
};

class Simplex2
{
    private:
        
    public:
        Simplex2();
        
        double operator() (double,double,double);
};

#endif
