/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef SELENE_PRIMITIVES_H
#define SELENE_PRIMITIVES_H

namespace Sel
{
    struct BoundingBox
    {
        double xm=0;
        double xp=0;

        double ym=0;
        double yp=0;

        double zm=0;
        double zp=0;

        double span_x() { return xp-xm; }
        double span_y() { return yp-ym; }
        double span_z() { return zp-zm; }
    };
}

#endif // SELENE_PRIMITIVES_H
