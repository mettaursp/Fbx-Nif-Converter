#include "Mesh.h"

#include "VulkanErrorHandling.h"

#include "Texture.h"
#include "GraphicsWindow.h"
#include "ShaderPipeline.h"
#include "GraphicsContext.h"
#include "SwapChain.h"

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif
#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Vector3.h>
#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector2S.h>

namespace Engine
{
	namespace Graphics
	{
		Mesh::~Mesh()
		{
			ReleaseBuffers();
		}

		void Mesh::FreeData()
		{
			ReleaseBuffers();
		}

		void Mesh::PrepareDataBuffers()
		{
			if (InitializedBuffers)
				ReleaseBuffers();

			const auto& vertexBuffer = MeshData->GetData();
			const auto& indexBuffer = MeshData->GetIndexBuffer();

			size_t bindings = MeshData->GetBindingCount();

			VertexStagingBuffers.resize(bindings);
			VertexBuffers.resize(bindings);
			VertexBufferHandles.resize(bindings);

			for (size_t binding = 0; binding < bindings; ++binding)
			{
				VertexStagingBuffers[binding].Usage = vk::BufferUsageFlagBits::eTransferSrc;
				VertexStagingBuffers[binding].Properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
				VertexStagingBuffers[binding].Initialize(GetDevice(), &GetContext()->GetDevice(), vertexBuffer[binding], MeshData->GetTotalSize(binding));

				VertexBuffers[binding].Usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
				VertexBuffers[binding].Properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
				VertexBuffers[binding].Initialize(GetDevice(), &GetContext()->GetDevice(), nullptr, VertexStagingBuffers[binding].Size);
				VertexBuffers[binding].Transfer(GetDevice(), GetContext()->GetDevice().CommandPool, GetContext()->GetDevice().GraphicsQueue, VertexStagingBuffers[binding]);

				VertexBufferHandles[binding] = VertexBuffers[binding].Buffer;
			}

			IndexStagingBuffer.Usage = vk::BufferUsageFlagBits::eTransferSrc;
			IndexStagingBuffer.Properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			IndexStagingBuffer.Initialize(GetDevice(), &GetContext()->GetDevice(), indexBuffer.data(), indexBuffer.size() * sizeof(indexBuffer[0]));

			IndexBuffer.Usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
			IndexBuffer.Properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			IndexBuffer.Initialize(GetDevice(), &GetContext()->GetDevice(), nullptr, IndexStagingBuffer.Size);
			IndexBuffer.Transfer(GetDevice(), GetContext()->GetDevice().CommandPool, GetContext()->GetDevice().GraphicsQueue, IndexStagingBuffer);

			InitializedBuffers = true;
		}

		void VulkanBuffer::Initialize(vk::Device device, const DeviceContext* context, const void* data, size_t size)
		{
			if (Initialized)
				Release(device);

			auto const bufferInfo = vk::BufferCreateInfo().setSize(size).setUsage(Usage);

			VK_CALL(device.createBuffer, &bufferInfo, nullptr, &Buffer);

			vk::MemoryRequirements memReqs;
			device.getBufferMemoryRequirements(Buffer, &memReqs);

			auto memAlloc = vk::MemoryAllocateInfo().setAllocationSize(memReqs.size).setMemoryTypeIndex(0);

			bool const pass = context->MeetsMemoryRequirements(memReqs.memoryTypeBits, Properties, &memAlloc.memoryTypeIndex);

			if (!pass)
				throw "failed to alloc";

			VK_CALL(device.allocateMemory, &memAlloc, nullptr, &Memory);

			if (data != nullptr)
			{
				VK_CALL(device.mapMemory, Memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags(), &Pointer);

				std::memcpy(Pointer, data, size);

				device.unmapMemory(Memory);
			}

			VK_CALL(device.bindBufferMemory, Buffer, Memory, 0);

			Size = size;
			Initialized = true;
		}

		void VulkanBuffer::Transfer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, const VulkanBuffer& stagingBuffer)
		{
			vk::CommandBuffer commandBuffer;

			auto const cmd = vk::CommandBufferAllocateInfo()
				.setCommandPool(commandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);

			VK_CALL(device.allocateCommandBuffers, &cmd, &commandBuffer);

			auto const cmd_buf_info = vk::CommandBufferBeginInfo().setPInheritanceInfo(nullptr);

			VK_CALL(commandBuffer.begin, cmd_buf_info);

			vk::BufferCopy copyRegion{ 0, 0, Size };

			commandBuffer.copyBuffer(stagingBuffer.Buffer, Buffer, 1, &copyRegion);

			VK_CALL(commandBuffer.end);

			vk::SubmitInfo submitInfo{ 0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr };

			VK_CALL(queue.submit, 1, &submitInfo, vk::Fence());
			VK_CALL(queue.waitIdle);


			device.freeCommandBuffers(commandPool, { commandBuffer });
		}

		void VulkanBuffer::Release(vk::Device device)
		{
			if (!Initialized) return;

			device.destroyBuffer(Buffer, nullptr);
			device.freeMemory(Memory, nullptr);

			Initialized = false;
		}

		void Mesh::ReleaseBuffers()
		{
			if (!InitializedBuffers) return;

			for (size_t i = 0; i < VertexBuffers.size(); ++i)
			{
				VertexBuffers[i].Release(GetDevice());
				VertexStagingBuffers[i].Release(GetDevice());
			}
			IndexBuffer.Release(GetDevice());
			IndexStagingBuffer.Release(GetDevice());

			InitializedBuffers = false;
		}
	}
}