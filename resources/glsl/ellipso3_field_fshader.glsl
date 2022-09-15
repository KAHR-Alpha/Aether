/*Copyright 2008-2021 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#version 430 core

in vec4 vnorms_Vout;
in vec4 vcol_Vout;

layout(location=11) uniform vec4 sun_dir;

out vec4 color;

void main(void)
{
    float sdot=(1.0+dot(sun_dir,vnorms_Vout))/2.0;
    float coeff=0.5+0.5*sdot*sdot;
    
    color=vec4(coeff*vcol_Vout.rgb,1.0);
}
