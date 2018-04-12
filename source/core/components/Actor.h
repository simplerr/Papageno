#pragma once
#include <string>
#include <type_traits>
#include <vector>
#include "core/components/Component.h"
#include "utility/Common.h"
#include "core/Object.h"
#include "core/ObjectManager.h"
#include "core/World.h"
#include "utility/math/Ray.h"

using namespace std;

namespace Utopian
{
	class CTransform;

	class Actor : public Object
	{
	public:
		Actor(string name);

		static SharedPtr<Actor> Create(string name);

		BoundingBox GetBoundingBox() const;
		const Transform& GetTransform() const;

		/*
		 * Component management functions
		*/
		template<class T, class... Args>
		T* AddComponent(Args &&... args)
		{
			static_assert((std::is_base_of<Component, T>::value), "Specified type is not a valid Component.");

			SharedPtr<T> newComponent(new T(this, std::forward<Args>(args)...));

			ObjectManager::Instance().RegisterObject(newComponent);
			World::Instance().NotifyComponentCreated(newComponent.get());

			mComponents.push_back(newComponent.get());

			return newComponent.get();
		}

		template <typename T>
		bool HasComponent() const
		{
			bool result = false;
			for(auto& entry : mComponents)
			{
				if (entry->GetType() == T::GetStaticType())
					result = true;
			}

			return result;
		}

		template <typename T>
		T* GetComponent() const
		{
			static_assert((std::is_base_of<Component, T>::value), "Specified type is not a valid Component.");

			T* component = GetComponent<T>(0);// T::GetType());

			return component;
		}

		template <typename T>
		T* GetComponent(uint32_t type) const
		{
			for(auto& entry : mComponents)
			{
				if (entry->GetType() == T::GetStaticType())
					return dynamic_cast<T*>(entry);
			}

			return nullptr;
		}

	private:
		vector<Component*> mComponents;
		bool mHasTransform;
	};
}