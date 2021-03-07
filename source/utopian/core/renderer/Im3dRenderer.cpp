#include "core/renderer/Im3dRenderer.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RendererUtility.h"
#include "core/Camera.h"
#include "core/Input.h"
#include "im3d/im3d.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/EffectManager.h"
#include "vulkan/Effect.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
   Im3dRenderer::Im3dRenderer(Vk::VulkanApp* vulkanApp, glm::vec2 viewportSize)
   {
      mViewportSize = viewportSize;
      mVulkanApp = vulkanApp;

      mVertexBuffer = std::make_shared<Vk::Buffer>(vulkanApp->GetDevice(), "Im3d vertex buffer");
      mVertexCount = 0;
   }

   Im3dRenderer::~Im3dRenderer()
   {
      mVertexBuffer->UnmapMemory();
   }

   SharedPtr<Vk::Buffer> Im3dRenderer::GetVertexBuffer()
   {
      return mVertexBuffer;
   }

   void Im3dRenderer::NewFrame()
   {
      Im3d::AppData& appData = Im3d::GetAppData();
      Camera* camera = gRenderer().GetMainCamera();

      //appData.m_deltaTime = 0.0f; // Todo
      appData.m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
      appData.m_viewOrigin = camera->GetPosition();
      appData.m_viewDirection = camera->GetDirection();
      appData.m_cursorRayOrigin = camera->GetPickingRay().origin;
      appData.m_cursorRayDirection = camera->GetPickingRay().direction;
      appData.m_projOrtho = false;
      appData.m_projScaleY = tanf(glm::radians(camera->GetFov()) * 0.5f) * 2.0f;
      appData.m_viewportSize = mViewportSize;
      appData.m_snapTranslation = 0.0f;
      appData.m_snapRotation = 0.0f;
      appData.m_snapScale = 0.0f;

      bool ctrlDown = gInput().KeyDown(VK_LCONTROL);
      appData.m_keyDown[Im3d::Mouse_Left] = gInput().KeyDown(VK_LBUTTON);
      appData.m_keyDown[Im3d::Key_L] = ctrlDown && gInput().KeyPressed('L');
      appData.m_keyDown[Im3d::Key_T] = ctrlDown && gInput().KeyPressed('T');
      appData.m_keyDown[Im3d::Key_R] = ctrlDown && gInput().KeyPressed('R');
      appData.m_keyDown[Im3d::Key_S] = ctrlDown && gInput().KeyPressed('S');

      Im3d::NewFrame();
   }

   void Im3dRenderer::EndFrame()
   {
      Im3d::EndFrame();
   }

   void Im3dRenderer::UploadVertexData()
   {
      Im3d::AppData& appData = Im3d::GetAppData();

      uint32_t totalNumVertices = GetTotalNumVertices();

      if ((mVertexBuffer->GetVkHandle() == VK_NULL_HANDLE) || (mVertexCount < totalNumVertices))
      {
         VkDeviceSize vertexBufferSize = totalNumVertices * sizeof(Im3d::VertexData);

         mVertexBuffer->UnmapMemory();
         mVulkanApp->GetDevice()->QueueDestroy(mVertexBuffer);
         mVertexBuffer = std::make_shared<Vk::Buffer>(mVulkanApp->GetDevice(), "Im3d vertex buffer");

         mVertexBuffer->Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
         mVertexCount = totalNumVertices;
         mVertexBuffer->MapMemory((void**)&mMappedVertices);
      }

      Im3d::VertexData* vertexDst = mMappedVertices;

      for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
      {
         const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

         memcpy(vertexDst, drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));
         vertexDst += drawList.m_vertexCount;
      }
   }

   uint32_t Im3dRenderer::GetTotalNumVertices()
   {
      uint32_t numVertices = 0;
      for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
      {
         const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];
         numVertices += drawList.m_vertexCount;
      }

      return numVertices;
   }
}