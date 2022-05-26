#pragma once

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

namespace Engine
{
	namespace Graphics
	{
		class FrameBuffer;
		class ShaderPipeline;
		class Mesh;
		class MeshAsset;
		class SwapChain;
		class GraphicsWindow;

		class RenderQueue
		{
		public:
			SwapChain* Owner = nullptr;
			GraphicsWindow* Window = nullptr;
			vk::CommandBuffer CommandBuffer;

			void StartFrame();
			void BindRenderPass();
			void SetFrameBuffer(const std::shared_ptr<FrameBuffer>& frameBuffer);
			void SetShaderPipeline(const std::shared_ptr<ShaderPipeline>& pipeline);
			void DrawMesh(const std::shared_ptr<Mesh>& mesh);
			void DrawMesh(const std::shared_ptr<MeshAsset>& mesh);
			void PushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, void* data);
			void EndFrame();

		private:
			std::shared_ptr<ShaderPipeline> BoundPipeline;
			std::shared_ptr<FrameBuffer> BoundBuffer;
		};
	}
}