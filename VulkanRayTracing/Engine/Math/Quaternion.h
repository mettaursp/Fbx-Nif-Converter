#pragma once

#include "Matrix4.h"

extern "C" {
import <cmath>;
}

class Quaternion
{
public:
	Float X, Y, Z, W;

	constexpr Quaternion(Float w = 0, Float x = 0, Float y = 0, Float z = 0) : X(x), Y(y), Z(z), W(w) {}
	constexpr Quaternion(const Quaternion& quat) : X(quat.X), Y(quat.Y), Z(quat.Z), W(quat.W) {}
	constexpr Quaternion(const Vector3& vec) : X(vec.X), Y(vec.Y), Z(vec.Z), W(vec.W) {}
	Quaternion(const Vector3& axis, Float angle);
	constexpr Quaternion(Float w, const Vector3& vector) : X(vector.X), Y(vector.Y), Z(vector.Z), W(w) {}
	Quaternion(const Matrix4& matrix);

	Quaternion& Invert();
	Quaternion Inverse() const { return Quaternion(*this).Invert(); }
	Quaternion Slerp(const Quaternion& destination, Float t) const;

	Quaternion operator*(const Quaternion& rhs) const;

	Matrix4 Matrix() const;

	Quaternion& Normalize();
	Quaternion Cross(const Quaternion& other) const;
	Quaternion Unit() const;
	Float Length() const;
	Float SquareLength() const;
	Float Dot(const Quaternion& other) const;

	Quaternion operator-() const;
	Quaternion operator+(const Quaternion& other) const;
	Quaternion operator-(const Quaternion& other) const;
	Quaternion operator*(Float scalar) const;
	Quaternion& operator=(const Quaternion& other);
	Quaternion& operator+=(const Quaternion& other);
	Quaternion& operator-=(const Quaternion& other);
	Quaternion& operator*=(Float scalar);
	bool operator==(const Quaternion& other) const;
	bool operator!=(const Quaternion& other) const;
	Float operator[](int i) const;
	Float& operator[](int i);
	operator Vector3() const;
	operator std::string() const;

private:
	bool Compare(Float x, Float y, Float epsilon) const;
};

Quaternion operator*(Float scalar, const Quaternion& quaternion);
std::ostream& operator<<(std::ostream& out, const Quaternion& quaternion);
