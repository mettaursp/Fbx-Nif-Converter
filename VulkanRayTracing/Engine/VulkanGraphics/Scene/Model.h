#pragma once

#include <Engine/Objects/Object.h>
#include "SceneObject.h"

namespace Engine
{
	namespace Graphics
	{
		class MeshAsset;
		class RenderQueue;
		class Camera;

		class Model : public SceneObject
		{
		public:
			std::shared_ptr<MeshAsset> MeshAsset;

			virtual void Draw(RenderQueue& queue, const Camera* camera) const;

		private:

		};
	}
}