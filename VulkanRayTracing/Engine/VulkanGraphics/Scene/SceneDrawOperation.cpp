#include "SceneDrawOperation.h"

#include "Scene.h"
#include "SceneObject.h"

namespace Engine
{
	namespace Graphics
	{
		void SceneDrawOperation::Draw(RenderQueue& queue)
		{
			if (TargetScene != nullptr && ViewCamera != nullptr)
			{
				TargetScene->CastFrustum(
					[&queue, this](const std::shared_ptr<SceneObject>& object)
					{
						object->Draw(queue, ViewCamera.get());
					}
				);
			}
		}
	}
}