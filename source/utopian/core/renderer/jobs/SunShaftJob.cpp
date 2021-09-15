#include "core/renderer/jobs/SunShaftJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/AtmosphereJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Model.h"
#include "core/Camera.h"

namespace Utopian
{
   SunShaftJob::SunShaftJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   SunShaftJob::~SunShaftJob()
   {
   }

   void SunShaftJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      // A workaround since both SkydomeJob and Atmosphere can produce the sun image,
      // depending on the job graph configuration
      SharedPtr<Vk::Image> sunImage = nullptr;
      SkydomeJob* skydomeJob = dynamic_cast<SkydomeJob*>(jobs[JobGraph::SKYDOME_INDEX]);
      if (skydomeJob != nullptr)
      {
         sunImage = skydomeJob->sunImage;
      }
      else
      {
         AtmosphereJob* atmosphereJob = dynamic_cast<AtmosphereJob*>(jobs[JobGraph::SKYDOME_INDEX]);
         sunImage = atmosphereJob->sunImage;
      }

      assert(sunImage);

      // Note: Todo: Probably don't need to be the native window size
      mRadialBlurRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRadialBlurRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      mRadialBlurRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/sun_shafts/sun_shafts.frag";
      effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ADDITIVE;

      mRadialBlurEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRadialBlurRenderTarget->GetRenderPass(), effectDesc);

      mRadialBlurParameters.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mRadialBlurEffect->BindUniformBuffer("UBO_parameters", mRadialBlurParameters);
      mRadialBlurEffect->BindCombinedImage("sunSampler", *sunImage, *mRadialBlurRenderTarget->GetSampler());

      mSkydomeModel = gModelLoader().LoadModel("data/models/sphere.obj");
   }

   void SunShaftJob::Render(const JobInput& jobInput)
   {
      float sunInclination = glm::radians(jobInput.renderingSettings.sunInclination);
      glm::vec3 sunDir = jobInput.sceneInfo.sunInfo.direction;
      float skydomeRadius = mSkydomeModel->GetBoundingBox().GetWidth() / 2.0f;

      // Note: Todo: Why -X here?
      // Note: Todo: Why - camera position?
      glm::vec3 sunWorldPos = glm::vec3(-1, 1, 1) * sunDir * skydomeRadius * mSkydomeScale - jobInput.sceneInfo.eyePos;
      glm::vec4 clipPos = jobInput.sceneInfo.projectionMatrix * jobInput.sceneInfo.viewMatrix * glm::vec4(sunWorldPos, 1.0f);
      glm::vec4 ndcPos = clipPos / clipPos.w;
      glm::vec2 texCoord = glm::vec2(ndcPos) / 2.0f + 0.5f;

      // Apply post process effect
      mRadialBlurParameters.data.radialOrigin = texCoord;
      mRadialBlurParameters.data.radialBlurScale = 1.0f;
      mRadialBlurParameters.data.radialBlurStrength = 1.0f;
      mRadialBlurParameters.UpdateMemory();

      mRadialBlurRenderTarget->Begin("Sun shaft pass", glm::vec4(0.7, 0.7, 0.0, 1.0));
      Vk::CommandBuffer* commandBuffer = mRadialBlurRenderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         commandBuffer->CmdBindPipeline(mRadialBlurEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mRadialBlurEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);
      }

      mRadialBlurRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
