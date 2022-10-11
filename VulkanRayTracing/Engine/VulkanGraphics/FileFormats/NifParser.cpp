#include "NifParser.h"

import <string>;
import <vector>;
import <map>;
import <set>;

#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector2S.h>
#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Quaternion.h>
#include <Engine/Assets/ParserUtils.h>
#include <Engine/Objects/Transform.h>

#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include "NifComponentInfo.h"
#include "NifBlockTypes.h"

void NifDocument::ParseTransform(std::istream& stream, NiTransform& transform, bool translationFirst, bool isQuaternion)
{
	if (translationFirst)
	{
		transform.Translation = Vector3SF(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		);
	}

	if (!isQuaternion)
	{
		transform.Rotation = Matrix4F(
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
	}
	else
	{
		transform.Rotation = Quaternion(Vector3(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		)).MatrixF();
	}

	if (!translationFirst)
	{
		transform.Translation = Vector3SF(
			Endian.read<float>(stream),
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		);
	}

	std::swap(transform.Translation.X, transform.Translation.Z);

	transform.Scale = Endian.read<float>(stream);
}

void NifDocument::ParseBounds(std::istream& stream, NiBounds& bounds)
{
	bounds.Center = Vector3SF(
		Endian.read<float>(stream),
		Endian.read<float>(stream),
		Endian.read<float>(stream)
	);
	bounds.Radius = Endian.read<float>(stream);

	std::swap(bounds.Center.X, bounds.Center.Z);
}

void NifDocument::ParseNode(std::istream& stream, BlockData& block)
{
	NiNode* data = block.AddData<NiNode>();

	unsigned int numExtraData = Endian.read<unsigned int>(stream);

	data->ExtraData.resize(numExtraData);

	for (unsigned int i = 0; i < numExtraData; ++i)
		data->ExtraData[i] = &Blocks[Endian.read<unsigned int>(stream)];

	data->Controller = FetchRef(stream);
	data->Flags = Endian.read<unsigned short>(stream);
	
	ParseTransform(stream, data->Transformation);

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

	ParseTransform(stream, data->Transformation);

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

	ParseBounds(stream, data->Bounds);

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

	if (data->BumpTexture.HasThisTexture)
	{
		data->BumpMapLumaScale = Endian.read<float>(stream);
		data->BumpMapLumaOffset = Endian.read<float>(stream);
		data->BumpMapRight.Set(
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		);
		data->BumpMapUp.Set(
			Endian.read<float>(stream),
			Endian.read<float>(stream)
		);
	}

	readTextureData(data->NormalTexture);
	readTextureData(data->ParallaxTexture);
	readTextureData(data->Decal0Texture);

	unsigned int shaderTextureCount = Endian.read<unsigned int>(stream);

	data->ShaderTextures.resize(shaderTextureCount);

	for (unsigned int i = 0; i < shaderTextureCount; ++i)
	{
		readTextureData(data->ShaderTextures[i].Map);

		if (data->ShaderTextures[i].Map.HasThisTexture)
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

void NifDocument::ParseSkinningMeshModifier(std::istream& stream, BlockData& block)
{
	NiSkinningMeshModifier* data = block.AddData<NiSkinningMeshModifier>();

	unsigned int numSubmitPoints = Endian.read<unsigned int>(stream);

	for (unsigned int i = 0; i < numSubmitPoints; ++i)
		data->SubmitPoints.push_back(Endian.read<unsigned short>(stream));

	unsigned int numCompletePoints = Endian.read<unsigned int>(stream);

	for (unsigned int i = 0; i < numCompletePoints; ++i)
		data->CompletePoints.push_back(Endian.read<unsigned short>(stream));

	data->Flags = Endian.read<unsigned short>(stream);
	data->SkeletonRoot = FetchRef(stream);

	ParseTransform(stream, data->SkeletonTransformation, false);

	unsigned int numBones = Endian.read<unsigned int>(stream);

	for (unsigned int i = 0; i < numBones; ++i)
		data->Bones.push_back(FetchRef(stream));

	for (unsigned int i = 0; i < numBones; ++i)
	{
		data->BoneTransforms.push_back(NiTransform{});

		ParseTransform(stream, data->BoneTransforms.back(), false);
	}

	for (unsigned int i = 0; i < numBones; ++i)
	{
		data->BoneBounds.push_back(NiBounds{});

		ParseBounds(stream, data->BoneBounds.back());
	}
}

void NifDocument::ParseSequenceData(std::istream& stream, BlockData& block)
{
	NiSequenceData* data = block.AddData<NiSequenceData>();

	unsigned int numEvaluators = Endian.read<unsigned int>(stream);

	data->Evaluators.resize(numEvaluators);

	for (unsigned int i = 0; i < numEvaluators; ++i)
		data->Evaluators[i] = FetchRef(stream);

	data->TextKeys = FetchRef(stream);
	data->Duration = Endian.read<float>(stream);
	data->CycleType = (CycleType)Endian.read<unsigned int>(stream);
	data->Frequency = Endian.read<float>(stream);

	unsigned int name = Endian.read<unsigned int>(stream);

	if (name != 0xFFFFFFFFu)
		data->AccumRootName = Strings[name];

	data->AccumFlags = (AccumFlags)Endian.read<unsigned int>(stream);
}

void NifDocument::ParseEvaluator(std::istream& stream, BlockData& block, NiEvaluator* data)
{
	data->NodeName = FetchString(stream);
	data->PropertyType = FetchString(stream);
	data->ControllerType = FetchString(stream);
	data->ControllerId = FetchString(stream);
	data->InterpolatorId = FetchString(stream);
	
	unsigned char positionChannel = Endian.read<unsigned char>(stream);
	unsigned char rotationChannel = Endian.read<unsigned char>(stream);
	unsigned char scaleChannel = Endian.read<unsigned char>(stream);
	unsigned char flags = Endian.read<unsigned char>(stream);

	data->PositionPosed = positionChannel & 0x40;
	data->PositionChannel = (ChannelType)(positionChannel & 0x2F);
	data->RotationPosed = rotationChannel & 0x40;
	data->RotationChannel = (ChannelType)(rotationChannel & 0x2F);
	data->ScalePosed = scaleChannel & 0x40;
	data->ScaleChannel = (ChannelType)(scaleChannel & 0x2F);

	data->ChannelFlags = (ChannelTypeFlags)flags;
}

void NifDocument::ParseBSplineCompTransformEvaluator(std::istream& stream, BlockData& block)
{
	NiBSplineCompTransformEvaluator* data = block.AddData<NiBSplineCompTransformEvaluator>();

	ParseEvaluator(stream, block, data);

	data->StartTime = Endian.read<float>(stream);
	data->EndTime = Endian.read<float>(stream);
	data->Data = FetchRef(stream);
	data->BasisData = FetchRef(stream);

	ParseTransform(stream, data->Transform, true, true);

	data->TranslationHandle = Endian.read<unsigned int>(stream);
	data->RotationHandle = Endian.read<unsigned int>(stream);
	data->ScaleHandle = Endian.read<unsigned int>(stream);
	data->TranslationOffset = Endian.read<float>(stream);
	data->TranslationHalfRange = Endian.read<float>(stream);
	data->RotationOffset = Endian.read<float>(stream);
	data->RotationHalfRange = Endian.read<float>(stream);
	data->ScaleOffset = Endian.read<float>(stream);
	data->ScaleHalfRange = Endian.read<float>(stream);
}

void NifDocument::ParseBSpineData(std::istream& stream, BlockData& block)
{
	NiBSpineData* data = block.AddData<NiBSpineData>();

	unsigned int numFloatControlPoints = Endian.read<unsigned int>(stream);

	data->FloatControlPoints.resize(numFloatControlPoints);

	for (unsigned int i = 0; i < numFloatControlPoints; ++i)
		data->FloatControlPoints[i] = Endian.read<float>(stream);

	unsigned int numCompactControlPoints = Endian.read<unsigned int>(stream);

	data->CompactControlPoints.resize(numCompactControlPoints);

	for (unsigned int i = 0; i < numCompactControlPoints; ++i)
		data->CompactControlPoints[i] = Endian.read<short>(stream);
}

void NifDocument::ParseBSplineBasisData(std::istream& stream, BlockData& block)
{
	NiBSplineBasisData* data = block.AddData<NiBSplineBasisData>();

	data->NumControlPoints = Endian.read<unsigned int>(stream);
}

void NifDocument::ParseTransformEvaluator(std::istream& stream, BlockData& block)
{
	NiTransformEvaluator* data = block.AddData<NiTransformEvaluator>();

	ParseEvaluator(stream, block, data);
	ParseTransform(stream, data->Value, true, true);

	data->Data = FetchRef(stream);
}

template <>
float ParseKey<float>(NifDocument* document, std::istream& stream)
{
	return document->Endian.read<float>(stream);
}

template <>
Quaternion ParseKey<Quaternion>(NifDocument* document, std::istream& stream)
{
	return Quaternion(Vector3(
		document->Endian.read<float>(stream),
		document->Endian.read<float>(stream),
		document->Endian.read<float>(stream),
		document->Endian.read<float>(stream)
	));
}

template <>
Vector3F ParseKey<Vector3F>(NifDocument* document, std::istream& stream)
{
	return Vector3F(
		document->Endian.read<float>(stream),
		document->Endian.read<float>(stream),
		document->Endian.read<float>(stream)
	);
}


void NifDocument::ParseTransformData(std::istream& stream, BlockData& block)
{
	NiTransformData* data = block.AddData<NiTransformData>();

	data->RotationKeys.Parse(this, stream);
	data->TranslationKeys.Parse(this, stream);
	data->ScaleKeys.Parse(this, stream);
}

void NifDocument::ParseTextKeyExtraData(std::istream& stream, BlockData& block)
{
	NiTextKeyExtraData* data = block.AddData<NiTextKeyExtraData>();

	unsigned int length = Endian.read<unsigned int>(stream);

	data->TextKeys.resize(length);

	for (unsigned int i = 0; i < length; ++i)
	{
		data->TextKeys[i].Time = Endian.read<float>(stream);
		data->TextKeys[i].Value = FetchString(stream);
	}
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
	{ "NiSkinningMeshModifier", &NifDocument::ParseSkinningMeshModifier },
	{ "NiSequenceData", &NifDocument::ParseSequenceData },
	{ "NiBSplineCompTransformEvaluator", &NifDocument::ParseBSplineCompTransformEvaluator },
	{ "NiBSpineData", &NifDocument::ParseBSpineData },
	{ "NiBSplineBasisData", &NifDocument::ParseBSplineBasisData },
	{ "NiTransformEvaluator", &NifDocument::ParseTransformEvaluator },
	{ "NiTransformData", &NifDocument::ParseTransformData },
	{ "NiTextKeyExtraData", &NifDocument::ParseTextKeyExtraData },
};

std::set<std::string> ignoreBlockName = {
	"NiFloatInterpolator",
	"NiFloatData",
	"NiDataStream",
	"NiTextureTransformController",
	"NiTransformController",
	"NiTransformInterpolator",
	"NiSkinningMeshModifier",
	"NiBSplineCompTransformEvaluator",
	"NiBSplineData",
	"NiBSplineBasisData",
	"NiTransformEvaluator",
	"NiTransformData"
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

	if (numBlockTypes == 0)
		return;

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
		document.BlockTypeIndices[i] = 0x7FFF & endian.read<unsigned short>(stream);

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
			auto parserIterator = parserFunctions.find(typeName);

			if (parserIterator == parserFunctions.end())
				document.ParserNoOp(stream, block);
			else
			{
				auto iterator = ignoreBlockName.find(typeName);

				if (iterator == ignoreBlockName.end())
				{
					unsigned int name = endian.read<unsigned int>(stream);

					if (name != 0xFFFFFFFFu)
						block.BlockName = document.Strings[name];

					block.BlockStart = 4;
				}

				(document.*(parserIterator->second))(stream, block);
			}
		}

		unsigned int parsedInBlock = (unsigned int)stream.tellg() - position;

		if (parsedInBlock != block.BlockSize)
		{
			stream.seekg(position + block.BlockStart);

			auto parserIterator = parserFunctions.find(typeName);

			if (parserIterator == parserFunctions.end())
				document.ParserNoOp(stream, block);
			else
				(document.*(parserIterator->second))(stream, block);

			throw "block parser read wrong amount";
		}
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

			NiTransform& nodeTransform = data->Transformation;

			std::shared_ptr<Engine::Transform> transform = Engine::Create<Engine::Transform>();
			transform->SetTransformation(Matrix4F(nodeTransform.Translation) * nodeTransform.Rotation * Matrix4F::NewScale(nodeTransform.Scale, nodeTransform.Scale, nodeTransform.Scale));
			transform->SetInheritsTransformation((data->Flags & 0x4) != 0);
			transform->Name = block.BlockName;

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

			NiTransform& nodeTransform = data->Transformation;

			std::shared_ptr<Engine::Transform> transform = Engine::Create<Engine::Transform>();
			transform->SetTransformation(Matrix4F(nodeTransform.Translation) * nodeTransform.Rotation * Matrix4F::NewScale(nodeTransform.Scale, nodeTransform.Scale, nodeTransform.Scale));
			transform->SetInheritsTransformation((data->Flags & 0x4) != 0);
			transform->Name = block.BlockName;

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
				const auto fetchPath = [this](const NiTexturingProperty::TextureData& texture) -> const std::string&
				{
					static const std::string empty("");

					if (texture.Source == nullptr) return empty;

					const NiSourceTexture* data = texture.Source->GetData<NiSourceTexture>();

					return data->FileName;
				};

				Package->Materials[parentIndex].Diffuse = fetchPath(data->BaseTexture);
				Package->Materials[parentIndex].Normal = fetchPath(data->NormalTexture);
				Package->Materials[parentIndex].Specular = fetchPath(data->GlossTexture);

				if (data->ShaderTextures.size() > 0)
				{
					if (data->ShaderTextures.size() > 1)
					{
						parentIndex += 0;
					}

					Package->Materials[parentIndex].OverrideColor = fetchPath(data->ShaderTextures[0].Map);
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

	for (size_t i = 0; i < Package->Nodes.size(); ++i)
		if (Package->Nodes[i].AttachedTo != (size_t)-1)
			Package->Nodes[i].Transform->SetParent(Package->Nodes[Package->Nodes[i].AttachedTo].Transform);
}