#pragma once

#include <Engine/Objects/Object.h>

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

#include "DeviceResource.h"

namespace Engine
{
	namespace Graphics
	{
		class Uniform : public DeviceResource
		{
		public:
			Uniform(uint32_t binding) : Binding(binding) {}

			virtual size_t GetSize() const { return 0; }
			virtual vk::DescriptorType GetType() const { return vk::DescriptorType::eMutableVALVE; }
			virtual const vk::DescriptorImageInfo* GetImageInfo() const { return nullptr; }
			virtual const vk::DescriptorBufferInfo* GetBufferInfo() const { return nullptr; }
			virtual const vk::BufferView* GetBufferViews() const { return nullptr; }

			void ClearWrittenThisFrame();
			void MarkStale();
			void ClearStale();
			bool IsStale() const { return Stale; }
			bool IsWrittenThisFrame() const { return WrittenThisFrame; }
			uint32_t GetBinding() const { return Binding; }

		private:
			uint32_t Binding = 0;
			bool Stale = false;
			bool WrittenThisFrame = false;
		};

		class Texture;

		class CombinedSamplerUniform : public Uniform
		{
		public:
			CombinedSamplerUniform(uint32_t binding) : Uniform(binding) {}

			void SetCount(size_t index);
			void Set(size_t index, const std::shared_ptr<Texture>& texture, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

			virtual size_t GetSize() const { return SamplerInfo.size(); }
			virtual vk::DescriptorType GetType() const { return vk::DescriptorType::eCombinedImageSampler; }
			virtual const vk::DescriptorImageInfo* GetImageInfo() const { return SamplerInfo.data(); }

		private:
			std::vector<vk::DescriptorImageInfo> SamplerInfo;
		};

		class BufferObjectUniform : public Uniform
		{
		public:
			BufferObjectUniform(uint32_t binding);
			~BufferObjectUniform();

			struct SceneInfo
			{
				float Time = 0;
			};

			SceneInfo Data;

			void InitializeData();
			void UpdateData();

			virtual void FreeData();

			virtual size_t GetSize() const { return 1; }
			virtual vk::DescriptorType GetType() const { return vk::DescriptorType::eUniformBuffer; }
			virtual const vk::DescriptorBufferInfo* GetBufferInfo() const { return &BufferInfo; }

		private:
			bool Initialized = false;
			vk::Buffer Buffer;
			vk::DeviceMemory Memory;
			vk::DescriptorBufferInfo BufferInfo;
			void* Pointer = nullptr;
		};
	}
}