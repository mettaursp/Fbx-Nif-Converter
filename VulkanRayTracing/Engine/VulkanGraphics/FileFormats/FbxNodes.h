#pragma once

import <iostream>;
import <vector>;
import <map>;

#include <Engine/Assets/ParserUtils.h>
#include <Engine/Math/Matrix4-decl.h>
#include "FbxPropertyHandler.h"
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

struct FbxVersionEnum
{
	enum FbxVersion : int
	{
		Geometry = 124,
		GeometryShape = 100,
		GeometryNormal = 102,
		GeometryBinormal = 102,
		GeometryTangent = 102,
		GeometryUv = 101,
		GeometryMaterial = 101,
		Layer = 100,
		Model = 232,
		Pose = 100,
		DeformerSkin = 101,
		DeformerCluster = 100,
		DeformerShape = 100,
		DeformerShapeChannel = 100,
		Material = 102,
		Texture = 202,
		Templates = 100,
		Header = 1003,
		SceneInfo = 100
	};
};

typedef FbxVersionEnum::FbxVersion FbxVersion;

const Endian fbxEndian(std::endian::little);

struct NodeProperty
{
	char TypeCode = 0;
	std::vector<char> Data;
	std::vector<char> CompressedData;
	unsigned int ArrayLength = 0;
	unsigned int Encoding = 0;
	unsigned int CompressedLength = 0;

	void PushData(std::istream& stream, int length);
	void Write(std::ostream& out);
	void Compress();
	void InsertData(const char* data, size_t length, size_t arrayLength = 0);

	size_t GetSize() const;

	bool operator==(const std::string& text) const;
	bool operator!=(const std::string& text) const;
};

std::ostream& operator<<(std::ostream& out, const NodeProperty& property);

struct FbxNodeHeader
{
	std::string Name;
	unsigned long long NumProperties = 0;
	unsigned long long EndOffset = 0;
	unsigned long long StartOffset = 0;
	unsigned long long PropertySize = 0;
	unsigned char NameLength = 0;
	std::vector<NodeProperty> Properties;

	void Read(std::istream& stream, unsigned int version, char* buffer);

	bool IsNull() const;
};

struct FbxNode;

struct FbxObjectNode
{
	FbxNode* Parent = nullptr;
	long long Id = 0;
	size_t MeshIndex = 0;
	std::shared_ptr<Engine::Graphics::MeshData> Mesh;
	std::shared_ptr<Engine::Graphics::MeshFormat> Format;
	std::vector<FbxObjectNode*> ReferencedBy;
	std::vector<FbxObjectNode*> References;
	size_t BufferLength = 0;
	int* IndexBuffer = nullptr;
	double* WeightBuffer = nullptr;
	Matrix4D Transformation;

	FbxObjectNode* FindRef(const char* name, const char* type = nullptr);
	FbxObjectNode* FindRefBy(const char* name, const char* type = nullptr);
};

std::string vectorToString(const std::vector<char>& vector);

struct FbxNode
{
	FbxNodeHeader Header;
	std::unique_ptr<FbxObjectNode> ObjectNode;

	size_t Index = 0;
	size_t Parent = 0;
	size_t Depth = 0;
	bool ForceEndMarker = false;
	std::vector<size_t> ChildIndices;
	std::vector<FbxNode*> Children;

	FbxNode* Find(const std::string& name);
	FbxNode* FindWith(const std::string& text, size_t index = 0);
	NodeProperty* GetProperty(size_t index);
	FbxObjectNode* MakeObjectNode();

	void Write(std::ostream& out);
	void ComputeEnd(size_t start, std::vector<FbxNode>& nodes);
	void ComputeChildrenEnd(std::vector<FbxNode>& nodes);

	template <typename T>
	bool ReadPropertySafe(size_t index, T& output)
	{
		return PropertyHandler<T>::ReadSafe(GetProperty(index), output);
	}

	template <typename T>
	T ReadProperty(size_t index)
	{
		return PropertyHandler<T>::Read(GetProperty(index));
	}

	template <typename T>
	bool ReadChildWithPropertySafe(const std::string& name, size_t index, T& output, size_t tagIndex = 0)
	{
		FbxNode* child = FindWith(name, tagIndex);

		if (child == nullptr) return false;

		return child->ReadPropertySafe(index, output);
	}

	template <typename T>
	T ReadChildWithProperty(const std::string& name, size_t index, size_t tagIndex = 0)
	{
		FbxNode* child = FindWith(name, tagIndex);

		if (child == nullptr) throw "couldn't find child with property";

		return child->ReadProperty<T>(index);
	}
};

struct FbxObject
{
	std::string Name;
	std::shared_ptr<Engine::Graphics::MeshData> Mesh;
	std::shared_ptr<Engine::Graphics::MeshFormat> Format;
	Matrix4D Transformation;
	size_t MaterialUsed = (size_t)-1;
	long long MeshModel = 0;
};

struct FbxMaterial
{
	std::string Name;
	std::string Diffuse;
	std::string Normal;
	std::string Specular;
};

struct FbxConnection
{
	const char* Type = nullptr;
	long long Object1 = 0;
	long long Object2 = 0;
	const char* Data = nullptr;
};

struct FbxTimeStamp
{
	size_t Year = (size_t)-1;
	size_t Month = (size_t)-1;
	size_t Day = (size_t)-1;
	size_t Hour = (size_t)-1;
	size_t Minute = (size_t)-1;
	size_t Second = (size_t)-1;
	size_t Millisecond = (size_t)-1;
};

namespace
{
	template <typename Type>
	concept EnumType = std::is_enum_v<Type>;
}

struct FbxFileStructure
{
	bool DebugPrint = false;

	FbxNode RootNode;
	std::vector<FbxNode> Nodes;
	std::map<long long, FbxNode*> FbxObjectNodes;
	std::vector<FbxConnection> ObjectConnections;
	std::vector<FbxObject> FbxObjects;
	std::vector<FbxMaterial> FbxMaterials;
	std::map<std::string, size_t> ObjectDefinitions;
	Engine::Graphics::ModelPackage* Package = nullptr;

	FbxTimeStamp TimeStamp;

	size_t HeaderExtension = (size_t)-1;
	size_t FileId = (size_t)-1;
	size_t CreationTime = (size_t)-1;
	size_t Creator = (size_t)-1;
	size_t GlobalSettings = (size_t)-1;
	size_t Documents = (size_t)-1;
	size_t References = (size_t)-1;
	size_t Definitions = (size_t)-1;
	size_t Objects = (size_t)-1;
	size_t Connections = (size_t)-1;
	size_t Takes = (size_t)-1;

	size_t ObjectCount = (size_t)-1;

	void ReadNodes(std::istream& stream);
	void WriteNodes(std::ostream& stream);
	void MakeFileStructure();
	void FinalizeNodes();
	size_t AddNode(const std::string& name, FbxNode* parent = nullptr);
	size_t AddNode(const std::string& name, size_t parent, bool allowNull = false);
	size_t AddDefinition(const std::string& typeName);
	size_t AddObject(const std::string& name, size_t parent);
	size_t AddCategory(const std::string& name);

	void AddConnection(const char* type, long long object1, long long object2, const char* data = nullptr);

	FbxNode* Node(size_t index);

	size_t AddProperty(size_t node, const char* property)
	{
		size_t length = 0;

		for (; property[length]; ++length);

		PropertyHandler<ArrayWrapper<char>>::Add(Node(node), ArrayWrapper<char>{ property, length });

		return node;
	}

	size_t AddProperty(size_t node, const std::string& property)
	{
		PropertyHandler<ArrayWrapper<char>>::Add(Node(node), ArrayWrapper<char>{ property.c_str(), property.size() });

		return node;
	}

	template <EnumType T>
	size_t AddProperty(size_t node, const T& property)
	{
		PropertyHandler<int>::Add(Node(node), property);

		return node;
	}

	template <typename T>
	size_t AddProperty(size_t node, const T& property)
	{
		PropertyHandler<T>::Add(Node(node), property);

		return node;
	}

	template <typename... T>
	size_t AddProperties(size_t node, const T&... properties)
	{
		(AddProperty(node, properties), ...);

		return node;
	}
};
