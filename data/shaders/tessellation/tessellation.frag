#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (location = 0) in vec3 InNormalL;
layout (location = 1) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

void main() 
{
    //OutColor = vec4(InTex.x, InTex.y, 0, 1);
    vec3 normal = getNormal(InTex);
    OutColor = vec4(normal, 1.0);
}