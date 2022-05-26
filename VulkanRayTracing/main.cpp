import <cassert>;
import <cinttypes>;
import <cstdio>;
import <cstdlib>;
import <cstring>;
import <csignal>;
import <iostream>;
import <sstream>;
import <memory>;
import <fstream>;
import <set>;

#include <Windows.h>

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan_structs.hpp>

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

#include <Engine/VulkanGraphics/Core/GraphicsContext.h>
#include <Engine/VulkanGraphics/Core/GraphicsWindow.h>
#include <Engine/VulkanGraphics/Core/ImageResource.h>
#include <Engine/VulkanGraphics/Core/Texture.h>
#include <Engine/VulkanGraphics/Core/Shader.h>
#include <Engine/VulkanGraphics/Core/ShaderPipeline.h>
#include <Engine/VulkanGraphics/Core/FrameBuffer.h>
#include <Engine/VulkanGraphics/Core/SwapChain.h>
#include <Engine/VulkanGraphics/Core/FrameSwapFences.h>
#include <Engine/VulkanGraphics/Core/VulkanErrorHandling.h>
#include <Engine/VulkanGraphics/Core/Mesh.h>
#include <Engine/VulkanGraphics/Core/RenderQueue.h>
#include <Engine/VulkanGraphics/Core/Uniform.h>
#include <Engine/VulkanGraphics/Core/PhongForwardPipeline.h>
#include <Engine/VulkanGraphics/Core/ShaderProgram.h>
#include <Engine/VulkanGraphics/Core/GraphicsRenderer.h>
#include <Engine/VulkanGraphics/Scene/MeshAsset.h>


#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Matrix3.h>
#include <Engine/Math/Vector3.h>
#include <Engine/VulkanGraphics/Scene/Camera.h>
#include <Engine/VulkanGraphics/Scene/Model.h>
#include <Engine/Objects/Transform.h>
#include <Engine/VulkanGraphics/Scene/SceneDrawOperation.h>
#include <Engine/VulkanGraphics/Scene/Scene.h>
#include <Engine/Assets/ModelPackageAsset.h>

using namespace Engine;

void draw();
void init(int, char**);

std::string name; 

bool initVulkan = true;
bool loadTestModels = false;

std::shared_ptr<Graphics::GraphicsContext> context;
std::shared_ptr<Graphics::GraphicsWindow> window;
std::shared_ptr<Graphics::ImageResource> testimage;
std::shared_ptr<Graphics::Shader> fragShader;
std::shared_ptr<Graphics::Shader> vertShader;
std::shared_ptr<Graphics::SwapChain> swapChain;
std::shared_ptr<Graphics::Camera> camera;
std::shared_ptr<Graphics::Model> model1;
std::shared_ptr<Transform> transform1;
std::shared_ptr<Graphics::Model> model2;
std::shared_ptr<Transform> transform2;
std::shared_ptr<Graphics::PhongForwardPipeline> phongForward;
std::shared_ptr<Graphics::Scene> scene;
std::shared_ptr<Graphics::SceneDrawOperation> drawOperation;
std::shared_ptr<Graphics::ShaderProgram> program;
std::shared_ptr<Graphics::GraphicsRenderer> renderer;

std::vector<std::shared_ptr<Graphics::Texture>> textures;


Matrix4F lookAt;
Matrix4F proj;
Matrix4F model;
Matrix4F modelt2;
Matrix4F viewProj;
Matrix4F modelViewProj;
Graphics::CombinedSamplerUniform* sampler = nullptr;
Graphics::BufferObjectUniform* sceneInfo = nullptr;
std::shared_ptr<Graphics::MeshAsset> meshAsset;
std::shared_ptr<Graphics::MeshAsset> meshAsset2;
std::shared_ptr<Graphics::MeshAsset> hairMeshAsset;
std::vector<std::shared_ptr<Transform>> transforms;
float spinAngle = 0.1f;

float degreesToRadians(float degrees)
{
	return degrees * 3.1415926535f / 180;
}

void draw()
{

	//if (loadTestModels)
		sampler->Set(0, textures[0]);
	//else
	//	sampler->Set(0, nullptr);

	sceneInfo->Data.Time += 1.f / 144.f;
	sceneInfo->UpdateData();

	for (size_t i = 0; i < transforms.size(); ++i)
	{
		const Matrix4& transformation = transforms[i]->GetTransformation();
		Matrix4 rotation = Matrix4(transformation.Translation()) * Matrix4F::PitchRotation(-(float)degreesToRadians(spinAngle)) * Matrix4(-transformation.Translation());
		transforms[i]->SetTransformation(rotation * transformation);
		transforms[i]->Update(0);
	}
}
struct hairobj
{
	std::shared_ptr<ModelPackageAsset> asset;
	std::vector<std::shared_ptr<Graphics::Model>> models;
	std::shared_ptr<Transform> transform;
};
std::vector<hairobj> hairs;
void init(int argc, char** argv)
{

	std::cout << argc << std::endl;

	std::string inputDirectory = "import/";
	std::string outputDirectory = "export/";

	for (int i = 0; i < argc; ++i)
	{
		std::cout << argv[i] << std::endl;
		std::string arg = argv[i];

		if (arg == "--no-vulkan")
			initVulkan = false;

		if (arg == "--import-dir" && i + 1 < argc)
			inputDirectory = argv[i + 1];

		if (arg == "--export-dir" && i + 1 < argc)
			outputDirectory = argv[i + 1];
	}

	meshAsset = Engine::Create<Graphics::MeshAsset>();
	meshAsset2 = Engine::Create<Graphics::MeshAsset>();
	hairMeshAsset = Engine::Create<Graphics::MeshAsset>();

	if (initVulkan)
	{
		context = Engine::Create<Graphics::GraphicsContext>();
		context->SetUp();

		{
			auto physicalDeviceProperties = context->GetDevice().Gpu->getProperties();
			fprintf(stderr, "Selected GPU %d: %s, type: %s\n", context->GetPrimaryDevice(), physicalDeviceProperties.deviceName.data(),
				to_string(physicalDeviceProperties.deviceType).c_str());
		}

		//if (loadTestModels)
		{
			textures.push_back(Engine::Create<Graphics::Texture>());
			textures[0]->AttachToContext(context);
		}
	}

	lookAt = Matrix4F::PitchRotation(0.2f) * Matrix4F(0, -2, -10);

	float val = 1 / std::tan((float)degreesToRadians(45.0f) / 2);

	proj.Projection(val / 2, 0.1f, 100, 1, 1);

	auto d2r = [](float val)
	{
		return -(float)degreesToRadians(val);
	};

	model = Matrix4F(3, -2, 0) * Matrix4F::NewScale(0.5f, 0.5f, 0.5f) * Matrix4F::YawRotation(-(float)degreesToRadians(135));
		// * Matrix4F::YawRotation(-(float)degreesToRadians(90)) * Matrix4F::RollRotation(-(float)degreesToRadians(90)) * Matrix4F::PitchRotation(-(float)degreesToRadians(90));
	modelt2 = Matrix4F(2, 0, 0);

	viewProj = proj * lookAt;

	std::vector<std::string> assets = {
		//"models/10200169_m_mohicandread_aout_morph.fbx",
		//"models/10200238_f_freeconcept020_aout.fbx", "models/exportedfbx.fbx", "models/test_ascii_2.fbx"
		//"models/10200238_f_freeconcept020_a.nif"//, "models/10200169_m_mohicandread_a.nif", "models/10200092_m_shineangelichair_a.nif", "models/10200047_f_dambihair_a.nif"//,
		//"models/10200047_f_dambihair_aout.fbx", "models/10200092_m_shineangelichair_aout.fbx", "models/10200169_m_mohicandread_aout.fbx", "models/10200238_f_freeconcept020_aout.fbx"
	};

	auto canUseDir = [](const std::filesystem::path& path)
	{
		bool exists = std::filesystem::exists(path);
		bool isDirectory = std::filesystem::is_directory(path);

		return exists && isDirectory;
	};

	std::filesystem::path inputPath(inputDirectory);
	std::filesystem::path outputPath(outputDirectory);

	bool canUseInput = canUseDir(inputPath);
	bool canUseOutput = canUseDir(outputPath);

	if (!canUseInput)
		std::cout << "failed to open input directory: '" << inputDirectory << "'" << std::endl;

	if (!canUseOutput)
		std::cout << "failed to open output directory: '" << outputDirectory << "'" << std::endl;

	if (canUseInput)
	{
		for (auto& file : std::filesystem::directory_iterator(inputDirectory))
		{
			assets.push_back(file.path().filename().string());
		}
	}

	bool doExport = canUseOutput;

	hairs.resize(assets.size());


	if (initVulkan)
	{
		camera = Engine::Create<Engine::Graphics::Camera>();
		camera->proj = viewProj;

		scene = Engine::Create<Graphics::Scene>();
		drawOperation = Engine::Create<Graphics::SceneDrawOperation>();
		drawOperation->TargetScene = scene;
		drawOperation->ViewCamera = camera;

		program = Engine::Create<Graphics::ShaderProgram>();
		program->DrawOperation = drawOperation;

		renderer = Engine::Create<Graphics::GraphicsRenderer>();
		renderer->AddProgram(program);

		if (loadTestModels)
		{
			model1 = Engine::Create<Engine::Graphics::Model>();
			transform1 = Engine::Create<Engine::Transform>();
			model2 = Engine::Create<Engine::Graphics::Model>();
			transform2 = Engine::Create<Engine::Transform>();

			scene->AddObject(model1);
			scene->AddObject(model2);

			model1->SetParent(transform1);
			model2->SetParent(transform2);

			model1->MeshAsset = meshAsset;
			model2->MeshAsset = meshAsset2;

			transform1->SetTransformation(model);
			transform2->SetTransformation(modelt2);

			transforms.push_back(transform1);
			transforms.push_back(transform2);
		}
	}

	for (size_t i = 0; i < hairs.size(); ++i)
	{
		hairs[i].asset = Engine::Create<Engine::ModelPackageAsset>();
		hairs[i].transform = Engine::Create<Transform>();

		hairs[i].asset->SetImportPath(inputDirectory);

		if (canUseOutput)
			hairs[i].asset->SetExportPath(outputDirectory);

		hairs[i].asset->SetPath(assets[i], Enum::AssetType::GameAsset, std::ios::binary);
		hairs[i].asset->Load();

		std::cout << "imported '" << (inputDirectory + assets[i]) << "'" << std::endl;

		if (doExport)
		{
			std::filesystem::path fileName(assets[i]);

			if (fileName.extension().string() == ".fbx")
				hairs[i].asset->Export(".nif");
			else
				hairs[i].asset->Export(".fbx");

			if (fileName.extension().string() == ".fbx")
				fileName.replace_extension(".nif");
			else
				fileName.replace_extension(".fbx");

			std::cout << "exported '" << (outputDirectory + fileName.string()) << "'" << std::endl;
		}


		const std::vector<std::shared_ptr<Graphics::MeshAsset>>& meshes = hairs[i].asset->GetImportedMeshes();
		const std::vector<std::shared_ptr<Transform>>& meshTransforms = hairs[i].asset->GetMeshTransforms();
		const Graphics::ModelPackage& package = hairs[i].asset->GetPackage();

		int x = (int)i % 3;
		int y = (int)i / 3;
		Matrix4 transformation = Matrix4(1.5 * Vector3((double)x - 1.5, (double)y - 1.5, 0)) * Matrix4::NewScale(0.02, 0.02, 0.02) * Matrix4::EulerAnglesRotation(d2r(90), d2r(45), 0);

		std::set<void*> reportedFormats;

		if (package.Nodes.size() > 0)
		{
			std::shared_ptr<Transform> modelTransform = Engine::Create<Transform>();
			modelTransform->SetTransformation(transformation);

			hairs[i].transform = modelTransform;

			transforms.push_back(hairs[i].transform);

			std::vector<std::shared_ptr<Transform>> hairTransforms(package.Nodes.size());

			for (size_t j = 0; j < package.Nodes.size(); ++j)
			{
				std::shared_ptr<Transform> transform = Engine::Create<Transform>();

				transform->SetTransformation(package.Nodes[j].Transform->GetTransformation());

				if (package.Nodes[j].Mesh != nullptr)
				{
					std::shared_ptr<Graphics::MeshAsset> asset = Engine::Create<Graphics::MeshAsset>();
					asset->SetMeshData(package.Nodes[j].Mesh);

					auto index = reportedFormats.find(package.Nodes[j].Format.get());

					if (index == reportedFormats.end())
					{
						reportedFormats.insert(package.Nodes[j].Format.get());

						std::shared_ptr<Graphics::MeshFormat> format = package.Nodes[j].Format;
						const std::vector<Graphics::VertexAttributeFormat>& attributes = format->GetAttributes();
						size_t bindings = format->GetBindingCount();

						std::cout << "found new mesh format in loaded package; " << bindings << " vertex buffer bindings with " << attributes.size() << " attributes" << std::endl;

						for (size_t binding = 0; binding < bindings; ++binding)
						{
							std::cout << "\tbinding " << binding << "; " << format->GetVertexSize(binding) << " bytes" << std::endl;

							for (size_t i = 0; i < attributes.size(); ++i)
							{
								if (attributes[i].Binding == binding)
								{
									std::cout << "\t\t[" << attributes[i].Offset << "] " << Graphics::VertexAttributeFormat::DataTypeNames[attributes[i].Type] << "[" << attributes[i].ElementCount << "] " << attributes[i].Name << std::endl;
								}
							}
						}
					}

					std::shared_ptr<Graphics::Model> model = Engine::Create<Graphics::Model>();
					model->MeshAsset = asset;
					model->SetParent(transform);

					asset->SetParent(model);

					if (initVulkan)
						scene->AddObject(model);
				}

				hairTransforms[j] = transform;
			}

			for (size_t j = 0; j < hairTransforms.size(); ++j)
			{
				if (package.Nodes[j].AttachedTo == (size_t)-1)
					hairTransforms[j]->SetParent(modelTransform);
				else
					hairTransforms[j]->SetParent(hairTransforms[package.Nodes[j].AttachedTo]);

				hairTransforms[j]->Update(0);
			}

			if (package.Materials.size() > 0)
			{
				for (size_t j = 0; j < package.Materials.size(); ++j)
				{
					const Engine::Graphics::ModelPackageMaterial& material = package.Materials[j];

					std::cout << "found new material in loaded package: '" << material.Name << std::endl;
					std::cout << "\tdiffuse: " << material.Diffuse << std::endl;
					std::cout << "\tnormal: " << material.Normal << std::endl;
					std::cout << "\tspecular:" << material.Specular << std::endl;
					std::cout << "\tdiffuse color:" << material.DiffuseColor << std::endl;
					std::cout << "\tspecular color:" << material.SpecularColor << std::endl;
					std::cout << "\tambient color:" << material.AmbientColor << std::endl;
					std::cout << "\temissive color:" << material.EmissiveColor << std::endl;
					std::cout << "\tshininess exponent:" << material.Shininess << std::endl;
					std::cout << "\talpha:" << material.Alpha << std::endl;
				}
			}
		}
		else
		{
			hairs[i].models.resize(meshes.size());

			for (size_t j = 0; j < meshes.size(); ++j)
			{
				hairs[i].models[j] = Engine::Create<Graphics::Model>();

				if (initVulkan)
					scene->AddObject(hairs[i].models[j]);

				hairs[i].models[j]->SetParent(hairs[i].transform);
				hairs[i].models[j]->MeshAsset = meshes[j];

				if (meshTransforms[j] != nullptr)
				{
					std::shared_ptr<Transform> meshTransform = Engine::Create<Transform>();
					meshTransform->SetParent(hairs[i].transform);
					meshTransform->SetTransformation(meshTransforms[j]->GetTransformation());

					hairs[i].models[j]->SetParent(meshTransform);
				}
			}

			transforms.push_back(hairs[i].transform);

			hairs[i].transform->SetTransformation(transformation);
		}

	}

	//hairMeshAsset->SetPath("models/10200238_f_freeconcept020_a.nif", Enum::AssetType::GameAsset, std::ios::binary);
	//hairMeshAsset->Load();

	if (initVulkan && loadTestModels)
	{
		meshAsset->SetPath("models/vagner.obj");
		meshAsset->Load();

		meshAsset2->SetPath("models/sand_hawk.obj");
		meshAsset2->Load();
	}
}

int main(int argc, char** argv)
{
	init(argc, argv);

	if (!initVulkan)
	{
		return 0;
	}

	name = "Vulkan Cube";

	window = Engine::Create<Graphics::GraphicsWindow>();
	window->AttachToContext(context);
	window->InitializeWindow(context);

	testimage = Engine::Create<Graphics::ImageResource>();
	testimage->Load("ayayaka.png", vk::Format::eR8G8B8Unorm);

	window->InitializeRenderState();

	swapChain = window->ViewSwapChain;
	swapChain->AddRenderer(renderer);

	//if (loadTestModels)
		for (uint32_t i = 0; i < textures.size(); i++)
			textures[i]->LoadResource(testimage, window);

	phongForward = Engine::Create<Graphics::PhongForwardPipeline>();
	phongForward->AttachToContext(context);
	phongForward->ColorFormat = window->GetSurfaceFormat();
	phongForward->Configure();
	sampler = dynamic_cast<Graphics::CombinedSamplerUniform*>(phongForward->GetUniform(0).get());
	sceneInfo = dynamic_cast<Graphics::BufferObjectUniform*>(phongForward->GetUniform(1).get());

	program->SetPipeline(phongForward);

	swapChain->PrepareRenderers();
	
	MSG msg;
	bool done = false;

	draw();

	while (!done)
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_PAINT)
			draw();

		RedrawWindow(window->GetWindowHandle(), nullptr, nullptr, RDW_INTERNALPAINT);
	}

	return (int)msg.wParam;
}
