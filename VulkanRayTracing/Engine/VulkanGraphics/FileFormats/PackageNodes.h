#pragma once

import <string>;
import <memory>;
import <vector>;

#include <Engine/Math/Color3.h>

namespace Engine
{
	class Transform;

	namespace Graphics
	{
		class MeshFormat;
		class MeshData;

		struct ModelPackageNode
		{
			std::string Name;
			size_t AttachedTo = (size_t)-1;
			size_t MaterialIndex = (size_t)-1;
			std::shared_ptr<Engine::Graphics::MeshFormat> Format;
			std::shared_ptr<Engine::Graphics::MeshData> Mesh;
			std::shared_ptr<Engine::Transform> Transform;
		};

		struct ModelPackageMaterial
		{
			std::string Name;
			std::string Diffuse;
			std::string Normal;
			std::string Specular;
			std::string OverrideColor;

			Color3 DiffuseColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 SpecularColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 AmbientColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 EmissiveColor;
			float Shininess = 10;
			float Alpha = 1;
		};

		struct ModelPackage
		{
			std::vector<ModelPackageNode> Nodes;
			std::vector<ModelPackageMaterial> Materials;
		};
	}
}