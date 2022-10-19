#include "RenderQueue.h"

#include "FrameBuffer.h"
#include "Mesh.h"
#include "ShaderPipeline.h"
#include "VulkanErrorHandling.h"
#include "SwapChain.h"
#include "GraphicsWindow.h"
#include <Engine/VulkanGraphics/Scene/MeshAsset.h>

namespace Engine
{
	namespace Graphics
	{
		void RenderQueue::StartFrame()
		{
			auto const commandInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			const Vector2I& resolution = Window->GetResolution();

			VK_CALL(CommandBuffer.reset);

			VK_CALL(CommandBuffer.begin, &commandInfo);

			float viewport_dimension;
			float viewport_x = 0.0f;
			float viewport_y = 0.0f;
			if (resolution.X < resolution.Y) {
				viewport_dimension = (float)resolution.X;
				viewport_y = (resolution.Y - resolution.X) / 2.0f;
			}
			else {
				viewport_dimension = (float)resolution.Y;
				viewport_x = (resolution.X - resolution.Y) / 2.0f;
			}
			auto const viewport = vk::Viewport()
				.setX(viewport_x)
				.setY(viewport_y)
				.setWidth((float)viewport_dimension)
				.setHeight((float)viewport_dimension)
				.setMinDepth((float)0.0f)
				.setMaxDepth((float)1.0f);
			CommandBuffer.setViewport(0, 1, &viewport);

			vk::Rect2D const scissor(vk::Offset2D(0, 0), vk::Extent2D(resolution.X, resolution.Y));
			CommandBuffer.setScissor(0, 1, &scissor);
		}

		void RenderQueue::BindRenderPass()
		{
			if (BoundPipeline == nullptr) return;

			vk::ClearValue const clearValues[2] = { vk::ClearColorValue(std::array<float, 4>({{0.2f, 0.2f, 0.2f, 0.2f}})),
												   vk::ClearDepthStencilValue(1.0f, 0u) };

			const Vector2I& resolution = Window->GetResolution();

			auto const passInfo = vk::RenderPassBeginInfo()
				.setRenderPass(BoundPipeline->GetRenderPass())
				.setFramebuffer(BoundBuffer == nullptr ? Owner->GetFrameBuffer()->GetResource() : BoundBuffer->GetResource())
				.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(resolution.X, resolution.Y)))
				.setClearValueCount(2)
				.setPClearValues(clearValues);

			CommandBuffer.beginRenderPass(&passInfo, vk::SubpassContents::eInline);
			CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, BoundPipeline->GetPipeline());
		}

		void RenderQueue::SetFrameBuffer(const std::shared_ptr<FrameBuffer>& frameBuffer)
		{
			BoundBuffer = frameBuffer;

			BindRenderPass();
		}

		void RenderQueue::SetShaderPipeline(const std::shared_ptr<ShaderPipeline>& pipeline)
		{
			if (BoundPipeline != nullptr)
				CommandBuffer.endRenderPass();

			BoundPipeline = pipeline;

			BindRenderPass();
		}

		void RenderQueue::DrawMesh(const std::shared_ptr<Mesh>& mesh)
		{
			const vk::DescriptorSet* descriptorSets = BoundPipeline->GetDescriptorSets();
			CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, BoundPipeline->GetPipelineLayout(), 0, (uint32_t)BoundPipeline->GetDescriptorSetCount(), descriptorSets, 0, nullptr);
			vk::DeviceSize offsets[0xF] = {};
			CommandBuffer.bindVertexBuffers(0, (uint32_t)mesh->GetBindings(), mesh->GetVertexBuffers(), offsets);
			CommandBuffer.bindIndexBuffer(*mesh->GetIndexBuffer(), 0, vk::IndexType::eUint32);
			CommandBuffer.drawIndexed((uint32_t)mesh->MeshData->GetTriangleVertices(), 1, 0, 0, 0);
		}

		void RenderQueue::DrawMesh(const std::shared_ptr<MeshAsset>& mesh)
		{
			DrawMesh(mesh->GetMesh(BoundPipeline->GetMeshFormat()->GetCachedIndex(), Window->GetContext().get()));
		}

		void RenderQueue::PushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, void* data)
		{
			CommandBuffer.pushConstants(BoundPipeline->GetPipelineLayout(), stageFlags, offset, size, data);
		}

		void RenderQueue::EndFrame()
		{
			if (BoundPipeline != nullptr)
				CommandBuffer.endRenderPass();

			if (Window->HasSeparatePresentQueue()) {
				auto const image_ownership_barrier =
					vk::ImageMemoryBarrier()
					.setSrcAccessMask(vk::AccessFlags())
					.setDstAccessMask(vk::AccessFlags())
					.setOldLayout(vk::ImageLayout::ePresentSrcKHR)
					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
					.setSrcQueueFamilyIndex(Window->GetGraphicsQueueFamilyIndex())
					.setDstQueueFamilyIndex(Window->GetPresentQueueFamilyIndex())
					.setImage(Owner->GetImage()->GetImage())
					.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

				CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
					vk::DependencyFlagBits(), 0, nullptr, 0, nullptr, 1, &image_ownership_barrier);
			}

			VK_CALL(CommandBuffer.end);

			BoundPipeline = nullptr;
		}
	}
}