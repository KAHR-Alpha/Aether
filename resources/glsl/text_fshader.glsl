#version 430 core

out vec4 color;
in vec2 uv_VS;

layout(binding=0) uniform sampler2D tex;

void main(void)
{
    vec4 val=texture(tex,uv_VS);
    color=vec4(1.0,1.0,1.0,val.r);
}
