#pragma once

#include "Asset.h"
#include <Engine/VulkanGraphics/FileFormats/PackageNodes.h>

namespace Engine
{
	class Transform;

	namespace Graphics
	{
		class MeshAsset;
		class Scene;
		class GraphicsContext;
		class GraphicsWindow;
	}

	class ModelPackageAsset : public Asset
	{
	public:
		std::shared_ptr<Graphics::GraphicsContext> GraphicsContext;
		std::shared_ptr< Graphics::GraphicsWindow> GraphicsWindow;

		virtual void LoadDefault();
		virtual void Loading(std::istream& file);
		virtual void Saving(std::ostream& file, const FilePath& extension);
		virtual void Unloading();

		const std::vector<std::shared_ptr<Graphics::MeshAsset>>& GetImportedMeshes() const { return ImportedMeshes; }
		const std::vector<std::shared_ptr<Transform>>& GetMeshTransforms() const { return MeshTransforms; }
		Graphics::ModelPackage& GetPackage() { return Package; }
		const Graphics::ModelPackage& GetPackage() const { return Package; }
		void Instantiate(std::shared_ptr<Transform>& parent, std::shared_ptr<Graphics::Scene>& scene);
		void InstantiateTextures(std::shared_ptr<Object>& parent);
		void InstantiateMaterials(std::shared_ptr<Object>& parent);

	private:
		std::vector<std::shared_ptr<Graphics::MeshAsset>> ImportedMeshes;
		std::vector<std::shared_ptr<Transform>> MeshTransforms;
		Graphics::ModelPackage Package;
	};
}