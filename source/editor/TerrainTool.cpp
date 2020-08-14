#include "editor/TerrainTool.h"
#include "vulkan/EffectManager.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "core/renderer/ImGuiRenderer.h"
#include "vulkan/TextureLoader.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RendererUtility.h"
#include "core/Terrain.h"
#include "core/Input.h"
#include "core/Camera.h"

namespace Utopian
{
   TerrainTool::TerrainTool(Terrain* terrain, Vk::Device* device)
   {
		mTerrain = terrain;
		mDevice = device;

		brushBlock = terrain->GetBrushBlock();

		SetupBlendmapBrushEffect();
		SetupHeightmapBrushEffect();

		RenderBlendmapBrush();
		RenderHeightmapBrush();

		Vk::gEffectManager().RegisterRecompileCallback(&TerrainTool::EffectRecompiledCallback, this);

		// Temporary:
		brushSettings.mode = BrushSettings::Mode::BLEND;
		brushSettings.operation = BrushSettings::Operation::ADD;
		brushSettings.blendLayer = BrushSettings::BlendLayer::GRASS;
		brushSettings.strength = 250.0f;
		brushSettings.radius = 1500.0f;

		heightToolTexture = Vk::gTextureLoader().LoadTexture("data/textures/height-tool.ktx");

		ImGuiRenderer* imGuiRenderer = gRenderer().GetUiOverlay();
		textureIdentifiers.grass = imGuiRenderer->AddImage(*mTerrain->GetMaterial("grass").diffuse->GetImage());
		textureIdentifiers.rock = imGuiRenderer->AddImage(*mTerrain->GetMaterial("rock").diffuse->GetImage());
		textureIdentifiers.dirt = imGuiRenderer->AddImage(*mTerrain->GetMaterial("dirt").diffuse->GetImage());
		textureIdentifiers.road = imGuiRenderer->AddImage(*mTerrain->GetMaterial("road").diffuse->GetImage());
		textureIdentifiers.heightTool = imGuiRenderer->AddImage(*heightToolTexture->GetImage());
   }

   TerrainTool::~TerrainTool()
   {

   }

   void TerrainTool::Update()
   {
		glm::vec3 cameraPos = gRenderer().GetMainCamera()->GetPosition();
		static glm::vec3 intersection = glm::vec3(0.0);

		Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
		intersection = mTerrain->GetIntersectPoint(ray);
		brushSettings.position = mTerrain->TransformToUv(intersection.x, intersection.z);
		brushSettings.radius += gInput().MouseDz() / 4.0f;

		UpdateBrushUniform();

		if (brushSettings.mode == BrushSettings::Mode::BLEND)
		{
			if (gInput().KeyDown(VK_LBUTTON))
			{
				RenderBlendmapBrush();
			}
		}
		else if (brushSettings.mode == BrushSettings::Mode::HEIGHT)
		{
			if (gInput().KeyDown(VK_LBUTTON) || gInput().KeyDown(VK_RBUTTON))
			{
				if (gInput().KeyDown(VK_LBUTTON))
					brushSettings.operation = BrushSettings::Operation::ADD;
				else
					brushSettings.operation = BrushSettings::Operation::REMOVE;

				RenderHeightmapBrush();
				mTerrain->RenderNormalmap();
				mTerrain->RenderBlendmap();
				mTerrain->RetrieveHeightmap();
			}
		}
   }

   void TerrainTool::RenderUi()
   {
	   if (ImGui::CollapsingHeader("Terrain tool", ImGuiTreeNodeFlags_DefaultOpen))
	   {
		   ImGui::SliderFloat("Brush radius", &brushSettings.radius, 0.0f, 10000.0f);
		   ImGui::SliderFloat("Brush strenth", &brushSettings.strength, 0.0f, 299.0f);

		   if (ImGui::ImageButton(textureIdentifiers.heightTool, ImVec2(64, 64)))
		   {
			   brushSettings.mode = BrushSettings::Mode::HEIGHT;
		   }

		   ImGui::SameLine();

		   if (ImGui::ImageButton(textureIdentifiers.grass, ImVec2(64, 64)))
		   {
			   brushSettings.mode = BrushSettings::Mode::BLEND;
			   brushSettings.blendLayer = BrushSettings::BlendLayer::GRASS;
		   }

		   ImGui::SameLine();

		   if (ImGui::ImageButton(textureIdentifiers.rock, ImVec2(64, 64)))
		   {
			   brushSettings.mode = BrushSettings::Mode::BLEND;
			   brushSettings.blendLayer = BrushSettings::BlendLayer::ROCK;
		   }

		   ImGui::SameLine();

		   if (ImGui::ImageButton(textureIdentifiers.dirt, ImVec2(64, 64)))
		   {
			   brushSettings.mode = BrushSettings::Mode::BLEND;
			   brushSettings.blendLayer = BrushSettings::BlendLayer::DIRT;
		   }

		   if (ImGui::ImageButton(textureIdentifiers.road, ImVec2(64, 64)))
		   {
			   brushSettings.mode = BrushSettings::Mode::BLEND;
			   brushSettings.blendLayer = BrushSettings::BlendLayer::ROAD;
		   }
	   }
   }

	void TerrainTool::EffectRecompiledCallback(std::string name)
	{
		RenderBlendmapBrush();
		RenderHeightmapBrush();

		//mTerrain->RetrieveHeightmap();
	}

   void TerrainTool::SetupBlendmapBrushEffect()
	{
		blendmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mTerrain->GetMapResolution(), mTerrain->GetMapResolution());
		blendmapBrushRenderTarget->AddReadWriteColorAttachment(mTerrain->GetBlendmapImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL);
		blendmapBrushRenderTarget->SetClearColor(1, 1, 1, 1);
		blendmapBrushRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/blendmap_brush.frag";

		mBlendmapBrushEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blendmapBrushRenderTarget->GetRenderPass(), effectDesc);

		mBlendmapBrushEffect->BindUniformBuffer("UBO_brush", *brushBlock);
	}

	void TerrainTool::SetupHeightmapBrushEffect()
	{
		heightmapBrushRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mTerrain->GetMapResolution(), mTerrain->GetMapResolution());
		heightmapBrushRenderTarget->AddReadWriteColorAttachment(mTerrain->GetHeightmapImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL);
		heightmapBrushRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapBrushRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/heightmap_brush.frag";
		effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ADDITIVE;

		mHeightmapBrushEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapBrushRenderTarget->GetRenderPass(), effectDesc);

		mHeightmapBrushEffect->BindUniformBuffer("UBO_brush", *brushBlock);
	}

	void TerrainTool::UpdateBrushUniform()
	{
		brushBlock->data.brushPos = brushSettings.position;
		brushBlock->data.radius = brushSettings.radius / mTerrain->GetTerrainSize();
		brushBlock->data.strength = brushSettings.strength;
		brushBlock->data.mode = brushSettings.mode;
		brushBlock->data.operation = brushSettings.operation;
		brushBlock->data.blendLayer = brushSettings.blendLayer;
		brushBlock->UpdateMemory();
	}

   void TerrainTool::RenderBlendmapBrush()
	{
		blendmapBrushRenderTarget->Begin("Blendmap brush pass");
		Vk::CommandBuffer* commandBuffer = blendmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mBlendmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlendmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		blendmapBrushRenderTarget->EndAndFlush();
	}

	void TerrainTool::RenderHeightmapBrush()
	{
		heightmapBrushRenderTarget->Begin("Heightmap brush pass");
		Vk::CommandBuffer* commandBuffer = heightmapBrushRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapBrushEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mHeightmapBrushEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapBrushRenderTarget->EndAndFlush();
	}

	BrushSettings* TerrainTool::GetBrushSettings()
	{
		return &brushSettings;
	}
}