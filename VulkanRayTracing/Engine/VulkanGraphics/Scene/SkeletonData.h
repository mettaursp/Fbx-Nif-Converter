#pragma once

#include "SkeletonData-decl.h"

namespace Engine
{
	namespace Graphics
	{
		template <typename Matrix>
		SkeletonData<Matrix>::SkeletonData(const std::vector<Matrix>& bones)
		{
			BaseBoneTransforms = bones;
			BoneNames.resize(bones.size());
			BoneParentNames.resize(bones.size());
		}

		template <typename Matrix>
		void SkeletonData<Matrix>::SetBoneName(int index, const std::string& name, const std::string& parent)
		{
			if (index < 0 || index >= (int)BaseBoneTransforms.size())
				return;

			if (BoneNames[index] != "")
			{
				BoneIndices.erase(BoneNames[index]);
			}

			BoneNames[index] = name;
			BoneParentNames[index] = parent;
			BoneIndices[name] = index;
		}

		template <typename Matrix>
		const std::vector<Matrix>& SkeletonData<Matrix>::GetBaseBoneTransforms() const
		{
			return BaseBoneTransforms;
		}

		template <typename Matrix>
		const std::vector<std::string>& SkeletonData<Matrix>::GetBoneNames() const
		{
			return BoneNames;
		}

		template <typename Matrix>
		const std::vector<std::string>& SkeletonData<Matrix>::GetBoneParentNames() const
		{
			return BoneParentNames;
		}

		template <typename Matrix>
		int SkeletonData<Matrix>::GetBoneIndex(const std::string& name) const
		{
			auto mapping = BoneNames.find(name);

			if (mapping != BoneNames.end())
			{
				mapping->second = index;

				return mapping->second;
			}

			return -1;
		}

		template <typename Matrix>
		const std::string& SkeletonData<Matrix>::GetBoneParent() const
		{

		}
	}
}