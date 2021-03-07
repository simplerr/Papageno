#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"
#include "vulkan/vulkanswapchain.hpp"
#include "FrameBuffers.h"
#include "Image.h"
#include "RenderPass.h"

namespace Utopian::Vk
{
   FrameBuffers::FrameBuffers(Device* device)
      : mDevice(device)
   {
   }

   FrameBuffers::FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, VulkanSwapChain* swapChain, uint32_t width, uint32_t height)
      : mDevice(device)
   {
      VkImageView attachments[2];
      attachments[1] = depthStencilImage->GetView();

      VkFramebufferCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      createInfo.renderPass = renderPass->GetVkHandle();
      createInfo.attachmentCount = 2;
      createInfo.pAttachments = attachments;
      createInfo.width = width;
      createInfo.height = height;
      createInfo.layers = 1;

      // Create a frame buffer for each swap chain image
      mFrameBuffers.resize(swapChain->imageCount);
      for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
      {
         attachments[0] = swapChain->buffers[i].view;
         Debug::ErrorCheck(vkCreateFramebuffer(mDevice->GetVkDevice(), &createInfo, nullptr, &mFrameBuffers[i]));
      }
   }

   FrameBuffers::~FrameBuffers()
   {
      for (uint32_t i = 0; i < mFrameBuffers.size(); i++)
      {
         vkDestroyFramebuffer(mDevice->GetVkDevice(), mFrameBuffers[i], nullptr);
      }
   }

   void FrameBuffers::AddAttachmentImage(Image* image)
   {
      mAttachments.push_back(image->GetView());
   }

   void FrameBuffers::AddAttachmentImage(VkImageView imageView)
   {
      mAttachments.push_back(imageView);
   }

   void FrameBuffers::Create(RenderPass* renderPass, uint32_t width, uint32_t height)
   {
      VkFramebufferCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      createInfo.renderPass = renderPass->GetVkHandle();
      createInfo.attachmentCount = (uint32_t)mAttachments.size();
      createInfo.pAttachments = mAttachments.data();
      createInfo.width = width;
      createInfo.height = height;
      createInfo.layers = 1;

      // Create a single frame buffer
      mFrameBuffers.resize(1);
      Debug::ErrorCheck(vkCreateFramebuffer(mDevice->GetVkDevice(), &createInfo, nullptr, &mFrameBuffers[0]));
   }

   VkFramebuffer FrameBuffers::GetFrameBuffer(uint32_t index) const
   {
      // [TODO] Add bound checks
      return mFrameBuffers.at(index);
   }
   VkFramebuffer FrameBuffers::GetCurrent() const
   {
      return mFrameBuffers[currentFrameBuffer];
   }
}