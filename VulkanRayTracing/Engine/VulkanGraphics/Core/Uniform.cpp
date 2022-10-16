#include "Uniform.h"

#include "Texture.h"

#include "VulkanErrorHandling.h"
#include "GraphicsContext.h"

namespace Engine
{
	namespace Graphics
	{
		void Uniform::ClearWrittenThisFrame()
		{
			WrittenThisFrame = false;
			Stale = false;
		}

		void Uniform::ClearStale()
		{
			Stale = false;
		}

		void Uniform::MarkStale()
		{
			Stale = true;
			WrittenThisFrame = true;
		}

		void CombinedSamplerUniform::SetCount(size_t index)
		{
			SamplerInfo.resize(index);
		}

		void CombinedSamplerUniform::Set(size_t index, const std::shared_ptr<Texture>& texture, vk::ImageLayout layout)
		{
			if (index < SamplerInfo.size())
			{
				if (texture != nullptr)
				{
					SamplerInfo[index].setSampler(texture->GetSampler());
					SamplerInfo[index].setImageView(texture->GetImageView());
				}
				else
				{
					SamplerInfo[index].setSampler(vk::Sampler());
					SamplerInfo[index].setImageView(vk::ImageView());
				}
				SamplerInfo[index].setImageLayout(layout);

				MarkStale();
			}
		}

		BufferObjectUniform::BufferObjectUniform(uint32_t binding, uint32_t set) : Uniform(binding, set)
		{
		}

		void BufferObjectUniform::InitializeData()
		{
			if (Initialized) return;

			vk::BufferUsageFlags Usage = vk::BufferUsageFlagBits::eUniformBuffer;
			vk::MemoryPropertyFlags Properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			size_t Size = sizeof(SceneInfo);

			auto const bufferInfo = vk::BufferCreateInfo().setSize(Size).setUsage(Usage);

			VK_CALL(GetDevice().createBuffer, &bufferInfo, nullptr, &Buffer);

			vk::MemoryRequirements memReqs;
			GetDevice().getBufferMemoryRequirements(Buffer, &memReqs);

			auto memAlloc = vk::MemoryAllocateInfo().setAllocationSize(memReqs.size).setMemoryTypeIndex(0);

			bool const pass = GetContext()->GetDevice().MeetsMemoryRequirements(memReqs.memoryTypeBits, Properties, &memAlloc.memoryTypeIndex);

			if (!pass)
				throw "failed to alloc";

			VK_CALL(GetDevice().allocateMemory, &memAlloc, nullptr, &Memory);

			VK_CALL(GetDevice().mapMemory, Memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags(), &Pointer);

			std::memcpy(Pointer, &Data, Size);

			VK_CALL(GetDevice().bindBufferMemory, Buffer, Memory, 0);

			BufferInfo.setBuffer(Buffer);
			BufferInfo.setOffset(0);
			BufferInfo.setRange(sizeof(SceneInfo));

			MarkStale();

			Initialized = true;
		}

		void BufferObjectUniform::UpdateData()
		{
			if (!Initialized) return;

			std::memcpy(Pointer, &Data, sizeof(SceneInfo));

			MarkStale();
		}

		BufferObjectUniform::~BufferObjectUniform()
		{
			FreeData();
		}

		void BufferObjectUniform::FreeData()
		{
			if (!Initialized) return;

			GetDevice().unmapMemory(Memory);
			GetDevice().destroyBuffer(Buffer, nullptr);
			GetDevice().freeMemory(Memory, nullptr);

			Initialized = false;
		}
	}
}