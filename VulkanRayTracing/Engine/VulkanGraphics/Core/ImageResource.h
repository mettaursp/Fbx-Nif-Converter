#pragma once

import <string>;

#include "VulkanSupport.h"
#include <Engine/Math/Vector2.h>
#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class ImageResource : public Object
		{
		public:
			~ImageResource();

			void Load(const std::string& path, vk::Format format = vk::Format::eUndefined);
			void Load(const char* path, vk::Format format = vk::Format::eUndefined);
			void CreateEmpty(vk::Format format = vk::Format::eUndefined, int x = 1, int y = 1);
			void ReleaseData();
			const Vector2I& GetSize() const { return Size; }
			const unsigned char* GetData() const { return Data; }
			vk::Format GetFormat() const { return Format; }
			bool IsDDS() const { return DDSTexture; }
			int GetBlockSize() const { return BlockSize; }
			unsigned int GetMipMapCount() { return MipMapCount; }

			void CopyData(unsigned char* buffer, vk::SubresourceLayout& layout);

		private:
			vk::Format LoadDDS(const char* path);

			unsigned int MipMapCount = 1;
			int BlockSize = 16;
			bool DDSTexture = false;
			unsigned int DataSize = 0;
			unsigned char* Data = nullptr;
			Vector2I Size;
			vk::Format Format = vk::Format::eUndefined;
		};
	}
}