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

			Format = resource->GetFormat();
			ViewType = vk::ImageViewType::e2D;
			ImageType = vk::ImageType::e2D;
			MipLevels = resource->GetMipMapCount();

			vk::FormatProperties formatProperties;

			context->Gpu->getFormatProperties(Format, &formatProperties);

			std::shared_ptr<Texture> stagingTexture = targetWindow->GetStagingTexture();
			vk::CommandBuffer& cmdBuffer = targetWindow->GetCommandBuffer();

			ReleaseResources();
			InitializeSampler();

			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled;
			vk::MemoryPropertyFlags requirements = vk::MemoryPropertyFlagBits::eDeviceLocal;

			if (resource->IsDDS())
				usage |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

			if ((formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) && stagingTexture != nullptr && !resource->IsDDS())
			{
				if (!resource->IsDDS())
					requirements = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

				LoadImageResource(resource, vk::ImageTiling::eLinear, usage, requirements);


				SetImageLayout(cmdBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::ePreinitialized,
					ImageLayout, vk::AccessFlagBits(), vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eFragmentShader);

				stagingTexture->Image = vk::Image();
			}
			else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage)
			{
				/* Must use staging buffer to copy linear texture to optimized */
				LoadBufferResource(resource, targetWindow);

				LoadImageResource(resource, vk::ImageTiling::eOptimal, usage, requirements);

				SetImageLayout(StagingCommandBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::ePreinitialized,
					vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits(), vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eTransfer);

				vk::ImageSubresourceLayers subresource = vk::ImageSubresourceLayers();

				subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				subresource.setMipLevel(0);
				subresource.setBaseArrayLayer(0);
				subresource.setLayerCount(1); // 6 for cube maps

				MipMapBuffer.resize(MipLevels);

				vk::DeviceSize offset = 0;
				//vk::Extent3D extent{ (uint32_t)stagingTexture->Size.X, (uint32_t)stagingTexture->Size.Y, 1 };
				vk::Extent3D extent{ (uint32_t)Size.X, (uint32_t)Size.Y, 1 };

				uint32_t pixelSize = 4;
				uint32_t blockWidth = 4;
				uint32_t blockHeight = 4;
				uint32_t blockSize = resource->GetBlockSize();

				for (uint32_t mipLevel = 0; mipLevel < MipLevels; ++mipLevel)
				{
					vk::BufferImageCopy& copyRegion = MipMapBuffer[mipLevel];

					copyRegion.setBufferOffset(offset);
					//copyRegion.setBufferRowLength(resource->IsDDS() ? 0 : stagingTexture->Size.X);
					//copyRegion.setBufferImageHeight(resource->IsDDS() ? 0 : stagingTexture->Size.Y);
					copyRegion.setBufferRowLength(resource->IsDDS() ? 0 : Size.X);
					copyRegion.setBufferImageHeight(resource->IsDDS() ? 0 : Size.Y);
					copyRegion.setImageSubresource(subresource);
					copyRegion.setImageOffset({ 0, 0, 0 });
					copyRegion.setImageExtent(extent);
					copyRegion.imageSubresource.mipLevel = mipLevel;

					if (resource->IsDDS())
						offset += ((extent.width + blockWidth - 1) / blockWidth) * ((extent.height + blockHeight - 1) / blockHeight) * blockSize;
					else
						offset += extent.width * extent.height * extent.depth * pixelSize;

					const uint32_t one = 1;

					extent.width = std::max(one, (uint32_t)extent.width >> 1);
					extent.height = std::max(one, (uint32_t)extent.height >> 1);
					extent.depth = std::max(one, (uint32_t)extent.depth >> 1);
				}

				//cmdBuffer.copyBufferToImage(stagingTexture->Buffer, Image, vk::ImageLayout::eTransferDstOptimal, MipLevels, MipMapBuffer.data());
				StagingCommandBuffer.copyBufferToImage(StagingBuffer, Image, vk::ImageLayout::eTransferDstOptimal, MipLevels, MipMapBuffer.data());

				SetImageLayout(StagingCommandBuffer, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal,
					ImageLayout, vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer,
					vk::PipelineStageFlagBits::eFragmentShader);

				CleanStagingBuffer(targetWindow);
			}
			else {
				assert(!"No support for R8G8B8A8_UNORM as texture image format");
			}

			vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo();
			viewInfo.setImage(Image);
			viewInfo.setViewType(ViewType);
			viewInfo.setFormat(Format);
			viewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, MipLevels, 0, 1));

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
				.setImageType(ImageType)
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

			AllocateMemory(device, mem_reqs, vk::MemoryPropertyFlagBits::eDeviceLocal, Memory);

			VK_CALL(GetDevice().bindImageMemory, Image, Memory, 0);

			auto const view = vk::ImageViewCreateInfo()
				.setImage(Image)
				.setViewType(ViewType)
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
				.setViewType(ViewType)
				.setFormat(targetWindow->GetSurfaceFormat())
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				.setImage(image);

			Image = image;
			Size = size;

			VK_CALL(GetDevice().createImageView, &color_image_view, nullptr, &ImageView);

			ViewInitialized = true;
		}

		void Texture::LoadBufferResource(const std::shared_ptr<ImageResource>& resource, const std::shared_ptr<GraphicsWindow>& targetWindow)
		{
			if (!resource) return;

			vk::Device device = GetDevice();

			if (device == vk::Device()) return;

			CleanStagingBuffer(targetWindow);

			vk::CommandBufferAllocateInfo allocateInfo;

			allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
			allocateInfo.setCommandBufferCount(1);
			allocateInfo.setCommandPool(targetWindow->CommandPool);

			VK_CALL(device.allocateCommandBuffers,&allocateInfo, &StagingCommandBuffer);

			vk::CommandBufferBeginInfo beginInfo;

			beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

			VK_CALL(StagingCommandBuffer.begin, &beginInfo);

			DispatchStagingCommands = true;

			Size = resource->GetSize();

			vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo();
			bufferInfo.setSize(Size.X * Size.Y * 4);
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
			bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
			bufferInfo.setQueueFamilyIndexCount(0);
			bufferInfo.setPQueueFamilyIndices(nullptr);

			VK_CALL(device.createBuffer, &bufferInfo, nullptr, &StagingBuffer);

			vk::MemoryRequirements memoryRequirements;
			device.getBufferMemoryRequirements(StagingBuffer, &memoryRequirements);

			vk::MemoryPropertyFlags requirements = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

			AllocateMemory(device, memoryRequirements, requirements, StagingMemory);

			VK_CALL(device.bindBufferMemory, StagingBuffer, StagingMemory, 0);

			vk::SubresourceLayout layout;
			layout.rowPitch = Size.X * 4;

			auto data = device.mapMemory(StagingMemory, 0, MemoryAllocateInfo.allocationSize);

			CheckError(data.result, "device.mapMemory");

			resource->CopyData((uint8_t*)data.value, layout);

			device.unmapMemory(StagingMemory);
			//device.freeMemory(StagingMemory, nullptr);

			StagingBufferInitialized = true;
		}

		void Texture::LoadImageResource(const std::shared_ptr<ImageResource>& resource, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags requirements)
		{
			if (ImageInitialized) return;
			if (!resource) return;

			vk::Device device = GetDevice();

			if (device == vk::Device()) return;

			Size = resource->GetSize();

			if (resource->IsDDS())
			{
				tiling = vk::ImageTiling::eOptimal;
			}

			vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo();

			uint32_t depth = 1;

			if (depth > 1)
			{
				ViewType = vk::ImageViewType::e3D;
				ImageType = vk::ImageType::e3D;
			}

			imageInfo.setImageType(ImageType);
			imageInfo.setFormat(Format);
			imageInfo.setExtent({ (uint32_t)Size.X, (uint32_t)Size.Y, depth });
			imageInfo.setMipLevels(MipLevels);
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

			AllocateMemory(device, memoryRequirements, requirements, Memory);

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

			CleanStagingBuffer(nullptr);

			if (ImageInitialized)
				device.destroyImage(Image, nullptr);

			if (MemoryAllocated)
				device.freeMemory(Memory, nullptr);

			if (ViewInitialized)
				device.destroyImageView(ImageView, nullptr);

			ImageInitialized = false;
			MemoryAllocated = false;
			ViewInitialized = false;
			SamplerInitialized = false;
		}

		void Texture::CleanStagingBuffer(const std::shared_ptr<GraphicsWindow>& targetWindow)
		{
			if (!StagingBufferInitialized) return;

			vk::Device device = GetDevice();

			if (DispatchStagingCommands && targetWindow != nullptr)
			{
				VK_CALL(StagingCommandBuffer.end);

				vk::SubmitInfo submitInfo;

				submitInfo.setCommandBufferCount(1);
				submitInfo.setCommandBuffers(StagingCommandBuffer);

				vk::Fence fence;

				// if sync
				{
					vk::FenceCreateInfo fenceCreateInfo;
					VK_CALL(device.createFence, &fenceCreateInfo, nullptr, &fence);
				}
				
				vk::Queue& queue = targetWindow->GetGraphicsQueue();

				VK_CALL(queue.submit, 1, &submitInfo, fence);

				// if sync
				{
					VK_CALL(device.waitForFences, 1, &fence, VK_TRUE, UINT64_MAX);
					device.destroyFence(fence);
				}

				VK_CALL(queue.waitIdle);
				device.freeCommandBuffers(targetWindow->CommandPool, 1, &StagingCommandBuffer);
			}

			device.destroyBuffer(StagingBuffer, nullptr);
			device.freeMemory(StagingMemory, nullptr);

			DispatchStagingCommands = false;
			StagingBufferInitialized = false;
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
			barrier.setSubresourceRange(vk::ImageSubresourceRange(aspectMask, 0, MipLevels, 0, 1));

			cmdBuffer.pipelineBarrier(src_stages, dest_stages, vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void Texture::AllocateMemory(vk::Device device, const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags requirements, vk::DeviceMemory& memory)
		{
			if (MemoryAllocated && memoryRequirements.size != MemoryAllocated)
				device.freeMemory(memory, nullptr);

			MemoryAllocated = memoryRequirements.size;

			MemoryAllocateInfo.setAllocationSize(memoryRequirements.size);
			MemoryAllocateInfo.setMemoryTypeIndex(0);

			bool found = VerifyMemoryType(memoryRequirements, requirements);

			assert(found);

			VK_CALL(device.allocateMemory, &MemoryAllocateInfo, nullptr, &memory);
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