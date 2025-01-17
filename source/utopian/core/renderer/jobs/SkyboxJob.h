#pragma once

#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/Effect.h"

namespace Utopian
{
   class Model;

   class SkyboxJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(SkyboxInput)
         UNIFORM_PARAM(glm::mat4, world)
      UNIFORM_BLOCK_END()

      SkyboxJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~SkyboxJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> sunImage;
   private:
      SharedPtr<Vk::Texture> mTexture;
      SharedPtr<Vk::Texture> mIrradianceMap;
      SharedPtr<Vk::Texture> mSpecularMap;
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mEffect;
      SharedPtr<Model> mCubeModel;
      SkyboxInput mInputBlock;
   };
}