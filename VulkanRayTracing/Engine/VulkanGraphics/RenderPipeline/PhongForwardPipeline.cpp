#include "PhongForwardPipeline.h"

#include <Engine/VulkanGraphics/Core/Shader.h>
#include <Engine/VulkanGraphics/Core/Uniform.h>

namespace Engine
{
	namespace Graphics
	{
		void PhongForwardPipeline::Configure()
		{
			std::shared_ptr<Shader> fragShader = AddShader("fragmentShaderTest", vk::ShaderStageFlagBits::eFragment);
			std::shared_ptr<Shader> vertShader = AddShader("vertexShaderTest", vk::ShaderStageFlagBits::eVertex);

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

			Graphics::RenderTargetAttachment colorBuffer
			{
				ColorFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};

			Graphics::ColorBlendAttachment colorBufferBlend
			{
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
				true,
				vk::BlendFactor::eSrcAlpha,
				vk::BlendFactor::eOneMinusSrcAlpha,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd
			};

			Graphics::RenderTargetAttachment depthBuffer
			{
				DepthFormat,
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

			AddRenderTarget(colorBuffer, Enum::AttachmentType::Color, vk::ImageLayout::eColorAttachmentOptimal);
			AddColorBlendAttachment(colorBufferBlend);
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

			std::shared_ptr<CombinedSamplerUniform> sampler = AddUniform(vk::DescriptorType::eCombinedImageSampler, 1)->Cast<Graphics::CombinedSamplerUniform>();
			sampler->SetCount(1);

			std::shared_ptr<BufferObjectUniform> buffer = AddUniform(vk::DescriptorType::eUniformBuffer, 2)->Cast<Graphics::BufferObjectUniform>();
			buffer->InitializeData();

			InitializeDescriptors(3);
		}
	}
}