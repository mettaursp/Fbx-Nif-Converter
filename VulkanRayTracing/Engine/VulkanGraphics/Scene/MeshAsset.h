#pragma once

#include <Engine/Assets/Asset.h>

namespace Engine
{
	namespace Graphics
	{
		class MeshFormat;
		class MeshData;
		class Mesh;
		class GraphicsContext;

		class MeshAsset : public Asset
		{
		public:

			virtual void LoadDefault();
			virtual void Loading(std::istream& file);
			virtual void Saving(std::ostream& file);
			virtual void Unloading();

			void SetMeshData(const std::shared_ptr<MeshData>& mesh);
			const std::shared_ptr<MeshData>& GetMeshData(int cachedIndex = -1);
			const std::shared_ptr<Mesh>& GetMesh(int cachedIndex, GraphicsContext* context);

		private:
			std::shared_ptr<MeshFormat> BaseFormat;
			std::shared_ptr<MeshData> BaseData;
			std::vector<std::shared_ptr<MeshData>> CachedMeshes;
			std::vector<std::shared_ptr<Mesh>> CachedGpuMeshes;

			size_t VertexCount = 0;
			size_t IndexCount = 0;
		};
	}
}