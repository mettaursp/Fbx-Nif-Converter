#pragma once

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class RenderQueue;

		class DrawOperation : public Object
		{
		public:
			virtual void Draw(RenderQueue& queue) {}
		private:
		};
	}
}