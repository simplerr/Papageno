#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Buffer.h"
#include "utility/math/BoundingBox.h"
#include "utility/Common.h"

#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"
#define DEFAULT_SPECULAR_MAP_TEXTURE "data/textures/default_specular_map.png"

namespace Utopian::Vk
{
   class Mesh
   {
   public:
      Mesh(Device* device);
      ~Mesh();

      void AddVertex(Vertex vertex);
      void AddVertex(float x, float y, float z);
      void AddLine(uint32_t v1, uint32_t v2);
      void AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
      void AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
      void BuildBuffers(Device* device);
      void BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>);

      void LoadTextures(std::string diffusePath, std::string normalPath = DEFAULT_NORMAL_MAP_TEXTURE, std::string specularPath = DEFAULT_SPECULAR_MAP_TEXTURE);

      void SetTexture(SharedPtr<Vk::Texture> texture);
      void SetSpecularTexture(SharedPtr<Vk::Texture> texture);
      VkDescriptorSet GetTextureDescriptorSet();

      BoundingBox GetBoundingBox();

      uint32_t GetNumIndices();

      Buffer* GetVertxBuffer();
      Buffer* GetIndexBuffer();

      Texture* GetDiffuseTexture();
      Texture* GetNormalTexture();
      Texture* GetSpecularTexture();

      // Note: Do not call this every run iteration
      std::vector<Texture*> GetTextures();

      void SetDebugName(std::string debugName);

      std::vector<Vertex> vertexVector;
      std::vector<unsigned int> indexVector;
   private:
      // Creates a DescriptorSet from the diffuse and normal textures that was added to the mesh.
      void CreateDescriptorSets(SharedPtr<DescriptorSetLayout> descriptorSetLayout, SharedPtr<DescriptorPool> descriptorPool);

   private:
      SharedPtr<Buffer> mVertexBuffer;
      SharedPtr<Buffer> mIndexBuffer;
      SharedPtr<DescriptorSet> mTextureDescriptorSet;

      Device* mDevice;
      SharedPtr<Texture> mDiffuseTexture;
      SharedPtr<Texture> mNormalTexture;
      SharedPtr<Texture> mSpecularTexture;

      BoundingBox mBoundingBox;

      uint32_t mIndicesCount;
      uint32_t mVerticesCount;

      std::string mDebugName = "unnamed";
   };
}
