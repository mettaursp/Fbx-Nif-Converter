#pragma once

#include <Engine/VulkanGraphics/Core/DrawOperation.h>

namespace Engine
{
	namespace Graphics
	{
		class Scene;
		class Camera;

		class SceneDrawOperation : public DrawOperation
		{
		public:
			std::shared_ptr<Scene> TargetScene;
			std::shared_ptr<Camera> ViewCamera;

			virtual void Draw(RenderQueue& queue);
		};
	}
}