#include "Scene.h"

namespace Engine
{
	namespace Graphics
	{
		void Scene::CastFrustum(const DrawCallback& callback) const
		{
			for (size_t i = 0; i < ObjectVector.size(); ++i)
				callback(ObjectVector[i]);
		}

		void Scene::AddObject(const std::shared_ptr<SceneObject>& object)
		{
			ObjectVector.push_back(object);
		}

		void Scene::RemoveObject(const std::shared_ptr<SceneObject>& object)
		{
			for (size_t i = 0; i < ObjectVector.size(); ++i)
			{
				if (ObjectVector[i] == object)
				{
					std::swap(ObjectVector[i], ObjectVector.back());
					ObjectVector.pop_back();

					return;
				}
			}
		}
	}
}