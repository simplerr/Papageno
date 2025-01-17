#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "core/Log.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/Buffer.h"
#include "Model.h"
#include "core/Engine.h"

// Todo: remove
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

namespace Utopian
{
   glm::mat4 Node::GetLocalMatrix()
   {
      return glm::translate(glm::mat4(1.0f), translation) *
             glm::mat4(rotation) *
             glm::scale(glm::mat4(1.0f), scale) *
             matrix;
   }

   Material::Material()
   {
      properties = std::make_shared<MaterialProperties>();
      properties->data.baseColorFactor = glm::vec4(1.0f);
      properties->data.metallicFactor = 1.0f;
      properties->data.roughnessFactor = 1.0f;
      properties->data.occlusionFactor = 1.0f;
   }

   void Material::UpdateTextureDescriptors(Vk::Device* device)
   {
      descriptorSet->BindCombinedImage(0, colorTexture->GetDescriptor());
      descriptorSet->BindCombinedImage(1, normalTexture->GetDescriptor());
      descriptorSet->BindCombinedImage(2, specularTexture->GetDescriptor());
      descriptorSet->BindCombinedImage(3, metallicRoughnessTexture->GetDescriptor());
      descriptorSet->BindCombinedImage(4, occlusionTexture->GetDescriptor());
      descriptorSet->BindUniformBuffer(20, properties->GetDescriptor());

      device->QueueDescriptorUpdate(descriptorSet.get());
   }

   Model::Model()
   {

   }

   Model::~Model()
   {
   }

   void Model::Init()
   {
      for (auto& node : mRootNodes)
      {
         mBoundingBox = CalculateBoundingBox(node, glm::mat4());
      }
   }

   BoundingBox Model::CalculateBoundingBox(Node* node, glm::mat4 world)
   {
      glm::mat4 nodeMatrix = world * node->GetLocalMatrix();
      BoundingBox bb;

      if (node->mesh.primitives.size() > 0)
         return node->mesh.primitives[0]->GetBoundingBox();

      for (auto& child : node->children)
      {
         bb = CalculateBoundingBox(child, nodeMatrix);
      }

      return bb;
   }

   Primitive* Model::AddPrimitive(const Primitive& primitive)
   {
      SharedPtr<Primitive> prim = std::make_shared<Primitive>(primitive);
      mPrimitives.push_back(prim);

      return prim.get();
   }

   Material* Model::AddMaterial(const Material& material)
   {
      SharedPtr<Material> mat = std::make_shared<Material>(material);
      mMaterials.push_back(mat);

      return mat.get();
   }

   Node* Model::CreateNode()
   {
      SharedPtr<Node> node = std::make_shared<Node>();
      mNodes.push_back(node);

      return node.get();
   }

   void Model::AddRootNode(Node* node)
   {
      mRootNodes.push_back(node);
   }

   void Model::AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator)
   {
      mSkinAnimator = skinAnimator;

      // Calculate initial pose
      for (auto node : mRootNodes)
         mSkinAnimator->UpdateJoints(node);
   }

   void Model::UpdateAnimation(float deltaTime)
   {
      // Note: Todo: Updating the animation is very slow and needs to be
      // improved, see #135.
      if (IsAnimated())
      {
         mSkinAnimator->UpdateAnimation(deltaTime);

         for (auto node : mRootNodes)
            mSkinAnimator->UpdateJoints(node);
      }
   }

   Primitive* Model::GetPrimitive(uint32_t index)
   {
      assert(index < mPrimitives.size());

      return mPrimitives[index].get();
   }

   Material* Model::GetMaterial(uint32_t index)
   {
      assert(index < mMaterials.size());

      return mMaterials[index].get();
   }

   SkinAnimator* Model::GetAnimator()
   {
      if (IsAnimated())
         return mSkinAnimator.get();
      else
         return nullptr;
   }

   void Model::GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix)
   {
      for (auto& node : mRootNodes) {
         AppendRenderCommands(renderCommands, node, worldMatrix);
      }
   }

   void Model::AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix)
   {
      glm::mat4 nodeMatrix = worldMatrix * node->GetLocalMatrix();

      RenderCommand command;
      command.world = nodeMatrix;
      command.skinDescriptorSet = VK_NULL_HANDLE;

      if (node->mesh.primitives.size() > 0)
      {
         if (IsAnimated())
         {
            command.skinDescriptorSet = mSkinAnimator->GetJointMatricesDescriptorSet(node->skin);
         }

         command.mesh = &node->mesh;
         renderCommands.push_back(command);
      }

      for (auto& child : node->children) {
         AppendRenderCommands(renderCommands, child, nodeMatrix);
      }
   }

   Node* Model::FindNode(Node* parent, uint32_t index)
   {
      Node* nodeFound = nullptr;

      if (parent->index == index)
      {
         return parent;
      }
      for (auto &child : parent->children)
      {
         nodeFound = FindNode(child, index);
         if (nodeFound)
         {
            break;
         }
      }

      return nodeFound;
   }

   Node* Model::NodeFromIndex(uint32_t index)
   {
      Node *nodeFound = nullptr;

      for (auto &node : mRootNodes)
      {
         nodeFound = FindNode(node, index);
         if (nodeFound)
         {
            break;
         }
      }

      return nodeFound;
   }

   void Model::SetFilename(std::string filename)
   {
      mFilename = filename;
   }

   bool Model::IsAnimated() const
   {
      return (mSkinAnimator != nullptr);
   }

   BoundingBox Model::GetBoundingBox()
   {
      return mBoundingBox;
   }

   uint32_t Model::GetNumPrimitives() const
   {
      return mPrimitives.size();
   }

   uint32_t Model::GetNumMaterials() const
   {
      return mMaterials.size();
   }
}
