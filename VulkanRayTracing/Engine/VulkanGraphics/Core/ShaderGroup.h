#pragma once

#include <Engine/Objects/Object.h>
#include <Engine/Math/Matrix4.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderPipeline;

		class ShaderGroup : public Object
		{
		public:
			std::shared_ptr<ShaderPipeline> ForwardShading;
			std::shared_ptr<ShaderPipeline> DeferredShading;

		private:

		};
	}
}