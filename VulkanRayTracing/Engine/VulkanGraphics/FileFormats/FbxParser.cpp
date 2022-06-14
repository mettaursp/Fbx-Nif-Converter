#include "FbxParser.h"

import <vector>;
import <iostream>;
import <map>;

#include <Engine/Assets/ParserUtils.h>
#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Vector3S.h>
#include <Engine/Math/Vector2S.h>
#include <Engine/Math/Vector3.h>
#include <Engine/Objects/Transform.h>
#include <Engine/VulkanGraphics/Scene/MeshData.h>

#include "FbxNodes.h"

using Engine::Graphics::VertexAttributeFormat;

std::shared_ptr<Engine::Graphics::MeshFormat> GetMeshFormat(const std::vector<Engine::Graphics::VertexAttributeFormat>& attributes)
{
	std::string hashString;

	Engine::Graphics::VertexAttributeFormat::GetHashString(hashString, attributes);

	std::shared_ptr<Engine::Graphics::MeshFormat> meshFormat = Engine::Graphics::MeshFormat::GetCachedFormat(hashString);

	if (meshFormat == nullptr)
	{
		meshFormat = Engine::Create<Engine::Graphics::MeshFormat>();

		for (size_t i = 0; i < attributes.size(); ++i)
			meshFormat->Push(attributes[i]);

		Engine::Graphics::MeshFormat::CacheFormat(hashString, meshFormat);
	}

	return meshFormat;
}

struct FbxGlobalSettings
{
	int UpAxis = 1;
	int UpAxisSign = 1;
	int FrontAxis = 2;
	int FrontAxisSign = 1;
	int CoordAxis = 0;
	int CoordAxisSign = 1;
	Matrix4D AxisTransformation;
	Matrix4D AxisNormalTransformation;

	int ReadProperty(FbxNode* propertyNode, size_t index)
	{
		if (propertyNode == nullptr) return -1;
		if (index >= propertyNode->Header.Properties.size()) return -1;

		return fbxEndian.read<int>(propertyNode->Header.Properties[index].Data.data());
	}

	void Fetch(FbxNode* settingsNode)
	{
		if (settingsNode == nullptr) return;

		FbxNode* propertiesNode = settingsNode->Find("Properties70");

		if (propertiesNode == nullptr) return;

		propertiesNode->ReadChildWithPropertySafe("UpAxis", 4, UpAxis);
		propertiesNode->ReadChildWithPropertySafe("UpAxisSign", 4, UpAxisSign);
		propertiesNode->ReadChildWithPropertySafe("FrontAxis", 4, FrontAxis);
		propertiesNode->ReadChildWithPropertySafe("FrontAxisSign", 4, FrontAxisSign);
		propertiesNode->ReadChildWithPropertySafe("CoordAxis", 4, CoordAxis);
		propertiesNode->ReadChildWithPropertySafe("CoordAxisSign", 4, CoordAxisSign);

		const Vector3D axis[3] = {
			Vector3D(1, 0, 0),
			Vector3D(0, 1, 0),
			Vector3D(0, 0, 1)
		};

		const int otherAxis[3][2] = {
			{ 1, 2 },
			{ 0, 2 },
			{ 0, 1 }
		};

		//Vector3D rightAxis = (double)CoordAxisSign * axis[CoordAxis];
		//Vector3D upAxis = (double)UpAxisSign * axis[UpAxis];
		//Vector3D frontAxis = (double)FrontAxisSign * axis[FrontAxis];
		int up = UpAxis - 1;
		int other = FrontAxis - 1;

		Vector3D rightAxis = (double)CoordAxisSign * axis[otherAxis[up][!other]];
		Vector3D upAxis = (double)UpAxisSign * axis[up];
		Vector3D frontAxis = (double)FrontAxisSign * axis[otherAxis[up][other]];

		AxisTransformation = Matrix4D(Vector3D(), rightAxis, upAxis, frontAxis).Inverse();
		AxisNormalTransformation = AxisTransformation;
		AxisNormalTransformation.Inverse().Transpose();
	}
};

struct FbxNodeData
{
	long long Id = 0;
	std::string Type;
	size_t MeshIndex = 0;
	std::shared_ptr<Engine::Graphics::MeshData> Mesh;
	std::shared_ptr<Engine::Graphics::MeshFormat> Format;
	std::vector<FbxNodeData*> ReferencedBy;
	std::vector<FbxNodeData*> References;
	size_t BufferLength = 0;
	int* IndexBuffer = nullptr;
	double* WeightBuffer = nullptr;
	Matrix4D Transformation;
	Matrix4D NormalTransformation;
};


void FbxParser::Parse(std::istream& stream)
{
	FbxFileStructure fbxFile;

	fbxFile.ReadNodes(stream);

	std::vector<FbxNode*> fbxRootNodes;

	auto tabs = [](std::ostream& stream, size_t count) -> std::ostream&
	{
		for (size_t i = 0; i < count; ++i)
			stream << '\t';

		return stream;
	};

	for (int i = -1; i < (int)fbxFile.Nodes.size(); ++i)
	{
		FbxNode& node = i == -1 ? fbxFile.RootNode : fbxFile.Nodes[i];

		if (!node.Header.IsNull() || i == -1)
		{
			if (DebugPrint)
			{
				tabs(std::cout, node.Depth) << (node.Header.Name.size() == 0 ? std::string("<Node>") : node.Header.Name) << " [ ";

				if (node.Header.Properties.size() > 0)
				{
					for (size_t j = 0; j < node.Header.Properties.size(); ++j)
						tabs(std::cout << std::endl, node.Depth + 1) << node.Header.Properties[j];

					tabs(std::cout << std::endl, node.Depth);
				}

				std::cout << "]:" << std::endl;
			}

			node.Children.resize(node.ChildIndices.size());

			for (size_t j = 0; j < node.Children.size(); ++j)
				node.Children[j] = &fbxFile.Nodes[node.ChildIndices[j]];

		}
	}

	FbxGlobalSettings settings;

	settings.Fetch(fbxFile.RootNode.Find("GlobalSettings"));

	FbxNode* objectsNode = fbxFile.RootNode.Find("Objects");
	
	for (size_t i = 0; i < objectsNode->Children.size(); ++i)
	{
		FbxNode* node = objectsNode->Children[i];

		long long id = fbxEndian.read<long long>(node->Header.Properties[0].Data.data());

		fbxFile.FbxObjectNodes[id] = node;
	}

	for (size_t i = 0; i < objectsNode->Children.size(); ++i)
	{
		FbxNode* node = objectsNode->Children[i];
		FbxObjectNode* fbxNode = node->MakeObjectNode();

		long long id = fbxEndian.read<long long>(node->Header.Properties[0].Data.data());

		fbxNode->Id = id;
	}

	FbxNode* connectionsNode = fbxFile.RootNode.Find("Connections");

	if (connectionsNode != nullptr)
	{
		for (size_t i = 0; i < connectionsNode->Children.size(); ++i)
		{
			FbxNode* connection = connectionsNode->Children[i];

			long long id1 = fbxEndian.read<long long>(connection->Header.Properties[1].Data.data());
			long long id2 = fbxEndian.read<long long>(connection->Header.Properties[2].Data.data());

			auto index1 = fbxFile.FbxObjectNodes.find(id1);
			auto index2 = fbxFile.FbxObjectNodes.find(id2);

			if (index1 != fbxFile.FbxObjectNodes.end() && index2 != fbxFile.FbxObjectNodes.end())
			{
				index1->second->ObjectNode->ReferencedBy.push_back(index2->second->ObjectNode.get());
				index2->second->ObjectNode->References.push_back(index1->second->ObjectNode.get());
			}
			else if (id2 == 0)
				fbxRootNodes.push_back(index1->second);
		}
	}

	std::map<long long, size_t> packageNodes;
	std::map<long long, size_t> packageNodeParents;

	auto getNodeIndex = [this, &packageNodes](long long nodeId) -> size_t
	{
		size_t index = 0;
		auto mapIndex = packageNodes.find(nodeId);

		if (mapIndex == packageNodes.end())
		{
			index = Package->Nodes.size();
			packageNodes[nodeId] = index;
			Package->Nodes.push_back(Engine::Graphics::ModelPackageNode());
		}
		else
			index = mapIndex->second;

		return index;
	};

	auto getMaterialNodeIndex = [this, &packageNodes](long long nodeId) -> size_t
	{
		size_t index = 0;
		auto mapIndex = packageNodes.find(nodeId);

		if (mapIndex == packageNodes.end())
		{
			index = Package->Materials.size();
			packageNodes[nodeId] = index;
			Package->Materials.push_back(Engine::Graphics::ModelPackageMaterial());
		}
		else
			index = mapIndex->second;

		return index;
	};

	for (size_t i = 0; i < objectsNode->Children.size(); ++i)
	{
		FbxNode* node = objectsNode->Children[i];

		long long id = fbxEndian.read<long long>(node->Header.Properties[0].Data.data());

		FbxObjectNode* fbxNode = node->ObjectNode.get();

		std::shared_ptr<Engine::Graphics::MeshFormat> format;
		std::shared_ptr<Engine::Graphics::MeshData> data;

		std::string objectType;

		if (node->Header.Properties.size() >= 2 && node->Header.Properties[2].TypeCode == 'S')
			objectType = vectorToString(node->Header.Properties[2].Data);

		if (node->Header.Name == "NodeAttribute")
		{
			FbxObjectNode* model = fbxNode->FindRefBy("Model", "Root");
			model = model != nullptr ? model : fbxNode->FindRefBy("Model", "Limb");

			if (model != nullptr)
			{
				size_t packageIndex = getNodeIndex(model->Id);

				Engine::Graphics::ModelPackageNode& packageNode = Package->Nodes[packageIndex];

				packageNode.Name = model->Parent->Header.Properties[1].Data.data();

				FbxObjectNode* parentModel = model->FindRefBy("Model");

				if (parentModel != nullptr)
					packageNode.AttachedTo = getNodeIndex(parentModel->Id);

				FbxObjectNode* deformer = model->FindRefBy("Deformer", "Cluster");

				if (deformer != nullptr)
				{
					FbxNode* transformBuffer = deformer->Parent->Find("Transform");

					if (transformBuffer != nullptr)
					{
						Matrix4D* transformData = reinterpret_cast<Matrix4D*>(transformBuffer->Header.Properties[0].Data.data());
						
						packageNode.Transform = Engine::Create<Engine::Transform>();
						packageNode.Transform->SetTransformation(*transformData);
					}
				}
			}
		}
		else if (node->Header.Name == "Geometry" && objectType == "Mesh")
		{
			FbxNode* indexBuffer = node->Find("PolygonVertexIndex");

			if (indexBuffer != nullptr)
			{
				FbxObjectNode* model = nullptr;
				FbxObjectNode* deformer = fbxNode->FindRef("Deformer", "Skin");

				if (deformer != nullptr)
				{
					FbxObjectNode* subdeformer = deformer->FindRef("Deformer", "Cluster");

					if (subdeformer != nullptr)
					{
						model = subdeformer->FindRef("Model", "Root");
						model = model != nullptr ? model : subdeformer->FindRef("Model", "Limb");
					}
				}

				FbxObjectNode* meshModel = fbxNode->FindRefBy("Model", "Mesh");

				FbxNode* vertexBuffer = node->Find("Vertices");

				std::vector<Engine::Graphics::VertexAttributeFormat> attributes;
				std::vector<void*> dataBuffers;
				std::vector<int> indexBufferData;

				size_t indices = (size_t)indexBuffer->Header.Properties[0].ArrayLength;

				indexBufferData.reserve(indices);

				for (size_t i = 0; i < indices; i += 3)
				{
					int vertex1 = fbxEndian.read<int>(indexBuffer->Header.Properties[0].Data.data() + 4 * (i + 0));
					int vertex2 = fbxEndian.read<int>(indexBuffer->Header.Properties[0].Data.data() + 4 * (i + 1));
					int vertex3 = fbxEndian.read<int>(indexBuffer->Header.Properties[0].Data.data() + 4 * (i + 2));

					if (vertex3 < 0)
					{
						indexBufferData.push_back(vertex1);
						indexBufferData.push_back(vertex2);
						indexBufferData.push_back(vertex3 ^ -1);
					}
					else
					{
						int vertex4 = fbxEndian.read<int>(indexBuffer->Header.Properties[0].Data.data() + 4 * (i + 3));

						indexBufferData.push_back(vertex1);
						indexBufferData.push_back(vertex2);
						indexBufferData.push_back(vertex3);

						indexBufferData.push_back(vertex1);
						indexBufferData.push_back(vertex3);
						indexBufferData.push_back(vertex4 ^ -1);

						++i;
					}
				}

				size_t vertexCount = (size_t)vertexBuffer->Header.Properties[0].ArrayLength / 3;

				attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "position", 0 });
				dataBuffers.push_back(vertexBuffer->Header.Properties[0].Data.data());

				Vector3SD* vertices = reinterpret_cast<Vector3SD*>(vertexBuffer->Header.Properties[0].Data.data());

				auto fetchAttribute = []<typename T = Vector3SD>(FbxNode* buffer, std::vector<VertexAttributeFormat>& attributes, std::vector<void*>& dataBuffers, const std::string& index, const std::string& name, size_t elements) -> T*
				{
					if (buffer == nullptr) return nullptr;

					FbxNode* mapping = buffer->Find("MappingInformationType");

					if (mapping != nullptr)
					{
						std::string type = vectorToString(mapping->Header.Properties[0].Data);

						if (type != "ByVertice" && type != "ByPolygonVertex")
						{
							std::cout << "attribute '" << index << "' unsupported mapping type: '" << vectorToString(mapping->Header.Properties[0].Data) << std::endl;
							return nullptr;
						}
					}

					FbxNode* referenceType = buffer->Find("ReferenceInformationType");

					if (referenceType != nullptr)
					{
						std::string type = vectorToString(referenceType->Header.Properties[0].Data);

						if (type == "IndexToDirect")
						{
							std::cout << "attribute '" << index << "' unsupported reference type: '" << type << std::endl;
							return nullptr;
						}
					}

					FbxNode* dataBuffer = buffer->Find(index);

					if (dataBuffer == nullptr) return nullptr;

					attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, elements, name, dataBuffers.size() });
					dataBuffers.push_back(dataBuffer->Header.Properties[0].Data.data());

					return reinterpret_cast<T*>(dataBuffer->Header.Properties[0].Data.data());
				};

				Vector3SD* normals = fetchAttribute(node->Find("LayerElementNormal"), attributes, dataBuffers, "Normals", "normal", 3);
				Vector2SD* uvs = fetchAttribute.operator()<Vector2SD>(node->Find("LayerElementUV"), attributes, dataBuffers, "UV", "textureCoords", 2);
				Vector3SD* binormals = fetchAttribute(node->Find("LayerElementBinormal"), attributes, dataBuffers, "Binormals", "binormal", 3);
				Vector3SD* tangents = fetchAttribute(node->Find("LayerElementTangent"), attributes, dataBuffers, "Tangents", "tangent", 3);

				FbxNode* shapeKeyGeometry = nullptr;

				for (size_t i = 0; i < node->ObjectNode->References.size(); ++i)
				{
					FbxObjectNode* blendShape = node->ObjectNode->FindRef("Deformer", "BlendShape");

					if (blendShape != nullptr)
					{
						FbxObjectNode* blendShapeChannel = blendShape->FindRef("Deformer", "BlendShapeChannel");

						if (blendShapeChannel != nullptr)
						{
							FbxObjectNode* shapeKey = blendShapeChannel->FindRef("Geometry", "Shape");

							if (shapeKey != nullptr)
								shapeKeyGeometry = shapeKey->Parent;
						}
					}
				}

				if (shapeKeyGeometry != nullptr)
				{
					FbxNode* shapeKeyVertexBuffer = shapeKeyGeometry->Find("Vertices");

					if (shapeKeyVertexBuffer != nullptr && shapeKeyVertexBuffer->Header.Properties.size() > 0 && shapeKeyVertexBuffer->Header.Properties[0].ArrayLength / 3 == vertexCount)
					{
						attributes.push_back(VertexAttributeFormat{ Enum::AttributeDataType::Float64, 3, "morphPosition1", dataBuffers.size() });
						dataBuffers.push_back(shapeKeyVertexBuffer->Header.Properties[0].Data.data());
					}
				}

				if (uvs != nullptr)
					for (size_t vertex = 0; vertex < vertexCount; ++vertex)
						uvs[vertex].Y = 1 - uvs[vertex].Y;

				format = GetMeshFormat(attributes);
				data = Engine::Create<Engine::Graphics::MeshData>();

				data->SetFormat(format);
				data->PushVertices(vertexCount, false);
				data->PushIndices(indexBufferData);

				format->Copy(dataBuffers.data(), data->GetData(), format, vertexCount);

				fbxNode->MeshIndex = ImportedMeshes.size();

				ImportedMeshes.push_back(ImportedFbxMesh{ format, data });

				fbxNode->Mesh = data;
				fbxNode->Format = format;

				if (model != nullptr)
				{
					size_t packageIndex = getNodeIndex(model->Id);

					Engine::Graphics::ModelPackageNode& packageNode = Package->Nodes[packageIndex];

					packageNode.Mesh = data;
					packageNode.Format = format;

					if (meshModel != nullptr)
					{
						FbxObjectNode* material = meshModel->FindRef("Material");

						if (material != nullptr)
							packageNode.MaterialIndex = getMaterialNodeIndex(material->Id);
					}
				}
			}
			else
			{

			}
		}
		else if (node->Header.Name == "Material")
		{
			size_t materialIndex = getMaterialNodeIndex(fbxNode->Id);

			Engine::Graphics::ModelPackageMaterial& packageNode = Package->Materials[materialIndex];

			packageNode.Name = node->Header.Properties[1].Data.data();

			FbxNode* propertiesNode = node->Find("Properties70");

			if (propertiesNode != nullptr)
			{
				FbxNode* diffuseNode = propertiesNode->FindWith("Diffuse");
				
				if (diffuseNode != nullptr)
				{
					packageNode.DiffuseColor.R = (float)diffuseNode->ReadProperty<double>(4);
					packageNode.DiffuseColor.G = (float)diffuseNode->ReadProperty<double>(5);
					packageNode.DiffuseColor.B = (float)diffuseNode->ReadProperty<double>(6);
				}

				FbxNode* specularNode = propertiesNode->FindWith("Specular");

				if (specularNode != nullptr)
				{
					packageNode.SpecularColor.R = (float)specularNode->ReadProperty<double>(4);
					packageNode.SpecularColor.G = (float)specularNode->ReadProperty<double>(5);
					packageNode.SpecularColor.B = (float)specularNode->ReadProperty<double>(6);
				}

				FbxNode* ambientNode = propertiesNode->FindWith("Ambient");

				if (ambientNode != nullptr)
				{
					packageNode.AmbientColor.R = (float)ambientNode->ReadProperty<double>(4);
					packageNode.AmbientColor.G = (float)ambientNode->ReadProperty<double>(5);
					packageNode.AmbientColor.B = (float)ambientNode->ReadProperty<double>(6);
				}

				FbxNode* emissiveNode = propertiesNode->FindWith("Emissive");

				if (emissiveNode != nullptr)
				{
					packageNode.EmissiveColor.R = (float)emissiveNode->ReadProperty<double>(4);
					packageNode.EmissiveColor.G = (float)emissiveNode->ReadProperty<double>(5);
					packageNode.EmissiveColor.B = (float)emissiveNode->ReadProperty<double>(6);
				}

				FbxNode* shininessNode = propertiesNode->FindWith("Shininess");

				if (shininessNode != nullptr)
					packageNode.Shininess = (float)shininessNode->ReadProperty<double>(4);

				FbxNode* opacityNode = propertiesNode->FindWith("Opacity");

				if (opacityNode != nullptr)
					packageNode.Alpha = (float)opacityNode->ReadProperty<double>(4);
			}

			for (size_t i = 0; i < fbxNode->References.size(); ++i)
			{
				FbxObjectNode* texture = fbxNode->References[i];
				FbxNode* textureNode = texture->Parent;

				if (textureNode->Header.Name == "Texture")
				{
					std::string slotName = textureNode->Header.Properties[1].Data.data();

					FbxNode* fileName = textureNode->Find("FileName");
					
					if (fileName != nullptr)
					{
						std::string path(fileName->Header.Properties[0].Data.data(), fileName->Header.Properties[0].Data.size());

						if (slotName == "Diffuse Texture")
							packageNode.Diffuse = path;
						else if (slotName == "Normal Texture")
							packageNode.Normal = path;
						else if (slotName == "Specular Texture")
							packageNode.Specular = path;
					}
				}
			}

		}
		else if (node->Header.Name == "Deformer")
		{
			FbxNode* indexBuffer = node->Find("Indexes");
			FbxNode* weightBuffer = node->Find("Weights");
			FbxNode* transformBuffer = node->Find("Transform");

			if (indexBuffer == nullptr || weightBuffer == nullptr || transformBuffer == nullptr) continue;
			
			Matrix4D* transformData = reinterpret_cast<Matrix4D*>(transformBuffer->Header.Properties[0].Data.data());

			fbxNode->Transformation = *transformData;

			if (!TransposedMatrices)
				fbxNode->Transformation.Transpose();

			fbxNode->BufferLength = indexBuffer->Header.Properties[0].ArrayLength;
			fbxNode->IndexBuffer = reinterpret_cast<int*>(indexBuffer->Header.Properties[0].Data.data());
			fbxNode->WeightBuffer = reinterpret_cast<double*>(weightBuffer->Header.Properties[0].Data.data());
		}
	}

	for (size_t i = 0; i < Package->Nodes.size(); ++i)
		if (Package->Nodes[i].AttachedTo != (size_t)-1)
			Package->Nodes[i].Transform->SetParent(Package->Nodes[Package->Nodes[i].AttachedTo].Transform);

	for (auto index : fbxFile.FbxObjectNodes)
	{
		FbxNode* node = index.second;
		FbxObjectNode* object = node->ObjectNode.get();

		if (node->Header.Name == "Deformer" && object->BufferLength != 0)
		{
			for (size_t i = 0; i < object->ReferencedBy.size(); ++i)
			{
				for (size_t j = 0; j < object->ReferencedBy[i]->ReferencedBy.size(); ++j)
				{
					FbxObjectNode* parentNode = object->ReferencedBy[i]->ReferencedBy[j];

					if (parentNode->Parent->Header.Name == "Geometry")
					{
						ImportedMeshes[parentNode->MeshIndex].Transform = Engine::Create<Engine::Transform>();
						ImportedMeshes[parentNode->MeshIndex].Transform->SetTransformation(object->Transformation);
					}
				}
			}
		}
	}
}