#pragma once

import <vector>;
import <string>;
import <memory>;

namespace Engine
{
	namespace Meta
	{
		struct ReflectedType;

		ReflectedType* MakeMeta();
		void InheritReflected(ReflectedType* type, const ReflectedType* parent);
		void Register(const ReflectedType* meta);

		template <typename Type>
		struct Reflected
		{
			static constexpr const ReflectedType* GetMeta();

			static void SetMeta(const ReflectedType& meta);

			template <typename Parent>
			static ReflectedType* SetMeta(const ReflectedType& meta)
			{
				ParentMeta = Reflected<Parent>::Meta;

				Reflected<Parent>::ChildTypes.push_back(Meta);
				Reflected<Parent>::ChildInitialization.push_back(&InitializeInheritedTypes);

				SetMeta(meta);

				if (Reflected<Parent>::Initialized)
				{
					InheritReflected(Meta, ParentMeta);

					InitializeInheritedTypes();
				}

				return Meta;
			}

			template <>
			static ReflectedType* SetMeta<void>(const ReflectedType& meta)
			{
				SetMeta(meta);

				InitializeInheritedTypes();

				return Meta;
			}

		private:
			typedef void(*InitializationCallback)();
			typedef std::vector<ReflectedType*> MetaVector;
			typedef std::vector<InitializationCallback> InitializationVector;

			static inline ReflectedType* Meta = MakeMeta();
			static inline ReflectedType* ParentMeta;
			static inline bool Initialized = false;
			static inline MetaVector ChildTypes;
			static inline InitializationVector ChildInitialization;

			static void InitializeInheritedTypes();

			template <typename Type>
			friend struct Reflected;
		};

		template <>
		struct Reflected<const char*> : public Reflected<std::string> {};

		template <>
		struct Reflected<char*> : public Reflected<std::string> {};

		template <typename Type>
		struct Reflected<std::shared_ptr<Type>> : public Reflected<Type> {};

		template <typename Type>
		struct Reflected<std::weak_ptr<Type>> : public Reflected<Type> {};

		template <typename Type>
		constexpr const ReflectedType* Reflected<Type>::GetMeta()
		{
			return Meta;
		}

		template <typename Type>
		void Reflected<Type>::SetMeta(const ReflectedType& meta)
		{
			*Meta = meta;
			Meta->Inherits.push_back(Meta);

			Register(Meta);
		}

		template <typename Type>
		void Reflected<Type>::InitializeInheritedTypes()
		{
			Initialized = true;

			for (int i = 0; i < int(ChildTypes.size()); ++i)
			{
				InheritReflected(ChildTypes[i], Meta);
				ChildInitialization[i]();
			}
		}
	}
}