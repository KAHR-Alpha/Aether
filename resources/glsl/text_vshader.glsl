#version 430 core

layout(location=0) in vec2 vpos;
layout(location=1) in vec4 world_pos;
layout(location=2) in vec2 screen_offset;
layout(location=3) in vec2 screen_scale;
layout(location=4) in vec2 uv_offset;
layout(location=5) in vec2 uv_scale;
layout(location=6) uniform mat4 proj_mat;
layout(location=7) uniform mat4 screen_mat;

out vec2 uv_VS;

void main(void) 
{
    gl_Position=proj_mat*world_pos+screen_mat*vec4(screen_scale*vpos+screen_offset,0,0);
    
    uv_VS=uv_offset+vpos*uv_scale;
}
