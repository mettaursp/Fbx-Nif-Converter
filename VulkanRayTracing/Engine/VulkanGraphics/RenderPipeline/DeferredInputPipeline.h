#pragma once

#include <Engine/VulkanGraphics/Core/ShaderPipeline.h>

namespace Engine
{
	namespace Graphics
	{
		class DeferredInputPipeline : public ShaderPipeline
		{
		public:
			virtual void Configure();
		};
	}
}