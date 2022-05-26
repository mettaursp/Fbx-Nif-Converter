#include "MeshAsset.h"

#include <Engine/VulkanGraphics/FileFormats/ObjParser.h>
#include <Engine/VulkanGraphics/FileFormats/NifParser.h>
#include "MeshData.h"
#include <Engine/VulkanGraphics/Core/Mesh.h>
#include <Engine/VulkanGraphics/Core/GraphicsWindow.h>
#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector2S.h>

namespace Engine
{
	namespace Graphics
	{
		std::shared_ptr<MeshFormat> GetObjFormat()
		{
			static std::shared_ptr<MeshFormat> meshFormat = nullptr;

			if (meshFormat != nullptr) return meshFormat;

			VertexAttributeFormat positionAttribute
			{
				Enum::AttributeDataType::Float32,
				3,
				"position"
			};

			VertexAttributeFormat normalAttribute
			{
				Enum::AttributeDataType::Float32,
				3,
				"normal"
			};

			VertexAttributeFormat textureCoordsAttribute
			{
				Enum::AttributeDataType::Float32,
				2,
				"textureCoords"
			};

			std::vector<VertexAttributeFormat> format;

			format.push_back(positionAttribute);
			format.push_back(normalAttribute);
			format.push_back(textureCoordsAttribute);

			std::string hash;

			VertexAttributeFormat::GetHashString(hash, format);

			meshFormat = MeshFormat::GetCachedFormat(hash);

			if (meshFormat == nullptr)
			{
				meshFormat = Engine::Create<MeshFormat>();

				meshFormat->Push(positionAttribute);
				meshFormat->Push(normalAttribute);
				meshFormat->Push(textureCoordsAttribute);

				MeshFormat::CacheFormat(hash, meshFormat);
			}

			return meshFormat;
		}

		void MeshAsset::LoadDefault()
		{

		}

		struct VertexData
		{
			Vector3SF Position;
			Vector3SF Normal;
			Vector2SF TextureCoords;
		};

		void MeshAsset::Loading(std::istream& file)
		{
			const FilePath& extension = GetExtension();

			if (extension == FilePath(".obj"))
			{
				ObjParser parser;

				parser.Parse(file);

				BaseFormat = GetObjFormat();

				BaseData = Engine::Create<MeshData>();

				VertexCount = parser.Faces.size() * 3;
				IndexCount = VertexCount;

				BaseData->SetFormat(BaseFormat);
				BaseData->PushVertices(VertexCount, false);
				BaseData->PushIndices(IndexCount);

				for (size_t i = 0; i < parser.Faces.size(); ++i)
				{
					BaseData->PushIndex(3 * i + 0, 3 * (int)i + 0);
					BaseData->PushIndex(3 * i + 1, 3 * (int)i + 1);
					BaseData->PushIndex(3 * i + 2, 3 * (int)i + 2);

					VertexData vertices[3];

					Face& face = parser.Faces[i];

					for (size_t j = 0; j < 3; ++j)
					{
						Vertex& vertexInfo = face.Vertices[j];

						vertices[j].Position = parser.Vertices[vertexInfo.Position];

						if (vertexInfo.Normal != -1)
							vertices[j].Normal = parser.Normals[vertexInfo.Normal];

						if (vertexInfo.UV != -1)
							vertices[j].TextureCoords = parser.UVs[vertexInfo.UV];
					}

					Vector3SF faceNormal = (vertices[2].Position - vertices[0].Position).Cross((vertices[1].Position - vertices[0].Position)).Normalize();

					for (size_t j = 0; j < 3; ++j)
					{
						Vertex& vertexInfo = face.Vertices[j];

						if (vertexInfo.Normal == -1)
							vertices[j].Normal = faceNormal;
					}

					void* vertexData = vertices;

					BaseFormat->Copy(&vertexData, BaseData->GetData(), BaseFormat, 3, i * 3);
				}

				int cachedIndex = BaseFormat->GetCachedIndex();

				if (cachedIndex >= CachedMeshes.size())
					CachedMeshes.resize(cachedIndex + 1);

				CachedMeshes[cachedIndex] = BaseData;
			}
		}

		void MeshAsset::Saving(std::ostream& file)
		{

		}

		void MeshAsset::Unloading()
		{

		}

		void MeshAsset::SetMeshData(const std::shared_ptr<MeshData>& mesh)
		{
			if (BaseData != nullptr)
				throw "yo wtf";

			BaseData = mesh;
			BaseFormat = mesh->GetFormat();

			VertexCount = BaseData->GetVertices();
			IndexCount = BaseData->GetTriangleVertices();

			int cachedIndex = BaseFormat->GetCachedIndex();

			if (cachedIndex >= CachedMeshes.size())
				CachedMeshes.resize(cachedIndex + 1);

			CachedMeshes[cachedIndex] = BaseData;
		}

		std::shared_ptr<MeshData> NullPointer = nullptr;

		const std::shared_ptr<MeshData>& MeshAsset::GetMeshData(int cachedIndex)
		{
			if (cachedIndex == -1) return BaseData;

			std::shared_ptr<MeshFormat> format = MeshFormat::GetCachedFormat(cachedIndex);

			if (format == nullptr) return NullPointer;

			if (cachedIndex >= CachedMeshes.size())
				CachedMeshes.resize(cachedIndex + 1);

			if (CachedMeshes[cachedIndex] == nullptr)
			{
				CachedMeshes[cachedIndex] = std::make_shared<MeshData>();

				CachedMeshes[cachedIndex]->SetFormat(format);
				CachedMeshes[cachedIndex]->PushVertices(VertexCount, false);
				CachedMeshes[cachedIndex]->PushIndices(BaseData->GetIndexBuffer());

				BaseFormat->Copy(BaseData->GetData(), CachedMeshes[cachedIndex]->GetData(), format, VertexCount);
			}

			return CachedMeshes[cachedIndex];
		}


		const std::shared_ptr<Mesh>& MeshAsset::GetMesh(int cachedIndex, GraphicsContext* context)
		{
			static const std::shared_ptr<Mesh> NullPointer = nullptr;

			if (cachedIndex < CachedGpuMeshes.size() && CachedGpuMeshes[cachedIndex] != nullptr)
				return CachedGpuMeshes[cachedIndex];

			const std::shared_ptr<MeshData>& pipelineMesh = GetMeshData(cachedIndex);

			if (pipelineMesh == nullptr)
				return NullPointer;

			std::shared_ptr<Mesh> mesh = Engine::Create<Graphics::Mesh>();

			mesh->MeshData = pipelineMesh;
			mesh->AttachToContext(context);
			mesh->PrepareDataBuffers();

			if (cachedIndex >= CachedGpuMeshes.size())
				CachedGpuMeshes.resize(cachedIndex + 1);

			CachedGpuMeshes[cachedIndex] = mesh;

			return CachedGpuMeshes[cachedIndex];
		}
	}
}