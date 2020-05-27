#include "vulkan/MarchingCubesEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	MarchingCubesEffect::MarchingCubesEffect()
	{
		mCounterSSBO.numVertices = 0;
	}

	void MarchingCubesEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NUM_MAX_STORAGE_BUFFERS); // NOTE:
		mDescriptorPool->Create();
	}

	void MarchingCubesEffect::CreateVertexDescription(Device* device)
	{
	}

	void MarchingCubesEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface = std::make_shared<PipelineInterface>(device);
		mPipelineInterface->AddCombinedImageSampler(SET_0, BINDING_0, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface->AddCombinedImageSampler(SET_0, BINDING_1, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface->AddCombinedImageSampler(SET_0, BINDING_2, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface->AddUniformBuffer(SET_0, BINDING_3, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface->AddStorageBuffer(SET_0, 4, VK_SHADER_STAGE_COMPUTE_BIT);

		// Descriptor set 1
		mPipelineInterface->AddStorageBuffer(SET_1, 0, VK_SHADER_STAGE_COMPUTE_BIT);

		mPipelineInterface->AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_COMPUTE_BIT);

		mPipelineInterface->Create();
	}

	void MarchingCubesEffect::CreateDescriptorSets(Device* device)
	{
		// Create the descriptor here, it's data needs to be set in Terrain.cpp
		ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mCounterSSBO.Create(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mCounterSSBO.UpdateMemory();

		DescriptorSetLayout* setLayout0 = mPipelineInterface->GetDescriptorSetLayout(SET_0);
		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, setLayout0, mDescriptorPool);
		mDescriptorSet0->BindCombinedImage(BINDING_0, edgeTableTex->GetDescriptor());
		mDescriptorSet0->BindCombinedImage(BINDING_1, triangleTableTex->GetDescriptor());
		mDescriptorSet0->BindCombinedImage(BINDING_2, texture3d->GetDescriptor());
		mDescriptorSet0->BindUniformBuffer(BINDING_3, ubo.GetDescriptor());
		mDescriptorSet0->BindStorageBuffer(BINDING_4, mCounterSSBO.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();

		DescriptorSetLayout* setLayout1 = mPipelineInterface->GetDescriptorSetLayout(SET_1);
		mDescriptorSet1 = new Utopian::Vk::DescriptorSet(device, setLayout1, mDescriptorPool);
	}

	void MarchingCubesEffect::CreatePipeline(Device* device, RenderPass* renderPass)
	{
		Utopian::Vk::Shader* computeShader = gShaderFactory().CreateComputeShader("data/shaders/marching_cubes/marching_cubes.comp.spv");
		mComputePipeline = new Utopian::Vk::ComputePipeline(device, mPipelineInterface.get(), computeShader);
		mComputePipeline->Create();
	}

	void MarchingCubesEffect::UpdateMemory()
	{
		ubo.UpdateMemory();
		mCounterSSBO.UpdateMemory();
	}

	ComputePipeline* MarchingCubesEffect::GetComputePipeline()
	{
		return mComputePipeline;
	}

	VkDescriptorSet MarchingCubesEffect::GetDescriptorSet0()
	{
		return mDescriptorSet0->GetVkHandle();
	}

	VkDescriptorSet MarchingCubesEffect::GetDescriptorSet1()
	{
		return mDescriptorSet1->GetVkHandle();
	}

	void MarchingCubesEffect::UniformBuffer::UpdateMemory()
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int MarchingCubesEffect::UniformBuffer::GetSize()
	{
		return sizeof(data);
	}
}