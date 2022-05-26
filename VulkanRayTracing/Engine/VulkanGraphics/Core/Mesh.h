#pragma once

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

#include "DeviceResource.h"

import <vector>;
import <memory>;

#include <Engine/Math/Vector2S-decl.h>
#include <Engine/Math/Vector3S-decl.h>
#include <Engine/Math/Matrix4-decl.h>

namespace Engine
{
	namespace Graphics
	{
		class ShaderPipeline;
		class Texture;
		struct DeviceContext;
		class MeshData;

		struct MeshGpuData
		{
			vk::DescriptorSet DescriptorSet;
		};

		class VulkanBuffer
		{
		public:
			vk::Buffer Buffer;
			vk::DeviceMemory Memory;
			void* Pointer = nullptr;
			vk::BufferUsageFlags Usage;
			vk::MemoryPropertyFlags Properties;
			size_t Size = 0;

			void Initialize(vk::Device device, const DeviceContext* context, const void* data, size_t size);
			void Release(vk::Device device);
			void Transfer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, const VulkanBuffer& stagingBuffer);

		private:
			bool Initialized = false;
		};

		class Mesh : public DeviceResource
		{
		public:
			std::shared_ptr<MeshData> MeshData;

			~Mesh();

			virtual void FreeData();

			void PrepareDataBuffers();
			void ReleaseBuffers();

			size_t GetBindings() const { return VertexBufferHandles.size(); }
			const vk::Buffer* GetVertexBuffers() const { return VertexBufferHandles.data(); }
			const vk::Buffer* GetIndexBuffer() const { return &IndexBuffer.Buffer; }
		private:
			bool InitializedBuffers = false;

			std::vector<VulkanBuffer> VertexStagingBuffers;
			std::vector<VulkanBuffer> VertexBuffers;
			std::vector<vk::Buffer> VertexBufferHandles;
			VulkanBuffer IndexStagingBuffer;
			VulkanBuffer IndexBuffer;
		};
	}
}