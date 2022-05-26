#pragma once

namespace Engine
{
	namespace Reflection
	{
		template <typename Package>
		void ReflectPackage();

		template <typename... Packages>
		void ReflectPackages()
		{
			(ReflectPackage<Packages>(), ...);
		}
	}
}