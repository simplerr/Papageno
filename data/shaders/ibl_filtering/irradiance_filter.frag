// Generates an irradiance cube from an environment map using convolution
// Source: https://learnopengl.com/PBR/IBL/Diffuse-irradiance

#version 450

layout (location = 0) in vec3 InPos;
layout (location = 1) in float InRoughness;
layout (location = 0) out vec4 OutColor;

layout (set = 0, binding = 0) uniform samplerCube samplerEnv;

#define PI 3.1415926535897932384626433832795

void main()
{
   vec3 irradiance = vec3(0.0);

   vec3 normal = normalize(InPos);
   vec3 up    = vec3(0.0, 1.0, 0.0);
   vec3 right = normalize(cross(up, normal));
   up         = normalize(cross(normal, right));

   float sampleDelta = 0.025;
   float nrSamples = 0.0;
   for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
   {
      for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
      {
         // Spherical to cartesian (in tangent space)
         vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
         // Tangent space to world
         vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

         irradiance += texture(samplerEnv, sampleVec).rgb * cos(theta) * sin(theta);
         nrSamples++;
      }
   }
   irradiance = PI * irradiance * (1.0 / float(nrSamples));

   OutColor = vec4(irradiance, 1.0f);
}
