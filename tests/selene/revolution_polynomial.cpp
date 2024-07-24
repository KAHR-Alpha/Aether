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

#include <selene_primitives.h>

int revolution_polynomial(int argc,char *argv[])
{
    Sel::BoundingBox bbox;
    std::vector<Sel::SelFace> faces;
    std::vector<std::string> face_names;

    Sel::Primitives::Polynomial poly(bbox, faces, face_names);

    std::vector<double> coeffs={1.0, 0.5};

    poly.set_parameters(10e2, 0, coeffs, true);

    return 0;
}
