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

layout(location=0) in vec4 vpos;
layout(location=1) in vec4 vnorms;
layout(location=2) in vec4 vcol;
layout(location=3) in mat4 transf_matrix;

layout(location=10) uniform mat4 proj_mat;
out vec4 vnorms_Vout;
out vec4 vcol_Vout;

void main(void) 
{
    gl_Position=proj_mat*(transf_matrix*vpos);
    
    vec4 t_vnorm=transf_matrix*vnorms;
    vnorms_Vout=t_vnorm/length(t_vnorm);
    
    vcol_Vout=vcol;
}
