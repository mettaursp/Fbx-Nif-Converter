#include "Object.h"

import <iostream>;

#include <Engine/IdentifierHeap.h>
#include <Engine/Reflection/MetaData.h>

namespace Engine
{
	typedef IDHeap<Object::ObjectHandleData> ObjectHandleHeap;

	ObjectHandleHeap ObjectIDs;

	std::shared_ptr<Object> Null = nullptr;

	unsigned long long Object::ObjectsCreated = 0;

	void Object::Initialize()
	{
		ObjectID = ObjectIDs.RequestID(ObjectHandleData{ this, This, ++ObjectsCreated });
		OriginalID = ObjectID;
		CreationOrderId = ObjectsCreated;
	}

	bool Object::IsAlive(int objectId, unsigned long long creationOrderId)
	{
		return ObjectIDs.NodeAllocated(objectId) && ObjectIDs.GetNode(objectId).GetData().CreationOrderId == creationOrderId;
	}

	std::string Object::GetTypeName() const
	{
		return GetMetaData()->Name;
	}

	bool Object::IsA(const std::string& className, bool inherited) const
	{
		const Meta::ReflectedType* data = GetMetaData();
		
		if (data->Name == className) return true;

		if (inherited)
			for (int i = (int)data->Inherits.size() - 2; i >= 0; --i)
				if (data->Inherits[i]->Name == className)
					return true;

		return false;
	}

	bool Object::IsA(const Meta::ReflectedType* metadata, bool inherited) const
	{
		const Meta::ReflectedType* data = GetMetaData();

		return data == metadata || data->InheritsType(metadata);
	}

	void Object::SetObjectID(int id)
	{
		if (ObjectID != -1)
			throw "Attempt to set ID";

		ObjectID = id;

		if (id != -1)
			OriginalID = id;
	}

	int Object::GetObjectID() const
	{
		return ObjectID;
	}

	Object::operator std::string() const
	{
		return Name;
	}

	void Object::Update(Float delta)
	{
		for (int i = 0; i < GetChildren(); ++i)
		{
			if (Children[i] != nullptr)// && Children[i]->DoesTick())
				Children[i]->Update(delta);
			else if (Children[i] == nullptr)
				throw "bad child detected";
		}
	}

	Object::~Object()
	{
		auto parent = Parent;

		if (parent != nullptr)
			parent->RemoveChild(This.lock());

		while (GetChildren() > 0)
		{
			if (Children.back() != nullptr)
				Children.back()->Parent = nullptr;

			Children.pop_back();
		}

		ObjectIDs.Release(ObjectID);
	}

	std::string Object::GetFullName() const
	{
		if (Parent == nullptr)
			return Name;

		std::string name = Name;
		
		for (Object* parent = Parent; parent != nullptr && parent->Parent != nullptr; parent = parent->Parent)
			name = parent->Name + "." + name;

		return name;
	}

	int Object::GetChildren() const
	{
		return int(Children.size());
	}

	std::shared_ptr<Object> Object::Get(const std::string& className, bool inherited)
	{
		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->IsA(className, inherited))
				return Children[i];

		return nullptr;
	}

	std::shared_ptr<Object> Object::Get(const std::string& className, bool inherited) const
	{
		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->IsA(className, inherited))
				return Children[i];

		return nullptr;
	}

	std::shared_ptr<Object> Object::Get(int index)
	{
		if (index < 0 || index >= int(Children.size()))
			return nullptr;

		return Children[index];
	}

	std::shared_ptr<Object> Object::GetByName(const std::string& name)
	{
		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->Name == name)
				return Children[i];

		return nullptr;
	}

	std::shared_ptr<Object> Object::GetAncestor(const std::string& className, bool inherited)
	{
		auto ancestor = Parent;

		while (ancestor != nullptr)
		{
			if (ancestor->IsA(className, inherited))
				return ancestor->This.lock();

			ancestor = ancestor->Parent;
		}

		return nullptr;
	}

	std::shared_ptr<Object> Object::GetComponent(const std::string& className, bool inherited) const
	{
		auto parent = Parent;

		if (parent != nullptr)
		{
			if (ParentComponent && parent->IsA(className, inherited))
				return parent->This.lock();

			if (AncestorComponents)
			{
				Object* ancestor = Parent;

				for (int height = 0; (SuperComponentHeight == -1 || height < SuperComponentHeight) && ancestor != nullptr; ++height)
				{
					if (AncestorComponents && ancestor->IsA(className, inherited))
						return ancestor->This.lock();

					if (ancestor->Parent != nullptr)
						ancestor = ancestor->Parent;
					else
						ancestor = nullptr;
				}
			}
		}

		return nullptr;
	}

	std::shared_ptr<Object> Object::GetComponent(const Meta::ReflectedType* data, bool inherited) const
	{
		auto parent = Parent;

		if (parent != nullptr)
		{
			if (ParentComponent && parent->IsA(data, inherited))
				return parent->This.lock();

			if (AncestorComponents)
			{
				Object* ancestor = Parent;

				for (int height = 0; (SuperComponentHeight == -1 || height < SuperComponentHeight) && ancestor != nullptr; ++height)
				{
					if (AncestorComponents && ancestor->IsA(data, inherited))
						return ancestor->This.lock();

					if (ancestor != nullptr)
						ancestor = ancestor->Parent;
					else
						ancestor = nullptr;
				}
			}
		}

		return nullptr;
	}

	bool Object::HasA(const std::string& className, bool inherited) const
	{
		if (IsA(className))
			return true;

		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->IsA(className, inherited))
				return true;

		return false;
	}

	void Object::AddChild(const std::shared_ptr<Object>& child)
	{
		child->Parent = This.lock().get();

		if (child->DoesTick())
		{
			++TickingChildren;

			UpdateTickingState();
		}

		Children.push_back(child);
	}

	void Object::Remove()
	{
		SetParent(nullptr);
	}

	void Object::RemoveChild(const std::shared_ptr<Object>& child)
	{
		if (child->DoesTick())
		{
			--TickingChildren;

			UpdateTickingState();
		}

		int index = 0;

		while (index < int(Children.size()) && Children[index] != child)
			++index;

		if (index < int(Children.size()))
		{
			if (index < int(Children.size()) - 1)
				Children[index] = Children[Children.size() - 1];

			Children.pop_back();
		}
	}

	void Object::SetParent(const std::shared_ptr<Object>& newParent)
	{
		if (newParent != nullptr && IsAncestorOf(newParent))
			throw "Attempt to create circular reference: " + GetTypeName() + " '" + Name + "' is already an ancestor of " + newParent->GetTypeName() + " '" + newParent->Name + "'";

		auto parent = Parent;

		if (parent != nullptr)
			parent->RemoveChild(This.lock());

		if (newParent != nullptr)
			newParent->AddChild(This.lock());
		else
			Parent = nullptr;

		ParentChanged(newParent);
	}

	std::shared_ptr<Object> Object::GetParent() const
	{
		return Parent->This.lock();
	}

	bool Object::IsAncestorOf(const std::shared_ptr<Object>& object) const
	{
		if (object == nullptr)
			return false;

		return object->IsDescendantOf(This.lock());
	}

	bool Object::IsDescendantOf(const std::shared_ptr<Object>& object) const
	{
		if (object == nullptr)
			return false;

		auto focus = this;

		while (focus != nullptr)
		{
			if (focus == object.get())
				return true;

			focus = focus->Parent;
		}

		return false;
	}

	void Object::SetTicks(bool ticks)
	{
		Ticks = ticks;

		UpdateTickingState();
	}

	bool Object::DoesObjectTick() const
	{
		return Ticks;
	}

	bool Object::DoesTick() const
	{
		return Ticks || TickingChildren > 0;
	}

	void Object::UpdateTickingState()
	{
		bool ticksNow = DoesTick();

		Object* parent = Parent;

		if (parent != nullptr && TickedBefore != ticksNow)
		{
			if (ticksNow)
				++parent->TickingChildren;
			else
				--parent->TickingChildren;

			parent->UpdateTickingState();
		}

		TickedBefore = ticksNow;
	}

	bool Object::MetaMatches(const Meta::ReflectedType* type, const Meta::ReflectedType* target, bool inherited)
	{
		return type == target || (inherited && type->InheritsType(target));
	}

	const std::weak_ptr<Object>& Object::GetHandle(int id)
	{
		return ObjectIDs.GetNode(id).GetData().SmartPointer;
	}
}
