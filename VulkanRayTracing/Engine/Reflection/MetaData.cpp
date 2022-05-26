#include "MetaData.h"

import <iostream>;

namespace Engine
{
	namespace Meta
	{
		bool ReflectedType::CanAllow(const ReflectedType* type) const
		{
			if (type->Inherits.size() >= Inherits.size() && type->Inherits[Inherits.size() - 1] == this)
				return true;

			for (int i = 0; i < (int)AllowedTypes.size(); ++i)
				if (AllowedTypes[i] == type)
					return true;

			return false;
		}

		bool ReflectedType::InheritsType(const ReflectedType* type) const
		{
			return Inherits.size() >= type->Inherits.size() && Inherits[type->Inherits.size() - 1] == type;
		}

		std::pair<int, MemberType> ReflectedType::GetRegisteredMember(const std::string& name) const
		{
			MemberDictionary::const_iterator key = RegisteredMembers.find(name);

			if (key != RegisteredMembers.end())
				return key->second;

			return std::make_pair(-1, MemberType::NotFound);
		}

		const Member* ReflectedType::GetMember(const std::string& name) const
		{
			Dictionary::const_iterator key = MemberKeys.find(name);

			if (key != MemberKeys.end())
				return &Members[key->second];

			return nullptr;
		}

		const Property* ReflectedType::GetProperty(const std::string& name) const
		{
			Dictionary::const_iterator key = PropertyKeys.find(name);

			if (key != PropertyKeys.end())
				return &Properties[key->second];

			return nullptr;
		}

		const Event* ReflectedType::GetEvent(const std::string& name) const
		{
			Dictionary::const_iterator key = EventKeys.find(name);

			if (key != EventKeys.end())
				return &Events[key->second];

			return nullptr;
		}

		const Function* ReflectedType::GetFunction(const std::string& name) const
		{
			Dictionary::const_iterator key = FunctionKeys.find(name);

			if (key != FunctionKeys.end())
				return &Functions[key->second];

			return nullptr;
		}

		const EnumItem* ReflectedType::GetEnumItem(int value) const
		{
			if (value < int(EnumItems.size()) && value >= 0)
				return &EnumItems[value];

			EnumDictionary::const_iterator item = UnorderedEnumItems.find(value);

			if (item != UnorderedEnumItems.end())
				return &item->second;

			return nullptr;
		}

		const EnumItem* ReflectedType::GetEnumItem(const std::string& name) const
		{
			Dictionary::const_iterator item = EnumKeys.find(name);

			if (item != EnumKeys.end())
				return GetEnumItem(item->second);

			return nullptr;
		}

		void ReflectedType::AddMember(const Member& member)
		{
			MemberKeys[member.Name] = int(Members.size());
			RegisteredMembers[member.Name] = std::make_pair(int(Members.size()), MemberType::Member);
			Members.push_back(member);
		}

		void ReflectedType::AddMember(const Property& member)
		{
			PropertyKeys[member.Name] = int(Properties.size());
			RegisteredMembers[member.Name] = std::make_pair(int(Properties.size()), MemberType::Property);
			Properties.push_back(member);
		}

		void ReflectedType::AddMember(const Event& member)
		{
			EventKeys[member.Name] = int(Events.size());
			RegisteredMembers[member.Name] = std::make_pair(int(Events.size()), MemberType::Event);
			Events.push_back(member);
		}

		void ReflectedType::AddMember(const Function& member)
		{
			if (member.IsConstructor)
			{
				Constructor = member;

				return;
			}

			FunctionKeys[member.Name] = int(Functions.size());
			RegisteredMembers[member.Name] = std::make_pair(int(Functions.size()), MemberType::Function);
			Functions.push_back(member);
		}

		void ReflectedType::AddValue(const EnumItem& item)
		{
			EnumKeys[item.Name] = item.Value;

			if (item.Value == int(EnumItems.size()))
			{
				EnumItems.push_back(item);

				return;
			}

			UnorderedEnumItems[item.Value] = item;
		}

		template <typename Container>
		void CopyData(const ReflectedType* meta, ReflectedType::Dictionary& keys, Container& members, const ReflectedType::Dictionary& otherKeys, const Container& otherMembers, ReflectedType::MemberDictionary& registeredMembers, MemberType memberType)
		{
			for (ReflectedType::Dictionary::const_iterator i = otherKeys.begin(); i != otherKeys.end(); ++i)
			{
				keys[i->first] = int(members.size());

				auto found = registeredMembers.find(i->first);

				if (found == registeredMembers.end())
					registeredMembers[i->first] = std::make_pair(int(members.size()), memberType);
				else
					std::cout << "warning: conflicting members '" << i->first << "' found while initializing inherited data in '" << meta->Name << "'" << std::endl;

				members.push_back(otherMembers[i->second]);

				members.back().InheritedBy = meta;
			}
		}

		void ReflectedType::CopyMembers(const ReflectedType* parent)
		{
			Inherits = parent->Inherits;

			Inherits.push_back(this);

			int allowed = (int)AllowedTypes.size();

			for (int i = 0; i < (int)parent->AllowedTypes.size(); ++i)
			{
				bool found = false;

				for (int j = 0; j < allowed; ++j)
					if (AllowedTypes[j] == parent->AllowedTypes[i])
						found = true;

				if (!found)
					AllowedTypes.push_back(parent->AllowedTypes[i]);
			}

			CopyData(this, MemberKeys, Members, parent->MemberKeys, parent->Members, RegisteredMembers, MemberType::Member);
			CopyData(this, PropertyKeys, Properties, parent->PropertyKeys, parent->Properties, RegisteredMembers, MemberType::Property);
			CopyData(this, EventKeys, Events, parent->EventKeys, parent->Events, RegisteredMembers, MemberType::Event);
			CopyData(this, FunctionKeys, Functions, parent->FunctionKeys, parent->Functions, RegisteredMembers, MemberType::Function);
		}

		const ReflectedType* ReflectedTypes::GetMeta(const std::string& name) const
		{
			Dictionary::const_iterator meta = MetaKeys.find(name);

			if (meta != MetaKeys.end())
				return Reflected[meta->second];

			return nullptr;
		}

		const ReflectedTypes* ReflectedTypes::GetScope(const std::string& name) const
		{
			Dictionary::const_iterator child = ScopeKeys.find(name);

			if (child != ScopeKeys.end())
				return ChildScopes[child->second];

			return nullptr;
		}

		ReflectedTypes* ReflectedTypes::GetScope(const std::string& name)
		{
			Dictionary::iterator child = ScopeKeys.find(name);

			if (child != ScopeKeys.end())
				return ChildScopes[child->second];

			return nullptr;
		}

		int ReflectedTypes::GetCount() const
		{
			return int(Reflected.size());
		}
		int ReflectedTypes::GetIndexFromName(const std::string& name) const
		{
			Dictionary::const_iterator child = MetaKeys.find(name);

			if (child != MetaKeys.end())
				return child->second;

			return -1;
		}

		const ReflectedType* ReflectedTypes::Get(int index) const
		{
			if (index >= 0 && index < int(Reflected.size()))
				return Reflected[index];

			return nullptr;
		}

		int ReflectedTypes::GetScopeCount() const
		{
			return int(ChildScopes.size());
		}

		int ReflectedTypes::GetScopeIndexFromName(const std::string& name) const
		{
			Dictionary::const_iterator child = ScopeKeys.find(name);

			if (child != ScopeKeys.end())
				return child->second;

			return -1;
		}

		const ReflectedTypes* ReflectedTypes::GetScope(int index) const
		{
			if (index >= 0 && index < int(ChildScopes.size()))
				return ChildScopes[index];

			return nullptr;
		}

		const ReflectedTypes* ReflectedTypes::GetGlobalScope()
		{
			return &ReflectionGlobalScope::GlobalScope;
		}

		void ReflectedTypes::RegisterType(const ReflectedType* meta)
		{
			ReflectedTypes* currentNamespace = &ReflectionGlobalScope::GlobalScope;

			if (currentNamespace->Name == "")
				currentNamespace->Name = "Global";

			for (const std::string& name : meta->Namespaces)
			{
				ReflectedTypes* next = currentNamespace->GetScope(name);

				if (next == nullptr)
				{
					next = new ReflectedTypes();

					next->Name = name;
					currentNamespace->ScopeKeys[name] = int(currentNamespace->ChildScopes.size());
					currentNamespace->ChildScopes.push_back(next);
				}

				currentNamespace = next;
			}

			for (int i = 0; i < (int)meta->Aliases.size(); ++i)
				currentNamespace->MetaKeys[meta->Aliases[i]] = int(currentNamespace->Reflected.size());

			currentNamespace->MetaKeys[meta->Name] = int(currentNamespace->Reflected.size());
			currentNamespace->Reflected.push_back(meta);
		}
	}
}