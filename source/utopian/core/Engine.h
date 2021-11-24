#pragma once
#include <functional>
#include <string>
#include "vulkan/VulkanApp.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "core/renderer/RenderSettings.h"

namespace Utopian
{
   class Engine;

   class EnginePlugin
   {
   public:
      EnginePlugin() {};
      virtual ~EnginePlugin() {};

      virtual void Start(Engine* engine) = 0;
      virtual void PostInit(Engine* engine) = 0;
      virtual void Destroy() = 0;
      virtual void Update(double deltaTime) = 0;
      virtual void Draw() {};
      virtual void NewFrame() {};
      virtual void EndFrame() {};
   };

   class DeferredRenderingPlugin : public EnginePlugin
   {
   public:
      DeferredRenderingPlugin(const std::string& settingsFile);
      virtual void Start(Engine* engine) override;
      virtual void PostInit(Engine* engine) override;
      virtual void Destroy() override;
      virtual void Update(double deltaTime) override;
      virtual void Draw() override;
      virtual void NewFrame() override;
      virtual void EndFrame() override;

   private:
      void LoadSettingsFromFile(Engine* engine);
   private:
      RenderingSettings mRenderingSettings;
      std::string mSettingsFile;
   };

   class ECSPlugin : public EnginePlugin
   {
   public:
      virtual void Start(Engine* engine) override;
      virtual void PostInit(Engine* engine) override;
      virtual void Destroy() override;
      virtual void Update(double deltaTime) override;
      virtual void Draw() override;
      virtual void NewFrame() override;
      virtual void EndFrame() override;
   };

   class Engine : public Module<Engine>
   {
   public:
      Engine(Window* window, const std::string& appName);
      ~Engine();

      /** Executes the main loop and calls Engine::Tick() every frame. */
      void Run();

      /** Handles Win32 messages and forwards them to needed modules. */
      void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

      /** Registers a callback function to be called in Engine::Update(). */
      template<class ...Args>
      void RegisterUpdateCallback(Args &&...args)
      {
         mUpdateCallback = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
      }

      /** Registers a callback function to be called in Engine::Render(). */
      template<class ...Args>
      void RegisterRenderCallback(Args &&...args)
      {
         mRenderCallback = std::bind(std::forward<Args>(args)...);
      }

      /** Registers a callback function to be called in Engine::~Engine()
       *  All Vulkan resources used by the application needs to be destroyed in this callback. */
      template<class ...Args>
      void RegisterDestroyCallback(Args &&...args)
      {
         mDestroyCallback = std::bind(std::forward<Args>(args)...);
      }

      /** Registers a callback function to be called before each frame begins.
        * Inside of this function Vulkan resources will not be used in a command
        * buffer and can modified. */
      template<class ...Args>
      void RegisterPreFrameCallback(Args &&...args)
      {
         mPreFrameCallback = std::bind(std::forward<Args>(args)...);
      }

      /** Starts all the modules included in the engine. */
      void StartModules();

      void AddPlugin(SharedPtr<EnginePlugin> plugin);

      /** Sets the source .lua file for the current scene. */
      void SetSceneSource(std::string sceneSource);

      Vk::VulkanApp* GetVulkanApp();
      ImGuiRenderer* GetImGuiRenderer();
      std::string GetSceneSource() const;

   private:
      /**
       * Calls the per frame update function of all modules in the engine.
       * 
       * @note Also calls registered application callback functions.
       */
      void Tick();

      /** Updates all modules included in the engine. */
      void Update(double deltaTime);

      /** Renders all modules included in the engine. */
      void Render();

   private:
      SharedPtr<Vk::VulkanApp> mVulkanApp;
      SharedPtr<ImGuiRenderer> mImGuiRenderer;
      std::vector<SharedPtr<EnginePlugin>> mPlugins;
      Window* mWindow;
      std::function<void(double)> mUpdateCallback = nullptr;
      std::function<void()> mRenderCallback = nullptr;
      std::function<void()> mDestroyCallback = nullptr;
      std::function<void()> mPreFrameCallback = nullptr;
      std::string mAppName;
      std::string mSceneSource;
      Timestamp mLastFrameTime;
   };

   /** Returns an instance to the Engine module. */
   Engine& gEngine();
}
