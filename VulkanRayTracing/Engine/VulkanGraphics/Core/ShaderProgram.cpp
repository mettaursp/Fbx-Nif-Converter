#include "ShaderProgram.h"

#include "SwapChain.h"
#include "ShaderPipeline.h"
#include "DrawOperation.h"
#include "RenderTarget.h"

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

		void ShaderProgram::SetOutput(const std::shared_ptr<RenderTarget>& output)
		{
			if (OutputFormat == nullptr)
				throw "output format not set";

			if (output->GetFormat() != OutputFormat)
				throw "render target format doesn't match";
		}

		void ShaderProgram::SetOutputFormat(const std::shared_ptr<BufferFormat>& outputFormat)
		{
			if (OutputFormat != nullptr)
				throw "already set output format";

			OutputFormat = outputFormat;
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