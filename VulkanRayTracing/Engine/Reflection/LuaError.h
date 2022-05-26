#pragma once

#include <lua.hpp>

template <typename... Arguments>
int LuaError(lua_State* lua, const char* format, Arguments... arguments)
{
	return luaL_error(lua, format, arguments...);
}