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

#version 430 core

out vec4 color;
in vec2 uv_VS;

layout(location=11) uniform float exposure;
layout(binding=0) uniform sampler2D tex;

void main(void)
{
    vec4 tmp_color=texture(tex,uv_VS);
    float val=tmp_color.r;
    float val_exp=1.0-exp(-val/exposure);
    
    color=vec4(val_exp,val_exp,val_exp,1.0);
}
