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
		class Material;

		class Model : public SceneObject
		{
		public:
			std::shared_ptr<MeshAsset> MeshAsset;
			std::shared_ptr<Material> Material;

			virtual void Draw(RenderQueue& queue, const Camera* camera) const;

		private:

		};
	}
}