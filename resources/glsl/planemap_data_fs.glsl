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

layout(location=0) out vec3 pos_FS;
layout(location=1) out float data_FS;

in vec4 pos_VS;
in vec2 uv_VS;

layout(binding=0) uniform sampler2D tex;
//uniform sampler2D tex;

void main(void)
{
    pos_FS=pos_VS.xyz;
    
    vec4 data_tmp=texture(tex,uv_VS);
    
    data_FS=data_tmp.r;
}
