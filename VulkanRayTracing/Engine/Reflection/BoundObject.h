#pragma once

import <memory>;
#include "../IdentifierHeap.h"

struct lua_State;

namespace Engine
{
	class Object;

	namespace Meta
	{
		struct ReflectedType;
		struct ReflectedTypes;
	}

	namespace Lua
	{
		struct LuaObjectTypeEnum
		{
			enum LuaObjectType
			{
				Object,
				Data,
				Enum,
				EnumType,

				Unknown
			};
		};

		struct LuaUserdataTypeEnum
		{
			enum LuaUserdataType
			{
				Value,
				Scope,

				Unknown
			};
		};

		typedef LuaObjectTypeEnum::LuaObjectType LuaObjectType;
		typedef LuaUserdataTypeEnum::LuaUserdataType LuaUserdataType;

		class BoundObject
		{
		public:
			typedef void (*DestructorCallback)(BoundObject& bound);

			const LuaUserdataType BoundType = LuaUserdataType::Value;
			std::shared_ptr<Object> GameObject;
			const Meta::ReflectedType* Type = nullptr;
			int DataId = -1;
			LuaObjectType ObjectType = LuaObjectType::Unknown;

			static BoundObject& MakeBinding(lua_State* lua, bool isEnum = false, bool isItem = false);
			static BoundObject& Get(lua_State* lua, int index = 1);

			static inline const char* const Metatable = "$BOMT"; // small string for fast look up
			static inline const char* const EnumTypeMetatable = "$BETMT"; // small string for fast look up
			static inline const char* const EnumItemMetatable = "$BEIMT"; // small string for fast look up
		};

		class BoundScope
		{
		public:
			const LuaUserdataType BoundType = LuaUserdataType::Scope;
			const Meta::ReflectedType* Type = nullptr;
			const Meta::ReflectedTypes* Scope = nullptr;

			static BoundScope& MakeBinding(lua_State* lua);
			static BoundScope& Get(lua_State* lua, int index = 1);

			static inline const char* const Metatable = "$BSMT"; // small string for fast look up
		};

		LuaUserdataType GetUserdataType(lua_State* lua, int index = 1);
	}
}