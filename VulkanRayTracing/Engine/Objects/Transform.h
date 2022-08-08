#pragma once

#include <Engine/Objects/Object.h>
#include <Engine/Math/Vector3.h>
#include <Engine/Math/Matrix4.h>
#include <Engine/Math/Quaternion.h>

namespace Engine
{
	class Transform : public Object
	{
	public:
		virtual ~Transform() {}

		void Update(Float delta);

		bool HasMoved() const;
		bool HasMoved();

		void SetStatic(bool isStatic);
		bool IsTransformStatic() const;

		void SetTransformation(const Matrix4& matrix);
		const Matrix4& GetTransformation();

		void SetInheritsTransformation(bool inherits);
		bool InheritsTransformation() const;

		Vector3 GetPosition() const;
		Vector3 GetPosition();
		void SetPosition(const Vector3& position);
		void Move(const Vector3& offset);

		Vector3 GetWorldPosition() const;
		Vector3 GetWorldPosition();
		const Matrix4& GetWorldTransformation() const;
		const Matrix4& GetWorldTransformation();
		const Matrix4& GetWorldTransformationInverse() const;
		const Matrix4& GetWorldTransformationInverse();
		const Matrix4& GetWorldRotation() const;
		const Matrix4& GetWorldRotation();
		Quaternion GetWorldOrientation() const;
		Quaternion GetWorldOrientation();
		const Matrix4& GetWorldNormalTransformation() const;
		const Matrix4& GetWorldNormalTransformation() ;

		Quaternion GetOrientation() const;
		Quaternion GetOrientation();
		void SetOrientation(const Quaternion& orientation);
		void Rotate(const Quaternion& rotation);
		void Rotate(const Vector3& axis, float angle);

		Vector3 GetEulerAngles() const;
		Vector3 GetEulerAngles();
		void SetEulerAngles(const Vector3& angles);
		void SetEulerAngles(float pitch, float roll, float yaw);
		void Rotate(const Vector3& angles);
		void Rotate(float pitch, float roll, float yaw);

		Vector3 GetEulerAnglesYaw() const;
		Vector3 GetEulerAnglesYaw();
		void SetEulerAnglesYaw(const Vector3& angles);
		void SetEulerAnglesYaw(float yaw, float pitch, float roll);
		void RotateYaw(const Vector3& angles);
		void RotateYaw(float yaw, float pitch, float roll);

		Vector3 GetScale() const;
		Vector3 GetScale();
		void SetScale(const Vector3& scale);
		void Rescale(const Vector3& scale);

		void TransformBy(const Matrix4& transformation);
		void TransformBy(const Quaternion& transformation, const Vector3& point = Vector3());
		void TransformByRelative(const Matrix4& transformation);
		void TransformByRelative(const Quaternion& transformation, const Vector3& point = Vector3());

		//Event<Transform*> TransformMoved;

	private:
		Matrix4 Transformation;
		Matrix4 WorldTransformation;
		Matrix4 WorldTransformationInverse;
		Matrix4 WorldRotation;
		Matrix4 WorldNormalTransformation;

		bool HasChanged();
		void Recompute(bool doUpdate = false);

		bool IsStaticTransformation = true;
		bool InheritTransformation = true;
		bool Moved = false;
		bool HadParent = false;
		Matrix4 OldParentTransform;
	};
}
