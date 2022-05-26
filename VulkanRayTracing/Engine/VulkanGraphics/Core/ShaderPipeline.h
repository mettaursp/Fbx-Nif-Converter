#pragma once

import <vector>;
import <memory>;
import <string>;

#include "VulkanSupport.h"

#include <vulkan/vulkan_handles.hpp>

#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include <Engine/Objects/Object.h>
#include "DeviceResource.h"

namespace Engine
{
	namespace Graphics
	{
		class GraphicsWindow;
		class Shader;
		class Texture;
		class Uniform;

		struct ShaderBinding
		{
			vk::DescriptorType Type;
			vk::ShaderStageFlagBits ActiveStages;
			int Count = 1;
			std::vector<vk::Sampler> ImmutableSamplers;
		};

		struct RenderTargetAttachment
		{
			vk::Format Format;
			vk::SampleCountFlagBits WriteSampleCount;
			vk::AttachmentLoadOp LoadOperation;
			vk::AttachmentStoreOp StoreOperation;
			vk::AttachmentLoadOp StencilLoadOperation;
			vk::AttachmentStoreOp StencilStoreOperation;
			vk::ImageLayout InitialLayout;
			vk::ImageLayout FinalLayout;
		};

		struct ColorBlendAttachment
		{
			vk::ColorComponentFlags ColorComponents;
			bool BlendEnabled = true;
			vk::BlendFactor SrcColorBlendFactor;
			vk::BlendFactor DstColorBlendFactor;
			vk::BlendOp ColorBlendOp;
			vk::BlendFactor SrcAlphaBlendFactor;
			vk::BlendFactor DstAlphaBlendFactor;
			vk::BlendOp AlphaBlendOp;
		};

		struct PassDependency
		{
			uint32_t SourceSubpass;
			uint32_t DestinationSubpass;
			vk::PipelineStageFlags SourceStageMask;
			vk::PipelineStageFlags DestinationStageMask;
			vk::AccessFlags SourceAccessMask;
			vk::AccessFlags DestinationAccessMask;
			vk::DependencyFlags DependencyFlags;
		};

		struct VertexBinding
		{
			uint32_t Binding;
			uint32_t Stride;
			vk::VertexInputRate InputRate;
		};

		struct VertexAttribute
		{
			uint32_t Location;
			uint32_t Binding;
			vk::Format Format;
			uint32_t Offset;
			std::string Name;
		};

		struct AttachmentTypeEnum
		{
			enum AttachmentType
			{
				Color,
				DepthStencil,
				Misc
			};
		};

		class ShaderPipeline : public DeviceResource
		{
		public:
			typedef AttachmentTypeEnum::AttachmentType AttachmentType;

			bool DepthTestEnabled = true;
			bool DepthWriteEnabled = true;
			vk::CompareOp DepthTestMode = vk::CompareOp::eLessOrEqual;
			vk::ShaderStageFlags PushConstantAccess = vk::ShaderStageFlagBits::eAllGraphics;

			~ShaderPipeline();

			virtual void Configure() {}
			virtual void FreeData();

			const std::shared_ptr<Shader>& AddShader(const std::string& filePath, vk::ShaderStageFlagBits stage);

			void CreatePipeline();
			void ResetPipeline();

			void AddBinding(const ShaderBinding& binding);
			void ClearBindings();

			void AddVertexAttribute(const VertexAttributeFormat& attribute, uint32_t location, uint32_t binding = 0);
			void ClearVertexAttributes();

			void InitializeBindings();
			void ResetBindings();

			void AddRenderTarget(const RenderTargetAttachment& attachment, AttachmentType attachmentType = AttachmentType::Misc, vk::ImageLayout layout = vk::ImageLayout::eUndefined);
			void ClearRenderTargets();

			void AddColorBlendAttachment(const ColorBlendAttachment& attachment);
			void ClearColorBlendAttachments();

			void AddDependency(const PassDependency& dependency);
			void ClearDependencies();

			void InitializeRenderPass();
			void ResetRenderPass();

			void AddPushConstantValue(Enum::AttributeDataType type, size_t elementCount = 1, size_t columnCount = 1);

			std::shared_ptr<Uniform> AddUniform(vk::DescriptorType type, uint32_t binding);
			void FlushUniformChanges();
			size_t GetUniformCount() const { return Uniforms.size(); }

			void SetFrame(size_t currentFrame);

			void InitializeDescriptors(size_t bufferCount);
			void UpdateDescriptors();
			void ReleaseDescriptors();
			void InitializeMeshFormat();

			void Reset();
			void ReleaseState();

			std::vector<std::shared_ptr<Texture>> Textures;

			vk::Pipeline GetPipeline() const { return Pipeline; }
			vk::PipelineCache GetCache() const { return Cache; }
			vk::PipelineLayout GetPipelineLayout() const { return PipelineLayout; }
			const vk::DescriptorSetLayout& GetDescriptorLayout() const { return DescriptorLayout; }
			vk::RenderPass GetRenderPass() const { return RenderPass; }
			const vk::DescriptorSet& GetDescriptorSet(uint32_t index) const { return DescriptorSets[index]; };
			const std::shared_ptr<MeshFormat>& GetMeshFormat() const { return BoundFormat; }
			const std::shared_ptr<Uniform> GetUniform(int index) const;
			size_t GetUniforms() const { return Uniforms.size(); }

			bool HasBindings() const { return InitializedBindings; }

		private:
			bool Initialized = false;
			bool InitializedMeshFormat = false;
			bool InitializedBindings = false;
			bool InitializedRenderPass = false;
			bool InitializedDescriptors = false;
			bool InitializedRenderTargets = false;
			bool InitializedColorBlends = false;
			bool InitializedShaders = false;

			size_t CurrentBuffer = 0;

			vk::Pipeline Pipeline;
			vk::PipelineCache Cache;
			vk::PipelineLayout PipelineLayout;
			vk::DescriptorSetLayout DescriptorLayout;
			vk::RenderPass RenderPass;
			vk::DescriptorPool DescriptorPool;

			std::vector<vk::DescriptorSet> DescriptorSets;
			std::vector<vk::DescriptorImageInfo> SamplerInfo;
			std::vector<std::shared_ptr<Uniform>> Uniforms;
			std::vector<vk::DescriptorPoolSize> DescriptorPoolSizes;
			std::vector<vk::WriteDescriptorSet> DescriptorSetWrites;

			vk::AttachmentReference DepthStencilAttachmentInfo;
			std::vector<vk::AttachmentReference> ColorAttachmentInfo;

			std::shared_ptr<MeshFormat> BoundFormat;

			size_t PushConstantSize = 0;

			std::vector<std::shared_ptr<Shader>> Shaders;
			std::vector<vk::PipelineShaderStageCreateInfo> ShaderStageInfo;
			std::vector<ShaderBinding> Bindings;
			std::vector<vk::DescriptorSetLayoutBinding> BindingInfo;
			std::vector<RenderTargetAttachment> RenderTargets;
			std::vector<vk::AttachmentDescription> RenderTargetInfo;
			std::vector<ColorBlendAttachment> ColorBlends;
			std::vector<vk::PipelineColorBlendAttachmentState> ColorBlendInfo;
			std::vector<PassDependency> PassDependencies;
			std::vector<vk::SubpassDependency> PassDependencyInfo;
			std::vector<VertexBinding> VertexBindings;
			std::vector<vk::VertexInputBindingDescription> VertexBindingInfo;
			std::vector<VertexAttributeFormat> VertexAttributeFormats;
			std::vector<VertexAttribute> VertexAttributes;
			std::vector<vk::VertexInputAttributeDescription> VertexAttributeInfo;

			void InitializeRenderTargetInfo();
			void InitializeColorBlends();
			void InitializeShaders();
		};
	}
}

namespace Enum
{
	typedef Engine::Graphics::AttachmentTypeEnum::AttachmentType AttachmentType;
}