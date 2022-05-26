#include "NifParser.h"

import <string>;
import <vector>;
import <map>;
import <set>;

#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector2S.h>
#include <Engine/Math/Matrix4.h>
#include <Engine/Assets/ParserUtils.h>
#include <Engine/Objects/Transform.h>

#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include "NifComponentInfo.h"
#include "NifBlockTypes.h"

void NifDocument::ParseNode(std::istream& stream, BlockData& block)
{
	NiNode* data = block.AddData<NiNode>();

	unsigned int numExtraData = Endian.read<unsigned int>(stream);

	data->ExtraData.resize(numExtraData);

	for (unsigned int i = 0; i < numExtraData; ++i)
		data->ExtraData[i] = &Blocks[Endian.read<unsigned int>(stream)];

	data->Controller = FetchRef(stream);
	data->Flags = Endian.read<unsigned short>(stream);
	data->Translation = Vector3SF(
		Endian.read<float>(stream),
		Endian.read<float>(stream),
		Endian.read<float>(stream)
	);

	std::swap(data->Translation.X, data->Translation.Z);
	
	data->Rotation = Matrix4F(
		Vector3F(),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		)
	);
	data->Scale = Endian.read<float>(stream);

	unsigned int numProperties = Endian.read<unsigned int>(stream);

	data->Properties.resize(numProperties);

	for (unsigned int i = 0; i < numProperties; ++i)
		data->Properties[i] = &Blocks[Endian.read<unsigned int>(stream)];

	data->CollisionObject = FetchRef(stream);

	unsigned int numChildren = Endian.read<unsigned int>(stream);

	data->Children.resize(numChildren);

	for (unsigned int i = 0; i < numChildren; ++i)
	{
		unsigned int childIndex = Endian.read<unsigned int>(stream);

		if (childIndex != (unsigned int)-1)
			data->Children[i] = &Blocks[childIndex];
	}

	unsigned int numEffects = Endian.read<unsigned int>(stream);

	data->Effects.resize(numEffects);

	for (unsigned int i = 0; i < numEffects; ++i)
		data->Effects[i] = &Blocks[Endian.read<unsigned int>(stream)];
}

void NifDocument::ParseMesh(std::istream& stream, BlockData& block)
{
	NiMesh* data = block.AddData<NiMesh>();

	unsigned int numExtraData = Endian.read<unsigned int>(stream);

	data->ExtraData.resize(numExtraData);

	for (unsigned int i = 0; i < numExtraData; ++i)
		data->ExtraData[i] = &Blocks[Endian.read<unsigned int>(stream)];

	data->Controller = FetchRef(stream);
	data->Flags = Endian.read<unsigned short>(stream);
	data->Translation = Vector3SF(
		Endian.read<float>(stream),
		Endian.read<float>(stream),
		Endian.read<float>(stream)
	);
	data->Rotation = Matrix4F(
		Vector3F(),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		),
		Vector3F(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		)
	);
	data->Scale = Endian.read<float>(stream);

	unsigned int numProperties = Endian.read<unsigned int>(stream);

	data->Properties.resize(numProperties);

	for (unsigned int i = 0; i < numProperties; ++i)
		data->Properties[i] = &Blocks[Endian.read<unsigned int>(stream)];

	data->CollisionObject = FetchRef(stream);

	unsigned int numMaterials = Endian.read<unsigned int>(stream);

	data->Materials.resize(numMaterials);
	data->MaterialExtraData.resize(numMaterials);

	for (unsigned int i = 0; i < numMaterials; ++i)
		data->Materials[i] = Strings[Endian.read<unsigned int>(stream)];

	for (unsigned int i = 0; i < numMaterials; ++i)
		data->MaterialExtraData[i] = FetchRef(stream);

	data->ActiveMaterial = Endian.read<unsigned int>(stream);
	data->MaterialNeedsUpdate = Endian.read<char>(stream);
	data->PrimitiveType = (MeshPrimitiveType)Endian.read<unsigned int>(stream);
	data->NumSubmeshes = Endian.read<unsigned short>(stream);
	data->InstancingEnabled = Endian.read<char>(stream);
	data->BoundingSphereCenter = Vector3SF(
		Endian.read<float>(stream),
		Endian.read<float>(stream),
		Endian.read<float>(stream)
	);
	data->BoundingSphereRadius = Endian.read<float>(stream);

	unsigned int numDataStreams = Endian.read<unsigned int>(stream);

	data->Streams.resize(numDataStreams);

	for (unsigned int i = 0; i < numDataStreams; ++i)
	{
		data->Streams[i].Stream = FetchRef(stream);
		data->Streams[i].IsPerInstance = Endian.read<char>(stream);
		
		unsigned short numSubMeshes = Endian.read<unsigned short>(stream);

		data->Streams[i].SubmeshToRegionMap.resize(numSubMeshes);

		for (unsigned short j = 0; j < numSubMeshes; ++j)
			data->Streams[i].SubmeshToRegionMap[j] = Endian.read<unsigned short>(stream);

		unsigned int numSemantics = Endian.read<unsigned int>(stream);

		data->Streams[i].ComponentSemantics.resize(numSemantics);

		for (unsigned int j = 0; j < numSemantics; ++j)
		{
			data->Streams[i].ComponentSemantics[j].Name = Strings[Endian.read<unsigned int>(stream)];
			data->Streams[i].ComponentSemantics[j].Index = Endian.read<unsigned int>(stream);
		}
	}

	unsigned int numModifiers = Endian.read<unsigned int>(stream);

	data->Modifiers.resize(numModifiers);

	for (unsigned int i = 0; i < numModifiers; ++i)
		data->Modifiers[i] = FetchRef(stream);
}

void NifDocument::ParseTexturingProperty(std::istream& stream, BlockData& block)
{
	NiTexturingProperty* data = block.AddData<NiTexturingProperty>();

	auto readTextureData = [&](NiTexturingProperty::TextureData& data)
	{
		data.HasThisTexture = Endian.read<char>(stream);

		if (!data.HasThisTexture) return;

		data.Source = FetchRef(stream);
		data.Flags = Endian.read<unsigned short>(stream);
		data.MaxAnisotropy = Endian.read<unsigned short>(stream);
		data.HasTextureTransform = Endian.read<char>(stream);

		if (data.HasTextureTransform)
		{
			data.Translation.Set(Endian.read<float>(stream), Endian.read<float>(stream));
			data.Scale.Set(Endian.read<float>(stream), Endian.read<float>(stream));
			data.Rotation = Endian.read<float>(stream);
			data.TransformMethod = Endian.read<unsigned int>(stream);
			data.Center.Set(Endian.read<float>(stream), Endian.read<float>(stream));
		}
	};

	ReadBlockRefs(stream, block, data->ExtraData);

	data->Controller = FetchRef(stream);
	data->Flags = Endian.read<unsigned short>(stream);
	data->TextureCount = Endian.read<unsigned int>(stream);

	readTextureData(data->BaseTexture);
	readTextureData(data->DarkTexture);
	readTextureData(data->DetailTexture);
	readTextureData(data->GlossTexture);
	readTextureData(data->GlowTexture);
	readTextureData(data->BumpTexture);
	readTextureData(data->NormalTexture);
	readTextureData(data->ParallaxTexture);
	readTextureData(data->Decal0Texture);

	unsigned int shaderTextureCount = Endian.read<unsigned int>(stream);

	data->ShaderTextures.resize(shaderTextureCount);

	for (unsigned int i = 0; i < shaderTextureCount; ++i)
	{
		readTextureData(data->ShaderTextures[i].Map);
		data->ShaderTextures[i].MapId = Endian.read<unsigned int>(stream);
	}
}

void NifDocument::ParseSourceTexture(std::istream& stream, BlockData& block)
{
	NiSourceTexture* data = block.AddData<NiSourceTexture>();

	ReadBlockRefs(stream, block, data->ExtraData);

	data->Controller = FetchRef(stream);
	data->UseExternal = Endian.read<unsigned char>(stream);
	
	unsigned int name = Endian.read<unsigned int>(stream);

	if (name != 0xFFFFFFFFu)
		data->FileName = Strings[name];

	data->PixelData = FetchRef(stream);
	data->PixelLayout = Endian.read<unsigned int>(stream);
	data->UseMipmaps = Endian.read<unsigned int>(stream);
	data->AlphaFormat = Endian.read<unsigned int>(stream);
	data->IsStatic = Endian.read<unsigned char>(stream);
	data->DirectRender = Endian.read<unsigned char>(stream);
	data->PersistRenderData = Endian.read<unsigned char>(stream);
}

void NifDocument::ParseStream(std::istream& stream, BlockData& block)
{
	NiDataStream* data = block.AddData<NiDataStream>();

	data->Usage = (StreamUsage)(block.BlockType[13] - '0'); // they embedded both Usage and Access in the name, ew
	data->StreamSize = Endian.read<unsigned int>(stream);
	data->CloningBehavior = (CloningBehavior)Endian.read<unsigned int>(stream);

	unsigned int numRegions = Endian.read<unsigned int>(stream);

	data->Regions.resize(numRegions);

	for (unsigned int i = 0; i < numRegions; ++i)
	{
		data->Regions[i].StartIndex = Endian.read<unsigned int>(stream);
		data->Regions[i].NumIndices = Endian.read<unsigned int>(stream);
	}

	unsigned int numComponents = Endian.read<unsigned int>(stream);

	data->ComponentFormats.resize(numComponents);
	data->Attributes.resize(numComponents);

	for (unsigned int i = 0; i < numComponents; ++i)
	{
		data->ComponentFormats[i] = (ComponentFormat)Endian.read<unsigned int>(stream);

		auto index = ComponentInfo.find(data->ComponentFormats[i]);

		if (index != ComponentInfo.end())
		{
			data->Attributes[i].Type = index->second.DataType;
			data->Attributes[i].ElementCount = index->second.ElementCount;
		}
		else
		{
			data->Attributes[i].Type = Enum::AttributeDataType::Unknown;
			data->Attributes[i].ElementCount = 0;
		}
	}

	data->StreamData.resize(data->StreamSize);

	stream.read(data->StreamData.data(), data->StreamSize);

	data->Streamable = Endian.read<char>(stream);
}

void NifDocument::ParseMaterialProperty(std::istream& stream, BlockData& block)
{
	NiMaterialProperty* data = block.AddData<NiMaterialProperty>();

	ReadBlockRefs(stream, block, data->ExtraData);

	data->Controller = FetchRef(stream);
	data->AmbientColor = Endian.read<Color3>(stream);
	data->DiffuseColor = Endian.read<Color3>(stream);
	data->SpecularColor = Endian.read<Color3>(stream);
	data->EmissiveColor = Endian.read<Color3>(stream);
	data->Glossiness = Endian.read<float>(stream);
	data->Alpha = Endian.read<float>(stream);
}

void NifDocument::ParserNoOp(std::istream& stream, BlockData& block)
{
	char buffer[0xFFF];

	for (unsigned int i = block.BlockStart; i < block.BlockSize;)
	{
		unsigned int amount = std::min(block.BlockSize - i, 0xFFFu);

		stream.read(buffer, amount);

		i += amount;
	}
}

std::map<std::string, NifDocument::BlockParseFunction> parserFunctions = {
	{ "NiNode", &NifDocument::ParseNode },
	{ "NiMesh", &NifDocument::ParseMesh },
	{ "NiTexturingProperty", &NifDocument::ParseTexturingProperty },
	{ "NiSourceTexture", &NifDocument::ParseSourceTexture },
	{ "NiDataStream", &NifDocument::ParseStream },
	{ "NiMaterialProperty", &NifDocument::ParseMaterialProperty },
};

std::set<std::string> ignoreBlockName = {
	"NiFloatInterpolator", "NiFloatData", "NiDataStream", "NiTextureTransformController"
};

std::map<std::string, std::string> attributeAliases = {
	{ "POSITION", "position" },
	{ "POSITION_BP", "position" },
	{ "NORMAL", "normal" },
	{ "NORMAL_BP", "normal" },
	{ "TEXCOORD", "textureCoords" },
	{ "BINORMAL", "binormal" },
	{ "TANGENT", "tangent" },
	{ "MORPH_POSITION", "morphPosition" }
};

using namespace Engine::Graphics;

void NifParser::Parse(std::istream& stream)
{
	NifDocument document;

	std::string headerString;

	const unsigned int bufferSize = 0xFFF;
	char buffer[bufferSize] = {};
	size_t size = 0;
	size_t index = 0;

	while (buffer[index] != 0x0A)
	{
		char next;
		stream.get(next);
		buffer[size] = next;
		++size;
		index = size - 1;
	}

	headerString.append(buffer, size - 1);

	stream.read(buffer, 4);

	stream.get(buffer[0]);
	
	Endian endian = Endian(buffer[0] ? std::endian::little : std::endian::big);
	document.Endian = endian;

	unsigned int userVersion = endian.read<unsigned int>(stream);
	unsigned int numBlocks = endian.read<unsigned int>(stream);
	unsigned int metaBlockSize = endian.read<unsigned int>(stream);
	
	for (unsigned int currentRemaining = metaBlockSize; currentRemaining <= metaBlockSize; currentRemaining -= bufferSize)
		stream.read(buffer, std::min(currentRemaining, bufferSize));

	unsigned short numBlockTypes = endian.read<unsigned short>(stream);

	document.BlockTypes.resize(numBlockTypes);
	document.BlockTypeIndices.resize(numBlocks);
	document.BlockSizes.resize(numBlocks);

	for (unsigned short i = 0; i < numBlockTypes; ++i)
	{
		unsigned int blockTypeSize = endian.read<unsigned int>(stream);

		if (blockTypeSize > 0)
		{
			stream.read(buffer, blockTypeSize);
			document.BlockTypes[i].append(buffer, blockTypeSize);
		}
	}

	for (unsigned int i = 0; i < numBlocks; ++i)
		document.BlockTypeIndices[i] = endian.read<unsigned short>(stream);

	for (unsigned int i = 0; i < numBlocks; ++i)
		document.BlockSizes[i] = endian.read<unsigned int>(stream);

	unsigned int numStrings = endian.read<unsigned int>(stream);
	unsigned int maxStringLength = endian.read<unsigned int>(stream);

	document.Strings.resize(numStrings);

	for (unsigned int i = 0; i < numStrings; ++i)
	{
		unsigned int stringLength = endian.read<unsigned int>(stream);

		if (stringLength > 0)
		{
			stream.read(buffer, stringLength);
			document.Strings[i].append(buffer, stringLength);
		}
	}

	unsigned int numGroups = endian.read<unsigned int>(stream);

	if (numGroups > 0)
		throw "WARNING, UNIMPLEMENTED";

	document.Blocks.resize(numBlocks);

	for (unsigned int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		BlockData& block = document.InitializeBlock(blockIndex);

		unsigned int position = (unsigned int)stream.tellg();

		size_t truncateIndex = 0;

		for (truncateIndex; truncateIndex < block.BlockType.size() && block.BlockType[truncateIndex] > 1; ++truncateIndex);

		std::string typeName = block.BlockType.substr(0, truncateIndex);

		if (block.BlockSize > 0)
		{
			auto iterator = ignoreBlockName.find(typeName);

			if (iterator == ignoreBlockName.end())
			{
				unsigned int name = endian.read<unsigned int>(stream);

				if (name != 0xFFFFFFFFu)
					block.BlockName = document.Strings[name];

				block.BlockStart = 4;
			}
		}

		if (block.BlockSize - block.BlockStart > 0)
		{
			auto iterator = parserFunctions.find(typeName);

			if (iterator == parserFunctions.end())
				document.ParserNoOp(stream, block);
			else
				(document.*(iterator->second))(stream, block);
		}

		unsigned int parsedInBlock = (unsigned int)stream.tellg() - position;

		if (parsedInBlock != block.BlockSize)
			throw "block parser read wrong amount";
	}

	std::map<unsigned int, BlockData*> parents;
	std::map<unsigned int, size_t> parentEntries;
	std::map<unsigned int, size_t> parentLinkTypes;

	for (unsigned int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
	{
		BlockData& block = document.Blocks[blockIndex];

		size_t parentIndex = (size_t)-1;
		auto parentEntryIndex = parentEntries.find(blockIndex);

		if (parentEntryIndex != parentEntries.end())
			parentIndex = parentEntryIndex->second;

		if (block.BlockType == "NiNode")
		{
			NiNode* data = block.Data->Cast<NiNode>();

			for (size_t i = 0; i < data->Children.size(); ++i)
			{
				if (data->Children[i] == nullptr) continue;

				unsigned int childBlock = data->Children[i]->BlockIndex;

				parents[childBlock] = &block;
				parentEntries[childBlock] = Package->Nodes.size();
			}

			std::shared_ptr<Engine::Transform> transform = Engine::Create<Engine::Transform>();
			transform->SetTransformation(Matrix4F(data->Translation) * data->Rotation * Matrix4F::NewScale(data->Scale, data->Scale, data->Scale));

			Package->Nodes.push_back(ModelPackageNode{ block.BlockName, parentIndex, (size_t)-1, nullptr, nullptr, transform });
		}
		else if (block.BlockType == "NiMesh")
		{
			NiMesh* data = block.Data->Cast<NiMesh>();

			size_t materialIndex = (size_t)-1;

			if (data->Materials.size() > 0)
			{
				for (size_t i = 0; i < data->Properties.size(); ++i)
				{
					if (data->Properties[i]->BlockType == "NiTexturingProperty")
					{
						auto textureIndex = parentEntries.find(data->Properties[i]->BlockIndex);

						if (textureIndex == parentEntries.end())
						{
							materialIndex = Package->Materials.size();
							parentEntries[data->Properties[i]->BlockIndex] = materialIndex;
							Package->Materials.push_back(ModelPackageMaterial{ data->Materials[0] });
						}
						else
							materialIndex = textureIndex->second;
					}
				}

				if (materialIndex != (size_t)-1)
					for (size_t i = 0; i < data->Properties.size(); ++i)
						if (data->Properties[i]->BlockType == "NiMaterialProperty")
							parentEntries[data->Properties[i]->BlockIndex] = materialIndex;
			}

			size_t indexBufferBinding = 0;
			std::vector<int> indexBuffer;

			for (size_t i = 0; i < data->Streams.size(); ++i)
			{
				NiDataStream* stream = data->Streams[i].Stream->Data->Cast<NiDataStream>();

				if (stream->Usage == StreamUsage::IndexBuffer)
				{
					NiDataStream* stream = data->Streams[i].Stream->Data->Cast<NiDataStream>();

					indexBufferBinding = i;

					size_t indexCount = stream->Regions[0].NumIndices;

					indexBuffer.resize(indexCount);

					const char* buffer = reinterpret_cast<const char*>(stream->StreamData.data());

					for (size_t j = 0; j < indexCount; ++j)
						stream->Attributes[0].Copy(buffer + j * stream->Attributes[0].GetSize(), indexBuffer.data() + j, Enum::AttributeDataType::Int32);

					break;
				}
			}

			std::vector<Engine::Graphics::VertexAttributeFormat> attributes;
			std::vector<void*> dataBuffers;

			size_t binding = 0;
			size_t vertexCount = 0;
			
			for (size_t i = 0; i < data->Streams.size(); ++i)
			{
				if (data->Streams[i].Stream->Data == nullptr) continue;

				NiDataStream* stream = data->Streams[i].Stream->Data->Cast<NiDataStream>();

				if (stream->Usage != StreamUsage::VertexBuffer) continue;

				size_t semanticsCount = data->Streams[i].ComponentSemantics.size();
				size_t attributeCount = stream->Attributes.size();

				vertexCount = stream->Regions[0].NumIndices;

				if (semanticsCount != attributeCount)
					throw "mismatching semantics and attributes";

				dataBuffers.push_back(stream->StreamData.data());

				for (size_t j = 0; j < semanticsCount; ++j)
				{
					auto index = attributeAliases.find(data->Streams[i].ComponentSemantics[j].Name);

					if (index == attributeAliases.end())
						stream->Attributes[j].Name = data->Streams[i].ComponentSemantics[j].Name;
					else
						stream->Attributes[j].Name = index->second;

					if (data->Streams[i].ComponentSemantics[j].Index != 0)
					{
						std::stringstream name;
						name << stream->Attributes[j].Name << data->Streams[i].ComponentSemantics[j].Index;
						stream->Attributes[j].Name = name.str();
					}

					stream->Attributes[j].Binding = binding;

					attributes.push_back(stream->Attributes[j]);
				}

				++binding;
			}

			std::shared_ptr<Engine::Transform> transform = Engine::Create<Engine::Transform>();
			transform->SetTransformation(Matrix4F(data->Translation) * data->Rotation * Matrix4F::NewScale(data->Scale, data->Scale, data->Scale));

			ImportedNiMesh mesh;

			mesh.Format = Engine::Graphics::MeshFormat::GetFormat(attributes);
			mesh.Mesh = Engine::Create<Engine::Graphics::MeshData>();
			mesh.Mesh->SetFormat(mesh.Format);
			mesh.Mesh->PushVertices(vertexCount, false);
			mesh.Mesh->PushIndices(indexBuffer);

			mesh.Format->Copy(dataBuffers.data(), mesh.Mesh->GetData(), mesh.Format, vertexCount);

			ImportedMeshes.push_back(mesh);

			Package->Nodes.push_back(ModelPackageNode{ block.BlockName, parentIndex, materialIndex, mesh.Format, mesh.Mesh, transform });
		}
		else if (block.BlockType == "NiTexturingProperty")
		{
			NiTexturingProperty* data = block.Data->Cast<NiTexturingProperty>();

			if (parentIndex != (size_t)-1)
			{
				if (data->BaseTexture.Source != nullptr)
				{
					parentEntries[data->BaseTexture.Source->BlockIndex] = parentIndex;
					parentLinkTypes[data->BaseTexture.Source->BlockIndex] = 0;
				}

				if (data->GlossTexture.Source != nullptr)
				{
					parentEntries[data->GlossTexture.Source->BlockIndex] = parentIndex;
					parentLinkTypes[data->GlossTexture.Source->BlockIndex] = 2;
				}

				if (data->NormalTexture.Source != nullptr)
				{
					parentEntries[data->NormalTexture.Source->BlockIndex] = parentIndex;
					parentLinkTypes[data->NormalTexture.Source->BlockIndex] = 1;
				}
			}
		}
		else if (block.BlockType == "NiSourceTexture")
		{
			NiSourceTexture* data = block.Data->Cast<NiSourceTexture>();

			auto linkTypeIndex = parentLinkTypes.find(blockIndex);

			if (parentIndex != (size_t)-1 && linkTypeIndex != parentLinkTypes.end())
			{
				switch (linkTypeIndex->second)
				{
				case 0: Package->Materials[parentIndex].Diffuse = data->FileName; break;
				case 1: Package->Materials[parentIndex].Normal = data->FileName; break;
				case 2: Package->Materials[parentIndex].Specular = data->FileName; break;
				}
			}
		}
		else if (block.BlockType == "NiMaterialProperty")
		{
			NiMaterialProperty* data = block.Data->Cast<NiMaterialProperty>();

			if (parentIndex != (size_t)-1)
			{
				Package->Materials[parentIndex].DiffuseColor = data->DiffuseColor;
				Package->Materials[parentIndex].SpecularColor = data->SpecularColor;
				Package->Materials[parentIndex].AmbientColor = data->AmbientColor;
				Package->Materials[parentIndex].EmissiveColor = data->EmissiveColor;
				Package->Materials[parentIndex].Shininess = data->Glossiness;
				Package->Materials[parentIndex].Alpha = data->Alpha;
			}
		}
	}
}