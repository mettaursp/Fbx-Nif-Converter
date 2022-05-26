#include "ObjectMetatable.h"

import <iostream>;

#include "MetaData.h"
#include "LuaError.h"

namespace Engine
{
	namespace Lua
	{
		const luaL_Reg ObjectMetatable::Metatable[25] = {
			{ "__index",	__index },
			{ "__newindex",	__newindex },
			{ "__tostring",	__tostring },
			{ "__call",		__call },
			{ "__len",		__len },
			{ "__gc",		__gc },
			{ "__unm",		__unaryMinus },
			{ "__add",		__add },
			{ "__sub",		__subtract },
			{ "__mul",		__multiply },
			{ "__div",		__divide },
			{ "__idiv",		__floorDivide },
			{ "__mod",		__modulus },
			{ "__pow",		__power },
			{ "__concat",	__concatenate },
			{ "__band",		__bitwiseAnd },
			{ "__bor",		__bitwiseOr },
			{ "__bxor",		__bitwiseXOr },
			{ "__bnot",		__bitwiseNot },
			{ "__bshl",		__bitwiseShiftLeft },
			{ "__bshr",		__bitwiseShiftRight },
			{ "__eq",		__equals },
			{ "__lt",		__lessThan },
			{ "__le",		__lessThanEqualTo },
			{0, 0}
		};

		int ObjectMetatable::__index(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (!lua_isstring(lua, 2))
				return LuaError(lua, "attempt to fetch field on %s '%s' with invalid key", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);

			const char* index = lua_tostring(lua, 2);
			{
				std::string indexString(index);

				auto member = bound.Type->GetRegisteredMember(indexString);

				switch (member.second)
				{
				case Meta::MemberType::Member:
					return bound.Type->Members[member.first].Binding.Getter(lua);
				case Meta::MemberType::Property:
					return bound.Type->Properties[member.first].Binding.Getter(lua);
				case Meta::MemberType::Function:
					lua_pushcfunction(lua, bound.Type->Functions[member.first].Binding.Callback);

					return 1;
				case Meta::MemberType::Event:
					// TODO: push event

					return 0;
				case Meta::MemberType::NotFound:
					if (bound.ObjectType == LuaObjectType::Object)
					{
						std::shared_ptr<Object> child = bound.GameObject->GetByName(indexString);

						if (child != nullptr)
						{
							BindType<std::shared_ptr<Object>>::Push(lua, child);

							return 1;
						}
					}
				}
			}

			return LuaError(lua, "failed to find member/child '%s' on %s '%s'", index, bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__newindex(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (!lua_isstring(lua, 2))
				return LuaError(lua, "attempt to write field on %s '%s' with invalid key", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);

			LuaFunction callback = nullptr;

			const char* index = lua_tostring(lua, 2);
			{
				std::string indexString(index);

				auto member = bound.Type->GetRegisteredMember(indexString);
				

				switch (member.second)
				{
				case Meta::MemberType::Member:
					callback = bound.Type->Members[member.first].Binding.Setter; break;
				case Meta::MemberType::Property:
					callback = bound.Type->Properties[member.first].Binding.Setter;

					lua_remove(lua, 2);
				}
			}

			if (callback != nullptr)
				return callback(lua);

			return LuaError(lua, "attempt to set invalid field '%s' on %s '%s'", index, bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__tostring(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.ToString != nullptr)
				return bound.Type->Binding.ToString(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "tostring", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__call(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Call != nullptr)
				return bound.Type->Binding.Call(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "call", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__len(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Length != nullptr)
				return bound.Type->Binding.Length(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "length", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__gc(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Free != nullptr)
				bound.Type->Binding.Free(bound);

			bound.~BoundObject();

			return 0;
		}

		int ObjectMetatable::__unaryMinus(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.UnaryMinus != nullptr)
				return bound.Type->Binding.UnaryMinus(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "unary minus", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__add(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Add != nullptr)
				return bound.Type->Binding.Add(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "add", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__subtract(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Subtract != nullptr)
				return bound.Type->Binding.Subtract(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "subtract", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__multiply(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Multiply != nullptr)
				return bound.Type->Binding.Multiply(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "multiply", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__divide(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Divide != nullptr)
				return bound.Type->Binding.Divide(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "divide", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__floorDivide(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.FloorDivide != nullptr)
				return bound.Type->Binding.FloorDivide(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "floor divide", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__modulus(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Modulus != nullptr)
				return bound.Type->Binding.Modulus(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "modulo", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__power(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Power != nullptr)
				return bound.Type->Binding.Power(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "pow", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__concatenate(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.Concatenate != nullptr)
				return bound.Type->Binding.Concatenate(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "concatenate", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseAnd(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseAnd != nullptr)
				return bound.Type->Binding.BitwiseAnd(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise and", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseOr(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseOr != nullptr)
				return bound.Type->Binding.BitwiseOr(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise or", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseXOr(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseXor != nullptr)
				return bound.Type->Binding.BitwiseXor(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise xor", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseNot(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseNot != nullptr)
				return bound.Type->Binding.BitwiseNot(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise not", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseShiftLeft(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseShiftLeft != nullptr)
				return bound.Type->Binding.BitwiseShiftLeft(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise left shift", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__bitwiseShiftRight(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseShiftRight != nullptr)
				return bound.Type->Binding.BitwiseShiftRight(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "bitwise right shift", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__equals(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.BitwiseEquals != nullptr)
				return bound.Type->Binding.BitwiseEquals(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "equals", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__lessThan(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.LessThan != nullptr)
				return bound.Type->Binding.LessThan(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "less than", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		int ObjectMetatable::__lessThanEqualTo(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			if (bound.Type->Binding.LessThanEqualTo != nullptr)
				return bound.Type->Binding.LessThanEqualTo(lua);

			return LuaError(lua, "attempt to use missing operator '%s' on %s '%s'", "less than or equal to", bound.ObjectType == LuaObjectType::Object ? "object" : "type", bound.Type->Name);
		}

		void ObjectMetatable::Initialize(lua_State* lua)
		{
			if (luaL_newmetatable(lua, BoundObject::Metatable))
			{
				int meta = lua_gettop(lua);

				luaL_setfuncs(lua, Metatable, 0);

				lua_pushstring(lua, "__metatable");
				lua_pushstring(lua, "This metatable is protected");

				lua_settable(lua, -3);
			}

			lua_pop(lua, 1);
		}

		const luaL_Reg BoundScopeMetatable::Metatable[5] = {
			{ "__index",	BoundScopeMetatable::__index },
			{ "__newindex",	BoundScopeMetatable::__newindex },
			{ "__tostring",	__tostring },
			{ "__call",		__call },
			{0, 0}
		};

		int BoundScopeMetatable::__index(lua_State* lua)
		{
			BoundScope& scope = BoundScope::Get(lua);

			const char* const scopeType = scope.Type != nullptr ? (scope.Type->IsComponent ? "object" : "type") : "scope";
			const char* const scopeName = scope.Type != nullptr ? scope.Type->Name : scope.Scope->GetName().c_str();

			if (!lua_isstring(lua, 2))
				return LuaError(lua, "attempt to fetch field on %s '%s' with invalid key", scopeType, scopeName);

			const char* index = lua_tostring(lua, 2);
			{
				std::string indexString(index);

				if (scope.Type != nullptr)
				{
					auto member = scope.Type->GetRegisteredMember(indexString);

					switch (member.second)
					{
					case Meta::MemberType::Member:
						return scope.Type->Members[member.first].Binding.Getter(lua);
					case Meta::MemberType::Property:
						return scope.Type->Properties[member.first].Binding.Getter(lua);
					case Meta::MemberType::Function:
						lua_pushcfunction(lua, scope.Type->Functions[member.first].Binding.Callback);

						return 1;
					case Meta::MemberType::Event:
						// TODO: push event

						return 0;
					}
				}
				else if (scope.Scope != nullptr)
				{
					const Meta::ReflectedType* type = scope.Scope->GetMeta(indexString);
					const Meta::ReflectedTypes* innerScope = scope.Scope->GetScope(indexString);

					if (type != nullptr || innerScope != nullptr)
					{
						if (type == nullptr || !type->IsEnum)
						{
							BoundScope& newScope = BoundScope::MakeBinding(lua);

							newScope.Type = type;
							newScope.Scope = innerScope;

							return 1;
						}
						else
						{
							BoundObject& bound = BoundObject::MakeBinding(lua, true);

							bound.Type = type;
							bound.ObjectType = LuaObjectType::EnumType;

							return 1;
						}
					}
				}
			}

			return LuaError(lua, "failed to find member/scope '%s' on %s '%s'", index, scopeType, scopeName);
		}

		int BoundScopeMetatable::__newindex(lua_State* lua)
		{
			BoundScope& scope = BoundScope::Get(lua);

			if (scope.Type == nullptr)
				return LuaError(lua, "attempt to set field on scope '%s'", scope.Scope->GetName().c_str());

			const char* const scopeType = scope.Type->IsComponent ? "object" : "type";

			if (!lua_isstring(lua, 2))
				return LuaError(lua, "attempt to write field on %s '%s' with invalid key", scopeType, scope.Type->Name);

			const char* index = lua_tostring(lua, 2);
			{
				std::string indexString(index);

				auto member = scope.Type->GetRegisteredMember(indexString);

				switch (member.second)
				{
				case Meta::MemberType::Member:
					return scope.Type->Members[member.first].Binding.Setter(lua);
				case Meta::MemberType::Property:
					return scope.Type->Properties[member.first].Binding.Setter(lua);
				}
			}

			return LuaError(lua, "attempt to set invalid field '%s' on %s '%s'", index, scopeType, scope.Type->Name);
		}

		int BoundScopeMetatable::__tostring(lua_State* lua)
		{
			BoundScope& scope = BoundScope::Get(lua);

			if (scope.Type != nullptr)
				lua_pushstring(lua, scope.Type->Name);
			else if (scope.Scope != nullptr)
				lua_pushstring(lua, scope.Scope->GetName().c_str());
			else
				lua_pushstring(lua, "BoundScope");

			return 1;
		}

		int BoundScopeMetatable::__call(lua_State* lua)
		{
			BoundScope& scope = BoundScope::Get(lua);

			if (scope.Type != nullptr)
			{
				if (scope.Type->Constructor.Binding.Callback != nullptr)
					return scope.Type->Constructor.Binding.Callback(lua);
				else
					return LuaError(lua, "attempt to construct %s '%s' with missing constructor", scope.Type->IsComponent == LuaObjectType::Object ? "object" : "type", scope.Type->Name);
			}

			return LuaError(lua, "attempt to construct non-type scope '%s'", scope.Scope->GetName().c_str());
		}

		int BoundScopeMetatable::__gc(lua_State* lua)
		{
			BoundScope& scope = BoundScope::Get(lua);

			scope.~BoundScope();

			return 0;
		}

		void BoundScopeMetatable::Initialize(lua_State* lua)
		{
			if (luaL_newmetatable(lua, BoundScope::Metatable))
			{
				int meta = lua_gettop(lua);

				luaL_setfuncs(lua, Metatable, 0);

				lua_pushstring(lua, "__metatable");
				lua_pushstring(lua, "This metatable is protected");

				lua_settable(lua, -3);
			}

			lua_pop(lua, 1);
		}

		const luaL_Reg EnumTypeMetatable::Metatable[4] = {
			{ "__index",	__index },
			{ "__tostring",	__tostring },
			{ "__gc",	__gc },
			{0, 0}
		};

		int EnumTypeMetatable::__index(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			int type = lua_type(lua, -1);

			if (type == LUA_TSTRING)
			{
				const char* index = lua_tostring(lua, -1);

				const Meta::EnumItem* item = bound.Type->GetEnumItem(index);

				if (item != nullptr)
				{
					BoundObject& newItem = BoundObject::MakeBinding(lua, true, true);

					newItem.Type = bound.Type;
					newItem.ObjectType = LuaObjectType::Enum;
					newItem.DataId = item->Value;

					return 1;
				}

				if (strcmp(index, "Name") == 0)
				{
					lua_pushstring(lua, bound.Type->Name);

					return 1;
				}

				auto callback = Callbacks.find(index);

				if (callback != Callbacks.end())
				{
					lua_pushcfunction(lua, callback->second);

					return 1;
				}
			}
			else if (type == LUA_TNUMBER)
			{
				int index = (int)lua_tointeger(lua, -1);

				const Meta::EnumItem* item = bound.Type->GetEnumItem(index);

				if (item != nullptr)
				{
					BoundObject& newItem = BoundObject::MakeBinding(lua, true, true);

					newItem.Type = bound.Type;
					newItem.ObjectType = LuaObjectType::Enum;
					newItem.DataId = item->Value;

					return 1;
				}
			}
			else
				return LuaError(lua, "attempt to use invalid index type on enum type '%s'", bound.Type->Name);

			return LuaError(lua, "attempt to use invalid index on enum type '%s'", bound.Type->Name);
		}

		int EnumTypeMetatable::__tostring(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			lua_pushfstring(lua, "Enum.%s", bound.Type->Name);

			return 1;
		}

		int EnumTypeMetatable::__gc(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			bound.~BoundObject();

			return 0;
		}

		void EnumTypeMetatable::Initialize(lua_State* lua)
		{
			if (luaL_newmetatable(lua, BoundObject::EnumTypeMetatable))
			{
				int meta = lua_gettop(lua);

				luaL_setfuncs(lua, Metatable, 0);

				lua_pushstring(lua, "__metatable");
				lua_pushstring(lua, "This metatable is protected");

				lua_settable(lua, -3);
			}

			lua_pop(lua, 1);
		}

		const luaL_Reg EnumItemMetatable::Metatable[4] = {
			{ "__index",	__index },
			{ "__tostring",	__tostring },
			{ "__gc",	__gc },
			{0, 0}
		};

		int EnumItemMetatable::__index(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			const Meta::EnumItem* item = bound.Type->GetEnumItem(bound.DataId);

			int type = lua_type(lua, -1);

			if (type == LUA_TSTRING)
			{
				const char* index = lua_tostring(lua, -1);

				if (strcmp(index, "Name") == 0)
				{
					if (item != nullptr)
						lua_pushstring(lua, bound.Type->Name);
					else
						lua_pushfstring(lua, "<InvalidItem:%d>", bound.DataId);

					return 1;
				}

				if (strcmp(index, "Value") == 0)
				{
					lua_pushinteger(lua, bound.DataId);

					return 1;
				}

				if (strcmp(index, "Parent") == 0)
				{
					BoundObject& newEnum = BoundObject::MakeBinding(lua, true);

					newEnum.Type = bound.Type;
					newEnum.ObjectType = LuaObjectType::EnumType;

					return 1;
				}
			}
			else
			{
				if (item != nullptr)
					return LuaError(lua, "attempt to use invalid index type on enum item '%s.%s'", bound.Type->Name, item->Name);
				else
					return LuaError(lua, "attempt to use invalid index type on enum item '%s.<InvalidItem:%d>'", bound.Type->Name, bound.DataId);
			}

			if (item != nullptr)
				return LuaError(lua, "attempt to use invalid index on enum item '%s.%s'", bound.Type->Name, item->Name);

			return LuaError(lua, "attempt to use invalid index on enum item '%s.%<InvalidItem:%d>'", bound.Type->Name, bound.DataId);
		}

		int EnumItemMetatable::__tostring(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			const Meta::EnumItem* item = bound.Type->GetEnumItem(bound.DataId);

			if (item != nullptr)
				lua_pushfstring(lua, "Enum.%s.%s", bound.Type->Name, item->Name);
			else
				lua_pushfstring(lua, "Enum.%s.<InvalidItem:%d>", bound.Type->Name, bound.DataId);

			return 1;
		}

		int EnumItemMetatable::__gc(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			bound.~BoundObject();

			return 0;
		}

		void EnumItemMetatable::Initialize(lua_State* lua)
		{
			if (luaL_newmetatable(lua, BoundObject::EnumItemMetatable))
			{
				int meta = lua_gettop(lua);

				luaL_setfuncs(lua, Metatable, 0);

				lua_pushstring(lua, "__metatable");
				lua_pushstring(lua, "This metatable is protected");

				lua_settable(lua, -3);
			}

			lua_pop(lua, 1);
		}
	}
}
