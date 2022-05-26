#pragma once

import <ostream>;

namespace Engine
{
	namespace Graphics
	{
		struct ModelPackage;
	}
}

class NifWriter
{
public:
	Engine::Graphics::ModelPackage* Package = nullptr;

	void Write(std::ostream& stream);
};