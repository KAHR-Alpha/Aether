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

void degra(in float val,out vec4 val_out)
{
    val_out=vec4(0,0,0,1.0);
    
    float R=1.0/5.0;
    float r,g,b;
    
         if(val>=0 && val<R)
    {
        b=1.0;
        g=0.0;
        r=1.0-1.0*val/R;
    }
    else if(val>=R && val<2*R)
    {
         b=1.0;
         g=1.0*(val-R)/R;
         r=0.0;
    }
    else if(val>=2*R && val<3*R)
    {
         b=1.0-1.0*(val-2*R)/R;
         g=1.0;
         r=0.0;
    }
    else if(val>=3*R && val<4*R)
    {
         b=0.0;
         g=1.0;
         r=1.0*(val-3*R)/R;
    }
    else if(val>=4*R && val<5*R)
    {
         b=0.0;
         g=1.0-1.0*(val-4*R)/R;
         r=1.0;
    }
    else if(val>=1.0)
    {
         b=0.0;
         g=0.0;
         r=1.0;
    }
    
    val_out.r=r;
    val_out.g=g;
    val_out.b=b;
}

void main(void)
{
    vec4 tmp_color=texture(tex,uv_VS);
    float val=tmp_color.r;
    float val_exp=1.0-exp(-val/exposure);
    
//    color=vec4(val_exp,val_exp,val_exp,1.0);
    degra(val_exp,color);
}
