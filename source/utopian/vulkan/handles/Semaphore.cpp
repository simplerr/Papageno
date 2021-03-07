#include "Semaphore.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
   Semaphore::Semaphore(Device* device)
      : Handle(device, vkDestroySemaphore)
   {
      VkSemaphoreCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      Debug::ErrorCheck(vkCreateSemaphore(GetVkDevice(), &createInfo, nullptr, &mHandle));
   }
}