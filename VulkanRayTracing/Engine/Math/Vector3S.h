#pragma once

#include "Vector3S-decl.h"

import <iostream>;
import <sstream>;

template <typename Number, typename DistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType() : X(0), Y(0), Z(0) {}

template <typename Number, typename DistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType(Number x, Number y, Number z) : X(x), Y(y), Z(z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType(const Vector2Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z) : X((Number)other.X), Y((Number)other.Y), Z(z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType(const Vector3Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3SType<Number, DistanceType>::Vector3SType(const Vector3SType<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::Set(Number x, Number y, Number z)
{
	X = x; Y = y; Z = z;
	return *this;
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::Normalize()
{
	return *this *= (Number)(1 / Length());
}

template <typename Number, typename DistanceType>
DistanceType Vector3SType<Number, DistanceType>::Dot(const Vector3SType& other) const
{
	return (DistanceType)(X * other.X + Y * other.Y + Z * other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::Cross(const Vector3SType& other) const
{
	return Vector3SType(
		Y * other.Z - Z * other.Y,
		Z * other.X - X * other.Z,
		X * other.Y - Y * other.X
	);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::Unit() const
{
	return (Number)(1 / Length()) * *this;
}

template <typename Number, typename DistanceType>
DistanceType Vector3SType<Number, DistanceType>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename Number, typename DistanceType>
DistanceType Vector3SType<Number, DistanceType>::SquareLength() const
{
	return Dot(*this);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::Scale(const Vector3SType& other)
{
	return Scale(other.X, other.Y, other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::Scale(Number x, Number y, Number z)
{
	return Set(X * x, Y * y, Z * z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::InvertLength()
{
	return *this *= (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::InvertedLength() const
{
	return *this * (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::Negate() const { return -*this; }

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::operator-() const
{
	return Vector3SType(-X, -Y, -Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::operator+(const Vector3SType& other) const
{
	return Vector3SType(X + other.X, Y + other.Y, Z + other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::operator-(const Vector3SType& other) const
{
	return Vector3SType(X - other.X, Y - other.Y, Z - other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> Vector3SType<Number, DistanceType>::operator*(Number scalar) const
{
	return Vector3SType(X * scalar, Y * scalar, Z * scalar);
}

template <typename Number, typename DistanceType>
DistanceType Vector3SType<Number, DistanceType>::operator*(const Vector3SType& other) const
{
	return Dot(other);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::operator=(const Vector3SType& other)
{
	return Set(other.X, other.Y, other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::operator+=(const Vector3SType& other)
{
	return Set(X + other.X, Y + other.Y, Z + other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::operator-=(const Vector3SType& other)
{
	return Set(X - other.X, Y - other.Y, Z - other.Z);
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>& Vector3SType<Number, DistanceType>::operator*=(Number scalar)
{
	return Set(X * scalar, Y * scalar, Z * scalar);
}

template <typename Number, typename DistanceType>
bool Vector3SType<Number, DistanceType>::operator==(const Vector3SType& other) const
{
	return Compare(X, other.X) && Compare(Y, other.Y) && Compare(Z, other.Z);
}

template <typename Number, typename DistanceType>
bool Vector3SType<Number, DistanceType>::operator!=(const Vector3SType& other) const
{
	return !(*this == other);
}

template <typename Number, typename DistanceType>
Number Vector3SType<Number, DistanceType>::operator[](int i) const
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
Number& Vector3SType<Number, DistanceType>::operator[](int i)
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
bool Vector3SType<Number, DistanceType>::Compare(Number x, Number y, Number epsilon) const
{
	return std::abs(x - y) < epsilon;
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> operator*(Number scalar, const Vector3SType<Number, DistanceType>& vector)
{
	return vector * scalar;
}

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector3SType<Number, DistanceType>& vector)
{
	return out << "< " << vector.X << ", " << vector.Y << ", " << vector.Z << " >";
}

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>::operator std::string() const
{
	std::stringstream out;
	out << *this;

	return out.str();
}