#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class Model;

   class AtmosphereJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(ParameterBlock)
         UNIFORM_PARAM(glm::vec3, sunDir)
         UNIFORM_PARAM(int, atmosphericScattering);
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(ConstantParameters)
         UNIFORM_PARAM(glm::mat4, projection)
         UNIFORM_PARAM(glm::mat4, world);
      UNIFORM_BLOCK_END()

      AtmosphereJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~AtmosphereJob();

      void LoadResources() override;

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PreRender(const JobInput& jobInput) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> sunImage;
      SharedPtr<Vk::Texture> environmentCube;
      SharedPtr<Vk::Texture> irradianceMap;
      SharedPtr<Vk::Texture> specularMap;
   private:
      void CaptureEnvironmentCubemap(glm::vec3 sunDir);
   private:
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mEffect;
      ParameterBlock mParameterBlock;
      ConstantParameters mConstantParameters;
      SharedPtr<Model> mSkydomeModel;
      bool mEnvironmentGenerated;
   };
}
