#include "CommandPool.h"
#include "VulkanDebug.h"
#include "Device.h"

namespace VulkanLib
{
	CommandPool::CommandPool()
		: Handle(vkDestroyCommandPool)
	{

	}

	CommandPool::CommandPool(VkDevice device, uint32_t queueFamilyIndex)
	{

	}

	void CommandPool::Create(VkDevice device, uint32_t queueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndex;									// NOTE: TODO: Need to store this as a member (Use Swapchain)!!!!!
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VulkanDebug::ErrorCheck(vkCreateCommandPool(device, &createInfo, nullptr, &mHandle));
	}
}
