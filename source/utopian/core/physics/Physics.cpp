#include "core/physics/Physics.h"
#include "core/physics/PhysicsDebugDraw.h"
#include "core/Log.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include <core/components/CRigidBody.h>
#include <core/physics/BulletHelpers.h>
#include <limits>

namespace Utopian
{
   Physics& gPhysics()
   {
      return Physics::Instance();
   }

   Physics::Physics()
   {
      // Note: Experienced heap corruption when BT_USE_DOUBLE_PRECISION was not added to the preprocessor
      mBroadphase = new btDbvtBroadphase();
      mCollisionConfiguration = new btDefaultCollisionConfiguration();
      mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
      mConstraintSolver = new btSequentialImpulseConstraintSolver();
      mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mConstraintSolver, mCollisionConfiguration);

      mDynamicsWorld->setGravity(btVector3(mGravity.x, mGravity.y, mGravity.z));

      mDebugDrawer = new PhysicsDebugDraw();
      mDynamicsWorld->setDebugDrawer(mDebugDrawer);

      mTerrainBody = nullptr;

      mEnabled = true;
      mDebugDrawEnabled = false;

      mLastFrameTime = gTimer().GetTimestamp();

      // Add ground shape for experimentation
      //AddGroundShape();
   }

   Physics::~Physics()
   {
      delete mDynamicsWorld;
      delete mConstraintSolver;
      delete mDispatcher;
      delete mCollisionConfiguration;
      delete mBroadphase;
      delete mDebugDrawer;
      delete mTerrainBody;
   }

   void Physics::Update()
   {
      // Update elapsed time
      double deltaTime = gTimer().GetElapsedTime(mLastFrameTime);
      deltaTime /= 1000.0f; // To seconds
      mLastFrameTime = gTimer().GetTimestamp();

      if (IsEnabled())
      {
         mDynamicsWorld->stepSimulation(deltaTime, 5);
      }

      if (IsDebugDrawEnabled())
      {
         mDynamicsWorld->debugDrawWorld();
      }
   }

   void Physics::EnableSimulation(bool enable)
   {
      mEnabled = enable;
   }

   void Physics::EnableDebugDraw(bool enable)
   {
      mDebugDrawEnabled = enable;
   }

   void Physics::AddGroundShape()
   {
      btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(5000.), btScalar(500.), btScalar(5000.)));

      btTransform groundTransform;
      groundTransform.setIdentity();
      groundTransform.setOrigin(btVector3(0, -500, 0));

      btScalar mass(0.);

      bool isDynamic = (mass != 0.f);

      btVector3 localInertia(0, 0, 0);
      if (isDynamic)
         groundShape->calculateLocalInertia(mass, localInertia);

      btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
      rbInfo.m_restitution = 0.0f;
      rbInfo.m_friction = 1.0f;

      btRigidBody* body = new btRigidBody(rbInfo);

      mDynamicsWorld->addRigidBody(body);
   }

   void Physics::SetHeightmap(const float* heightmap, const uint32_t size, float scale, float terrainSize)
   {
      // Bullet expects the UV coordinates to be flipped.
      // Terrain::GeneratePatches() calculates them as Pos(0, 0) = Tex(0, 0) while
      // Bullets expects Pos(0, 0) = Tex(1, 1)
      for (uint32_t i = 0; i < size * size; i++)
      {
         uint32_t x = i % size;
         uint32_t y = i / size;
         uint32_t index = ((size - 1) - x) + (((size - 1) - y) * size);
         mHeightmapCopy[i] = -heightmap[index]; // Note: The negative sign
      }

      double minHeight = std::numeric_limits<double>::max();
      double maxHeight = std::numeric_limits<double>::min();

      for (uint32_t i = 0; i < size * size; i++)
      {
         if (mHeightmapCopy[i] < minHeight)
            minHeight = mHeightmapCopy[i];
         else if (mHeightmapCopy[i] > maxHeight)
            maxHeight = mHeightmapCopy[i];
      }

      btHeightfieldTerrainShape* terrainShape = new btHeightfieldTerrainShape(size, size, mHeightmapCopy, scale, minHeight, maxHeight, 1, PHY_FLOAT, false);

      float gridScaling = terrainSize / size;
      btVector3 localScaling = btVector3(gridScaling, scale, gridScaling);
      terrainShape->setLocalScaling(localScaling);

      float minHeightScaled = (float)minHeight * scale;
      float maxHeightScaled = (float)maxHeight * scale;

      btTransform groundTransform;
      groundTransform.setIdentity();
      groundTransform.setOrigin(btVector3(0, (minHeightScaled + (maxHeightScaled - minHeightScaled) / 2.0f), 0));

      btScalar mass(0.0f);
      btVector3 localInertia(0, 0, 0);
      btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, terrainShape, localInertia);
      rbInfo.m_restitution = 0.0f;
      rbInfo.m_friction = 1.0f;

      if (mTerrainBody != nullptr)
      {
         mDynamicsWorld->removeRigidBody(mTerrainBody);
         delete mTerrainBody;
      }

      btRigidBody* mTerrainBody = new btRigidBody(rbInfo);
      mDynamicsWorld->addRigidBody(mTerrainBody);
   }

   void Physics::Draw()
   {

   }

   IntersectionInfo Physics::RayIntersection(const Ray& ray)
   {
      IntersectionInfo intersectInfo;
      const float maxRayDistance = 1000.0f;
      btVector3 from = ToBulletVec3(ray.origin);
      btVector3 to = ToBulletVec3(ray.direction * maxRayDistance);

      btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
      closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

      mDynamicsWorld->rayTest(from, to, closestResults);

      if (closestResults.hasHit())
      {
         void* userPtr = closestResults.m_collisionObject->getUserPointer();
         glm::vec3 n = ToVec3(closestResults.m_hitNormalWorld);
         glm::vec3 p = ToVec3(closestResults.m_hitPointWorld);

         CRigidBody* rigidBody = static_cast<CRigidBody*>(userPtr);

         if (rigidBody != nullptr)
         {
            intersectInfo.actor = rigidBody->GetParent();
            intersectInfo.normal = n; // Todo: Note: Normal calculation is incorrect when objects are rotated
            intersectInfo.distance = glm::distance(ray.origin, p);
            intersectInfo.hit = true;
         }

         // UTO_LOG("nx: " + std::to_string(n.x) + " ny: " + std::to_string(n.y) + " nz: " + std::to_string(n.z));
         // UTO_LOG("px: " + std::to_string(p.x) + " py: " + std::to_string(p.y) + " pz: " + std::to_string(p.z));
         // UTO_LOG("ptr: " + std::to_string((uint64_t)userPtr));
      }

      return intersectInfo;
   }

   bool Physics::IsOnGround(CRigidBody* rigidBody)
   {
      bool onGround = false;

      int numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();
      for (int i = 0; i < numManifolds; i++)
      {
         btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
         if (contactManifold->getNumContacts() > 0)
         {
            const btCollisionObject* objectA = contactManifold->getBody0();
            const btCollisionObject* objectB = contactManifold->getBody1();

            btRigidBody* otherBody = nullptr;
            float sign = 1.0f;
            if (objectA->getUserPointer() == rigidBody)
               otherBody = static_cast<btRigidBody*>((btCollisionObject*)(objectB));
            else if (objectB->getUserPointer() == rigidBody) {
               otherBody = static_cast<btRigidBody*>((btCollisionObject*)(objectA));
               sign = -1.0f;
            }

            if (otherBody != nullptr)
            {
               glm::vec3 normal = sign * ToVec3(contactManifold->getContactPoint(0).m_normalWorldOnB);

               const float threshold = 0.001f;
               if (glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f)) > threshold)
               {
                  onGround = true;
                  // UTO_LOG("Player collision! nx: " + std::to_string(normal.x) +
                  //       " ny: " + std::to_string(normal.y) +
                  //       " nz: " + std::to_string(normal.z));
               }
            }
         }
      }

      return onGround;
   }

   btDiscreteDynamicsWorld* Physics::GetDynamicsWorld() const
   {
      return mDynamicsWorld;
   }

   bool Physics::IsEnabled() const
   {
      return mEnabled;
   }
   
   bool Physics::IsDebugDrawEnabled() const
   {
      return mDebugDrawEnabled;
   }
}