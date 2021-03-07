#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "material_types.glsl"
#include "shared_variables.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 0, binding = 0) uniform sampler2D reflectionSampler;
layout (set = 0, binding = 1) uniform sampler2D refractionSampler;
layout (set = 0, binding = 3) uniform sampler2D distortionSampler;
layout (set = 0, binding = 4) uniform sampler2D positionSampler;
layout (set = 0, binding = 5) uniform sampler2D normalSampler;
layout (set = 0, binding = 7) uniform sampler2D specularSampler;

layout (set = 0, binding = 8) uniform UBO_parameters
{
   float transparency;
   float underwaterViewDistance;
} ubo_parameters;

const vec3 deepWaterColor = vec3(0.0f, 0.2f, 0.3f);

void main()
{
   vec4 specular = texture(specularSampler, InTex);
   if (specular.r == 0.0f)
   {
      OutFragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      return;
   }

   vec3 position = texture(positionSampler, InTex).xyz;
   vec3 normal = texture(normalSampler, InTex).xyz;

   // Water samples refractions and also uses distorted texture coordinates
   uint type = uint(specular.g);
   if (type == MATERIAL_TYPE_WATER)
   {
      vec2 distortion = texture(distortionSampler, InTex).rg;
      vec3 reflectionColor = texture(reflectionSampler, InTex + distortion).rgb;
      vec3 refractionColor = texture(refractionSampler, InTex + distortion).rgb;

      vec3 toEyeW = normalize(sharedVariables.eyePos.xyz + position); // Todo: Note: the +
      float refractivity = dot(toEyeW, normal);

      // Deep water effect
      const float fadeDistance = 3000.0f;
      float deepWaterFactor = clamp((specular.b - ubo_parameters.underwaterViewDistance) / fadeDistance, 0.0f, 1.0f);
      refractionColor = mix(refractionColor, deepWaterColor, deepWaterFactor);

      vec3 finalColor = mix(reflectionColor, refractionColor, clamp(refractivity + ubo_parameters.transparency, 0.0f, 1.0f));
      OutFragColor = vec4(finalColor, 1.0f);
   }
   else
   {
      vec3 reflectionColor = texture(reflectionSampler, InTex).rgb;
      OutFragColor = vec4(reflectionColor, 1.0f);
   }
}