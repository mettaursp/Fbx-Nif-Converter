#include "Reflected.h"


#include "MetaData.h"

namespace Engine
{
	namespace Meta
	{
		ReflectedType* MakeMeta()
		{
			return new ReflectedType();
		}

		void InheritReflected(ReflectedType* type, const ReflectedType* parent)
		{
			type->Parent = parent;
			type->CopyMembers(parent);
		}

		void Register(const ReflectedType* meta)
		{
			ReflectedTypes::RegisterType(meta);
		}
	}
}