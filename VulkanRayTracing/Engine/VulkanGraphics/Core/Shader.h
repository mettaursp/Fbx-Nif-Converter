#pragma once

import <vector>;
import <string>;

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

#include <Engine/Objects/Object.h>
#include "DeviceResource.h"

namespace Engine
{
	namespace Graphics
	{
		class GraphicsWindow;

		class Shader : public DeviceResource
		{
		public:
			std::string EntryFunction = "main";
			vk::ShaderStageFlagBits ShaderStage;
			std::shared_ptr<GraphicsWindow> Window;
			vk::ShaderModule Module;

			Shader() {}
			Shader(const std::string& filePath, vk::ShaderStageFlagBits stage);

			~Shader();

			virtual void FreeData();

			void SetPath(const std::string& filePath);
			void LoadByteCode();
			void UnloadByteCode();
			void LoadModule();
			void ResetModule();

		private:
			bool Initialized = false;
			std::string FilePath;
			std::vector<uint32_t> ByteCode;
		};
	}
}