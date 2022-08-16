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

			void CopyData(unsigned char* buffer, vk::SubresourceLayout& layout);

		private:
			unsigned char* Data = nullptr;
			Vector2I Size;
			vk::Format Format = vk::Format::eUndefined;
		};
	}
}