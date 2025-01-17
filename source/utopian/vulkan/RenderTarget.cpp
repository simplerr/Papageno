#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/QueryPoolTimestamp.h"
#include "vulkan/handles/QueryPoolStatistics.h"
#include "vulkan/Debug.h"
#include "core/Profiler.h"

namespace Utopian::Vk
{
   RenderTarget::RenderTarget(Device* device, uint32_t width, uint32_t height)
   {
      mWidth = width;
      mHeight = height;

      mCommandBuffer = std::make_shared<CommandBuffer>(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
      mRenderPass = std::make_shared<RenderPass>(device);
      mFrameBuffer = std::make_shared<FrameBuffers>(device);
      mSampler = std::make_shared<Sampler>(device);
      mTimestampQueryPool = std::make_shared<Vk::QueryPoolTimestamp>(device);
      
      SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   }

   RenderTarget::~RenderTarget()
   {

   }

   void RenderTarget::AddReadWriteColorAttachment(const SharedPtr<Image>& image, VkImageLayout finalImageLayout, VkImageLayout initialImageLayout)
   {
      mRenderPass->AddColorAttachment(image->GetFormat(), finalImageLayout, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, initialImageLayout);
      mFrameBuffer->AddAttachmentImage(image.get());
   }

   void RenderTarget::AddWriteOnlyColorAttachment(const SharedPtr<Image>& image, VkImageLayout finalImageLayout, VkImageLayout initialImageLayout)
   {
      mRenderPass->AddColorAttachment(image->GetFormat(), finalImageLayout, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, initialImageLayout);
      mFrameBuffer->AddAttachmentImage(image.get());
   }

   void RenderTarget::AddReadWriteDepthAttachment(const SharedPtr<Image>& image, VkImageLayout finalImageLayout, VkImageLayout initialImageLayout)
   {
      mRenderPass->AddDepthAttachment(image->GetFormat(), VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, finalImageLayout, initialImageLayout);
      mFrameBuffer->AddAttachmentImage(image.get());
   }

   void RenderTarget::AddWriteOnlyDepthAttachment(const SharedPtr<Image>& image, VkImageLayout finalImageLayout, VkImageLayout initialImageLayout)
   {
      mRenderPass->AddDepthAttachment(image->GetFormat(), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, finalImageLayout, initialImageLayout);
      mFrameBuffer->AddAttachmentImage(image.get());
   }

   void RenderTarget::AddColorAttachment(VkImageView imageView, VkFormat format, VkImageLayout finalImageLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
   {
      mRenderPass->AddColorAttachment(format, finalImageLayout, loadOp, storeOp);
      mFrameBuffer->AddAttachmentImage(imageView);
   }

   void RenderTarget::AddDepthAttachment(VkImageView imageView, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
   {
      mRenderPass->AddDepthAttachment(format, loadOp, storeOp);
      mFrameBuffer->AddAttachmentImage(imageView);
   }

   void RenderTarget::Create()
   {
      mRenderPass->Create();
      mFrameBuffer->Create(mRenderPass.get(), GetWidth(), GetHeight());

      for (uint32_t i = 0; i < mRenderPass->GetNumColorAttachments(); i++)
      {
         VkClearValue clearValue;
         clearValue.color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
         mClearValues.push_back(clearValue);
      }

      // Note: Always assumes that the depth stencil attachment is the last one
      VkClearValue clearValue;
      clearValue.depthStencil = { 1.0f, 0 };
      mClearValues.push_back(clearValue);
   }

   void RenderTarget::Begin(std::string debugName, glm::vec4 debugColor)
   {
      BeginCommandBuffer();
      BeginDebugLabelAndQueries(debugName, debugColor);
      BeginRenderPass();
   }

   void RenderTarget::BeginCommandBuffer()
   {
      mCommandBuffer->Begin();
   }
   
   void RenderTarget::BeginRenderPass()
   {
      VkRenderPassBeginInfo renderPassBeginInfo = {};
      renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
      renderPassBeginInfo.renderArea.extent.width = GetWidth();
      renderPassBeginInfo.renderArea.extent.height = GetHeight();
      renderPassBeginInfo.clearValueCount = (uint32_t)mClearValues.size();
      renderPassBeginInfo.pClearValues = mClearValues.data();
      renderPassBeginInfo.framebuffer = mFrameBuffer->GetFrameBuffer(0); // TODO: NOTE: Should not be like this

      mCommandBuffer->CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      mCommandBuffer->CmdSetViewPort((float)GetWidth(), (float)GetHeight());
      mCommandBuffer->CmdSetScissor(GetWidth(), GetHeight());
   }

   void RenderTarget::End(const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore)
   {
      mCommandBuffer->CmdEndRenderPass();

      EndDebugLabelAndQueries();

      mCommandBuffer->Submit(waitSemaphore, signalSemaphore);

      if (gProfiler().IsEnabled())
      {   
         gProfiler().AddProfilerTask(mDebugName, mTimestampQueryPool->GetElapsedTime(), mDebugColor);

         if (mStatisticsQueryPool != nullptr)
             mStatisticsQueryPool->RetreiveResults();
      }
   }

   void RenderTarget::EndAndFlush()
   {
      mCommandBuffer->CmdEndRenderPass();
      
      EndDebugLabelAndQueries();

      mCommandBuffer->Flush();
   }

   uint32_t RenderTarget::GetWidth()
   {
      return mWidth;
   }

   uint32_t RenderTarget::GetHeight()
   {
      return mHeight;
   }

   void RenderTarget::BeginDebugLabelAndQueries(std::string debugName, glm::vec4 debugColor)
   {
      mDebugName = debugName;
      mDebugColor = debugColor;

      Vk::DebugLabel::BeginRegion(mCommandBuffer->GetVkHandle(), mDebugName.c_str(), mDebugColor);

      mTimestampQueryPool->Reset(mCommandBuffer.get());
      mTimestampQueryPool->Begin(mCommandBuffer.get());

      if (mStatisticsQueryPool != nullptr)
      {
         mStatisticsQueryPool->Reset(mCommandBuffer.get());
         mStatisticsQueryPool->Begin(mCommandBuffer.get());
      }
   }

   void RenderTarget::EndDebugLabelAndQueries()
   {
      Vk::DebugLabel::EndRegion(mCommandBuffer->GetVkHandle());

      mTimestampQueryPool->End(mCommandBuffer.get());

      if (mStatisticsQueryPool != nullptr)
      {
         mStatisticsQueryPool->End(mCommandBuffer.get());
      }
   }

   Utopian::Vk::Sampler* RenderTarget::GetSampler()
   {
      return mSampler.get();
   }

   Utopian::Vk::CommandBuffer* RenderTarget::GetCommandBuffer()
   {
      return mCommandBuffer.get();
   }

   RenderPass* RenderTarget::GetRenderPass()
   {
      return mRenderPass.get();
   }

   void RenderTarget::SetClearColor(float r, float g, float b, float a)
   {
      mClearColor = glm::vec4(r, g, b, a);
   }

   void RenderTarget::AddStatisticsQuery(SharedPtr<Vk::QueryPoolStatistics> statisticsQuery)
   {
      mStatisticsQueryPool = statisticsQuery;
   }
}