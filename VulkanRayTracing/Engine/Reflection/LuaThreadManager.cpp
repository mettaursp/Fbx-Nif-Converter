#include "LuaThreadManager.h"

import <iostream>;
import <chrono>;
import <map>;
#include <lua.hpp>

#include "../IdentifierHeap.h"
#include "LuaEnvironment.h"

namespace Engine
{
	namespace Lua
	{

		typedef std::vector<int> IntVector;
		typedef std::map<lua_State*, int> IDMap;

		struct Thread
		{
			bool IsScriptMainThread = false;
			std::string Name;
			std::string Source;
			std::string Error;
			lua_State* State = nullptr;
			LuaCallback InitializeCallback;
			StatusChangedCallback StatusChanged;
			int ID = -1;
			ThreadStatus Status = ThreadStatus::Uninitialized;
			std::chrono::steady_clock::time_point ThreadStarted;

			void SetStatus(ThreadStatus status)
			{
				Status = status;

				if (StatusChanged != nullptr)
					StatusChanged(status);
			}

			void Kill()
			{
				SetStatus(ThreadStatus::Dead);
			}

			Thread()
			{
				ThreadStarted = std::chrono::high_resolution_clock::now();
			}
		};

		IDHeap<Thread> Threads;
		IDMap ThreadIDs;
		IntVector ThreadQueue;
		IntVector ThreadStack;

		bool CheckError(lua_State* lua, Thread& thread, int status)
		{
			if (!status)
			{
				if (lua_type(lua, -1) == LUA_TSTRING)
					std::cout << lua_tostring(lua, -1) << std::endl;
				else if (status == LUA_ERRSYNTAX)
					std::cout << "unknown syntax error loading script '" << thread.Name << "'" << std::endl;
				else if (status == LUA_ERRMEM)
					std::cout << "memory error loading script '" << thread.Name << "'" << std::endl;
				else if (status == LUA_ERRRUN)
					std::cout << "unknown runtime error loading script '" << thread.Name << "'" << std::endl;
				else if (status == LUA_ERRERR)
					std::cout << "unknown error handler error loading script '" << thread.Name << "'" << std::endl;
			}

			return !status;
		}

		void Initialize(lua_State* lua)
		{
			lua_pushstring(lua, "ThreadData");
			lua_createtable(lua, 0, 0);

			lua_settable(lua, LUA_REGISTRYINDEX);

			const char coroutineWrapper[] =
				"	return function(coroutine, created, resumed, yielded, traceback, pcallwrap)"
				"\n		local create = coroutine.create"
				"\n		local resume = coroutine.resume"
				"\n		local yield = coroutine.yield"
				"\n		local wrap = coroutine.wrap"
				"\n		"
				"\n		function coroutine.create(func)"
				"\n			local thread = create(function(...) local res, err = xpcall(func, traceback, ...) return pcallwrap(res, err) end)"
				"\n			"
				"\n			created(thread)"
				"\n			"
				"\n			return thread"
				"\n		end"
				"\n		"
				"\n		function coroutine.resume(thread, var1, var2, ...)"
				"\n			resumed(thread)"
				"\n			"
				"\n			return resume(thread, var1, var2, ...)"
				"\n		end"
				"\n		"
				"\n		function coroutine.yield(var1, var2, ...)"
				"\n			yielded()"
				"\n			"
				"\n			return yield(var1, var2, ...)"
				"\n		end"
				"\n		"
				"\n		function coroutine.wrap(func)"
				"\n			local thread = coroutine.create(func)"
				"\n			"
				"\n			return function(...)"
				"\n				return coroutine.resume(thread, ...)"
				"\n			end"
				"\n		end"
				"\n		"
				"\n		local function wrapper(func)"
				"\n			local thread = create(func)"
				"\n			"
				"\n			created(thread)"
				"\n			"
				"\n			return function(...)"
				"\n				return coroutine.resume(thread, ...)"
				"\n			end"
				"\n		end"
				"\n		"
				"\n		return wrapper"
				"\n end";

			Lua::LuaEnvironment::RunChunk(lua, coroutineWrapper, __FILE__, __LINE__, sizeof(coroutineWrapper) - 1);

			lua_call(lua, 0, 1);
			lua_getglobal(lua, "coroutine");
			lua_pushcfunction(lua, CoroutineCreate);
			lua_pushcfunction(lua, CoroutineResume);
			lua_pushcfunction(lua, CoroutineYield);
			lua_pushcfunction(lua, Lua::LuaEnvironment::Traceback);
			lua_pushcfunction(lua, &PCall);
			lua_call(lua, 6, 1);

			lua_setglobal(lua, "coroutine.wrap");
		}

		void Update(lua_State* lua, float delta)
		{
			lua_pushstring(lua, "ThreadData");
			lua_gettable(lua, LUA_REGISTRYINDEX);

			for (int i = 0; i < int(ThreadQueue.size()); ++i)
			{
				if (!Threads.NodeAllocated(i))
					continue;

				Thread& thread = Threads.GetNode(ThreadQueue[i]).GetData();

				if (thread.Status == ThreadStatus::Uninitialized)
				{
					lua_pushinteger(lua, lua_Integer(thread.ID));
					lua_State* newThread = lua_newthread(lua);

					thread.State = newThread;

					ThreadIDs[thread.State] = thread.ID;

					lua_settable(lua, -3); // push to ThreadData

					const char threadWrapper[] = "return function(chunk, traceback, pcallwrap) local res, err = xpcall(chunk, traceback) return pcallwrap(res, err) end";

					int wrapperError = luaL_loadbuffer(newThread, threadWrapper, sizeof(threadWrapper) - 1, "ScriptDispatcher");
					
					lua_call(newThread, 0, 1);

					int error = luaL_loadbuffer(newThread, thread.Source.c_str(), thread.Source.size(), thread.Name.c_str());

					if (error)
					{
						thread.SetStatus(ThreadStatus::ParseError);

						std::cout << lua_tostring(newThread, -1);
					}
					else
					{
						{
							lua_createtable(newThread, 0, 0);

							thread.InitializeCallback(newThread);

							lua_createtable(newThread, 0, 1);
							lua_pushstring(newThread, "__index");
							lua_getglobal(newThread, "_G");
							lua_settable(newThread, -3);

							lua_setmetatable(newThread, -2);

							lua_setupvalue(newThread, -2, 1);
						}

						thread.SetStatus(ThreadStatus::Running);

						lua_pushcfunction(newThread, Lua::LuaEnvironment::Traceback);
						lua_pushcfunction(newThread, &PCall);
						
						lua_call(newThread, 3, 1);

						Thread& postCallThread = Threads.GetNode(ThreadQueue[i]).GetData();

						if (postCallThread.Status != ThreadStatus::Yielded)
							postCallThread.SetStatus(ThreadStatus::Finished);
					}
				}
			}

			lua_pop(lua, 1);

			ThreadQueue.clear();
		}

		int PCall(lua_State* lua)
		{
			if (!lua_toboolean(lua, 1))
			{
				int threadID = GetThreadID(lua);
				Thread& thread = Threads.GetNode(threadID).GetData();
				thread.Error = lua_tostring(lua, 2);

				std::cout << thread.Error.c_str() << std::endl;

				thread.SetStatus(ThreadStatus::RuntimeError);
			}

			lua_pushvalue(lua, 1);

			return 1;
		}

		int Spawn(const std::string& source, const std::string& name, const LuaCallback& initializeCallback, const StatusChangedCallback& statusChangedCallback)
		{
			int threadID = Threads.RequestID();

			Thread& thread = Threads.GetNode(threadID).GetData();

			thread.Name = name;
			thread.ID = threadID;
			thread.Source = source;
			thread.InitializeCallback = initializeCallback;
			thread.StatusChanged = statusChangedCallback;
			thread.IsScriptMainThread = true;

			ThreadQueue.push_back(threadID);

			return threadID;
		}

		void Kill(int threadID)
		{
			if (threadID == -1)
				return;

			if (!Threads.NodeAllocated(threadID))
				return;

			Thread& thread = Threads.GetNode(threadID).GetData();

			thread.Kill();
		}

		bool Running(int threadID)
		{
			if (threadID == -1)
				return false;

			if (!Threads.NodeAllocated(threadID))
				return false;

			return Threads.GetNode(threadID).GetData().Status == ThreadStatus::Running;
		}

		bool Yielded(int threadID)
		{
			if (threadID == -1)
				return false;

			if (!Threads.NodeAllocated(threadID))
				return false;

			Thread& thread = Threads.GetNode(threadID).GetData();

			return thread.Status == ThreadStatus::Yielded;
		}

		bool Dead(int threadID)
		{
			if (threadID == -1)
				return false;

			if (!Threads.NodeAllocated(threadID))
				return false;

			ThreadStatus status = Threads.GetNode(threadID).GetData().Status;

			return status == ThreadStatus::Dead || status == ThreadStatus::ParseError || status == ThreadStatus::RuntimeError;
		}

		int GetData(lua_State* lua, int threadID)
		{
			if (threadID == -1)
				return 0;

			if (!Threads.NodeAllocated(threadID))
				return 0;

			lua_pushstring(lua, "ThreadData");
			lua_gettable(lua, LUA_REGISTRYINDEX);

			lua_pushnumber(lua, lua_Number(threadID));
			lua_gettable(lua, -2);

			lua_replace(lua, -2);
			lua_pop(lua, 1);

			return 1;
		}

		int GetThreadID(lua_State* lua)
		{
			IDMap::iterator i = ThreadIDs.find(lua);

			if (i != ThreadIDs.end())
				return i->second;

			return -1;
		}

		lua_State* GetThread(int id)
		{
			if (id == -1) return nullptr;

			Thread& thread = Threads.GetNode(id).GetData();

			return thread.State;
		}

		int CoroutineCreate(lua_State* lua)
		{
			int threadID = Threads.RequestID();

			Thread& thread = Threads.GetNode(threadID).GetData();

			thread.ID = threadID;
			thread.State = lua_tothread(lua, 1);

			ThreadIDs[thread.State] = threadID;

			lua_pushstring(lua, "ThreadData");
			lua_gettable(lua, LUA_REGISTRYINDEX);

			lua_pushnumber(lua, lua_Number(threadID));
			lua_pushvalue(lua, 1);
			lua_settable(lua, -3);

			return 0;
		}

		bool CompareThreads(lua_State* thread1, lua_State* thread2)
		{
			const auto id1 = ThreadIDs.find(thread1);
			const auto id2 = ThreadIDs.find(thread2);

			if (id1 == ThreadIDs.end() || id2 == ThreadIDs.end())
			{
				std::cout << "warning: untracked thread registered" << std::endl;
				
				return false;
			}

			Thread& threadData1 = Threads.GetNode(id1->second).GetData();
			Thread& threadData2 = Threads.GetNode(id2->second).GetData();

			return threadData1.ThreadStarted > threadData2.ThreadStarted;
		}

		int CoroutineResume(lua_State* lua)
		{
			int threadID = GetThreadID(lua);

			if (threadID >= 0 && threadID <= Threads.Size() && Threads.NodeAllocated(threadID))
			{
				Thread& thread = Threads.GetNode(threadID).GetData();

				thread.SetStatus(ThreadStatus::Running);
			}

			return 0;
		}

		int CoroutineYield(lua_State* lua)
		{
			int threadID = GetThreadID(lua);

			if (threadID >= 0 && threadID <= Threads.Size() && Threads.NodeAllocated(threadID))
			{
				Thread& thread = Threads.GetNode(threadID).GetData();

				if (thread.IsScriptMainThread)
				{
					lua_pushstring(lua, "cannot yield in LuaScript main thread");
					lua_error(lua);
				}

				thread.SetStatus(ThreadStatus::Yielded);
			}

			return 0;
		}
	}
}
