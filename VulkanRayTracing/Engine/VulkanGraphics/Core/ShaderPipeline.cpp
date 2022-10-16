#include "ShaderPipeline.h"

import <filesystem>;
import <map>;

#include "Shader.h"
#include "VulkanErrorHandling.h"
#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include "Texture.h"
#include "SwapChain.h"
#include "Uniform.h"

namespace Engine
{
	namespace Graphics
	{
		ShaderPipeline::~ShaderPipeline()
		{
			ReleaseState();
		}

		void ShaderPipeline::FreeData()
		{
			ReleaseState();
		}

		const std::shared_ptr<Shader> NullPointer = nullptr;

		const std::shared_ptr<Shader>& ShaderPipeline::AddShader(const std::string& filePath, vk::ShaderStageFlagBits stage)
		{
			if (InitializedShaders)
				return NullPointer;

			std::string extension;

			switch (stage)
			{
			case vk::ShaderStageFlagBits::eVertex: extension = ".vert"; break;
			case vk::ShaderStageFlagBits::eFragment: extension = ".frag"; break;
			}

			std::string inputPath = "./shaderSource/" + filePath + extension;
			std::string outputPath = "./shaders/" + filePath + extension;

			if (!std::filesystem::exists(std::filesystem::path(outputPath)))
			{
				throw "compile yo damn shaders";
			}

			Shaders.push_back(Engine::Create<Graphics::Shader>(outputPath, stage));
			Shaders.back()->AttachToContext(GetContext());

			return Shaders.back();
		}

		void ShaderPipeline::CreatePipeline()
		{
			if (Initialized) return;//ResetPipeline();

			vk::PipelineCacheCreateInfo const pipelineCacheInfo;
			VK_CALL(GetDevice().createPipelineCache, &pipelineCacheInfo, nullptr, &Cache);

			InitializeShaders();
			InitializeColorBlends();
			InitializeMeshFormat();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
			vertexInputInfo.setVertexBindingDescriptionCount((uint32_t)VertexBindingInfo.size());
			vertexInputInfo.setPVertexBindingDescriptions(VertexBindingInfo.data());
			vertexInputInfo.setVertexAttributeDescriptionCount((uint32_t)VertexAttributes.size());
			vertexInputInfo.setPVertexAttributeDescriptions(VertexAttributeInfo.data());

			auto const inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

			// TODO: Where are pViewports and pScissors set?
			auto const viewportInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setScissorCount(1);

			auto const rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
				.setDepthClampEnable(VK_FALSE)
				.setRasterizerDiscardEnable(VK_FALSE)
				.setPolygonMode(vk::PolygonMode::eFill)
				.setCullMode(vk::CullModeFlagBits::eBack)
				.setFrontFace(vk::FrontFace::eCounterClockwise)
				.setDepthBiasEnable(VK_FALSE)
				.setLineWidth(1.0f);

			auto const multisampleInfo = vk::PipelineMultisampleStateCreateInfo();

			auto const stencilOp =
				vk::StencilOpState().setFailOp(vk::StencilOp::eKeep).setPassOp(vk::StencilOp::eKeep).setCompareOp(vk::CompareOp::eAlways);

			// make this data driven
			auto const depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo()
				.setDepthTestEnable(DepthTestEnabled)
				.setDepthWriteEnable(DepthWriteEnabled)
				.setDepthCompareOp(DepthTestMode)
				.setDepthBoundsTestEnable(VK_FALSE)
				.setStencilTestEnable(VK_FALSE)
				.setFront(stencilOp)
				.setBack(stencilOp);

			InitializeColorBlends();

			auto const colorBlendInfo =
				vk::PipelineColorBlendStateCreateInfo().setAttachmentCount((uint32_t)ColorBlendInfo.size()).setPAttachments(ColorBlendInfo.data());

			vk::DynamicState const dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

			auto const dynamicStateInfo = vk::PipelineDynamicStateCreateInfo().setPDynamicStates(dynamicStates).setDynamicStateCount(2);

			auto const pipeline = vk::GraphicsPipelineCreateInfo()
				.setStageCount((uint32_t)ShaderStageInfo.size())
				.setPStages(ShaderStageInfo.data())
				.setPVertexInputState(&vertexInputInfo)
				.setPInputAssemblyState(&inputAssemblyInfo)
				.setPViewportState(&viewportInfo)
				.setPRasterizationState(&rasterizationInfo)
				.setPMultisampleState(&multisampleInfo)
				.setPDepthStencilState(&depthStencilInfo)
				.setPColorBlendState(&colorBlendInfo)
				.setPDynamicState(&dynamicStateInfo)
				.setLayout(PipelineLayout)
				.setRenderPass(RenderPass);

			VK_CALL(GetDevice().createGraphicsPipelines, Cache, 1, &pipeline, nullptr, &Pipeline);

			for (size_t i = 0; i < Shaders.size(); ++i)
				Shaders[i]->UnloadByteCode();


			Initialized = true;
		}

		void ShaderPipeline::InitializeShaders()
		{
			if (InitializedShaders)
				return;

			for (size_t i = 0; i < Shaders.size(); ++i)
			{
				Shaders[i]->LoadByteCode();
				Shaders[i]->LoadModule();
			}

			ShaderStageInfo.resize(Shaders.size());

			for (size_t i = 0; i < Shaders.size(); ++i)
			{
				ShaderStageInfo[i] = vk::PipelineShaderStageCreateInfo();
				ShaderStageInfo[i].setStage(Shaders[i]->ShaderStage);
				ShaderStageInfo[i].setModule(Shaders[i]->Module);
				ShaderStageInfo[i].setPName(Shaders[i]->EntryFunction.c_str());
			}

			InitializedShaders = true;
		}

		void ShaderPipeline::InitializeColorBlends()
		{
			if (InitializedColorBlends)
				return;

			ColorBlendInfo.resize(ColorBlends.size());

			for (size_t i = 0; i < ColorBlends.size(); ++i)
			{
				//if (i < ColorBlends.size())
				//{
				ColorBlendInfo[i] = ColorBlendInfo[i] = vk::PipelineColorBlendAttachmentState();
				ColorBlendInfo[i].setColorWriteMask(ColorBlends[i].ColorComponents);
				ColorBlendInfo[i].setBlendEnable(ColorBlends[i].BlendEnabled);
				ColorBlendInfo[i].setSrcColorBlendFactor(ColorBlends[i].SrcColorBlendFactor);
				ColorBlendInfo[i].setDstColorBlendFactor(ColorBlends[i].DstColorBlendFactor);
				ColorBlendInfo[i].setColorBlendOp(ColorBlends[i].ColorBlendOp);
				ColorBlendInfo[i].setSrcAlphaBlendFactor(ColorBlends[i].SrcAlphaBlendFactor);
				ColorBlendInfo[i].setDstAlphaBlendFactor(ColorBlends[i].DstAlphaBlendFactor);
				ColorBlendInfo[i].setAlphaBlendOp(ColorBlends[i].AlphaBlendOp);
				//}
				//else
				//	ColorBlendInfo[i] = vk::PipelineColorBlendAttachmentState().setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				//		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
			}

			InitializedColorBlends = true;
		}

		void ShaderPipeline::InitializeMeshFormat()
		{
			if (InitializedMeshFormat)
				return;

			VertexAttributeInfo.resize(VertexAttributes.size());

			std::vector<VertexAttributeFormat> attributeFormats;
			VertexBindings.clear();

			for (size_t i = 0; i < VertexAttributes.size(); ++i)
			{
				size_t binding = VertexAttributes[i].Binding;

				if (VertexBindings.size() <= binding)
				{
					VertexBindings.resize(binding + 1);
					VertexBindings[binding].Binding = (uint32_t)binding;
					VertexBindings[binding].InputRate = vk::VertexInputRate::eVertex;
					VertexBindings[binding].Stride = 0;
				}

				VertexAttributeInfo[i] = vk::VertexInputAttributeDescription();
				VertexAttributeInfo[i].setLocation(VertexAttributes[i].Location);
				VertexAttributeInfo[i].setBinding((uint32_t)binding);
				VertexAttributeInfo[i].setFormat(VertexAttributes[i].Format);
				VertexAttributeInfo[i].setOffset(VertexAttributes[i].Offset);

				VertexBindings[binding].Stride += (uint32_t)VertexAttributeFormats[i].GetSize();
			}

			VertexBindingInfo.resize(VertexBindings.size());

			for (size_t i = 0; i < VertexBindings.size(); ++i)
			{
				VertexBindingInfo[i] = vk::VertexInputBindingDescription();
				VertexBindingInfo[i].setBinding(VertexBindings[i].Binding);
				VertexBindingInfo[i].setStride(VertexBindings[i].Stride);
				VertexBindingInfo[i].setInputRate(VertexBindings[i].InputRate);
			}

			std::string vertexHashString;

			VertexAttributeFormat::GetHashString(vertexHashString, VertexAttributeFormats);

			BoundFormat = MeshFormat::GetCachedFormat(vertexHashString);

			if (BoundFormat == nullptr)
			{
				BoundFormat = Engine::Create<MeshFormat>();

				for (size_t i = 0; i < VertexAttributeFormats.size(); ++i)
					BoundFormat->Push(VertexAttributeFormats[i]);

				MeshFormat::CacheFormat(BoundFormat);
			}

			InitializedMeshFormat = true;
		}

		std::shared_ptr<Uniform> MakeUniform(vk::DescriptorType type, uint32_t binding, uint32_t set)
		{
			switch (type)
			{
			case vk::DescriptorType::eCombinedImageSampler:
				return Engine::Create<CombinedSamplerUniform>(binding, set);
				break;

			case vk::DescriptorType::eUniformBuffer:
				return Engine::Create<BufferObjectUniform>(binding, set);
				break;

			default:
				throw "not supported";
			}

			return nullptr;
		}

		std::shared_ptr<Uniform> ShaderPipeline::AddUniform(vk::DescriptorType type, uint32_t binding, uint32_t set)
		{
			std::shared_ptr<Uniform> uniform = MakeUniform(type, binding, set);

			uniform->AttachToContext(GetContext());

			Uniforms.push_back(uniform);

			return uniform;
		}

		const std::shared_ptr<Uniform> ShaderPipeline::GetUniform(int binding, int set) const
		{
			for (const auto& uniform : Uniforms)
				if (uniform->GetBinding() == binding && uniform->GetSet() == set)
					return uniform;

			return nullptr;
		}

		void ShaderPipeline::InitializeDescriptors(size_t bufferCount)
		{
			if (InitializedDescriptors)
				return;

			std::map<vk::DescriptorType, size_t> descriptorPoolMap;

			DescriptorPoolSizes.clear();

			for (const auto& uniform : Uniforms)
			{
				vk::DescriptorType type = uniform->GetType();

				if (type == vk::DescriptorType::eInlineUniformBlockEXT)
				{
					for (size_t i = 0; i < bufferCount; ++i)
					{
						vk::DescriptorPoolSize size;

						size.setType(type);
						size.setDescriptorCount((uint32_t)uniform->GetSize());

						DescriptorPoolSizes.push_back(size);
					}
				}
				else
				{
					const auto& size = descriptorPoolMap.find(type);

					if (size == descriptorPoolMap.end())
					{
						vk::DescriptorPoolSize size;

						size.setType(type);
						size.setDescriptorCount((uint32_t)bufferCount);

						descriptorPoolMap[type] = DescriptorPoolSizes.size();

						DescriptorPoolSizes.push_back(size);
					}
					else
						DescriptorPoolSizes[size->second].descriptorCount += (uint32_t)bufferCount;
				}
			}

			DescriptorSets.resize(DescriptorLayouts.size() * bufferCount);

			auto const descriptor_pool =
				vk::DescriptorPoolCreateInfo().setMaxSets((uint32_t)DescriptorSets.size()).setPoolSizeCount((uint32_t)DescriptorPoolSizes.size()).setPPoolSizes(DescriptorPoolSizes.data());

			VK_CALL(GetDevice().createDescriptorPool, &descriptor_pool, nullptr, &DescriptorPool);

			auto const alloc_info =
				vk::DescriptorSetAllocateInfo().setDescriptorPool(DescriptorPool).setDescriptorSetCount((uint32_t)DescriptorLayouts.size()).setPSetLayouts(DescriptorLayouts.data());

			for (unsigned int i = 0; i < bufferCount; i++)
				VK_CALL(GetDevice().allocateDescriptorSets, &alloc_info, DescriptorSets.data() + (i * DescriptorSetCount));

			InitializedDescriptors = true;
		}

		void ShaderPipeline::FlushUniformChanges()
		{
			DescriptorSetWrites.clear();
			DescriptorSetWrites.reserve(Uniforms.size());
			
			for (size_t i = 0; i < Uniforms.size(); ++i)
			{
				//if (!Uniforms[i]->IsWrittenThisFrame())
				//	std::cout << "unset uniform" << std::endl;

				if (!Uniforms[i]->IsStale()) continue;

				DescriptorSetWrites.push_back(vk::WriteDescriptorSet());

				vk::WriteDescriptorSet& write = DescriptorSetWrites.back();

				write.setDstBinding(Uniforms[i]->GetBinding());
				write.setDescriptorCount((uint32_t)Uniforms[i]->GetSize());
				write.setDescriptorType(Uniforms[i]->GetType());
				write.setPImageInfo(Uniforms[i]->GetImageInfo());
				write.setPBufferInfo(Uniforms[i]->GetBufferInfo());
				write.setPTexelBufferView(Uniforms[i]->GetBufferViews());
				write.setDstSet(DescriptorSets[CurrentBuffer * DescriptorSetCount + Uniforms[i]->GetSet()]);

				Uniforms[i]->ClearStale();
			}

			GetDevice().updateDescriptorSets((uint32_t)DescriptorSetWrites.size(), DescriptorSetWrites.data(), 0, nullptr);
		}

		void ShaderPipeline::SetFrame(size_t currentFrame)
		{
			CurrentBuffer = currentFrame;
		}

		void ShaderPipeline::ReleaseDescriptors()
		{
			if (!InitializedDescriptors) return;

			GetDevice().destroyDescriptorPool(DescriptorPool, nullptr);

			InitializedDescriptors = false;
		}

		void ShaderPipeline::ResetPipeline()
		{
			if (!Initialized) return;

			GetDevice().destroyPipeline(Pipeline, nullptr);
			GetDevice().destroyPipelineCache(Cache, nullptr);

			Initialized = false;
		}

		void ShaderPipeline::AddColorBlendAttachment(const ColorBlendAttachment& attachment)
		{
			if (InitializedColorBlends)
				return;

			ColorBlends.push_back(attachment);
		}

		void ShaderPipeline::ClearColorBlendAttachments()
		{
			InitializedColorBlends = false;

			ColorBlends.clear();
		}

		void ShaderPipeline::AddBinding(const ShaderBinding& binding)
		{
			if (InitializedBindings) return;

			Bindings.push_back(binding);
		}

		void ShaderPipeline::ClearBindings()
		{
			Bindings.clear();
		}

		const vk::Format AttributeFormats[Enum::AttributeDataType::Count][4] = {
			/* Float32 */ {vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat, vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat },
			/* Float64 */ {},
			/* Bool	   */ {},
			/* Int8	   */ {},
			/* Int16   */ {},
			/* Int32   */ {},
			/* Int64   */ {},
			/* UInt8   */ {},
			/* UInt16  */ {},
			/* UInt32  */ {},
			/* UInt64  */ {}
		};

		void ShaderPipeline::AddVertexAttribute(const VertexAttributeFormat& attribute, uint32_t location, uint32_t binding)
		{
			if (InitializedMeshFormat)
				return;

			vk::Format format = AttributeFormats[attribute.Type][attribute.ElementCount - 1];
			uint32_t offset = 0;

			if (format == vk::Format::eUndefined)
				throw "not supported yet";
			
			for (size_t i = 0; i < VertexAttributes.size(); ++i)
				if (VertexAttributes[i].Binding == attribute.Binding)
					offset += (uint32_t)VertexAttributeFormats[i].GetSize();

			VertexAttribute attributeInfo
			{
				location,
				(uint32_t)attribute.Binding,
				format,
				offset,
				attribute.Name
			};

			VertexAttributes.push_back(attributeInfo);
			VertexAttributeFormats.push_back(attribute);
		}

		void ShaderPipeline::ClearVertexAttributes()
		{
			InitializedMeshFormat = false;

			VertexAttributeFormats.clear();
			VertexAttributes.clear();
			VertexBindings.clear();
		}

		void ShaderPipeline::InitializeBindings()
		{
			if (InitializedBindings)
				return;
				//ResetBindings();

			DescriptorSetCount = 0;

			for (size_t i = 0; i < Bindings.size(); ++i)
			{
				auto& binding = Bindings[i];

				if (DescriptorSetCount <= binding.DescriptorSetIndex)
				{
					DescriptorSetCount = binding.DescriptorSetIndex + 1;

					DescriptorLayouts.resize(DescriptorSetCount);
					DescriptorLayoutInfo.resize(DescriptorSetCount);
				}

				++DescriptorLayoutInfo[binding.DescriptorSetIndex].DescriptorCount;
			}

			BindingInfo.resize(Bindings.size());

			size_t offset = 0;

			for (size_t i = 0; i < DescriptorLayouts.size(); ++i)
			{
				auto& descriptorLayoutInfo = DescriptorLayoutInfo[i];
				auto& descriptorLayout = DescriptorLayouts[i];

				descriptorLayoutInfo.DescriptorStartIndex = offset;

				for (size_t j = 0; j < Bindings.size(); ++j)
				{
					auto& binding = Bindings[j];
					auto& bindingInfo = BindingInfo[offset];

					if (binding.DescriptorSetIndex != i) continue;

					bindingInfo = vk::DescriptorSetLayoutBinding();
					bindingInfo.setBinding(binding.Binding);
					bindingInfo.setDescriptorType(binding.Type);
					bindingInfo.setDescriptorCount(binding.Count);
					bindingInfo.setStageFlags(binding.ActiveStages);
					bindingInfo.setPImmutableSamplers(binding.ImmutableSamplers.size() == 0 ? nullptr : binding.ImmutableSamplers.data());

					++offset;
				}

				auto const descriptor_layout = vk::DescriptorSetLayoutCreateInfo().setBindingCount((uint32_t)descriptorLayoutInfo.DescriptorCount).setPBindings(BindingInfo.data() + descriptorLayoutInfo.DescriptorStartIndex);
				
				VK_CALL(GetDevice().createDescriptorSetLayout, &descriptor_layout, nullptr, &descriptorLayout);
			}

			vk::PushConstantRange pushConstantRange{
				PushConstantAccess,
				0,
				(uint32_t)PushConstantSize
			};

			auto pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount((uint32_t)DescriptorSetCount).setPSetLayouts(DescriptorLayouts.data());

			if (PushConstantSize > 0)
				pPipelineLayoutCreateInfo.setPushConstantRangeCount(1).setPPushConstantRanges(&pushConstantRange);

			VK_CALL(GetDevice().createPipelineLayout, &pPipelineLayoutCreateInfo, nullptr, &PipelineLayout);

			InitializedBindings = true;
		}

		void ShaderPipeline::ResetBindings()
		{
			if (!InitializedBindings) return;

			PushConstantSize = false;
			GetDevice().destroyPipelineLayout(PipelineLayout, nullptr);

			for (size_t i = 0; i < DescriptorLayouts.size(); ++i)
			{
				auto& descriptorLayoutInfo = DescriptorLayoutInfo[i];

				GetDevice().destroyDescriptorSetLayout(DescriptorLayouts[i], nullptr);

				descriptorLayoutInfo.DescriptorCount = 0;
				descriptorLayoutInfo.DescriptorStartIndex = 0;
			}

			InitializedBindings = false;
		}

		void ShaderPipeline::AddRenderTarget(const RenderTargetAttachment& attachment, AttachmentType attachmentType, vk::ImageLayout layout)
		{
			if (InitializedRenderTargets)
				return;

			uint32_t slot = (uint32_t)RenderTargets.size();

			RenderTargets.push_back(attachment);

			if (attachmentType == AttachmentType::DepthStencil)
				DepthStencilAttachmentInfo = vk::AttachmentReference().setAttachment(slot).setLayout(layout);
			else if (attachmentType == AttachmentType::Color)
				ColorAttachmentInfo.push_back(vk::AttachmentReference().setAttachment(slot).setLayout(layout));
		}

		void ShaderPipeline::ClearRenderTargets()
		{
			InitializedRenderTargets = false;

			RenderTargets.clear();
			ColorAttachmentInfo.clear();
		}

		void ShaderPipeline::AddDependency(const PassDependency& dependency)
		{
			if (InitializedRenderPass)
				return;

			PassDependencies.push_back(dependency);
		}

		void ShaderPipeline::ClearDependencies()
		{
			InitializedRenderTargets = false; 
			PassDependencies.clear();
		}

		void ShaderPipeline::InitializeRenderPass()
		{
			if (InitializedRenderPass)
				return;
				//ResetRenderPass();

			InitializeRenderTargetInfo();

			auto const subpass = vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachmentCount(0)
				.setPInputAttachments(nullptr)
				.setColorAttachmentCount((uint32_t)ColorAttachmentInfo.size())
				.setPColorAttachments(ColorAttachmentInfo.data())
				.setPResolveAttachments(nullptr)
				.setPDepthStencilAttachment(&DepthStencilAttachmentInfo)
				.setPreserveAttachmentCount(0)
				.setPPreserveAttachments(nullptr);

			auto const rp_info = vk::RenderPassCreateInfo()
				.setAttachmentCount((uint32_t)RenderTargetInfo.size())
				.setPAttachments(RenderTargetInfo.data())
				.setSubpassCount(1)
				.setPSubpasses(&subpass)
				.setDependencyCount((uint32_t)PassDependencyInfo.size())
				.setPDependencies(PassDependencyInfo.data());

			VK_CALL(GetDevice().createRenderPass, &rp_info, nullptr, &RenderPass);

			CreatePipeline();

			InitializedRenderPass = true;
		}

		void ShaderPipeline::InitializeRenderTargetInfo()
		{
			if (InitializedRenderTargets)
				return;

			RenderTargetInfo.resize(RenderTargets.size());

			for (size_t i = 0; i < RenderTargets.size(); ++i)
			{
				RenderTargetInfo[i] = vk::AttachmentDescription();
				RenderTargetInfo[i].setFormat(RenderTargets[i].Format);
				RenderTargetInfo[i].setSamples(RenderTargets[i].WriteSampleCount);
				RenderTargetInfo[i].setLoadOp(RenderTargets[i].LoadOperation);
				RenderTargetInfo[i].setStoreOp(RenderTargets[i].StoreOperation);
				RenderTargetInfo[i].setStencilLoadOp(RenderTargets[i].StencilLoadOperation);
				RenderTargetInfo[i].setStencilStoreOp(RenderTargets[i].StencilStoreOperation);
				RenderTargetInfo[i].setInitialLayout(RenderTargets[i].InitialLayout);
				RenderTargetInfo[i].setFinalLayout(RenderTargets[i].FinalLayout);
			};

			PassDependencyInfo.resize(PassDependencies.size());

			for (size_t i = 0; i < PassDependencies.size(); ++i)
			{
				PassDependencyInfo[i] = vk::SubpassDependency();
				PassDependencyInfo[i].setSrcSubpass(PassDependencies[i].SourceSubpass);
				PassDependencyInfo[i].setDstSubpass(PassDependencies[i].DestinationSubpass);
				PassDependencyInfo[i].setSrcStageMask(PassDependencies[i].SourceStageMask);
				PassDependencyInfo[i].setDstStageMask(PassDependencies[i].DestinationStageMask);
				PassDependencyInfo[i].setSrcAccessMask(PassDependencies[i].DestinationAccessMask);
				PassDependencyInfo[i].setDstAccessMask(PassDependencies[i].DestinationAccessMask);
				PassDependencyInfo[i].setDependencyFlags(PassDependencies[i].DependencyFlags);
			}

			InitializedRenderTargets = true;
		}

		void ShaderPipeline::ResetRenderPass()
		{
			if (!InitializedRenderPass) return;

			GetDevice().destroyRenderPass(RenderPass, nullptr);

			InitializedRenderPass = false;
		}

		void ShaderPipeline::AddPushConstantValue(Enum::AttributeDataType type, size_t elementCount, size_t columnCount)
		{
			if (InitializedBindings)
				return;

			PushConstantSize += GetDataSize(type, elementCount, columnCount);
		}

		void ShaderPipeline::Reset()
		{
			ResetPipeline();
			ResetRenderPass();
		}

		void ShaderPipeline::ReleaseState()
		{
			ClearBindings();
			ResetBindings();
			ClearVertexAttributes();
			ClearRenderTargets();
			ClearColorBlendAttachments();
			ClearDependencies();
			ReleaseDescriptors();

			for (size_t i = 0; i < Shaders.size(); ++i)
				Shaders[i]->ResetModule();

			Reset();
		}
	}
}