#include <array>
#include <time.h>
#include <cstdlib>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "VulkanDebug.h"
#include "StaticModel.h"
#include "Camera.h"
#include "VulkanHelpers.h"
#include "Light.h"
#include "TextureLoader.h"
#include "Device.h"
#include "ShaderManager.h"
#include "TextOverlay.h"
#include "handles/DescriptorSet.h"
#include "handles/CommandBuffer.h"
#include "handles/CommandPool.h"
#include "handles/Pipeline.h"
#include "handles/Fence.h"
#include "handles/PipelineLayout.h"
#include "handles/RenderPass.h"
#include "handles/FrameBuffers.h"
#include "handles/Queue.h"
#include "handles/DescriptorSetLayout.h"
#include "ecs/systems/RenderSystem.h"

#define VK_FLAGS_NONE 0
#define VERTEX_BUFFER_BIND_ID 0
#define VULKAN_ENABLE_VALIDATION true		// Debug validation layers toggle (affects performance a lot)

namespace VulkanLib
{
	Renderer::Renderer() : VulkanBase(VULKAN_ENABLE_VALIDATION)
	{
		srand(time(NULL));
		mCamera = nullptr;
	}

	Renderer::~Renderer()
	{
		mVertexUniformBuffer.Cleanup(GetVkDevice());
		mFragmentUniformBuffer.Cleanup(GetVkDevice());

		delete mDescriptorPool;

		delete mCameraDescriptorSetLayout;
		delete mLightDescriptorSetLayout;
		delete mTextureDescriptorSetLayout;

		for (auto const& pipeline : mPipelines)
		{
			delete pipeline.second;
		}

		delete mPipelineLayout;

		delete mRenderFence;

		delete mPrimaryCommandBuffer;
		delete mSecondaryCommandBuffer;

		for (CommandBuffer* commandBuffer : mApplicationCommandBuffers)
		{
			delete commandBuffer;
		}

		delete mShaderManager;

		delete mTextOverlay;
		delete mTextureLoader;
	}

	void Renderer::Prepare()
	{
		VulkanBase::Prepare();

		mRenderFence = new Fence(mDevice, VK_FLAGS_NONE);
		mShaderManager = new ShaderManager(mDevice);

		SetupVertexDescriptions();			
		SetupDescriptorSetLayout();			// Must run before PreparePipelines() (VkPipelineLayout)
		PreparePipelines();
		PrepareUniformBuffers();			// Must run before SetupDescriptorSet() (Creates the uniform buffer)

		SetupDescriptorPool();
		SetupDescriptorSet();
		PrepareCommandBuffers();

		mTextureLoader = new VulkanLib::TextureLoader(this, GetQueue()->GetVkHandle());
		mTextOverlay = new TextOverlay(this);
		mPrepared = true;
	}

	void Renderer::PrepareCommandBuffers()
	{
		// Create the primary and secondary command buffers
		mPrimaryCommandBuffer = new CommandBuffer(mDevice, GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		mSecondaryCommandBuffer = new CommandBuffer(mDevice, GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	void Renderer::CompileShaders()
	{
		// [TODO] Move to ShaderManager
		system("cd data/shaders/phong/ && generate-spirv.bat");
		system("cd data/shaders/test/ && generate-spirv.bat");
		system("cd data/shaders/geometry/ && generate-spirv.bat");
		system("cd data/shaders/textoverlay/ && generate-spirv.bat");
	}

	void Renderer::SetCamera(Camera* camera)
	{
		mCamera = camera;
	}

	Pipeline* Renderer::GetPipeline(PipelineType pipelineType)
	{
		// TODO: Add boundary check
		return mPipelines[pipelineType];
	}

	PipelineLayout* Renderer::GetPipelineLayout()
	{
		return mPipelineLayout;
	}

	DescriptorSetLayout* Renderer::GetTextureDescriptorSetLayout()
	{
		return mTextureDescriptorSetLayout;
	}

	DescriptorPool* Renderer::GetDescriptorPool()
	{
		return mDescriptorPool;
	}

	VertexDescription* Renderer::GetVertexDescription()
	{
		return &mVertexDescription;
	}

	CommandBuffer* Renderer::CreateCommandBuffer(VkCommandBufferLevel level)
	{
		CommandBuffer* commandBuffer = new CommandBuffer(mDevice, GetCommandPool(), level);
		mApplicationCommandBuffers.push_back(commandBuffer);
		return commandBuffer;
	}
	
	void Renderer::PrepareUniformBuffers()
	{
		// Create the fragment shader uniform buffer
		Light light;
		light.SetMaterials(vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(1, -1, 1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.8f, 1.0f);
		light.SetType(LightType::DIRECTIONAL_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mFragmentUniformBuffer.lights.push_back(light);

		light.SetMaterials(vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(-1, -1, -1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.5f, 1.0f);
		light.SetType(LightType::SPOT_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mFragmentUniformBuffer.lights.push_back(light);

		// Important to call this before CreateBuffer() since # lights affects the total size
		mFragmentUniformBuffer.constants.numLights = mFragmentUniformBuffer.lights.size();

		mFragmentUniformBuffer.CreateBuffer(this, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		// Create the vertex shader uniform buffer
		mVertexUniformBuffer.CreateBuffer(this, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		UpdateUniformBuffers();
	}

	// Call this every time any uniform buffer should be updated (view changes etc.)
	void Renderer::UpdateUniformBuffers()
	{
		if (mCamera != nullptr)
		{
			mVertexUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mVertexUniformBuffer.camera.viewMatrix = mCamera->GetView();
			mVertexUniformBuffer.camera.projectionMatrix = mCamera->GetProjection();
			mVertexUniformBuffer.camera.eyePos = mCamera->GetPosition();
		}

		mVertexUniformBuffer.UpdateMemory(GetVkDevice());
		mFragmentUniformBuffer.UpdateMemory(GetVkDevice());
	}

	void Renderer::SetupDescriptorSetLayout()
	{
		// Here we want to split the descriptor set into 3 different ones
		// The PipelineLayout can take a an array of several DescriptorSetLayout
		// And when rendering vkCmdBindDescriptorSets also takes an array of DescriptorSets

		mCameraDescriptorSetLayout = new DescriptorSetLayout(GetDevice());
		mLightDescriptorSetLayout = new DescriptorSetLayout(GetDevice());
		mTextureDescriptorSetLayout = new DescriptorSetLayout(GetDevice());

		// TODO: Separate DescriptorSet and DescriptorSetLayout
		// DescriptorSets should be able to be created from an existing DescriptorSetLayout
		mCameraDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);					// Uniform buffer binding: 0
		mLightDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);					// Uniform buffer binding: 1
		mTextureDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);			// Combined image sampler binding: 2

		mCameraDescriptorSetLayout->Create();
		mLightDescriptorSetLayout->Create();
		mTextureDescriptorSetLayout->Create();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.push_back(mCameraDescriptorSetLayout->GetVkHandle());
		descriptorSetLayouts.push_back(mLightDescriptorSetLayout->GetVkHandle());
		descriptorSetLayouts.push_back(mTextureDescriptorSetLayout->GetVkHandle());

		PushConstantRange pushConstantRange = PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstantBlock));
		mPipelineLayout = new PipelineLayout(mDevice, descriptorSetLayouts, &pushConstantRange);
	}

	void Renderer::SetupDescriptorPool()
	{
		mDescriptorPool = new DescriptorPool(GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_NUM_TEXTURES);
		mDescriptorPool->Create();
	}

	// [TODO] Let each thread have a separate descriptor set!!
	void Renderer::SetupDescriptorSet()
	{
		mCameraDescriptorSet = new DescriptorSet(GetDevice(), mCameraDescriptorSetLayout, mDescriptorPool);
		mLightDescriptorSet = new DescriptorSet(GetDevice(), mLightDescriptorSetLayout, mDescriptorPool);

		mCameraDescriptorSet->AllocateDescriptorSets();
		mLightDescriptorSet->AllocateDescriptorSets();

		mCameraDescriptorSet->BindUniformBuffer(0, &mVertexUniformBuffer.GetDescriptor());
		mLightDescriptorSet->BindUniformBuffer(0, &mFragmentUniformBuffer.GetDescriptor());

		mCameraDescriptorSet->UpdateDescriptorSets();
		mLightDescriptorSet->UpdateDescriptorSets();
	}

	void Renderer::PreparePipelines()
	{
		// Load shader
		// [TODO] Move this into Pipeline?
		Shader* shader = mShaderManager->CreateShader("data/shaders/phong/phong.vert.spv", "data/shaders/phong/phong.frag.spv");
			
		// Solid pipeline
		Pipeline*  pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, shader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_BASIC] = pipeline;

		// Wireframe pipeline
		pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, shader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_WIREFRAME] = pipeline;

		// Test pipeline
		Shader* testShader = mShaderManager->CreateShader("data/shaders/test/test.vert.spv", "data/shaders/test/test.frag.spv");
		pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, testShader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_TEST] = pipeline;

		pipeline = new Pipeline(mDevice, mPipelineLayout, mRenderPass, &mVertexDescription, testShader);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		// TODO: Disable depth test
		pipeline->Create();
		mPipelines[PipelineType::PIPELINE_DEBUG] = pipeline;
	}

	void Renderer::SetupVertexDescriptions()
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription.AddBinding(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);					// Per vertex

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 0 : Position
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 1 : Color
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec3Attribute());	// Location 2 : Normal
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec2Attribute());	// Location 3 : Texture
		mVertexDescription.AddAttribute(VERTEX_BUFFER_BIND_ID, Vec4Attribute());	// Location 4 : Tangent
	}

	void Renderer::RecordRenderingCommandBuffer(VkFramebuffer frameBuffer)
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = GetWindowWidth();
		renderPassBeginInfo.renderArea.extent.height = GetWindowHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer;

		// Begin command buffer recording & the render pass
		mPrimaryCommandBuffer->Begin();
		vkCmdBeginRenderPass(mPrimaryCommandBuffer->GetVkHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);	// VK_SUBPASS_CONTENTS_INLINE

		std::vector<VkCommandBuffer> commandBuffers;
		for (CommandBuffer* commandBuffer : mApplicationCommandBuffers)
		{
			commandBuffers.push_back(commandBuffer->GetVkHandle());
		}

		// This is where multithreaded command buffers can be added
		// ...

		// Execute render commands from the secondary command buffer
		vkCmdExecuteCommands(mPrimaryCommandBuffer->GetVkHandle(), commandBuffers.size(), commandBuffers.data());

		// End command buffer recording & the render pass
		vkCmdEndRenderPass(mPrimaryCommandBuffer->GetVkHandle());
		mPrimaryCommandBuffer->End();
	}

	void Renderer::Draw()
	{
		// When presenting (vkQueuePresentKHR) the swapchain image has to be in the VK_IMAGE_LAYOUT_PRESENT_SRC_KHR format
		// When rendering to the swapchain image has to be in the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// The transition between these to formats is performed by using image memory barriers (VkImageMemoryBarrier)
		// VkImageMemoryBarrier have oldLayout and newLayout fields that are used 
		RecordRenderingCommandBuffer(mFrameBuffers->GetCurrent());

		mQueue->Submit(mPrimaryCommandBuffer, mRenderFence);

		// Wait for all command buffers to complete
		mRenderFence->Wait(); 
	}

	void Renderer::Render()
	{
		mCamera->Update();

		if (mPrepared) {
			UpdateUniformBuffers();
			Draw();
		}
	}

	void Renderer::Update()
	{
		mTextOverlay->beginTextUpdate();
		mTextOverlay->addText("Test string", 5.0f, 65.0f, TextOverlay::alignLeft);
		mTextOverlay->addText("This is cool", 500.0f, 650.0f, TextOverlay::alignLeft);
		mTextOverlay->endTextUpdate();
	}

	void Renderer::HandleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Default message handling
		VulkanBase::HandleMessages(hwnd, msg, wParam, lParam);

		// Let the camera handle user input
		mCamera->HandleMessages(hwnd, msg, wParam, lParam);
	}
}	// VulkanLib namespace