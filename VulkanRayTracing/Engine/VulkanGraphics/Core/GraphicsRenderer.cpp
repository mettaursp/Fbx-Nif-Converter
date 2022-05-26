#include "GraphicsRenderer.h"

#include "ShaderProgram.h"
#include "GraphicsWindow.h"
#include "SwapChain.h"
#include "RenderQueue.h"
#include "VulkanErrorHandling.h"

namespace Engine
{
	namespace Graphics
	{
		void GraphicsRenderer::Draw(const std::shared_ptr<SwapChain>& swapChain)
		{
			Graphics::RenderQueue& queue = swapChain->GetRenderQueue();

			for (size_t i = 0; i < ShaderPrograms.size(); ++i)
				ShaderPrograms[i]->Draw(swapChain);
		}

		void GraphicsRenderer::AddProgram(const std::shared_ptr<ShaderProgram>& program)
		{
			ShaderPrograms.push_back(program);
		}

		void GraphicsRenderer::Prepare(const std::shared_ptr<SwapChain>& swapChain)
		{
			for (size_t i = 0; i < ShaderPrograms.size(); ++i)
				ShaderPrograms[i]->Prepare();
		}

		void GraphicsRenderer::Resize(const std::shared_ptr<SwapChain>& swapChain)
		{
			VK_CALL(swapChain->GetDevice().waitIdle);

			bool resetSwapChain = false;

			for (size_t i = 0; i < ShaderPrograms.size(); ++i)
				if (ShaderPrograms[i]->OutputsToSwapChain())
					ShaderPrograms[i]->Reset();

			Prepare(swapChain);
		}

		const std::shared_ptr<ShaderProgram>& GraphicsRenderer::GetOutputProgram() const
		{
			static const std::shared_ptr<ShaderProgram> NullPointer = nullptr;

			for (size_t i = 0; i < ShaderPrograms.size(); ++i)
				if (ShaderPrograms[i]->OutputsToSwapChain())
					return ShaderPrograms[i];

			return NullPointer;
		}

		bool GraphicsRenderer::OutputsToSwapChain()
		{
			for (size_t i = 0; i < ShaderPrograms.size(); ++i)
				if (ShaderPrograms[i]->OutputsToSwapChain())
					return true;

			return false;
		}
	}
}