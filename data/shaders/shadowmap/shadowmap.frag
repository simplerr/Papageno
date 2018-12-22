#version 450

layout (location = 0) in vec3 InColor;

layout (location = 0) out float OutColor;
layout (location = 1) out float OutColorDebug;

const float NEAR_PLANE = 1.0f; //todo: specialization const
const float FAR_PLANE = 10000.0f; //todo: specialization const 

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{
	//OutColor = vec4(vec3(linearDepth(gl_FragCoord.z) / 25600), 1.0f);
	OutColor = gl_FragCoord.z;// / 2560;
	OutColorDebug = linearDepth(gl_FragCoord.z) / FAR_PLANE;
}