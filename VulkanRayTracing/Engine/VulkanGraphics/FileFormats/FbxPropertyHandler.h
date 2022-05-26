#pragma once

template <typename T>
struct ArrayWrapper
{
	const T* Data = nullptr;
	size_t Length = 0;
	bool ShouldCompress = false;

	ArrayWrapper(const T* data, size_t length, bool shouldCompress = false) : Data(data), Length(length), ShouldCompress(shouldCompress) {}
};

template <typename T>
char GetTypeCode()
{
	//static_assert(false);
	throw "bad type";
	return 0;
}

template <>
char GetTypeCode<short>();

template <>
char GetTypeCode<unsigned char>();

template <>
char GetTypeCode<int>();

template <>
char GetTypeCode<float>();

template <>
char GetTypeCode<double>();

template <>
char GetTypeCode<long long>();

template <>
char GetTypeCode<int*>();

template <>
char GetTypeCode<bool*>();

template <>
char GetTypeCode<float*>();

template <>
char GetTypeCode<double*>();

template <>
char GetTypeCode<long long*>();

template <>
char GetTypeCode<char*>();

template <>
char GetTypeCode<const char*>();

template <>
char GetTypeCode<unsigned char*>();

struct NodeProperty;
struct FbxNode;

void AddFbxNodeProperty(FbxNode* node, char typeCode, const char* data, size_t length, size_t arrayLength = 0, bool shouldCompress = false);
char* GetFbxPropertyData(NodeProperty* property);
char GetFbxPropertyTypeCode(NodeProperty* property);

template <typename T>
struct PropertyHandler
{
	static bool ReadSafe(NodeProperty* property, T& output)
	{
		if (property == nullptr) return false;
		else if (GetFbxPropertyTypeCode(property) != GetTypeCode<T>()) return false;

		output = *reinterpret_cast<T*>(GetFbxPropertyData(property));

		return true;
	}

	static T Read(NodeProperty* property)
	{
		if (property == nullptr) throw "attempt to read nonexistant property";
		else if (GetFbxPropertyTypeCode(property) != GetTypeCode<T>()) throw "attempt to read property with mismatching type";

		return *reinterpret_cast<T*>(GetFbxPropertyData(property));
	}

	static void Add(FbxNode* node, const T& value)
	{
		AddFbxNodeProperty(node, GetTypeCode<T>(), reinterpret_cast<const char*>(&value), sizeof(value));
	}
};

template <typename T>
struct PropertyHandler<T*>
{
	static bool ReadSafe(NodeProperty* property, T*& output)
	{
		if (property == nullptr) return false;
		else if (GetFbxPropertyTypeCode(property) != GetTypeCode<T*>()) return false;

		output = reinterpret_cast<T*>(GetFbxPropertyData(property));

		return true;
	}

	static T* Read(NodeProperty* property)
	{

		if (property == nullptr) throw "attempt to read nonexistant property";
		else if (GetFbxPropertyTypeCode(property) != GetTypeCode<T>()) throw "attempt to read property with mismatching type";

		return reinterpret_cast<T*>(GetFbxPropertyData(property));
	}
};

template <typename T>
struct PropertyHandler<ArrayWrapper<T>>
{
	static void Add(FbxNode* node, const ArrayWrapper<T>& value)
	{
		AddFbxNodeProperty(node, GetTypeCode<T*>(), reinterpret_cast<const char*>(value.Data), sizeof(value.Data[0]) * value.Length, value.Length, /*value.ShouldCompress*/false);
	}
};