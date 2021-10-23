#include "core/renderer/jobs/FXAAJob.h"
#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
   FXAAJob::FXAAJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      mFXXAImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "FXAA image");

      mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
      mRenderTarget->AddWriteOnlyColorAttachment(mFXXAImage);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/fxaa/fxaa.frag";

      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mSettingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);

      gScreenQuadUi().AddQuad(0u, 0u, width, height, mFXXAImage.get(), mRenderTarget->GetSampler(), 1u);
   }

   FXAAJob::~FXAAJob()
   {
   }

   void FXAAJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      TonemapJob* tonemapJob = static_cast<TonemapJob*>(jobs[JobGraph::TONEMAP_INDEX]);

      mSampler = std::make_shared<Vk::Sampler>(mDevice, false);
      mSampler->createInfo.anisotropyEnable = VK_FALSE;
      mSampler->Create();

      mEffect->BindCombinedImage("textureSampler", *tonemapJob->outputImage, *mSampler);
   }

   void FXAAJob::Render(const JobInput& jobInput)
   {
      mSettingsBlock.data.enabled = jobInput.renderingSettings.fxaaEnabled;
      mSettingsBlock.data.debug = jobInput.renderingSettings.fxaaDebug;
      mSettingsBlock.data.threshold = jobInput.renderingSettings.fxaaThreshold;
      mSettingsBlock.UpdateMemory();

      mRenderTarget->Begin("FXAA pass", glm::vec4(0.5, 0.2, 0.8, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      // Todo: Should this be moved to the effect instead?
      commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mEffect);

      gRendererUtility().DrawFullscreenQuad(commandBuffer);

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
