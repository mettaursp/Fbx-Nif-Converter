#pragma once

#include "Vector3-decl.h"

import <iostream>;
import <sstream>;

template <typename Number, typename DistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type() : X(0), Y(0), Z(0), W(0) {}

template <typename Number, typename DistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type(Number x, Number y, Number z, Number w) : X(x), Y(y), Z(z), W(w) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type(const Vector2Type<OtherNumber, OtherDistanceType>& other, Number w) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z), W(w) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z, Number w) : X((Number)other.X), Y((Number)other.Y), Z(z), W(w) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type(const Vector3Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z), W((Number)other.W) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector3Type<Number, DistanceType>::Vector3Type(const Vector3SType<OtherNumber, OtherDistanceType>& other, Number w) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z), W(w) {}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::Set(Number x, Number y, Number z, Number w)
{
	X = x; Y = y; Z = z; W = w;
	return *this;
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::Normalize()
{
	return *this *= (Number)(1 / Length());
}

template <typename Number, typename DistanceType>
DistanceType Vector3Type<Number, DistanceType>::Dot(const Vector3Type& other) const
{
	return (DistanceType)(X * other.X + Y * other.Y + Z * other.Z);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::Cross(const Vector3Type& other) const
{
	return Vector3Type(
		Y * other.Z - Z * other.Y,
		Z * other.X - X * other.Z,
		X * other.Y - Y * other.X
	);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::Unit() const
{
	return (Number)(1 / Length()) * *this;
}

template <typename Number, typename DistanceType>
DistanceType Vector3Type<Number, DistanceType>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename Number, typename DistanceType>
DistanceType Vector3Type<Number, DistanceType>::SquareLength() const
{
	return Dot(*this);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::Scale(const Vector3Type& other)
{
	return Scale(other.X, other.Y, other.Z, other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::Scale(Number x, Number y, Number z, Number w)
{
	return Set(X * x, Y * y, Z * z, W * w);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::InvertLength()
{
	return *this *= (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::InvertedLength() const
{
	return *this * (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::Negate() const { return -*this; }

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::operator-() const
{
	return Vector3Type(-X, -Y, -Z, -W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::operator+(const Vector3Type& other) const
{
	return Vector3Type(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::operator-(const Vector3Type& other) const
{
	return Vector3Type(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> Vector3Type<Number, DistanceType>::operator*(Number scalar) const
{
	return Vector3Type(X * scalar, Y * scalar, Z * scalar, W * scalar);
}

template <typename Number, typename DistanceType>
DistanceType Vector3Type<Number, DistanceType>::operator*(const Vector3Type& other) const
{
	return Dot(other);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::operator=(const Vector3Type& other)
{
	return Set(other.X, other.Y, other.Z, other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::operator+=(const Vector3Type& other)
{
	return Set(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::operator-=(const Vector3Type& other)
{
	return Set(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>& Vector3Type<Number, DistanceType>::operator*=(Number scalar)
{
	return Set(X * scalar, Y * scalar, Z * scalar, W * scalar);
}

template <typename Number, typename DistanceType>
bool Vector3Type<Number, DistanceType>::operator==(const Vector3Type& other) const
{
	return Compare(X, other.X) && Compare(Y, other.Y) && Compare(Z, other.Z) && Compare(W, other.W);
}

template <typename Number, typename DistanceType>
bool Vector3Type<Number, DistanceType>::operator!=(const Vector3Type& other) const
{
	return !(*this == other);
}

template <typename Number, typename DistanceType>
Number Vector3Type<Number, DistanceType>::operator[](int i) const
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
Number& Vector3Type<Number, DistanceType>::operator[](int i)
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
bool Vector3Type<Number, DistanceType>::Compare(Number x, Number y, Number epsilon) const
{
	return std::abs(x - y) < epsilon;
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> operator*(Number scalar, const Vector3Type<Number, DistanceType>& vector)
{
	return vector * scalar;
}

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector3Type<Number, DistanceType>& vector)
{
	return out << "< " << vector.X << ", " << vector.Y << ", " << vector.Z << ", " << vector.W << " >";
}

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>::operator std::string() const
{
	std::stringstream out;
	out << *this;

	return out.str();
}