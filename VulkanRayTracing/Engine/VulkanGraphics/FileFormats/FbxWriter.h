#pragma once

import <ostream>;

#include "FbxNodes.h"
#include "PackageNodes.h"

class FbxWriter
{
public:
	std::vector<FbxObject> FbxObjects;
	std::vector<FbxMaterial> FbxMaterials;
	Engine::Graphics::ModelPackage* Package = nullptr;

	void Write(std::ostream& out);
};