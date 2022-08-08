#pragma once

#include <Engine/VulkanGraphics/Core/ShaderPipeline.h>

namespace Engine
{
	namespace Graphics
	{
		class PhongForwardPipeline : public ShaderPipeline
		{
		public:
			vk::Format ColorFormat = vk::Format::eR8G8B8A8Unorm;
			vk::Format DepthFormat = vk::Format::eD16Unorm;

			virtual void Configure();
		};
	}
}