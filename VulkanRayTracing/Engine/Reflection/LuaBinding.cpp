#include "LuaBinding.h"

#include "MetaData.h"
#include "BoundObject.h"

const bool TestFunctions = true;

namespace Engine
{
	namespace Lua
	{
		const char* GetTypeName(const Meta::ReflectedType* type, const char* nullName)
		{
			if (type == nullptr) return nullName;

			return type->Name;
		}

		template <typename T>
		const Meta::ReflectedType* GetMeta()
		{
			return Meta::Reflected<T>::GetMeta();
		}

		const Meta::ReflectedType* GetMeta(lua_State* lua, int index)
		{
			LuaUserdataType type = GetUserdataType(lua, index);

			if (type == LuaUserdataType::Value)
			{
				BoundObject& bound = BoundObject::Get(lua, index);

				return bound.Type != nullptr ? bound.Type : GetMeta<void>();
			} // TODO: add meta/scope type

			return GetMeta<void>();
		}

		const Meta::ReflectedType* GetArgumentType(lua_State* lua, int index)
		{
			if (lua_gettop(lua) < index) return GetMeta<void>();

			int type = lua_type(lua, index);

			switch (type)
			{
			case LUA_TNIL: return GetMeta<void>();
			case LUA_TNUMBER:
				if (lua_isinteger(lua, index))
					return GetMeta<lua_Integer>();
				else
					return GetMeta<lua_Number>();
			case LUA_TBOOLEAN: return GetMeta<bool>();
			case LUA_TSTRING: return GetMeta<std::string>();
			case LUA_TTABLE: return GetMeta<void>();
			case LUA_TFUNCTION: return GetMeta<void>();
			case LUA_TUSERDATA: return GetMeta(lua, index);
			case LUA_TTHREAD: return GetMeta<void>();
			case LUA_TLIGHTUSERDATA: return GetMeta<void>();
			default: return GetMeta<void>();
			}

			return GetMeta<void>();
		}

		void LuaBinding::BoundMember::Validate() const
		{
			if (Getter == nullptr || Setter == nullptr)
				throw "missing callback";

			if (TestFunctions)
			{
				Getter(nullptr);
				Setter(nullptr);
			}
		}

		void LuaBinding::BoundFunction::Validate() const
		{
			if (Callback == nullptr)
				throw "missing callback";

			if (TestFunctions)
				Callback(nullptr);
		}

		void LuaBinding::BoundEvent::Validate() const
		{
			if (Connect == nullptr || Disconnect == nullptr || Wait == nullptr)
				throw "missing callback";

			if (TestFunctions)
			{
				Connect(nullptr);
				Disconnect(nullptr);
				Wait(nullptr);
			}
		}

		void LuaBinding::BoundObject::Validate() const
		{
			if (Create == nullptr)
				throw "missing callback";

			if (TestFunctions)
				Create(nullptr);
		}

		LuaFunction GetOperatorOverload(const Meta::ReflectedType& type, const std::string& name)
		{
			const Meta::Function* function = type.GetFunction(name);

			if (function != nullptr)
				return function->Binding.Callback;

			return nullptr;
		}

		void LuaBinding::BoundObject::CacheOperators(Meta::ReflectedType& type)
		{
			type.Binding.ToString = GetOperatorOverload(type, "operator string");
			type.Binding.Call = GetOperatorOverload(type, "operator()");
			type.Binding.Length = GetOperatorOverload(type, "Size");
			type.Binding.UnaryMinus = GetOperatorOverload(type, "operator-");
			type.Binding.Add = GetOperatorOverload(type, "operator+");
			type.Binding.Subtract = GetOperatorOverload(type, "operator-");
			type.Binding.Multiply = GetOperatorOverload(type, "operator*");
			type.Binding.Divide = GetOperatorOverload(type, "operator/");
			type.Binding.FloorDivide = GetOperatorOverload(type, "FloorDivide");
			type.Binding.Modulus = GetOperatorOverload(type, "operator%");
			type.Binding.Power = GetOperatorOverload(type, "Pow");
			type.Binding.Concatenate = GetOperatorOverload(type, "Concat");
			type.Binding.BitwiseAnd = GetOperatorOverload(type, "operator&");
			type.Binding.BitwiseOr = GetOperatorOverload(type, "operator|");
			type.Binding.BitwiseXor = GetOperatorOverload(type, "operator^");
			type.Binding.BitwiseNot = GetOperatorOverload(type, "operator~");
			type.Binding.BitwiseShiftLeft = GetOperatorOverload(type, "operator<<");
			type.Binding.BitwiseShiftRight = GetOperatorOverload(type, "operator>>");
			type.Binding.BitwiseEquals = GetOperatorOverload(type, "operator==");
			type.Binding.LessThan = GetOperatorOverload(type, "operator<");
			type.Binding.LessThanEqualTo = GetOperatorOverload(type, "operator<=");
		}

		lua_State* LuaBinder::GetGlobalLuaState()
		{
			return nullptr;
		}

		int LuaBinder::RegisterConnectedFunction(lua_State* lua)
		{
			return -1;
		}
	}
}