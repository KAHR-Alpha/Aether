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

layout(binding=0) uniform sampler2D tex;
layout(location=11) uniform int color_palette;

vec4 BW_gradient(float value)
{
    return vec4(value,value,value,1.0);
}

vec4 jet_gradient(float value)
{
    float r,g,b;
    float R=1.0/4.0;
    
         if(value>=0 && value<R)
    {
         b=1.0;
         g=1.0*(value)/R;
         r=0.0;
    }
    else if(value>=R && value<2*R)
    {
         b=1.0-1.0*(value-R)/R;
         g=1.0;
         r=0.0;
    }
    else if(value>=2*R && value<3*R)
    {
         b=0.0;
         g=1.0;
         r=1.0*(value-2*R)/R;
    }
    else if(value>=3*R && value<4*R)
    {
         b=0.0;
         g=1.0-1.0*(value-3*R)/R;
         r=1.0;
    }
    else if(value>=1.0)
    {
         b=0.0;
         g=0.0;
         r=1.0;
    }
    
    return vec4(r,g,b,1.0);
}

vec4 extended_jet_gradient(float value)
{
    float r,g,b;
    float R=1.0/5.0;
    
         if(value>=0 && value<R)
    {
        b=1.0;
        g=0.0;
        r=1.0-1.0*value/R;
    }
    else if(value>=R && value<2*R)
    {
         b=1.0;
         g=1.0*(value-R)/R;
         r=0.0;
    }
    else if(value>=2*R && value<3*R)
    {
         b=1.0-1.0*(value-2*R)/R;
         g=1.0;
         r=0.0;
    }
    else if(value>=3*R && value<4*R)
    {
         b=0.0;
         g=1.0;
         r=1.0*(value-3*R)/R;
    }
    else if(value>=4*R && value<5*R)
    {
         b=0.0;
         g=1.0-1.0*(value-4*R)/R;
         r=1.0;
    }
    else if(value>=1.0)
    {
         b=0.0;
         g=0.0;
         r=1.0;
    }
    
    return vec4(r,g,b,1.0);
}

void main(void)
{
    vec4 tmp_color=texture(tex,uv_VS);
//    color=extended_jet_gradient(tmp_color.r);
         if(color_palette==0) color=BW_gradient(tmp_color.r);
    else if(color_palette==1) color=jet_gradient(tmp_color.r);
    else if(color_palette==2) color=extended_jet_gradient(tmp_color.r);
}
