#pragma once

import <memory>;

#include "VulkanSupport.h"
#include "DeviceResource.h"
#include <Engine/Math/Vector2.h>

namespace Engine
{
	namespace Graphics
	{
		class ImageResource;
		class GraphicsWindow;

		class Texture : public DeviceResource
		{
		public:
			~Texture();

			virtual void FreeData();

			void LoadResource(const std::shared_ptr<ImageResource>& resource, const std::shared_ptr<GraphicsWindow>& targetWindow = nullptr);
			
			void InitializeDepth(const std::shared_ptr<GraphicsWindow>& targetWindow);

			void InitializeViewFromImage(const vk::Image& image, const Vector2I& size, const std::shared_ptr<GraphicsWindow>& targetWindow);

			void ReleaseResources();
			void InitializeSampler();

			const Vector2I& GetSize() const { return Size; }

			vk::Sampler& GetSampler() { return Sampler; }
			vk::ImageView& GetImageView() { return ImageView; }
			vk::Format GetFormat() const { return Format; }
			vk::Image& GetImage() { return Image; }

		private:
			bool BufferInitialized = false;
			bool ImageInitialized = false;
			bool MemoryInitialized = false;
			bool ViewInitialized = false;
			bool SamplerInitialized = false;

			vk::Format Format = vk::Format::eR8G8B8A8Unorm;
			Vector2I Size;
			vk::DeviceSize MemoryAllocated;
			vk::ImageLayout ImageLayout = vk::ImageLayout::eUndefined;

			vk::Sampler Sampler;
			vk::Buffer Buffer;
			vk::Image Image;
			vk::ImageView ImageView;

			vk::MemoryAllocateInfo MemoryAllocateInfo;
			vk::DeviceMemory Memory;

			void LoadBufferResource(const std::shared_ptr<ImageResource>& resource);
			void LoadImageResource(const std::shared_ptr<ImageResource>& resource, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags requirements);
			void SetImageLayout(vk::CommandBuffer& cmdBuffer, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::PipelineStageFlags src_stages, vk::PipelineStageFlags dest_stages);

			void AllocateMemory(vk::Device device, const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags requirements);
			bool VerifyMemoryType(const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags requirements);
		};
	}
}