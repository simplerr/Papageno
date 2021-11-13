#pragma once

#include <vector>
#include <thread>
#include "core/renderer/SceneInfo.h"
#include "core/renderer/RenderSettings.h"
#include "vulkan/handles/Semaphore.h"

namespace Utopian
{
   class Renderable;
   class Light;
   class Camera;
   class BaseJob;
   class PerlinTerrain; 

   struct GBuffer
   {
      SharedPtr<Vk::Image> positionImage;
      SharedPtr<Vk::Image> normalImage;
      SharedPtr<Vk::Image> normalViewImage;
      SharedPtr<Vk::Image> albedoImage;
      SharedPtr<Vk::Image> depthImage;
      SharedPtr<Vk::Image> specularImage; // R = specularity, G = material type, B = water depth, A = undefined
      SharedPtr<Vk::Image> pbrImage; // R = occlusion, G = roughness, B = metallic, A = undefined

      // The main render target where the entire scene is rendered to after
      // the G-buffer pass.
      SharedPtr<Vk::Image> mainImage;
   };

   struct JobInput
   {
      JobInput(const SceneInfo& sceneInfo, const std::vector<BaseJob*>& jobs, const RenderingSettings& renderingSettings) 
         : sceneInfo(sceneInfo), jobs(jobs) , renderingSettings(renderingSettings) {

      }

      const SceneInfo& sceneInfo;
      const std::vector<BaseJob*>& jobs;
      const RenderingSettings& renderingSettings;
   };

   class BaseJob
   {
   public:
      BaseJob(Vk::Device* device, uint32_t width, uint32_t height) {
         mDevice = device;
         mWidth = width;
         mHeight = height;
         mCompletedSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
         mEnabled = true;
      }

      virtual ~BaseJob() {};

      /**
       * Create and add worker threads for multithreaded resource loading.
       * @note Currently only shader compilation is tested and supports multithreading.
       */
      virtual void LoadResources() {};

      /**
       * If a job needs to query information from another job that's already added
       * it should be done inside of this function.
       */
      virtual void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) = 0;
      virtual void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) {};

      virtual void PreRender(const JobInput& jobInput) {};
      virtual void Render(const JobInput& jobInput) = 0;
      virtual void Update() {};

      void SetWaitSemaphore(const SharedPtr<Vk::Semaphore>& waitSemaphore) { mWaitSemaphore = waitSemaphore; };
      void SetEnabled(bool enabled) { mEnabled = enabled; };

      SharedPtr<Vk::Semaphore>& GetCompletedSemahore() { return mCompletedSemaphore; };
      SharedPtr<Vk::Semaphore>& GetWaitSemahore() { return mWaitSemaphore; };

      bool IsEnabled() const { return mEnabled; };
   protected:
      Vk::Device* mDevice;
      SharedPtr<Vk::Semaphore> mCompletedSemaphore;
      SharedPtr<Vk::Semaphore> mWaitSemaphore;
      uint32_t mWidth;
      uint32_t mHeight;
      bool mEnabled;
   };
}
