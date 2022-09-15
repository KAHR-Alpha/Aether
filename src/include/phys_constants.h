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

#ifndef PHYS_CONSTANTS_H
#define PHYS_CONSTANTS_H

#define elem_charge 1.6021766208e-19
#define e0 8.854187817e-12
#define mu0 12.566370614e-7
#define c_light 299792458.0
#define hbar 1.054571726e-34
#define h_planck 6.62606957e-34
#define k_boltzmann 1.3806485279e-23

enum
{
    TE=0,
    TM,
    TEM,
    EX_FIELD=0,
    EY_FIELD,
    EZ_FIELD,
    HX_FIELD,
    HY_FIELD,
    HZ_FIELD,
};

#endif // PHYS_CONSTANTS_H
