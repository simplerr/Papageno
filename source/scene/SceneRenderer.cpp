#include <glm/gtc/matrix_transform.hpp>
#include "scene/SceneRenderer.h"
#include "scene/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "Camera.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "scene/Light.h"
#include "Terrain.h"
#include "WaterRenderer.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/RenderTarget.h"

namespace Scene
{
	SceneRenderer::SceneRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
	{
		mRenderer = renderer;
		mCamera = camera;
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		mWaterRenderer = new WaterRenderer(mRenderer, renderer->mModelLoader, renderer->mTextureLoader);
		mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
		mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

		mScreenGui = new Vulkan::ScreenGui(mRenderer);
		mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 2*350 - 50, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetReflectionRenderTarget()->GetImage(), mWaterRenderer->GetReflectionRenderTarget()->GetSampler());
		mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 350, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetRefractionRenderTarget()->GetImage(), mWaterRenderer->GetRefractionRenderTarget()->GetSampler());
	}

	SceneRenderer::~SceneRenderer()
	{
		delete mScreenGui;
		delete mWaterRenderer;
	}

	void SceneRenderer::InitShader()
	{
		// Important to do this before PhongEffect::Init()
		for (auto& light : mLights)
		{
			mPhongEffect.per_frame_ps.lights.push_back(light->GetLightData());
		}

		mPhongEffect.per_frame_ps.constants.numLights = mPhongEffect.per_frame_ps.lights.size();

		mPhongEffect.Init(mRenderer);
	}

	void SceneRenderer::Update()
	{
		mTerrain->Update();
		mWaterRenderer->Update(mRenderer, mCamera);
	}

	void SceneRenderer::RenderNodes(Vulkan::CommandBuffer* commandBuffer)
	{
		// From Renderer.cpp
		if (mCamera != nullptr)
		{
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.viewMatrix = mCamera->GetView();
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.eyePos = mCamera->GetPosition();
		}

		mPhongEffect.UpdateMemory(mRenderer->GetDevice());

		for (auto& renderable : mRenderables)
		{
			Vulkan::StaticModel* model = renderable->GetModel();
			mPhongEffect.SetPipeline(0);

			for (Vulkan::Mesh* mesh : model->mMeshes)
			{
				commandBuffer->CmdBindPipeline(mPhongEffect.GetPipeline());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();

				VkDescriptorSet descriptorSets[3] = { mPhongEffect.mCameraDescriptorSet->descriptorSet, mPhongEffect.mLightDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPhongEffect.GetPipelineLayout(), 0, 3, descriptorSets, 0, NULL);

				// Push the world matrix constant
				PushConstantBlock pushConstantBlock;
				Transform transform = renderable->GetTransform();
				mat4 world;
				world = glm::translate(world, transform.GetPosition());
				world = glm::scale(world, transform.GetScale());
				pushConstantBlock.world = world;

				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				commandBuffer->CmdPushConstants(&mPhongEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}
	}

	void SceneRenderer::RenderScene(Vulkan::CommandBuffer* commandBuffer)
	{
		mTerrain->Render(commandBuffer);
		RenderNodes(commandBuffer);
	}

	void SceneRenderer::Render()
	{
		RenderOffscreen();

		mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		RenderScene(mCommandBuffer);

		mScreenGui->Render(mRenderer, mCommandBuffer);
		mWaterRenderer->Render(mRenderer, mCommandBuffer);

		mCommandBuffer->End();
	}

	void SceneRenderer::RenderOffscreen()
	{
		glm::vec3 cameraPos = mCamera->GetPosition();

		// Reflection renderpass
		mCamera->SetPosition(mCamera->GetPosition() - glm::vec3(0, mCamera->GetPosition().y *  2, 0)); // NOTE: Water is hardcoded to be at y = 0
		mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
		mTerrain->SetClippingPlane(glm::vec4(0, -1, 0, 0));
		mTerrain->UpdateUniformBuffer();

		mWaterRenderer->GetReflectionRenderTarget()->Begin();	
		RenderScene(mWaterRenderer->GetReflectionRenderTarget()->GetCommandBuffer());
		mWaterRenderer->GetReflectionRenderTarget()->End(mRenderer->GetQueue());

		// Refraction renderpass
		mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 0));
		mCamera->SetPosition(cameraPos);
		mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
		mTerrain->UpdateUniformBuffer();

		mWaterRenderer->GetRefractionRenderTarget()->Begin();	
		RenderScene(mWaterRenderer->GetRefractionRenderTarget()->GetCommandBuffer());
		mWaterRenderer->GetRefractionRenderTarget()->End(mRenderer->GetQueue());

		mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 1500000));
		mTerrain->UpdateUniformBuffer();
	}


	void SceneRenderer::AddRenderable(Renderable* renderable)
	{
		mRenderables.push_back(renderable);
	}

	void SceneRenderer::AddLight(Light* light)
	{
		mLights.push_back(light);
	}

	void SceneRenderer::SetTerrain(Terrain* terrain)
	{
		mTerrain = terrain;
	}
}