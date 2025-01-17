#pragma once

#include "core/renderer/jobs/BaseJob.h"
#include "core/renderer/jobs/AtmosphereJob.h"
#include "vulkan/VulkanPrerequisites.h"
#include "core/renderer/CommonBuffers.h"

namespace Utopian
{
   class DeferredJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(SettingsUniformBuffer)
         UNIFORM_PARAM(glm::vec3, fogColor)
         UNIFORM_PARAM(float, fogStart)
         UNIFORM_PARAM(float, fogDistance)
         UNIFORM_PARAM(int, cascadeColorDebug)
         UNIFORM_PARAM(int, useIBL)
         UNIFORM_PARAM(float, ambientIntensity)
      UNIFORM_BLOCK_END()

      DeferredJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~DeferredJob();

      void LoadResources() override;

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

   private:
      SharedPtr<Vk::Sampler> mDepthSampler;
      SharedPtr<Vk::Effect> mPhongEffect;
      SharedPtr<Vk::Effect> mPbrEffect;
      SharedPtr<Vk::Sampler> mSampler;
      LightUniformBuffer light_ubo;
      SettingsUniformBuffer settings_ubo;
      CascadeBlock cascade_ubo;
      AtmosphereJob::ParameterBlock atmosphere_ubo;
      SharedPtr<Vk::RenderTarget> renderTarget;

      struct ImageBasedLighting {
         SharedPtr<Vk::Texture> defaultEnvironmentMap;
         SharedPtr<Vk::Texture> irradianceMap;
         SharedPtr<Vk::Texture> specularMap;
         SharedPtr<Vk::Texture> brdfLut;
      } mIbl;
   };
}
