#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 InPosL;
layout (location = 1) in vec4 InNormal;

layout (location = 0) out vec3 outPosW;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outNormal;

layout (std140, set = 0, binding = 0) uniform UBO 
{
	// Camera 
	mat4 projection;
	mat4 view;
	vec4 clippingPlane;
	vec3 eyePos;
} per_frame_vs;

layout(push_constant) uniform PushConsts {
	 mat4 world;		
	 vec3 color;
} pushConsts;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_ClipDistance[];
};

void main(void)
{
	outPosW = (pushConsts.world * vec4(InPosL.xyz, 1.0)).xyz;
	outColor = pushConsts.color;
	outNormal = InNormal.xyz;

	gl_Position = per_frame_vs.projection * per_frame_vs.view * pushConsts.world * vec4(InPosL.xyz, 1.0);

	vec4 clipPlane = vec4(0.0, 1.0, 0.0, 1500);	
	gl_ClipDistance[0] = dot(pushConsts.world * vec4(InPosL.xyz, 1.0), per_frame_vs.clippingPlane);	
}