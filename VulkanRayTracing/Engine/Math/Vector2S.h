#pragma once

#include "Vector2S-decl.h"

import <iostream>;
import <sstream>;


template <typename Number, typename DistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType() : X(0), Y(0) {}

template <typename Number, typename DistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType(Number x, Number y) : X(x), Y(y) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType(const Vector2Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType(const Vector2SType<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType(const Vector3Type<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y) {}

template <typename Number, typename DistanceType>
template <typename OtherNumber, typename OtherDistanceType>
constexpr Vector2SType<Number, DistanceType>::Vector2SType(const Vector3SType<OtherNumber, OtherDistanceType>& other) : X((Number)other.X), Y((Number)other.Y) {}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::Set(Number x, Number y)
{
	X = x; Y = y;
	return *this;
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::Normalize()
{
	return *this *= (Number)(1 / Length());
}

template <typename Number, typename DistanceType>
DistanceType Vector2SType<Number, DistanceType>::Dot(const Vector2SType& other) const
{
	return (DistanceType)(X * other.X + Y * other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::Unit() const
{
	return (Number)(1 / Length()) * *this;
}

template <typename Number, typename DistanceType>
DistanceType Vector2SType<Number, DistanceType>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename Number, typename DistanceType>
DistanceType Vector2SType<Number, DistanceType>::SquareLength() const
{
	return Dot(*this);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::InvertLength()
{
	return *this *= (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::InvertedLength() const
{
	return *this * (Number)(1 / SquareLength());
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::Scale(const Vector2SType& other)
{
	return Scale(other.X, other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::Scale(Number x, Number y)
{
	return Set(X * x, Y * y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::Negate() const { return -*this; }

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::operator-() const
{
	return Vector2SType(-X, -Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::operator+(const Vector2SType& other) const
{
	return Vector2SType(X + other.X, Y + other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::operator-(const Vector2SType& other) const
{
	return Vector2SType(X - other.X, Y - other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> Vector2SType<Number, DistanceType>::operator*(Number scalar) const
{
	return Vector2SType(X * scalar, Y * scalar);
}

template <typename Number, typename DistanceType>
DistanceType Vector2SType<Number, DistanceType>::operator*(const Vector2SType& other) const
{
	return Dot(other);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::operator=(const Vector2SType& other)
{
	return Set(other.X, other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::operator+=(const Vector2SType& other)
{
	return Set(X + other.X, Y + other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::operator-=(const Vector2SType& other)
{
	return Set(X - other.X, Y - other.Y);
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>& Vector2SType<Number, DistanceType>::operator*=(Number scalar)
{
	return Set(X * scalar, Y * scalar);
}

template <typename Number, typename DistanceType>
bool Vector2SType<Number, DistanceType>::operator==(const Vector2SType& other) const
{
	return Compare(X, other.X) && Compare(Y, other.Y);
}

template <typename Number, typename DistanceType>
bool Vector2SType<Number, DistanceType>::operator!=(const Vector2SType& other) const
{
	return !(*this == other);
}

template <typename Number, typename DistanceType>
Number Vector2SType<Number, DistanceType>::operator[](int i) const
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
Number& Vector2SType<Number, DistanceType>::operator[](int i)
{
	return (&X)[i];
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> operator*(Number scalar, const Vector2SType<Number, DistanceType>& vector)
{
	return vector * scalar;
}

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector2SType<Number, DistanceType>& vector)
{
	return out << "< " << vector.X << ", " << vector.Y << " >";
}

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>::operator std::string() const
{
	std::stringstream out;
	out << *this;

	return out.str();
}

template <typename Number, typename DistanceType>
bool Vector2SType<Number, DistanceType>::Compare(Number x, Number y, Number epsilon) const
{
	return std::abs(x - y) < epsilon;
}