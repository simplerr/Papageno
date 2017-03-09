#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	vec3 color = vec3(1, 1, 1);
	vec3 lightVec = vec3(1, 1, 1);
	lightVec = normalize(lightVec);

	float diffuseFactor = max(0, dot(lightVec, inNormal));

	float ambientFactor = 0.1;
	outFragColor = vec4(color * ambientFactor + diffuseFactor * color, 1.0);

	outFragColor = vec4(inNormal.r, inNormal.g, inNormal.b, 1.0);
	//outFragColor = vec4(0, 0, inNormal.b, 1.0);
}