#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Buffer.h"
#include "utility/math/BoundingBox.h"
#include "utility/Common.h"

namespace Utopian
{
   class Primitive
   {
   public:
      Primitive();
      ~Primitive();

      void AddVertex(const Vk::Vertex& vertex);
      void AddVertex(glm::vec3 pos);
      void AddLine(uint32_t v1, uint32_t v2);
      void AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
      void AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
      void BuildBuffers(Vk::Device* device);

      void ReserveVertices(uint32_t numVertices);
      void ReserveIndices(uint32_t numIndices);

      uint32_t GetNumIndices() const;
      uint32_t GetNumVertices() const;

      Vk::Buffer* GetVertxBuffer();
      Vk::Buffer* GetIndexBuffer();
      BoundingBox GetBoundingBox();

      void SetDebugName(std::string debugName);
      std::string GetDebugName() const;

      std::vector<Vk::Vertex> vertices;
      std::vector<unsigned int> indices;

   private:
      SharedPtr<Vk::Buffer> mVertexBuffer;
      SharedPtr<Vk::Buffer> mIndexBuffer;
      BoundingBox mBoundingBox;
      std::string mDebugName = "unnamed";
   };
}
