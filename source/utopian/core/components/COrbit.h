#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
   class Actor;
   class CCamera;
   class CTransform;

   class COrbit : public Component
   {
   public:
      COrbit(Actor* parent, float speed);
      ~COrbit();

      void Update(double deltaTime) override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      // Setters
      void SetSpeed(float speed);
      void SetRadius(float radius);
      void SetTarget(const glm::vec3& target);

      // Getters
      float GetSpeed() const;
      const glm::vec3& GetTarget() const;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::ORBIT;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

   private:
      CCamera* mCamera; // For convenience
      CTransform* mTransform;
      glm::vec3 mTarget;
      float mSpeed;
      float mRadius;
      float mCounter;
   };
}
