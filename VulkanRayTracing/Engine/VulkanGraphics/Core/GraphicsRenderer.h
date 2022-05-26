#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderProgram;
		class SwapChain;

		class GraphicsRenderer : public Object
		{
		public:
			void Draw(const std::shared_ptr<SwapChain>& swapChain);
			void AddProgram(const std::shared_ptr<ShaderProgram>& program);
			const std::shared_ptr<ShaderProgram>& GetOutputProgram() const;
			void Prepare(const std::shared_ptr<SwapChain>& swapChain);
			void Resize(const std::shared_ptr<SwapChain>& swapChain);
			bool OutputsToSwapChain();

		private:
			std::vector<std::shared_ptr<ShaderProgram>> ShaderPrograms;

		};
	}
}