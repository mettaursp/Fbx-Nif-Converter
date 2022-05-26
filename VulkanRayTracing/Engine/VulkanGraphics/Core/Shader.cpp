#include "Shader.h"

import <fstream>;

#include "VulkanErrorHandling.h"
#include "GraphicsWindow.h"

namespace Engine
{
	namespace Graphics
	{
		template <std::endian Endian = std::endian::native>
		void writeInt(std::ofstream& stream, uint32_t value);

		template <>
		void writeInt<std::endian::little>(std::ofstream& stream, uint32_t value)
		{
			unsigned char bytes[4] = {
				(value >> 24) & 0xFF,
				(value >> 16) & 0xFF,
				(value >> 8) & 0xFF,
				(value >> 0) & 0xFF,
			};

			for (int i = 0; i < 4; ++i)
				stream << bytes[i];
		}

		template <>
		void writeInt<std::endian::big>(std::ofstream& stream, uint32_t value)
		{
			unsigned char bytes[4] = {
				(value >> 0) & 0xFF,
				(value >> 8) & 0xFF,
				(value >> 16) & 0xFF,
				(value >> 24) & 0xFF,
			};

			for (int i = 0; i < 4; ++i)
				stream << bytes[i];
		}

		consteval std::endian opposite()
		{
			return std::endian::native == std::endian::little ? std::endian::big : std::endian::little;
		}

		template <std::endian Endian = std::endian::native>
		uint32_t readInt(std::ifstream& stream);

		template <>
		uint32_t readInt<std::endian::little>(std::ifstream& stream)
		{
			char bytes[4] = {};

			stream.read(bytes, 4);

			return ((unsigned char)bytes[0] << 24) | ((unsigned char)bytes[1] << 16) | ((unsigned char)bytes[2] << 8) | ((unsigned char)bytes[3] << 0);
		}

		template <>
		uint32_t readInt<std::endian::big>(std::ifstream& stream)
		{
			char bytes[4] = {};

			stream.read(bytes, 4);

			return ((unsigned char)bytes[3] << 24) | ((unsigned char)bytes[2] << 16) | ((unsigned char)bytes[1] << 8) | ((unsigned char)bytes[0] << 0);
		}

		Shader::Shader(const std::string& filePath, vk::ShaderStageFlagBits stage) : ShaderStage(stage)
		{
			SetPath(filePath);
			LoadByteCode();
		}

		Shader::~Shader()
		{
			ResetModule();
		}

		void Shader::FreeData()
		{
			ResetModule();
		}

		void Shader::SetPath(const std::string& filePath)
		{
			FilePath = filePath;
		}

		void Shader::LoadByteCode()
		{
			ByteCode.clear();

			{
				std::ifstream infs(FilePath, std::ios::in | std::ios::binary);

				uint32_t endianFlag = readInt(infs);

				ByteCode.push_back(0x07230203);

				if (endianFlag == 0x07230203)
					while (!infs.eof())
						ByteCode.push_back(readInt(infs));
				else
					while (!infs.eof())
						ByteCode.push_back(readInt<opposite()>(infs));

				std::vector<unsigned char> byteout;

				for (size_t i = 0; i < ByteCode.size(); ++i)
				{
					uint32_t value = ByteCode[i];

					unsigned char bytes[4] = {
						(value >> 24) & 0xFF,
						(value >> 16) & 0xFF,
						(value >> 8) & 0xFF,
						(value >> 0) & 0xFF,
					};

					for (int j = 0; j < 4; ++j)
						byteout.push_back(bytes[3 - j]);
				}

				byteout.push_back(0);

				//std::string outPath = FilePath + ".flip";
				//
				//std::ofstream outfs(outPath, std::ios::out | std::ios::binary);
				//
				//uint32_t magic = 0x07230203;
				//
				//const std::endian format = std::endian::native == std::endian::big ? std::endian::little : std::endian::big;
				//
				////writeInt<format>(outfs, magic);
				//
				//for (size_t i = 0; i < ByteCode.size(); ++i)
				//	writeInt<format>(outfs, ByteCode[i]);
				//
				//outfs.flush();
			}
		}

		void Shader::UnloadByteCode()
		{
			ByteCode.clear();
		}

		void Shader::LoadModule()
		{
			ResetModule();

			if (ByteCode.size() == 0) return;

			size_t size = (ByteCode.size() - 1) * sizeof(ByteCode[0]);

			const auto moduleCreateInfo = vk::ShaderModuleCreateInfo().setCodeSize(size).setPCode(ByteCode.data());

			VK_CALL(GetDevice().createShaderModule, &moduleCreateInfo, nullptr, &Module);

			Initialized = true;
		}

		void Shader::ResetModule()
		{
			if (!Initialized) return;

			GetDevice().destroyShaderModule(Module, nullptr);

			Initialized = false;
		}
	}
}