#pragma once

import <istream>;
import <vector>;

#include "PackageNodes.h"

namespace Engine
{
	class Transform;

	namespace Graphics
	{
		class MeshFormat;
		class MeshData;
	}
}

struct ImportedFbxMesh
{
	std::shared_ptr<Engine::Graphics::MeshFormat> Format;
	std::shared_ptr<Engine::Graphics::MeshData> Mesh;
	std::shared_ptr<Engine::Transform> Transform;
};

class FbxParser
{
public:
	bool DebugPrint = false;

	std::vector<ImportedFbxMesh> ImportedMeshes;
	Engine::Graphics::ModelPackage* Package = nullptr;

	void Parse(std::istream& stream);
};