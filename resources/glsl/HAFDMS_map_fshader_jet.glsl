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

void jet(in float val,out vec4 val_out)
{
    val_out=vec4(0,0,0,1.0);
    
    float R=8.0*val;
    if(R<0) R=0;
    if(R>8.0) R=8.0;
    
    float r,g,b;
    
         if(R<=1.0)
    {
        b=0.5*(R+1.0);
        g=0.0;
        r=0.0;
    }
    else if(R>1.0 && R<=3.0)
    {
         b=1.0;
         g=0.5*(R-1.0);
         r=0.0;
    }
    else if(R>3.0 && R<=5.0)
    {
         b=1.0-0.5*(R-3.0);
         g=1.0;
         r=0.5*(R-3.0);
    }
    else if(R>5.0 && R<=7.0)
    {
         b=0.0;
         g=1.0-0.5*(R-5.0);
         r=1.0;
    }
    else if(R>7.0)
    {
         b=0.0;
         g=0.0;
         r=1.0-0.5*(R-7.0);
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
    
    jet(val_exp,color);
}
