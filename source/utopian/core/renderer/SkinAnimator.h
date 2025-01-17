#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
   class Model;
   class glTFLoader;
   struct Node;

   class SkinAnimator
   {
   public:
      struct Skin
      {
         std::string name;
         Node* skeletonRoot = nullptr;
         std::vector<glm::mat4> inverseBindMatrices;
         std::vector<Node*> joints;
         SharedPtr<Vk::Buffer> ssbo;
         SharedPtr<Vk::DescriptorSet> descriptorSet;
      };

      struct AnimationSampler
      {
         std::string interpolation;
         std::vector<float> inputs;
         std::vector<glm::vec4> outputsVec4;
      };

      struct AnimationChannel
      {
         std::string path;
         Node* node;
         uint32_t samplerIndex;
      };

      struct Animation
      {
         std::string name;
         std::vector<AnimationSampler> samplers;
         std::vector<AnimationChannel> channels;
         float start = std::numeric_limits<float>::max();
         float end = std::numeric_limits<float>::min();
         float currentTime = 0.0f;
      };

      SkinAnimator(tinygltf::Model& input, Model* model, Vk::Device* device);
      ~SkinAnimator();

      void LoadSkins(tinygltf::Model& input, Model* model, Vk::Device* device);
      void LoadAnimations(tinygltf::Model& input, Model* model);

      void UpdateAnimation(float deltaTime);
      void UpdateJoints(Node* node);
      glm::mat4 GetNodeMatrix(Node* node);

      VkDescriptorSet GetJointMatricesDescriptorSet(int32_t skin);
      uint32_t GetNumAnimations() const;
      uint32_t GetActiveAnimation() const;
      std::string GetAnimationName(uint32_t index) const;
      bool GetPaused() const;

      void SetAnimation(uint32_t index);
      void SetPaused(bool paused);

      void CreateSkinningDescriptorSet(Vk::Device* device, Vk::DescriptorSetLayout* setLayout, Vk::DescriptorPool* pool);

   private:
      std::vector<Skin> mSkins;
      std::vector<Animation> mAnimations;
      uint32_t mActiveAnimation = 0;
      bool mPaused = false;
   };
}