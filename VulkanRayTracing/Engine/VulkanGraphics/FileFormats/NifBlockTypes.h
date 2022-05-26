#pragma once

import <vector>;
import <string>;
import <memory>;
import <limits>;

#include <Engine/Assets/ParserUtils.h>
#include <Engine/Math/Vector2S.h>
#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Color4.h>
#include <Engine/Math/Color3.h>
#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include "NifComponentInfo.h"

struct BlockData;
struct NifDocument;

struct NiDataBlock {
	unsigned int BlockIndex = 0;
	NifDocument* Document = nullptr;

	virtual ~NiDataBlock() {}

	BlockData* Get();

	template <typename T>
	T* Cast()
	{
		return reinterpret_cast<T*>(this);
	}

	template <typename T>
	T* Cast() const
	{
		return reinterpret_cast<const T*>(this);
	}
};

struct BlockData
{
	std::string BlockType;
	std::string BlockName;
	NifDocument* Document = nullptr;
	unsigned int BlockIndex = 0;
	unsigned int BlockSize = 0;
	std::unique_ptr<NiDataBlock> Data = nullptr;
	unsigned int BlockStart = 0;

	template <typename T, typename... Arguments>
	T* AddData(const Arguments&... arguments)
	{
		std::unique_ptr<T> data = std::make_unique<T>(arguments...);
	
		data->BlockIndex = BlockIndex;
		data->Document = Document;
	
		T* pointer = data.get();
	
		Data = std::move(data);
		
		return pointer;
	}

	template <typename T, typename... Arguments>
	T* MakeType(const Arguments&... arguments)
	{
		T* data = AddData<T>(arguments...);

		BlockType = T::BlockTypeName;

		return data;
	}

	template <typename T>
	T* GetData()
	{
		return reinterpret_cast<T*>(Data.get());
	}

	template <typename T>
	const T* GetData() const
	{
		return reinterpret_cast<const T*>(Data.get());
	}
};

struct MeshPrimitiveTypeEnum
{
	enum MeshPrimitiveType
	{
		Triangles,
		TriStrips,
		Lines,
		LineStrips,
		Quads,
		Points
	};
};

typedef MeshPrimitiveTypeEnum::MeshPrimitiveType MeshPrimitiveType;

struct CloningBehaviorEnum
{
	enum CloningBehavior
	{
		Share,
		Copy,
		BlankCopy
	};
};

typedef CloningBehaviorEnum::CloningBehavior CloningBehavior;

struct NiNodeType : public NiDataBlock
{
	std::vector<const BlockData*> ExtraData;
	const BlockData* Controller = nullptr;
	unsigned short Flags = 0;
	Vector3SF Translation;
	Matrix4F Rotation;
	float Scale = 1;
	std::vector<const BlockData*> Properties;
	const BlockData* CollisionObject = nullptr;
};

struct NiNode : public NiNodeType
{
	static inline const std::string BlockTypeName = "NiNode";

	std::vector<const BlockData*> Children;
	std::vector<const BlockData*> Effects;
};

struct NiMesh : public NiNodeType
{
	static inline const std::string BlockTypeName = "NiMesh";

	struct Semantics
	{
		std::string Name;
		unsigned int Index = 0;
	};

	struct DataStreams
	{
		const BlockData* Stream = nullptr;
		bool IsPerInstance = false;
		std::vector<unsigned short> SubmeshToRegionMap;
		std::vector<Semantics> ComponentSemantics;
	};

	std::vector<std::string> Materials;
	std::vector<const BlockData*> MaterialExtraData;
	unsigned int ActiveMaterial = 0;
	bool MaterialNeedsUpdate = false;
	MeshPrimitiveType PrimitiveType;
	unsigned short NumSubmeshes = 0;
	bool InstancingEnabled = false;
	Vector3SF BoundingSphereCenter;
	float BoundingSphereRadius = 1;
	std::vector<DataStreams> Streams;
	std::vector<const BlockData*> Modifiers;
};

struct NiMorphMeshModifier : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiMorphMeshModifier";

	struct ElementData
	{
		NiMesh::Semantics Semantic;
		unsigned int NormalizeFlag = 0;
	};

	std::vector<unsigned short> SubmitPoints;
	std::vector<unsigned short> CompletePoints;
	unsigned char Flags = 0;
	unsigned short NumTargets = 0;
	std::vector<ElementData> Elements;
};

struct NiMorphWeightsController : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiMorphWeightsController";

	const BlockData* NextController = nullptr;
	unsigned short Flags = 0;
	float Frequency = 1;
	float Phase = 0;
	float StartTime = 0;
	float StopTime = 1e-5f;
	const BlockData* Target = nullptr;
	unsigned int Count = 0;
	std::vector<const BlockData*> Interpolators;
	std::vector<std::string> TargetNames;
};

struct NiFloatInterpolator : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiFloatInterpolator";

	float Value = std::numeric_limits<float>::min();
	const BlockData* Data = nullptr;
};

struct NiFloatData : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiFloatData";

	struct Key
	{
		float Time = 0;
		float Value = 0;
	};

	unsigned int Interpolation = 0;

	std::vector<Key> Keys;
};

struct NiMaterialProperty : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiMaterialProperty";

	std::vector<const BlockData*> ExtraData;
	const BlockData* Controller = nullptr;
	Color3 AmbientColor = Color3(0.7f, 0.7f, 0.7f);
	Color3 DiffuseColor = Color3(0.7f, 0.7f, 0.7f);
	Color3 SpecularColor = Color3(1.f, 1.f, 1.f);
	Color3 EmissiveColor = Color3(0.f, 0.f, 0.f);
	float Glossiness = 10;
	float Alpha = 1;
};

struct NiTexturingProperty : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiTexturingProperty";

	struct TextureData
	{
		bool HasThisTexture = false;
		const BlockData* Source = nullptr;
		unsigned short Flags = 0;
		unsigned short MaxAnisotropy = 1;
		bool HasTextureTransform = false;
		Vector2SF Translation;
		Vector2SF Scale = Vector2SF(1, 1);
		float Rotation = 0;
		unsigned int TransformMethod = 0;
		Vector2SF Center = Vector2SF(0.5f, 0.5f);
	};

	struct ShaderTextureData
	{
		TextureData Map;
		unsigned int MapId = 0;
	};

	std::vector<const BlockData*> ExtraData;
	const BlockData* Controller = nullptr;
	unsigned short Flags = 0;
	unsigned int TextureCount = 0;
	TextureData BaseTexture;
	TextureData DarkTexture;
	TextureData DetailTexture;
	TextureData GlossTexture;
	TextureData GlowTexture;
	TextureData BumpTexture;
	TextureData NormalTexture;
	TextureData ParallaxTexture;
	TextureData Decal0Texture;
	std::vector<ShaderTextureData> ShaderTextures;
};

struct NiSourceTexture : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiSourceTexture";

	std::vector<const BlockData*> ExtraData;
	const BlockData* Controller = nullptr;
	unsigned char UseExternal = 1;
	std::string FileName;
	const BlockData* PixelData = nullptr;
	unsigned int PixelLayout = 0;
	unsigned int UseMipmaps = 0;
	unsigned int AlphaFormat = 0;
	unsigned char IsStatic = 1;
	bool DirectRender = false;
	bool PersistRenderData = false;
};

template <typename T>
struct NiExtraData : public NiDataBlock
{
	T Value;
};

struct NiFloatExtraData : public NiExtraData<float>
{
	typedef float ValueType;

	static inline const std::string BlockTypeName = "NiFloatExtraData";
};

struct NiColorExtraData : public NiExtraData<Color4>
{
	typedef Color4 ValueType;

	static inline const std::string BlockTypeName = "NiColorExtraData";
};

struct NiIntegerExtraData : public NiExtraData<unsigned int>
{
	typedef unsigned int ValueType;

	static inline const std::string BlockTypeName = "NiIntegerExtraData";
};

struct NiProperty : public NiDataBlock
{
	std::vector<const BlockData*> ExtraData;
	const BlockData* Controller = nullptr;
	unsigned short Flags = 0;
};

struct NiVertexColorProperty : public NiProperty
{
	static inline const std::string BlockTypeName = "NiVertexColorProperty";
};

struct NiZBufferProperty : public NiProperty
{
	static inline const std::string BlockTypeName = "NiZBufferProperty";
};

struct NiSpecularProperty : public NiProperty
{
	static inline const std::string BlockTypeName = "NiSpecularProperty";
};

struct NiAlphaProperty : public NiProperty
{
	static inline const std::string BlockTypeName = "NiAlphaProperty";

	unsigned char Threshold = 0;
};

struct StreamUsageEnum
{
	enum StreamUsage
	{
		IndexBuffer,
		VertexBuffer,
		ShaderConstant,
		User
	};
};

typedef StreamUsageEnum::StreamUsage StreamUsage;

struct NiDataStream : public NiDataBlock
{
	static inline const std::string BlockTypeName = "NiDataStream";

	struct Region
	{
		unsigned int StartIndex = 0;
		unsigned int NumIndices = 0;
	};

	unsigned int StreamSize = 0;
	CloningBehavior CloningBehavior;
	std::vector<Region> Regions;
	std::vector<ComponentFormat> ComponentFormats;
	std::vector<char> StreamData;
	StreamUsage Usage;
	bool Streamable = false;
	std::vector<Engine::Graphics::VertexAttributeFormat> Attributes;
};

struct NifDocument
{
	typedef void (NifDocument::* BlockParseFunction)(std::istream& stream, BlockData& block);
	typedef void (NifDocument::* BlockWriteFunction)(std::ostream& stream, BlockData& block);

	std::vector<std::string> BlockTypes;
	std::vector<unsigned short> BlockTypeIndices;
	std::vector<unsigned int> BlockSizes;
	std::vector<std::string> Strings;
	std::vector<BlockData> Blocks;
	std::map<unsigned short, BlockData> BlockMap;
	Endian Endian;

	void ParserNoOp(std::istream& stream, BlockData& block);
	void ParseStream(std::istream& stream, BlockData& block);
	void ParseSourceTexture(std::istream& stream, BlockData& block);
	void ParseTexturingProperty(std::istream& stream, BlockData& block);
	void ParseMesh(std::istream& stream, BlockData& block);
	void ParseNode(std::istream& stream, BlockData& block);
	void ParseMaterialProperty(std::istream& stream, BlockData& block);

	void WriteNode(std::ostream& stream, BlockData& block);
	void WriteMesh(std::ostream& stream, BlockData& block);
	void WriteVertexColorProperty(std::ostream& stream, BlockData& block);
	void WriteZBufferProperty(std::ostream& stream, BlockData& block);
	void WriteSpecularProperty(std::ostream& stream, BlockData& block);
	void WriteMaterialProperty(std::ostream& stream, BlockData& block);
	void WriteTexturingProperty(std::ostream& stream, BlockData& block);
	void WriteSourceTexture(std::ostream& stream, BlockData& block);
	void WriteFloatExtraData(std::ostream& stream, BlockData& block);
	void WriteColorExtraData(std::ostream& stream, BlockData& block);
	void WriteIntegerExtraData(std::ostream& stream, BlockData& block);
	void WriteMorphWeightsController(std::ostream& stream, BlockData& block);
	void WriteFloatInterpolator(std::ostream& stream, BlockData& block);
	void WriteFloatData(std::ostream& stream, BlockData& block);
	void WriteMorphMeshModifier(std::ostream& stream, BlockData& block);
	void WriteDataStream(std::ostream& stream, BlockData& block);

	void WriteString(std::ostream& stream, const std::string& text);
	void WriteRef(std::ostream& stream, const BlockData* block);
	void WriteMatrix(std::ostream& stream, const Matrix4F& block);

	template <typename T>
	BlockData* MakeExtraData(const std::string& name, const typename T::ValueType& value)
	{
		BlockData& block = MakeBlock(name);
		T* data = block.MakeType<T>();
		
		data->Value = value;

		return &block;
	}

	template <typename T>
	BlockData* MakeProperty(const std::string& name, unsigned int flags)
	{
		BlockData& block = MakeBlock(name);
		T* data = block.MakeType<T>();

		data->Flags = flags;

		return &block;
	}

	BlockData& MakeBlock(const std::string& name);
	BlockData& InitializeBlock(unsigned int blockIndex);
	const BlockData* FetchRef(unsigned int ref);
	const BlockData* FetchRef(std::istream& stream);
	void ReadBlockRefs(std::istream& stream, BlockData& block, std::vector<const BlockData*>& refs);
};

std::shared_ptr<Engine::Graphics::MeshFormat> GetNiMeshFormat();