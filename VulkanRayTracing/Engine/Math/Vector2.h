#pragma once

import <sstream>;

#include "Vector2-decl.h"

template <typename Number, typename DistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type() : X(0), Y(0), Z(0) {}

template <typename Number, typename DistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type(Number x, Number y, Number z) : X(x), Y(y), Z(z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type(const Vector2Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z) : X((Number)other.X), Y((Number)other.Y), Z(z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type(const Vector3Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2Type<Number, DistanceType>::Vector2Type(const Vector3SType<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y), Z((Number)other.Z) {}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::Set(Number x, Number y, Number z)
{
	X = x; Y = y; Z = z;
	return *this;
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::Normalize()
{
	return *this *= (Number)(1 / Length());
}

template <typename Number, typename DistanceType>
DistanceType Vector2Type<Number, DistanceType>::Dot(const Vector2Type& other) const
{
	return (DistanceType)(X * other.X + Y * other.Y);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::Unit() const
{
	return (Number)(1 / Length()) * *this;
}

template <typename Number, typename DistanceType>
DistanceType Vector2Type<Number, DistanceType>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename Number, typename DistanceType>
DistanceType Vector2Type<Number, DistanceType>::SquareLength() const
{
	return Dot(*this);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::InvertLength()
{
	return *this *= (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::InvertedLength() const
{
	return *this * (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::Scale(const Vector2Type& other)
{
	return Scale(other.X, other.Y, other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::Scale(Number x, Number y, Number z)
{
	return Set(X * x, Y * y, Z * z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::Negate() const { return -*this; }

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::operator-() const
{
	return Vector2Type<Number, DistanceType>(-X, -Y, -Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::operator+(const Vector2Type& other) const
{
	return Vector2Type<Number, DistanceType>(X + other.X, Y + other.Y, Z + other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::operator-(const Vector2Type& other) const
{
	return Vector2Type<Number, DistanceType>(X - other.X, Y - other.Y, Z - other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> Vector2Type<Number, DistanceType>::operator*(Number scalar) const
{
	return Vector2Type<Number, DistanceType>(X * scalar, Y * scalar, Z * scalar);
}

template <typename Number, typename DistanceType>
DistanceType Vector2Type<Number, DistanceType>::operator*(const Vector2Type& other) const
{
	return Dot(other);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::operator=(const Vector2Type& other)
{
	return Set(other.X, other.Y, other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::operator+=(const Vector2Type& other)
{
	return Set(X + other.X, Y + other.Y, Z + other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::operator-=(const Vector2Type& other)
{
	return Set(X - other.X, Y - other.Y, Z - other.Z);
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>& Vector2Type<Number, DistanceType>::operator*=(Number scalar)
{
	return Set(X * scalar, Y * scalar, Z * scalar);
}

template <typename Number, typename DistanceType>
bool Vector2Type<Number, DistanceType>::operator==(const Vector2Type& other) const
{
	return Compare(X, other.X) && Compare(Y, other.Y) && Compare(Z, other.Z);
}

template <typename Number, typename DistanceType>
bool Vector2Type<Number, DistanceType>::operator!=(const Vector2Type& other) const
{
	return !(*this == other);
}

template <typename Number, typename DistanceType>
Number Vector2Type<Number, DistanceType>::operator[](int i) const
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
Number& Vector2Type<Number, DistanceType>::operator[](int i)
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
bool Vector2Type<Number, DistanceType>::Compare(Number x, Number y, Number epsilon) const
{
	return std::abs(x - y) <= epsilon;
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> operator*(Number scalar, const Vector2Type<Number, DistanceType>& vector)
{
	return vector * scalar;
}

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector2Type<Number, DistanceType>& vector)
{
	return out << "< " << vector.X << ", " << vector.Y << ", " << vector.Z << " >";
}

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>::operator std::string() const
{
	std::stringstream out;
	out << *this;

	return out.str();
}
