#include "ShaderProgram.h"

#include "SwapChain.h"
#include "ShaderPipeline.h"
#include "DrawOperation.h"

namespace Engine
{
	namespace Graphics
	{
		void ShaderProgram::Draw(const std::shared_ptr<SwapChain>& swapChain)
		{
			RenderQueue& queue = swapChain->GetRenderQueue();

			queue.SetShaderPipeline(BoundPipeline);

			BoundPipeline->SetFrame(swapChain->GetCurrentBuffer());

			UpdateUniforms();

			BoundPipeline->FlushUniformChanges();

			queue.SetShaderPipeline(BoundPipeline);

			DrawOperation->Draw(queue);
		}

		void ShaderProgram::SetPipeline(const std::shared_ptr<ShaderPipeline>& shaderPipeline)
		{
			if (BoundPipeline != nullptr) return;

			BoundPipeline = shaderPipeline;
		}

		void ShaderProgram::SetOutput(const std::shared_ptr<FrameBuffer>& output)
		{

		}

		void ShaderProgram::Reset()
		{
			BoundPipeline->Reset();
		}

		void ShaderProgram::Prepare()
		{
			BoundPipeline->InitializeRenderPass();
		}

		bool ShaderProgram::OutputsToSwapChain()
		{
			return true;
		}
	}
}