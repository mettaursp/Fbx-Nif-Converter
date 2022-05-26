#pragma once

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

#include "DeviceResource.h"
#include "FrameSwapFences.h"
#include "RenderQueue.h"
#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class Texture;
		class FrameBuffer;
		class ShaderPipeline;
		class Mesh;
		class GraphicsRenderer;

		struct SwapChainBuffer
		{
			RenderQueue Queue;
			vk::CommandBuffer PresentCommandBuffer;
			std::shared_ptr<Texture> Image;
			std::shared_ptr<FrameBuffer> Buffer;
		};

		class SwapChain : public DeviceResource
		{
		public:
			uint32_t DesiredBuffers = 3;

			FrameSwapFences Fences;
			GraphicsWindow* Window;

			~SwapChain();

			virtual void FreeData();

			void AddRenderer(const std::shared_ptr<GraphicsRenderer>& renderer);
			void RemoveRenderer(const std::shared_ptr<GraphicsRenderer>& renderer);
			void DrawRenderers();
			void PrepareRenderers();
			void Resize();

			void InitializeFences(const std::shared_ptr<GraphicsWindow>& window);
			void Reinitialize(const std::shared_ptr<GraphicsWindow>& window, const std::shared_ptr<ShaderPipeline>& pipeline);

			void InitializeFrameBuffers(const std::shared_ptr<ShaderPipeline>& pipeline);
			void ReleaseFrameBuffers();
			void InitializeImages(const std::shared_ptr<GraphicsWindow>& window);
			void ReleaseImages();
			void ReleaseSwapChain();
			void ReleaseData();

			vk::Result AttemptAdvanceBuffer();

			vk::Result SendQueuedCommands();

			uint32_t GetImageCount() const { return ImageCount; }
			const std::shared_ptr<Texture>& GetDepthTexture() const { return DepthTexture; }
			const std::shared_ptr<Texture>& GetImage(int index = -1);
			const std::shared_ptr<FrameBuffer>& GetFrameBuffer(int index = -1);
			const vk::CommandBuffer& GetCommandBuffer(int index = -1);
			const vk::CommandBuffer& GetPresentCommandBuffer(int index = -1);
			RenderQueue& GetRenderQueue(int index = -1);
			vk::SwapchainKHR& GetResource() { return Resource; }
			const uint32_t& GetCurrentBuffer() const { return CurrentBuffer; }
			void SetCurrentBuffer(uint32_t newBuffer);
			void InitializeCommandBuffers();
			void ReleaseCommandBuffers();

		private:
			bool InitializedImages = false;
			bool InitializedFrameBuffers = false;
			bool InitializedSwapChain = false;
			bool InitializedCommandBuffers = false;
			bool InitializedDepth = false;

			uint32_t CurrentBuffer = 0;

			vk::SwapchainKHR Resource;

			uint32_t ImageCount;

			std::shared_ptr<Texture> DepthTexture;

			std::vector<SwapChainBuffer> Buffers;
			std::vector<std::shared_ptr<GraphicsRenderer>> AttachedRenderers;
			std::shared_ptr<GraphicsRenderer> AttachedRenderer;

			// TODO: refactor these out into command buffer objects
			void BuildImageOwnershipBarrier(uint32_t index);
			void InitializeDepth(const std::shared_ptr<GraphicsWindow>& window);
		};
	}
}