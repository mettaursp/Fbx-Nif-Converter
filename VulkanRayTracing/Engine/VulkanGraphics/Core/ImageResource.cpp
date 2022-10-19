#include "ImageResource.h"

import <fstream>;

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
			case vk::Format::eBc1RgbaUnormBlock: return 4;
			case vk::Format::eBc2UnormBlock: return 2;
			case vk::Format::eBc3UnormBlock: return 2;
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

			int pathSize;

			for (pathSize = 0; path[pathSize]; ++pathSize);

			Format = vk::Format::eR8G8B8A8Unorm;

			if (pathSize > 4 && strcmp(path + (pathSize - 4), ".dds") == 0)
				format = LoadDDS(path);

			int desiredChannels = GetChannels(format);

			if (!DDSTexture)
			{
				Data = stbi_load(path, &x, &y, &channels, desiredChannels);

				Size = Vector2I(x, y);
			}

			if (desiredChannels == 0)
			{
				vk::Format formats[] = { vk::Format::eUndefined, vk::Format::eR8Unorm, vk::Format::eR8G8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm };

				Format = formats[channels];
			}
		}

		vk::Format ImageResource::LoadDDS(const char* path)
		{
			std::ifstream file(path, std::ios_base::in | std::ios_base::binary);

			if (!file.is_open() || !file.good())
				throw std::string("cannot open file: '" + std::string(path) + "'");

			char headerType[4];

			file.read(headerType, 4);

			if (std::string(headerType, 4) != "DDS ")
				throw std::string("not a valid DDS: '" + std::string(path) + "'");

			unsigned char header[124];

			file.read((char*)header, 124);

			Size.X = (header[8]) | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);
			Size.Y = (header[12]) | (header[13] << 8) | (header[14] << 16) | (header[15] << 24);
			unsigned int linearSize = (header[16]) | (header[17] << 8) | (header[18] << 16) | (header[19] << 24);
			MipMapCount = (header[24]) | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);

			if (MipMapCount == 0)
				MipMapCount = 1;

			unsigned int size = (MipMapCount > 1 ? linearSize + linearSize : linearSize);
			DataSize = size;
			Data = new unsigned char[size];

			int count = -1;
			int remaining = size;
			char* offset = (char*)Data;

			while (count != 0 && remaining > 0)
			{
				file.read((char*)Data, remaining);
				count = int(file.gcount());
				remaining -= count;
				offset += count;
			}

			bool eof = file.eof();

			std::string formatName((char*)header + 80, 4);

			DDSTexture = true;
			BlockSize = 16;

			vk::Format format;

			if (formatName == std::string("DXT1", 4))
			{
				BlockSize = 8;
				format = vk::Format::eBc1RgbaUnormBlock;
			}
			else if (formatName == std::string("DXT3", 4))
				format = vk::Format::eBc2UnormBlock;
			else if (formatName == std::string("DXT5", 4))
				format = vk::Format::eBc3UnormBlock;
			else
			{
				DDSTexture = false;

				delete[] Data;

				throw "unsupported DDS: '" + std::string(path) + "'";
			}

			return format;
		}

		void ImageResource::CreateEmpty(vk::Format format, int x, int y)
		{
			ReleaseData();

			format = vk::Format::eR32G32B32Sfloat;
			int channels = GetChannels(format);

			if (channels == 0)
			{
				vk::Format formats[] = { vk::Format::eUndefined, vk::Format::eR8Unorm, vk::Format::eR8G8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm };

				Format = formats[channels];
			}

			Size = Vector2I(x, y);
			Format = format;
			Data = new unsigned char [channels*x*y] {0};
		}

		void ImageResource::CopyData(unsigned char* buffer, vk::SubresourceLayout& layout)
		{
			const uint8_t* data = Data;

			if (DDSTexture)
			{
				std::memcpy(buffer, Data, DataSize);
				
				return;
			}

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
			{
				if (DDSTexture)
					delete[] Data;
				else
					stbi_image_free(Data);
			}

			Data = nullptr;
		}
	}
}