#pragma once

#include <lua.hpp>
import <string>;
import <memory>;

#include "BoundObject.h"
#include "../Object.h"
#include "Reflected.h"

namespace Engine
{
	namespace Lua
	{
		template <typename Type>
		struct BoundTypeAllocators
		{
			static inline IDHeap<Type> AllocatedData;
		};

		template <typename Type>
		struct BindType;

		template <>
		struct BindType<void>
		{
			static inline const int Returns = 0;
		};

		template <>
		struct BindType<bool>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, bool value)
			{
				lua_pushboolean(lua, (int)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static bool Pop(lua_State* lua, int index)
				{
					return (bool)lua_toboolean(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static bool Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (bool)lua_toboolean(lua, index);
				}
			};
		};

		template <>
		struct BindType<unsigned char>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, unsigned char value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static unsigned char Pop(lua_State* lua, int index)
				{
					return (unsigned char)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static unsigned char Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (unsigned char)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<signed char>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, signed char value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static signed char Pop(lua_State* lua, int index)
				{
					return (signed char)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static signed char Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (signed char)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<unsigned short>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, unsigned short value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static unsigned short Pop(lua_State* lua, int index)
				{
					return (unsigned short)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static unsigned short Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (unsigned short)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<short>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, short value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static short Pop(lua_State* lua, int index)
				{
					return (short)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static short Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (short)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<unsigned int>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, unsigned int value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static unsigned int Pop(lua_State* lua, int index)
				{
					return (unsigned int)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static unsigned int Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (unsigned int)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<int>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, int value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static int Pop(lua_State* lua, int index)
				{
					return (int)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static int Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (int)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<unsigned long long>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, unsigned long long value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static unsigned long long Pop(lua_State* lua, int index)
				{
					return (unsigned long long)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static unsigned long long Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (unsigned long long)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<long long>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, long long value)
			{
				lua_pushinteger(lua, (lua_Integer)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static int Pop(lua_State* lua, int index)
				{
					return (long long)lua_tointeger(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static int Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (long long)lua_tointeger(lua, index);
				}
			};
		};

		template <>
		struct BindType<float>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, float value)
			{
				lua_pushnumber(lua, (lua_Number)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static float Pop(lua_State* lua, int index)
				{
					return (float)lua_tonumber(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static float Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (float)lua_tonumber(lua, index);
				}
			};
		};

		template <>
		struct BindType<double>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, double value)
			{
				lua_pushnumber(lua, (lua_Number)value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static double Pop(lua_State* lua, int index)
				{
					return (double)lua_tonumber(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static double Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return (double)lua_tonumber(lua, index);
				}
			};
		};

		template <>
		struct BindType<std::string>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, const std::string& value)
			{
				lua_pushlstring(lua, value.c_str(), value.size());
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static std::string Pop(lua_State* lua, int index)
				{
					return std::string(lua_tostring(lua, index));
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static std::string Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return std::string(lua_tostring(lua, index));
				}
			};
		};

		template <>
		struct BindType<const char*>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, const char* value)
			{
				lua_pushstring(lua, value);
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static const char* Pop(lua_State* lua, int index)
				{
					return lua_tostring(lua, index);
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static const char* Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					return lua_tostring(lua, index);
				}
			};
		};

		template <typename Type>
		struct BindType<std::shared_ptr<Type>>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, const std::shared_ptr<Type>& value)
			{
				if (value == nullptr)
				{
					lua_pushnil(lua);

					return;
				}

				BoundObject& bound = BoundObject::MakeBinding(lua);

				bound.GameObject = value;
				bound.ObjectType = LuaObjectType::Object;
				bound.Type = bound.GameObject->GetMetaData();
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static std::shared_ptr<Type> Pop(lua_State* lua, int index)
				{
					if (lua_isnil(lua, index))
						return nullptr;

					BoundObject& bound = BoundObject::Get(lua, index);

					return bound.GameObject->Cast<Type>();
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static std::shared_ptr<Type> Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					if (lua_isnil(lua, index))
						return nullptr;

					BoundObject& bound = BoundObject::Get(lua, index);

					return bound.GameObject->Cast<Type>();
				}
			};
		};

		template <typename Type>
		struct BindType<std::weak_ptr<Type>>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, const std::weak_ptr<Type>& value)
			{
				if (value.expired())
				{
					lua_pushnil(lua);

					return;
				}

				BoundObject& bound = BoundObject::MakeBinding(lua);

				bound.GameObject = value.lock();
				bound.ObjectType = LuaObjectType::Object;
				bound.Type = bound.GameObject->GetMetaData();
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static std::weak_ptr<Type> Pop(lua_State* lua, int index)
				{
					if (lua_isnil(lua, index))
						return std::weak_ptr<Type>();

					BoundObject& bound = BoundObject::Get(lua, index);

					return bound.GameObject->Cast<Type>();
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static std::weak_ptr<Type> Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					if (lua_isnil(lua, index))
						return std::weak_ptr<Type>();

					BoundObject& bound = BoundObject::Get(lua, index);

					return bound.GameObject->Cast<Type>();
				}
			};
		};

		template <typename Type>
		struct BindType<Type*>
		{
			static inline const int Returns = 1;
		
			static void Push(lua_State* lua, Type* value)
			{
				if (value == nullptr)
				{
					lua_pushnil(lua);
		
					return;
				}

				BoundObject& bound = BoundObject::MakeBinding(lua);
		
				bound.GameObject = value->This.lock()->Cast<Type>();
				bound.ObjectType = LuaObjectType::Object;
				bound.Type = bound.GameObject->GetMetaData();
			}
		
			static void Push(lua_State* lua, const Type* value)
			{
				if (value == nullptr)
				{
					lua_pushnil(lua);
		
					return;
				}

				BoundObject& bound = BoundObject::MakeBinding(lua);
		
				bound.GameObject = value->This.lock()->Cast<Type>();
				bound.ObjectType = LuaObjectType::Object;
				bound.Type = bound.GameObject->GetMetaData();
			}
		
			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static Type* Pop(lua_State* lua, int index)
				{
					if (lua_isnil(lua, index))
						return nullptr;

					int reference = (int)(intptr_t)lua_touserdata(lua, index);
					BoundObject& bound = BoundObject::GetBound(reference);
		
					return bound.GameObject->Cast<Type>().get();
				}
			};
		
			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static Type* Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					if (lua_isnil(lua, index))
						return nullptr;

					int reference = (int)(intptr_t)lua_touserdata(lua, index);
					BoundObject& bound = BoundObject::GetBound(reference);
		
					return bound.GameObject->Cast<Type>().get();
				}
			};
		};

		template <typename Type>
		concept EnumType = std::is_enum_v<Type>;

		template <EnumType Type>
		struct BindType<Type>
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, Type value)
			{
				BoundObject& bound = BoundObject::MakeBinding(lua);

				bound.DataId = (int)value;
				bound.ObjectType = LuaObjectType::Enum;
				bound.Type = Meta::Reflected<Type>::GetMeta();
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static Type Pop(lua_State* lua, int index)
				{
					int type = lua_type(lua, index);

					if (type == LUA_TNUMBER)
						return (Type)lua_tointeger(lua, index);
					
					if (type == LUA_TSTRING)
						(Type)Meta::Reflected<Type>::GetMeta()->GetEnumItem(lua_tostring(lua, index))->Value;

					BoundObject& bound = BoundObject::Get(lua, index);

					return (Type)bound.DataId;
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static Type Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					int type = lua_type(lua, index);

					if (type == LUA_TNUMBER)
						return (Type)lua_tointeger(lua, index);

					if (type == LUA_TSTRING)
						(Type)Meta::Reflected<Type>::GetMeta()->GetEnumItem(lua_tostring(lua, index))->Value;

					BoundObject& bound = BoundObject::Get(lua, index);

					return (Type)bound.DataId;
				}
			};
		};

		template <typename Type>
		struct BindType
		{
			static inline const int Returns = 1;

			static void Push(lua_State* lua, const Type& value)
			{
				int dataId = BoundTypeAllocators<Type>::AllocatedData.RequestID();

				BoundTypeAllocators<Type>::AllocatedData.GetNode(dataId).GetData() = value;

				BoundObject& bound = BoundObject::MakeBinding(lua);

				bound.DataId = dataId;
				bound.ObjectType = LuaObjectType::Data;
				bound.Type = Meta::Reflected<Type>::GetMeta();
			}

			template <bool HasDefault>
			struct WithDefault
			{
				template <typename DefaultTraits>
				static Type& Pop(lua_State* lua, int index)
				{
					BoundObject& bound = BoundObject::Get(lua, index);

					return BoundTypeAllocators<Type>::AllocatedData.GetNode(bound.DataId).GetData();
				}
			};

			template <>
			struct WithDefault<true>
			{
				template <typename DefaultTraits>
				static Type& Pop(lua_State* lua, int index)
				{
					if (lua_gettop(lua) < index)
						return DefaultTraits::Value;

					BoundObject& bound = BoundObject::Get(lua, index);

					return BoundTypeAllocators<Type>::AllocatedData.GetNode(bound.DataId).GetData();
				}
			};
		};

		template <typename Base, typename Derived>
		inline constexpr bool isnt_base_of_v = !std::is_base_of_v<Base, Derived>;

		template <typename Type> requires std::is_base_of_v<Object, Type>
		Type* GetObject(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			return bound.GameObject->Cast<Type>().get();
		}

		template <typename Type> requires isnt_base_of_v<Object, Type>
		Type* GetObject(lua_State* lua)
		{
			BoundObject& bound = BoundObject::Get(lua);

			return &BoundTypeAllocators<Type>::AllocatedData.GetNode(bound.DataId).GetData();
		}
	}
}