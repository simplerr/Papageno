#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vulkan/RenderTarget.h>
#include <vulkan/handles/Semaphore.h>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"

using namespace Utopian;

class MiniCamera;

class MarchingCubes
{
public:
	UNIFORM_BLOCK_BEGIN(InputParameters)
		UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_PARAM(glm::vec4, offsets[8])
		UNIFORM_PARAM(glm::vec4, color)
		UNIFORM_PARAM(float, voxelSize)
		UNIFORM_PARAM(float, time)
	UNIFORM_BLOCK_END()

	MarchingCubes(Utopian::Window* window);
	~MarchingCubes();

	void Run();

	void DestroyCallback();
	void UpdateCallback();
	void DrawCallback();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitResources();

	Vk::VulkanApp* mVulkanApp;
	Utopian::Window* mWindow;

	// Test
	SharedPtr<Vk::Effect> mEffect;
	SharedPtr<Vk::Semaphore> mRayTraceComplete;
	SharedPtr<Vk::Image> mOutputImage;
	SharedPtr<Vk::Sampler> mSampler;

	SharedPtr<MiniCamera> mCamera;
	InputParameters mInputParameters;
};
