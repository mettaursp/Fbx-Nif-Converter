#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderGroup;

		struct MaterialProperties
		{

		};

		class Material : public Object
		{
		public:
			std::shared_ptr<ShaderGroup> ShaderGroup;

		private:

		};
	}
}