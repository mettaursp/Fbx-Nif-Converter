#include "FbxPropertyHandler.h"
#include "FbxNodes.h"

void AddFbxNodeProperty(FbxNode* node, char typeCode, const char* data, size_t length, size_t arrayLength, bool shouldCompress)
{
	shouldCompress = length >= 1024;

	node->Header.Properties.push_back(NodeProperty{ typeCode });
	node->Header.Properties.back().InsertData(data, length, arrayLength);

	if (shouldCompress)
		node->Header.Properties.back().Compress();
}

char* GetFbxPropertyData(NodeProperty* property)
{
	return property->Data.data();
}

char GetFbxPropertyTypeCode(NodeProperty* property)
{
	return property->TypeCode;
}


template <>
char GetTypeCode<short>() { return 'Y'; }

template <>
char GetTypeCode<unsigned char>() { return 'C'; }

template <>
char GetTypeCode<int>() { return 'I'; }

template <>
char GetTypeCode<float>() { return 'F'; }

template <>
char GetTypeCode<double>() { return 'D'; }

template <>
char GetTypeCode<long long>() { return 'L'; }

template <>
char GetTypeCode<int*>() { return 'i'; }

template <>
char GetTypeCode<bool*>() { return 'b'; }

template <>
char GetTypeCode<float*>() { return 'f'; }

template <>
char GetTypeCode<double*>() { return 'd'; }

template <>
char GetTypeCode<long long*>() { return 'l'; }

template <>
char GetTypeCode<char*>() { return 'S'; }

template <>
char GetTypeCode<unsigned char*>() { return 'R'; }

template <>
char GetTypeCode<const char*>() { return GetTypeCode<char*>(); }