#pragma once

import <functional>;
import <memory>;
import <vector>;

#include <Engine/Objects/Object.h>

namespace Engine
{
	namespace Graphics
	{
		class SceneObject;

		class Scene : public Object
		{
		public:
			typedef std::function<void(const std::shared_ptr<SceneObject>& object)> DrawCallback;

			void CastFrustum(const DrawCallback& callback) const;
			void AddObject(const std::shared_ptr<SceneObject>& object);
			void RemoveObject(const std::shared_ptr<SceneObject>& object);

		private:
			std::vector<std::shared_ptr<SceneObject>> ObjectVector;
		};
	}
}