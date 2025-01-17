#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/jobs/BloomJob.h"
#include "core/renderer/jobs/DepthOfFieldJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
   TonemapJob::TonemapJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
   }

   TonemapJob::~TonemapJob()
   {
   }

   void TonemapJob::LoadResources()
   {
      auto loadShader = [&]()
      {
         Vk::EffectCreateInfo effectDesc;
         effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
         effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/tonemap.frag";
         mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);
      };

      loadShader();
   }

   void TonemapJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      outputImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT, "Tonemap image");

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddWriteOnlyColorAttachment(outputImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->SetClearColor(1, 1, 1, 1);
      mRenderTarget->Create();
      mSampler = std::make_shared<Vk::Sampler>(mDevice, false);
      mSampler->createInfo.anisotropyEnable = VK_FALSE;
      mSampler->Create();
   }

   void TonemapJob::PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      BloomJob* bloomJob = static_cast<BloomJob*>(jobs[JobGraph::BLOOM_INDEX]);
      DepthOfFieldJob* depthOfFieldJob = static_cast<DepthOfFieldJob*>(jobs[JobGraph::DOF_INDEX]);

      mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);

      mEffect->BindCombinedImage("hdrSampler", *depthOfFieldJob->outputImage, *mSampler);
      mEffect->BindCombinedImage("bloomSampler", *bloomJob->outputImage, *mSampler);
   }

   void TonemapJob::Render(const JobInput& jobInput)
   {
      mSettingsBlock.data.tonemapping = jobInput.renderingSettings.tonemapping;
      mSettingsBlock.data.exposure = jobInput.renderingSettings.exposure;
      mSettingsBlock.UpdateMemory();

      mRenderTarget->Begin("Tonemap pass", glm::vec4(0.5, 1.0, 1.0, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      // Todo: Should this be moved to the effect instead?
      commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mEffect);

      gRendererUtility().DrawFullscreenQuad(commandBuffer);

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
