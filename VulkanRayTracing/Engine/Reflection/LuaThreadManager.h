#pragma once

import <functional>;
import <string>;

struct lua_State;

typedef std::function<int(lua_State*)> LuaCallback;

namespace Engine
{
	namespace Lua
	{
		struct ThreadStatusEnum
		{
			enum ThreadStatus
			{
				Uninitialized,
				ParseError,
				Running,
				RuntimeError,
				Yielded,
				Finished,
				Dead
			};
		};

		typedef ThreadStatusEnum::ThreadStatus ThreadStatus;
		typedef std::function<void(ThreadStatus)> StatusChangedCallback;

		void Initialize(lua_State* lua);
		void Update(lua_State* lua, float delta);
		void Update2(lua_State* lua, float delta);
		int Spawn(const std::string& source, const std::string& name, const LuaCallback& initializeCallback = nullptr, const StatusChangedCallback& statusChangedCallback = nullptr);
		int PCall(lua_State* lua);
		void Kill(int threadID);
		bool Running(int threadID);
		bool Yielded(int threadID);
		bool Dead(int threadID);
		int GetData(lua_State* lua, int threadID);
		int GetThreadID(lua_State* lua);
		lua_State* GetThread(int id);

		bool CompareThreads(lua_State* thread1, lua_State* thread2);

		int CoroutineCreate(lua_State* lua);
		int CoroutineResume(lua_State* lua);
		int CoroutineYield(lua_State* lua);
	}
}
