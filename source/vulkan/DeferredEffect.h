#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "core/CommonBuffers.h"

namespace Utopian
{
	class Light;
	struct RenderingSettings;
}

UNIFORM_BLOCK_BEGIN(DeferredEyePos)
	UNIFORM_PARAM(glm::vec4, eyePos)
UNIFORM_BLOCK_END()

namespace Utopian::Vk
{
	class DeferredEffect : public Effect
	{
	public:
		DeferredEffect(Device* device, RenderPass* renderPass);

		void SetEyePos(glm::vec3 eyePos);
		void SetLightArray(const std::vector<Light*>& lights);
		void SetSettingsData(const RenderingSettings& renderingSettings);
		void SetLightTransform(glm::mat4 viewProjection);

		virtual void UpdateMemory();

		DeferredEyePos eyeBlock;
		LightUniformBuffer light_ubo;
		SettingsUniformBuffer settings_ubo;
		CascadeBlock cascade_ubo;
	private:
	};
}
