#pragma once

import <algorithm>;

#include "Reflected.h"
#include "LuaTypeBinding.h"
#include "../Event.h"
#include "../Engine.h"
#include "LuaError.h"

#undef max

namespace Engine
{
	namespace Lua
	{
		const bool TestFunctions = false;

		const char* GetTypeName(const Meta::ReflectedType* type, const char* nullName = "unknown");
		const Meta::ReflectedType* GetArgumentType(lua_State* lua, int index);

		typedef int(*LuaFunction)(lua_State*);
		typedef bool(*LuaCheckFunction)(lua_State*);
		typedef void(*DestructorFunction)(BoundObject& object);

		struct LuaBinding
		{
			struct BoundMember
			{
				LuaFunction Getter = nullptr;
				LuaFunction Setter = nullptr;

				void Validate() const;
			};

			struct BoundFunction
			{
				LuaFunction Callback = nullptr;

				void Validate() const;
			};

			struct BoundEvent
			{
				LuaFunction Connect = nullptr;
				LuaFunction Disconnect = nullptr;
				LuaFunction Wait = nullptr;

				void Validate() const;
			};

			struct BoundObject
			{
				LuaFunction Create = nullptr;
				DestructorFunction Free = nullptr;
				LuaFunction ToString = nullptr;
				LuaFunction Call = nullptr;
				LuaFunction Length = nullptr;
				LuaFunction UnaryMinus = nullptr;
				LuaFunction Add = nullptr;
				LuaFunction Subtract = nullptr;
				LuaFunction Multiply = nullptr;
				LuaFunction Divide = nullptr;
				LuaFunction FloorDivide = nullptr;
				LuaFunction Modulus = nullptr;
				LuaFunction Power = nullptr;
				LuaFunction Concatenate = nullptr;
				LuaFunction BitwiseAnd = nullptr;
				LuaFunction BitwiseOr = nullptr;
				LuaFunction BitwiseXor = nullptr;
				LuaFunction BitwiseNot = nullptr;
				LuaFunction BitwiseShiftLeft = nullptr;
				LuaFunction BitwiseShiftRight = nullptr;
				LuaFunction BitwiseEquals = nullptr;
				LuaFunction LessThan = nullptr;
				LuaFunction LessThanEqualTo = nullptr;

				void CacheOperators(Meta::ReflectedType& type);
				void Validate() const;
			};
		};

		struct LuaBinder
		{
			template <typename Type>
			static const Meta::ReflectedType* GetMeta()
			{
				typedef std::remove_cvref_t<Type> UnqualifiedType;

				return Meta::Reflected<UnqualifiedType>::GetMeta();
			}

			template <typename Type>
			static bool ArgumentMatches(const Meta::ReflectedType* found)
			{
				return GetMeta<Type>()->CanAllow(found);
			}

			template <typename Type>
			struct TypeHolder
			{
				Type GetType();
			};

			lua_State* GetGlobalLuaState();
			int RegisterConnectedFunction(lua_State* lua);

			template <typename... Arguments>
			static bool ArgumentsMatch(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
			{
				auto matches = [count, &closestMatch, &expected]<typename T, T... Indices, typename... Holders>(const std::integer_sequence<T, Indices...>&indices, const Meta::ReflectedType** arguments, Holders... types) -> bool
				{
					auto matches = [count, &closestMatch, &expected]<typename Type>(TypeHolder<Type>, const Meta::ReflectedType* argument, int index) -> bool
					{
						if (index >= count || ArgumentMatches<Type>(argument)) return true;

						if (index > closestMatch)
						{
							closestMatch = index + 1;
							expected = GetMeta<Type>();
						}

						return false;
					};
					
					return (matches(types, arguments[Indices], Indices) && ...);
				};

				std::tuple<TypeHolder<Arguments>...> argumentsTuple;

				return std::apply([&matches, arguments]<typename... Holders>(Holders... types) -> bool { return matches(std::make_index_sequence<sizeof...(Arguments)>{}, arguments, types...); }, argumentsTuple);
			}

			template <typename ClassType>
			struct BindObjectFactory
			{
				static void Destructor(BoundObject& bound)
				{
					bound.GameObject = nullptr;
				}

				template <typename... Arguments>
				static void FactoryFunction(lua_State* lua, const Arguments&... arguments)
				{
					BoundObject& bound = BoundObject::MakeBinding(lua);

					bound.GameObject = Engine::Create<ClassType, Arguments...>(arguments...);
					bound.ObjectType = LuaObjectType::Object;
					bound.Type = GetMeta<ClassType>();
				}
			};

			template <typename ClassType>
			struct BindTypeFactory
			{
				static void Destructor(BoundObject& bound)
				{
					BoundTypeAllocators<ClassType>::AllocatedData.Release(bound.DataId);
				}

				template <typename... Arguments>
				static void FactoryFunction(lua_State* lua, const Arguments&... arguments)
				{
					int dataId = BoundTypeAllocators<ClassType>::AllocatedData.RequestID();

					BoundTypeAllocators<ClassType>::AllocatedData.GetNode(dataId).GetData() = ClassType(arguments...);

					BoundObject& bound = BoundObject::MakeBinding(lua);

					bound.DataId = dataId;
					bound.ObjectType = LuaObjectType::Data;
					bound.Type = GetMeta<ClassType>();
				}
			};

			template <typename ClassType, typename Type, auto PointerData>
			struct BindMember
			{
				typedef decltype(PointerData) Traits;
				typedef Traits::PointerType PointerType;

				static inline PointerType const Pointer = PointerData.Pointer;

				template <bool IsStatic, bool IsConst>
				struct Bind;

				template <>
				struct Bind<false, false>
				{
					static int Getter(lua_State* lua)
					{;
						ClassType* pointer = GetObject<ClassType>(lua);

						BindType<Clean<Type>>::Push(lua, (pointer->*Pointer));

						return 1;
					}

					static int Setter(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						(pointer->*Pointer) = BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, 3);

						return 0;
					}
				};

				template <>
				struct Bind<true, false>
				{
					static int Getter(lua_State* lua)
					{
						BindType<Clean<Type>>::Push(lua, *Pointer);

						return 1;
					}

					static int Setter(lua_State* lua)
					{
						int top = lua_gettop(lua);

						*Pointer = BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, top);

						return 0;
					}
				};

				template <>
				struct Bind<false, true>
				{
					static int Getter(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						BindType<Clean<Type>>::Push(lua, (pointer->*Pointer));

						return 1;
					}

					static int Setter(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						(pointer->*Pointer) = BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, 3);

						return 0;
					}
				};
			};

			template <typename Type>
			using Clean = std::remove_cvref_t<Type>;

			template <typename ClassType, typename Type, typename PointerType, auto Pointer>
			struct BindGetter
			{
				template <bool IsStatic, bool IsConst>
				struct Bind;

				template <>
				struct Bind<false, false>
				{
					static const inline int MinimumArguments = 1;
					static const inline int MaximumArguments = 1;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return count == 1;
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						BindType<Clean<Type>>::Push(lua, (pointer->*Pointer)());

						return 1;
					}
				};

				template <>
				struct Bind<true, false>
				{
					static const inline int MinimumArguments = 0;
					static const inline int MaximumArguments = 1;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return count == 1;
					}

					static int Callback(lua_State* lua)
					{
						BindType<Clean<Type>>::Push(lua, Pointer());

						return 1;
					}
				};

				template <>
				struct Bind<false, true>
				{
					static const inline int MinimumArguments = 1;
					static const inline int MaximumArguments = 1;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return count == 1;
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						BindType<Clean<Type>>::Push(lua, (pointer->*Pointer)());

						return 1;
					}
				};
			};

			template <typename ClassType, typename Type, typename PointerType, auto Pointer>
			struct BindSetter
			{
				template <bool IsStatic, bool IsConst>
				struct Bind;

				template <>
				struct Bind<false, false>
				{
					static const inline int MinimumArguments = 2;
					static const inline int MaximumArguments = 2;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return ArgumentsMatch<ClassType, Clean<Type>>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						(pointer->*Pointer)(BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, 2));

						return 0;
					}
				};

				template <>
				struct Bind<true, false>
				{
					static const inline int MinimumArguments = 1;
					static const inline int MaximumArguments = 2;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						if (count == 2)
							return ArgumentMatches<ClassType, Clean<Type>>(arguments[0]);

						return ArgumentsMatch<void, Clean<Type>>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						int top = lua_gettop(lua);

						Pointer(BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, top));

						return 0;
					}
				};

				template <>
				struct Bind<false, true>
				{
					static const inline int MinimumArguments = 2;
					static const inline int MaximumArguments = 2;

					static constexpr int GetArguments() { return MaximumArguments; }

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return ArgumentsMatch<ClassType, Clean<Type>>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						(pointer->*Pointer)(BindType<Clean<Type>>::WithDefault<false>::Pop<void>(lua, 2));

						return 0;
					}
				};
			};

			template <typename ReturnType>
			struct CallbackWrapper
			{
				template <auto Function, typename PointerType, typename... Arguments>
				static int Invoke(PointerType pointer, lua_State* lua, const Arguments&... arguments)
				{
					BindType<Clean<ReturnType>>::Push(lua, (pointer->*Function)(arguments...));

					return 1;
				}

				template <auto Function, typename... Arguments>
				static int Invoke(lua_State* lua, const Arguments&... arguments)
				{
					BindType<Clean<ReturnType>>::Push(lua, (*Function)(arguments...));

					return 1;
				}
			};

			template <>
			struct CallbackWrapper<void>
			{
				template <auto Function, typename PointerType, typename... Arguments>
				static int Invoke(PointerType pointer, lua_State* lua, const Arguments&... arguments)
				{
					(pointer->*Function)(arguments...);

					return 0;
				}

				template <auto Function, typename... Arguments>
				static int Invoke(lua_State* lua, const Arguments&... arguments)
				{
					(*Function)(arguments...);

					return 0;
				}
			};

			template <auto Pointer, typename ClassType>
			struct BindLuaOverload
			{
				template <bool IsStatic, bool IsConst>
				struct Bind;

				template <>
				struct Bind<false, false>
				{
					static constexpr int GetArguments()
					{
						return 0;
					}
					
					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return ArgumentsMatch<ClassType>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						return (pointer->*Pointer)(lua);
					}
				};

				template <>
				struct Bind<true, false>
				{
					static constexpr int GetArguments()
					{
						return 0;
					}

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return true;
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						return (*Pointer)(lua);
					}
				};

				template <>
				struct Bind<false, true>
				{
					static constexpr int GetArguments()
					{
						return 0;
					}

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return ArgumentsMatch<ClassType>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						return (pointer->*Pointer)(lua);
					}
				};
			};

			template <auto Pointer, int MinimumArguments, int MaximumArguments, typename ClassType, typename ReturnType, typename... Arguments>
			struct BindOverload
			{
				template <bool IsStatic, bool IsConst>
				struct Bind;

				template <>
				struct Bind<false, false>
				{
					static const inline int MinimumArguments = MinimumArguments + 1;
					static const inline int MaximumArguments = MaximumArguments + 1;

					static constexpr int GetArguments()
					{
						return MaximumArguments;
					}

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return count >= MinimumArguments && count <= MaximumArguments && ArgumentsMatch<ClassType, Clean<typename Arguments::Type>...>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						auto invoke = []<typename... Type>(lua_State * lua, ClassType* pointer, const Type&... arguments)
						{
							return CallbackWrapper<ReturnType>::template Invoke<Pointer, ClassType*, Type...>(pointer, lua, arguments...);
						};

						auto pop = []<typename Argument>(lua_State* lua, Argument)
						{
							return std::move(BindType<Clean<Argument::Type>>::WithDefault<Argument::HasDefault>::Pop<Argument::DefaultValue>(lua, Argument::Index + 1));
						};

						return invoke(lua, pointer, pop(lua, Arguments{})...);
					}
				};

				template <>
				struct Bind<true, false>
				{
					static const inline int MinimumArguments = MinimumArguments;
					static const inline int MaximumArguments = MaximumArguments;

					static constexpr int GetArguments()
					{
						return MaximumArguments;
					}

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						if (count < MinimumArguments || count > MaximumArguments) return false;

						return ArgumentsMatch<Clean<typename Arguments::Type>...>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						auto invoke = []<typename... Type>(lua_State* lua, const Type&... arguments)
						{
							return CallbackWrapper<ReturnType>::template Invoke<Pointer, Type...>(lua, arguments...);
						};

						auto pop = []<typename Argument>(lua_State* lua, Argument)
						{
							return std::move(BindType<Clean<Argument::Type>>::WithDefault<Argument::HasDefault>::Pop<Argument::DefaultValue>(lua, Argument::Index));
						};

						return invoke(lua, pop(lua, Arguments{})...);
					}
				};

				template <>
				struct Bind<false, true>
				{
					static const inline int MinimumArguments = MinimumArguments + 1;
					static const inline int MaximumArguments = MaximumArguments + 1;

					static constexpr int GetArguments()
					{
						return MaximumArguments;
					}

					static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
					{
						return count >= MinimumArguments && count <= MaximumArguments && ArgumentsMatch<ClassType, Clean<typename Arguments::Type>...>(arguments, count, closestMatch, expected);
					}

					static int Callback(lua_State* lua)
					{
						ClassType* pointer = GetObject<ClassType>(lua);

						auto invoke = []<typename... Type>(lua_State* lua, const ClassType* pointer, const Type&... arguments)
						{
							return CallbackWrapper<ReturnType>::template Invoke<Pointer, const ClassType*, Type...>(pointer, lua, arguments...);
						};

						auto pop = []<typename Argument>(lua_State* lua, Argument)
						{
							return std::move(BindType<Clean<Argument::Type>>::WithDefault<Argument::HasDefault>::Pop<Argument::DefaultValue>(lua, Argument::Index + 1));
						};

						return invoke(lua, pointer, pop(lua, Arguments{})...);
					}
				};
			};
			template <int MinimumArguments, int MaximumArguments, typename ClassType, typename... Arguments>
			struct BindConstructorOverload
			{
				static const inline int MinimumArguments = MinimumArguments + 1;
				static const inline int MaximumArguments = MaximumArguments + 1;

				static constexpr int GetArguments()
				{
					return MaximumArguments;
				}

				static bool MatchesOverload(const Meta::ReflectedType** arguments, int count, int& closestMatch, const Meta::ReflectedType*& expected)
				{
					return count >= MinimumArguments && count <= MaximumArguments && ArgumentsMatch<void, Clean<Arguments::Type>...>(arguments, count, closestMatch, expected);
				}

				template <bool IsObject>
				static int Callback(lua_State* lua)
				{
					auto pop = []<typename Argument>(lua_State * lua, Argument)
					{
						return std::move(BindType<Clean<Argument::Type>>::WithDefault<Argument::HasDefault>::Pop<Argument::DefaultValue>(lua, Argument::Index + 1));
					};

					BindObjectFactory<ClassType>::template FactoryFunction<typename Arguments::Type...>(lua, pop(lua, Arguments{})...);

					return 1;
				}

				template <>
				static int Callback<false>(lua_State* lua)
				{
					auto pop = []<typename Argument>(lua_State * lua, Argument)
					{
						return std::move(BindType<Clean<Argument::Type>>::WithDefault<Argument::HasDefault>::Pop<Argument::DefaultValue>(lua, Argument::Index + 1));
					};

					BindTypeFactory<ClassType>::template FactoryFunction<Arguments::Type...>(lua, pop(lua, Arguments{})...);

					return 1;
				}
			};

			template <typename... Overloads>
			struct ArgumentCounter;

			template <typename Overload, typename... Overloads>
			struct ArgumentCounter<Overload, Overloads...>
			{
				static consteval int GetMaxArguments()
				{
					constexpr int arguments = Overload::GetArguments();

					return std::max(arguments, ArgumentCounter<Overloads...>::GetMaxArguments());
				}
			};

			template <>
			struct ArgumentCounter<>
			{
				static consteval int GetMaxArguments()
				{
					return 0;
				}
			};

			template <typename... Overloads>
			struct BindFunction
			{
				static inline const int MaxArguments = std::max(1, ArgumentCounter<Overloads...>::GetMaxArguments()) + 1;

				static int Callback(lua_State* lua)
				{
					const Meta::ReflectedType* arguments[MaxArguments] = { nullptr };
					int stackTop = lua_gettop(lua);
					int returnedValues = 0;
					int closestMatch = -1;
					const Meta::ReflectedType* expected = nullptr;

					for (int i = 0; i < stackTop && i < MaxArguments; ++i)
						arguments[i] = GetArgumentType(lua, i + 1);

					auto attemptOverload = [lua, &arguments, &returnedValues, &closestMatch, &expected, stackTop]<typename Overload>(Overload)
					{
						if (TestFunctions || Overload::MatchesOverload(arguments, stackTop, closestMatch, expected))
						{
							returnedValues = Overload::Callback(lua);

							return true;
						}

						return false;
					};

					std::tuple<Overloads...> overloadTuple;

					bool foundOverload = std::apply([&attemptOverload](Overloads... overload) { return (attemptOverload(overload) || ...); }, overloadTuple);

					if (foundOverload)
						return returnedValues;

					foundOverload = std::apply([&attemptOverload](Overloads... overload) { return (attemptOverload(overload) || ...); }, overloadTuple);

					if (closestMatch == -1)
						return LuaError(lua, "got an unexpected number of arguments: %d", stackTop);

					return LuaError(lua, "got unexpected type on argument %d: expected '%s', got '%s'", closestMatch, GetTypeName(expected), GetTypeName(arguments[closestMatch], "nil"));
				}

				template <bool IsObject>
				struct Constructor;

				template <>
				struct Constructor<true>
				{
					static int Callback(lua_State* lua)
					{
						const Meta::ReflectedType* arguments[MaxArguments] = { nullptr };
						int stackTop = lua_gettop(lua);
						int returnedValues = 0;
						int closestMatch = -1;
						const Meta::ReflectedType* expected = nullptr;
						
						for (int i = 0; i < stackTop && i < MaxArguments; ++i)
							arguments[i] = GetArgumentType(lua, i + 1);
						
						auto attemptOverload = [lua, &arguments, &returnedValues, &closestMatch, &expected, stackTop]<typename Overload>(Overload)
						{
							if (TestFunctions || Overload::MatchesOverload(arguments, stackTop, closestMatch, expected))
							{
								returnedValues = Overload::Callback<true>(lua);
						
								return true;
							}
						
							return false;
						};
						
						std::tuple<Overloads...> overloadTuple;
						
						bool foundOverload = std::apply([&attemptOverload](Overloads... overload) { return (attemptOverload(overload) || ...); }, overloadTuple);

						if (foundOverload)
							return returnedValues;

						if (closestMatch == -1)
							return LuaError(lua, "got an unexpected number of arguments: %d", stackTop);

						return LuaError(lua, "got unexpected type on argument %d: expected '%s', got '%s'", closestMatch, GetTypeName(expected), GetTypeName(arguments[closestMatch], "nil"));
					}
				};

				template <>
				struct Constructor<false>
				{
					static int Callback(lua_State* lua)
					{
						const Meta::ReflectedType* arguments[MaxArguments] = { nullptr };
						int stackTop = lua_gettop(lua);
						int returnedValues = 0;
						int closestMatch = -1;
						const Meta::ReflectedType* expected = nullptr;
						
						for (int i = 0; i < stackTop && i < MaxArguments; ++i)
							arguments[i] = GetArgumentType(lua, i + 1);
						
						auto attemptOverload = [lua, &arguments, &returnedValues, &closestMatch, &expected, stackTop]<typename Overload>(Overload)
						{
							if (TestFunctions || Overload::MatchesOverload(arguments, stackTop, closestMatch, expected))
							{
								returnedValues = Overload::Callback<false>(lua);
						
								return true;
							}
						
							return false;
						};
						
						std::tuple<Overloads...> overloadTuple;
						
						bool foundOverload = std::apply([&attemptOverload](Overloads... overload) { return (attemptOverload(overload) || ...); }, overloadTuple);

						if (foundOverload)
							return returnedValues;

						if (closestMatch == -1)
							return LuaError(lua, "got an unexpected number of arguments: %d", stackTop);

						return LuaError(lua, "got unexpected type on argument %d: expected '%s', got '%s'", closestMatch, GetTypeName(expected), GetTypeName(arguments[closestMatch], "nil"));
					}
				};
			};

			template <typename PointerType, PointerType Pointer, typename ClassType, typename... Arguments>
			struct BindEvent
			{
				typedef Engine::Event<typename Arguments::Type...> BoundEvent;

				void ConnectEvent(lua_State* lua, BoundEvent& event)
				{
					int functionId = RegisterConnectedFunction(lua);

					int connection = Pointer->Connect(
						[functionId](Arguments::Type... arguments)
						{
							lua_State* lua = GetGlobalLuaState();

							//FetchConnectedFunction(lua, functionId);

							(CoreTypes<Arguments::Type>::Push(lua, arguments), ...);

							lua_call(lua, sizeof...(Arguments), 1);

							bool keepConnectionAlive = true;

							if (lua_isboolean(lua, -1))
								keepConnectionAlive = lua_toboolean(lua, -1) == 1;

							return keepConnectionAlive;
						}
					);

					lua_pushinteger(lua, connection);
				}

				template <bool IsStatic>
				struct Bind;

				template <>
				struct Bind<true>
				{
					static int Connect(lua_State* lua)
					{
						ConnectEvent(*Pointer);

						return 1;
					}

					static int Disconnect(lua_State* lua)
					{
						return 0;
					}

					static int Wait(lua_State* lua)
					{
						return 0;
					}
				};

				template <>
				struct Bind<false>
				{
					static int Connect(lua_State* lua)
					{
						const std::shared_ptr<ClassType>& object = GetObject<ClassType>(lua);
						const ClassType* pointer = object.get();

						ConnectEvent(pointer->*Pointer);

						return 0;
					}

					static int Disconnect(lua_State* lua)
					{
						return 0;
					}

					static int Wait(lua_State* lua)
					{
						return 0;
					}
				};
			};
		};
	}

	namespace Meta
	{
		typedef Lua::LuaBinding ActiveBinding;
	}

	namespace Reflection
	{
		typedef Lua::LuaBinder ActiveBinder;
	}
}