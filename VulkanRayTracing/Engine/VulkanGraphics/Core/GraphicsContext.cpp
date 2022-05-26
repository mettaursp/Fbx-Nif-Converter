#include "GraphicsContext.h"

import <iostream>;
import <set>;

#include "VulkanErrorHandling.h"
#include "DeviceResource.h"

namespace Engine
{
	namespace Graphics
	{
		template <typename PropertyType>
		struct ArrayType
		{
			typedef std::function<vk::Result(uint32_t* arraySize, PropertyType* data)> Callback;
			typedef vk::ArrayWrapper1D<char, 256Ui64> PropertyType::* NameMember;
		};

		template <typename PropertyType>
		void ValidateProperties(const char** data, int size, typename ArrayType<PropertyType>::NameMember nameMember, const char* ownerName, const char* typeName, typename ArrayType<PropertyType>::Callback callback);

		void DeviceContext::Update()
		{
			Gpu->getMemoryProperties(&GpuMemoryProperties);
		}

		bool DeviceContext::MeetsMemoryRequirements(uint32_t typeBits, vk::MemoryPropertyFlags requirements_mask, uint32_t* typeIndex) const
		{
			// Search memtypes to find first index with those properties
			for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
				if ((typeBits & 1) == 1) {
					// Type is available, does it match user properties?
					if ((GpuMemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
						*typeIndex = i;
						return true;
					}
				}
				typeBits >>= 1;
			}

			// No memory types matched, return failure
			return false;
		}

		void DeviceContext::FindQueueFamilies()
		{
			QueueSupportsPresenting.resize(GpuQueueProperties.size());

			for (uint32_t i = 0; i < GpuQueueProperties.size(); i++)
				QueueSupportsPresenting[i] = Gpu->getWin32PresentationSupportKHR(i);

			for (uint32_t i = 0; i < GpuQueueProperties.size(); i++)
			{
				if (GpuQueueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
				{
					if (GraphicsQueueFamilyIndex == UINT32_MAX)
						GraphicsQueueFamilyIndex = i;

					if (QueueSupportsPresenting[i] == VK_TRUE)
					{
						GraphicsQueueFamilyIndex = i;
						PresentQueueFamilyIndex = i;

						break;
					}
				}
			}

			if (PresentQueueFamilyIndex == UINT32_MAX)
			{
				for (uint32_t i = 0; i < GpuQueueProperties.size(); ++i)
				{
					if (QueueSupportsPresenting[i] == VK_TRUE)
					{
						PresentQueueFamilyIndex = i;

						break;
					}
				}
			}

			if (GraphicsQueueFamilyIndex == UINT32_MAX || PresentQueueFamilyIndex == UINT32_MAX)
				throw VulkanException(vk::Result::eErrorUnknown, "GraphicsWindow::FindQueueFamilies", "Could not find both graphics and present queues, swapchain initialization failure");

			SeparatePresentQueue = GraphicsQueueFamilyIndex != PresentQueueFamilyIndex;
		}

		void DeviceContext::InitializeDevice()
		{
			float const priorities[1] = { 0.0 };

			vk::DeviceQueueCreateInfo queues[2];
			queues[0].setQueueFamilyIndex(GraphicsQueueFamilyIndex);
			queues[0].setQueueCount(1);
			queues[0].setPQueuePriorities(priorities);

			vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo();

			deviceInfo.setQueueCreateInfoCount(1);
			deviceInfo.setPQueueCreateInfos(queues);
			deviceInfo.setEnabledLayerCount(0);
			deviceInfo.setPpEnabledLayerNames(nullptr);
			deviceInfo.setEnabledExtensionCount(uint32_t(Extensions.size()) - 1);
			deviceInfo.setPpEnabledExtensionNames((const char* const*)Extensions.data());
			deviceInfo.setPEnabledFeatures(nullptr);

			if (SeparatePresentQueue)
			{
				queues[1].setQueueFamilyIndex(PresentQueueFamilyIndex);
				queues[1].setQueueCount(1);
				queues[1].setPQueuePriorities(priorities);
				deviceInfo.setQueueCreateInfoCount(2);
			}

			VK_CALL(Gpu->createDevice, &deviceInfo, nullptr, &Device);
		}

		void DeviceContext::InitializeQueues()
		{
			Device.getQueue(GraphicsQueueFamilyIndex, 0, &GraphicsQueue);

			if (!SeparatePresentQueue)
				PresentQueue = GraphicsQueue;
			else
				Device.getQueue(PresentQueueFamilyIndex, 0, &PresentQueue);
		}

		void DeviceContext::InitializeCommandPool()
		{
			auto const cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex(GraphicsQueueFamilyIndex).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
			VK_CALL(Device.createCommandPool, &cmd_pool_info, nullptr, &CommandPool);

			if (SeparatePresentQueue) {
				auto const present_cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex(PresentQueueFamilyIndex).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

				VK_CALL(Device.createCommandPool, &present_cmd_pool_info, nullptr, &PresentCommandPool);
			}
		}

		void DeviceContext::Initialize()
		{
			Update();

			if (Initialized)
				return;

			FindQueueFamilies();
			InitializeDevice();
			InitializeQueues();
			InitializeCommandPool();

			Initialized = true;
		}

		void DeviceContext::Destroy()
		{
			if (!Initialized) return;

			Device.destroyCommandPool(CommandPool, nullptr);

			if (SeparatePresentQueue)
				Device.destroyCommandPool(PresentCommandPool, nullptr);

			Device.destroy(nullptr);
		}

		void GraphicsContext::UpdateDevice(int index)
		{
			DeviceContext& deviceContext = GetDevice(index);

			deviceContext.Initialize();
		}

		void GraphicsContext::SetUp()
		{
			vk::ApplicationInfo application;
			application.setPApplicationName("vkray");
			application.setApplicationVersion(0);
			application.setPEngineName("vkray");
			application.setEngineVersion(0);
			application.setApiVersion(VK_API_VERSION_1_0);

			InitializeLayers();
			InitializeExtensions();

			vk::InstanceCreateInfo instanceInfo;
			instanceInfo.setPApplicationInfo(&application);
			instanceInfo.setEnabledLayerCount(unsigned int(LayerPropertiesBuffer.size()) - 1);
			instanceInfo.setPpEnabledLayerNames(LayerPropertiesBuffer.data());
			instanceInfo.setEnabledExtensionCount(unsigned int(InstanceExtensionsBuffer.size()) - 1);
			instanceInfo.setPpEnabledExtensionNames(InstanceExtensionsBuffer.data());

			VK_CALL(vk::createInstance, &instanceInfo, nullptr, &VulkanInstance);

			InitializeDevices();

			UpdateDevice();

			InitializedInstance = true;
		}

		void GraphicsContext::CleanUpInstance()
		{
			if (!InitializedInstance) return;

			for (size_t i = 0; i < Resources.size(); ++i)
				Resources[i]->DeviceDestroyed();

			Resources.clear();

			for (size_t i = 0; i < DeviceContexts.size(); ++i)
				if (DeviceContexts[i].Initialized)
					DeviceContexts[i].Destroy();

			VulkanInstance.destroy(nullptr);

			InitializedInstance = false;
		}

		GraphicsContext::~GraphicsContext()
		{
			CleanUpInstance();
		}

		void GraphicsContext::InitializeLayers()
		{
			CopyBuffer(LayerPropertiesBuffer, LayerProperties);

			ValidateProperties<vk::LayerProperties>(LayerPropertiesBuffer.data(), int(LayerPropertiesBuffer.size()) - 1, &vk::LayerProperties::layerName, "Instance", "Layers", [](uint32_t* arraySize, vk::LayerProperties* data) -> vk::Result
				{
					return vk::enumerateInstanceLayerProperties(arraySize, data);
				});
		}

		void GraphicsContext::InitializeExtensions()
		{
			CopyBuffer(InstanceExtensionsBuffer, InstanceExtensions);

			ValidateProperties<vk::ExtensionProperties>(InstanceExtensionsBuffer.data(), int(InstanceExtensionsBuffer.size()) - 1, &vk::ExtensionProperties::extensionName, "Instance", "Extensions", [](uint32_t* arraySize, vk::ExtensionProperties* data) -> vk::Result
				{
					return vk::enumerateInstanceExtensionProperties(nullptr, arraySize, data);
				});
		}

		void GraphicsContext::InitializeDevices()
		{
			uint32_t gpuCount;
			VK_CALL(VulkanInstance.enumeratePhysicalDevices, &gpuCount, nullptr);

			Devices.resize(gpuCount);
			DeviceContexts.resize(gpuCount);

			VK_CALL(VulkanInstance.enumeratePhysicalDevices, &gpuCount, Devices.data());

			std::cout << "found devices:" << std::endl;

			for (size_t i = 0; i < gpuCount; ++i)
				std::cout << "\t" << Devices[i].getProperties().deviceName.data() << std::endl;

			InitializeDevice(PrimaryDevice);
		}

		void GraphicsContext::InitializeDevice(int index)
		{
			vk::PhysicalDevice& gpu = Devices[index];
			DeviceContexts[index].Gpu = &gpu;

			gpu.getProperties(&DeviceContexts[index].GpuProperties);
			gpu.getFeatures(&DeviceContexts[index].GpuFeatures);

			CopyBuffer(DeviceContexts[index].Extensions, DeviceExtensions);

			ValidateProperties<vk::ExtensionProperties>(DeviceContexts[index].Extensions.data(), int(DeviceContexts[index].Extensions.size()) - 1, &vk::ExtensionProperties::extensionName, "Device", "Extension", [&gpu](uint32_t* arraySize, vk::ExtensionProperties* data) -> vk::Result
				{
					return gpu.enumerateDeviceExtensionProperties(nullptr, arraySize, data);
				});

			uint32_t propertyCount;

			gpu.getQueueFamilyProperties(&propertyCount, nullptr);

			DeviceContexts[index].GpuQueueProperties.resize(propertyCount);
			DeviceContexts[index].Index = index;

			gpu.getQueueFamilyProperties(&propertyCount, DeviceContexts[index].GpuQueueProperties.data());
		}

		int GraphicsContext::GetDevices() const
		{
			return int(DeviceContexts.size());
		}

		DeviceContext& GraphicsContext::GetDevice(int index)
		{
			if (index < 0)
				return DeviceContexts[PrimaryDevice];

			return DeviceContexts[index];
		}

		const DeviceContext& GraphicsContext::GetDevice(int index) const
		{
			if (index < 0)
				return DeviceContexts[PrimaryDevice];

			return DeviceContexts[index];
		}

		void GraphicsContext::CopyBuffer(StringBuffer& buffer, const StringVector& vector)
		{
			int count = int(vector.size());

			buffer.resize(count + 1);

			buffer.back() = nullptr;

			for (int i = 0; i < count; ++i)
				buffer[i] = vector[i].c_str();
		}

		void GraphicsContext::WatchResource(DeviceResource* resource)
		{
			resource->ArrayIndex = Resources.size();
			Resources.push_back(resource);
		}

		void GraphicsContext::ResourceDied(DeviceResource* resource)
		{
			Resources.back()->ArrayIndex = resource->ArrayIndex;
			Resources[resource->ArrayIndex] = Resources.back();
			Resources.pop_back();
		}

		template <typename PropertyType>
		void ValidateProperties(const char** data, int size, typename ArrayType<PropertyType>::NameMember nameMember, const char* ownerName, const char* typeName, typename ArrayType<PropertyType>::Callback callback)
		{
			std::string debugFunctionName("enumerate");
			debugFunctionName += ownerName;
			debugFunctionName += typeName;

			uint32_t count;

			CheckError(callback(&count, nullptr), debugFunctionName.c_str());

			if (count > 0)
			{
				std::set<std::string> desired;

				for (int i = 0; i < size; ++i)
					desired.insert(data[i]);

				std::unique_ptr<PropertyType[]> fetched(new PropertyType[count]);

				CheckError(callback(&count, fetched.get()), debugFunctionName.c_str());

				std::cout << "supported " << ownerName << " " << typeName << "s:" << std::endl;

				for (size_t i = 0; i < count; ++i)
				{
					std::cout << "\t" << fetched[i].*nameMember << std::endl;

					auto name = desired.find(fetched[i].*nameMember);

					if (name != desired.end())
						desired.erase(name);
				}

				if (desired.size() > 0)
				{
					std::cout << "failed to find desired " << ownerName << " " << typeName << "s:" << std::endl;

					for (auto name = desired.begin(); name != desired.end(); ++name)
						std::cout << "\t" << *name << std::endl;

					throw VulkanException(vk::Result::eErrorExtensionNotPresent, debugFunctionName.c_str(), "properties not found");
				}
			}
		}
	}
}