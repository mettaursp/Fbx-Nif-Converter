#pragma once

#include <Engine/Objects/Object.h>
#include <Engine/Math/Matrix4.h>

namespace Engine
{
	namespace Graphics
	{
		class Camera : public Object
		{
		public:
			Matrix4F proj; // FIX THIS

			Matrix4F GetProjection() const { return proj; }
		};
	}
}