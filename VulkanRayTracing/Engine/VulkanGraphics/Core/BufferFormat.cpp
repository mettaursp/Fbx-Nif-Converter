#include "BufferFormat.h"

namespace Engine
{
	namespace Graphics
	{
		const size_t TypeSizes[Enum::AttributeDataType::Count] = {
			4, //Float32
			8, //Float64
			1, //Bool
			1, //Int8
			2, //Int16
			4, //Int32
			8, //Int64
			1, //UInt8
			2, //UInt16
			4, //UInt32
			8 //UInt64
		};

		std::string DataTypeNames[Enum::AttributeDataType::Unknown + 1] = {
				"Float32",
				"Float64",
				"Bool",
				"Int8",
				"Int16",
				"Int32",
				"Int64",
				"UInt8",
				"UInt16",
				"UInt32",
				"UInt64",

				"Count",
				"Unknown"
		};

		size_t GetDataSize(AttributeDataTypeEnum::AttributeDataType type, size_t elementCount, size_t columnCount)
		{
			return TypeSizes[type] * elementCount * columnCount;
		}

		const std::string& GetDataName(AttributeDataTypeEnum::AttributeDataType type)
		{
			return DataTypeNames[type];
		}

		size_t BufferItem::GetSize() const
		{
			return GetDataSize(Type, ElementCount);
		}

		void BufferItem::GetHashString(std::string& hash) const
		{
			const size_t MaxBufferSize = 0xFF;

			char buffer[MaxBufferSize];

			size_t addedSize = std::min(sizeof(Type) + sizeof(ElementCount) + Name.size(), (size_t)0xFF);

			buffer[0] = (Type >> 0) & 0xFF;
			buffer[1] = (Type >> 8) & 0xFF;
			buffer[2] = (Type >> 16) & 0xFF;
			buffer[3] = (Type >> 24) & 0xFF;

			buffer[4] = (ElementCount >> 0) & 0xFF;
			buffer[5] = (ElementCount >> 8) & 0xFF;
			buffer[6] = (ElementCount >> 16) & 0xFF;
			buffer[7] = (ElementCount >> 24) & 0xFF;
			buffer[8] = (ElementCount >> 32) & 0xFF;
			buffer[9] = (ElementCount >> 40) & 0xFF;
			buffer[10] = (ElementCount >> 48) & 0xFF;
			buffer[11] = (ElementCount >> 56) & 0xFF;

			size_t nameStart = sizeof(Type) + sizeof(ElementCount);

			for (size_t i = 0; i < Name.size() && i < MaxBufferSize; ++i)
				buffer[i + nameStart] = Name[i];

			hash.append(buffer, addedSize);
		}

		void BufferItem::GetHashString(std::string& hash, const std::vector<BufferItem>& attributes, size_t bufferArrayLength)
		{
			for (size_t i = 0; i < attributes.size(); ++i)
				attributes[i].GetHashString(hash);
		}

		void BufferFormat::SetBufferArrayLength(size_t arrayLength)
		{
			BufferArrayLength = arrayLength;
		}

		size_t BufferFormat::GetItemCount() const
		{
			return BufferItems.size();
		}

		size_t BufferFormat::GetBufferArraySize(size_t arrayLength) const
		{
			if (arrayLength == 0)
				return BufferArrayLength * GetBufferSize();

			return arrayLength * GetBufferSize();
		}

		std::string BufferFormat::GetHashString(size_t arrayLength) const
		{
			std::string hash;

			if (arrayLength == 0)
				arrayLength = BufferArrayLength;

			BufferItem::GetHashString(hash, BufferItems, arrayLength);

			return hash;
		}

		void BufferFormat::AddBufferItem(const BufferItem& item)
		{
			IndexMap[item.Name] = BufferItems.size();
			BufferItems.push_back(item);

			BufferSize += item.GetSize();
		}

		std::shared_ptr<BufferFormat> BufferFormat::GetFormat(const std::vector<BufferItem>& attributes)
		{
			std::string hashString;

			BufferItem::GetHashString(hashString, attributes);

			std::shared_ptr<BufferFormat> bufferFormat = GetCachedFormat(hashString);

			if (bufferFormat == nullptr)
			{
				bufferFormat = Engine::Create<BufferFormat>();

				for (size_t i = 0; i < attributes.size(); ++i)
					bufferFormat->AddBufferItem(attributes[i]);

				CacheFormat(hashString, bufferFormat);
			}

			return bufferFormat;
		}

		std::shared_ptr<BufferFormat> BufferFormat::GetCachedFormat(const std::string& hashString)
		{
			auto entry = Cache.find(hashString);

			if (entry != Cache.end())
				return entry->second;

			return nullptr;
		}

		std::shared_ptr<BufferFormat> BufferFormat::GetCachedFormat(int index)
		{
			if (index < 0 || index >= CacheVector.size())
				return nullptr;

			return CacheVector[index];
		}

		void BufferFormat::CacheFormat(const std::string& hashString, const std::shared_ptr<BufferFormat>& format)
		{
			format->CachedIndex = (int)CacheVector.size();

			Cache[hashString] = format;
			CacheVector.push_back(format);
		}

		void BufferFormat::CacheFormat(const std::shared_ptr<BufferFormat>& format)
		{
			std::string hash = format->GetHashString();

			if (GetCachedFormat(hash) == nullptr)
				CacheFormat(hash, format);
		}
	}
}