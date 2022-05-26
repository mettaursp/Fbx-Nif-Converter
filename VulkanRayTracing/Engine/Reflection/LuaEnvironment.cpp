#include "LuaEnvironment.h"

#include <lua.hpp>
import <iostream>;

#include "ObjectMetatable.h"
#include "MetaData.h"
#include "../Engine.h"
#include "../GameEngine.h"
#include "../Graphics.h"
#include "LuaThreadManager.h"
#include "../LuaJson.h"

namespace Engine
{
	namespace Lua
	{
		int getUserdataType(lua_State* lua)
		{
			LuaUserdataType type = GetUserdataType(lua, 1);

			if (type == LuaUserdataType::Value)
			{
				BoundObject& bound = BoundObject::Get(lua);

				if (bound.ObjectType == LuaObjectType::Enum)
				{
					const Meta::EnumItem* item = bound.Type->GetEnumItem(bound.DataId);

					if (item != nullptr)
						lua_pushfstring(lua, "Enum.%s.%s", bound.Type->Name, item->Name);
					else
						lua_pushfstring(lua, "Enum.%s.<InvalidItem:%d>", bound.Type->Name, bound.DataId);
				}
				else if (bound.ObjectType == LuaObjectType::EnumType)
					lua_pushfstring(lua, "Enum.%s", bound.Type->Name);
				else if (bound.ObjectType == LuaObjectType::Unknown)
					lua_pushstring(lua, "userdata");
				else
					lua_pushstring(lua, bound.Type->Name);
			}
			else if (type == LuaUserdataType::Scope)
			{
				BoundScope& scope = BoundScope::Get(lua);

				lua_pushfstring(lua, "scope:%s", scope.Type != nullptr ? scope.Type->Name : scope.Scope->GetName().c_str());
			}
			else
				lua_pushstring(lua, "userdata");

			return 1;
		}



		int typeof(lua_State* lua)
		{
			if (lua_gettop(lua) == 0)
			{
				lua_pushstring(lua, "nil");

				return 1;
			}

			int type = lua_type(lua, 1);

			const char* name = nullptr;

			switch (type)
			{
			case LUA_TNIL: name = "nil"; break;
			case LUA_TNUMBER: name = "number"; break;
			case LUA_TBOOLEAN: name = "bool"; break;
			case LUA_TSTRING: name = "string"; break;
			case LUA_TTABLE: name = "table"; break;
			case LUA_TFUNCTION: name = "function"; break;
			case LUA_TUSERDATA: return getUserdataType(lua);
			case LUA_TTHREAD: name = "thread"; break;
			case LUA_TLIGHTUSERDATA: name = "lightuserdata"; break;
			default: name = "unknown"; break;
			}

			lua_pushstring(lua, name);

			return 1;
		}

		LuaEnvironment::LuaEnvironment()
		{
			State = luaL_newstate();

			InitializeCorePackages(State);

			ObjectMetatable::Initialize(State);
			BoundScopeMetatable::Initialize(State);
			EnumTypeMetatable::Initialize(State);
			EnumItemMetatable::Initialize(State);

			InitializeGlobalNamespaces(State);
			InitializeGlobalFunctions(State);
			InitializeDebugLibrary(State);
			InitializeWaitFunction(State);
			InitializeTimerFunction(State);
			InitializeDelayFunction(State);

			Lua::Initialize(State);
		}

		LuaEnvironment::~LuaEnvironment()
		{
			lua_close(State);
		}

		void LuaEnvironment::InitializeCorePackages(lua_State* lua)
		{
			luaL_requiref(State, "_G", luaopen_base, 1);
			luaL_requiref(State, LUA_LOADLIBNAME, luaopen_package, 1);
			luaL_requiref(State, LUA_COLIBNAME, luaopen_coroutine, 1);
			luaL_requiref(State, LUA_TABLIBNAME, luaopen_table, 1);
			luaL_requiref(State, LUA_IOLIBNAME, luaopen_io, 1);
			luaL_requiref(State, LUA_OSLIBNAME, luaopen_os, 1);
			luaL_requiref(State, LUA_STRLIBNAME, luaopen_string, 1);
			luaL_requiref(State, LUA_MATHLIBNAME, luaopen_math, 1);
			luaL_requiref(State, LUA_UTF8LIBNAME, luaopen_utf8, 1);

			int addedToStack = 9;

#if defined(LUA_COMPAT_BITLIB)
			luaL_requiref(lua, LUA_BITLIBNAME, luaopen_bit32, 1);
			++addedToStack;
#endif

			lua_pop(State, addedToStack);
		}

		void LuaEnvironment::InitializeGlobalNamespaces(lua_State* lua)
		{
			const Meta::ReflectedTypes* global = Meta::ReflectedTypes::GetGlobalScope();

			std::vector<bool> pushedScopes(global->GetScopeCount());

			for (int i = 0; i < global->GetCount(); ++i)
			{
				const Meta::ReflectedType* type = global->Get(i);
				int scopeIndex = global->GetScopeIndexFromName(type->Name);
				const Meta::ReflectedTypes* scope = global->GetScope(scopeIndex);

				if (scopeIndex != -1)
					pushedScopes[scopeIndex] = true;

				BoundScope& bound = BoundScope::MakeBinding(lua);

				bound.Type = type;
				bound.Scope = scope;

				lua_setglobal(lua, type->Name);

				for (int j = 0; j < (int)type->Aliases.size(); ++j)
				{
					int scopeIndex = global->GetScopeIndexFromName(type->Aliases[j]);
					const Meta::ReflectedTypes* scope = global->GetScope(scopeIndex);

					if (scopeIndex != -1)
						pushedScopes[scopeIndex] = true;

					BoundScope& bound = BoundScope::MakeBinding(lua);

					bound.Type = type;
					bound.Scope = scope;

					lua_setglobal(lua, type->Aliases[j].c_str());
				}
			}

			for (int i = 0; i < global->GetScopeCount(); ++i)
			{
				if (!pushedScopes[i])
				{
					const Meta::ReflectedTypes* scope = global->GetScope(i);

					BoundScope& bound = BoundScope::MakeBinding(lua);

					bound.Scope = scope;

					lua_setglobal(lua, scope->GetName().c_str());
				}
			}
		}

		void LuaEnvironment::InitializeDebugLibrary(lua_State* lua)
		{
			lua_createtable(lua, 0, 1);
			lua_pushstring(lua, "traceback");
			lua_pushcfunction(lua,
				[](lua_State* lua) -> int
				{
					luaL_traceback(lua, lua, 0, 0);

					return 1;
				}
			);
			lua_settable(lua, -3);
			lua_setglobal(lua, "debug");

			luaL_newlib(lua, LuaJsonLib);
			lua_setglobal(lua, "json");
		}

		void LuaEnvironment::InitializeGlobalFunctions(lua_State* lua)
		{
			lua_gc(lua, LUA_GCSETPAUSE, 100);
			lua_gc(lua, LUA_GCSETSTEPMUL, 100);

			lua_pushcfunction(lua, typeof);
			lua_setglobal(lua, "typeof");

			{
				BoundObject& root = BoundObject::MakeBinding(lua);

				root.ObjectType = LuaObjectType::Object;
				root.GameObject = Engine::Root();
				root.Type = root.GameObject->GetMetaData();

				lua_setglobal(lua, "Engine");
			}

			lua_pushcfunction(lua,
				[](lua_State* lua) -> int
				{
					lua_pushnumber(lua, (lua_Number)Graphics::ActiveWindow->GetTime());
					
					return 1;
				}
			);
			lua_setglobal(lua, "gameTime");

			lua_pushstring(lua, "functions");
			lua_createtable(lua, 10, 0);
			lua_settable(lua, LUA_REGISTRYINDEX);

			lua_pushstring(lua, "threads");
			lua_createtable(lua, 10, 0);
			lua_settable(lua, LUA_REGISTRYINDEX);

			//lua_pushcfunction(lua, ProtectedDoFile);
			//lua_setglobal(lua, "dofile");
		}

		void LuaEnvironment::InitializeWaitFunction(lua_State* lua)
		{

			{
				char waitImplementation[] = (
					"	return function(queueThread)"
					"\n		function wait(time)"
					"\n			time = time or 1/1000"
					"\n			if type(time) ~= 'number' then"
					"\n				error('Expected type \"number\" for argument 1 of \"wait\", got \"' .. type(callback) .. '\"\\\n')"
					"\n			end"
					"\n			"
					"\n			local start = queueThread(time)"
					"\n			"
					"\n			coroutine.yield()"
					"\n			"
					"\n			return gameTime() - start"
					"\n		end"
					"\n		"
					"\n		return wait"
					"\n	end"
				);

				luaL_loadbuffer(lua, waitImplementation, sizeof(waitImplementation) - 1, "wait");

				lua_call(lua, 0, 1);
				lua_pushcfunction(lua, LuaQueueThread);
				lua_call(lua, 1, 1);
				lua_setglobal(lua, "wait");
			}
		}

		struct LuaThreadMarker
		{
			int ID = -1;
			lua_State* State = nullptr;

			LuaThreadMarker(lua_State* lua, int id) : ID(id), State(lua) {}

			bool operator<(const LuaThreadMarker& other) const
			{
				return Lua::CompareThreads(State, other.State);
			}
		};

		typedef std::vector<LuaThreadMarker> LuaThreadQueue;
		typedef IDHeap<lua_State*> LuaThreadHeap;

		LuaThreadQueue resumedThreads;

		void LuaEnvironment::UpdateLua(lua_State* lua, float delta)
		{
			lua_gc(lua, LUA_GCCOLLECT, 0);

			while (resumedThreads.size() > 0)
			{
				lua_pushstring(lua, "threads");
				lua_gettable(lua, LUA_REGISTRYINDEX);

				int threads = lua_gettop(lua);

				lua_pushnumber(lua, lua_Number(resumedThreads[0].ID));
				lua_pushnil(lua);
				lua_settable(lua, threads);

				lua_pop(lua, 1);

				lua_State* thread = GetThread(resumedThreads[0].ID);

				Lua::CoroutineResume(thread);
				lua_resume(thread, lua, 0);

				std::pop_heap(resumedThreads.begin(), resumedThreads.end());

				resumedThreads.pop_back();
			}

			resumedThreads.clear();

			Engine::Lua::Update(lua, delta);
		}

		void LuaEnvironment::InitializeDelayFunction(lua_State* lua)
		{
			char debugImplementation[] = (
				"	return function(queueThread, debug)"
				"\n		function delay(time, callback, ...)"
				"\n			if type(callback) ~= 'function' then"
				"\n				error('Expected type \"function\" for argument 2 of \"delay\", got \"' .. type(callback) .. '\"')"
				"\n			elseif type(time) ~= 'number' then"
				"\n				error('Expected type \"number\" for argument 1 of \"delay\", got \"' .. type(time) .. '\"')"
				"\n			end"
				"\n			"
				"\n			--local sourceThread = coroutine.running()"
				"\n			"
				"\n			--local entryTrace = debug.traceback()"
				"\n			"
				"\n			coroutine.wrap(function(...)"
				"\n				wait(time)"
				"\n				"
				"\n				--[[if scriptAlive(sourceThread) then"
				"\n					return"
				"\n				end]]"
				"\n				"
				"\n				local success, result = pcall(callback, ...)"
				"\n				"
				"\n				if not success then"
				"\n					print(result)"
				"\n					--print(entryTrace)"
				"\n				end"
				"\n			end)(...)"
				"\n		end"
				"\n		"
				"\n		return delay"
				"\n	end"
			);

			luaL_loadbuffer(lua, debugImplementation, sizeof(debugImplementation) - 1, "wait");

			lua_call(lua, 0, 1);
			lua_pushcfunction(lua, LuaQueueThread);
			lua_getglobal(lua, "debug");
			lua_call(lua, 2, 1);
			lua_setglobal(lua, "delay");
		}

		void LuaEnvironment::InitializeTimerFunction(lua_State* lua)
		{
			char timerImplementation[] = (
				"	return function(queueThread, debug)"
				"\n		function timer(time, callback, ...)"
				"\n			if type(callback) ~= 'function' then"
				"\n				error('Expected type \"function\" for argument 2 of \"timer\", got \"' .. type(callback) .. '\"\\\n')"
				"\n			elseif type(time) ~= 'number' then"
				"\n				error('Expected type \"number\" for argument 1 of \"timer\", got \"' .. type(time) .. '\"')"
				"\n			end"
				"\n			"
				"\n			--local sourceThread = coroutine.running()"
				"\n			"
				"\n			--local entryTrace = debug.traceback()"
				"\n			"
				"\n			coroutine.wrap(function(...)"
				"\n				while true do"
				"\n					wait(time)"
				"\n					"
				"\n					--[[if coroutine.status(sourceThread) == 'dead' then"
				"\n						break"
				"\n					end]]"
				"\n					"
				"\n					local success, result = pcall(callback, ...)"
				"\n					"
				"\n					if not success then"
				"\n						print(result)"
				"\n						--print(entryTrace)"
				"\n					end"
				"\n					"
				"\n					if not success then"
				"\n						break"
				"\n					end"
				"\n				end"
				"\n			end)(...)"
				"\n		end"
				"\n		"
				"\n		return timer"
				"\n	end"
			);

			luaL_loadbuffer(lua, timerImplementation, sizeof(timerImplementation) - 1, "wait");

			lua_call(lua, 0, 1);
			lua_pushcfunction(lua, LuaQueueThread);
			lua_getglobal(lua, "debug");
			lua_call(lua, 2, 1);
			lua_setglobal(lua, "timer");
		}

		int LuaGameTime(lua_State* lua)
		{
			lua_pushnumber(lua, lua_Number(Graphics::ActiveWindow->GetTime()));

			return 1;
		}

		int LuaEnvironment::LuaQueueThread(lua_State* lua)
		{
			if (lua_isnumber(lua, 1))
			{
				Float delay = Float(lua_tonumber(lua, 1));

				int id = Lua::GetThreadID(lua);

				lua_pushstring(lua, "threads");
				lua_gettable(lua, LUA_REGISTRYINDEX);

				int threads = lua_gettop(lua);

				lua_pushnumber(lua, lua_Number(id));
				lua_pushthread(lua);
				lua_settable(lua, threads);

				TaskScheduler::Repeat(delay, 1, [lua, id](Float, Float, int)
					{
						if (Dead(id)) return;

						resumedThreads.push_back(LuaThreadMarker(lua, id));

						std::push_heap(resumedThreads.begin(), resumedThreads.end());
					});
			}
			else
				lua_pushfstring(lua, "bad argument #1 to internal QueueThread, expected 'string'");

			return LuaGameTime(lua);
		}

		bool LuaEnvironment::RunChunk(lua_State* lua, const char* source, const char* fileName, int lineNumber, int length)
		{
			for (int i = 0; fileName[i]; ++i)
			{
				if (fileName[i] == '\\' || fileName[i] == '/')
				{
					fileName += i + 1;
					i = -1;
				}
			}

			if (length == -1)
				for (length = 0; source[length]; ++length);

			const std::string chunkName = std::string("C++: ") + fileName + "[" + std::to_string(lineNumber) + "]";

			int error = luaL_loadbuffer(lua, source, length, chunkName.c_str());

			if (error)
			{
				if (lua_isstring(lua, -1))
					std::cout << lua_tostring(lua, -1);

				lua_pop(lua, 1);
			}

			return error == 0;
		}

		int LuaEnvironment::RunProtectedFunction(lua_State* lua)
		{
			// expect function at stack top

			lua_pushcfunction(lua, Lua::LuaEnvironment::Traceback); // put traceback on stack for pcall

			lua_pushvalue(lua, -2); // move function above traceback for pcall

			int error = lua_pcall(lua, 0, 0, -2); // run pcall on input with traceback

			if (error)
			{
				if (lua_isstring(lua, -1))
					std::cout << lua_tostring(lua, -1) << std::endl;

				lua_pop(lua, 1);
			}

			lua_pop(lua, 1); // remove pcall return value & traceback

			return 0;
		}

		int LuaEnvironment::RunProtectedChunk(lua_State* lua, const char* source, const char* fileName, int lineNumber, int length)
		{
			bool succeeded = RunChunk(lua, source, fileName, lineNumber, length);

			if (succeeded)
			{
				RunProtectedFunction(lua);

				lua_pop(lua, 1);
			}

			return 0;
		}

		int LuaEnvironment::ProtectedDoFile(lua_State* lua)
		{
			const char* filePath = nullptr;

			if (lua_isstring(lua, -1))
				filePath = lua_tostring(lua, -1);

			if (!filePath)
				return 0;

			lua_Debug ar;
			int ret1 = lua_getstack(lua, 1, &ar);
			int ret2 = lua_getinfo(lua, "nSl", &ar);

			//std::fstream file(filePath, std::ios_base::in);
			//
			//if (!file.good() || !file.is_open())
			//	return 0;
			//
			//char buffer[0xFFF] = {};
			//std::string source = "";
			//
			//while (!file.eof())
			//{
			//	file.read(buffer, 0xFFF);
			//
			//	source.append(buffer, int(file.gcount()));
			//}


			int error = luaL_loadfile(lua, filePath);

			if (error)
			{
				if (lua_isstring(lua, -1))
				{
					lua_error(lua);
					//std::cout << lua_tostring(lua, -1) << std::endl;
					//
					//while ()
				}

				lua_pop(lua, 1);
			}
			else
			{
				RunProtectedFunction(lua);

				lua_pop(lua, 1);
			}

			//RunProtectedChunk(lua, source.c_str(), ar.source, ar.currentline, (int)source.size());

			return 0;
		}

		int LuaEnvironment::Traceback(lua_State* lua)
		{
			StackTraceback.clear();

			luaL_traceback(lua, lua, 0, 1);

			const char* trace = lua_tostring(lua, -1);

			for (int i = 0; trace[i];)
			{
				while (trace[i] == ' ' || trace[i] == '\t' || trace[i] == '\n')
					++i;

				int j;

				for (j = 0; trace[i + j] && trace[i + j] != '\n'; ++j);

				std::string line(trace + i, j);

				if (line.size() > 0)
					StackTraceback.push_back(line);

				i += j;
			}

			int start = -1;

			for (int i = 1; start == -1 && i < int(StackTraceback.size()); ++i)
			{
				std::string& line = StackTraceback[i];

				if (!(line[0] == '[' && line[1] == 'C' && line[2] == ']'))
					start = i;
			}

			if (start == -1)
				start = 1;

			std::string& line = StackTraceback[start];

			int size = 0;

			for (int i = 0; line[i] != ' '; ++i)
				if (line[i] == ':')
					size = i + 2;

			const char* message = lua_tostring(lua, 1);

			bool errorThrownByCpp = false;

			for (int i = 0; i < size - 1; ++i)
			{
				if (message[i] != line[i])
				{
					errorThrownByCpp = true;

					break;
				}
			}

			if (errorThrownByCpp)
				ErrorMessage = std::string(line.c_str(), size);
			else
				ErrorMessage = std::string();

			ErrorMessage += message;
			ErrorMessage += "\n\nStack Begin\n";

			for (int i = start; i < int(StackTraceback.size()); ++i)
				ErrorMessage += '\t' + StackTraceback[i] + '\n';

			ErrorMessage += "Stack End\n";

			lua_pushstring(lua, ErrorMessage.c_str());

			return 1;
		}
	}
}