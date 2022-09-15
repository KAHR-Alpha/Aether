#version 430 core

layout(location=0) in vec4 vpos;
layout(location=1) uniform mat4 proj_mat;

void main(void) 
{
    gl_Position=proj_mat*vpos;
}
