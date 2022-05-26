#pragma once

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>
import <vector>;

#include "Texture.h"
#include "DeviceResource.h"
#include <Engine/Math/Vector2.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderPipeline;

		class FrameBuffer : public DeviceResource
		{
		public:
			~FrameBuffer();

			virtual void FreeData();

			void AddAttachment(const std::shared_ptr<Texture>& texture, bool setParent = false);
			void ClearAttachments();

			void InitializeResource(const Vector2I& size);
			void InitializeAttachments(const std::shared_ptr<ShaderPipeline>& pipeline);
			void ReleaseFrameBuffer();

			const vk::Framebuffer& GetResource() const { return Resource; }

		private:
			bool Initialized = false;

			vk::Framebuffer Resource;
			Vector2I Size;

			std::vector<std::shared_ptr<Texture>> Attachments;
			std::vector<vk::ImageView> AttachmentViews;
		};
	}
}