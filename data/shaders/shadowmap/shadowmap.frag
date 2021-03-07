#version 450

layout (location = 0) in vec2 InTex;

layout (location = 0) out float OutColor;

const float NEAR_PLANE = 1.0f; //todo: specialization const
const float FAR_PLANE = 10000.0f; //todo: specialization const 

layout (set = 1, binding = 0) uniform sampler2D texSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D specularSampler;

float linearDepth(float depth)
{
   float z = depth * 2.0f - 1.0f; 
   return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));  
}

void main()
{
   vec4 color = texture(texSampler, InTex);

   // Todo: Remove, use to get a descriptor set layout that matches mMeshTexturesDescriptorSetLayout in ModelLoader
   vec4 hack = texture(normalSampler, InTex);
   vec4 hack2 = texture(specularSampler, InTex);

   if (color.a < 0.01f)
      discard;

   OutColor = gl_FragCoord.z;
   //OutColorDebug = linearDepth(gl_FragCoord.z) / FAR_PLANE; // Use this if perspective projection matrix
}