#pragma once

#include "vulkan/VulkanInclude.h"
#include <vector>
#include "../external/vk_mem_alloc.h"
#include "Handle.h"

namespace Utopian::Vk
{
   struct BUFFER_CREATE_INFO
   {
      VkBufferUsageFlags usageFlags;
      VkMemoryPropertyFlags memoryPropertyFlags;
      VkDeviceSize size;
      void* data;
   };

   class Buffer : public Handle<VkBuffer>
   {
   public:
      Buffer(Device* device);
      Buffer(const BUFFER_CREATE_INFO& createInfo, Device* device);

      Buffer(Device* device,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize size,
            void* data);
      ~Buffer();

      void Create(Device* device,
                  VkBufferUsageFlags usageFlags,
                  VkMemoryPropertyFlags memoryPropertyFlags,
                  VkDeviceSize size,
                  void* data = nullptr);

      void Destroy();

      void MapMemory(void** data);
      void UnmapMemory();
      void UpdateMemory(void* data, VkDeviceSize size);

      void Copy(CommandBuffer* commandBuffer, Image* destination, const std::vector<VkBufferImageCopy>& regions);

   private:
      VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
      Device* mDevice;

      /* Device memory allocation. */
      VmaAllocation mAllocation;

      bool mMapped = false;
   };
}
