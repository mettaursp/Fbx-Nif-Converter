#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderPipeline;
		class DrawOperation;
		class FrameBuffer;
		class RenderTarget;
		class BufferFormat;
		class SwapChain;

		class ShaderProgram : public Object
		{
		public:
			std::shared_ptr<DrawOperation> DrawOperation;

			virtual void UpdateUniforms() {}

			void Draw(const std::shared_ptr<SwapChain>& swapChain);
			void SetPipeline(const std::shared_ptr<ShaderPipeline>& shaderPipeline);
			void SetOutput(const std::shared_ptr<RenderTarget>& output);
			void SetOutputFormat(const std::shared_ptr<BufferFormat>& outputFormat);
			void Reset();
			void Prepare();
			const std::shared_ptr<ShaderPipeline>& GetPipeline() const { return BoundPipeline; }
			bool OutputsToSwapChain();

		private:
			std::shared_ptr<ShaderPipeline> BoundPipeline;
			std::shared_ptr<RenderTarget> Output;
			std::shared_ptr<BufferFormat> OutputFormat;
		};
	}
}