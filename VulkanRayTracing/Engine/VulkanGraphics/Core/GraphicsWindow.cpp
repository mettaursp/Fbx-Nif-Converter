#include "GraphicsWindow.h"

import <iostream>;

#include <vulkan/vulkan_structs.hpp>

#include "VulkanErrorHandling.h"
#include "GraphicsContext.h"
#include "Texture.h"
#include "SwapChain.h"

namespace Engine
{
	namespace Graphics
	{
		LRESULT CALLBACK GraphicsWindow::ProcessWindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg) {
			case WM_CLOSE:
				PostQuitMessage(0);
				break;
			case WM_PAINT:
				ViewSwapChain->DrawRenderers();
				break;
			case WM_GETMINMAXINFO:  // set window's minimum size
				return 0;
			case WM_ERASEBKGND:
				return 1;
			case WM_SIZE:
				// Resize the application to the new window size, except when
				// it was minimized. Vulkan doesn't support images or swapchains
				// with width=0 and height=0.
				if (wParam != SIZE_MINIMIZED) {
					Vector2I resolution;
		
					resolution.X = lParam & 0xffff;
					resolution.Y = (lParam & 0xffff0000) >> 16;
		
					SetResolution(resolution);
		
					if (ViewSwapChain != nullptr)
						ViewSwapChain->Resize();
				}
				break;
			case WM_KEYDOWN:
				switch (wParam) {
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
				}
				return 0;
			default:
				break;
			}
		
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}

		LRESULT CALLBACK HandleEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (uMsg == WM_CREATE)
			{
				CREATESTRUCT* creationParameters = reinterpret_cast<CREATESTRUCT*>(lParam);

				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)creationParameters->lpCreateParams);
			}

			GraphicsWindow* window = GraphicsWindow::GetWindow(hWnd);

			if (window)
				return window->HandleEvents(hWnd, uMsg, wParam, lParam);

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		GraphicsWindow::WindowMap GraphicsWindow::WindowMappings = WindowMap();

		GraphicsWindow::GraphicsWindow()
		{
			StagingTexture = Engine::Create<Texture>();
		}

		void GraphicsWindow::InitializeWindow(std::shared_ptr<GraphicsContext>& context, int selectedDevice)
		{
			ApplicationHandle = GetModuleHandle(NULL);

			WNDCLASSEX win_class;

			// Initialize the window class structure:
			win_class.cbSize = sizeof(WNDCLASSEX);
			win_class.style = CS_HREDRAW | CS_VREDRAW;
			win_class.lpfnWndProc = Graphics::HandleEvents;
			win_class.cbClsExtra = 0;
			win_class.cbWndExtra = sizeof(LONG_PTR);
			win_class.hInstance = ApplicationHandle;  // hInstance
			win_class.hIcon = nullptr;
			win_class.hCursor = nullptr;
			win_class.hbrBackground = nullptr;
			win_class.lpszMenuName = nullptr;
			win_class.lpszClassName = "Ray Tracing Demo";
			win_class.hIconSm = nullptr;

			RegisterClassEx(&win_class);

			WindowHandle = CreateWindowEx(0,
				"Ray Tracing Demo",				  // class name
				"Ray Tracing Demo",				  // app name
				WS_OVERLAPPEDWINDOW |  // window style
				WS_VISIBLE | WS_SYSMENU,
				100, 100,			// x/y coords
				Resolution.X,  // width
				Resolution.Y,  // height
				nullptr,			 // handle to parent
				nullptr,			 // handle to menu
				ApplicationHandle,		  // hInstance
				this
			);			// no extra parameters

			if (!WindowHandle) {
				// It didn't work, so try to give a useful error:
				std::cout << "Cannot create a window in which to draw!" << std::endl;
				exit(1);
			}

			InitializedWindow = true;

			Context = context;

			Graphics::DeviceContext& deviceContext = context->GetDevice(selectedDevice);

			InitializeSurface(context, deviceContext);
		}

		void GraphicsWindow::CreateSurface()
		{
			if (!InitializedWindow) return;
			if (InitializedSurface) return;

			std::shared_ptr<GraphicsContext> context = Context.lock();

			if (!context) return;

			Graphics::DeviceContext& deviceContext = context->GetDevice(SelectedDevice);

			InitializeSurface(context, deviceContext);
		}

		void GraphicsWindow::InitializeSurface(std::shared_ptr<GraphicsContext>& context, DeviceContext& deviceContext)
		{
			vk::Win32SurfaceCreateInfoKHR createInfo = vk::Win32SurfaceCreateInfoKHR().setHinstance(ApplicationHandle).setHwnd(WindowHandle);

			VK_CALL(context->GetInstance().createWin32SurfaceKHR, &createInfo, nullptr, &Surface);

			QueueSupportsPresenting.resize(deviceContext.GpuQueueProperties.size());

			for (uint32_t i = 0; i < deviceContext.GpuQueueProperties.size(); i++)
				VK_CALL(deviceContext.Gpu->getSurfaceSupportKHR, i, Surface, &QueueSupportsPresenting[i]);

			SeparatePresentQueue = deviceContext.SeparatePresentQueue;
			GraphicsQueueFamilyIndex = deviceContext.GraphicsQueueFamilyIndex;
			PresentQueueFamilyIndex = deviceContext.PresentQueueFamilyIndex;

			InitializedSurface = true;
		}

		vk::Queue& GraphicsWindow::GetGraphicsQueue() { return GetContext()->GetDevice().GraphicsQueue; }
		vk::Queue& GraphicsWindow::GetPresentQueue() { return GetContext()->GetDevice().PresentQueue; }

		void GraphicsWindow::CleanUpSurface()
		{
			if (!InitializedWindow) return;
			if (!InitializedSurface) return;

			std::shared_ptr<GraphicsContext> context = Context.lock();

			if (!context) return;

			if (ViewSwapChain != nullptr)
				ViewSwapChain->FreeData();

			context->GetInstance().destroySurfaceKHR(Surface);

			InitializedSurface = false;
		}

		void GraphicsWindow::ReinitializeSurface()
		{
			CleanUpSurface();
			CreateSurface();
		}

		void GraphicsWindow::CleanUpWindow()
		{
			if (!InitializedWindow) return;

			VK_CALL(GetDevice().waitIdle);

			ReleaseRenderState();

			if (InitializedSurface)
				CleanUpSurface();

			DestroyWindow(WindowHandle);

			InitializedWindow = false;
		}

		DeviceContext* GraphicsWindow::GetGpu()
		{
			if (Context.expired()) return nullptr;

			std::shared_ptr<GraphicsContext> context = Context.lock();

			if (context)
				return &context->GetDevice(SelectedDevice);

			return nullptr;
		}

		const vk::SurfaceCapabilitiesKHR& GraphicsWindow::GetCapabilities(bool refresh)
		{
			if (refresh)
			{
				std::shared_ptr<GraphicsContext> context = Context.lock();

				if (context)
				{
					VK_CALL(context->GetDevice(SelectedDevice).Gpu->getSurfaceCapabilitiesKHR, Surface, &SurfaceCapabilities);
				}
			}

			return SurfaceCapabilities;
		}

		const GraphicsWindow::PresentModeVector& GraphicsWindow::GetPresentModes(bool refresh)
		{
			if (refresh)
			{
				std::shared_ptr<GraphicsContext> context = Context.lock();

				if (context)
				{
					uint32_t presentModeCount;

					VK_CALL(context->GetDevice(SelectedDevice).Gpu->getSurfacePresentModesKHR, Surface, &presentModeCount, nullptr);

					PresentModes.resize(presentModeCount);

					VK_CALL(context->GetDevice(SelectedDevice).Gpu->getSurfacePresentModesKHR, Surface, &presentModeCount, PresentModes.data());
				}

				PresentMode = vk::PresentModeKHR::eFifo;

				if (PresentMode != DesiredPresentMode)
				{
					for (size_t i = 0; i < PresentModes.size(); ++i)
					{
						if (PresentModes[i] == DesiredPresentMode)
						{
							PresentMode = DesiredPresentMode;

							break;
						}
					}
				}
			}

			return PresentModes;
		}

		const GraphicsWindow::SurfaceFormatVector& GraphicsWindow::GetSurfaceFormats(bool refresh)
		{
			if (refresh)
			{
				std::shared_ptr<GraphicsContext> context = Context.lock();

				if (context)
				{
					uint32_t surfaceFormatCount;

					VK_CALL(context->GetDevice(SelectedDevice).Gpu->getSurfaceFormatsKHR, Surface, &surfaceFormatCount, nullptr);

					SurfaceFormats.resize(surfaceFormatCount);

					VK_CALL(context->GetDevice(SelectedDevice).Gpu->getSurfaceFormatsKHR, Surface, &surfaceFormatCount, SurfaceFormats.data());

					if (SurfaceFormats.size() == 1 && SurfaceFormats[0].format == vk::Format::eUndefined)
						SurfaceFormat = vk::Format::eB8G8R8A8Unorm;
					else
					{
						assert(SurfaceFormats.size() >= 1);
						SurfaceFormat = SurfaceFormats[0].format;
					}

					SurfaceColorSpace = SurfaceFormats[0].colorSpace;
				}
			}

			return SurfaceFormats;
		}

		GraphicsWindow::~GraphicsWindow()
		{
			CleanUpWindow();
		}

		void GraphicsWindow::FreeData()
		{
			CleanUpWindow();
		}

		void GraphicsWindow::SetResolution(const Vector2I& resolution)
		{
			Resolution = resolution;
		}

		void GraphicsWindow::InitializeRenderState()
		{
			InitializeCommandPool();

			InitializeSwapChain();
		}

		void GraphicsWindow::InitializeSwapChain()
		{
			if (ViewSwapChain == nullptr)
			{
				ViewSwapChain = Engine::Create<Graphics::SwapChain>();
				ViewSwapChain->AttachToContext(GetContext());
			}

			GetSurfaceFormats(true);

			ViewSwapChain->Window = this;
			ViewSwapChain->InitializeCommandBuffers();
			ViewSwapChain->InitializeFences(This.lock()->Cast<GraphicsWindow>());
			ViewSwapChain->InitializeImages(This.lock()->Cast<GraphicsWindow>());
		}

		void GraphicsWindow::ReleaseRenderState()
		{
			ViewSwapChain->ReleaseData();
		}

		void GraphicsWindow::InitializeCommandPool()
		{
			CommandPool = GetContext()->GetDevice().CommandPool;
			PresentCommandPool = GetContext()->GetDevice().PresentCommandPool;

			if (InitializedCommandBuffer)
				return;

			auto const cmd = vk::CommandBufferAllocateInfo()
				.setCommandPool(CommandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

			VK_CALL(GetDevice().allocateCommandBuffers, &cmd, &CommandBuffer);

			auto const cmd_buf_info = vk::CommandBufferBeginInfo().setPInheritanceInfo(nullptr);

			VK_CALL(CommandBuffer.begin, &cmd_buf_info);

			InitializedCommandBuffer = true;
		}

		void GraphicsWindow::FlushCommands()
		{
			if (!CommandBuffer)
				return;

			VK_CALL(CommandBuffer.end);

			auto const fenceInfo = vk::FenceCreateInfo();
			vk::Fence fence;
			VK_CALL(GetDevice().createFence, &fenceInfo, nullptr, &fence);

			vk::CommandBuffer const commandBuffers[] = { CommandBuffer };
			auto const submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(commandBuffers);

			VK_CALL(GetGraphicsQueue().submit, 1, &submitInfo, fence);

			VK_CALL(GetDevice().waitForFences, 1, &fence, VK_TRUE, UINT64_MAX);

			GetDevice().freeCommandBuffers(CommandPool, 1, commandBuffers);
			GetDevice().destroyFence(fence, nullptr);

			CommandBuffer = vk::CommandBuffer();

			ViewSwapChain->SetCurrentBuffer(0);

			InitializedCommandBuffer = false;
		}

		GraphicsWindow* GraphicsWindow::GetWindow(HWND handle)
		{
			LONG_PTR windowPointer = GetWindowLongPtr(handle, GWLP_USERDATA);

			return reinterpret_cast<GraphicsWindow*>(windowPointer);
		}

		LRESULT GraphicsWindow::HandleEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			//if (EventHandler)
			return ProcessWindowEvent(hWnd, uMsg, wParam, lParam);

			std::cout << "bound window has no callback!" << std::endl;
			exit(1);

			return 0;
		}
	}
}