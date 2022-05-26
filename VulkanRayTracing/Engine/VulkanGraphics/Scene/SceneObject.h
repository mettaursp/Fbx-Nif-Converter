#pragma once

#include <Engine/Objects/Object.h>
#include <Engine/Math/Matrix4.h>

namespace Engine
{
	namespace Graphics
	{
		class MeshAsset;
		class RenderQueue;
		class Camera;

		struct SceneObjectPushConstants
		{
			Matrix4F ModelViewProjection;
			Matrix4F WorldTransform;
		};

		class SceneObject : public Object
		{
		public:

			virtual void Draw(RenderQueue& queue, const Camera* camera) const {}

		private:

		};
	}
}