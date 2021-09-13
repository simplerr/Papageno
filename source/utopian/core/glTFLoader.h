#pragma once
#include <string>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "core/renderer/Model.h"

namespace Utopian
{
   class glTFLoader
   {
   public:
      glTFLoader(Vk::Device* device);
      ~glTFLoader();

      SharedPtr<Model> LoadModel(std::string filename, Vk::Device* device);

      Material GetDefaultMaterial();

      void SetInverseTranslation(bool inverse);

   private:
      void LoadMaterials(tinygltf::Model& input, Model* model);
      void LoadNode(Model* model, const tinygltf::Node& inputNode, const tinygltf::Model& input,
                    Node* parent, uint32_t nodeIndex, Vk::Device* device);
      void AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive);
      void AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive);
      void CreateDescriptorPools();

   private:
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorSetLayout> mMeshSkinningDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
      SharedPtr<Vk::DescriptorPool> mMeshSkinningDescriptorPool;
      Vk::Device* mDevice;

      // Todo: this is due to the coordinate system being inversed in the engine,
      // needs to be fixed.
      bool mInverseTranslation = true;
   };
}