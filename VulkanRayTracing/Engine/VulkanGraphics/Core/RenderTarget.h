#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class BufferFormat;
		class ShaderPipeline;
		class Texture;
		class FrameBuffer;

		class RenderTarget : public Object
		{
		public:

			void SetFormat(const std::shared_ptr<BufferFormat>& format);
			void AttachTexture(const std::shared_ptr<Texture>& texture, size_t attachmentSlot);

			const std::shared_ptr<BufferFormat>& GetFormat() const { return Format; }

		private:
			struct AttachedPipeline
			{
				std::shared_ptr<ShaderPipeline> Pipeline;
				std::shared_ptr<FrameBuffer> FrameBuffer;
			};

			typedef std::vector<std::shared_ptr<FrameBuffer>> FrameBufferVector;

			std::shared_ptr<BufferFormat> Format;
			std::vector<AttachedPipeline> AttachedPipelines;
			std::vector<std::shared_ptr<Texture>> AttachedTextures;
		};
	}
}