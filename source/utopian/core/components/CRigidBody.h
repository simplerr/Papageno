#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "core/Transform.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

class btRigidBody;
class btCollisionShape;

namespace Utopian
{
   class Actor;
   class CTransform;
   class CRenderable;

   enum CollisionShapeType
   {
      BOX,
      SPHERE,
      MESH,
      CAPSULE
   };

   class CRigidBody : public Component
   {
   public:
      CRigidBody(Actor* parent, CollisionShapeType collisionShape, float mass, float friction,
                 float rollingFriction, float restitution, bool kinematic, glm::vec3 anisotropicFriction);
      CRigidBody(Actor* parent);
      ~CRigidBody();

      void Update() override;
      void OnCreated() override;
      void OnDestroyed() override;
      void PostInit() override;

      void SetPosition(const glm::vec3& position);
      void SetRotation(const glm::vec3& rotation);
      void SetQuaternion(const glm::quat& quaternion);
      void ApplyCentralImpulse(const glm::vec3 impulse);
      void ApplyCentralForce(const glm::vec3 force);
      void SetVelocityXZ(const glm::vec3 velocity);
      void SetAngularVelocity(const glm::vec3 angularVelocity);

      float GetMass() const;
      float GetFriction() const;
      float GetRollingFriction() const;
      glm::vec3 GetAnisotropicFriction() const;
      float GetRestitution() const;
      glm::vec3 GetVelocity() const;
      bool IsKinematic() const;
      bool IsActive() const;

      void SetMass(float mass);
      void SetFriction(float friction);
      void SetRollingFriction(float rollingFriction);
      void SetAnisotropicFriction(const glm::vec3 anisotropicFriction);
      void SetRestitution(float restitution);
      void SetKinematic(bool isKinematic);

      // Todo: Should also recreate the btCollisionShape
      void SetCollisionShapeType(CollisionShapeType collisonShapeType);

      const Transform& GetTransform() const;

      LuaPlus::LuaObject GetLuaObject() override;

      // Type identification
      static uint32_t GetStaticType() {
         return Component::ComponentType::RIGID_BODY;
      }

      virtual uint32_t GetType() {
         return GetStaticType();
      }

      void AddToWorld();
   private:
      void RemoveFromWorld();
      void UpdateKinematicFlag();
   private:
      CTransform* mTransform;
      CRenderable* mRenderable;
      btRigidBody* mRigidBody;
      btCollisionShape* mCollisionShape;
      CollisionShapeType mCollisionShapeType;

      float mMass;
      float mFriction;
      float mRollingFriction;
      glm::vec3 mAnisotropicFriction;
      float mRestitution;
      bool mIsKinematic;
   };

   class MotionState : public btMotionState
   {
   public:
      MotionState(CRigidBody* rigidBody);

      // Engine -> Bullet
      // For dynamic bodies its called once and for kinematic bodies its called every frame
      void getWorldTransform(btTransform& worldTrans) const override;

      // Bullet -> Engine
      // Called every frame
      void setWorldTransform(const btTransform& worldTrans) override;

   private:
      CRigidBody* mRigidBody;
   };
}
