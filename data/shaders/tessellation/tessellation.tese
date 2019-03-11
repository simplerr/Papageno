#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared.glsl"

layout (push_constant) uniform PushConstants {
	 mat4 world;
	 mat4 worldInv;
} pushConstants;

layout(quads, fractional_odd_spacing, ccw) in;

layout (location = 0) in vec3 InNormalL[];
layout (location = 1) in vec2 InTex[];
 
layout (location = 0) out vec3 OutNormalL;
layout (location = 1) out vec2 OutTex;
layout (location = 2) out vec3 OutPosW;

layout (set = 0, binding = 6) uniform sampler2D samplerDisplacement[3];

void main()
{
	// Interpolate UV coordinates
	vec2 uv1 = mix(InTex[0], InTex[1], gl_TessCoord.x);
	vec2 uv2 = mix(InTex[3], InTex[2], gl_TessCoord.x);
	OutTex = mix(uv1, uv2, gl_TessCoord.y);

	// Interpolate normals
	vec3 n1 = mix(InNormalL[0], InNormalL[1], gl_TessCoord.x);
	vec3 n2 = mix(InNormalL[3], InNormalL[2], gl_TessCoord.x);
	OutNormalL = mix(n1, n2, gl_TessCoord.y);

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	// Displace large details
    pos.y = getHeight(OutTex);

    float textureScaling = ubo_settings.textureScaling;

	// Test
	float lowAltitudeDisplacement = texture(samplerDisplacement[0], OutTex * textureScaling).r; 
	float highAltitudeDisplacement = texture(samplerDisplacement[1], OutTex * textureScaling).r; 
	float cliffDisplacement = texture(samplerDisplacement[2], OutTex * textureScaling).r; 
    vec4 blend = texture(samplerBlendmap, OutTex);
	blend = clamp(blend, vec4(0.0), vec4(1.0));

    float finalDisplacement = blend.r * lowAltitudeDisplacement +
							  blend.g * highAltitudeDisplacement +
							  blend.b * cliffDisplacement;

	// Displace small details from displacement map along normal
	vec3 normal = normalize(getNormal(OutTex));
	float amplitude = ubo_settings.bumpmapAmplitude;
	pos.xyz -= normal * finalDisplacement * amplitude;

	OutPosW = (pushConstants.world * pos).xyz;
	// Perspective projection
	gl_Position = ubo_camera.projection * ubo_camera.view * pushConstants.world * pos;
}