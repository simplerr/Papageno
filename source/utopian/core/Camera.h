#pragma once

#include <glm/glm.hpp>
#include "utility/Platform.h"
#include "utility/math/Ray.h"
#include "utility/math/Frustum.h"
#include "utility/Common.h"
#include "core/SceneNode.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian
{
   class Window;

   class Camera : public Utopian::SceneNode
   {
   public:
      Camera(Utopian::Window* window, glm::vec3 position, float fieldOfView, float nearPlane, float farPlane);
      ~Camera();

      static SharedPtr<Camera> Create(Utopian::Window* window, glm::vec3 position, float fieldOfView, float nearPlane, float farPlane);
      void Initialize();
      void OnDestroyed();

      void UpdateFrustum();

      void SetFov(float fov);
      void SetNearPlane(float nearPlane);
      void SetFarPlane(float farPlane);
      void SetWindow(Utopian::Window* window);
      void SetAspectRatio(float aspectRatio);

      Ray GetPickingRay();

      glm::vec3 GetDirection();

      // New
      glm::mat4 GetOrientation();
      glm::mat4 GetView();
      glm::mat4 GetProjection();
      glm::mat4 GetMatrix();
      glm::vec3 GetRight();
      glm::vec3 GetTarget();
      glm::vec3 GetUp();
      glm::vec3 GetLookAt();
      float GetPitch();
      float GetYaw();
      float GetFov() const;
      float GetNearPlane() const;
      float GetFarPlane() const;
      const Frustum& GetFrustum() const;

      void AddOrientation(float yaw, float pitch);
      void SetOrientation(float yaw, float pitch);
      void LookAt(glm::vec3 target);
      void CapAngles();

      // [NOTE][HACK] Vulkan & OpenGL have different pitch movement
      int hack = 1;
   private:
      Frustum mFrustum;
      Utopian::Window* mWindow;
      glm::vec3 mUp;
      glm::vec3 mLookAt; // Note: This is only the initial target position

      float mPitch;  // Vertical angle
      float mYaw;    // Horizontal angle
      float mFov;
      float mNearPlane;
      float mFarPlane;
      float mAspectRatio;
   };
}  // VulkanLib namespace