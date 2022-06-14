#include "NifWriter.h"

import <map>;

#include <Engine/Objects/Transform.h>

#include "NifBlockTypes.h"
#include "PackageNodes.h"

BlockData& NifDocument::MakeBlock(const std::string& name)
{
	unsigned int index = (unsigned int)BlockMap.size();

	BlockData& block = BlockMap[index];
	block.BlockIndex = (unsigned int)index;
	block.BlockName = name;
	block.Document = this;

	return block;
}

using namespace Engine::Graphics;

bool compare(float x, float y, float epsilon = 1e-5f)
{
	return std::abs(x - y) < epsilon;
}

using namespace std::string_literals;

std::map<std::string, NifDocument::BlockWriteFunction> writeFunctions = {
	{ "NiNode", &NifDocument::WriteNode },
	{ "NiMesh", &NifDocument::WriteMesh },
	{ "NiVertexColorProperty", &NifDocument::WriteVertexColorProperty },
	{ "NiZBufferProperty", &NifDocument::WriteZBufferProperty },
	{ "NiSpecularProperty", &NifDocument::WriteSpecularProperty },
	{ "NiMaterialProperty", &NifDocument::WriteMaterialProperty },
	{ "NiTexturingProperty", &NifDocument::WriteTexturingProperty },
	{ "NiSourceTexture", &NifDocument::WriteSourceTexture },
	{ "NiFloatExtraData", &NifDocument::WriteFloatExtraData },
	{ "NiColorExtraData", &NifDocument::WriteColorExtraData },
	{ "NiIntegerExtraData", &NifDocument::WriteIntegerExtraData },
	{ "NiMorphWeightsController", &NifDocument::WriteMorphWeightsController },
	{ "NiFloatInterpolator", &NifDocument::WriteFloatInterpolator },
	{ "NiFloatData", &NifDocument::WriteFloatData },
	{ "NiMorphMeshModifier", &NifDocument::WriteMorphMeshModifier },
	{ "NiDataStream", &NifDocument::WriteDataStream }
};

template <typename T>
void write(std::ostream& stream, const T& value)
{
	const size_t length = sizeof(T);
	stream.write(reinterpret_cast<const char*>(&value), length);
}

void NifWriter::Write(std::ostream& stream)
{
	NifDocument document;

	std::map<size_t, BlockData*> nodeMap;
	std::map<size_t, BlockData*> materialMap;
	std::map<size_t, BlockData*> materialTextureMap;
	std::shared_ptr<Engine::Graphics::MeshFormat> format = GetNiMeshFormat();

	for (size_t i = 0; i < Package->Nodes.size(); ++i)
	{
		ModelPackageNode& node = Package->Nodes[i];

		BlockData& block = document.MakeBlock(node.Name);
		NiNodeType* nodeTypeData = nullptr;
		NiNode* nodeData = nullptr;
		NiMesh* meshData = nullptr;

		if (node.Mesh != nullptr)
		{
			meshData = block.MakeType<NiMesh>();
			nodeTypeData = meshData;
		}
		else
		{
			nodeData = block.MakeType<NiNode>();
			nodeTypeData = nodeData;
		}

		nodeTypeData->Transformation.Translation = node.Transform->GetTransformation().Translation();
		nodeTypeData->Transformation.Rotation.ExtractRotation(node.Transform->GetTransformation());

		Vector3SF scale = node.Transform->GetTransformation().ExtractScale();

		if (!(compare(scale.X, scale.Y) && compare(scale.Y, scale.Z)))
			std::cout << "warning: nonuniform scaling used in package exported to nif: " << node.Name << " " << scale << std::endl;

		nodeTypeData->Transformation.Scale = std::max(std::max(scale.X, scale.Y), scale.Z);

		if (node.AttachedTo == (size_t)-1)
		{
			nodeTypeData->Properties.push_back(document.MakeProperty<NiVertexColorProperty>("", 8));
			nodeTypeData->Properties.push_back(document.MakeProperty<NiZBufferProperty>("", 15));
		}

		nodeMap[i] = &block;

		if (node.AttachedTo != (size_t)-1)
		{
			auto parentIndex = nodeMap.find(node.AttachedTo);

			if (parentIndex != nodeMap.end())
				parentIndex->second->GetData<NiNode>()->Children.push_back(&block);
		}
		
		if (nodeData != nullptr)
		{
			for (size_t j = 0; j < Package->Nodes.size(); ++j)
			{
				size_t attachedTo = Package->Nodes[j].AttachedTo;

				if (attachedTo == i)
				{
					auto childIndex = nodeMap.find(j);

					if (childIndex != nodeMap.end())
						nodeData->Children.push_back(childIndex->second);
				}
			}
		}
		else
		{
			meshData->ExtraData.push_back(document.MakeExtraData<NiFloatExtraData>("FresnelBoost", 5));
			meshData->ExtraData.push_back(document.MakeExtraData<NiFloatExtraData>("FresnelExponent", 4));
			meshData->ExtraData.push_back(document.MakeExtraData<NiColorExtraData>("OverrideColor1", Color4(0.f, 1.f, 0.f, 1.f)));
			meshData->ExtraData.push_back(document.MakeExtraData<NiColorExtraData>("OverrideColor2", Color4(0.129f, 0.086f, 0.075f, 1.f)));
			meshData->ExtraData.push_back(document.MakeExtraData<NiColorExtraData>("OverrideColor0", Color4(0.702f, 0.471f, 0.369f, 1.f)));
			meshData->ExtraData.push_back(document.MakeExtraData<NiIntegerExtraData>("HairTangentMapIndex", 1));
			meshData->ExtraData.push_back(document.MakeExtraData<NiIntegerExtraData>("ColorOverrideMapIndex", 0));
			meshData->ExtraData.push_back(document.MakeExtraData<NiFloatExtraData>("ColorBoost", 1));

			BlockData& controllerBlock = document.MakeBlock("");
			NiMorphWeightsController* morphController = controllerBlock.MakeType<NiMorphWeightsController>();

			morphController->Target = &block;

			BlockData& interpolator1Block = document.MakeBlock("");
			NiFloatInterpolator* interpolator1 = interpolator1Block.MakeType<NiFloatInterpolator>();

			BlockData& interpolator1KeysBlock = document.MakeBlock("");
			NiFloatData* interpolator1Keys = interpolator1KeysBlock.MakeType<NiFloatData>();

			interpolator1Keys->Keys.push_back(NiFloatData::Key{ 0, 0 });
			interpolator1Keys->Keys.push_back(NiFloatData::Key{ 1e-5f, 0 });

			interpolator1->Data = &interpolator1KeysBlock;

			BlockData& interpolator2Block = document.MakeBlock("");
			NiFloatInterpolator* interpolator2 = interpolator2Block.MakeType<NiFloatInterpolator>();

			interpolator2->Value = 0.5f;

			morphController->Interpolators.push_back(&interpolator1Block);
			morphController->Interpolators.push_back(&interpolator2Block);

			morphController->TargetNames.push_back("Base");
			morphController->TargetNames.push_back(node.Name);

			meshData->Controller = &controllerBlock;
			meshData->Flags = 50; // magic number

			meshData->Properties.push_back(document.MakeProperty<NiSpecularProperty>("", 1));

			if (node.MaterialIndex != (size_t)-1)
			{
				ModelPackageMaterial& material = Package->Materials[node.MaterialIndex];

				auto materialIndex = materialMap.find(node.MaterialIndex);

				if (materialIndex == materialMap.end())
				{
					BlockData& materialBlock = document.MakeBlock(material.Name);
					NiMaterialProperty* materialData = materialBlock.MakeType<NiMaterialProperty>();

					materialData->DiffuseColor = material.DiffuseColor;
					materialData->SpecularColor = material.SpecularColor;
					materialData->AmbientColor = material.AmbientColor;
					materialData->EmissiveColor = material.EmissiveColor;
					materialData->Glossiness = material.Shininess;
					materialData->Alpha = material.Alpha;

					BlockData& texturesBlock = document.MakeBlock("");
					NiTexturingProperty* textures = texturesBlock.MakeType<NiTexturingProperty>();

					textures->Flags = 5;
					textures->TextureCount = 9;
					
					textures->BaseTexture.HasThisTexture = true;
					textures->BaseTexture.Flags = 12800;
					textures->BaseTexture.MaxAnisotropy = 1;

					BlockData& baseTextureBlock = document.MakeBlock(material.Diffuse);
					NiSourceTexture* baseTexture = baseTextureBlock.MakeType<NiSourceTexture>();

					baseTexture->FileName = material.Diffuse;
					baseTexture->PixelLayout = 6; // LAY_DEFAULT
					baseTexture->AlphaFormat = 3; // ALPHA_DEFAULT

					textures->BaseTexture.Source = &baseTextureBlock;

					textures->GlossTexture.HasThisTexture = true;
					textures->GlossTexture.Flags = 12800;
					textures->GlossTexture.MaxAnisotropy = 1;

					BlockData& glossTextureBlock = document.MakeBlock(material.Specular);
					NiSourceTexture* glossTexture = glossTextureBlock.MakeType<NiSourceTexture>();

					glossTexture->FileName = material.Specular;
					glossTexture->PixelLayout = 6; // LAY_DEFAULT
					glossTexture->AlphaFormat = 3; // ALPHA_DEFAULT

					textures->GlossTexture.Source = &glossTextureBlock;

					textures->NormalTexture.HasThisTexture = true;
					textures->NormalTexture.Flags = 12800;
					textures->NormalTexture.MaxAnisotropy = 1;

					BlockData& normalTextureBlock = document.MakeBlock(material.Normal);
					NiSourceTexture* normalTexture = normalTextureBlock.MakeType<NiSourceTexture>();

					normalTexture->FileName = material.Normal;
					normalTexture->PixelLayout = 6; // LAY_DEFAULT
					normalTexture->AlphaFormat = 3; // ALPHA_DEFAULT

					textures->NormalTexture.Source = &normalTextureBlock;
					
					// probably needs shader textures too, but those arent imported yet

					materialMap[node.MaterialIndex] = &materialBlock;
					materialTextureMap[node.MaterialIndex] = &texturesBlock;

					meshData->Properties.push_back(&materialBlock);
					meshData->Properties.push_back(&texturesBlock);
				}
				else
				{
					meshData->Properties.push_back(materialIndex->second);
					meshData->Properties.push_back(materialTextureMap[node.MaterialIndex]);
				}

				meshData->Materials.push_back(material.Name);
				meshData->MaterialExtraData.push_back(nullptr);
			}
			
			meshData->PrimitiveType = MeshPrimitiveType::Triangles;
			meshData->NumSubmeshes = 1;
			
			meshData->Streams.resize(6);

			meshData->Streams[0].SubmeshToRegionMap.push_back(0);
			meshData->Streams[0].ComponentSemantics.push_back(NiMesh::Semantics{ "INDEX", 0 });

			meshData->Streams[1].SubmeshToRegionMap.push_back(0);
			meshData->Streams[1].ComponentSemantics.push_back(NiMesh::Semantics{ "POSITION", 0 });

			meshData->Streams[2].SubmeshToRegionMap.push_back(0);
			meshData->Streams[2].ComponentSemantics.push_back(NiMesh::Semantics{ "NORMAL", 0 });
			meshData->Streams[2].ComponentSemantics.push_back(NiMesh::Semantics{ "TEXCOORD", 0 });
			meshData->Streams[2].ComponentSemantics.push_back(NiMesh::Semantics{ "BINORMAL", 0 });
			meshData->Streams[2].ComponentSemantics.push_back(NiMesh::Semantics{ "TANGENT", 0 });

			meshData->Streams[3].SubmeshToRegionMap.push_back(0);
			meshData->Streams[3].ComponentSemantics.push_back(NiMesh::Semantics{ "MORPH_POSITION", 0 });

			meshData->Streams[4].SubmeshToRegionMap.push_back(0);
			meshData->Streams[4].ComponentSemantics.push_back(NiMesh::Semantics{ "MORPH_POSITION", 1 });

			meshData->Streams[5].SubmeshToRegionMap.push_back(0);
			meshData->Streams[5].ComponentSemantics.push_back(NiMesh::Semantics{ "MORPH_WEIGHTS", 0 });

			BlockData& meshModifierBlock = document.MakeBlock("");
			NiMorphMeshModifier* meshModifier = meshModifierBlock.MakeType<NiMorphMeshModifier>();

			meshModifier->SubmitPoints.push_back(32816);
			meshModifier->CompletePoints.push_back(32832);
			meshModifier->Flags = 5;
			meshModifier->NumTargets = 2;
			meshModifier->Elements.push_back(NiMorphMeshModifier::ElementData{ { "POSITION", 0 }, 0 });

			meshData->Modifiers.push_back(&meshModifierBlock);

			size_t vertexCount = node.Mesh->GetVertices();

			const std::vector<int>& indexBuffer = node.Mesh->GetIndexBuffer();

			BlockData& stream1Block = document.MakeBlock("");
			NiDataStream* stream1 = stream1Block.MakeType<NiDataStream>();
			stream1Block.BlockType = "NiDataStream\0010\00119"s;

			stream1->CloningBehavior = CloningBehavior::Share;
			stream1->Regions.push_back(NiDataStream::Region{ 0, (unsigned int)indexBuffer.size()});
			stream1->ComponentFormats.push_back(ComponentFormat::F_UINT16_1);
			stream1->Streamable = true;
			stream1->StreamData.resize(2 * indexBuffer.size());

			unsigned short* outIndexBuffer = reinterpret_cast<unsigned short*>(stream1->StreamData.data());

			for (size_t i = 0; i < indexBuffer.size(); ++i)
				outIndexBuffer[i] = (unsigned short)indexBuffer[i];

			BlockData& stream2Block = document.MakeBlock("");
			NiDataStream* stream2 = stream2Block.MakeType<NiDataStream>();
			stream2Block.BlockType = "NiDataStream\0011\00121"s;

			stream2->CloningBehavior = CloningBehavior::BlankCopy;
			stream2->Regions.push_back(NiDataStream::Region{ 0, (unsigned int)vertexCount });
			stream2->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream2->Streamable = true;
			stream2->StreamData.resize(vertexCount * format->GetVertexSize(0));

			BlockData& stream3Block = document.MakeBlock("");
			NiDataStream* stream3 = stream3Block.MakeType<NiDataStream>();
			stream3Block.BlockType = "NiDataStream\0011\00119"s;

			stream3->CloningBehavior = CloningBehavior::Share;
			stream3->Regions.push_back(NiDataStream::Region{ 0, (unsigned int)vertexCount });
			stream3->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream3->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_2);
			stream3->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream3->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream3->Streamable = true;
			stream3->StreamData.resize(vertexCount * format->GetVertexSize(1));

			BlockData& stream4Block = document.MakeBlock("");
			NiDataStream* stream4 = stream4Block.MakeType<NiDataStream>();
			stream4Block.BlockType = "NiDataStream\0011\0013"s;

			stream4->CloningBehavior = CloningBehavior::Share;
			stream4->Regions.push_back(NiDataStream::Region{ 0, (unsigned int)vertexCount });
			stream4->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream4->Streamable = true;
			stream4->StreamData.resize(vertexCount * format->GetVertexSize(0));

			BlockData& stream5Block = document.MakeBlock("");
			NiDataStream* stream5 = stream5Block.MakeType<NiDataStream>();
			stream5Block.BlockType = "NiDataStream\0011\0013"s;

			stream5->CloningBehavior = CloningBehavior::Share;
			stream5->Regions.push_back(NiDataStream::Region{ 0, (unsigned int)vertexCount });
			stream5->ComponentFormats.push_back(ComponentFormat::F_FLOAT32_3);
			stream5->Streamable = true;
			stream5->StreamData.resize(vertexCount * format->GetVertexSize(2));

			void* vertexBuffers[] = { stream2->StreamData.data(), stream3->StreamData.data(), stream5->StreamData.data() };

			node.Format->Copy(node.Mesh->GetData(), vertexBuffers, format, vertexCount);

			stream4->StreamData = stream2->StreamData;

			BlockData& stream6Block = document.MakeBlock("");
			NiDataStream* stream6 = stream6Block.MakeType<NiDataStream>();
			stream6Block.BlockType = "NiDataStream\0013\0015"s;

			stream6->CloningBehavior = CloningBehavior::Copy;
			stream6->Regions.push_back(NiDataStream::Region{ 0, 2 });
			stream6->ComponentFormats.push_back(ComponentFormat::F_FLOAT_32_1);
			stream6->Streamable = true;
			stream6->StreamData.resize(2 * sizeof(float));

			*reinterpret_cast<Vector2SF*>(stream6->StreamData.data()) = Vector2SF(1.f, 0.5f);

			meshData->Streams[0].Stream = &stream1Block;
			meshData->Streams[1].Stream = &stream2Block;
			meshData->Streams[2].Stream = &stream3Block;
			meshData->Streams[3].Stream = &stream4Block;
			meshData->Streams[4].Stream = &stream5Block;
			meshData->Streams[5].Stream = &stream6Block;
		}
	}

	std::vector<std::stringstream> blockStreams(document.BlockMap.size());

	for (auto index = document.BlockMap.begin(); index != document.BlockMap.end(); ++index)
	{
		BlockData& block = index->second;

		size_t truncateIndex = 0;

		for (truncateIndex; truncateIndex < block.BlockType.size() && block.BlockType[truncateIndex] > 1; ++truncateIndex);

		std::string typeName = block.BlockType.substr(0, truncateIndex);

		auto iterator = writeFunctions.find(typeName);

		if (iterator == writeFunctions.end())
			throw "unsupported nif block type export";
		else
			(document.*(iterator->second))(blockStreams[index->first], block);

		auto addBlockType = [this, &block, &document]()
		{
			for (size_t i = 0; i < document.BlockTypes.size(); ++i)
			{
				if (document.BlockTypes[i] == block.BlockType)
				{
					document.BlockTypeIndices.push_back((unsigned short)i);

					return;
				}
			}

			document.BlockTypeIndices.push_back((unsigned short)document.BlockTypes.size());
			document.BlockTypes.push_back(block.BlockType);
		};

		addBlockType();

		blockStreams[index->first].seekg(0, std::ios::end);

		document.BlockSizes.push_back((unsigned int)blockStreams[index->first].tellg());
	}

	stream << "Gamebryo File Format, Version 30.2.0.3";

	char version[] = { 10, 3, 0, 2, 30, (unsigned char)(std::endian::native == std::endian::little) };

	stream.write(version, 6);

	unsigned int userVersion = 0;
	unsigned int numBlocks = (unsigned int)blockStreams.size();
	unsigned int metaBlockSize = 0;
	unsigned short numBlockTypes = (unsigned short)document.BlockTypes.size();

	size_t pos = stream.tellp();
	write(stream, userVersion);
	write(stream, numBlocks);
	write(stream, metaBlockSize);
	write(stream, numBlockTypes);
	pos = stream.tellp();

	for (size_t i = 0; i < document.BlockTypes.size(); ++i)
	{
		unsigned int length = (unsigned int)document.BlockTypes[i].size();

		write(stream, length);

		stream.write(document.BlockTypes[i].c_str(), length);
	}

	stream.write(reinterpret_cast<char*>(document.BlockTypeIndices.data()), numBlocks * sizeof(document.BlockTypeIndices[0]));
	stream.write(reinterpret_cast<char*>(document.BlockSizes.data()), numBlocks * sizeof(document.BlockSizes[0]));

	unsigned int numStrings = (unsigned int)document.Strings.size();
	unsigned int maxStringLength = 0;

	for (size_t i = 0; i < document.Strings.size(); ++i)
		maxStringLength = std::max(maxStringLength, (unsigned int)document.Strings[i].size());

	write(stream, numStrings);
	write(stream, maxStringLength);

	for (size_t i = 0; i < document.Strings.size(); ++i)
	{
		unsigned int length = (unsigned int)document.Strings[i].size();

		write(stream, length);

		stream.write(document.Strings[i].c_str(), length);
	}

	unsigned int numGroups = 0;

	write(stream, numGroups);

	for (size_t i = 0; i < blockStreams.size(); ++i)
	{
		std::string block = blockStreams[i].str();
		stream.write(block.c_str(), block.size());
	}

	blockStreams[0].tellg();
}

void NifDocument::WriteString(std::ostream& stream, const std::string& text)
{
	for (unsigned int i = 0; i < Strings.size(); ++i)
	{
		if (Strings[i] == text)
		{
			write(stream, i);

			return;
		}
	}

	unsigned int index = (unsigned int)Strings.size();
	Strings.push_back(text);

	write(stream, index);
}

void NifDocument::WriteRef(std::ostream& stream, const BlockData* block)
{
	if (block == nullptr)
		write(stream, (unsigned int)-1);
	else
		write(stream, block->BlockIndex);
}

void NifDocument::WriteMatrix(std::ostream& stream, const Matrix4F& block)
{
	write(stream, Vector3SF(block.RightVector()));
	write(stream, Vector3SF(block.UpVector()));
	write(stream, Vector3SF(block.FrontVector()));
}

void NifDocument::WriteNode(std::ostream& stream, BlockData& block)
{
	NiNode* data = block.GetData<NiNode>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
	write(stream, data->Transformation.Translation);
	WriteMatrix(stream, data->Transformation.Rotation);
	write(stream, data->Transformation.Scale);
	write(stream, (unsigned int)data->Properties.size());
	
	for (size_t i = 0; i < data->Properties.size(); ++i)
		WriteRef(stream, data->Properties[i]);

	WriteRef(stream, data->CollisionObject);

	write(stream, (unsigned int)data->Children.size());

	for (size_t i = 0; i < data->Children.size(); ++i)
		WriteRef(stream, data->Children[i]);


	write(stream, (unsigned int)data->Effects.size());

	for (size_t i = 0; i < data->Effects.size(); ++i)
		WriteRef(stream, data->Effects[i]);
}

void NifDocument::WriteMesh(std::ostream& stream, BlockData& block)
{
	NiMesh* data = block.GetData<NiMesh>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
	write(stream, data->Transformation.Translation);
	WriteMatrix(stream, data->Transformation.Rotation);
	write(stream, data->Transformation.Scale);
	write(stream, (unsigned int)data->Properties.size());

	for (size_t i = 0; i < data->Properties.size(); ++i)
		WriteRef(stream, data->Properties[i]);

	WriteRef(stream, data->CollisionObject);

	write(stream, (unsigned int)data->Materials.size());

	for (size_t i = 0; i < data->Materials.size(); ++i)
		WriteString(stream, data->Materials[i]);

	for (size_t i = 0; i < data->MaterialExtraData.size(); ++i)
		WriteRef(stream, data->MaterialExtraData[i]);

	write(stream, data->ActiveMaterial);
	write(stream, (unsigned char)data->MaterialNeedsUpdate);
	write(stream, (unsigned int)data->PrimitiveType);
	write(stream, data->NumSubmeshes);
	write(stream, (unsigned char)data->InstancingEnabled);
	write(stream, data->Bounds.Center);
	write(stream, data->Bounds.Radius);

	write(stream, (unsigned int)data->Streams.size());

	for (size_t i = 0; i < data->Streams.size(); ++i)
	{
		WriteRef(stream, data->Streams[i].Stream);
		write(stream, (unsigned char)data->Streams[i].IsPerInstance);

		write(stream, (unsigned short)data->Streams[i].SubmeshToRegionMap.size());

		for (size_t j = 0; j < data->Streams[i].SubmeshToRegionMap.size(); ++j)
			write(stream, data->Streams[i].SubmeshToRegionMap[j]);

		write(stream, (unsigned int)data->Streams[i].ComponentSemantics.size());

		for (size_t j = 0; j < data->Streams[i].ComponentSemantics.size(); ++j)
		{
			WriteString(stream, data->Streams[i].ComponentSemantics[j].Name);
			write(stream, data->Streams[i].ComponentSemantics[j].Index);
		}
	}

	write(stream, (unsigned int)data->Modifiers.size());

	for (size_t i = 0; i < data->Modifiers.size(); ++i)
		WriteRef(stream, data->Modifiers[i]);
}

void NifDocument::WriteVertexColorProperty(std::ostream& stream, BlockData& block)
{
	NiProperty* data = block.GetData<NiProperty>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
}

void NifDocument::WriteZBufferProperty(std::ostream& stream, BlockData& block)
{
	NiProperty* data = block.GetData<NiProperty>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
}

void NifDocument::WriteSpecularProperty(std::ostream& stream, BlockData& block)
{
	NiProperty* data = block.GetData<NiProperty>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
}

void NifDocument::WriteMaterialProperty(std::ostream& stream, BlockData& block)
{
	NiMaterialProperty* data = block.GetData<NiMaterialProperty>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->AmbientColor);
	write(stream, data->DiffuseColor);
	write(stream, data->SpecularColor);
	write(stream, data->EmissiveColor);
	write(stream, data->Glossiness);
	write(stream, data->Alpha);
}

void NifDocument::WriteTexturingProperty(std::ostream& stream, BlockData& block)
{
	NiTexturingProperty* data = block.GetData<NiTexturingProperty>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, data->Flags);
	write(stream, data->TextureCount);

	auto dumpTexture = [this, data, &stream](NiTexturingProperty::TextureData& texture)
	{
		write(stream, (unsigned char)texture.HasThisTexture);

		if (!texture.HasThisTexture) return;

		WriteRef(stream, texture.Source);
		write(stream, texture.Flags);
		write(stream, texture.MaxAnisotropy);

		write(stream, (unsigned char)texture.HasTextureTransform);

		if (!texture.HasTextureTransform) return;

		write(stream, texture.Translation);
		write(stream, texture.Scale);
		write(stream, texture.Rotation);
		write(stream, texture.TransformMethod);
		write(stream, texture.Center);
	};

	dumpTexture(data->BaseTexture);
	dumpTexture(data->DarkTexture);
	dumpTexture(data->DetailTexture);
	dumpTexture(data->GlossTexture);
	dumpTexture(data->GlowTexture);
	dumpTexture(data->BumpTexture);
	dumpTexture(data->NormalTexture);
	dumpTexture(data->ParallaxTexture);
	dumpTexture(data->Decal0Texture);

	write(stream, (unsigned int)data->ShaderTextures.size());

	for (size_t i = 0; i < data->ShaderTextures.size(); ++i)
	{
		dumpTexture(data->ShaderTextures[i].Map);
		write(stream, data->ShaderTextures[i].MapId);
	}
}

void NifDocument::WriteSourceTexture(std::ostream& stream, BlockData& block)
{
	NiSourceTexture* data = block.GetData<NiSourceTexture>();

	WriteString(stream, block.BlockName);

	write(stream, (unsigned int)data->ExtraData.size());

	for (size_t i = 0; i < data->ExtraData.size(); ++i)
		WriteRef(stream, data->ExtraData[i]);

	WriteRef(stream, data->Controller);
	write(stream, (unsigned char)data->UseExternal);
	WriteString(stream, data->FileName);
	WriteRef(stream, data->PixelData);
	write(stream, data->PixelLayout);
	write(stream, data->UseMipmaps);
	write(stream, data->AlphaFormat);
	write(stream, data->IsStatic);
	write(stream, (unsigned char)data->DirectRender);
	write(stream, (unsigned char)data->PersistRenderData);
}

void NifDocument::WriteFloatExtraData(std::ostream& stream, BlockData& block)
{
	NiFloatExtraData* data = block.GetData<NiFloatExtraData>();

	WriteString(stream, block.BlockName);
	write(stream, data->Value);
}

void NifDocument::WriteColorExtraData(std::ostream& stream, BlockData& block)
{
	NiColorExtraData* data = block.GetData<NiColorExtraData>();

	WriteString(stream, block.BlockName);
	write(stream, data->Value);
}

void NifDocument::WriteIntegerExtraData(std::ostream& stream, BlockData& block)
{
	NiIntegerExtraData* data = block.GetData<NiIntegerExtraData>();

	WriteString(stream, block.BlockName);
	write(stream, data->Value);
}

void NifDocument::WriteMorphWeightsController(std::ostream& stream, BlockData& block)
{
	NiMorphWeightsController* data = block.GetData<NiMorphWeightsController>();

	WriteRef(stream, data->NextController);
	write(stream, data->Flags);
	write(stream, data->Frequency);
	write(stream, data->Phase);
	write(stream, data->StartTime);
	write(stream, data->StopTime);
	WriteRef(stream, data->Target);
	write(stream, data->Count);

	write(stream, (unsigned int)data->Interpolators.size());

	for (size_t i = 0; i < data->Interpolators.size(); ++i)
		WriteRef(stream, data->Interpolators[i]);

	write(stream, (unsigned int)data->TargetNames.size());

	for (size_t i = 0; i < data->TargetNames.size(); ++i)
		WriteString(stream, data->TargetNames[i]);
}

void NifDocument::WriteFloatInterpolator(std::ostream& stream, BlockData& block)
{
	NiFloatInterpolator* data = block.GetData<NiFloatInterpolator>();

	write(stream, data->Value);
	WriteRef(stream, data->Data);
}

void NifDocument::WriteFloatData(std::ostream& stream, BlockData& block)
{
	NiFloatData* data = block.GetData<NiFloatData>();

	write(stream, (unsigned int)data->Keys.size());
	write(stream, data->Interpolation);

	for (size_t i = 0; i < data->Keys.size(); ++i)
	{
		write(stream, data->Keys[i].Time);
		write(stream, data->Keys[i].Value);
	}
}

void NifDocument::WriteMorphMeshModifier(std::ostream& stream, BlockData& block)
{
	NiMorphMeshModifier* data = block.GetData<NiMorphMeshModifier>();

	write(stream, (unsigned int)data->SubmitPoints.size());

	for (size_t i = 0; i < data->SubmitPoints.size(); ++i)
		write(stream, data->SubmitPoints[i]);

	write(stream, (unsigned int)data->CompletePoints.size());

	for (size_t i = 0; i < data->CompletePoints.size(); ++i)
		write(stream, data->CompletePoints[i]);

	write(stream, data->Flags);
	write(stream, data->NumTargets);

	write(stream, (unsigned int)data->Elements.size());

	for (size_t i = 0; i < data->Elements.size(); ++i)
	{
		WriteString(stream, data->Elements[i].Semantic.Name);
		write(stream, data->Elements[i].Semantic.Index);
		write(stream, data->Elements[i].NormalizeFlag);
	}
}

void NifDocument::WriteDataStream(std::ostream& stream, BlockData& block)
{
	NiDataStream* data = block.GetData<NiDataStream>();

	write(stream, (unsigned int)data->StreamData.size());
	write(stream, (unsigned int)data->CloningBehavior);

	write(stream, (unsigned int)data->Regions.size());

	for (size_t i = 0; i < data->Regions.size(); ++i)
	{
		write(stream, data->Regions[i].StartIndex);
		write(stream, data->Regions[i].NumIndices);
	}

	write(stream, (unsigned int)data->ComponentFormats.size());

	for (size_t i = 0; i < data->ComponentFormats.size(); ++i)
		write(stream, (unsigned int)data->ComponentFormats[i]);

	stream.write(data->StreamData.data(), data->StreamData.size());

	write(stream, (unsigned char)data->Streamable);
}
