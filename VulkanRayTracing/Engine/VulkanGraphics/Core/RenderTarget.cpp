#include "RenderTarget.h"

namespace Engine
{
	namespace Graphics
	{
		void RenderTarget::SetFormat(const std::shared_ptr<BufferFormat>& format)
		{
			if (Format != nullptr)
				throw "format already set";

			Format = format;
		}
	}
}