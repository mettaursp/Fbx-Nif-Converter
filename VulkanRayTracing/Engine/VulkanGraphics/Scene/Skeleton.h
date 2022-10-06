#pragma once

#include "Skeleton-decl.h"

namespace Engine
{
	namespace Graphics
	{
		template <typename Matrix>
		void Skeleton<Matrix>::SetSkeletonData(const std::shared_ptr<SkeletonData<Matrix>>& data)
		{
			if (data == nullptr)
			{
				BoneNodes.resize(0);
				BoneTransforms.resize(0);

				return;
			}

			const std::vector<Matrix>& baseTransforms = data->GetBaseBoneTransforms();

			Data = data;
			BoneNodes.resize(baseTransforms.size());
			BoneTransforms.resize(baseTransforms.size());

			for (size_t i = 0; i < BoneNodes.size(); ++i)
			{
				BoneNodes[i] = nullptr;
				BoneTransforms[i] = Matrix();
			}
		}

		template <typename Matrix>
		void Skeleton<Matrix>::SetBone(const std::shared_ptr<Transform>& bone, const std::string& name)
		{
			if (Data.expired())
				return;

			int index = Data.lock()->GetBoneIndex(name);

			if (index < 0 || index >= BoneNodes.size())
				return;

			BoneNodes[index] = bone;
		}

		template <typename Matrix>
		void Skeleton<Matrix>::SetRig(const std::shared_ptr<Transform>& root)
		{
			std::shared_ptr<SkeletonData<Matrix>> data = Data.lock();

			if (data == nullptr)
				return;

			const std::vector<std::string>& boneNames = data->GetBoneNames();
			const std::vector<std::string>& boneParentNames = data->GetBoneParentNames();

			for (size_t i = 0; i < boneParentNames.size(); ++i)
			{
				if (boneParentNames[i] == "Root")
				{
					BoneNodes[i] = root->GetByName<Transform>(boneNames[i]);

					continue;
				}

				std::shared_ptr<Transform> parent = GetBone(boneParentNames[i]);

				if (parent != nullptr)
					BoneNodes[i] = parent->GetByName<Transform>(boneNames[i]);
			}
		}

		template <typename Matrix>
		void Skeleton<Matrix>::SetRoot(const std::shared_ptr<Transform>& root)
		{
			Root = root;
		}

		template <typename Matrix>
		void Skeleton<Matrix>::UpdateBones()
		{
			std::shared_ptr<SkeletonData<Matrix>> data = Data.lock();

			if (data == nullptr)
				return;

			const std::shared_ptr<Transform>& root = Root.lock();
			Matrix rootInverse;

			if (root != nullptr)
				rootInverse = root->GetWorldTransformationInverse();

			const std::vector<Matrix>& baseTransforms = data->GetBaseBoneTransforms();

			for (size_t i = 0; i < BoneNodes.size(); ++i)
			{
				std::shared_ptr<Transform> node = BoneNodes[i].lock();

				if (node == nullptr)
				{
					BoneTransforms[i] = baseTransforms[i];

					continue;
				}

				BoneTransforms[i] = baseTransforms[i] * rootInverse * node->GetWorldTransformation();
			}
		}

		template <typename Matrix>
		const std::vector<Matrix>& Skeleton<Matrix>::GetBoneTransforms() const
		{
			return BoneTransforms;
		}

		template <typename Matrix>
		std::shared_ptr<Transform> Skeleton<Matrix>::GetBone(const std::string& name) const
		{
			if (Data.expired())
				return nullptr;

			return GetBone(Data.lock()->GetBoneIndex(name));
		}

		template <typename Matrix>
		std::shared_ptr<Transform> Skeleton<Matrix>::GetBone(int index) const
		{
			if (index < 0 || index >= BoneNodes.size())
				return nullptr;

			return BoneNodes[index].lock();
		}
	}
}