#pragma once

import <map>;

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		struct AttributeDataTypeEnum
		{
			enum AttributeDataType
			{
				Float32,
				Float64,
				Bool,
				Int8,
				Int16,
				Int32,
				Int64,
				UInt8,
				UInt16,
				UInt32,
				UInt64,

				Count,
				Unknown
			};
		};

		size_t GetDataSize(AttributeDataTypeEnum::AttributeDataType type, size_t elementCount = 1, size_t columnCount = 1);
		const std::string& GetDataName(AttributeDataTypeEnum::AttributeDataType type);

		struct BufferItem
		{
			typedef AttributeDataTypeEnum::AttributeDataType AttributeDataType;

			AttributeDataType Type = AttributeDataType::Unknown;
			size_t ElementCount = 1;
			std::string Name;

			size_t GetSize() const;
			void GetHashString(std::string& hash) const;

			static void GetHashString(std::string& hash, const std::vector<BufferItem>& attributes, size_t bufferArrayLength = 1);
		};

		class BufferFormat : public Object
		{
		public:
			void SetBufferArrayLength(size_t arrayLength);
			void AddBufferItem(const BufferItem& item);

			const std::vector<BufferItem>& GetBufferItems() const { return BufferItems; }
			size_t GetItemCount() const;
			size_t GetBufferSize() const { return BufferSize; }
			size_t GetBufferArraySize(size_t arrayLength = 0) const;
			size_t GetBufferArrayLength() const { return BufferArrayLength; }

			std::string GetHashString(size_t arrayLength = 0) const;
			int GetCachedIndex() const { return CachedIndex; }

			static std::shared_ptr<BufferFormat> GetFormat(const std::vector<BufferItem>& attributes);
			static std::shared_ptr<BufferFormat> GetCachedFormat(const std::string& hashString);
			static std::shared_ptr<BufferFormat> GetCachedFormat(int index);
			static void CacheFormat(const std::string& hashString, const std::shared_ptr<BufferFormat>& format);
			static void CacheFormat(const std::shared_ptr<BufferFormat>& format);

		private:
			typedef std::map<std::string, std::shared_ptr<BufferFormat>> BufferFormatMap;
			typedef std::vector<std::shared_ptr<BufferFormat>> BufferFormatVector;

			int CachedIndex = -1;
			size_t BufferArrayLength = 1;
			size_t BufferSize = 0;
			std::vector<BufferItem> BufferItems;
			std::map<std::string, size_t> IndexMap;

			static inline BufferFormatMap Cache = BufferFormatMap();
			static inline BufferFormatVector CacheVector = BufferFormatVector();
		};
	}
}

namespace Enum
{
	typedef Engine::Graphics::AttributeDataTypeEnum::AttributeDataType AttributeDataType;
}