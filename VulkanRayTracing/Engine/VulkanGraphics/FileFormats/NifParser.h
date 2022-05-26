#pragma once

import <istream>;
import <vector>;

#include "PackageNodes.h"

namespace Engine
{
	namespace Graphics
	{
		class MeshFormat;
		class MeshData;
	}
}

struct ImportedNiMesh
{
	std::shared_ptr<Engine::Graphics::MeshFormat> Format;
	std::shared_ptr<Engine::Graphics::MeshData> Mesh;
};

class NifParser
{
public:
	std::vector<ImportedNiMesh> ImportedMeshes;
	Engine::Graphics::ModelPackage* Package = nullptr;

	void Parse(std::istream& stream);
};