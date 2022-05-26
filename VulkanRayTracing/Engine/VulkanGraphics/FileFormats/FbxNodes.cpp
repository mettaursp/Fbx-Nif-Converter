#include "FbxNodes.h"

#include <zlib.h>
import <sstream>;
import <iomanip>;

#include <Engine/VulkanGraphics/Scene/MeshData.h>
#include <Engine/Objects/Transform.h>
#include <Engine/Math/Vector2S.h>

void NodeProperty::PushData(std::istream& stream, int length)
{
	if (!Encoding)
	{
		Data.resize(length);
		stream.read(Data.data(), length);
	}
	else
	{
		unsigned int entryLength = 4;

		if (TypeCode == 'd' || TypeCode == 'l')
			entryLength = 8;
		else if (TypeCode == 'b')
			entryLength = 1;

		CompressedData.resize(length);
		stream.read(CompressedData.data(), length);

		Data.resize((size_t)((float)(ArrayLength * entryLength) * 1.1f) + 16);

		uLongf sizeUncompressed = (uLongf)Data.size();
		int result = uncompress(reinterpret_cast<Bytef*>(Data.data()), &sizeUncompressed, reinterpret_cast<Bytef*>(CompressedData.data()), (uLong)CompressedData.size());

		switch (result)
		{
		case Z_OK:

			break;
		case Z_MEM_ERROR:
			throw "unhandled zlib memory error";

			break;
		case Z_BUF_ERROR:
			throw "unhandled zlib buffer error";

			break;

		default:
			throw "unhandled zlib error";
		}
	}
}

void NodeProperty::Compress()
{
	CompressedData.resize((size_t)((float)Data.size() * 1.1f) + 12);

	uLongf sizeCompressed = (uLongf)CompressedData.size();
	int result = compress(reinterpret_cast<Bytef*>(CompressedData.data()), &sizeCompressed, reinterpret_cast<Bytef*>(Data.data()), (uLong)Data.size());

	switch (result)
	{
	case Z_OK:

		break;
	case Z_MEM_ERROR:
		throw "unhandled zlib memory error";

		break;
	case Z_BUF_ERROR:
		throw "unhandled zlib buffer error";

		break;

	default:
		throw "unhandled zlib error";
	}

	Encoding = 1;
	CompressedLength = (unsigned int)sizeCompressed;
}

void NodeProperty::InsertData(const char* data, size_t length, size_t arrayLength)
{
	Data.resize(length);
	
	for (size_t i = 0; i < length; ++i)
		Data[i] = data[i];

	ArrayLength = (unsigned int)arrayLength;
}

size_t NodeProperty::GetSize() const
{
	size_t size = sizeof(char);

	if (TypeCode == 'Y')
		size += sizeof(short);
	else if (TypeCode == 'C')
		size += sizeof(unsigned char);
	else if (TypeCode == 'I')
		size += sizeof(int);
	else if (TypeCode == 'F')
		size += sizeof(float);
	else if (TypeCode == 'D')
		size += sizeof(double);
	else if (TypeCode == 'L')
		size += sizeof(long long);
	else if (TypeCode == 'f' || TypeCode == 'd' || TypeCode == 'l' || TypeCode == 'i' || TypeCode == 'b')
	{
		size += sizeof(unsigned int);
		size += sizeof(unsigned int);
		size += sizeof(unsigned int);

		if (Encoding)
			size += (size_t)CompressedLength;
		else
		{
			size_t entryLength = 4;

			if (TypeCode == 'd' || TypeCode == 'l')
				entryLength = 8;
			else if (TypeCode == 'b')
				entryLength = 1;

			size += entryLength * ArrayLength;
		}
	}
	else if (TypeCode == 'S' || TypeCode == 'R')
		size += sizeof(unsigned int) + Data.size();

	return size;
}

void NodeProperty::Write(std::ostream& out)
{
	out.write(&TypeCode, 1);

	if (TypeCode == 'Y')
		out.write(Data.data(), sizeof(short));
	else if (TypeCode == 'C')
		out.write(Data.data(), sizeof(unsigned char));
	else if (TypeCode == 'I')
		out.write(Data.data(), sizeof(int));
	else if (TypeCode == 'F')
		out.write(Data.data(), sizeof(float));
	else if (TypeCode == 'D')
		out.write(Data.data(), sizeof(double));
	else if (TypeCode == 'L')
		out.write(Data.data(), sizeof(long long));
	else if (TypeCode == 'f' || TypeCode == 'd' || TypeCode == 'l' || TypeCode == 'i' || TypeCode == 'b')
	{
		size_t entryLength = 4;

		if (TypeCode == 'd' || TypeCode == 'l')
			entryLength = 8;
		else if (TypeCode == 'b')
			entryLength = 1;

		if (CompressedLength == 0)
			CompressedLength = ArrayLength * (unsigned int)entryLength;

		out.write(reinterpret_cast<char*>(&ArrayLength), sizeof(ArrayLength));
		out.write(reinterpret_cast<char*>(&Encoding), sizeof(Encoding));
		out.write(reinterpret_cast<char*>(&CompressedLength), sizeof(CompressedLength));

		if (Encoding)
			out.write(CompressedData.data(), CompressedLength);
		else
		{

			out.write(Data.data(), entryLength * ArrayLength);
		}
	}
	else if (TypeCode == 'S' || TypeCode == 'R')
	{
		unsigned int length = (unsigned int)Data.size();

		out.write(reinterpret_cast<char*>(&length), sizeof(length));
		out.write(Data.data(), Data.size());
	}
}

bool NodeProperty::operator==(const std::string& text) const
{
	if (TypeCode != 'S') return false;

	return text.compare(0, text.size(), Data.data(), ArrayLength) == 0;
}

bool NodeProperty::operator!=(const std::string& text) const
{
	return !(*this == text);
}

std::ostream& operator<<(std::ostream& out, const NodeProperty& property)
{
	char typeCode = property.TypeCode;

	out << typeCode << " ";

	if (typeCode == 'Y')
		out << fbxEndian.read<short>(property.Data.data());
	else if (typeCode == 'C')
		out << fbxEndian.read<unsigned char>(property.Data.data());
	else if (typeCode == 'I')
		out << fbxEndian.read<int>(property.Data.data());
	else if (typeCode == 'F')
		out << fbxEndian.read<float>(property.Data.data());
	else if (typeCode == 'D')
		out << fbxEndian.read<double>(property.Data.data());
	else if (typeCode == 'L')
		out << fbxEndian.read<long long>(property.Data.data());
	else if (typeCode == 'f' || typeCode == 'd' || typeCode == 'l' || typeCode == 'i' || typeCode == 'b')
	{
		unsigned int entryLength = 4;

		if (typeCode == 'd' || typeCode == 'l')
			entryLength = 8;
		else if (typeCode == 'b')
			entryLength = 1;

		out << "< ";

		for (unsigned int i = 0; i < property.ArrayLength && i < 4 && ((i + 1) * entryLength - 1) < property.Data.size(); ++i)
		{
			if (typeCode == 'f')
				out << fbxEndian.read<float>(property.Data.data() + i * entryLength);
			if (typeCode == 'd')
				out << fbxEndian.read<double>(property.Data.data() + i * entryLength);
			else if (typeCode == 'l')
				out << fbxEndian.read<long long>(property.Data.data() + i * entryLength);
			else if (typeCode == 'i')
				out << fbxEndian.read<int>(property.Data.data() + i * entryLength);
			else if (typeCode == 'b')
				out << fbxEndian.read<unsigned char>(property.Data.data() + i * entryLength);

			out << ", ";
		}

		if (property.ArrayLength >= 4)
			out << "... ";

		out << ">";

		if (property.Encoding)
			out << " Compressed";
	}
	else if (typeCode == 'S')
		out << "\"" << std::string(property.Data.data(), property.Data.size()) << "\"";
	else if (typeCode == 'R')
		out << "<raw data>, ";
	else
		out << "<unknown>, ";

	return out;
}

void FbxNodeHeader::Read(std::istream& stream, unsigned int version, char* buffer)
{
	if (version >= 7500)
	{
		EndOffset = fbxEndian.read<unsigned long long>(stream);
		NumProperties = fbxEndian.read<unsigned long long>(stream);
		PropertySize = fbxEndian.read<unsigned long long>(stream);
	}
	else
	{
		EndOffset = fbxEndian.read<unsigned int>(stream);
		NumProperties = fbxEndian.read<unsigned int>(stream);
		PropertySize = fbxEndian.read<unsigned int>(stream);
	}

	unsigned char nodeNameLength = fbxEndian.read<unsigned char>(stream);

	if (nodeNameLength > 0)
	{
		stream.read(buffer, nodeNameLength);

		Name.append(buffer, nodeNameLength);
	}

	Properties.resize(NumProperties);

	for (unsigned long long i = 0; i < NumProperties; ++i)
	{
		char typeCode = fbxEndian.read<char>(stream);

		Properties[i].TypeCode = typeCode;

		if (typeCode == 'Y')
			Properties[i].PushData(stream, 2);
		else if (typeCode == 'C')
			Properties[i].PushData(stream, 1);
		else if (typeCode == 'I' || typeCode == 'F')
			Properties[i].PushData(stream, 4);
		else if (typeCode == 'D' || typeCode == 'L')
			Properties[i].PushData(stream, 8);
		else if (typeCode == 'f' || typeCode == 'd' || typeCode == 'l' || typeCode == 'i' || typeCode == 'b')
		{
			unsigned int entryLength = 4;

			if (typeCode == 'd' || typeCode == 'l')
				entryLength = 8;
			else if (typeCode == 'b')
				entryLength = 1;

			Properties[i].ArrayLength = fbxEndian.read<unsigned int>(stream);
			Properties[i].Encoding = fbxEndian.read<unsigned int>(stream);
			Properties[i].CompressedLength = fbxEndian.read<unsigned int>(stream);

			if (Properties[i].Encoding == 0)
				Properties[i].PushData(stream, entryLength * Properties[i].ArrayLength);
			else
				Properties[i].PushData(stream, Properties[i].CompressedLength);
		}
		else if (typeCode == 'S' || typeCode == 'R')
		{
			Properties[i].ArrayLength = fbxEndian.read<unsigned int>(stream);
			Properties[i].PushData(stream, Properties[i].ArrayLength);
		}
	}
}

bool FbxNodeHeader::IsNull() const
{
	return EndOffset == 0 && NumProperties == 0 && PropertySize == 0 && NameLength == 0;
}

FbxNode* FbxNode::Find(const std::string& name)
{
	for (size_t i = 0; i < Children.size(); ++i)
		if (Children[i]->Header.Name == name)
			return Children[i];

	return nullptr;
};

FbxNode* FbxNode::FindWith(const std::string& text, size_t index)
{
	for (size_t i = 0; i < Children.size(); ++i)
		if (Children[i]->Header.Properties.size() > index && Children[i]->Header.Properties[index] == text)
			return Children[i];

	return nullptr;
}

NodeProperty* FbxNode::GetProperty(size_t index)
{
	if (Header.Properties.size() <= index) return nullptr;

	return &Header.Properties[index];
}

FbxObjectNode* FbxNode::MakeObjectNode()
{
	if (ObjectNode == nullptr)
		ObjectNode = std::make_unique<FbxObjectNode>();

	ObjectNode->Parent = this;

	return ObjectNode.get();
}

void FbxNode::Write(std::ostream& out)
{
	unsigned long long currentPos = (unsigned long long)out.tellp();

	if (currentPos != Header.StartOffset)
		throw "mismatching start offset";

	unsigned char nameLength = (unsigned char)Header.Name.size();

	out.write(reinterpret_cast<char*>(&Header.EndOffset), sizeof(Header.EndOffset));
	out.write(reinterpret_cast<char*>(&Header.NumProperties), sizeof(Header.NumProperties));
	out.write(reinterpret_cast<char*>(&Header.PropertySize), sizeof(Header.PropertySize));
	out.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
	out.write(Header.Name.c_str(), Header.Name.size());

	for (size_t i = 0; i < Header.Properties.size(); ++i)
		Header.Properties[i].Write(out);

	for (size_t i = 0; i < Children.size(); ++i)
		Children[i]->Write(out);

	char nullNode[3 * sizeof(Header.EndOffset) + sizeof(unsigned char)] = {0};

	if (Children.size() > 0)
		out.write(nullNode, sizeof(nullNode));

	currentPos = (unsigned long long)out.tellp();

	if (currentPos != Header.EndOffset)
		throw "mismatching end offset";
}

void FbxNode::ComputeEnd(size_t start, std::vector<FbxNode>& nodes)
{
	Header.StartOffset = (unsigned long long)start;
	Header.NameLength = (unsigned char)Header.Name.size();

	Children.resize(ChildIndices.size());

	for (size_t i = 0; i < Children.size(); ++i)
		Children[i] = &nodes[ChildIndices[i]];

	size_t end = start;
	end += sizeof(unsigned long long); // EndOffset
	end += sizeof(unsigned long long); // NumProperties
	end += sizeof(unsigned long long); // PropertySize
	end += sizeof(unsigned char); // NameLength
	end += (size_t)Header.NameLength;
	
	Header.PropertySize = 0;
	Header.NumProperties = (unsigned long long)Header.Properties.size();

	for (unsigned long long i = 0; i < Header.NumProperties; ++i)
	{
		size_t size = Header.Properties[i].GetSize();

		Header.PropertySize += (unsigned long long)size;
		end += size;
	}

	for (size_t i = 0; i < Children.size(); ++i)
	{
		Children[i]->ComputeEnd(end, nodes);
		end = Children[i]->Header.EndOffset;
	}

	if (Children.size() > 0)
		end += 3 * sizeof(unsigned long long) + sizeof(unsigned char);

	Header.EndOffset = (unsigned long long)end;
}

void FbxNode::ComputeChildrenEnd(std::vector<FbxNode>& nodes)
{
	Children.resize(ChildIndices.size());

	if (ChildIndices.size() > 0)
		Header.EndOffset = std::max(Header.EndOffset, nodes[ChildIndices.back()].Header.EndOffset + 3 * sizeof(unsigned long long) + sizeof(unsigned char));
	
	for (size_t i = 0; i < Children.size(); ++i)
		Children[i] = &nodes[ChildIndices[i]];
}

void FbxFileStructure::ReadNodes(std::istream& stream)
{
	const size_t bufferSize = 0xFFF;
	char buffer[bufferSize] = {};

	stream.read(buffer, 27);

	unsigned int version = fbxEndian.read<unsigned int>(buffer + 23);
	unsigned long long currentPos = (unsigned long long)stream.tellg();

	std::vector<size_t> nodeStack;

	bool reachedEnd = false;
	int nullNodes = 0;

	while (Nodes.size() == 0 || nodeStack.size() > 0 || !reachedEnd)
	{
		size_t index = Nodes.size();
		size_t parentIndex = 0;
	
		if (nodeStack.size() > 0)
			parentIndex = nodeStack.back();
	
		nodeStack.push_back(index);
		Nodes.push_back(FbxNode());
	
		FbxNode& node = Nodes.back();
	
		node.Index = index;
		node.Parent = parentIndex;
		node.Header.StartOffset = currentPos;
		node.Depth = nodeStack.size();
		node.Header.Read(stream, version, buffer);

		if (node.Header.IsNull())
			++nullNodes;
	
		if (nodeStack.size() == 1 && node.Header.IsNull())
			reachedEnd = true;
		else if (!node.Header.IsNull())
		{
			if (nodeStack.size() > 1)
				Nodes[parentIndex].ChildIndices.push_back(index);
			else
				RootNode.ChildIndices.push_back(index);
		}
	
		currentPos = (unsigned long long)stream.tellg();
	
		while (nodeStack.size() > 0 && currentPos >= Nodes[nodeStack.back()].Header.EndOffset)
		{
			if (currentPos != Nodes[nodeStack.back()].Header.EndOffset && !Nodes[nodeStack.back()].Header.IsNull())
				std::cout << "warning: currentPos & EndOffset mismatch: " << currentPos << ", " << Nodes[nodeStack.back()].Header.EndOffset << std::endl;
	
			nodeStack.pop_back();
		}
	}

	if (DebugPrint)
		std::cout << "null nodes: " << nullNodes << std::endl;

	unsigned long long beforeHeader = (unsigned long long)stream.tellg();

	stream.read(buffer, bufferSize); // footer

	currentPos = (unsigned long long)stream.tellg();

	auto diff1 = stream.gcount();

	unsigned long long diff = currentPos - beforeHeader;
}

void encrypt(char* bytes, const char* key)
{
	char c = 64;
	for (int i = 0; i < 16; ++i)
	{
		bytes[i] = (char)(bytes[i] ^ (char)(c ^ key[i]));
		c = bytes[i];
	}
}

void FbxFileStructure::WriteNodes(std::ostream& out)
{
	char buffer[0xFF] = { 'K', 'a', 'y', 'd', 'a', 'r', 'a', ' ', 'F', 'B', 'X', ' ', 'B', 'i', 'n', 'a', 'r', 'y', ' ', ' ', 0, 0x1A, 0 };

	*reinterpret_cast<unsigned int*>(buffer + 23) = 7700; // TODO: fix endian later

	out.write(buffer, 27);

	for (size_t i = 0; i < RootNode.ChildIndices.size(); ++i)
		Nodes[RootNode.ChildIndices[i]].Write(out);

	char nullNode[3 * sizeof(unsigned long long) + sizeof(unsigned char)] = { 0 };

	out.write(nullNode, sizeof(nullNode));

	char sourceId[] = { (char)0x58, (char)0xAB, (char)0xA9, (char)0xF0, (char)0x6C, (char)0xA2, (char)0xD8, (char)0x3F, (char)0x4D, (char)0x47, (char)0x49, (char)0xA3, (char)0xB4, (char)0xB2, (char)0xE7, (char)0x3D };
	const char key[] = { (char)0xE2, (char)0x4F, (char)0x7B, (char)0x5F, (char)0xCD, (char)0xE4, (char)0xC8, (char)0x6D, (char)0xDB, (char)0xD8, (char)0xFB, (char)0xD7, (char)0x40, (char)0x58, (char)0xC6, (char)0x78 };

	long long year = 		  (long long)fbxEndian.read<int>(Node(TimeStamp.Year 		 )->Header.Properties[0].Data.data());
	long long month = 	  (long long)fbxEndian.read<int>(Node(TimeStamp.Month 		 )->Header.Properties[0].Data.data());
	long long day = 		  (long long)fbxEndian.read<int>(Node(TimeStamp.Day 		 )->Header.Properties[0].Data.data());
	long long hour = 		  (long long)fbxEndian.read<int>(Node(TimeStamp.Hour 		 )->Header.Properties[0].Data.data());
	long long minute = 	  (long long)fbxEndian.read<int>(Node(TimeStamp.Minute 	 )->Header.Properties[0].Data.data());
	long long second = 	  (long long)fbxEndian.read<int>(Node(TimeStamp.Second 	 )->Header.Properties[0].Data.data());
	long long millisecond = (long long)fbxEndian.read<int>(Node(TimeStamp.Millisecond )->Header.Properties[0].Data.data());

	long long timeStamps[] = { month, hour, day, (millisecond / 10) % 100, year, minute };
	size_t count = sizeof(timeStamps) / sizeof(timeStamps[0]);

	long long mangledTime = second;

	for (size_t i = 0; i < count; ++i)
		mangledTime = (mangledTime * (i == 4 ? 10000 : 100)) + timeStamps[i];

	std::stringstream mangled;
	mangled << mangledTime;

	encrypt(sourceId, mangled.str().c_str());
	encrypt(sourceId, key);
	encrypt(sourceId, mangled.str().c_str());

	out.write(sourceId, 16);

	const size_t footerZeros1 = 20;
	const size_t footerZeros2 = 120;
	
	char zeros[std::max(footerZeros1, footerZeros2)] = { 0 };

	out.write(zeros, footerZeros1);
	out.write(buffer + 23, 4);
	out.write(zeros, footerZeros2);

	char extension[] = { (char)0xF8, (char)0x5A, (char)0x8C, (char)0x6A, (char)0xDE, (char)0xF5, (char)0xD9, (char)0x7E, (char)0xEC, (char)0xE9, (char)0x0C, (char)0xE3, (char)0x75, (char)0x8F, (char)0x29, (char)0x0B };

	out.write(extension, sizeof(extension));
}

using Engine::Graphics::VertexAttributeFormat;

const std::shared_ptr<Engine::Graphics::MeshFormat>& GetFbxMeshFormat()
{
	static std::shared_ptr<Engine::Graphics::MeshFormat> format;

	if (format != nullptr) return format;

	std::vector<VertexAttributeFormat> attributes;

	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "position", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "normal", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 2, "textureCoords", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "binormal", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "tangent", attributes.size() });


	format = Engine::Graphics::MeshFormat::GetFormat(attributes);

	return format;
}

const std::shared_ptr<Engine::Graphics::MeshFormat>& GetFbxMeshKeyFormat()
{
	static std::shared_ptr<Engine::Graphics::MeshFormat> format;

	if (format != nullptr) return format;

	std::vector<VertexAttributeFormat> attributes;

	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "position", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "normal", attributes.size() });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "morphPosition1", attributes.size() });

	format = Engine::Graphics::MeshFormat::GetFormat(attributes);

	return format;
}

size_t FbxFileStructure::AddDefinition(const std::string& typeName)
{
	size_t objectType = AddProperty(AddNode("ObjectType", Definitions), typeName);
	size_t count = AddProperty(AddNode("Count", objectType), 0);

	ObjectDefinitions[typeName] = count;

	return objectType;
}

size_t FbxFileStructure::AddObject(const std::string& name, size_t parent)
{
	size_t object = AddNode(name, parent);

	auto index = ObjectDefinitions.find(name);

	if (index != ObjectDefinitions.end())
		++*reinterpret_cast<int*>(Nodes[index->second].Header.Properties[0].Data.data());

	++*reinterpret_cast<int*>(Nodes[ObjectCount].Header.Properties[0].Data.data());

	return object;
}

FbxObjectNode* FbxObjectNode::FindRef(const char* name, const char* type)
{
	FbxNode* node = Parent;

	for (size_t i = 0; i < node->ObjectNode->References.size(); ++i)
	{
		FbxObjectNode* childObject = node->ObjectNode->References[i];
		FbxNode* childNode = childObject->Parent;

		std::string childObjectType;

		if (childNode->Header.Properties.size() >= 2 && childNode->Header.Properties[2].TypeCode == 'S')
			childObjectType = vectorToString(childNode->Header.Properties[2].Data);

		if (childNode->Header.Name == name && (type == nullptr || childObjectType == type))
			return childObject;
	}

	return nullptr;
}

FbxObjectNode* FbxObjectNode::FindRefBy(const char* name, const char* type)
{
	FbxNode* node = Parent;

	for (size_t i = 0; i < node->ObjectNode->ReferencedBy.size(); ++i)
	{
		FbxObjectNode* childObject = node->ObjectNode->ReferencedBy[i];
		FbxNode* childNode = childObject->Parent;

		std::string childObjectType;

		if (childNode->Header.Properties.size() >= 2 && childNode->Header.Properties[2].TypeCode == 'S')
			childObjectType = vectorToString(childNode->Header.Properties[2].Data);

		if (childNode->Header.Name == name && (type == nullptr || childObjectType == type))
			return childObject;
	}

	return nullptr;
}

std::string vectorToString(const std::vector<char>& vector)
{
	return std::string(vector.data(), vector.size());
}

using namespace std::string_literals;

void FbxFileStructure::MakeFileStructure()
{
	HeaderExtension = AddNode("FBXHeaderExtension");

	AddProperty(AddNode("FBXHeaderVersion", HeaderExtension), 100);
	AddProperty(AddNode("FBXVersion", HeaderExtension), 7700);
	AddProperty(AddNode("EncryptionType", HeaderExtension), 0);
	size_t creationTimeStamp = AddNode("CreationTimeStamp", HeaderExtension);
	{
		AddProperty(AddNode("Version", creationTimeStamp), 100);
		TimeStamp.Year = AddProperty(AddNode("Year", creationTimeStamp), 2020);
		TimeStamp.Month = AddProperty(AddNode("Month", creationTimeStamp), 8);
		TimeStamp.Day = AddProperty(AddNode("Day", creationTimeStamp), 5);
		TimeStamp.Hour = AddProperty(AddNode("Hour", creationTimeStamp), 13);
		TimeStamp.Minute = AddProperty(AddNode("Minute", creationTimeStamp), 46);
		TimeStamp.Second = AddProperty(AddNode("Second", creationTimeStamp), 22);
		TimeStamp.Millisecond = AddProperty(AddNode("Millisecond", creationTimeStamp), 173);
	}
	AddProperty(AddNode("Creator", HeaderExtension), "HornetEngine 2022/5/18");
	size_t otherFlags = AddNode("OtherFlags", HeaderExtension);
	AddProperty(AddNode("TCDefinition", otherFlags), 127);
	size_t sceneInfo = AddProperties(AddNode("SceneInfo", HeaderExtension), std::string("GlobalInfo\00\01SceneInfo"s), "UserData");
	{
		AddProperty(AddNode("Type", sceneInfo), "UserData");
		AddProperty(AddNode("Version", sceneInfo), 100);
		size_t metaData = AddNode("MetaData", sceneInfo);
		{
			AddProperty(AddNode("Version", metaData), 100);
			AddProperty(AddNode("Title", metaData), "");
			AddProperty(AddNode("Subject", metaData), "");
			AddProperty(AddNode("Author", metaData), "");
			AddProperty(AddNode("Keywords", metaData), "");
			AddProperty(AddNode("Revision", metaData), "");
			AddProperty(AddNode("Comment", metaData), "");
		}
		size_t properties = AddNode("Properties70", sceneInfo);
		{
			AddProperties(AddNode("P", properties), "DocumentUrl", "KString", "Url", "", "");
			AddProperties(AddNode("P", properties), "SrcDocumentUrl", "KString", "Url", "", "");
			AddProperties(AddNode("P", properties), "Original", "Compound", "", "");
			AddProperties(AddNode("P", properties), "Original|ApplicationVendor", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "Original|ApplicationName", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "Original|ApplicationVersion", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "Original|DateTime_GMT", "DateTime", "", "", "");
			AddProperties(AddNode("P", properties), "Original|FileName", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "LastSaved", "Compound", "", "");
			AddProperties(AddNode("P", properties), "LastSaved|ApplicationVendor", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "LastSaved|ApplicationName", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "LastSaved|ApplicationVersion", "KString", "", "", "");
			AddProperties(AddNode("P", properties), "LastSaved|DateTime_GMT", "DateTime", "", "", "");
		}
	}
	AddProperty(AddNode("FileId"), ArrayWrapper<unsigned char>{ nullptr, 0 });
	AddProperty(AddNode("CreationTime"), "2020-08-05 13:46:22:173");
	AddProperty(AddNode("Creator"), "HornetEngine 2022/5/18");
	GlobalSettings = AddNode("GlobalSettings");
	{
		AddProperty(AddNode("Version", GlobalSettings), 100);
		size_t properties = AddNode("Properties70", GlobalSettings);
		{
			AddProperties(AddNode("P", properties), "UpAxis", "int", "Integer", "", 1);
			AddProperties(AddNode("P", properties), "UpAxisSign", "int", "Integer", "", 1);
			AddProperties(AddNode("P", properties), "FrontAxis", "int", "Integer", "", 2);
			AddProperties(AddNode("P", properties), "FrontAxisSign", "int", "Integer", "", 1);
			AddProperties(AddNode("P", properties), "CoordAxis", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "CoordAxisSign", "int", "Integer", "", 1);
			AddProperties(AddNode("P", properties), "OriginalUpAxis", "int", "Integer", "", -1);
			AddProperties(AddNode("P", properties), "OriginalUpAxisSign", "int", "Integer", "", 1);
			AddProperties(AddNode("P", properties), "UnitScaleFactor", "double", "Number", "", 1.0);
			AddProperties(AddNode("P", properties), "OriginalUnitScaleFactor", "double", "Number", "", 1.0);
			AddProperties(AddNode("P", properties), "AmbientColor", "ColorRGB", "Color", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "DefaultCamera", "KString", "", "", "Producer Perspective");
			AddProperties(AddNode("P", properties), "TimeMode", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "TimeProtocol", "enum", "", "", 2);
			AddProperties(AddNode("P", properties), "SnapOnFrameMode", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "TimeSpanStart", "KTime", "Time", "", 0LL);
			AddProperties(AddNode("P", properties), "TimeSpanStop", "KTime", "Time", "", 46186158000LL);
			AddProperties(AddNode("P", properties), "CustomFrameRate", "double", "Number", "", -1.0);
			AddProperties(AddNode("P", properties), "TimeMarker", "Compound", "", "");
			AddProperties(AddNode("P", properties), "CurrentTimeMarker", "int", "Integer", "", -1);
		}
	}

	auto objectId = [this]() -> long long
	{
		return (long long)Nodes.size();
	};

	Documents = AddNode("Documents");
	{
		AddProperty(AddNode("Count", Documents), 1);
		size_t document = AddProperties(AddNode("Document", Documents), objectId(), "", "Scene");
		{
			size_t properties = AddNode("Properties70", document);
			{
				AddProperties(AddNode("P", properties), "SourceObject", "object", "", "");
				AddProperties(AddNode("P", properties), "ActiveAnimStackName", "KString", "", "", "");
			}
			AddProperty(AddNode("RootNode", document), 0LL);
		}
	}
	References = AddNode("References");
	Definitions = AddNode("Definitions");
	{
		AddProperty(AddNode("Version", Definitions), 100);
		ObjectCount = AddProperty(AddNode("Count", Definitions), 1); // SET COUNT
		{
			size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "GlobalSettings");
			AddProperty(AddNode("Count", objectType), 1);
		}
		{
			size_t objectType = AddDefinition("Model");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Model");
			//AddProperty(AddNode("Count", objectType), 1 + 3 * (int)FbxObjects.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxNode");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "QuaternionInterpolate", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationOffset", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "RotationPivot", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "ScalingOffset", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "ScalingPivot", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "TranslationActive", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMin", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "TranslationMax", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "TranslationMinX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMinY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMinZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMaxX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMaxY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "TranslationMaxZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationOrder", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationSpaceForLimitOnly", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationStiffnessX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "RotationStiffnessY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "RotationStiffnessZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "AxisLen", "double", "Number", "", 10.0);
			AddProperties(AddNode("P", properties), "PreRotation", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "PostRotation", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "RotationActive", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMin", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "RotationMax", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "RotationMinX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMinY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMinZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMaxX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMaxY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "RotationMaxZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "InheritType", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingActive", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMin", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "ScalingMax", "Vector3D", "Vector", "", 1.0, 1.0, 1.0);
			AddProperties(AddNode("P", properties), "ScalingMinX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMinY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMinZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMaxX", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMaxY", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ScalingMaxZ", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "GeometricTranslation", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "GeometricRotation", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "GeometricScaling", "Vector3D", "Vector", "", 1.0, 1.0, 1.0);
			AddProperties(AddNode("P", properties), "MinDampRangeX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MinDampRangeY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MinDampRangeZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampRangeX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampRangeY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampRangeZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MinDampStrengthX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MinDampStrengthY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MinDampStrengthZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampStrengthX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampStrengthY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "MaxDampStrengthZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "PreferedAngleX", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "PreferedAngleY", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "PreferedAngleZ", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "LookAtProperty", "object", "", "");
			AddProperties(AddNode("P", properties), "UpVectorProperty", "object", "", "");
			AddProperties(AddNode("P", properties), "Show", "bool", "", "", 1);
			AddProperties(AddNode("P", properties), "NegativePercentShapeSupport", "bool", "", "", 1);
			AddProperties(AddNode("P", properties), "DefaultAttributeIndex", "int", "Integer", "", -1);
			AddProperties(AddNode("P", properties), "Freeze", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "LODBox", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "Lcl Translation", "Lcl Translation", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "Lcl Rotation", "Lcl Rotation", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "Lcl Scaling", "Lcl Scaling", "", "A", 1.0, 1.0, 1.0);
			AddProperties(AddNode("P", properties), "Visibility", "Visibility", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "Visibility Inheritance", "Visibility Inheritance", "", "", 1);
		}
		{
			size_t objectType = AddDefinition("NodeAttribute");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "NodeAttribute");
			//AddProperty(AddNode("Count", objectType), 1 + 2 * (int)FbxObjects.size()); 
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxSkeleton");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "Color", "ColorRGB", "Color", "", 0.8, 0.8, 0.8);
			AddProperties(AddNode("P", properties), "Size", "double", "Number", "", 100.0);
			AddProperties(AddNode("P", properties), "LimbLength", "double", "Number", "H", 1.0);
		}
		{
			size_t objectType = AddDefinition("Geometry");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Geometry");
			//AddProperty(AddNode("Count", objectType), (int)FbxObjects.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxMesh");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "Color", "ColorRGB", "Color", "", 0.8, 0.8, 0.8);
			AddProperties(AddNode("P", properties), "BBoxMin", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "BBoxMax", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "Primary Visibility", "bool", "", "", 1);
			AddProperties(AddNode("P", properties), "Casts Shadows", "bool", "", "", 1);
			AddProperties(AddNode("P", properties), "Receive Shadows", "bool", "", "", 1);
		}
		{
			size_t objectType = AddDefinition("Material");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Material");
			//AddProperty(AddNode("Count", objectType), (int)FbxMaterials.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxSurfacePhong");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "ShadingModel", "KString", "", "", "Phong");
			AddProperties(AddNode("P", properties), "MultiLayer", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "EmissiveColor", "Color", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "EmissiveFactor", "Number", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "AmbientColor", "Color", "", "A", 0.2, 0.2, 0.2);
			AddProperties(AddNode("P", properties), "AmbientFactor", "Number", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "DiffuseColor", "Color", "", "A", 0.8, 0.8, 0.8);
			AddProperties(AddNode("P", properties), "DiffuseFactor", "Number", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "Bump", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "NormalMap", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "BumpFactor", "double", "Number", "", 1.0);
			AddProperties(AddNode("P", properties), "TransparentColor", "Color", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "TransparencyFactor", "Number", "", "A", 0.0);
			AddProperties(AddNode("P", properties), "DisplacementColor", "ColorRGB", "Color", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "DisplacementFactor", "double", "Number", "", 1.0);
			AddProperties(AddNode("P", properties), "VectorDisplacementColor", "ColorRGB", "Color", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "VectorDisplacementFactor", "double", "Number", "", 1.0);
			AddProperties(AddNode("P", properties), "SpecularColor", "Color", "", "A", 0.2, 0.2, 0.2);
			AddProperties(AddNode("P", properties), "SpecularFactor", "Number", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "ShininessExponent", "Number", "", "A", 20.0);
			AddProperties(AddNode("P", properties), "ReflectionColor", "Color", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "ReflectionFactor", "Number", "", "A", 1.0);
		}
		{
			size_t objectType = AddDefinition("Texture");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Texture");
			//AddProperty(AddNode("Count", objectType), 3 * (int)FbxMaterials.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxFileTexture");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "TextureTypeUse", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "Texture alpha", "Number", "", "A", 1.0);
			AddProperties(AddNode("P", properties), "CurrentMappingType", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "WrapModeU", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "WrapModeV", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "UVSwap", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "PremultiplyAlpha", "bool", "", "", 1);
			AddProperties(AddNode("P", properties), "Translation", "Vector", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "Rotation", "Vector", "", "A", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "Scaling", "Vector", "", "A", 1.0, 1.0, 1.0);
			AddProperties(AddNode("P", properties), "TextureRotationPivot", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "TextureScalingPivot", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
			AddProperties(AddNode("P", properties), "CurrentTextureBlendMode", "enum", "", "", 1);
			AddProperties(AddNode("P", properties), "UVSet", "KString", "", "", "default");
			AddProperties(AddNode("P", properties), "UseMaterial", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "UseMipMap", "bool", "", "", 0);
		}
		{
			size_t objectType = AddDefinition("Deformer");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Deformer");
			//AddProperty(AddNode("Count", objectType), 4 * (int)FbxObjects.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxCluster");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "SrcModelReference", "object", "", "");
		}
		{
			size_t objectType = AddDefinition("Pose");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Pose");
			//AddProperty(AddNode("Count", objectType), (int)FbxObjects.size());
		}
		{
			size_t objectType = AddDefinition("Video");
			//size_t objectType = AddProperty(AddNode("ObjectType", Definitions), "Video");
			//AddProperty(AddNode("Count", objectType), 3 * (int)FbxMaterials.size());
			size_t propertyTemplate = AddProperty(AddNode("PropertyTemplate", objectType), "FbxVideo");
			size_t properties = AddNode("Properties70", propertyTemplate);
			AddProperties(AddNode("P", properties), "Path", "KString", "XRefUrl", "", "");
			AddProperties(AddNode("P", properties), "RelPath", "KString", "XRefUrl", "", "");
			AddProperties(AddNode("P", properties), "Color", "ColorRGB", "Color", "", 0.8, 0.8, 0.8);
			AddProperties(AddNode("P", properties), "ClipIn", "KTime", "Time", "", 0LL);
			AddProperties(AddNode("P", properties), "ClipOut", "KTime", "Time", "", 0LL);
			AddProperties(AddNode("P", properties), "Offset", "KTime", "Time", "", 0LL);
			AddProperties(AddNode("P", properties), "PlaySpeed", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "FreeRunning", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "Loop", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "Mute", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "AccessMode", "enum", "", "", 0);
			AddProperties(AddNode("P", properties), "ImageSequence", "bool", "", "", 0);
			AddProperties(AddNode("P", properties), "ImageSequenceOffset", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "FrameRate", "double", "Number", "", 0.0);
			AddProperties(AddNode("P", properties), "LastFrame", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "Width", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "Height", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "StartFrame", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "StopFrame", "int", "Integer", "", 0);
			AddProperties(AddNode("P", properties), "InterlaceMode", "enum", "", "", 0);
		}
	}

	Objects = AddNode("Objects");
	{
		std::shared_ptr<Engine::Graphics::MeshFormat> format = GetFbxMeshFormat();
		std::shared_ptr<Engine::Graphics::MeshData> stagingData = Engine::Create<Engine::Graphics::MeshData>();

		stagingData->SetFormat(format);

		std::shared_ptr<Engine::Graphics::MeshFormat> keyFormat = GetFbxMeshKeyFormat();
		std::shared_ptr<Engine::Graphics::MeshData> keyStagingData = Engine::Create<Engine::Graphics::MeshData>();

		std::vector<double> shapeKeyStagingData;
		std::vector<int> indexStagingBuffer;

		keyStagingData->SetFormat(keyFormat);

		std::vector<double> wAxisBuffer;
		std::vector<int> materialBuffer;

		struct NodeData
		{
			size_t Skeleton = (size_t)-1;
			size_t Model = (size_t)-1;
			size_t MeshModel = (size_t)-1;
			size_t Geometry = (size_t)-1;
			size_t ShapeKey = (size_t)-1;
			size_t Pose = (size_t)-1;
			size_t Deformer = (size_t)-1;
		};

		std::vector<NodeData> nodes(Package->Nodes.size());

		for (size_t i = 0; i < Package->Nodes.size(); ++i)
		{
			Engine::Graphics::ModelPackageNode& node = Package->Nodes[i];
			NodeData& data = nodes[i];

			bool isRoot = node.AttachedTo == (size_t)-1;
			const char* attachmentTag = isRoot ? "Root" : "Limb";

			data.Skeleton = AddProperties(AddObject("NodeAttribute", Objects), objectId(), node.Name + std::string("_skel\00\01NodeAttribute"s), attachmentTag);

			if (isRoot)
				AddProperties(AddNode("TypeFlags", data.Skeleton), "Null", "Skeleton", "Root");
			else
				AddProperties(AddNode("TypeFlags", data.Skeleton), "Skeleton");

			data.Model = AddProperties(AddObject("Model", Objects), objectId(), node.Name + std::string("\00\01Model"s), attachmentTag);
			{
				AddProperty(AddNode("Version", data.Model), 100);

				size_t properties = AddNode("Properties70", data.Model);

				AddProperties(AddNode("P", properties), "ScalingMax", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
				AddProperties(AddNode("P", properties), "DefaultAttributeIndex", "int", "Integer", "", 0);

				if (!isRoot)
				{
					AddProperties(AddNode("P", properties), "Lcl Translation", "Lcl Translation", "", "A", 0.0, 0.0, 0.0);
					AddProperties(AddNode("P", properties), "Lcl Rotation", "Lcl Rotation", "", "A", 0.0, 0.0, 0.0);
					AddProperties(AddNode("P", properties), "Lcl Scaling", "Lcl Scaling", "", "A", 1.0, 1.0, 1.0);
				}

				AddProperty<unsigned char>(AddNode("Shading", data.Model), 'Y');
				AddProperty(AddNode("Culling", data.Model), "CullingOff");
			}

			AddConnection("OO", (long long)data.Model, isRoot ? 0 : nodes[node.AttachedTo].Model);
			AddConnection("OO", (long long)data.Skeleton, (long long)data.Model);

			if (node.Mesh != nullptr)
			{
				size_t vertexCount = node.Mesh->GetVertices();

				const VertexAttributeFormat* morphPose = node.Format->GetAttribute("morphPosition1");

				data.Geometry = AddProperties(AddObject("Geometry", Objects), objectId(), node.Name + "\00\01Geometry"s, "Mesh");
				{
					if (morphPose != nullptr)
					{
						size_t properties = AddNode("Properties70", data.Geometry);

						AddProperties(AddNode("P", properties), "Key 1", "Number", "", "A", 0.0);
					}

					stagingData->ResetData();
					stagingData->PushVertices(vertexCount, false);

					void** vertexBuffers = stagingData->GetData();

					node.Format->Copy(node.Mesh->GetData(), vertexBuffers, format, vertexCount);

					const VertexAttributeFormat* position = node.Format->GetAttribute("position");
					const VertexAttributeFormat* stagingPosition = format->GetAttribute("position");

					AddProperty(AddNode("Vertices", data.Geometry), ArrayWrapper<double>{reinterpret_cast<double*>(vertexBuffers[stagingPosition->Binding]), vertexCount * 3, true });

					wAxisBuffer.resize(vertexCount);

					for (size_t i = 0; i < vertexCount; ++i)
						wAxisBuffer[i] = 1;

					std::vector<int> indexBuffer = node.Mesh->GetIndexBuffer();
					size_t faceCount = indexBuffer.size() / 3;

					for (size_t i = 0; i < faceCount; ++i)
						indexBuffer[i * 3 + 2] ^= -1;

					materialBuffer.resize(faceCount);

					for (size_t i = 0; i < faceCount; ++i)
						materialBuffer[i] = 0;

					AddProperty(AddNode("PolygonVertexIndex", data.Geometry), ArrayWrapper<int>{indexBuffer.data(), indexBuffer.size(), true });
					AddProperty(AddNode("GeometryVersion", data.Geometry), 100);

					auto addAttribute = [this, &wAxisBuffer, &node, &format, data, vertexBuffers, i, vertexCount](const char* attributeName, const char* nodeName, size_t elementCount, const char* bufferNodeName, const char* elementName, const char* bufferWNodeName = nullptr)
					{
						const VertexAttributeFormat* attribute = node.Format->GetAttribute(attributeName);
						const VertexAttributeFormat* stagingAttribute = format->GetAttribute(attributeName);

						if (attribute != nullptr)
						{
							if (attributeName == std::string("textureCoords"))
							{
								for (size_t i = 0; i < vertexCount; ++i)
								{
									double& y = reinterpret_cast<Vector2SD*>(vertexBuffers[stagingAttribute->Binding])->Y;

									y = 1 - y;
								}
							}

							size_t element = AddProperty(AddNode(nodeName, data.Geometry), 0);

							AddProperty(AddNode("Version", element), 100);
							AddProperty(AddNode("Name", element), elementName);
							AddProperty(AddNode("MappingInformationType", element), "ByVertice");
							AddProperty(AddNode("ReferenceInformationType", element), "Direct");
							AddProperty(AddNode(bufferNodeName, element), ArrayWrapper<double>{reinterpret_cast<double*>(vertexBuffers[stagingAttribute->Binding]), vertexCount* elementCount, true });

							if (bufferWNodeName != nullptr)
								AddProperty(AddNode(bufferWNodeName, element), ArrayWrapper<double>{wAxisBuffer.data(), vertexCount, true });
						}
					};

					addAttribute("normal", "LayerElementNormal", 3, "Normals", "", "NormalsW");
					addAttribute("binormal", "LayerElementBinormal", 3, "Binormals", "", "BinormalsW");
					addAttribute("tangent", "LayerElementTangent", 3, "Tangents", "", "TangentsW");
					addAttribute("textureCoords", "LayerElementUV", 2, "UV", "DiffuseUV");

					size_t element = AddProperty(AddNode("LayerElementMaterial", data.Geometry), 0);

					AddProperty(AddNode("Version", element), 100);
					AddProperty(AddNode("Name", element), "");
					AddProperty(AddNode("MappingInformationType", element), "ByPolygon");
					AddProperty(AddNode("ReferenceInformationType", element), "IndexToDirect");
					AddProperty(AddNode("Materials", element), ArrayWrapper<int>{materialBuffer.data(), faceCount, true });

					size_t layer = AddProperty(AddNode("Layer", data.Geometry), 0);

					AddProperty(AddNode("Version", layer), 100);

					auto addLayerElement = [this, &node, layer, i](const char* attributeName, const char* nodeName)
					{
						const VertexAttributeFormat* attribute = node.Format->GetAttribute(attributeName);

						if (attribute != nullptr)
						{
							size_t layerElement = AddNode("LayerElement", layer);

							AddProperty(AddNode("Type", layerElement), nodeName);
							AddProperty(AddNode("TypedIndex", layerElement), 0);
						}
					};

					addLayerElement("normal", "LayerElementNormal");
					addLayerElement("binormal", "LayerElementBinormal");
					addLayerElement("tangent", "LayerElementTangent");

					size_t layerElement = AddNode("LayerElement", layer);

					AddProperty(AddNode("Type", layerElement), "LayerElementMaterial");
					AddProperty(AddNode("TypedIndex", layerElement), 0);

					addLayerElement("textureCoords", "LayerElementUV");
				}

				data.MeshModel = AddProperties(AddObject("Model", Objects), objectId(), node.Name + "\00\01Model"s, "Mesh");
				{
					AddProperty(AddNode("Version", data.MeshModel), 100);

					size_t properties = AddNode("Properties70", data.MeshModel);

					AddProperties(AddNode("P", properties), "ScalingMax", "Vector3D", "Vector", "", 0.0, 0.0, 0.0);
					AddProperties(AddNode("P", properties), "DefaultAttributeIndex", "int", "Integer", "", 0);

					AddProperty<unsigned char>(AddNode("Shading", data.MeshModel), 'Y');
					AddProperty(AddNode("Culling", data.MeshModel), "CullingOff");
				}

				AddConnection("OO", (long long)data.MeshModel, 0);
				AddConnection("OO", (long long)data.Geometry, (long long)data.MeshModel);

				data.Pose = AddProperties(AddObject("Pose", Objects), objectId(), node.Name + "\00\01Pose"s, "BindPose");
				{
					AddProperty(AddNode("Type", data.Pose), "BindPose");
					AddProperty(AddNode("Version", data.Pose), 100);

					Matrix4D identity;

					std::vector<size_t> parentTree;

					for (size_t current = i; current != (size_t)-1; current = Package->Nodes[current].AttachedTo)
						parentTree.push_back(current);

					size_t poses = AddProperty(AddNode("NbPoseNodes", data.Pose), (int)(parentTree.size() + 1));

					for (size_t j = 0; j < parentTree.size(); ++j)
					{
						size_t current = parentTree[parentTree.size() - 1 - j];

						Matrix4D transformation = Package->Nodes[current].Transform->GetTransformation();

						size_t poseNode = AddNode("PoseNode", data.Pose);
						AddProperty(AddNode("Node", poseNode), (long long)nodes[current].Model);
						AddProperty(AddNode("Matrix", poseNode), ArrayWrapper<double>(&transformation.Data[0][0], (size_t)16));
					}

					{
						size_t poseNode = AddNode("PoseNode", data.Pose);
						AddProperty(AddNode("Node", poseNode), (long long)data.MeshModel);
						AddProperty(AddNode("Matrix", poseNode), ArrayWrapper<double>(&identity.Data[0][0], 16));
					}
				}

				data.Deformer = AddProperties(AddObject("Deformer", Objects), objectId(), std::string("\00\01Deformer"s), "Skin");
				{
					AddProperty(AddNode("Version", data.Deformer), 100);
					AddProperty(AddNode("Link_DeformAcuracy", data.Deformer), 50.0);
				}

				AddConnection("OO", (long long)data.Deformer, (long long)data.Geometry);

				size_t meshSubdeformer = AddProperties(AddObject("Deformer", Objects), objectId(), std::string("\00\01SubDeformer"s), "Cluster");
				{
					AddProperty(AddNode("Version", meshSubdeformer), 100);
					AddProperties(AddNode("UserData", meshSubdeformer), "", "");

					std::vector<int> indices(vertexCount);
					std::vector<double> weights(vertexCount);

					for (int i = 0; i < (int)vertexCount; ++i)
					{
						indices[i] = i;
						weights[i] = 1;
					}

					Matrix4D transformation = node.Transform->GetTransformation();

					AddProperty(AddNode("Indexes", meshSubdeformer), ArrayWrapper<int>{indices.data(), vertexCount, true });
					AddProperty(AddNode("Weights", meshSubdeformer), ArrayWrapper<double>{weights.data(), vertexCount, true });
					AddProperty(AddNode("Transform", meshSubdeformer), ArrayWrapper<double>{ &transformation.Data[0][0], 16 });
					AddProperty(AddNode("TransformLink", meshSubdeformer), ArrayWrapper<double>{ &transformation.Data[0][0], 16 });
				}

				AddConnection("OO", (long long)meshSubdeformer, (long long)data.Deformer);
				AddConnection("OO", (long long)data.Model, (long long)meshSubdeformer);

				for (size_t parent = node.AttachedTo; parent != (size_t)-1; parent = Package->Nodes[parent].AttachedTo)
				{
					Engine::Graphics::ModelPackageNode& parentNode = Package->Nodes[parent];

					size_t subdeformer = AddProperties(AddObject("Deformer", Objects), objectId(), std::string("\00\01SubDeformer"s), "Cluster");
					{
						Matrix4D transformation = parentNode.Transform->GetTransformation();

						AddProperty(AddNode("Version", subdeformer), 100);
						AddProperties(AddNode("UserData", subdeformer), "", "");
						AddProperty(AddNode("Transform", subdeformer), ArrayWrapper<double>{ &transformation.Data[0][0], 16 });
						AddProperty(AddNode("TransformLink", subdeformer), ArrayWrapper<double>{ &transformation.Data[0][0], 16 });
					}

					AddConnection("OO", (long long)subdeformer, (long long)data.Deformer);
					AddConnection("OO", (long long)nodes[parent].Model, (long long)subdeformer);
				}

				if (morphPose != nullptr)
				{
					data.ShapeKey = AddProperties(AddObject("Geometry", Objects), objectId(), "Key 1\00\01Geometry"s, "Shape");
					{
						AddProperty(AddNode("Version", data.ShapeKey), 100);

						keyStagingData->ResetData();
						keyStagingData->PushVertices(vertexCount, false);

						void** vertexBuffers = keyStagingData->GetData();

						node.Format->Copy(node.Mesh->GetData(), vertexBuffers, keyFormat, vertexCount);

						indexStagingBuffer.resize(vertexCount);

						for (size_t i = 0; i < vertexCount; ++i)
							indexStagingBuffer[i] = (int)i;

						AddProperty(AddNode("Indexes", data.ShapeKey), ArrayWrapper<int>{indexStagingBuffer.data(), vertexCount, true });

						const VertexAttributeFormat* position = node.Format->GetAttribute("position");
						const VertexAttributeFormat* normal = node.Format->GetAttribute("normal");
						const VertexAttributeFormat* positionOffset = node.Format->GetAttribute("morphPosition1");

						const VertexAttributeFormat* stagingPosition = keyFormat->GetAttribute("position");
						const VertexAttributeFormat* stagingNormal = keyFormat->GetAttribute("normal");
						const VertexAttributeFormat* stagingPositionOffset = keyFormat->GetAttribute("morphPosition1");

						double* vertices = reinterpret_cast<double*>(vertexBuffers[position->Binding]);
						double* offsets = reinterpret_cast<double*>(vertexBuffers[positionOffset->Binding]);

						//shapeKeyStagingData.resize(vertexCount * 3);
						//
						//for (size_t i = 0; i < vertexCount * 3; ++i)
						//	shapeKeyStagingData[i] = vertices[i] + offsets[i];

						AddProperty(AddNode("Vertices", data.ShapeKey), ArrayWrapper<double>{reinterpret_cast<double*>(vertexBuffers[stagingPositionOffset->Binding]), vertexCount * 3, true });
						AddProperty(AddNode("Normals", data.ShapeKey), ArrayWrapper<double>{reinterpret_cast<double*>(vertexBuffers[stagingNormal->Binding]), vertexCount * 3, true });
					}

					size_t shapeKeyDeformer = AddProperties(AddObject("Deformer", Objects), objectId(), node.Name + std::string("\00\01Deformer"s), "BlendShape");
					{
						AddProperty(AddNode("Version", shapeKeyDeformer), 100);
					}

					size_t shapeKeySubdeformer = AddProperties(AddObject("Deformer", Objects), objectId(), std::string("Key 1\00\01SubDeformer"s), "BlendShapeChannel");
					{
						AddProperty(AddNode("Version", shapeKeySubdeformer), 100);
						AddProperty(AddNode("DeformPercent", shapeKeySubdeformer), 0.0);

						shapeKeyStagingData.resize(vertexCount);

						for (size_t i = 0; i < vertexCount; ++i)
							shapeKeyStagingData[i] = 100;

						AddProperty(AddNode("FullWeights", shapeKeySubdeformer), ArrayWrapper<double>{shapeKeyStagingData.data(), vertexCount, true });
					}

					AddConnection("OO", (long long)shapeKeySubdeformer, (long long)shapeKeyDeformer);
					AddConnection("OO", (long long)data.ShapeKey, (long long)shapeKeySubdeformer);
					AddConnection("OO", (long long)shapeKeyDeformer, (long long)data.Geometry);
				}
			}
		}

		for (size_t i = 0; i < Package->Materials.size(); ++i)
		{
			Engine::Graphics::ModelPackageMaterial packageMaterial = Package->Materials[i];

			size_t material = AddProperties(AddObject("Material", Objects), objectId(), Package->Materials[i].Name + "\00\01Material"s, "");
			{
				AddProperty(AddNode("Version", material), 100);
				AddProperty(AddNode("ShadingModel", material), "phong");
				AddProperty(AddNode("MultiLayer", material), 0);

				size_t properties = AddNode("Properties70", material);

				AddProperties(AddNode("P", properties), "Emissive", "Vector3D", "Vector", "", (double)packageMaterial.EmissiveColor.R, (double)packageMaterial.EmissiveColor.G, (double)packageMaterial.EmissiveColor.B);
				AddProperties(AddNode("P", properties), "Ambient", "Vector3D", "Vector", "", (double)packageMaterial.AmbientColor.R, (double)packageMaterial.AmbientColor.G, (double)packageMaterial.AmbientColor.B);
				AddProperties(AddNode("P", properties), "Diffuse", "Vector3D", "Vector", "", (double)packageMaterial.DiffuseColor.R, (double)packageMaterial.DiffuseColor.G, (double)packageMaterial.DiffuseColor.B);
				AddProperties(AddNode("P", properties), "Specular", "Vector3D", "Vector", "", (double)packageMaterial.SpecularColor.R, (double)packageMaterial.SpecularColor.G, (double)packageMaterial.SpecularColor.B);
				AddProperties(AddNode("P", properties), "Shininess", "double", "Number", "", (double)packageMaterial.Shininess);
				AddProperties(AddNode("P", properties), "Opacity", "double", "Number", "", (double)packageMaterial.Alpha);
				AddProperties(AddNode("P", properties), "Reflectivity", "double", "Number", "", 0.0);
			}

			for (size_t j = 0; j < Package->Nodes.size(); ++j)
				if (Package->Nodes[j].MaterialIndex == i)
					AddConnection("OO", (long long)material, nodes[j].MeshModel);

			size_t diffuseVideo = AddProperties(AddObject("Video", Objects), objectId(), std::string("Diffuse Texture\00\01Video"s), "Clip");
			{
				AddProperty(AddNode("Type", diffuseVideo), "Clip");

				size_t properties = AddNode("Properties70", diffuseVideo);

				AddProperties(AddNode("P", properties), "Path", "KString", "XRefUrl", "", Package->Materials[i].Diffuse);
				AddProperties(AddNode("P", properties), "RelPath", "KString", "XRefUrl", "", ".\\" + Package->Materials[i].Diffuse);

				AddProperty(AddNode("UseMipMap", diffuseVideo), 0);
				AddProperty(AddNode("Filename", diffuseVideo), Package->Materials[i].Diffuse);
				AddProperty(AddNode("RelativeFilename", diffuseVideo), ".\\" + Package->Materials[i].Diffuse);
			}

			size_t normalVideo = AddProperties(AddObject("Video", Objects), objectId(), std::string("Normal Texture\00\01Video"s), "Clip");
			{
				AddProperty(AddNode("Type", normalVideo), "Clip");

				size_t properties = AddNode("Properties70", normalVideo);

				AddProperties(AddNode("P", properties), "Path", "KString", "XRefUrl", "", Package->Materials[i].Normal);
				AddProperties(AddNode("P", properties), "RelPath", "KString", "XRefUrl", "", ".\\" + Package->Materials[i].Normal);

				AddProperty(AddNode("UseMipMap", normalVideo), 0);
				AddProperty(AddNode("Filename", normalVideo), Package->Materials[i].Normal);
				AddProperty(AddNode("RelativeFilename", normalVideo), ".\\" + Package->Materials[i].Normal);
			}

			size_t specularVideo = AddProperties(AddObject("Video", Objects), objectId(), std::string("Specular Texture\00\01Video"s), "Clip");
			{
				AddProperty(AddNode("Type", specularVideo), "Clip");

				size_t properties = AddNode("Properties70", specularVideo);

				AddProperties(AddNode("P", properties), "Path", "KString", "XRefUrl", "", Package->Materials[i].Specular);
				AddProperties(AddNode("P", properties), "RelPath", "KString", "XRefUrl", "", ".\\" + Package->Materials[i].Specular);

				AddProperty(AddNode("UseMipMap", specularVideo), 0);
				AddProperty(AddNode("Filename", specularVideo), Package->Materials[i].Specular);
				AddProperty(AddNode("RelativeFilename", specularVideo), ".\\" + Package->Materials[i].Specular);
			}

			size_t diffuseTexture = AddProperties(AddObject("Texture", Objects), objectId(), std::string("Diffuse Texture\00\01Texture"s), "");
			{
				AddProperty(AddNode("Type", diffuseTexture), "TextureVideoClip");
				AddProperty(AddNode("Version", diffuseTexture), 100);
				AddProperty(AddNode("TextureName", diffuseTexture), std::string("Diffuse Texture\00\01Texture"s));

				size_t properties = AddNode("Properties70", diffuseTexture);

				AddProperties(AddNode("P", properties), "UVSet", "KString", "", "", "DiffuseUV");
				AddProperties(AddNode("P", properties), "UseMaterial", "bool", "", "", 1);

				AddProperty(AddNode("Media", diffuseTexture), std::string("Diffuse Texture\00\01Video"s));
				AddProperty(AddNode("FileName", diffuseTexture), Package->Materials[i].Diffuse);
				AddProperty(AddNode("RelativeFilename", diffuseTexture), ".\\" + Package->Materials[i].Diffuse);
				AddProperties(AddNode("ModelUVTranslation", diffuseTexture), 0.0, 0.0);
				AddProperties(AddNode("ModelUVScaling", diffuseTexture), 1.0, 1.0);
				AddProperty(AddNode("Texture_Alpha_Source", diffuseTexture), "None");
				AddProperties(AddNode("Cropping", diffuseTexture), 0, 0, 0, 0);
			}

			size_t normalTexture = AddProperties(AddObject("Texture", Objects), objectId(), std::string("Normal Texture\00\01Texture"s), "");
			{
				AddProperty(AddNode("Type", normalTexture), "TextureVideoClip");
				AddProperty(AddNode("Version", normalTexture), 100);
				AddProperty(AddNode("TextureName", normalTexture), std::string("Normal Texture\00\01Texture"s));

				size_t properties = AddNode("Properties70", normalTexture);

				AddProperties(AddNode("P", properties), "UVSet", "KString", "", "", "DiffuseUV");
				AddProperties(AddNode("P", properties), "UseMaterial", "bool", "", "", 1);

				AddProperty(AddNode("Media", normalTexture), std::string("Normal Texture\00\01Video"s));
				AddProperty(AddNode("FileName", normalTexture), Package->Materials[i].Normal);
				AddProperty(AddNode("RelativeFilename", normalTexture), ".\\" + Package->Materials[i].Normal);
				AddProperties(AddNode("ModelUVTranslation", normalTexture), 0.0, 0.0);
				AddProperties(AddNode("ModelUVScaling", normalTexture), 1.0, 1.0);
				AddProperty(AddNode("Texture_Alpha_Source", normalTexture), "None");
				AddProperties(AddNode("Cropping", normalTexture), 0, 0, 0, 0);
			}

			size_t specularTexture = AddProperties(AddObject("Texture", Objects), objectId(), std::string("Specular Texture\00\01Texture"s), "");
			{
				AddProperty(AddNode("Type", specularTexture), "TextureVideoClip");
				AddProperty(AddNode("Version", specularTexture), 100);
				AddProperty(AddNode("TextureName", specularTexture), std::string("Specular Texture\00\01Texture"s));

				size_t properties = AddNode("Properties70", specularTexture);

				AddProperties(AddNode("P", properties), "UVSet", "KString", "", "", "DiffuseUV");
				AddProperties(AddNode("P", properties), "UseMaterial", "bool", "", "", 1);

				AddProperty(AddNode("Media", specularTexture), std::string("Specular Texture\00\01Video"s));
				AddProperty(AddNode("FileName", specularTexture), Package->Materials[i].Specular);
				AddProperty(AddNode("RelativeFilename", specularTexture), ".\\" + Package->Materials[i].Specular);
				AddProperties(AddNode("ModelUVTranslation", specularTexture), 0.0, 0.0);
				AddProperties(AddNode("ModelUVScaling", specularTexture), 1.0, 1.0);
				AddProperty(AddNode("Texture_Alpha_Source", specularTexture), "None");
				AddProperties(AddNode("Cropping", specularTexture), 0, 0, 0, 0);
			}

			AddConnection("OP", (long long)diffuseTexture, (long long)material, "DiffuseColor");
			AddConnection("OP", (long long)normalTexture, (long long)material, "NormalMap");
			AddConnection("OP", (long long)specularTexture, (long long)material, "SpecularColor");

			AddConnection("OO", (long long)diffuseVideo, (long long)diffuseTexture);
			AddConnection("OO", (long long)normalVideo, (long long)normalTexture);
			AddConnection("OO", (long long)specularVideo, (long long)specularTexture);
		}
	}
	Connections = AddNode("Connections");
	{
		for (size_t i = 0; i < ObjectConnections.size(); ++i)
		{
			auto& connection = ObjectConnections[i];

			if (connection.Data != nullptr)
				AddProperties(AddNode("C", Connections), connection.Type, connection.Object1, connection.Object2, connection.Data);
			else
				AddProperties(AddNode("C", Connections), connection.Type, connection.Object1, connection.Object2);
		}
	}
	Takes = AddNode("Takes");
	{
		AddProperty(AddNode("Current", Takes), "");
	}
}

void FbxFileStructure::FinalizeNodes()
{
	size_t end = 27;

	for (size_t i = 0; i < RootNode.ChildIndices.size(); ++i)
	{
		Nodes[RootNode.ChildIndices[i]].ComputeEnd(end, Nodes);
		end = Nodes[RootNode.ChildIndices[i]].Header.EndOffset;
	}
	//for (size_t i = 0; i < Nodes.size(); ++i)
	//	Nodes[i].ComputeEnd(i == 0 ? 27 : Nodes[i - 1].Header.EndOffset);
	//
	//for (size_t i = Nodes.size() - 1; i < Nodes.size(); --i)
	//	Nodes[i].ComputeChildrenEnd(Nodes);
}

size_t FbxFileStructure::AddNode(const std::string& name, FbxNode* parent)
{
	if (parent == nullptr)
		return AddNode(name, -1, true);

	return AddNode(name, parent->Index);
}

size_t FbxFileStructure::AddNode(const std::string& name, size_t parent, bool allowNull)
{
	if (parent == (long long)(size_t)-1 && !allowNull)
		throw "bad parent";

	size_t index = Nodes.size();

	Nodes.push_back(FbxNode());

	FbxNode& node = Nodes.back();
	node.Index = index;
	node.Parent = parent;
	node.Depth = parent == -1 ? 1 : Nodes[parent].Depth + 1;
	node.Header.Name = name;

	if (parent != -1)
		Nodes[parent].ChildIndices.push_back(index);
	else
		RootNode.ChildIndices.push_back(index);

	return index;
}

void FbxFileStructure::AddConnection(const char* type, long long object1, long long object2, const char* data)
{
	if (object1 == (long long)(size_t)-1 || object2 == (long long)(size_t)-1)
		throw "bad connection";

	ObjectConnections.push_back(FbxConnection{ type, object1, object2, data });
}

FbxNode* FbxFileStructure::Node(size_t index)
{
	return &Nodes[index];
}