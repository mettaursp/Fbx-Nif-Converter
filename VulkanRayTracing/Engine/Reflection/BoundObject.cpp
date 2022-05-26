#include "BoundObject.h"

#include <lua.hpp>

namespace Engine
{
	namespace Lua
	{
		BoundObject& BoundObject::MakeBinding(lua_State* lua, bool isEnum, bool isItem)
		{
			BoundObject& bound = *reinterpret_cast<BoundObject*>(lua_newuserdata(lua, sizeof(BoundObject)));

			new (&bound) BoundObject();

			luaL_getmetatable(lua, !isEnum ? Metatable : (isItem ? EnumItemMetatable : EnumTypeMetatable));
			lua_setmetatable(lua, -2);

			return bound;
		}

		BoundObject& BoundObject::Get(lua_State* lua, int index)
		{
			return *reinterpret_cast<BoundObject*>(lua_touserdata(lua, index));
		}

		BoundScope& BoundScope::MakeBinding(lua_State* lua)
		{
			BoundScope& bound = *reinterpret_cast<BoundScope*>(lua_newuserdata(lua, sizeof(BoundScope)));

			new (&bound) BoundScope();

			luaL_getmetatable(lua, Metatable);
			lua_setmetatable(lua, -2);

			return bound;
		}

		BoundScope& BoundScope::Get(lua_State* lua, int index)
		{
			return *reinterpret_cast<BoundScope*>(lua_touserdata(lua, index));
		}

		LuaUserdataType GetUserdataType(lua_State* lua, int index)
		{
			return *reinterpret_cast<LuaUserdataType*>(lua_touserdata(lua, index));
		}
	}
}