#pragma once

#include <Engine/VulkanGraphics/Scene/SkeletonData.h>
#include <Engine/Objects/Transform.h>

namespace Engine
{
	namespace Graphics
	{
		template <typename Matrix>
		class Skeleton : public Object
		{
		public:
			void SetSkeletonData(const std::shared_ptr<SkeletonData<Matrix>>& data);
			void SetBone(const std::shared_ptr<Transform>& bone, const std::string& name = "");
			void SetRig(const std::shared_ptr<Transform>& root);
			void SetRoot(const std::shared_ptr<Transform>& root);

			void UpdateBones();

			const std::vector<Matrix>& GetBoneTransforms() const;
			std::shared_ptr<Transform> GetBone(const std::string& name) const;
			std::shared_ptr<Transform> GetBone(int index) const;

		private:
			std::weak_ptr<SkeletonData<Matrix>> Data;
			std::weak_ptr<Transform> Root;
			std::vector<std::weak_ptr<Transform>> BoneNodes;
			std::vector<Matrix> BoneTransforms;
		};

		typedef Skeleton<Matrix3> Skeleton2D;
		typedef Skeleton<Matrix4> Skeleton3D;
	}
}