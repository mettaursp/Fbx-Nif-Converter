#pragma once

#include <Windows.h>
import <memory>;
import <map>;
import <vector>;

#include <Engine/IdentifierHeap.h>
#include <Engine/Math/Vector2.h>
#include <Engine/Objects/Object.h>

#include "VulkanSupport.h"
#include "DeviceResource.h"

namespace Engine
{
	namespace Graphics
	{
		class GraphicsContext;
		struct DeviceContext;
		class Texture;
		class SwapChain;

		class GraphicsWindow : public DeviceResource
		{
		public:
			typedef LRESULT(*WindowEventCallback)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			typedef std::vector<vk::PresentModeKHR> PresentModeVector;
			typedef std::vector<vk::SurfaceFormatKHR> SurfaceFormatVector;

			//WindowEventCallback EventHandler = nullptr;

			vk::CommandBuffer CommandBuffer = nullptr;
			vk::CommandPool CommandPool;
			vk::CommandPool PresentCommandPool;

			std::shared_ptr<SwapChain> ViewSwapChain;

			GraphicsWindow();
			~GraphicsWindow();

			virtual void FreeData();

			void InitializeWindow(std::shared_ptr<GraphicsContext>& context, int selectedDevice = -1);

			void CreateSurface();
			void CleanUpSurface();
			void CleanUpWindow();
			void ReinitializeSurface();

			std::shared_ptr<GraphicsContext> GetContext() { return Context.lock(); }
			std::shared_ptr<Texture>& GetStagingTexture() { return StagingTexture; }

			HINSTANCE GetApplicationHandle() { return ApplicationHandle; }
			HWND GetWindowHandle() { return WindowHandle; }

			bool HasSeparatePresentQueue() const { return SeparatePresentQueue; }
			uint32_t GetGraphicsQueueFamilyIndex() const { return GraphicsQueueFamilyIndex; }
			uint32_t GetPresentQueueFamilyIndex() const { return PresentQueueFamilyIndex; }

			vk::CommandBuffer& GetCommandBuffer() { return CommandBuffer; }
			DeviceContext* GetGpu();
			int GetSelectedDevice() const { return SelectedDevice; }
			vk::SurfaceKHR& GetSurface() { return Surface; }
			vk::Queue& GetGraphicsQueue();
			vk::Queue& GetPresentQueue();
			const vk::SurfaceCapabilitiesKHR& GetCapabilities(bool refresh = false);
			const PresentModeVector& GetPresentModes(bool refresh = false);
			const SurfaceFormatVector& GetSurfaceFormats(bool refresh = false);
			vk::PresentModeKHR GetPresentMode() { return PresentMode; }
			vk::Format GetSurfaceFormat() { return SurfaceFormat; }
			vk::ColorSpaceKHR GetSurfaceColorSpace() { return SurfaceColorSpace; }
			const Vector2I& GetResolution() const { return Resolution; }
			void SetResolution(const Vector2I& resolution);

			void InitializeRenderState();
			void InitializeSwapChain();
			void ReleaseRenderState();
			void InitializeCommandPool();
			void FlushCommands();

			static GraphicsWindow* GetWindow(HWND handle);

			LRESULT CALLBACK HandleEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT CALLBACK ProcessWindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			typedef std::map<HWND, GraphicsWindow*> WindowMap;
			typedef std::vector<vk::Bool32> FlagVector;

			bool InitializedCommandBuffer = false;
			bool InitializedWindow = false;
			bool InitializedSurface = false;
			HINSTANCE ApplicationHandle;
			HWND WindowHandle;

			std::shared_ptr<Texture> StagingTexture;
			std::weak_ptr<GraphicsContext> Context;
			int SelectedDevice = -1;
			Vector2I Resolution = Vector2I(1000, 1000);

			vk::SurfaceKHR Surface;
			vk::Format SurfaceFormat;
			vk::ColorSpaceKHR SurfaceColorSpace;
			vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;

			vk::PresentModeKHR DesiredPresentMode = vk::PresentModeKHR::eFifo;

			bool SeparatePresentQueue = false;
			uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
			uint32_t PresentQueueFamilyIndex = UINT32_MAX;
			FlagVector QueueSupportsPresenting;
			vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
			PresentModeVector PresentModes;
			SurfaceFormatVector SurfaceFormats;

			void InitializeSurface(std::shared_ptr<GraphicsContext>& context, DeviceContext& deviceContext);

			static WindowMap WindowMappings;
		};
	}
}