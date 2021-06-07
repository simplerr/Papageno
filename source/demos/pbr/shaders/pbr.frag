#version 450

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;
layout (location = 5) in vec3 InTangentL;

layout (location = 0) out vec4 OutColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;

void main()
{
   vec4 diffuse = texture(diffuseSampler, InTex);
   vec4 normal = texture(normalSampler, InTex);

   vec3 T = normalize(InTangentL);
   vec3 B = cross(InNormalW, InTangentL); // Todo: * inTangent.w;
   vec3 N = normalize(InNormalW);
   mat3 TBN = mat3(T, B, N);
   N = TBN * normalize(normal.xyz * 2.0 - vec3(1.0));

   vec3 lightDir = vec3(0.5, 0.5, -0.5);
   float diffuseFactor = max(dot(lightDir, N), 0.0) + 0.2;
   OutColor = vec4(diffuse.rgb * diffuseFactor, 1.0f);
}
