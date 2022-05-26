#pragma once

import <vector>;
import <string>;

struct lua_State;

namespace Engine
{
	namespace Lua
	{
		class LuaEnvironment
		{
		public:
			LuaEnvironment();
			~LuaEnvironment();

			void UpdateLua(lua_State* lua, float delta);
			lua_State* GetEnv() const { return State; }

			static int Traceback(lua_State* lua);
			static bool RunChunk(lua_State* lua, const char* source, const char* fileName, int lineNumber, int length);
			static int RunProtectedFunction(lua_State* lua);
			static int RunProtectedChunk(lua_State* lua, const char* source, const char* fileName, int lineNumber, int length);
			static int ProtectedDoFile(lua_State* lua);
		private:
			lua_State* State = nullptr;

			static inline std::vector<std::string> StackTraceback = std::vector<std::string>();
			static inline std::string ErrorMessage = std::string();

			void InitializeCorePackages(lua_State* lua);
			void InitializeGlobalNamespaces(lua_State* lua);
			void InitializeDebugLibrary(lua_State* lua);
			void InitializeGlobalFunctions(lua_State* lua);
			void InitializeWaitFunction(lua_State* lua);
			void InitializeTimerFunction(lua_State* lua);
			void InitializeDelayFunction(lua_State* lua);


			static int LuaQueueThread(lua_State* lua);
		};
	}
}