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

layout(location=0) in vec4 vpos;
layout(location=1) in vec4 vnorms;
layout(location=2) in vec4 offset;
layout(location=3) in vec4 A;
layout(location=4) in float gen;
layout(location=5) in float lambda;
layout(location=6) in float lost;

layout(location=10) uniform mat4 proj_mat;
layout(location=14) uniform float lost_length;
layout(location=15) uniform float gen_min;
layout(location=16) uniform float gen_max;
layout(location=17) uniform float lambda_min;
layout(location=18) uniform float lambda_max;
layout(location=19) uniform int display_type;

out float gen_Vout;
out float lambda_Vout;

void main(void) 
{
    if(lost<0.5) gl_Position=proj_mat*(offset+A*vpos.x);
    else gl_Position=proj_mat*(offset+A*lost_length*vpos.x);
    
    if(   gen<gen_min-0.1 || gen>gen_max+0.1
       || lambda<lambda_min    || lambda>lambda_max   ) gl_Position=vec4(2,2,2,1.0);
       
//    if(gen<gen_min-0.1 || gen>gen_max+0.1) gl_Position=vec4(2,2,2,1.0);
    
    gen_Vout=gen;
    lambda_Vout=lambda;
}
