#pragma once

import <memory>;

#include <Engine/ObjectAllocator.h>
#include <Engine/Reflection/Reflected.h>
#include <Engine/Precision.h>

namespace Engine
{
	namespace Meta
	{
		struct ReflectedType;
	}

	template <typename T>
	const Meta::ReflectedType* GetMeta()
	{
		return Engine::Meta::Reflected<T>::GetMeta();
	}

	class Object
	{
	public:
		typedef std::vector<std::string> StringVector;

		std::string Name = "Object";
		std::weak_ptr<Object> This;

		virtual ~Object();

		bool ParentComponent = true;
		bool AncestorComponents = true;
		int SuperComponentHeight = -1;

		virtual void Initialize();
		virtual void Update(Float delta);

		virtual void ParentChanged(std::shared_ptr<Object> newParent) {}

		std::string GetTypeName() const;
		bool IsA(const std::string& className, bool inherited = true) const;
		bool IsA(const Meta::ReflectedType* type, bool inherited = true) const;

		void SetObjectID(int id);
		int GetObjectID() const;

		template <typename T>
		bool IsA(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> Cast();

		template <typename T>
		std::shared_ptr<T> Cast() const;

		operator std::string() const;

		template <typename T = Object>
		static std::shared_ptr<T> GetObjectFromID(int id);

		std::string GetFullName() const;
		int GetChildren() const;
		std::shared_ptr<Object> Get(const std::string& className, bool inherited = true);
		std::shared_ptr<Object> Get(const std::string& className, bool inherited = true) const;
		std::shared_ptr<Object> Get(int index);
		std::shared_ptr<Object> GetByName(const std::string& name);
		std::shared_ptr<Object> GetAncestor(const std::string& className, bool inherited = true);
		bool HasA(const std::string& className, bool inherited = true) const;

		void AddChild(const std::shared_ptr<Object>& child);
		void RemoveChild(const std::shared_ptr<Object>& child);
		void Remove();
		void SetParent(const std::shared_ptr<Object>& newParent);
		std::shared_ptr<Object> GetParent() const;
		std::shared_ptr<Object> GetComponent(const std::string& className, bool inherited = true) const;
		bool IsAncestorOf(const std::shared_ptr<Object>& object) const;
		bool IsDescendantOf(const std::shared_ptr<Object>& object) const;
		Object* GetParentRaw() const { return Parent; }

		void SetTicks(bool ticks);
		bool DoesTick() const;
		bool DoesObjectTick() const;

		bool HasChildren() const { return !Children.empty(); }

		template <typename T>
		std::shared_ptr<T> Get(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> Get(int index);

		template <typename T>
		std::shared_ptr<T> GetByName(const std::string& name);

		template <typename T>
		std::shared_ptr<T> GetAncestor(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> GetComponent(bool inherited = true) const;

		template <typename T>
		T* GetComponent2(bool inherited = true) const;

		template <typename T>
		bool HasA(bool inherited = true);

		const unsigned long long& GetCreationOrderId() const { return CreationOrderId; }

		static bool IsAlive(int objectID, unsigned long long creationOrderId);

		const Meta::ReflectedType* GetMetaData() const
		{
			return ObjectMetaData;
		}

		void SetMetaData(const Meta::ReflectedType* meta)
		{
			if (ObjectMetaData != nullptr) return;

			ObjectMetaData = meta;
		}

		bool operator==(const std::shared_ptr<Object>& object) const { return this == object.get(); }

		struct ObjectHandleData
		{
			Object* Data = nullptr;
			std::weak_ptr<Object> SmartPointer;
			unsigned long long CreationOrderId = 0;
		};

	private:
		typedef std::vector<std::shared_ptr<Object>> ObjectVector;

		static unsigned long long ObjectsCreated;

		int ObjectID = -1;
		int OriginalID = -1;
		unsigned long long CreationOrderId = 0;
		bool Ticks = false;
		int TickingChildren = 0;
		bool TickedBefore = false;

		const Meta::ReflectedType* ObjectMetaData = nullptr;
		Object* Parent = nullptr;
		ObjectVector Children;

		std::shared_ptr<Object> GetComponent(const Meta::ReflectedType* data, bool inherited) const;
		void UpdateTickingState();

		static bool MetaMatches(const Meta::ReflectedType* type, const Meta::ReflectedType* target, bool inherited);

		static const std::weak_ptr<Object>& GetHandle(int id);
	};

	template <typename T>
	std::shared_ptr<T> Object::Cast()
	{
		return std::dynamic_pointer_cast<T>(This.lock());
	}

	template <typename T>
	std::shared_ptr<T> Object::Cast() const
	{
		return std::dynamic_pointer_cast<T>(This.lock());
	}

	template <typename T, typename... Arguments>
	std::shared_ptr<T> Create(Arguments&&... arguments)
	{
		std::shared_ptr<T> object = GameObjectAllocator<T>::Create(arguments...);

		object->SetMetaData(GetMeta<T>());
		object->Name = "Object";// object->GetTypeName();
		object->Initialize();

		return object;
	}

	template <typename T>
	std::shared_ptr<T> Object::Get(bool inherited)
	{
		for (int i = 0; i < GetChildren(); ++i)
			if (MetaMatches(Get(i)->GetMetaData(), GetMeta<T>(), inherited))
				return Get(i)->Cast<T>();

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Object::Get(int index)
	{
		return Get(index)->Cast<T>();
	}

	template <typename T>
	std::shared_ptr<T> Object::GetByName(const std::string& name)
	{
		return GetByName(name)->Cast<T>();
	}

	template <typename T>
	std::shared_ptr<T> Object::GetAncestor(bool inherited)
	{
		Object* ancestor = Parent;

		while (ancestor != nullptr)
		{
			if (MetaMatches(ancestor->GetMetaData(), GetMeta<T>(), inherited))
				return ancestor;

			ancestor = ancestor->Parent;
		}

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Object::GetComponent(bool inherited) const
	{
		std::shared_ptr<Object> child = GetComponent(GetMeta<T>(), inherited);

		if (child == nullptr)
			return nullptr;

		return child->Cast<T>();
	}

	template <typename T>
	T* Object::GetComponent2(bool inherited) const
	{
		auto parent = Parent;

		if (parent != nullptr)
		{
			if (ParentComponent && dynamic_cast<T*>(parent) != nullptr)
				return reinterpret_cast<T*>(parent);

			if (AncestorComponents)
			{
				Object* ancestor = Parent;

				for (int height = 0; (SuperComponentHeight == -1 || height < SuperComponentHeight) && ancestor != nullptr; ++height)
				{
					if (AncestorComponents && dynamic_cast<T*>(ancestor) != nullptr)
						return reinterpret_cast<T*>(ancestor);

					if (ancestor->Parent != nullptr)
						ancestor = ancestor->Parent;
					else
						ancestor = nullptr;
				}
			}
		}

		return nullptr;
	}

	template <typename T>
	bool Object::HasA(bool inherited)
	{
		return HasA(T::GetClassMetaData()->Name, inherited);
	}

	template <typename T>
	bool Object::IsA(bool inherited)
	{
		return MetaMatches(GetMetaData(), GetMeta<T>(), inherited);
	}

	template <typename T>
	std::shared_ptr<T> Object::GetObjectFromID(int id)
	{
		if (id != -1)
			return GetHandle(id).lock()->Cast<T>();

		return nullptr;
	}
}
