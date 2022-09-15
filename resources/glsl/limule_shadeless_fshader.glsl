#version 430 core

in vec4 vnorms_Vout;
in vec4 vcol_Vout;

out vec4 color;

void main(void)
{
    color=vec4(vcol_Vout.rgb,1.0);
}
