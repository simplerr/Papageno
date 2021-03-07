#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
   PipelineInterface::PipelineInterface(Device* device)
   {
      mDevice = device;
   }

   PipelineInterface::~PipelineInterface()
   {
      vkDestroyPipelineLayout(mDevice->GetVkDevice(), mPipelineLayout, nullptr);
   }

   void PipelineInterface::Create()
   {
      // Create all VkDescriptorSetLayouts
      for (auto& setLayout : mDescriptorSetLayouts)
      {
         setLayout.second->Create();
      }

      std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
      for (auto& setLayout : mDescriptorSetLayouts)
      {
         descriptorSetLayouts.push_back(setLayout.second->GetVkHandle());
      }

      // Create a VkPipelineLayout from all the VkDescriptorSetLayouts and Push Constant ranges
      VkPipelineLayoutCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      createInfo.pNext = NULL;
      createInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
      createInfo.pSetLayouts = descriptorSetLayouts.data();

      if (mPushConstantRanges.size() > 0)
      {
         createInfo.pushConstantRangeCount = (uint32_t)mPushConstantRanges.size();
         createInfo.pPushConstantRanges = mPushConstantRanges.data();
      }

      Debug::ErrorCheck(vkCreatePipelineLayout(mDevice->GetVkDevice(), &createInfo, nullptr, &mPipelineLayout));
   }

   void PipelineInterface::AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
   }

   void PipelineInterface::AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
   }

   void PipelineInterface::AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet, binding, descriptorCount, stageFlags);
   }

   void PipelineInterface::AddStorageImage(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorSet, binding, descriptorCount, stageFlags);
   }

   void PipelineInterface::AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset)
   {
      VkPushConstantRange pushConstantRange = {};
      pushConstantRange.stageFlags = shaderStage;
      pushConstantRange.size = size;
      pushConstantRange.offset = offset;

      mPushConstantRanges.push_back(pushConstantRange);
   }

   DescriptorSetLayout* PipelineInterface::GetDescriptorSetLayout(uint32_t descriptorSet)
   {
      if (mDescriptorSetLayouts.find(descriptorSet) == mDescriptorSetLayouts.end())
         assert(0);

      return mDescriptorSetLayouts[descriptorSet].get();
   }

   VkDescriptorSetLayout PipelineInterface::GetVkDescriptorSetLayout(uint32_t descriptorSet)
   {
      if (mDescriptorSetLayouts.find(descriptorSet) == mDescriptorSetLayouts.end())
         assert(0);

      return mDescriptorSetLayouts[descriptorSet]->GetVkHandle();
   }

   VkPipelineLayout PipelineInterface::GetPipelineLayout() const
   {
      return mPipelineLayout;
   }

   void PipelineInterface::AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
   {
      if (mDescriptorSetLayouts.find(descriptorSet) == mDescriptorSetLayouts.end())
      {
         mDescriptorSetLayouts[descriptorSet] = std::make_shared<DescriptorSetLayout>(mDevice);
      }

      mDescriptorSetLayouts[descriptorSet]->AddBinding(binding, descriptorType, descriptorCount, stageFlags);
   }

   uint32_t PipelineInterface::GetNumDescriptorSets() const
   {
      return (uint32_t)mDescriptorSetLayouts.size();
   }
}