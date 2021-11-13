#include <array>
#include "RenderPass.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
   RenderPass::RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat, VkImageLayout colorImageLayout, bool create)
      : Handle(device, vkDestroyRenderPass)
   {
      AddColorAttachment(colorFormat, colorImageLayout);
      AddDepthAttachment(depthFormat);

      if (create)
      {
         Create();
      }
   }

   RenderPass::RenderPass(Device* device)
      : Handle(device, vkDestroyRenderPass)
   {

   }

   void RenderPass::Create()
   {
      std::array<VkSubpassDependency, 2> dependencies;

      // First dependency at the start of the renderpass
      // Does the transition from final to initial layout 
      dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[0].dstSubpass = 0;
      dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

      // Second dependency at the end the renderpass
      // Does the transition from the initial to the final layout
      dependencies[1].srcSubpass = 0;
      dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
      dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
      
      // Setup a single subpass reference
      VkSubpassDescription subpassDescription = {};
      subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpassDescription.colorAttachmentCount = (uint32_t)colorReferences.size();
      subpassDescription.pColorAttachments = colorReferences.data();
      subpassDescription.pDepthStencilAttachment = depthReferences.data();
      subpassDescription.inputAttachmentCount = 0;
      subpassDescription.pInputAttachments = nullptr;
      subpassDescription.preserveAttachmentCount = 0;
      subpassDescription.pPreserveAttachments = nullptr;
      subpassDescription.pResolveAttachments = nullptr;

      // Create the actual renderpass
      VkRenderPassCreateInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      renderPassInfo.pAttachments = attachments.data();
      renderPassInfo.subpassCount = 1;
      renderPassInfo.pSubpasses = &subpassDescription;
      renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
      renderPassInfo.pDependencies = dependencies.data();
      
      Debug::ErrorCheck(vkCreateRenderPass(GetVkDevice(), &renderPassInfo, nullptr, &mHandle));
   }

   void RenderPass::AddColorAttachment(VkFormat format, VkImageLayout finalImageLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialImageLayout)
   {
      VkAttachmentReference colorReference = {};
      colorReference.attachment = (uint32_t)attachments.size();
      colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      colorReferences.push_back(colorReference);

      VkAttachmentDescription attachment = {};
      attachment.format = format;
      attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment.loadOp = loadOp;
      attachment.storeOp = storeOp;
      attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      if (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
         attachment.initialLayout = initialImageLayout; // Default is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      else
         attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      attachment.finalLayout = finalImageLayout; // This is the layout the attachment will be transitioned to, e.g VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL and VK_IMAGE_LAYOUT_PRESENT_SRC_KHR

      attachments.push_back(attachment);
   }

   void RenderPass::AddDepthAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalImageLayout, VkImageLayout initialImageLayout)
   {
      VkAttachmentReference depthReference = {};
      depthReference.attachment = (uint32_t)attachments.size();
      depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      depthReferences.push_back(depthReference);
      VkAttachmentDescription attachment = {};

      attachment.format = format;
      attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment.loadOp = loadOp;
      attachment.storeOp = storeOp;
      attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      if (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
         attachment.initialLayout = initialImageLayout;
      else
         attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      attachment.finalLayout = finalImageLayout; // This is the layout the attachment will be transitioned to, e.g VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL and VK_IMAGE_LAYOUT_PRESENT_SRC_KHR

      attachments.push_back(attachment);
   }

   uint32_t RenderPass::GetNumColorAttachments() const
   {
      return (uint32_t)colorReferences.size();
   }
}