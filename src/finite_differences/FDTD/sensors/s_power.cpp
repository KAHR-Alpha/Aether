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

#include <sensors.h>

extern std::ofstream plog;

Box_Poynting::Box_Poynting(int x1_,int x2_,
                           int y1_,int y2_,
                           int z1_,int z2_)
{
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
}


void Box_Poynting::deep_feed(FDTD const &fdtd)
{
    plog<<step<<" "<<fdtd.compute_poynting_box(x1,x2,y1,y2,z1,z2)<<" "<<fdtd.Ey.max()<<std::endl;
}
