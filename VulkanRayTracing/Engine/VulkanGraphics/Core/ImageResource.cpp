#include "ImageResource.h"

#define STB_IMAGE_IMPLEMENTATION  

#include <stb_image.h>
#include "VulkanErrorHandling.h"

namespace Engine
{
	namespace Graphics
	{
		int GetChannels(vk::Format format)
		{
			switch (format)
			{
			case vk::Format::eUndefined: return 0;
			case vk::Format::eR8Unorm: return 1;
			case vk::Format::eR8G8Unorm: return 2;
			case vk::Format::eR8G8B8Unorm: return 3;
			case vk::Format::eR8G8B8A8Unorm: return 4;
			default: throw VulkanException(vk::Result::eErrorUnknown, "ImageResource::LoadImage", "attempt to load unsupported image format");
			}

			return 0;
		}

		ImageResource::~ImageResource()
		{
			ReleaseData();
		}

		void ImageResource::Load(const std::string& path, vk::Format format)
		{
			Load(path.c_str(), format);
		}

		void ImageResource::Load(const char* path, vk::Format format)
		{
			ReleaseData();

			int x, y, channels;

			int desiredChannels = GetChannels(format);

			Data = stbi_load(path, &x, &y, &channels, desiredChannels);

			Size = Vector2I(x, y);
			Format = format;

			if (desiredChannels == 0)
			{
				vk::Format formats[] = { vk::Format::eUndefined, vk::Format::eR8Unorm, vk::Format::eR8G8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm };

				Format = formats[channels];
			}
		}

		void ImageResource::CopyData(unsigned char* buffer, vk::SubresourceLayout& layout)
		{
			const uint8_t* data = Data;

			for (int y = 0; y < Size.Y; y++)
			{
				uint8_t* rowPtr = buffer;

				for (int x = 0; x < Size.X; x++)
				{
					memcpy(rowPtr, data, 3);
					rowPtr[3] = 255; /* Alpha of 1 */
					rowPtr += 4;
					data += 3;
				}

				buffer += layout.rowPitch;
			}
		}

		void ImageResource::ReleaseData()
		{
			if (Data)
				stbi_image_free(Data);

			Data = nullptr;
		}
	}
}