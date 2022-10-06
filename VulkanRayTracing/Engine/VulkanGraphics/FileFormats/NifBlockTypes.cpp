#include "NifBlockTypes.h"

BlockData* NiDataBlock::Get()
{
	return &Document->Blocks[BlockIndex];
}

BlockData& NifDocument::InitializeBlock(unsigned int blockIndex)
{
	BlockData& block = Blocks[blockIndex];

	block.BlockIndex = blockIndex;
	block.BlockSize = BlockSizes[blockIndex];
	block.BlockType = BlockTypes[BlockTypeIndices[blockIndex]];
	block.Document = this;

	return block;
}

const BlockData* NifDocument::FetchRef(unsigned int ref)
{
	if (ref == 0xFFFFFFFFu) return nullptr;

	return &Blocks[ref];
}

const BlockData* NifDocument::FetchRef(std::istream& stream)
{
	return FetchRef(Endian.read<unsigned int>(stream));
}

const std::string& NifDocument::FetchString(unsigned int ref)
{
	static const std::string emptyString;

	if (ref == 0xFFFFFFFFu) return emptyString;

	return Strings[ref];
}

const std::string& NifDocument::FetchString(std::istream& stream)
{
	return FetchString(Endian.read<unsigned int>(stream));
}

void NifDocument::ReadBlockRefs(std::istream& stream, BlockData& block, std::vector<const BlockData*>& refs)
{
	unsigned int count = Endian.read<unsigned int>(stream);

	refs.resize(count);

	for (unsigned int i = 0; i < count; ++i)
		refs[i] = &Blocks[Endian.read<unsigned int>(stream)];
}

std::shared_ptr<Engine::Graphics::MeshFormat> GetNiMeshFormat()
{
	static std::shared_ptr<Engine::Graphics::MeshFormat> format = nullptr;

	if (format != nullptr)
		return format;

	using Engine::Graphics::VertexAttributeFormat;

	std::vector<Engine::Graphics::VertexAttributeFormat> attributes;

	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 3, "position", 0 });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 3, "normal", 1 });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 2, "textureCoords", 1 });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 3, "binormal", 1 });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 3, "tangent", 1 });
	attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float32, 3, "morphPosition1", 2 });

	format = Engine::Graphics::MeshFormat::GetFormat(attributes);

	return format;
}