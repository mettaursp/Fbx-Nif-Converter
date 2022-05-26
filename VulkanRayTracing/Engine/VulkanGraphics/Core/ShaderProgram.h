#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderPipeline;
		class DrawOperation;
		class FrameBuffer;
		class SwapChain;

		class ShaderProgram : public Object
		{
		public:
			std::shared_ptr<DrawOperation> DrawOperation;

			virtual void UpdateUniforms() {}

			void Draw(const std::shared_ptr<SwapChain>& swapChain);
			void SetPipeline(const std::shared_ptr<ShaderPipeline>& shaderPipeline);
			void SetOutput(const std::shared_ptr<FrameBuffer>& output);
			void Reset();
			void Prepare();
			const std::shared_ptr<ShaderPipeline>& GetPipeline() const { return BoundPipeline; }
			bool OutputsToSwapChain();

		private:
			std::shared_ptr<ShaderPipeline> BoundPipeline;
			std::shared_ptr<FrameBuffer> Output;
		};
	}
}