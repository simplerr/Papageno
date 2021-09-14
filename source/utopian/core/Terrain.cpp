#include "core/Terrain.h"
#include "core/renderer/RendererUtility.h"
#include "core/ModelLoader.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/Effect.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Texture.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/renderer/Primitive.h"
#include "core/renderer/Renderer.h"
#include "core/physics/Physics.h"
#include "core/Input.h"
#include "core/Camera.h"
#include "core/Log.h"
#include "core/renderer/ImGuiRenderer.h"

namespace Utopian
{
   Terrain::Terrain(Vk::Device* device)
   {
      mDevice = device;

      GeneratePatches(1.0f, 512);
      GenerateTerrainMaps();

      Vk::gEffectManager().RegisterRecompileCallback(&Terrain::EffectRecomiledCallback, this);

      // This is used by both TerrainTool and GBufferTerrainJob
      // TerrainTool is responsible for updating it
      mBrushBlock = std::make_shared<Terrain::BrushBlock>();
      mBrushBlock->Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      // Load materials
      //AddMaterial("grass", "data/textures/ground/grass_diffuse.ktx", "data/textures/ground/grass_normal.ktx", "data/textures/ground/grass_displacement.ktx");
      //AddMaterial("grass", "data/textures/ground/aerial_rocks_01_1k_png/aerial_rocks_01_diff_1k.png", "data/textures/ground/aerial_rocks_01_1k_png/aerial_rocks_01_nor_1k.png", "data/textures/ground/aerial_rocks_01_1k_png/aerial_rocks_01_disp_1k.png");
      //AddMaterial("grass", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_diff_1k-modified.png", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_nor_1k.ktx", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_disp_1k.ktx");
      //AddMaterial("grass", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_diff_1k.ktx", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_nor_1k.ktx", "data/textures/ground/aerial_grass_rock_1k_png/aerial_grass_rock_disp_1k.ktx");
      AddMaterial("grass", "data/textures/ground/grass_countryside/grass_countryside_diffuse.ktx", "data/textures/ground/grass_countryside/grass_countryside_normal.ktx", "data/textures/ground/grass_countryside/grass_countryside_height.ktx");
      AddMaterial("rock", "data/textures/ground/rock_diffuse.ktx", "data/textures/ground/rock_normal.ktx", "data/textures/ground/rock_displacement.ktx");
      AddMaterial("dirt", "data/textures/ground/dirt_diffuse.ktx", "data/textures/ground/dirt_normal.ktx", "data/textures/ground/dirt_displacement.ktx");
      AddMaterial("road", "data/textures/ground/cobblestone_large_01_1k_png/cobblestone_large_01_diff_1k.ktx", "data/textures/ground/cobblestone_large_01_1k_png/cobblestone_large_01_nor_1k.ktx", "data/textures/ground/cobblestone_large_01_1k_png/cobblestone_large_01_disp_1k.ktx");

      // Add heightmap to physics world
      UpdatePhysicsHeightmap();
   }

   Terrain::~Terrain()
   {
      mBlendmapEffect = nullptr;
      mNormalmapEffect = nullptr;
      mHeightmapEffect = nullptr;

      mMaterials.clear();

      delete mQuadPrimitive;
   }

   void Terrain::Update()
   {
      // Experimentation
      if (gInput().KeyPressed('U'))
         UpdatePhysicsHeightmap();

      if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
      {
         ImGuiRenderer::BeginWindow("Terrain debug", glm::vec2(300.0f, 10.0f), 400.0f);

         ImVec2 textureSize = ImVec2(256, 256);
         ImGui::BeginGroup();
         ImGui::Text("Heightmap");
         ImGui::Image(mDebugDescriptorSets.heightmap, textureSize);
         ImGui::EndGroup();

         ImGui::SameLine();

         ImGui::BeginGroup();
         ImGui::Text("Normal map");
         ImGui::Image(mDebugDescriptorSets.normalmap, textureSize);
         ImGui::EndGroup();

         ImGui::BeginGroup();
         ImGui::Text("Blend map");
         ImGui::Image(mDebugDescriptorSets.blendmap, textureSize);
         ImGui::EndGroup();

         ImGuiRenderer::EndWindow();
      }
   }

   void Terrain::AddMaterial(std::string name, std::string diffuse, std::string normal, std::string displacement)
   {
      TerrainMaterial material;
      material.diffuse = Vk::gTextureLoader().LoadTexture(diffuse);
      material.normal = Vk::gTextureLoader().LoadTexture(normal);
      material.displacement = Vk::gTextureLoader().LoadTexture(displacement);
      mMaterials[name] = material;
   }

   void Terrain::SaveHeightmap(std::string filename)
   {
      uint32_t resolution = GetMapResolution();
      gRendererUtility().SaveToFile(mDevice, GetHeightmapImage(), filename, resolution, resolution, VK_FORMAT_R32_SFLOAT);
   }

   void Terrain::SaveBlendmap(std::string filename)
   {
      uint32_t resolution = GetMapResolution();
      gRendererUtility().SaveToFile(mDevice, GetBlendmapImage(), filename, resolution, resolution, VK_FORMAT_R32G32B32A32_SFLOAT);
   }

   void Terrain::LoadHeightmap(std::string filename)
   {
      SharedPtr<Vk::Texture> texture = Vk::gTextureLoader().LoadTexture(filename, VK_FORMAT_R32_SFLOAT, false);
      gRendererUtility().CopyImage(mDevice, *heightmapImage, texture->GetImage());
   }

   void Terrain::LoadBlendmap(std::string filename)
   {
      SharedPtr<Vk::Texture> texture = Vk::gTextureLoader().LoadTexture(filename, VK_FORMAT_R32G32B32A32_SFLOAT, false);
      gRendererUtility().CopyImage(mDevice, *blendmapImage, texture->GetImage());
   }

   void Terrain::GenerateTerrainMaps()
   {
      SetupHeightmapEffect();
      SetupNormalmapEffect();
      SetupBlendmapEffect();

      RenderHeightmap();
      RenderNormalmap();
      RenderBlendmap();

      // Testing
      RetrieveHeightmap();

      // Add debug render targets
      ImGuiRenderer* imGuiRenderer = gRenderer().GetUiOverlay();
      mDebugDescriptorSets.heightmap = imGuiRenderer->AddImage(*heightmapImage);
      mDebugDescriptorSets.normalmap = imGuiRenderer->AddImage(*normalImage);
      mDebugDescriptorSets.blendmap = imGuiRenderer->AddImage(*blendmapImage);
   }

   void Terrain::EffectRecomiledCallback(std::string name)
   {
      RenderHeightmap();
      RenderNormalmap();
      RenderBlendmap();

      RetrieveHeightmap();
      UpdatePhysicsHeightmap();
   }

   void Terrain::RetrieveHeightmap()
   {
      hostImage = gRendererUtility().CreateHostVisibleImage(mDevice, heightmapImage, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32_SFLOAT);

      // Get layout of the image (including row pitch)
      VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
      VkSubresourceLayout subResourceLayout;
      vkGetImageSubresourceLayout(mDevice->GetVkDevice(), hostImage->GetVkHandle(), &subResource, &subResourceLayout);

      const char* data;
      hostImage->MapMemory((void**)&data);
      data += subResourceLayout.offset;

      assert(subResourceLayout.rowPitch == MAP_RESOLUTION * sizeof(float));
      assert(subResourceLayout.size == MAP_RESOLUTION * MAP_RESOLUTION * sizeof(float));

      // Since the image tiling is linear we can use memcpy
      memcpy(heightmap.data(), data, subResourceLayout.size);

      hostImage->UnmapMemory();

      // Note: Todo: Hidden dependency to Renderer
      gRenderer().UpdateInstanceAltitudes();
   }

   void Terrain::UpdatePhysicsHeightmap()
   {
      gPhysics().SetHeightmap(heightmap.data(), MAP_RESOLUTION, mAmplitudeScaling, terrainSize);
   }

   void Terrain::SetupHeightmapEffect()
   {
      heightmapImage = std::make_shared<Vk::ImageColor>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32_SFLOAT, "Terrain heightmap image");
      heightmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL);

      heightmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION);
      heightmapRenderTarget->AddWriteOnlyColorAttachment(heightmapImage, VK_IMAGE_LAYOUT_GENERAL);
      heightmapRenderTarget->SetClearColor(1, 1, 1, 1);
      heightmapRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/heightmap.frag";

      mHeightmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapRenderTarget->GetRenderPass(), effectDesc);
   }

   void Terrain::SetupNormalmapEffect()
   {
      normalImage = std::make_shared<Vk::ImageColor>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32G32B32A32_SFLOAT, "Terrain normalmap image");

      normalRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION);
      normalRenderTarget->AddWriteOnlyColorAttachment(normalImage);
      normalRenderTarget->SetClearColor(1, 1, 1, 1);
      normalRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/normalmap.frag";

      mNormalmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, normalRenderTarget->GetRenderPass(), effectDesc);

      mNormalmapEffect->BindCombinedImage("samplerHeightmap", *heightmapImage, *heightmapRenderTarget->GetSampler());
   }

   void Terrain::SetupBlendmapEffect()
   {
      blendmapImage = std::make_shared<Vk::ImageColor>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION, VK_FORMAT_R32G32B32A32_SFLOAT, "Terrain blendmap image");
      blendmapImage->SetFinalLayout(VK_IMAGE_LAYOUT_GENERAL); // Special case since it needs to be used both as color attachment and descriptor

      blendmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, MAP_RESOLUTION, MAP_RESOLUTION);
      blendmapRenderTarget->AddWriteOnlyColorAttachment(blendmapImage, VK_IMAGE_LAYOUT_GENERAL);
      blendmapRenderTarget->SetClearColor(1, 1, 1, 1);
      blendmapRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/terrain_creation/blendmap.frag";

      mBlendmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blendmapRenderTarget->GetRenderPass(), effectDesc);

      settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mBlendmapEffect->BindUniformBuffer("UBO_settings", settingsBlock);

      mBlendmapEffect->BindCombinedImage("samplerHeightmap", *heightmapImage, *heightmapRenderTarget->GetSampler());
      mBlendmapEffect->BindCombinedImage("samplerNormalmap", *normalImage, *heightmapRenderTarget->GetSampler());
      mBlendmapEffect->BindUniformBuffer("UBO_settings", settingsBlock);
   }

   void Terrain::RenderHeightmap()
   {
      heightmapRenderTarget->Begin("Heightmap pass");
      Vk::CommandBuffer* commandBuffer = heightmapRenderTarget->GetCommandBuffer();
      commandBuffer->CmdBindPipeline(mHeightmapEffect->GetPipeline());
      gRendererUtility().DrawFullscreenQuad(commandBuffer);
      heightmapRenderTarget->EndAndFlush();
   }

   void Terrain::RenderNormalmap()
   {
      normalRenderTarget->Begin("Normalmap pass");
      Vk::CommandBuffer* commandBuffer = normalRenderTarget->GetCommandBuffer();
      commandBuffer->CmdBindPipeline(mNormalmapEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mNormalmapEffect);
      gRendererUtility().DrawFullscreenQuad(commandBuffer);
      normalRenderTarget->EndAndFlush();
   }

   void Terrain::RenderBlendmap()
   {
      settingsBlock.data.amplitudeScaling = mAmplitudeScaling;
      settingsBlock.UpdateMemory();

      blendmapRenderTarget->Begin("Blendmap pass");
      Vk::CommandBuffer* commandBuffer = blendmapRenderTarget->GetCommandBuffer();
      commandBuffer->CmdBindPipeline(mBlendmapEffect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(mBlendmapEffect);
      gRendererUtility().DrawFullscreenQuad(commandBuffer);
      blendmapRenderTarget->EndAndFlush();
   }

   void Terrain::GeneratePatches(float cellSize, int numCells)
   {
      terrainSize = cellSize * (numCells - 1);

      mQuadPrimitive = new Primitive();
      mQuadPrimitive->SetDebugName("Terrain patches");

      // Vertices
      for (auto x = 0; x < numCells; x++)
      {
         for (auto z = 0; z < numCells; z++)
         {
            Vk::Vertex vertex;
            const float originOffset = (float)numCells * cellSize / 2.0f;
            vertex.pos = glm::vec3(x * cellSize + cellSize / 2.0f - originOffset, 0.0f, z * cellSize + cellSize / 2.0f - originOffset);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.uv = glm::vec2((float)x / (numCells - 1), (float)z / (numCells - 1));
            mQuadPrimitive->AddVertex(vertex);
         }
      }

      // Indices
      const uint32_t w = (numCells - 1);
      for (auto x = 0u; x < w; x++)
      {
         for (auto z = 0u; z < w; z++)
         {
            uint32_t v1 = (x + z * numCells);
            uint32_t v2 = v1 + numCells;
            uint32_t v3 = v2 + 1;
            uint32_t v4 = v1 + 1;
            mQuadPrimitive->AddQuad(v1, v2, v3, v4);
         }
      }

      mQuadPrimitive->BuildBuffers(mDevice);
   }

   glm::vec2 Terrain::TransformToUv(float x, float z)
   {
      // Transform to terrain UV coordinates [0, 1]
      glm::vec2 uv = glm::vec2(x, z);
      uv += terrainSize / 2.0f;
      uv /= terrainSize;

      // To get correct UV coordinates
      uv = glm::vec2(1.0, 1.0) - uv;
      return uv;
   }

   float Terrain::GetHeight(float x, float z)
   {
      // If the creation of the host image failed
      if (hostImage == nullptr)
         return 0.0f;

      float height = -1.0f;

      glm::vec2 uv = TransformToUv(x, z);

      uint32_t col = (uint32_t)floorf(uv.x * hostImage->GetWidth());
      uint32_t row = (uint32_t)floorf(uv.y * hostImage->GetHeight());

      if (row >= 0 && col >= 0 && row < hostImage->GetWidth() && col < hostImage->GetHeight())
      {
         height = heightmap[row * hostImage->GetWidth() + col] * mAmplitudeScaling * -1; // Todo: Fix amplitude and -1
      }

      return height;
   }

   glm::vec3 Terrain::GetIntersectPoint(Ray ray)
   {
      Ray shorterRay = LinearSearch(ray);

      // This is good enough accuracy for now
      return shorterRay.origin;
      //return BinarySearch(shorterRay);
   }

   Ray Terrain::LinearSearch(Ray ray)
   {
      float stepSize = 0.25;
      glm::vec3 nextPoint = ray.origin + ray.direction * stepSize;
      float heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
      int counter = 0;
      const int numMaxSteps = 2000;
      while (heightAtNextPoint < nextPoint.y && counter < numMaxSteps)
      {
         counter++;
         ray.origin = nextPoint;
         nextPoint = ray.origin + ray.direction * stepSize;
         heightAtNextPoint = GetHeight(nextPoint.x, nextPoint.z);
      }

      // Return infinity if the ray dont strike anything
      if (counter >= numMaxSteps)
         ray.direction.x = std::numeric_limits<float>::infinity();

      return ray;
   }

   glm::vec3 Terrain::GetNormal(float x, float z)
   {
      return glm::vec3(0, 1, 0);
   }

   SharedPtr<Vk::Image>& Terrain::GetHeightmapImage()
   {
      return heightmapImage;
   }

   SharedPtr<Vk::Image>& Terrain::GetNormalmapImage()
   {
      return normalImage;
   }

   SharedPtr<Vk::Image>& Terrain::GetBlendmapImage()
   {
      return blendmapImage;
   }

   Primitive* Terrain::GetPrimitive()
   {
      return mQuadPrimitive;
   }

   uint32_t Terrain::GetMapResolution()
   {
      return MAP_RESOLUTION;
   }

   float Terrain::GetTerrainSize()
   {
      return terrainSize;
   }

   void Terrain::SetBrushBlock(const SharedPtr<BrushBlock> brushBlock)
   {
      mBrushBlock = brushBlock;
   }

   SharedPtr<Terrain::BrushBlock> Terrain::GetBrushBlock()
   {
      return mBrushBlock;
   }

   float Terrain::GetAmplitudeScaling()
   {
      return mAmplitudeScaling;
   }

   void Terrain::SetAmplitudeScaling(float amplitudeScaling)
   {
      mAmplitudeScaling = amplitudeScaling;
   }

   TerrainMaterial Terrain::GetMaterial(std::string material)
   {
      if (mMaterials.find(material) != mMaterials.end())
      {
         return mMaterials[material];
      }
      else
         assert(0);
   }
}
