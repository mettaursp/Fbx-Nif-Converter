#pragma once

#include "Asset.h"
#include <Engine/VulkanGraphics/FileFormats/PackageNodes.h>

namespace Engine
{
	class Transform;

	namespace Graphics
	{
		class MeshAsset;
	}

	class ModelPackageAsset : public Asset
	{
	public:
		virtual void LoadDefault();
		virtual void Loading(std::istream& file);
		virtual void Saving(std::ostream& file, const FilePath& extension);
		virtual void Unloading();

		const std::vector<std::shared_ptr<Graphics::MeshAsset>>& GetImportedMeshes() const { return ImportedMeshes; }
		const std::vector<std::shared_ptr<Transform>>& GetMeshTransforms() const { return MeshTransforms; }
		const Graphics::ModelPackage& GetPackage() const { return Package; }

	private:
		std::vector<std::shared_ptr<Graphics::MeshAsset>> ImportedMeshes;
		std::vector<std::shared_ptr<Transform>> MeshTransforms;
		Graphics::ModelPackage Package;
	};
}