#include "Texture.h"

#include <vulkan/vulkan_funcs.hpp>

#include "VulkanErrorHandling.h"
#include "ImageResource.h"
#include "GraphicsContext.h"
#include "GraphicsWindow.h"

namespace Engine
{
	namespace Graphics
	{
		Texture::~Texture()
		{
			ReleaseResources();
		}

		void Texture::FreeData()
		{
			ReleaseResources();
		}

		void Texture::LoadResource(const std::shared_ptr<ImageResource>& resource, const std::shared_ptr<GraphicsWindow>& targetWindow)
		{
			DeviceContext* context = &GetContext()->GetDevice();

			if (!context) return;

			vk::Device device = GetDevice();

			Format = vk::Format::eR8G8B8A8Unorm;

			vk::FormatProperties formatProperties;

			context->Gpu->getFormatProperties(Format, &formatProperties);

			std::shared_ptr<Texture> stagingTexture = targetWindow->GetStagingTexture();
			vk::CommandBuffer& cmdBuffer = targetWindow->GetCommandBuffer();

			ReleaseResources();
			InitializeSampler();

			if ((formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) && stagingTexture != nullptr) {

				LoadImageResource(resource, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);


				SetImageLayout(cmdBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::ePreinitialized,
					ImageLayout, vk::AccessFlagBits(), vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eFragmentShader);

				stagingTexture->Image = vk::Image();
			}
			else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
				/* Must use staging buffer to copy linear texture to optimized */

				LoadBufferResource(resource);
				LoadImageResource(resource, vk::ImageTiling::eOptimal,
					vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
					vk::MemoryPropertyFlagBits::eDeviceLocal);

				SetImageLayout(cmdBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::ePreinitialized,
					vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits(), vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eTransfer);

				vk::ImageSubresourceLayers subresource = vk::ImageSubresourceLayers();

				subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				subresource.setMipLevel(0);
				subresource.setBaseArrayLayer(0);
				subresource.setLayerCount(1);

				vk::BufferImageCopy copyRegion = vk::BufferImageCopy();

				copyRegion.setBufferOffset(0);
				copyRegion.setBufferRowLength(stagingTexture->Size.X);
				copyRegion.setBufferImageHeight(stagingTexture->Size.Y);
				copyRegion.setImageSubresource(subresource);
				copyRegion.setImageOffset({ 0, 0, 0 });
				copyRegion.setImageExtent({ (uint32_t)stagingTexture->Size.X, (uint32_t)stagingTexture->Size.Y, 1 });

				cmdBuffer.copyBufferToImage(stagingTexture->Buffer, Image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

				SetImageLayout(cmdBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal,
					ImageLayout, vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer,
					vk::PipelineStageFlagBits::eFragmentShader);
			}
			else {
				assert(!"No support for R8G8B8A8_UNORM as texture image format");
			}


			vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
			viewInfo.setImage(Image);
			viewInfo.setViewType(vk::ImageViewType::e2D);
			viewInfo.setFormat(Format);
			viewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

			VK_CALL(device.createImageView, &viewInfo, nullptr, &ImageView);

			ViewInitialized = true;
		}

		void Texture::InitializeDepth(const std::shared_ptr<GraphicsWindow>& targetWindow)
		{
			vk::Device device = GetDevice();

			Format = vk::Format::eD16Unorm;
			Size = targetWindow->GetResolution();

			ReleaseResources();
			InitializeSampler();

			const Vector2I& resolution = targetWindow->GetResolution();

			auto const image = vk::ImageCreateInfo()
				.setImageType(vk::ImageType::e2D)
				.setFormat(Format)
				.setExtent({ (uint32_t)resolution.X, (uint32_t)resolution.Y, 1 })
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setTiling(vk::ImageTiling::eOptimal)
				.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(0)
				.setPQueueFamilyIndices(nullptr)
				.setInitialLayout(vk::ImageLayout::eUndefined);

			VK_CALL(GetDevice().createImage, &image, nullptr, &Image);

			vk::MemoryRequirements mem_reqs;
			GetDevice().getImageMemoryRequirements(Image, &mem_reqs);

			AllocateMemory(device, mem_reqs, vk::MemoryPropertyFlagBits::eDeviceLocal);

			VK_CALL(GetDevice().bindImageMemory, Image, Memory, 0);

			auto const view = vk::ImageViewCreateInfo()
				.setImage(Image)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(Format)
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));

			VK_CALL(GetDevice().createImageView, &view, nullptr, &ImageView);

			ImageInitialized = true;
			ViewInitialized = true;
		}

		void Texture::InitializeViewFromImage(const vk::Image& image, const Vector2I& size, const std::shared_ptr<GraphicsWindow>& targetWindow)
		{
			ReleaseResources();
			InitializeSampler();

			auto color_image_view = vk::ImageViewCreateInfo()
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(targetWindow->GetSurfaceFormat())
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				.setImage(image);

			Image = image;
			Size = size;

			VK_CALL(GetDevice().createImageView, &color_image_view, nullptr, &ImageView);

			ViewInitialized = true;
		}

		void Texture::LoadBufferResource(const std::shared_ptr<ImageResource>& resource)
		{
			if (BufferInitialized) return;
			if (!resource) return;

			vk::Device device = GetDevice();

			if (device == vk::Device()) return;

			Size = resource->GetSize();

			vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
			bufferInfo.setSize(Size.X * Size.Y * 4);
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
			bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
			bufferInfo.setQueueFamilyIndexCount(0);
			bufferInfo.setPQueueFamilyIndices(nullptr);

			VK_CALL(device.createBuffer, &bufferInfo, nullptr, &Buffer);

			vk::MemoryRequirements memoryRequirements;
			device.getBufferMemoryRequirements(Buffer, &memoryRequirements);

			vk::MemoryPropertyFlags requirements = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

			AllocateMemory(device, memoryRequirements, requirements);

			VK_CALL(device.bindBufferMemory, Buffer, Memory, 0);

			vk::SubresourceLayout layout;
			layout.rowPitch = Size.X * 4;

			auto data = device.mapMemory(Memory, 0, MemoryAllocateInfo.allocationSize);

			CheckError(data.result, "device.mapMemory");

			resource->CopyData((uint8_t*)data.value, layout);

			device.unmapMemory(Memory);

			BufferInitialized = true;
		}

		void Texture::LoadImageResource(const std::shared_ptr<ImageResource>& resource, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags requirements)
		{
			if (ImageInitialized) return;
			if (!resource) return;

			vk::Device device = GetDevice();

			if (device == vk::Device()) return;

			Size = resource->GetSize();

			vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();

			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
			imageInfo.setExtent({ (uint32_t)Size.X, (uint32_t)Size.Y, 1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(1);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setTiling(tiling);
			imageInfo.setUsage(usage);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setQueueFamilyIndexCount(0);
			imageInfo.setPQueueFamilyIndices(nullptr);
			imageInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);

			VK_CALL(device.createImage, &imageInfo, nullptr, &Image);

			vk::MemoryRequirements memoryRequirements;
			device.getImageMemoryRequirements(Image, &memoryRequirements);

			AllocateMemory(device, memoryRequirements, requirements);

			VK_CALL(device.bindImageMemory, Image, Memory, 0);

			if (requirements & vk::MemoryPropertyFlagBits::eHostVisible) {
				vk::ImageSubresource subres = vk::ImageSubresource().setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setArrayLayer(0);
				vk::SubresourceLayout layout;

				device.getImageSubresourceLayout(Image, &subres, &layout);

				auto data = device.mapMemory(Memory, 0, MemoryAllocateInfo.allocationSize);

				CheckError(data.result, "device.mapMemory");

				resource->CopyData((uint8_t*)data.value, layout);

				device.unmapMemory(Memory);
			}

			ImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			ImageInitialized = true;
		}

		void Texture::InitializeSampler()
		{
			if (SamplerInitialized) return;

			vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo();

			samplerInfo.setMagFilter(vk::Filter::eNearest);
			samplerInfo.setMinFilter(vk::Filter::eNearest);
			samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
			samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
			samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
			samplerInfo.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
			samplerInfo.setMipLodBias(0.0f);
			samplerInfo.setAnisotropyEnable(VK_FALSE);
			samplerInfo.setMaxAnisotropy(1);
			samplerInfo.setCompareEnable(VK_FALSE);
			samplerInfo.setCompareOp(vk::CompareOp::eNever);
			samplerInfo.setMinLod(0.0f);
			samplerInfo.setMaxLod(0.0f);
			samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
			samplerInfo.setUnnormalizedCoordinates(VK_FALSE);

			VK_CALL(GetDevice().createSampler, &samplerInfo, nullptr, &Sampler);

			SamplerInitialized = true;
		}

		void Texture::ReleaseResources()
		{
			vk::Device device = GetDevice();

			if (GetContext() == nullptr) return;

			VK_CALL(device.waitIdle);

			if (SamplerInitialized)
				device.destroySampler(Sampler, nullptr);

			if (BufferInitialized)
				device.destroyBuffer(Buffer, nullptr);

			if (ImageInitialized)
				device.destroyImage(Image, nullptr);

			if (MemoryAllocated)
				device.freeMemory(Memory, nullptr);

			if (ViewInitialized)
				device.destroyImageView(ImageView, nullptr);

			BufferInitialized = false;
			ImageInitialized = false;
			MemoryAllocated = false;
			ViewInitialized = false;
			SamplerInitialized = false;
		}

		void Texture::SetImageLayout(vk::CommandBuffer& cmdBuffer, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::PipelineStageFlags src_stages, vk::PipelineStageFlags dest_stages)
		{
			auto DstAccessMask = [](vk::ImageLayout const& layout) {
				vk::AccessFlags flags;

				switch (layout) {
				case vk::ImageLayout::eTransferDstOptimal:
					// Make sure anything that was copying from this image has
					// completed
					flags = vk::AccessFlagBits::eTransferWrite;
					break;
				case vk::ImageLayout::eColorAttachmentOptimal:
					flags = vk::AccessFlagBits::eColorAttachmentWrite;
					break;
				case vk::ImageLayout::eDepthStencilAttachmentOptimal:
					flags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					// Make sure any Copy or CPU writes to image are flushed
					flags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					flags = vk::AccessFlagBits::eTransferRead;
					break;
				case vk::ImageLayout::ePresentSrcKHR:
					flags = vk::AccessFlagBits::eMemoryRead;
					break;
				default:
					break;
				}

				return flags;
			};

			vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier();

			barrier.setSrcAccessMask(srcAccessMask);
			barrier.setDstAccessMask(DstAccessMask(newLayout));
			barrier.setOldLayout(oldLayout);
			barrier.setNewLayout(newLayout);
			barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
			barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
			barrier.setImage(Image);
			barrier.setSubresourceRange(vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1));

			cmdBuffer.pipelineBarrier(src_stages, dest_stages, vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void Texture::AllocateMemory(vk::Device device, const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags requirements)
		{
			if (MemoryAllocated && memoryRequirements.size != MemoryAllocated)
				device.freeMemory(Memory, nullptr);

			MemoryAllocated = memoryRequirements.size;

			MemoryAllocateInfo.setAllocationSize(memoryRequirements.size);
			MemoryAllocateInfo.setMemoryTypeIndex(0);

			bool found = VerifyMemoryType(memoryRequirements, requirements);

			assert(found);

			VK_CALL(device.allocateMemory, &MemoryAllocateInfo, nullptr, &Memory);
		}

		bool Texture::VerifyMemoryType(const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags requirements)
		{
			DeviceContext* device = &GetContext()->GetDevice();

			if (!device) return false;

			uint32_t typeBits = memoryRequirements.memoryTypeBits;

			for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
				if ((typeBits & 1) == 1) {
					// Type is available, does it match user properties?
					if ((device->GpuMemoryProperties.memoryTypes[i].propertyFlags & requirements) == requirements) {
						MemoryAllocateInfo.memoryTypeIndex = i;
						return true;
					}
				}
				typeBits >>= 1;
			}

			// No memory types matched, return failure
			return false;
		}
	}
}