#pragma once

import <vector>;
import <memory>;
import <string>;
import <map>;

#include <Engine/Objects/Object.h>
#include <Engine/VulkanGraphics/Core/BufferFormat.h>

namespace Engine
{
	namespace Graphics
	{
		struct VertexAttributeFormat
		{
			typedef AttributeDataTypeEnum::AttributeDataType AttributeDataType;

			AttributeDataType Type = AttributeDataType::Float32;
			size_t ElementCount = 1;
			std::string Name;
			size_t Binding = 0;
			size_t Offset = 0;
			size_t Index = 0;

			size_t GetElementSize() const;
			size_t GetSize() const;

			void Copy(const void* source, void* destination, AttributeDataType destinationType) const;
			void GetHashString(std::string& hash) const;

			static void GetHashString(std::string& hash, const std::vector<VertexAttributeFormat>& attributes);
		};

		class MeshFormat : public Object
		{
		public:
			size_t GetVertexSize(size_t binding) const;
			size_t GetBindingCount() const { return VertexSizes.size(); }

			void Push(const VertexAttributeFormat& attribute);
			void Copy(const void* const* source, void** destination, const std::shared_ptr<MeshFormat>& destinationFormat, size_t vertices, size_t offsetCount = 0) const;
			void WriteAttribute(const void* const * source, void* destination, size_t attribute, size_t element) const;
			void WriteAttribute(const void* const * source, void* destination, const std::string& attribute, size_t element) const;
			std::string GetHashString() const;
			int GetCachedIndex() const { return CachedIndex; }

			const std::vector<VertexAttributeFormat>& GetAttributes() const { return Attributes; }
			const VertexAttributeFormat* GetAttribute(const std::string& name) const;

			static std::shared_ptr<MeshFormat> GetFormat(const std::vector<VertexAttributeFormat>& attributes);
			static std::shared_ptr<MeshFormat> GetCachedFormat(const std::string& hashString);
			static std::shared_ptr<MeshFormat> GetCachedFormat(int index);
			static void CacheFormat(const std::string& hashString, const std::shared_ptr<MeshFormat>& format);
			static void CacheFormat(const std::shared_ptr<MeshFormat>& format);

		private:
			typedef std::map<std::string, std::shared_ptr<MeshFormat>> MeshFormatMap;
			typedef std::vector<std::shared_ptr<MeshFormat>> MeshFormatVector;

			std::vector<VertexAttributeFormat> Attributes;
			std::map<std::string, size_t> IndexMap;

			static inline MeshFormatMap Cache = MeshFormatMap();
			static inline MeshFormatVector CacheVector = MeshFormatVector();
			static inline int CachedFormats = 0;

			size_t Bindings = 0;
			std::vector<size_t> VertexSizes;
			int CachedIndex = -1;
		};

		class MeshData : public Object
		{
		public:
			void SetFormat(const std::shared_ptr<MeshFormat>& format);
			const std::shared_ptr<MeshFormat>& GetFormat() const { return Format; }

			size_t GetTotalSize(size_t binding) const;
			size_t GetBindingCount() const;
			size_t GetVertices() const { return Vertices; }
			size_t GetTriangleVertices() const { return Indices.size(); }
			const void* const* GetData() const { return DataPointers.data(); }
			void** GetData() { return DataPointers.data(); }
			void* GetIndexData() { return Indices.data(); }
			const void* GetIndexData() const { return Indices.data(); }

			void PushVertices(size_t count = 1, bool pushIndices = false);
			void PushIndices(size_t count = 1);
			void PushIndices(const std::vector<int>& indices);
			void PushIndex(size_t location, int index);
			void ResetData();

			//const std::vector<unsigned char>& GetVertexBuffer() const { return Data; }
			const std::vector<int>& GetIndexBuffer() const { return Indices; }

		private:
			std::shared_ptr<MeshFormat> Format;

			size_t Vertices = 0;
			std::vector<std::vector<unsigned char>> Data;
			std::vector<void*> DataPointers;
			std::vector<int> Indices;
		};
	}
}