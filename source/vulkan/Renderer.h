#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "VulkanBase.h"
#include "VertexDescription.h"
#include "ShaderBuffer.h"

namespace ECS
{
	class RenderSystem;
}

namespace Utopian::Vk
{
	class ScreenQuad;

	enum PipelineType
	{
		PIPELINE_BASIC,
		PIPELINE_WIREFRAME,
		PIPELINE_TEST,
		PIPELINE_DEBUG
	};

	struct InstanceData {
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 color;
	};

	struct PushConstantBlock {
		PushConstantBlock() {
			world = glm::mat4();
			color = glm::vec4(1.0f);
			worldInvTranspose = glm::mat4();
			textureTiling = glm::vec2(1.0f, 1.0f);
		}

		PushConstantBlock(glm::mat4 w, glm::vec4 c = glm::vec4(1.0f), glm::vec2 tiling = glm::vec2(1.0f, 1.0f)) {
			world = w;
			color = c;
			textureTiling = tiling;

			// Note: This needs to be done to have the physical world match the rendered world.
			// See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
			world[3][0] = -world[3][0];
			world[3][1] = -world[3][1];
			world[3][2] = -world[3][2];

			worldInvTranspose = glm::inverseTranspose(world);
		}
		
		glm::mat4 world;
		glm::mat4 worldInvTranspose;
		glm::vec4 color;
		glm::vec2 textureTiling;
		glm::vec2 pad;
	};

	class Renderer : public VulkanBase
	{
	public:
		Renderer();
		~Renderer();

		void Prepare();
		void PostInitPrepare();

		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void PrepareCommandBuffers();						

		void RecordRenderingCommandBuffer(VkFramebuffer frameBuffer);

		virtual void Render();
		virtual void Update();

		// Note: This should probably be moved to some utility class
		void DrawFullscreenQuad(CommandBuffer* commandBuffer);

		void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void BeginUiUpdate();
		void EndUiUpdate();
		void ToggleUi();

		// 
		//	High level code
		//
		void CompileShaders();

		CommandBuffer* CreateCommandBuffer(VkCommandBufferLevel level);

		void SetCamera(Utopian::Camera* camera);
		Utopian::Camera* GetCamera();

		void SetClearColor(glm::vec4 color);
		glm::vec4 GetClearColor();

	private:
		CommandBuffer*					mPrimaryCommandBuffer;
		std::vector<CommandBuffer*>		mApplicationCommandBuffers;

		Camera*							mCamera;
		TextOverlay*					mTextOverlay;
		glm::vec4						mClearColor;

		UIOverlay*						mUiOverlay = nullptr;
	};
}	// VulkanLib namespace
