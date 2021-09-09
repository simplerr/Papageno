#pragma once

#include "core/renderer/jobs/BaseJob.h"

namespace Utopian
{
   class Model;

   class SkydomeJob : public BaseJob
   {
   public:

      UNIFORM_BLOCK_BEGIN(SkydomeInput)
         UNIFORM_PARAM(glm::mat4, world)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(ParameterBlock)
         UNIFORM_PARAM(float, sphereRadius)
         UNIFORM_PARAM(float, inclination)
         UNIFORM_PARAM(float, azimuth)
         UNIFORM_PARAM(float, sunSpeed)
         UNIFORM_PARAM(int, onlySun)
      UNIFORM_BLOCK_END()

      SkydomeJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~SkydomeJob();

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

      SharedPtr<Vk::Image> sunImage;
   private:
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mEffect;
      SkydomeInput mInputBlock;
      ParameterBlock mParameterBlock;
      SharedPtr<Model> mSkydomeModel;
   };
}
