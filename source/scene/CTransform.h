#pragma once
#include <glm/glm.hpp>
#include "SceneComponent.h"
#include "scene/Transform.h"

using namespace glm;

namespace Scene
{
	class SceneEntity;

	class CTransform : public SceneComponent
	{
	public:
		CTransform(SceneEntity* parent, const vec3& position);
		~CTransform();

		void SetTransform(const Transform& transform);
		void SetPosition(const vec3& position);
		void SetRotation(const vec3& rotation);
		void SetScale(const vec3& scale);

		void AddRotation(float x, float y, float z);
		void AddRotation(const vec3& rotation);
		void AddScale(float x, float y, float z);
		void AddScale(const vec3& scale);

		const Transform& GetTransform() const;
		const vec3& GetPosition() const;
		const vec3& GetRotation() const;
		const vec3& GetScale() const;
		const mat4& GetWorldMatrix() const;
		const mat4& GetWorldInverseTransposeMatrix() const;

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::TRANSFORM;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		Transform mTransform;
	};
}