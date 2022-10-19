#include "DeferredInputPipeline.h"

#include <Engine/VulkanGraphics/Core/Shader.h>
#include <Engine/VulkanGraphics/Core/Uniform.h>

namespace Engine
{
	namespace Graphics
	{
		void DeferredInputPipeline::Configure()
		{
			std::shared_ptr<Shader> fragShader = AddShader("deferredInput", vk::ShaderStageFlagBits::eFragment);
			std::shared_ptr<Shader> vertShader = AddShader("morphAnimation", vk::ShaderStageFlagBits::eVertex);

			Graphics::ShaderBinding uniformBuffer{ vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1 };
			Graphics::ShaderBinding samplers{ vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1 };
			Graphics::ShaderBinding uniformBuffer2{ vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1 };

			AddBinding(uniformBuffer);
			AddBinding(samplers);
			AddBinding(uniformBuffer2);
			AddPushConstantValue(Enum::AttributeDataType::Float32, 4, 4);
			AddPushConstantValue(Enum::AttributeDataType::Float32, 4, 4);
			PushConstantAccess = vk::ShaderStageFlagBits::eVertex;
			InitializeBindings();

			Graphics::RenderTargetAttachment albedoBuffer
			{
				vk::Format::eR8G8B8Unorm,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			Graphics::RenderTargetAttachment positionBuffer
			{
				vk::Format::eR16G16B16Sfloat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			Graphics::RenderTargetAttachment normalBuffer
			{
				vk::Format::eR16G16B16Sfloat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			Graphics::RenderTargetAttachment uvBuffer
			{
				vk::Format::eR16G16Sint,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			Graphics::ColorBlendAttachment rgbBlend
			{
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB,
				true,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd
			};

			Graphics::ColorBlendAttachment rgBlend
			{
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG,
				true,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd
			};

			Graphics::RenderTargetAttachment depthBuffer
			{
				vk::Format::eD16Unorm,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			};

			Graphics::VertexAttributeFormat positionAttribute
			{
				Enum::AttributeDataType::Float32,
				3,
				"position"
			};

			Graphics::VertexAttributeFormat normalAttribute
			{
				Enum::AttributeDataType::Float32,
				3,
				"normal",
				1
			};

			Graphics::VertexAttributeFormat textureCoordsAttribute
			{
				Enum::AttributeDataType::Float32,
				2,
				"textureCoords"
			};

			Graphics::VertexAttributeFormat morphPos1Attribute
			{
				Enum::AttributeDataType::Float32,
				3,
				"morphPosition1",
				2
			};

			AddRenderTarget(albedoBuffer, Enum::AttachmentType::Color, vk::ImageLayout::eColorAttachmentOptimal);
			AddColorBlendAttachment(rgbBlend);
			AddRenderTarget(positionBuffer, Enum::AttachmentType::Color, vk::ImageLayout::eColorAttachmentOptimal);
			AddColorBlendAttachment(rgbBlend);
			AddRenderTarget(normalBuffer, Enum::AttachmentType::Color, vk::ImageLayout::eColorAttachmentOptimal);
			AddColorBlendAttachment(rgbBlend);
			AddRenderTarget(uvBuffer, Enum::AttachmentType::Color, vk::ImageLayout::eColorAttachmentOptimal);
			AddColorBlendAttachment(rgBlend);
			AddRenderTarget(depthBuffer, Enum::AttachmentType::DepthStencil, vk::ImageLayout::eDepthStencilAttachmentOptimal);
			AddVertexAttribute(positionAttribute, 0);
			AddVertexAttribute(normalAttribute, 1);
			AddVertexAttribute(textureCoordsAttribute, 2);
			AddVertexAttribute(morphPos1Attribute, 3);

			InitializeMeshFormat();

			vk::PipelineStageFlags stages = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

			Graphics::PassDependency depthDependency
			{
				VK_SUBPASS_EXTERNAL,
				0,
				stages,
				stages,
				vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
				vk::DependencyFlags()
			};

			Graphics::PassDependency imageLayoutTransitionDependency
			{
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits(),
				vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
				vk::DependencyFlags()
			};

			AddDependency(depthDependency);
			AddDependency(imageLayoutTransitionDependency);

			std::shared_ptr<CombinedSamplerUniform> sampler = AddUniform(vk::DescriptorType::eCombinedImageSampler, 0, 0)->Cast<Graphics::CombinedSamplerUniform>();
			sampler->SetCount(1);

			std::shared_ptr<BufferObjectUniform> buffer = AddUniform(vk::DescriptorType::eUniformBuffer, 0, 0)->Cast<Graphics::BufferObjectUniform>();
			buffer->InitializeData();

			InitializeDescriptors(3);
		}
	}
}
