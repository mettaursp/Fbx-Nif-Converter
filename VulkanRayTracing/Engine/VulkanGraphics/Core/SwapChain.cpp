#include "SwapChain.h"

#include "GraphicsWindow.h"
#include "VulkanErrorHandling.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "ShaderPipeline.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "GraphicsRenderer.h"

namespace Engine
{
	namespace Graphics
	{
		SwapChain::~SwapChain()
		{
			ReleaseFrameBuffers();
			ReleaseImages();
			ReleaseSwapChain();
		}

		void SwapChain::FreeData()
		{
			ReleaseFrameBuffers();
			ReleaseImages();
			ReleaseSwapChain();
		}

		void SwapChain::AddRenderer(const std::shared_ptr<GraphicsRenderer>& renderer)
		{
			AttachedRenderers.push_back(renderer);
		}

		void SwapChain::RemoveRenderer(const std::shared_ptr<GraphicsRenderer>& renderer)
		{
			for (size_t i = 0; i < AttachedRenderers.size(); ++i)
			{
				if (AttachedRenderers[i] == renderer)
				{
					std::swap(AttachedRenderers[i], AttachedRenderers.back());
					AttachedRenderers.pop_back();

					return;
				}
			}
		}

		void SwapChain::DrawRenderers()
		{
			Fences.WaitForFence(true);

			vk::Result result;

			do
			{
				result = AttemptAdvanceBuffer();

				if (result == vk::Result::eErrorOutOfDateKHR)
					Resize();
				else if (result == vk::Result::eSuboptimalKHR)
					break;
				else if (result == vk::Result::eErrorSurfaceLostKHR)
				{
					if (Window)
						Window->ReinitializeSurface();

					Resize();
				}
				else
					Graphics::CheckError(result, "swapChain->AttemptAdvanceBuffer");
			} while (result != vk::Result::eSuccess);

			Graphics::RenderQueue& queue = GetRenderQueue();

			if (queue.CommandBuffer == vk::CommandBuffer())
				return;

			queue.StartFrame();

			for (size_t i = 0; i < AttachedRenderers.size(); ++i)
			{
				AttachedRenderers[i]->Draw(This.lock()->Cast<SwapChain>());
			}

			queue.EndFrame();

			result = SendQueuedCommands();

			if (result == vk::Result::eErrorOutOfDateKHR)
				Resize();
			else if (result == vk::Result::eSuboptimalKHR && Window != nullptr) {
				const vk::SurfaceCapabilitiesKHR& surfCapabilities = Window->GetCapabilities(true);
				const Vector2I& resolution = Window->GetResolution();

				if (surfCapabilities.currentExtent.width != resolution.X || surfCapabilities.currentExtent.height != resolution.Y)
					Resize();
			}
			else if (result == vk::Result::eErrorSurfaceLostKHR)
			{
				if (Window)
					Window->ReinitializeSurface();

				Resize();
			}
			else
				Graphics::CheckError(result, "swapChain->SendQueuedCommands");
		}

		void SwapChain::Resize()
		{
			VK_CALL(Window->GetDevice().waitIdle);

			ReleaseData();

			for (size_t i = 0; i < AttachedRenderers.size(); ++i)
				if (AttachedRenderers[i]->OutputsToSwapChain())
					AttachedRenderers[i]->Resize(This.lock()->Cast<SwapChain>());

			PrepareRenderers();
		}

		void SwapChain::PrepareRenderers()
		{
			Window->InitializeCommandPool();

			for (size_t i = 0; i < AttachedRenderers.size(); ++i)
				AttachedRenderers[i]->Prepare(This.lock()->Cast<SwapChain>());

			bool reinitialized = false;

			for (size_t i = 0; i < AttachedRenderers.size(); ++i)
			{
				if (AttachedRenderers[i]->OutputsToSwapChain())
				{
					reinitialized = true;

					Reinitialize(nullptr, AttachedRenderers[i]->GetOutputProgram()->GetPipeline());

					break;
				}
			}

			if (!reinitialized)
				Reinitialize(nullptr, nullptr);

			Window->FlushCommands();
		}

		void SwapChain::InitializeFences(const std::shared_ptr<GraphicsWindow>& window)
		{
			Fences.Device = GetDevice();
			Fences.HasSeparatePresentQueue = window->HasSeparatePresentQueue();
			Fences.InitializeSemaphores(2);
			InitializeDepth(window);
		}

		void SwapChain::Reinitialize(const std::shared_ptr<GraphicsWindow>& window, const std::shared_ptr<ShaderPipeline>& pipeline)
		{
			InitializeImages(window);
			InitializeCommandBuffers();

			if (pipeline != nullptr)
				InitializeFrameBuffers(pipeline);
		}

		void SwapChain::InitializeImages(const std::shared_ptr<GraphicsWindow>& window)
		{
			vk::SwapchainKHR oldSwapchain = Resource;

			const vk::SurfaceCapabilitiesKHR& surfCapabilities = Window->GetCapabilities(true);
			auto& presentModes = Window->GetPresentModes(true);

			const Vector2I& resolution = Window->GetResolution();

			vk::Extent2D swapchainExtent;

			if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
				swapchainExtent = vk::Extent2D(resolution.X, resolution.Y);
			}
			else {
				swapchainExtent = surfCapabilities.currentExtent;
				Window->SetResolution(Vector2I(surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height));
			}

			uint32_t DesiredBuffers = 3;
			if (DesiredBuffers < surfCapabilities.minImageCount) {
				DesiredBuffers = surfCapabilities.minImageCount;
			}

			if ((surfCapabilities.maxImageCount > 0) && (DesiredBuffers > surfCapabilities.maxImageCount)) {
				DesiredBuffers = surfCapabilities.maxImageCount;
			}

			vk::SurfaceTransformFlagBitsKHR preTransform;
			if (surfCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
				preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
			}
			else {
				preTransform = surfCapabilities.currentTransform;
			}

			vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
				vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
				vk::CompositeAlphaFlagBitsKHR::eInherit,
			};

			for (uint32_t i = 0; i < sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); i++) {
				if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
					compositeAlpha = compositeAlphaFlags[i];
					break;
				}
			}

			auto const swapchain_ci = vk::SwapchainCreateInfoKHR()
				.setSurface(Window->GetSurface())
				.setMinImageCount(DesiredBuffers)
				.setImageFormat(Window->GetSurfaceFormat())
				.setImageColorSpace(Window->GetSurfaceColorSpace())
				.setImageExtent({ swapchainExtent.width, swapchainExtent.height })
				.setImageArrayLayers(1)
				.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
				.setImageSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(0)
				.setPQueueFamilyIndices(nullptr)
				.setPreTransform(preTransform)
				.setCompositeAlpha(compositeAlpha)
				.setPresentMode(Window->GetPresentMode())
				.setClipped(true)
				.setOldSwapchain(oldSwapchain);

			VK_CALL(GetDevice().createSwapchainKHR, &swapchain_ci, nullptr, &Resource);

			if (oldSwapchain) {
				GetDevice().destroySwapchainKHR(oldSwapchain, nullptr);
			}

			VK_CALL(GetDevice().getSwapchainImagesKHR, Resource, &ImageCount, static_cast<vk::Image*>(nullptr));

			std::vector<vk::Image> images(ImageCount);

			VK_CALL(GetDevice().getSwapchainImagesKHR, Resource, &ImageCount, images.data());

			Buffers.resize(ImageCount);

			for (uint32_t i = 0; i < ImageCount; ++i) {
				if (Buffers[i].Image == nullptr)
				{
					Buffers[i].Image = Engine::Create<Graphics::Texture>();
					Buffers[i].Image->AttachToContext(GetContext());
				}

				if (InitializedImages)
					Buffers[i].Image->ReleaseResources();

				Buffers[i].Image->InitializeViewFromImage(images[i], Window->GetResolution(), Window->This.lock()->Cast<GraphicsWindow>());
			}

			InitializeDepth(Window->This.lock()->Cast<GraphicsWindow>());

			InitializedImages = true;
			InitializedSwapChain = true;
		}

		void SwapChain::InitializeDepth(const std::shared_ptr<GraphicsWindow>& window)
		{
			if (InitializedDepth)
				return;

			if (DepthTexture == nullptr)
			{
				DepthTexture = Engine::Create<Graphics::Texture>();
				DepthTexture->AttachToContext(GetContext());
			}

			DepthTexture->InitializeDepth(window);

			InitializedDepth = true;
		}

		void SwapChain::InitializeCommandBuffers()
		{
			if (InitializedCommandBuffers)
				ReleaseCommandBuffers();

			auto const cmd = vk::CommandBufferAllocateInfo()
				.setCommandPool(Window->CommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

			for (uint32_t i = 0; i < ImageCount; ++i)
			{
				VK_CALL(GetDevice().allocateCommandBuffers, &cmd, &Buffers[i].Queue.CommandBuffer);

				Buffers[i].Queue.Owner = this;
				Buffers[i].Queue.Window = Window;
			}

			if (Window->HasSeparatePresentQueue()) {
				auto const present_cmd = vk::CommandBufferAllocateInfo()
					.setCommandPool(Window->PresentCommandPool)
					.setLevel(vk::CommandBufferLevel::ePrimary)
					.setCommandBufferCount(1);

				for (uint32_t i = 0; i < ImageCount; i++) {
					VK_CALL(GetDevice().allocateCommandBuffers, &present_cmd, &Buffers[i].PresentCommandBuffer);

					BuildImageOwnershipBarrier(i);
				}
			}

			InitializedCommandBuffers = true;
		}

		void SwapChain::ReleaseCommandBuffers()
		{
			if (!InitializedCommandBuffers) return;

			for (uint32_t i = 0; i < ImageCount; i++)
			{
				GetDevice().freeCommandBuffers(Window->CommandPool, { Buffers[i].Queue.CommandBuffer });

				if (Window->HasSeparatePresentQueue())
					GetDevice().freeCommandBuffers(Window->CommandPool, { Buffers[i].PresentCommandBuffer });
			}

			InitializedCommandBuffers = false;
		}

		void SwapChain::BuildImageOwnershipBarrier(uint32_t index)
		{
			auto const cmd_buf_info = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			VK_CALL(Buffers[index].PresentCommandBuffer.begin, &cmd_buf_info);

			auto const image_ownership_barrier =
				vk::ImageMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlags())
				.setDstAccessMask(vk::AccessFlags())
				.setOldLayout(vk::ImageLayout::ePresentSrcKHR)
				.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
				.setSrcQueueFamilyIndex(Window->GetGraphicsQueueFamilyIndex())
				.setDstQueueFamilyIndex(Window->GetPresentQueueFamilyIndex())
				.setImage(Buffers[index].Image->GetImage())
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

			Buffers[index].PresentCommandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlagBits(), 0, nullptr, 0,
				nullptr, 1, &image_ownership_barrier);

			VK_CALL(Buffers[index].PresentCommandBuffer.end);
		}

		void SwapChain::ReleaseData()
		{
			ReleaseFrameBuffers();
			ReleaseCommandBuffers();
			ReleaseImages();
		}

		void SwapChain::InitializeFrameBuffers(const std::shared_ptr<ShaderPipeline>& pipeline)
		{
			if (InitializedFrameBuffers)
				ReleaseFrameBuffers();

			const Vector2I& resolution = Window->GetResolution();

			for (uint32_t i = 0; i < ImageCount; i++) {
				if (Buffers[i].Buffer == nullptr)
				{
					Buffers[i].Buffer = Engine::Create<FrameBuffer>();
					Buffers[i].Buffer->AttachToContext(GetContext());
				}

				Buffers[i].Buffer->InitializeResource(Window->GetResolution());
				Buffers[i].Buffer->AddAttachment(Buffers[i].Image);
				Buffers[i].Buffer->AddAttachment(DepthTexture);
				Buffers[i].Buffer->InitializeAttachments(pipeline);
			}

			InitializedFrameBuffers = true;
		}

		void SwapChain::ReleaseFrameBuffers()
		{
			if (!InitializedFrameBuffers) return;

			for (uint32_t i = 0; i < ImageCount; i++)
				Buffers[i].Buffer->ReleaseFrameBuffer();

			InitializedFrameBuffers = false;
		}

		void SwapChain::ReleaseImages()
		{
			if (!InitializedImages)
				return;

			for (uint32_t i = 0; i < ImageCount; ++i)
			{
				if (Buffers[i].Image != nullptr)
					Buffers[i].Image->ReleaseResources();
			}

			if (DepthTexture != nullptr)
				DepthTexture->ReleaseResources();

			InitializedImages = false;
			InitializedDepth = false;
		}

		void SwapChain::ReleaseSwapChain()
		{
			Fences.ReleaseSemaphores();

			if (InitializedSwapChain)
				GetDevice().destroySwapchainKHR(Resource, nullptr);

			for (uint32_t i = 0; i < ImageCount; i++)
				Buffers[i].Image->ReleaseResources();

			InitializedSwapChain = false;
		}

		vk::Result SwapChain::AttemptAdvanceBuffer()
		{
			return GetDevice().acquireNextImageKHR(Resource, UINT64_MAX, Fences.GetAcquiredSemaphore(), vk::Fence(), &CurrentBuffer);
		}

		vk::Result SwapChain::SendQueuedCommands()
		{
			vk::PipelineStageFlags const pipe_stage_flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			auto const submit_info = vk::SubmitInfo()
				.setPWaitDstStageMask(&pipe_stage_flags)
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&Fences.GetAcquiredSemaphore())
				.setCommandBufferCount(1)
				.setPCommandBuffers(&GetCommandBuffer())
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores(&Fences.GetCompletedSemaphore());

			VK_CALL(Window->GetGraphicsQueue().submit, 1, &submit_info, Fences.GetFence());

			if (Window->HasSeparatePresentQueue()) {
				auto const present_submit_info = vk::SubmitInfo()
					.setPWaitDstStageMask(&pipe_stage_flags)
					.setWaitSemaphoreCount(1)
					.setPWaitSemaphores(&Fences.GetCompletedSemaphore())
					.setCommandBufferCount(1)
					.setPCommandBuffers(&GetPresentCommandBuffer())
					.setSignalSemaphoreCount(1)
					.setPSignalSemaphores(&Fences.GetOwnershipSemaphore());

				VK_CALL(Window->GetPresentQueue().submit, 1, &present_submit_info, vk::Fence());
			}

			auto const presentInfo = vk::PresentInfoKHR()
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(Window->HasSeparatePresentQueue() ? &Fences.GetOwnershipSemaphore()
					: &Fences.GetCompletedSemaphore())
				.setSwapchainCount(1)
				.setPSwapchains(&GetResource())
				.setPImageIndices(&GetCurrentBuffer());

			vk::Result result = Window->GetPresentQueue().presentKHR(&presentInfo);

			Fences.AdvanceFrame();

			return result;
		}

		void SwapChain::SetCurrentBuffer(uint32_t newBuffer)
		{
			CurrentBuffer = newBuffer;
		}

		const std::shared_ptr<Texture> NullTexture = nullptr;

		const std::shared_ptr<Texture>& SwapChain::GetImage(int index)
		{
			if (index == -1)
				index = CurrentBuffer;

			if (index < 0 || (size_t)index >= ImageCount)
				return NullTexture;

			return Buffers[index].Image;
		}

		const std::shared_ptr<FrameBuffer> NullFrameBuffer = nullptr;

		const std::shared_ptr<FrameBuffer>& SwapChain::GetFrameBuffer(int index)
		{
			if (index == -1)
				index = CurrentBuffer;

			if (index < 0 || (size_t)index >= ImageCount)
				return NullFrameBuffer;

			return Buffers[index].Buffer;
		}

		const vk::CommandBuffer& SwapChain::GetCommandBuffer(int index)
		{
			if (index == -1)
				index = CurrentBuffer;

			return Buffers[index].Queue.CommandBuffer;
		}

		const vk::CommandBuffer& SwapChain::GetPresentCommandBuffer(int index)
		{
			if (index == -1)
				index = CurrentBuffer;

			return Buffers[index].PresentCommandBuffer;
		}

		RenderQueue& SwapChain::GetRenderQueue(int index)
		{
			if (index == -1)
				index = CurrentBuffer;

			if (index < 0 || (size_t)index >= ImageCount)
				throw "out of bounds";

			return Buffers[index].Queue;
		}
	}
}