#pragma once

import <vector>
import <map>

#include <Engine/Objects/Object.h>
#include <Engine/Math/Matrix3-decl.h>
#include <Engine/Math/Matrix4-decl.h>

namespace Engine
{
	namespace Graphics
	{
		template <typename Matrix>
		class SkeletonData : public Object
		{
		public:

			SkeletonData(const std::vector<Matrix>& bones);

			void SetBoneName(int index, const std::string& name, const std::string& parent = "");

			const std::vector<Matrix>& GetBaseBoneTransforms() const;
			const std::vector<std::string>& GetBoneNames() const;
			const std::vector<std::string>& GetBoneParentNames() const;
			int GetBoneIndex(const std::string& name) const;
			const std::string& GetBoneParent() const;

		private:

			std::vector<Matrix> BaseBoneTransforms;
			std::vector<std::string> BoneNames;
			std::vector<std::string> BoneParentNames;
			std::map<std::string, int> BoneIndices;
		};

		typedef SkeletonData<Matrix3> SkeletonData2D;
		typedef SkeletonData<Matrix4> SkeletonData3D;
	}
}