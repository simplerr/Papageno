#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Light.h"
#include "core/renderer/Primitive.h"
#include "core/renderer/Model.h"

namespace Utopian
{
   SkydomeJob::SkydomeJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      mParameterBlock.data.inclination = 90.0f;
      mParameterBlock.data.azimuth = 0.0f;

      sunImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM, "Skydome sun image");
   }

   SkydomeJob::~SkydomeJob()
   {
   }

   void SkydomeJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      mRenderTarget->AddWriteOnlyColorAttachment(sunImage);
      mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      mRenderTarget->SetClearColor(0, 0, 0);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/skydome/skydome.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/skydome/skydome.frag";
      effectDesc.pipelineDesc.depthStencilState.depthWriteEnable = VK_FALSE;
      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mEffect->BindUniformBuffer("UBO_input", mInputBlock);
      mEffect->BindUniformBuffer("UBO_parameters", mParameterBlock);

      mSkydomeModel = Vk::gModelLoader().LoadModel2("data/models/sphere.obj");

      // const uint32_t size = 240;
      // gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (2 * size + 10), size, size, sunImage.get(), mRenderTarget->GetSampler());
   }

   void SkydomeJob::Render(const JobInput& jobInput)
   {
      // Removes the translation components of the matrix to always keep the skydome at the same distance
      glm::mat4 world = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      mInputBlock.data.world = glm::scale(world, glm::vec3(16.0f));
      mInputBlock.UpdateMemory();

      mParameterBlock.data.sphereRadius = mSkydomeModel->GetBoundingBox().GetHeight() / 2.0f;
      mParameterBlock.data.inclination = glm::radians(jobInput.renderingSettings.sunInclination);
      mParameterBlock.data.azimuth = jobInput.sceneInfo.sunInfo.azimuth;
      mParameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
      mParameterBlock.data.onlySun = false;
      mParameterBlock.UpdateMemory();

      mRenderTarget->Begin("Skydome pass", glm::vec4(0.3, 0.8, 1.0, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      // Todo: Should this be moved to the effect instead?
      commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mEffect);

      Primitive* primitive = mSkydomeModel->GetFirstPrimitive();
      commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
      commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
      commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
