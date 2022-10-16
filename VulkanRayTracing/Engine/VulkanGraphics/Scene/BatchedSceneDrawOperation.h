#pragma once

import <unordered_map>;

#include <Engine/VulkanGraphics/Core/DrawOperation.h>

namespace Engine
{
	namespace Graphics
	{
		class Scene;
		class Camera;
		class Material;
		class SceneObject;

		class BatchedSceneDrawOperation : public DrawOperation
		{
		public:
			std::shared_ptr<Scene> TargetScene;
			std::shared_ptr<Camera> ViewCamera;

			virtual void Draw(RenderQueue& queue);
		private:
			typedef std::vector<SceneObject*> SceneObjectVector;
			typedef std::unordered_map<Material*, SceneObjectVector> MaterialQueueMap;
			typedef std::vector<std::pair<Material*, SceneObjectVector*>> QueueVector;

			MaterialQueueMap DrawQueue;
			QueueVector ActiveQueues;
		};
	}
}