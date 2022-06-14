#include "Transform.h"

namespace Engine
{
	void Transform::Update(Float delta)
	{
		Regenerate();

		Object::Update(delta);

		//if (Moved)
		//	TransformMoved.Fire(this);

		Moved = false;

		if (IsStaticTransformation)
			SetTicks(false);
	}

	void Transform::Recompute(const Transform* inherited)
	{
		if (!Stale)
			return;

		Stale = false;

		OldParent = inherited;
		HadParent = InheritTransformation && inherited != nullptr;

		if (inherited == nullptr)
			WorldTransformation = Transformation;
		else
			WorldTransformation = inherited->GetWorldTransformation() * Transformation;

		WorldTransformationInverse.Invert(WorldTransformation);
		WorldNormalTransformation = WorldTransformation.Inverted().Transpose();
		WorldRotation = Matrix4(true).ExtractRotation(WorldTransformation);

		if (HadParent)
			OldParentVersion = inherited->Version;

		++Version;

		SetTicks(true);
	}

	void Transform::Regenerate()
	{
		if (InheritTransformation)
		{
			Transform* inherited = GetComponent2<Transform>(true);

			if (inherited != nullptr)
				inherited->Regenerate();

			Stale |= inherited != OldParent || (inherited != nullptr && inherited->Version != OldParentVersion);

			Recompute(inherited);
		}
		else if (HadParent || Stale)
		{
			Stale = true;

			Recompute(nullptr);
		}
	}

	bool Transform::HasMoved() const
	{
		if (Moved)
			return true;
		else if (InheritTransformation)
		{
			Transform* parent = GetComponent2<Transform>();

			bool hadParent = parent != nullptr;

			if (hadParent != HadParent)
				return true;
			else if (hadParent && (parent->HasMoved()))
				return true;
		}

		if (HadParent)
			return true;

		return false;
	}

	bool Transform::HasMoved()
	{
		if (Moved)
			return true;
		else if (InheritTransformation)
		{
			Transform* parent = GetComponent2<Transform>();

			bool hadParent = parent != nullptr;
			
			if (hadParent != HadParent)
				MarkStale();
			else if (hadParent && (parent->HasMoved()))
				MarkStale();

			HadParent = hadParent;

			return Moved;
		}
		
		if (HadParent)
			MarkStale();

		HadParent = false;

		return Moved;
	}

	void Transform::SetStatic(bool isStatic)
	{
		IsStaticTransformation = isStatic;

		SetTicks(isStatic);

		MarkStale();
	}

	bool Transform::IsTransformStatic() const
	{
		return IsStaticTransformation;
	}

	void Transform::SetTransformation(const Matrix4& matrix)
	{
		Transformation = matrix;

		MarkStale();
	}

	const Matrix4& Transform::GetTransformation() const
	{
		return Transformation;
	}

	void Transform::SetInheritsTransformation(bool inherits)
	{
		InheritTransformation = inherits;

		MarkStale();
	}

	bool Transform::InheritsTransformation() const
	{
		return InheritTransformation;
	}

	bool Transform::HasChanged()
	{
		if (Stale)
			return true;

		if (InheritTransformation)
		{
			Transform* parent = GetComponent2<Transform>();

			bool hasParent = parent != nullptr;
			bool changed = false;

			if (hasParent != HadParent)
				changed = true;
			else if (hasParent && parent->Moved && parent->Version != OldParentVersion)
				changed = true;

			if (changed)
			{
				MarkStale();

				return true;
			}
		}

		return false;
	}

	Vector3 Transform::GetPosition() const
	{
		return Transformation.Translation();
	}

	void Transform::SetPosition(const Vector3& position)
	{
		Transformation.SetTranslation(position);

		MarkStale();
	}

	void Transform::Move(const Vector3& offset)
	{
		Transformation.SetTranslation(Transformation.Translation() + offset);

		MarkStale();
	}

	Vector3 Transform::GetWorldPosition() const
	{
		return WorldTransformation.Translation();
	}

	Vector3 Transform::GetWorldPosition()
	{
		Regenerate();
		
		return WorldTransformation.Translation();
	}

	const Matrix4& Transform::GetWorldTransformation() const
	{
		return WorldTransformation;
	}

	const Matrix4& Transform::GetWorldTransformation()
	{
		Regenerate();
		
		return WorldTransformation;
	}

	const Matrix4& Transform::GetWorldTransformationInverse() const
	{
		return WorldTransformationInverse;
	}

	const Matrix4& Transform::GetWorldTransformationInverse()
	{
		Regenerate();
	
		return WorldTransformationInverse;
	}

	const Matrix4& Transform::GetWorldRotation() const
	{
		return WorldRotation;
	}

	const Matrix4& Transform::GetWorldRotation()
	{
		Regenerate();
	
		return WorldRotation;
	}

	Quaternion Transform::GetWorldOrientation() const
	{
		return Quaternion(WorldTransformation);
	}

	Quaternion Transform::GetWorldOrientation()
	{
		Regenerate();
	
		return Quaternion(WorldTransformation);
	}

	const Matrix4& Transform::GetWorldNormalTransformation() const
	{
		return WorldNormalTransformation;
	}

	const Matrix4& Transform::GetWorldNormalTransformation()
	{
		Regenerate();
	
		return WorldNormalTransformation;
	}

	Quaternion Transform::GetOrientation() const
	{
		return Quaternion(Transformation);
	}

	void Transform::SetOrientation(const Quaternion& orientation)
	{
		Transformation = Matrix4(orientation).SetTranslation(Transformation.Translation());

		MarkStale();
	}

	void Transform::Rotate(const Quaternion& rotation)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());
		
		Transformation = (Matrix4(rotation) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	void Transform::Rotate(const Vector3& axis, float angle)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4(true).RotateAxis(axis, angle) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	Vector3 Transform::GetEulerAngles() const
	{
		return Vector3();
	}

	void Transform::SetEulerAngles(const Vector3& angles)
	{
		Transformation = Matrix4::EulerAnglesRotation(angles.X, angles.Y, angles.Z).SetTranslation(Transformation.Translation());

		MarkStale();
	}

	void Transform::SetEulerAngles(float pitch, float roll, float yaw)
	{
		Transformation = Matrix4::EulerAnglesRotation(pitch, roll, yaw).SetTranslation(Transformation.Translation());

		MarkStale();
	}

	void Transform::Rotate(const Vector3& angles)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4::EulerAnglesRotation(angles.X, angles.Y, angles.Z) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	void Transform::Rotate(float pitch, float roll, float yaw)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4::EulerAnglesRotation(pitch, roll, yaw) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	Vector3 Transform::GetEulerAnglesYaw() const
	{
		return Vector3();
	}

	void Transform::SetEulerAnglesYaw(float yaw, float pitch, float roll)
	{
		Transformation = Matrix4::EulerAnglesYawRotation(yaw, pitch, roll).SetTranslation(Transformation.Translation());

		MarkStale();
	}

	void Transform::SetEulerAnglesYaw(const Vector3& angles)
	{
		Transformation = Matrix4::EulerAnglesYawRotation(angles.X, angles.Y, angles.Z).SetTranslation(Transformation.Translation());

		MarkStale();
	}

	void Transform::RotateYaw(const Vector3& angles)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4::EulerAnglesYawRotation(angles.X, angles.Y, angles.Z) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	void Transform::RotateYaw(float yaw, float pitch, float roll)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4::EulerAnglesYawRotation(yaw, pitch, roll) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	Vector3 Transform::GetScale() const
	{
		return Vector3(Transformation.RightVector().Length(), Transformation.UpVector().Length(), Transformation.FrontVector().Length());
	}

	void Transform::SetScale(const Vector3& scale)
	{
		Transformation.SetRight(Transformation.RightVector().Normalize() * scale.X);
		Transformation.SetUp(Transformation.UpVector().Normalize() * scale.Y);
		Transformation.SetFront(Transformation.FrontVector().Normalize() * scale.Z);

		MarkStale();
	}

	void Transform::Rescale(const Vector3& scale)
	{
		Transformation.SetRight(Transformation.RightVector() * scale.X);
		Transformation.SetUp(Transformation.UpVector() * scale.Y);
		Transformation.SetFront(Transformation.FrontVector() * scale.Z);

		MarkStale();
	}

	void Transform::TransformBy(const Matrix4& transformation)
	{
		Transformation = transformation * Transformation;

		MarkStale();
	}

	void Transform::TransformBy(const Quaternion& transformation, const Vector3& point)
	{
		Transformation = Matrix4(transformation).SetTranslation(point) * Transformation;

		MarkStale();
	}

	void Transform::TransformByRelative(const Matrix4& transformation)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (transformation * Transformation).SetTranslation(translation);

		MarkStale();
	}

	void Transform::TransformByRelative(const Quaternion& transformation, const Vector3& point)
	{
		Vector3 translation = Transformation.Translation();

		Transformation.SetTranslation(Vector3());

		Transformation = (Matrix4(transformation).SetTranslation(point) * Transformation).SetTranslation(translation);

		MarkStale();
	}

	void Transform::MarkStale()
	{
		Moved = true;
		Stale = true;
	}
}
