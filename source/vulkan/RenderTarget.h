#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class RenderTarget
	{
	public:
		RenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height);
		~RenderTarget();

		void Begin();
		void End(Utopian::Vk::Queue* queue);

		void SetClearColor(float r, float g, float b, float a = 0.0f);

		/* Note: The order in which these are called is important */
		void AddColorAttachment(Image* image,
								VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddDepthAttachment(Image* image,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddColorAttachment(const SharedPtr<Image>& image,
		 						VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void AddDepthAttachment(const SharedPtr<Image>& image,
								VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
								VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE);

		void Create();

		Utopian::Vk::Sampler* GetSampler();
		Utopian::Vk::CommandBuffer* GetCommandBuffer();
		RenderPass* GetRenderPass();

		uint32_t GetWidth();
		uint32_t GetHeight();

	private:
		FrameBuffers* mFrameBuffer;
		RenderPass* mRenderPass;
		CommandBuffer* mCommandBuffer;
		DescriptorSet* mTextureDescriptorSet;
		SharedPtr<Sampler> mSampler;
		uint32_t mWidth, mHeight;
		glm::vec4 mClearColor;
		std::vector<VkClearValue> mClearValues;
	};
}
