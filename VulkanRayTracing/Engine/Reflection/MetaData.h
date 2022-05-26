#pragma once

import <unordered_map>;
import <vector>;
import <string>;
import <utility>;

//#include "LuaBinding.h"

namespace Engine
{
	namespace Meta
	{
		struct MemberTypeEnum
		{
			enum MemberType
			{
				Member,
				Property,
				Function,
				Event,

				NotFound
			};
		};

		typedef MemberTypeEnum::MemberType MemberType;

		struct ReflectedType;

		struct Member
		{
			const char* Name = nullptr;
			bool IsStatic = false;
			bool IsConst = false;
			const ReflectedType* Type = nullptr;
			const ReflectedType* Parent = nullptr;
			const ReflectedType* InheritedBy = nullptr;
			//ActiveBinding::BoundMember Binding;
		};

		struct Property;

		struct PropertyFunction
		{
			const ReflectedType* Type = nullptr;
			//ActiveBinding::BoundFunction Binding;
		};

		struct Property
		{
			typedef std::vector<PropertyFunction> FunctionVector;

			const char* Name = nullptr;
			bool IsStatic = false;
			PropertyFunction Getter;
			FunctionVector Setters;
			const ReflectedType* Parent = nullptr;
			const ReflectedType* InheritedBy = nullptr;
			//ActiveBinding::BoundMember Binding;
		};

		struct FunctionOverload;

		struct Argument
		{
			const char* Name = nullptr;
			const char* DefaultValue = nullptr;
			const ReflectedType* Type = nullptr;
		};

		struct Function;

		struct FunctionOverload
		{
			typedef std::vector<Argument> ParameterVector;

			bool IsStatic = false;
			bool IsConst = false;
			const ReflectedType* ReturnType = nullptr;
			ParameterVector Parameters;
		};

		struct Function
		{
			typedef std::vector<FunctionOverload> OverloadVector;

			const char* Name = nullptr;
			bool IsConstructor = false;
			OverloadVector Overloads;
			const ReflectedType* Parent = nullptr;
			const ReflectedType* InheritedBy = nullptr;
			//ActiveBinding::BoundFunction Binding;
		};

		struct Event
		{
			typedef std::vector<Argument> ParameterVector;

			const char* Name = nullptr;
			bool IsStatic = false;
			ParameterVector Parameters;
			const ReflectedType* Parent = nullptr;
			const ReflectedType* InheritedBy = nullptr;
			//ActiveBinding::BoundEvent Binding;
		};

		struct EnumItem
		{
			const char* Name = nullptr;
			int Value = 0;
			const ReflectedType* Parent = nullptr;
		};

		struct ReflectedType
		{
			typedef std::unordered_map<std::string, int> Dictionary;
			typedef std::vector<Member> MemberVector;
			typedef std::vector<Property> PropertyVector;
			typedef std::vector<Function> FunctionVector;
			typedef std::vector<Event> EventVector;
			typedef std::vector<std::string> StringVector;
			typedef std::vector<EnumItem> EnumVector;
			typedef std::unordered_map<int, EnumItem> EnumDictionary;
			typedef std::vector<const ReflectedType*> MetaVector;
			typedef std::unordered_map<std::string, std::pair<int, MemberType>> MemberDictionary;

			bool IsComponent = false;
			bool IsFundamental = false;
			bool IsEnum = false;
			const char* Name = nullptr;
			StringVector Namespaces;
			StringVector Aliases;
			Function Constructor;
			const ReflectedType* Parent = nullptr;
			MemberVector Members;
			Dictionary MemberKeys;
			PropertyVector Properties;
			Dictionary PropertyKeys;
			EventVector Events;
			Dictionary EventKeys;
			FunctionVector Functions;
			Dictionary FunctionKeys;
			Dictionary EnumKeys;
			EnumVector EnumItems;
			EnumDictionary UnorderedEnumItems;
			//ActiveBinding::BoundObject Binding;
			MetaVector AllowedTypes;
			MetaVector Inherits;
			MemberDictionary RegisteredMembers;

			bool CanAllow(const ReflectedType* type) const;
			bool InheritsType(const ReflectedType* type) const;

			std::pair<int, MemberType> GetRegisteredMember(const std::string& name) const;
			const Member* GetMember(const std::string& name) const;
			const Property* GetProperty(const std::string& name) const;
			const Event* GetEvent(const std::string& name) const;
			const Function* GetFunction(const std::string& name) const;
			const EnumItem* GetEnumItem(int value) const;
			const EnumItem* GetEnumItem(const std::string& name) const;
			void AddMember(const Member& member);
			void AddMember(const Property& member);
			void AddMember(const Event& member);
			void AddMember(const Function& member);
			void AddValue(const EnumItem& item);
			void CopyMembers(const ReflectedType* parent);
		};

		struct ReflectedTypes
		{
			const ReflectedType* GetMeta(const std::string& name) const;
			const ReflectedTypes* GetScope(const std::string& name) const;
			ReflectedTypes* GetScope(const std::string& name);
			int GetCount() const;
			int GetIndexFromName(const std::string& name) const;
			const ReflectedType* Get(int index) const;
			int GetScopeCount() const;
			int GetScopeIndexFromName(const std::string& name) const;
			const ReflectedTypes* GetScope(int index) const;
			const std::string& GetName() const { return Name; }

			static void RegisterType(const ReflectedType* meta);
			static const ReflectedTypes* GetGlobalScope();

		private:
			typedef std::unordered_map<std::string, int> Dictionary;
			typedef std::vector<ReflectedTypes*> ScopeVector;
			typedef std::vector<const ReflectedType*> MetaVector;

			std::string Name;
			Dictionary MetaKeys;
			MetaVector Reflected;
			Dictionary ScopeKeys;
			ScopeVector ChildScopes;
		};

		struct ReflectionGlobalScope
		{
			static inline ReflectedTypes GlobalScope;
		};
	}
}