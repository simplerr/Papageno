#include "vulkan/MarchingCubesTerrainEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Vertex.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	MarchingCubesTerrainEffect::MarchingCubesTerrainEffect()
	{
		SetPipeline(PipelineType2::WIREFRAME);
	}

	void MarchingCubesTerrainEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
		mDescriptorPool->Create();
	}

	void MarchingCubesTerrainEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = TerrainVertex::GetDescription();
	}

	void MarchingCubesTerrainEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface = std::make_shared<PipelineInterface>(device);
		mPipelineInterface->AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);						// per_frame_vs UBO
		mPipelineInterface->AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface->AddUniformBuffer(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface->AddUniformBuffer(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface->AddCombinedImageSampler(SET_1, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface->AddPushConstantRange(sizeof(PushConstantBasicBlock), VK_SHADER_STAGE_VERTEX_BIT);	// pushConsts
		mPipelineInterface->Create();
	}

	void MarchingCubesTerrainEffect::CreateDescriptorSets(Device* device)
	{
		
	}

	void MarchingCubesTerrainEffect::CreatePipeline(Device* device, RenderPass* renderPass)
	{
		Utopian::Vk::Shader* shader = gShaderFactory().CreateShader("data/shaders/terrain/terrain.vert.spv", "data/shaders/terrain/terrain.frag.spv");

		Pipeline2* pipeline = new Utopian::Vk::Pipeline2(device, renderPass, mVertexDescription, shader);
		pipeline->SetPipelineInterface(mPipelineInterface.get());
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType2::WIREFRAME] = pipeline;

		Pipeline2* pipeline1 = new Utopian::Vk::Pipeline2(device, renderPass, mVertexDescription, shader);
		pipeline1->SetPipelineInterface(mPipelineInterface.get());
		pipeline1->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline1->Create();
		mPipelines[PipelineType2::SOLID] = pipeline1;
	}

	void MarchingCubesTerrainEffect::UpdateMemory()
	{
	}
}