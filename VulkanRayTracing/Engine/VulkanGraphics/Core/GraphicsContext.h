#pragma once

import <vector>;
import <string>;
import <source_location>;

#include "VulkanSupport.h"
#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class DeviceResource;

		struct DeviceContext
		{
			typedef std::vector<const char*> StringBuffer;
			typedef std::vector<vk::QueueFamilyProperties> QueuePropertyVector;
			typedef std::vector<vk::Bool32> FlagVector;

			bool Initialized = false;
			int Index = -1;
			StringBuffer Extensions;
			vk::Device Device;
			vk::CommandPool CommandPool;
			vk::CommandPool PresentCommandPool;
			vk::PhysicalDevice* Gpu = nullptr;
			vk::PhysicalDeviceProperties GpuProperties;
			vk::PhysicalDeviceFeatures GpuFeatures;
			vk::PhysicalDeviceMemoryProperties GpuMemoryProperties;
			vk::PhysicalDeviceDescriptorIndexingFeaturesEXT DescriptorIndexingFeatures;
			vk::Queue GraphicsQueue;
			vk::Queue PresentQueue;
			QueuePropertyVector GpuQueueProperties;
			uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
			uint32_t PresentQueueFamilyIndex = UINT32_MAX;
			bool SeparatePresentQueue = false;
			FlagVector QueueSupportsPresenting;

			void Update();
			void FindQueueFamilies();
			void InitializeDevice();
			void InitializeQueues();
			void InitializeCommandPool();
			void Initialize();
			void Destroy();

			bool MeetsMemoryRequirements(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t* typeIndex) const;
		};

		class GraphicsContext : public Object
		{
		public:
			~GraphicsContext();

			void SetUp();
			void CleanUpInstance();

			vk::Instance& GetInstance() { return VulkanInstance; }
			int GetDevices() const;
			DeviceContext& GetDevice(int index = -1);
			const DeviceContext& GetDevice(int index = -1) const;
			void UpdateDevice(int index = -1);
			int GetPrimaryDevice() const { return PrimaryDevice; }

			void WatchResource(DeviceResource* resource);
			void ResourceDied(DeviceResource* resource);

			bool HasSeparatePresentQueue(int index = -1) const { return GetDevice(index).SeparatePresentQueue; }
			uint32_t GetGraphicsQueueFamilyIndex(int index = -1) const { return GetDevice(index).GraphicsQueueFamilyIndex; }
			uint32_t GetPresentQueueFamilyIndex(int index = -1) const { return GetDevice(index).PresentQueueFamilyIndex; }

		private:
			typedef std::vector<std::string> StringVector;
			typedef std::vector<const char*> StringBuffer;
			typedef std::vector<DeviceContext> DeviceContextVector;
			typedef std::vector<vk::PhysicalDevice> DeviceVector;
			typedef std::vector<DeviceResource*> ResourceVector;

			ResourceVector Resources;
			StringBuffer LayerPropertiesBuffer;
			StringBuffer DeviceExtensionsBuffer;
			StringBuffer InstanceExtensionsBuffer;
			StringVector LayerProperties = { "VK_LAYER_KHRONOS_validation" };
			StringVector DeviceExtensions = { "VK_KHR_swapchain", "VK_KHR_maintenance3", "VK_EXT_descriptor_indexing" };
			StringVector InstanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_KHR_get_physical_device_properties2" };

			int PrimaryDevice = 0;
			bool InitializedInstance = false;
			vk::Instance VulkanInstance;

			DeviceVector Devices;
			DeviceContextVector DeviceContexts;

			void InitializeLayers();
			void InitializeExtensions();
			void InitializeDevices();
			void InitializeDevice(int index);
			void CopyBuffer(StringBuffer& buffer, const StringVector& vector);
		};
	}
}