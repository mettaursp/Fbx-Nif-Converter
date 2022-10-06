#include "ModelPackageAsset.h"

import <sstream>;

#include <Engine/VulkanGraphics/Scene/MeshAsset.h>
#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include <Engine/VulkanGraphics/FileFormats/NifParser.h>
#include <Engine/VulkanGraphics/FileFormats/NifWriter.h>
#include <Engine/VulkanGraphics/FileFormats/FbxParser.h>
#include <Engine/VulkanGraphics/FileFormats/FbxWriter.h>
#include <Engine/Objects/Transform.h>

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

			ImportedMeshes.resize(parser.ImportedMeshes.size());
			MeshTransforms.resize(ImportedMeshes.size());

			for (size_t i = 0; i < parser.ImportedMeshes.size(); ++i)
			{
				ImportedMeshes[i] = Engine::Create<Graphics::MeshAsset>();
				ImportedMeshes[i]->SetMeshData(parser.ImportedMeshes[i].Mesh);
				MeshTransforms[i] = Engine::Create<Transform>();
			}
		}
		else if (extension == FilePath(".fbx"))
		{
			FbxParser parser;

			parser.Package = &Package;
			parser.Parse(file);

			ImportedMeshes.resize(parser.ImportedMeshes.size());
			MeshTransforms.resize(ImportedMeshes.size());

			for (size_t i = 0; i < parser.ImportedMeshes.size(); ++i)
			{
				ImportedMeshes[i] = Engine::Create<Graphics::MeshAsset>();
				ImportedMeshes[i]->SetMeshData(parser.ImportedMeshes[i].Mesh);
				MeshTransforms[i] = parser.ImportedMeshes[i].Transform;
			}
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
}