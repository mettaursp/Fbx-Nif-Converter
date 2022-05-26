#include "Reflection.h"

import <iostream>;
#include <assert.h>

namespace Engine
{
	namespace Reflection
	{
		namespace Warnings
		{
			void DefaultName(const std::string& name)
			{
				std::cout << "warning: class '" << name << "'has the default name 'Object'!" << std::endl;
			}

			void MissingConstructor(const std::string& name)
			{
				std::cout << "warning: type '" << name << "' is missing a constructor!" << std::endl;
			}

			void DefaultEnumName(const std::string& name)
			{
				std::cout << "warning: enum '" << name << "'has the default name 'TestEnum'!" << std::endl;
			}

			void Assert()
			{
				assert(!Engine::Core::WarnAboutMissingConstructors);
			}
		}
	}
}