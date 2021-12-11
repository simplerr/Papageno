#include "core/renderer/RendererUtility.h"
#include "core/renderer/Im3dRenderer.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/Engine.h"
#include "core/Input.h"
#include "core/World.h"
#include "core/physics/Physics.h"
#include "core/renderer/Renderer.h"
#include "core/LuaManager.h"
#include "core/ScriptExports.h"
#include "core/AssetLoader.h"
#include "core/ActorFactory.h"
#include "core/Profiler.h"
#include "core/Log.h"
#include "vulkan/EffectManager.h"
#include "core/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/Debug.h"
#include "vulkan/VulkanApp.h"
#include "imgui/imgui.h"
#include <core/renderer/RenderSettings.h>

namespace Utopian
{
   Engine::Engine(Window* window, const std::string& appName)
      : mAppName(appName)
   {
      srand((unsigned int)time(NULL));

      Vk::Debug::TogglePerformanceWarnings();
      Vk::Debug::SetupDebugLayers();

      mWindow = window;
      mWindow->SetTitle(mAppName);

      gLog().Start();

      mVulkanApp = std::make_shared<Vk::VulkanApp>(window);
      mVulkanApp->Prepare();

      mLastFrameTime = gTimer().GetTimestamp();

      UTO_LOG(mAppName);
   }
   
   Engine::~Engine()
   {
      // Vulkan resources cannot be destroyed when they are in use on the GPU
      while (!mVulkanApp->PreviousFrameComplete())
      {
      }

      // Call application destroy function
      mDestroyCallback();

      Vk::gShaderFactory().Destroy();
      Vk::gEffectManager().Destroy();
      Vk::gTextureLoader().Destroy();

      gModelLoader().Destroy();
      gTimer().Destroy();
      gInput().Destroy();
      gLuaManager().Destroy();
      gScreenQuadUi().Destroy();
      gProfiler().Destroy();
      gRendererUtility().Destroy();

      // Destroy all plugins
      for(auto& plugin : mPlugins)
         plugin->Destroy();

      mImGuiRenderer->GarbageCollect();
   }

   void Engine::StartModules()
   {
      UTO_LOG("Starting engine modules");

      Vk::Device* device = mVulkanApp->GetDevice();
      Vk::gEffectManager().Start();
      Vk::gTextureLoader().Start(device);
      Vk::gShaderFactory().Start(device);
      Vk::gShaderFactory().AddIncludeDirectory("data/shaders/include");
      Vk::gShaderFactory().AddIncludeDirectory("data/shaders/");

      gModelLoader().Start(device);
      gTimer().Start();
      gInput().Start();
      gLuaManager().Start();
      gProfiler().Start(mVulkanApp.get());
      gRendererUtility().Start();
      gScreenQuadUi().Start(mVulkanApp.get());

      mImGuiRenderer = std::make_shared<ImGuiRenderer>(mVulkanApp.get(), mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

      for(auto& plugin : mPlugins)
         plugin->Start(this);

      for(auto& plugin : mPlugins)
         plugin->PostInit(this);

      UTO_LOG("Engine modules ready");
   }

   void Engine::Run()
   {
      while (true)
      {
         bool closeWindow = mWindow->DispatchMessages();

         if (!closeWindow)
         {
            Tick();
         }
         else
         {
            break;
         }
      }
   }

   void Engine::Tick()
   {
      double deltaTime = gTimer().GetElapsedTime(mLastFrameTime);
      deltaTime /= 1000.0f; // To seconds
      mLastFrameTime = gTimer().GetTimestamp();

      Update(deltaTime);
      Render();

      gInput().Update(0);
   }

   void Engine::Update(double deltaTime)
   {
      for(auto& plugin : mPlugins)
         plugin->NewFrame();

      mImGuiRenderer->NewFrame();

      for(auto& plugin : mPlugins)
         plugin->Update(deltaTime);

      gProfiler().Update(deltaTime);

      Vk::gEffectManager().Update(deltaTime);

      // Call the application Update() function
      mUpdateCallback(deltaTime);

      for(auto& plugin : mPlugins)
         plugin->EndFrame();

      mImGuiRenderer->EndFrame();
   }

   void Engine::Render()
   {
      if (mVulkanApp->PreviousFrameComplete())
      {
         mImGuiRenderer->GarbageCollect();

         if (mPreFrameCallback != nullptr)
            mPreFrameCallback();

         mVulkanApp->PrepareFrame();

         for(auto& plugin : mPlugins)
            plugin->Draw();

         mImGuiRenderer->Render();

         // Call the application Render() function
         mRenderCallback();

         mVulkanApp->Render();

         mVulkanApp->SubmitFrame();

         gTimer().CalculateFrameTime();
      }
   }
   
   void Engine::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      mVulkanApp->HandleMessages(hWnd, uMsg, wParam, lParam);

      gInput().HandleMessages(uMsg, wParam, lParam);

      switch (uMsg)
      {
      case WM_CLOSE:
         PostQuitMessage(0);
         break;
      default:
         DefWindowProc(hWnd, uMsg, wParam, lParam);
      }
   }

   void Engine::AddPlugin(SharedPtr<EnginePlugin> plugin)
   {
      mPlugins.push_back(plugin);
   }

   Vk::VulkanApp* Engine::GetVulkanApp()
   {
      return mVulkanApp.get();
   }

   ImGuiRenderer* Engine::GetImGuiRenderer()
   {
      return mImGuiRenderer.get();
   }

   void Engine::SetSceneSource(std::string sceneSource)
   {
      mSceneSource = sceneSource;
   }

   std::string Engine::GetSceneSource() const
   {
      return mSceneSource;
   }

   Engine& gEngine()
   {
      return Engine::Instance();
   }

   DeferredRenderingPlugin::DeferredRenderingPlugin(const std::string& settingsFile)
   {
      mRenderingSettings = RenderingSettings();
      mSettingsFile = settingsFile;
   }

   void DeferredRenderingPlugin::Start(Engine* engine)
   {
      LoadSettingsFromFile(mRenderingSettings, engine, mSettingsFile);

      gRenderer().Start(engine->GetVulkanApp());
      gRenderer().SetUiOverlay(engine->GetImGuiRenderer());
      gRenderer().SetRenderingSettings(mRenderingSettings);

      // Todo: There is a dependency between loading the actors from Lua and the terrain creation
      // Terrain needs to be created before World::Instance().LoadScene();
      gRenderer().PostWorldInit();
   }

   void DeferredRenderingPlugin::PostInit(Engine* engine)
   {

   }

   void DeferredRenderingPlugin::Destroy()
   {
      gRenderer().Destroy();
   }

   void DeferredRenderingPlugin::Update(double deltaTime)
   {
      gRenderer().Update(deltaTime);
   }

   void DeferredRenderingPlugin::Draw()
   {
      gRenderer().Render();
   }

   void DeferredRenderingPlugin::NewFrame()
   {
      gRenderer().NewUiFrame();
   }

   void DeferredRenderingPlugin::EndFrame()
   {
      gRenderer().EndUiFrame();
   }

   void ECSPlugin::Start(Engine* engine)
   {
      gWorld().Start();
      gPhysics().Start();
      gAssetLoader().Start();
   }

   void ECSPlugin::PostInit(Engine* engine)
   {
      ActorFactory::LoadFromFile(engine->GetVulkanApp()->GetWindow(), engine->GetSceneSource());
      gWorld().LoadProceduralAssets();

      ScriptExports::Register();
      ScriptImports::Register();
      //gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");
   }

   void ECSPlugin::Destroy()
   {
      gWorld().Destroy();
      gAssetLoader().Destroy();
      gPhysics().Destroy();
   }

   void ECSPlugin::Update(double deltaTime)
   {
      gWorld().Update(deltaTime);
      gPhysics().Update(deltaTime);
   }

   void ECSPlugin::Draw()
   {
      gWorld().RemoveDeadActors();
      gWorld().Render();
   }

   void ECSPlugin::NewFrame()
   {
   }

   void ECSPlugin::EndFrame()
   {
   }
}