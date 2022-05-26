#include "FrameBuffer.h"

#include "ShaderPipeline.h"
#include "GraphicsWindow.h"
#include "VulkanErrorHandling.h"

namespace Engine
{
	namespace Graphics
	{
		FrameBuffer::~FrameBuffer()
		{
			ReleaseFrameBuffer();
		}

		void FrameBuffer::FreeData()
		{
			ReleaseFrameBuffer();
		}

		void FrameBuffer::ReleaseFrameBuffer()
		{
			if (Initialized)
				GetDevice().destroyFramebuffer(Resource, nullptr);

			ClearAttachments();

			Initialized = false;
		}

		void FrameBuffer::AddAttachment(const std::shared_ptr<Texture>& texture, bool setParent)
		{
			if (texture->GetSize() != Size)
				throw "size mismatch";

			Attachments.push_back(texture);
		}

		void FrameBuffer::ClearAttachments()
		{
			Attachments.clear();
		}

		void FrameBuffer::InitializeResource(const Vector2I& size)
		{
			if (Initialized) ReleaseFrameBuffer();

			Size = size;
		}

		void FrameBuffer::InitializeAttachments(const std::shared_ptr<ShaderPipeline>& pipeline)
		{
			if (Initialized) ReleaseFrameBuffer();

			AttachmentViews.resize(Attachments.size());

			for (size_t i = 0; i < Attachments.size(); ++i)
				AttachmentViews[i] = Attachments[i]->GetImageView();

			auto const fb_info = vk::FramebufferCreateInfo()
				.setRenderPass(pipeline->GetRenderPass())
				.setAttachmentCount((uint32_t)AttachmentViews.size())
				.setPAttachments(AttachmentViews.data())
				.setWidth((uint32_t)Size.X)
				.setHeight((uint32_t)Size.Y)
				.setLayers(1);

			VK_CALL(GetDevice().createFramebuffer, &fb_info, nullptr, &Resource);

			Initialized = true;
		}
	}
}