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
		class ShaderGroup;
		class Texture;
		class Material;

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
			std::string ShaderName;
			std::string Diffuse;
			std::string Normal;
			std::string Specular;
			std::string OverrideColor;

			std::shared_ptr<Texture> DiffuseTexture;
			std::shared_ptr<Texture> NormalTexture;
			std::shared_ptr<Texture> SpecularTexture;
			std::shared_ptr<Texture> OverrideColorTexture;

			std::shared_ptr<Material> Material;

			Color3 DiffuseColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 SpecularColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 AmbientColor = Color3(0.5f, 0.5f, 0.5f);
			Color3 OverrideColor0 = Color3(1.f, 0.f, 0.f);
			Color3 OverrideColor1 = Color3(0.f, 1.f, 0.f);
			Color3 OverrideColor2 = Color3(0.f, 0.f, 1.f);
			Color3 EmissiveColor;
			float Shininess = 10;
			float Alpha = 1;
			float FresnelBoost = 1;
			float FresnelExponent = 1;

			std::shared_ptr<ShaderGroup> ShaderGroup;
		};

		struct ModelPackage
		{
			std::vector<ModelPackageNode> Nodes;
			std::vector<ModelPackageMaterial> Materials;
		};
	}
}