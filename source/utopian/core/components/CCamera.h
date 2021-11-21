#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
   class Actor;

   class CCamera : public Component
   {
   public:
      CCamera(Actor* parent, Utopian::Window* window, float fieldOfView, float nearPlane, float farPlane);
      ~CCamera();

      void Update(double deltaTime) override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      LuaPlus::LuaObject GetLuaObject() override;

      // Setters
      void LookAt(const glm::vec3& target);
      void AddOrientation(float yaw, float pitch);
      void SetOrientation(float yaw, float pitch);
      void SetFov(float fov);
      void SetNearPlane(float nearPlane);
      void SetFarPlane(float farPlane);
      void SetAspectRatio(float aspectRatio);
      void SetWindow(Utopian::Window* window);
      void SetMainCamera();

      // Getters
      glm::vec3 GetDirection() const;
      glm::vec3 GetTarget() const;
      glm::vec3 GetRight() const;
      glm::vec3 GetUp() const;
      glm::vec3 GetLookAt() const;
      float GetPitch() const;
      float GetYaw() const;
      float GetFov() const;
      float GetNearPlane() const;
      float GetFarPlane() const;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::CAMERA;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      SharedPtr<Utopian::Camera> mInternal;
   };
}
