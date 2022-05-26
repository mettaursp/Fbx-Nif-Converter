#pragma once

import <tuple>;
import <string>;
import <vector>;

#include "MetaData.h"

#ifdef Constructor
#undef Constructor
#endif

namespace Engine
{
	//template <typename... Arguments>
	//class Event;

	class Object;

	namespace Core
	{
		const bool WarnAboutMissingConstructors = true;

		template <typename Class>
		struct ParentCheck
		{
			static inline bool IsTypeObject = false;

			template <typename Parent>
			struct IsValid
			{
				static inline const bool Value = true;
			};

			template <>
			struct IsValid<void>
			{
				static inline const bool Value = false;
			};
		};

		template <>
		struct ParentCheck<Engine::Object>
		{
			static inline bool IsTypeObject = true;

			template <typename Parent>
			struct IsValid
			{
				static inline const bool Value = true;
			};
		};

		template <typename Type>
		struct TypeContainer
		{
			typedef Type Type;
		};
	}

	namespace Reflection
	{
		namespace Warnings
		{
			void DefaultName(const std::string& name);
			void MissingConstructor(const std::string& name);
			void DefaultEnumName(const std::string& name);
			void Assert();
		}

		template <typename Type>
		void ReflectType();

		template <typename... Types>
		void ReflectFromPackage()
		{
			(ReflectType<Types>(), ...);
		}

		template <typename ClassType, typename Parent = void, typename... AllowedClasses>
		struct Reflect
		{
			template <typename... Members>
			static void Class(const char* name, const std::vector<std::string>& namespaces, const std::vector<std::string>& aliases, Members... members)
			{
				static_assert(Core::template ParentCheck<ClassType>::template IsValid<Parent>::Value);
				static_assert(std::is_base_of_v<Object, ClassType>);

				Meta::ReflectedType reflected{ true, false, false, name, namespaces, aliases };

				auto bindMembers = [&reflected]<typename Member>(Member member)
				{
					reflected.AddMember(member.template Bind<ClassType, true>());
				};

				(bindMembers(members), ...);

				if (!Core::ParentCheck<ClassType>::IsTypeObject && std::string(name) == "Object")
					Warnings::DefaultName(typeid(ClassType).name());

				reflected.Constructor.Parent = Meta::Reflected<ClassType>::GetMeta();
				reflected.Constructor.Name = name;
				reflected.Constructor.IsConstructor = true;

				if (reflected.Constructor.Binding.Callback == nullptr)
				{
					if (Core::WarnAboutMissingConstructors)
						Warnings::MissingConstructor(name);

					Warnings::Assert();

					//reflected.Constructor.Binding.Callback = &ActiveBinder::BindObjectFactory<ClassType>::FactoryFunction;
				}

				reflected.Binding.Create = reflected.Constructor.Binding.Callback;
				reflected.Binding.Free = &ActiveBinder::BindObjectFactory<ClassType>::Destructor;

				reflected.Constructor.Binding.Validate();
				reflected.Binding.Validate();

				reflected.AllowedTypes.push_back(Meta::Reflected<void>::GetMeta());

				auto addAllowed = [] <typename Allowed> (Meta::ReflectedType & reflected, Allowed allowed)
				{
					reflected.AllowedTypes.push_back(Meta::Reflected<Allowed::Type>::GetMeta());
				};

				(addAllowed(reflected, Core::TypeContainer<AllowedClasses>{}), ...);

				Meta::ReflectedType* meta = Meta::Reflected<ClassType>::template SetMeta<Parent>(reflected);

				reflected.Binding.CacheOperators(*meta);
			}

			template <typename... Members>
			static void Class(const char* name, const std::vector<std::string>& namespaces, Members... members)
			{
				Class<Members...>(name, namespaces, {}, members...);
			}

			template <typename... Members>
			static void Class(const char* name, Members... members)
			{
				Class<Members...>(name, {}, {}, members...);
			}

			template <typename... Members>
			static void Type(const char* name, const std::vector<std::string>& namespaces, const std::vector<std::string>& aliases, Members... members)
			{
				static_assert(!std::is_base_of_v<Object, ClassType>);

				Meta::ReflectedType reflected{ true, false, false, name, namespaces, aliases };

				auto bindMembers = [&reflected]<typename Member>(Member member)
				{
					reflected.AddMember(member.template Bind<ClassType, false>());
				};

				(bindMembers(members), ...);

				if (!Core::ParentCheck<ClassType>::IsTypeObject && std::string(name) == "Object")
					Warnings::DefaultName(typeid(ClassType).name());

				reflected.Constructor.Parent = Meta::Reflected<ClassType>::GetMeta();
				reflected.Constructor.Name = name;
				reflected.Constructor.IsConstructor = true;
				
				if (reflected.Constructor.Binding.Callback == nullptr)
				{
					if (Core::WarnAboutMissingConstructors)
						Warnings::MissingConstructor(name);

					Warnings::Assert();
					//reflected.Constructor.Binding.Callback = &ActiveBinder::BindTypeFactory<ClassType>::FactoryFunction;
				}

				reflected.Binding.Create = reflected.Constructor.Binding.Callback;
				reflected.Binding.Free = &ActiveBinder::BindTypeFactory<ClassType>::Destructor;

				reflected.Constructor.Binding.Validate();
				reflected.Binding.Validate();

				auto addAllowed = [] <typename Allowed> (Meta::ReflectedType & reflected, Allowed allowed)
				{
					reflected.AllowedTypes.push_back(Meta::Reflected<Allowed::Type>::GetMeta());
				};

				(addAllowed(reflected, Core::TypeContainer<AllowedClasses>{}), ...);

				Meta::ReflectedType* meta = Meta::Reflected<ClassType>::template SetMeta<Parent>(reflected);

				reflected.Binding.CacheOperators(*meta);
			}

			template <typename... Members>
			static void Type(const char* name, const std::vector<std::string>& namespaces, Members... members)
			{
				Type<Members...>(name, namespaces, {}, members...);
			}

			template <typename... Members>
			static void Type(const char* name, Members... members)
			{
				Type<Members...>(name, {}, {}, members...);
			}

			template <typename... AllowedTypes>
			static void FundamentalType(const char* name)
			{
				Meta::ReflectedType reflected{ false, true, false, name };

				reflected.Constructor.Parent = Meta::Reflected<ClassType>::GetMeta();
				reflected.Constructor.Name = name;
				reflected.Constructor.IsConstructor = true;
				reflected.IsFundamental = true;

				auto addAllowed = [] <typename Allowed> (Meta::ReflectedType& reflected, Allowed allowed)
				{
					reflected.AllowedTypes.push_back(Meta::Reflected<Allowed::Type>::GetMeta());
				};

				(addAllowed(reflected, Core::TypeContainer<AllowedTypes>{}), ...);

				Meta::Reflected<ClassType>::template SetMeta<Parent>(reflected);
			}

			template <typename... EnumValues>
			static void Enum(const char* name, EnumValues... values)
			{
				Meta::ReflectedType reflected{ false, true, false, name };

				if (std::string(name) == "TestEnum")
					Warnings::DefaultEnumName(typeid(ClassType).name());

				reflected.Constructor.Parent = Meta::Reflected<ClassType>::GetMeta();
				reflected.Constructor.Name = name;
				reflected.Constructor.IsConstructor = true;
				reflected.IsEnum = true;
				reflected.Namespaces = { "Enum" };

				reflected.AllowedTypes.push_back(Meta::Reflected<std::string>::GetMeta());
				reflected.AllowedTypes.push_back(Meta::Reflected<lua_Number>::GetMeta());
				reflected.AllowedTypes.push_back(Meta::Reflected<lua_Integer>::GetMeta());

				auto bindValues = [&reflected]<typename EnumValue>(EnumValue value)
				{
					reflected.AddValue(value.template Bind<ClassType>());
				};

				(bindValues(values), ...);

				Meta::Reflected<ClassType>::template SetMeta<Parent>(reflected);
			}
		};

		namespace Core
		{
			template <typename Type1, typename Type2>
			inline constexpr bool isnt_same_v = !std::is_same_v<Type1, Type2>;

			template <bool IsMutable>
			struct Mutability
			{
			};

			struct StaticFunction {};

			template <auto Pointer>
			struct Member
			{
				const char* Name = nullptr;

				template <typename ClassType>
				struct Traits
				{
					template <typename MemberType, bool IsStatic, bool IsConst>
					struct TraitsData
					{
						static const bool IsStatic = IsStatic;
						static const bool IsConst = IsConst;

						const Meta::ReflectedType* Type;

						MemberType GetType() const {}
					};

					template <typename MemberType>
					static constexpr auto Get(MemberType ClassType::*)
					{
						typedef std::remove_cvref_t<MemberType> UnqualifiedType;

						return TraitsData<MemberType, false, std::is_const_v<MemberType>> { Meta::Reflected<UnqualifiedType>::GetMeta() };
					}

					template <typename MemberType >
					static constexpr auto Get(MemberType*)
					{
						typedef std::remove_cvref_t<MemberType> UnqualifiedType;

						return TraitsData<MemberType, true, std::is_const_v<MemberType>> { Meta::Reflected<UnqualifiedType>::GetMeta() };
					}
				};

				template <typename ClassType, bool>
				auto Bind() const
				{
					typedef decltype(Pointer) Traits;
					typedef Traits::MemberType MemberType;
					typedef std::remove_cvref_t<MemberType> UnqualifiedType;

					Meta::Member member{ Name, Traits::IsStatic, Traits::IsConst, Meta::Reflected<UnqualifiedType>::GetMeta(), Meta::Reflected<ClassType>::GetMeta() };

					member.Binding.Getter = &ActiveBinder::BindMember<ClassType, Traits::MemberType, Pointer>::Bind<Traits::IsStatic, Traits::IsConst>::Getter;
					member.Binding.Setter = &ActiveBinder::BindMember<ClassType, Traits::MemberType, Pointer>::Bind<Traits::IsStatic, Traits::IsConst>::Setter;

					member.Binding.Validate();

					return member;
				}
			};

			template <typename... Overloads>
			struct Function
			{
				const char* Name = nullptr;
				std::tuple<Overloads...> FunctionOverloads;

				template <typename ClassType, bool>
				auto Bind() const
				{
					Meta::Function::OverloadVector overloads;

					auto bindOverload = [&overloads]<typename Overload>(Overload overload)
					{
						return overload.template Bind<ClassType>(overloads);
					};

					auto bindOverloads = []<typename... BoundOverloads>(BoundOverloads...)
					{
						return &ActiveBinder::BindFunction<BoundOverloads...>::Callback;
					};

					Meta::Function function{ Name, false, {}, Meta::Reflected<ClassType>::GetMeta() };

					function.Binding.Callback = std::apply([&bindOverloads, &bindOverload](const auto&... functionOverloads) { return bindOverloads(bindOverload(functionOverloads)...); }, FunctionOverloads);

					function.Binding.Validate();

					function.Overloads = std::move(overloads);

					return function;
				}
			};

			template <typename... Overloads>
			struct Constructor
			{
				std::tuple<Overloads...> FunctionOverloads;

				template <typename ClassType, bool IsObject>
				auto Bind() const
				{
					Meta::Function::OverloadVector overloads;

					Meta::Function function{ nullptr, true, {}, Meta::Reflected<ClassType>::GetMeta() };

					function.Binding.Callback =  std::apply([&overloads](const auto&... functionOverloads)
					{
						auto bindOverload = [&overloads]<typename Accessor>(Accessor overload)
						{
							return overload.Bind<ClassType>(overloads);
						};

						auto bindOverloads = []<typename... BoundOverloads>(BoundOverloads... overloads)
						{
							return &ActiveBinder::BindFunction<BoundOverloads...>::Constructor<IsObject>::Callback;
						};

						return bindOverloads(bindOverload(functionOverloads)...);
					}, FunctionOverloads);

					function.Binding.Validate();

					function.Overloads = std::move(overloads);

					return function;
				}
			};

			template <auto Pointer, typename PointerType, typename OverloadData, typename ReturnType, typename... Arguments>
			struct Overload
			{
				typedef OverloadData OverloadData;

				static constexpr int GetArguments() { return OverloadData::GetArguments(); }

				bool IsStatic;
				bool IsConst;
				OverloadData FunctionOverload;

				template <typename ClassType>
				auto Bind(Meta::Function::OverloadVector& overloads) const
				{
					typedef std::remove_cvref_t<ReturnType> UnqualifiedReturnType;
					typedef ActiveBinder::BindOverload<Pointer, OverloadData::GetRequiredArguments(), OverloadData::GetArguments(), ClassType, ReturnType, Arguments...>::template Bind<OverloadData::IsStatic, OverloadData::IsConst> BoundOverload;

					Meta::FunctionOverload::ParameterVector parameters;

					auto bindParameters = [&parameters]<typename Argument>(Argument parameter)
					{
						parameters.push_back(parameter.template Bind<ClassType>());
					};

					std::apply([&bindParameters](const auto&... functionParameters) { (bindParameters(functionParameters), ...); }, FunctionOverload.FunctionArguments);

					Meta::FunctionOverload overload{ IsStatic, IsConst, Meta::Reflected<UnqualifiedReturnType>::GetMeta(), std::move(parameters) };

					overloads.push_back(std::move(overload));

					return BoundOverload();
				}
			};

			template <auto Pointer, bool IsStatic, bool IsConst>
			struct LuaOverload
			{
				template <typename ClassType>
				auto Bind(Meta::Function::OverloadVector& overloads) const
				{
					typedef ActiveBinder::BindLuaOverload<Pointer, ClassType>::template Bind<IsStatic, IsConst> BoundOverload;

					Meta::FunctionOverload overload{ IsStatic, IsConst, Meta::Reflected<void>::GetMeta() }; // TODO: change to "any" type

					overloads.push_back(overload);

					return BoundOverload();
				}
			};

			template <typename OverloadData, typename... Arguments>
			struct ConstructorOverload
			{
				typedef OverloadData OverloadData;

				static constexpr int GetArguments() { return OverloadData::GetArguments(); }

				OverloadData FunctionOverload;

				template <typename ClassType>
				auto Bind(Meta::Function::OverloadVector& overloads) const
				{
					typedef ActiveBinder::BindConstructorOverload<OverloadData::GetRequiredArguments(), OverloadData::GetArguments(), ClassType, Arguments...> BoundConstructorOverload;

					Meta::FunctionOverload::ParameterVector parameters;

					auto bindParameters = [&parameters]<typename Argument>(Argument parameter)
					{
						parameters.push_back(parameter.template Bind<ClassType>());
					};

					std::apply([&bindParameters](const auto&... functionParameters) { (bindParameters(functionParameters), ...); }, FunctionOverload.FunctionArguments);

					Meta::FunctionOverload overload{ false, false, Meta::Reflected<ClassType>::GetMeta(), std::move(parameters) };

					overloads.push_back(std::move(overload));

					return BoundConstructorOverload();
				}
			};

			template <bool IsStatic, bool IsConst, typename ReturnType, typename... Arguments>
			struct OverloadData
			{
				static inline const bool IsStatic = IsStatic;
				static inline const bool IsConst = IsConst;

				ReturnType Returns;
				std::tuple<Arguments...> FunctionArguments;

				template <int ArgumentCount>
				static constexpr int OptionalArgumentSum()
				{
					return (Arguments::OptionalArgument + ...);
				}

				template <>
				static constexpr int OptionalArgumentSum<0>()
				{
					return 0;
				}

				static constexpr int GetArguments()
				{
					return OptionalArgumentSum<sizeof...(Arguments)>();

				}

				template <int ArgumentCount>
				static constexpr int RequiredArgumentSum()
				{
					return (Arguments::RequiredArgument + ...);
				}

				template <>
				static constexpr int RequiredArgumentSum<0>()
				{
					return 0;
				}

				static constexpr int GetRequiredArguments()
				{
					

					return RequiredArgumentSum<sizeof...(Arguments)>();
				}
			};

			template <auto Pointer, typename PointerType, typename AccessorType, bool IsStatic, bool IsConst>
			struct Getter
			{
				static inline constexpr bool IsGetter = true;

				template <typename ClassType>
				auto Bind(Meta::Property& property) const
				{
					typedef std::remove_cvref_t<AccessorType> UnqualifiedReturnType;
					typedef ActiveBinder::BindGetter<ClassType, AccessorType, PointerType, Pointer>::template Bind<IsStatic, IsConst> BoundGetter;

					Meta::PropertyFunction accessor{ Meta::Reflected<UnqualifiedReturnType>::GetMeta() };

					property.Getter = accessor;
					property.Binding.Getter = &BoundGetter::Callback;

					return BoundGetter();
				}
			};

			template <auto Pointer, typename PointerType, typename AccessorType, bool IsStatic, bool IsConst>
			struct Setter
			{
				typedef AccessorType Type;
				typedef PointerType PointerType;

				static inline constexpr PointerType Pointer = Pointer;
				static inline constexpr bool IsGetter = false;

				template <typename ClassType>
				auto Bind(Meta::Property& property) const
				{
					typedef std::remove_cvref_t<AccessorType> UnqualifiedReturnType;
					typedef ActiveBinder::BindSetter<ClassType, AccessorType, PointerType, Pointer>::template Bind<IsStatic, IsConst> BoundSetter;

					Meta::PropertyFunction accessor{ Meta::Reflected<UnqualifiedReturnType>::GetMeta() };

					property.Setters.push_back(accessor);

					return BoundSetter();
				}
			};

			template <bool IsStatic, bool IsConst>
			struct OverloadAccessor;

			template <>
			struct OverloadAccessor<false, false>
			{
				template <typename OverloadData, typename ReturnType, typename... Arguments>
				struct Binder
				{
					OverloadData FunctionOverload;

					template <typename Type, ReturnType(Type::* Callback) (Arguments::Type...)>
					auto Bind()
					{
						return Overload<Callback, decltype(Callback), OverloadData, ReturnType, Arguments...> { false, false, FunctionOverload };
					}
				};

				template <typename AccessorType>
				struct GetterBinder
				{
					template <typename Type, AccessorType(Type::* Callback)()>
					auto Bind()
					{
						return Getter<Callback, decltype(Callback), AccessorType, false, false>{ };
					}
				};

				template <typename AccessorType>
				struct SetterBinder
				{
					template <typename Type, void(Type::* Callback)(AccessorType)>
					auto Bind()
					{
						return Setter<Callback, decltype(Callback), AccessorType, false, false>{ };
					}
				};
			};

			template <>
			struct OverloadAccessor<true, false>
			{
				template <typename OverloadData, typename ReturnType, typename... Arguments>
				struct Binder
				{
					OverloadData FunctionOverload;

					template <ReturnType(*Callback) (Arguments::Type...)>
					auto Bind()
					{
						return Overload<Callback, decltype(Callback), OverloadData, ReturnType, Arguments...> { true, false, FunctionOverload };
					}
				};

				template <typename AccessorType>
				struct GetterBinder
				{
					template <AccessorType(*Callback)()>
					auto Bind()
					{
						return Getter<Callback, decltype(Callback), AccessorType, true, false>{ };
					}
				};

				template <typename AccessorType>
				struct SetterBinder
				{
					template <void(*Callback)(AccessorType)>
					auto Bind()
					{
						return Setter<Callback, decltype(Callback), AccessorType, true, false>{ };
					}
				};
			};

			template <>
			struct OverloadAccessor<false, true>
			{
				template <typename OverloadData, typename ReturnType, typename... Arguments>
				struct Binder
				{
					OverloadData FunctionOverload;

					template <typename Type, ReturnType(Type::* Callback) (Arguments::Type...) const>
					auto Bind()
					{
						return Overload<Callback, decltype(Callback), OverloadData, ReturnType, Arguments...> { false, true, FunctionOverload };
					}
				};

				template <typename AccessorType>
				struct GetterBinder
				{
					template <typename Type, AccessorType(Type::* Callback) () const>
					auto Bind()
					{
						return Getter<Callback, decltype(Callback), AccessorType, false, true>{ };
					}
				};

				template <typename AccessorType>
				struct SetterBinder
				{
					template <typename Type, void(Type::* Callback) (AccessorType) const>
					auto Bind()
					{
						return Setter<Callback, decltype(Callback), AccessorType, false, true>{ };
					}
				};
			};

			template <typename ReturnType>
			struct Returns
			{
				typedef ReturnType Type;
			};

			template <typename Type>
			struct Argument
			{
				static inline constexpr int OptionalArgument = 1;
				static inline constexpr int RequiredArgument = 1;

				const char* Name;
				const char* DefaultValue;

				Type GetType() const {}

				template <int Index>
				struct Bound
				{
					typedef Type Type;
					typedef void DefaultValue;

					static inline const bool HasDefault = false;
					static inline const int Index = Index;

					Type GetType() const {}
				};

				template <int Index>
				static constexpr auto BindArgument()
				{
					return Bound<Index>();
				}

				template <typename ClassType>
				auto Bind() const
				{
					typedef std::remove_cvref_t<Type> UnqualifiedType;

					Meta::Argument argument{ Name, DefaultValue, Meta::Reflected<UnqualifiedType>::GetMeta() };

					return argument;
				}
			};

			template <typename Type, std::remove_cvref_t<Type> Default>
			struct DefaultArgument
			{
				typedef std::remove_cvref_t<Type> UnqualifiedType;

				static inline constexpr int OptionalArgument = 1;
				static inline constexpr int RequiredArgument = 0;

				static inline constexpr UnqualifiedType DefaultValue = Default;

				const char* Name;

				Type GetType() const {}

				template <int Index>
				struct Bound
				{
					typedef Type Type;

					static inline const bool HasDefault = true;
					static inline const int Index = Index;

					Type GetType() const {}

					struct DefaultValue
					{
						static inline UnqualifiedType Value = Default;
					};
				};

				template <int Index>
				static constexpr auto BindArgument()
				{
					return Bound<Index>();
				}

				template <typename ClassType>
				auto Bind() const
				{
					typedef std::remove_cvref_t<Type> UnqualifiedType;

					Meta::Argument argument{ Name, "", Meta::Reflected<UnqualifiedType>::GetMeta() };

					return argument;
				}
			};

			template <typename Type, auto Default>
			struct DefaultDifferentArgument
			{
				typedef std::remove_cvref_t<Type> UnqualifiedType;

				static inline constexpr int OptionalArgument = 1;
				static inline constexpr int RequiredArgument = 0;

				static inline constexpr decltype(Default) DefaultValue = Default;

				const char* Name;

				Type GetType() const {}

				template <int Index>
				struct Bound
				{
					typedef Type Type;

					static inline const bool HasDefault = true;
					static inline const int Index = Index;

					Type GetType() const {}

					struct DefaultValue
					{
						static inline decltype(Default) Value = Default;
					};
				};

				template <int Index>
				static constexpr auto BindArgument()
				{
					return Bound<Index>();
				}

				template <typename ClassType>
				auto Bind() const
				{
					typedef std::remove_cvref_t<Type> UnqualifiedType;

					Meta::Argument argument{ Name, "", Meta::Reflected<UnqualifiedType>::GetMeta() };

					return argument;
				}
			};

			template <typename... Accessors>
			struct Property
			{
				const char* Name;
				bool IsStatic;
				std::tuple<Accessors...> PropertyAccessors;


				template <typename ClassType, bool>
				auto Bind() const
				{
					Meta::Property property{ Name, IsStatic, Meta::PropertyFunction(), Meta::Property::FunctionVector(), Meta::Reflected<ClassType>::GetMeta() };

					property.Binding.Setter =  std::apply([&property](const auto&... propertyAccessors)
					{
						auto bindAccessors = [&property]<typename Accessor>(Accessor accessor)
						{
							return accessor.Bind<ClassType>(property);
						};

						auto bindOverloads = []<typename... Overloads>(Overloads... overloads)
						{
							return &ActiveBinder::BindFunction<Overloads...>::Callback;
						};

						return bindOverloads(bindAccessors(propertyAccessors)...);
					}, PropertyAccessors);

					property.Binding.Validate();

					return property;
				}
			};

			template <typename PointerType, PointerType Pointer, bool IsStatic, typename ArgumentTuple, typename... Arguments>
			struct Event
			{
				const char* Name;
				ArgumentTuple EventArguments;

				template <typename ClassType, bool>
				auto Bind() const
				{
					typedef ActiveBinder::BindEvent<PointerType, Pointer, ClassType, Arguments...>::template Bind<IsStatic> BoundEvent;

					Meta::Event::ParameterVector parameters;

					auto bindParameters = [&parameters]<typename Argument>(Argument parameter)
					{
						parameters.push_back(parameter.template Bind<ClassType>());
					};

					std::apply([&bindParameters](const auto&... eventParameters) { (bindParameters(eventParameters), ...); }, EventArguments);

					Meta::Event event{ Name, IsStatic, std::move(parameters), Meta::Reflected<ClassType>::GetMeta() };

					event.Binding.Connect = &BoundEvent::Connect;
					event.Binding.Disconnect = &BoundEvent::Disconnect;
					event.Binding.Wait = &BoundEvent::Wait;

					event.Binding.Validate();

					return event;
				}
			};

			template <bool IsStatic>
			struct EventAccessor;

			template <>
			struct EventAccessor<true>
			{
				template <typename ArgumentTuple, typename... Arguments>
				struct Binder
				{
					const char* Name;
					ArgumentTuple EventArguments;

					template <Engine::Event<Arguments...>* Pointer>
					auto Bind() const
					{
						return Event<decltype(Pointer), Pointer, true, ArgumentTuple, Arguments...>{ Name, EventArguments };
					}
				};
			};

			template <>
			struct EventAccessor<false>
			{
				template <typename ArgumentTuple, typename... Arguments>
				struct Binder
				{
					const char* Name;
					ArgumentTuple EventArguments;

					template <typename ClassType, Engine::Event<Arguments...> ClassType::*Pointer>
					auto Bind() const
					{
						return Event<decltype(Pointer), Pointer, false, ArgumentTuple, Arguments...>{ Name, EventArguments };
					}
				};
			};

			struct EnumItem
			{
				const char* Name = nullptr;
				int Value = 0;

				template <typename ClassType>
				auto Bind() const
				{
					Meta::EnumItem item{ Name, Value, Meta::Reflected<ClassType>::GetMeta() };

					return item;
				}
			};

			template <typename MemberType, bool IsStatic, typename PointerType >
			struct MemberPointer
			{
				typedef MemberType MemberType;
				typedef PointerType PointerType;

				static inline const bool IsStatic = IsStatic;
				static inline const bool IsConst = std::is_const_v<PointerType>;

				PointerType Pointer;
			};

			template <int ArgCount>
			struct OverloadBinding
			{
				template <bool IsStatic, bool IsConst, typename OverloadData, typename ReturnType, typename... Arguments>
				struct BindWith
				{
					template <typename Type, Type... Indices>
					static auto GetBinder(const OverloadData& data, std::integer_sequence<Type, Indices...>)
					{
						typedef Core::OverloadAccessor<IsStatic, IsConst> Accessor;
						typedef Accessor::template Binder<OverloadData, ReturnType::Type, Arguments::template Bound<Indices + 1>...> Binder;

						return Binder{ data };
					}
				};

				template <typename OverloadData, typename... Arguments>
				struct BindWithConstructor
				{

					template <typename Type, Type... Indices>
					static auto GetBinder(const OverloadData& data, std::integer_sequence<Type, Indices...>)
					{
						typedef Core::ConstructorOverload<OverloadData, Arguments::template Bound<Indices + 1>...> Binder;

						return Binder{ data };
					}
				};
			};

			template <>
			struct OverloadBinding<0>
			{
				template <bool IsStatic, bool IsConst, typename OverloadData, typename ReturnType>
				struct BindWith
				{
					template <typename Type, Type... Indices>
					static auto GetBinder(const OverloadData& data, std::integer_sequence<Type, Indices...>)
					{
						typedef Core::OverloadAccessor<IsStatic, IsConst> Accessor;
						typedef Accessor::template Binder<OverloadData, ReturnType::Type> Binder;

						return Binder{ data };
					}
				};

				template <typename OverloadData>
				struct BindWithConstructor
				{

					template <typename Type, Type... Indices>
					static auto GetBinder(const OverloadData& data, std::integer_sequence<Type, Indices...>)
					{
						typedef Core::ConstructorOverload<OverloadData> Binder;

						return Binder{ data };
					}
				};
			};

			template <typename Type>
			concept MemberPointerType = requires(Type value)
			{
				{ MemberPointer{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept OverloadType = requires(Type value)
			{
				{ Overload{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept LuaOverloadType = requires(Type value)
			{
				{ LuaOverload{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept FunctionOverloadType = OverloadType<Type> || LuaOverloadType<Type>;

			template <typename Type>
			concept ConstructorOverloadType = requires(Type value)
			{
				{ ConstructorOverload{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept ReturnsType = requires(Type value)
			{
				{ Returns{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept ArgumentType = requires(Type value)
			{
				{ Argument{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept DefaultArgumentType = requires(Type value)
			{
				{ DefaultArgument{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept DefaultDifferentArgumentType = requires(Type value)
			{
				{ DefaultDifferentArgument{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept AnyArgumentType = ArgumentType<Type> || DefaultArgumentType<Type> || DefaultDifferentArgumentType<Type>;

			template <typename Type>
			concept GetterType = requires(Type value)
			{
				{ Getter{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept SetterType = requires(Type value)
			{
				{ Setter{ value } } -> std::same_as<Type>;
			};

			template <typename Type>
			concept AccessorType = GetterType<Type> || SetterType<Type>;
		}

		constexpr Core::Mutability<true> Mutable = Core::Mutability<true>();
		constexpr Core::Mutability<false> Const = Core::Mutability<false>();
		constexpr Core::StaticFunction Static = Core::StaticFunction();

		template <typename Type, typename Class>
		consteval auto Bind(Type Class::* pointer)
		{
			return Core::MemberPointer<Type, false, Type Class::*>{ pointer };
		}

		template <typename Type>
		consteval auto Bind(Type* pointer)
		{
			return Core::MemberPointer<Type, true, Type*>{ pointer };
		}

		template <auto Pointer> requires Core::MemberPointerType<decltype(Pointer)>
		auto Member(const char* name)
		{
			return Core::Member<Pointer> { name };
		}

		template <Core::FunctionOverloadType... Overloads>
		auto Function(const char* name, Overloads... overloads)
		{
			return Core::Function<Overloads...> { name, std::make_tuple(overloads...) };
		}

		template <Core::ConstructorOverloadType... Overloads>
		auto Constructor(Overloads... overloads)
		{
			return Core::Constructor<Overloads...> { std::make_tuple(overloads...) };
		}

		template <Core::ReturnsType ReturnType, Core::AnyArgumentType... Arguments>
		auto Overload(Core::Mutability<true>, ReturnType returnType, Arguments... arguments)
		{
			auto data = Core::OverloadData<false, false, ReturnType, Arguments...>{ returnType, std::make_tuple(arguments...) };

			typedef Core::OverloadBinding<sizeof...(Arguments)> Binding;
			typedef Binding::template BindWith<false, false, decltype(data), ReturnType, Arguments...> BindWith;

			return BindWith::GetBinder(data, std::make_index_sequence<sizeof...(Arguments)>{});
		}

		template <Core::ReturnsType ReturnType, Core::AnyArgumentType... Arguments>
		auto Overload(Core::Mutability<false>, ReturnType returnType, Arguments... arguments)
		{
			auto data = Core::OverloadData<false, true, ReturnType, Arguments...>{ returnType, std::make_tuple(arguments...) };

			typedef Core::OverloadBinding<sizeof...(Arguments)> Binding;
			typedef Binding::template BindWith<false, true, decltype(data), ReturnType, Arguments...> BindWith;

			return BindWith::GetBinder(data, std::make_index_sequence<sizeof...(Arguments)>{});
		}

		template <Core::ReturnsType ReturnType, Core::AnyArgumentType... Arguments>
		auto Overload(Core::StaticFunction, ReturnType returnType, Arguments... arguments)
		{
			auto data = Core::OverloadData<true, false, ReturnType, Arguments...>{ returnType, std::make_tuple(arguments...) };

			typedef Core::OverloadBinding<sizeof...(Arguments)> Binding;
			typedef Binding::template BindWith<true, false, decltype(data), ReturnType, Arguments...> BindWith;

			return BindWith::GetBinder(data, std::make_index_sequence<sizeof...(Arguments)>{});
		}

		template <Core::AnyArgumentType... Arguments>
		auto Overload(Arguments... arguments)
		{
			auto data = Core::OverloadData<false, false, Core::Returns<void>, Arguments...>{ Core::Returns<void>(), std::make_tuple(arguments...) };


			typedef Core::OverloadBinding<sizeof...(Arguments)> Binding;
			typedef Binding::template BindWithConstructor<decltype(data), Arguments...> BindWith;

			return BindWith::GetBinder(data, std::make_index_sequence<sizeof...(Arguments)>{});
		}

		template <typename ClassType, int(ClassType::*Pointer)(lua_State*)>
		auto LuaOverload()
		{
			return Core::LuaOverload<Pointer, false, false>();
		}

		template <typename ClassType, int(ClassType::* Pointer)(lua_State*) const>
		auto LuaOverload()
		{
			return Core::LuaOverload<Pointer, true, false>();
		}

		template <int(*Pointer)(lua_State*)>
		auto LuaOverload()
		{
			return Core::LuaOverload<Pointer, false, true>();
		}

		template <typename ReturnType>
		auto Returns()
		{
			return Core::Returns<ReturnType>();
		}

		template <typename Type>
		auto Argument(const char* name)
		{
			return Core::Argument<Type>{ name, nullptr };
		}

		template <typename Type, std::remove_cvref_t<Type> DefaultValue>
		auto Argument(const char* name)
		{
			return Core::DefaultArgument<Type, DefaultValue>{ name };
		}

		template <typename Type, auto DefaultValue> requires Core::isnt_same_v<Type, decltype(DefaultValue)>
		auto Argument(const char* name)
		{
			return Core::DefaultDifferentArgument<Type, DefaultValue>{ name };
		}

		template <typename Type>
		consteval Type Default(const Type& type)
		{
			return type;
		}

		template <Core::AccessorType... Accessors>
		auto Property(const char* name, Core::StaticFunction, Accessors... accessors)
		{
			return Core::Property<Accessors...>{ name, true, std::make_tuple(accessors...) };
		}

		template <Core::AccessorType... Accessors>
		auto Property(const char* name, Accessors... accessors)
		{
			return Core::Property<Accessors...>{ name, false, std::make_tuple(accessors...) };
		}

		template <typename AccessorType>
		auto Getter(Core::Mutability<true>)
		{
			return Core::OverloadAccessor<false, false>::GetterBinder<AccessorType> {};
		}

		template <typename AccessorType>
		auto Getter(Core::Mutability<false>)
		{
			return Core::OverloadAccessor<false, true>::GetterBinder<AccessorType> {};
		}

		template <typename AccessorType>
		auto Getter(Core::StaticFunction)
		{
			return Core::OverloadAccessor<true, false>::GetterBinder<AccessorType> {};
		}

		template <typename AccessorType>
		auto Setter(Core::Mutability<true>)
		{
			return Core::OverloadAccessor<false, false>::SetterBinder<AccessorType> {};
		}

		template <typename AccessorType>
		auto Setter(Core::Mutability<false>)
		{
			return Core::OverloadAccessor<false, true>::SetterBinder<AccessorType> {};
		}

		template <typename AccessorType>
		auto Setter(Core::StaticFunction)
		{
			return Core::OverloadAccessor<true, false>::SetterBinder<AccessorType> {};
		}

		template <Core::AnyArgumentType... Arguments>
		auto Event(const char* name, Core::StaticFunction, Arguments... arguments)
		{
			return Core::EventAccessor<true>::Binder<std::tuple<Arguments...>, decltype(arguments.GetType())...> { name, std::make_tuple(arguments...) };
		}

		template <Core::AnyArgumentType... Arguments>
		auto Event(const char* name, Arguments... arguments)
		{
			return Core::EventAccessor<false>::Binder<std::tuple<Arguments...>, decltype(arguments.GetType())...> { name, std::make_tuple(arguments...) };
		}

		template <auto EnumValue> requires std::is_enum_v<decltype(EnumValue)>
		auto Value(const char* name)
		{
			return Core::EnumItem{ name, EnumValue };
		}
	}
}