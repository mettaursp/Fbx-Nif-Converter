#include "ModelPackageAsset.h"

import <sstream>;
import <filesystem>;

#include <Engine/VulkanGraphics/Scene/MeshAsset.h>
#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include <Engine/VulkanGraphics/FileFormats/NifParser.h>
#include <Engine/VulkanGraphics/FileFormats/NifWriter.h>
#include <Engine/VulkanGraphics/FileFormats/FbxParser.h>
#include <Engine/VulkanGraphics/FileFormats/FbxWriter.h>
#include <Engine/Objects/Transform.h>
#include <Engine/VulkanGraphics/Scene/Scene.h>
#include <Engine/VulkanGraphics/Scene/Model.h>
#include <Engine/VulkanGraphics/Core/Texture.h>
#include <Engine/VulkanGraphics/Core/ImageResource.h>
#include <Engine/VulkanGraphics/Scene/DyeablePhongMaterial.h>

namespace Engine
{
	void ModelPackageAsset::LoadDefault() {}

	void ModelPackageAsset::Loading(std::istream& file)
	{
		const FilePath& extension = GetExtension();

		if (extension == FilePath(".nif") || extension == FilePath(".kf"))
		{
			NifParser parser;
			parser.Package = &Package;
			parser.Parse(file);
		}
		else if (extension == FilePath(".fbx"))
		{
			FbxParser parser;

			parser.Package = &Package;
			parser.Parse(file);
		}
	}

	void ModelPackageAsset::Saving(std::ostream& file, const FilePath& extension)
	{
		if (extension == FilePath(".nif") || extension == FilePath(".kf"))
		{
			NifWriter writer;
			writer.Package = &Package;
			writer.Write(file);
		}
		if (extension == FilePath(".fbx"))
		{
			FbxWriter writer;

			writer.Package = &Package;

			for (size_t i = 0; i < ImportedMeshes.size(); ++i)
			{
				std::stringstream name;
				name << "HR" << i;

				std::shared_ptr<Graphics::MeshData> data = ImportedMeshes[i]->GetMeshData();
				std::shared_ptr<Graphics::MeshFormat> format = data->GetFormat();

				Matrix4D transform;

				if (MeshTransforms.size() > i)
					transform = MeshTransforms[i]->GetTransformation();

				writer.FbxObjects.push_back(FbxObject{ name.str(), data, format, transform, i });
				writer.FbxMaterials.push_back(FbxMaterial{ "material", "diffuse.dds", "normal.dds", "specular.dds" });
			}

			writer.Write(file);
		}
	}

	void ModelPackageAsset::Unloading()
	{

	}

	void ModelPackageAsset::Instantiate(std::shared_ptr<Transform>& parent, std::shared_ptr<Graphics::Scene>& scene)
	{
		if (!IsLoaded()) return;

		std::vector<std::shared_ptr<Transform>> transforms(Package.Nodes.size());

		for (size_t j = 0; j < Package.Nodes.size(); ++j)
		{
			Graphics::ModelPackageNode& node = Package.Nodes[j];

			std::shared_ptr<Transform> transform = Engine::Create<Transform>();

			transform->Name = node.Transform->Name;
			transform->SetTransformation(node.Transform->GetTransformation());

			if (node.Mesh != nullptr)
			{
				if (ImportedMeshes.size() <= j)
					ImportedMeshes.resize(Package.Nodes.size());

				std::shared_ptr<Graphics::MeshAsset> asset = ImportedMeshes[j];
				
				if (asset == nullptr)
				{
					asset = Engine::Create<Graphics::MeshAsset>();
					asset->SetMeshData(node.Mesh);

					ImportedMeshes[j] = asset;
				}

				std::shared_ptr<Graphics::Model> model = Engine::Create<Graphics::Model>();
				model->MeshAsset = asset;
				model->Material = node.MaterialIndex != -1 ? Package.Materials[node.MaterialIndex].Material : nullptr;
				model->SetParent(transform);

				asset->SetParent(model);

				scene->AddObject(model);
			}

			transforms[j] = transform;
		}

		for (size_t j = 0; j < transforms.size(); ++j)
		{
			if (Package.Nodes[j].AttachedTo == (size_t)-1)
				transforms[j]->SetParent(parent);
			else
				transforms[j]->SetParent(transforms[Package.Nodes[j].AttachedTo]);

			transforms[j]->Update(0);
		}
	}

	void ModelPackageAsset::InstantiateTextures(std::shared_ptr<Object>& parent)
	{
		auto loadTexture = [this](const std::string& path) -> std::shared_ptr<Graphics::Texture>
		{
			if (path == "") return nullptr;

			std::filesystem::path fullPath = GetDirectoryPath();
			fullPath += path;

			if (!std::filesystem::exists(fullPath)) return nullptr;

			std::shared_ptr<Graphics::ImageResource> resource = Engine::Create<Graphics::ImageResource>();

			resource->Load(fullPath.string().c_str());

			std::shared_ptr<Graphics::Texture> texture = Engine::Create<Graphics::Texture>();

			texture->AttachToContext(GraphicsContext);
			texture->LoadResource(resource, GraphicsWindow);

			return texture;
		};

		for (size_t j = 0; j < Package.Materials.size(); ++j)
		{
			Graphics::ModelPackageMaterial& material = Package.Materials[j];

			material.DiffuseTexture = loadTexture(material.Diffuse);
			material.SpecularTexture = loadTexture(material.Specular);
			material.NormalTexture = loadTexture(material.Normal);
			material.OverrideColorTexture = loadTexture(material.OverrideColor);

			if (material.Material != nullptr)
			{
				std::shared_ptr<Graphics::DyeablePhongMaterial> materialData = material.Material->Cast<Graphics::DyeablePhongMaterial>();
				
				materialData->Albedo = material.DiffuseTexture;
				materialData->Specular = material.SpecularTexture;
				materialData->Normal = material.NormalTexture;
				materialData->ColorParams = material.OverrideColorTexture;
			}
		}
	}

	void ModelPackageAsset::InstantiateMaterials(std::shared_ptr<Object>& parent)
	{
		for (size_t j = 0; j < Package.Materials.size(); ++j)
		{
			Graphics::ModelPackageMaterial& material = Package.Materials[j];

			std::shared_ptr<Graphics::DyeablePhongMaterial> materialData = Engine::Create<Graphics::DyeablePhongMaterial>();

			materialData->Albedo = material.DiffuseTexture;
			materialData->Specular = material.SpecularTexture;
			materialData->Normal = material.NormalTexture;
			materialData->ColorParams = material.OverrideColorTexture;
			materialData->ShaderGroup = material.ShaderGroup;

			material.Material = materialData;
		}
	}
}