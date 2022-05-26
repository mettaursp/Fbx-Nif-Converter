#include "Model.h"

#include <Engine/Objects/Transform.h>
#include <Engine/VulkanGraphics/Core/RenderQueue.h>
#include "Camera.h"

namespace Engine
{
	namespace Graphics
	{
		void Model::Draw(RenderQueue& queue, const Camera* camera) const
		{
			Transform* transform = GetComponent2<Transform>();

			SceneObjectPushConstants constants
			{
				camera->GetProjection(),
				transform->GetWorldTransformation()
			};

			queue.PushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(constants), &constants);
			queue.DrawMesh(MeshAsset);
		}
	}
}