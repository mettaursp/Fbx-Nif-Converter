#pragma once

import <iostream>;

#include "../Precision.h"

template <typename Number, typename DistanceType>
class Vector2SType;

template <typename Number, typename DistanceType>
class Vector3Type;

template <typename Number, typename DistanceType>
class Vector3SType;

template <typename Number, typename DistanceType>
class Vector2Type
{
public:
	Number X, Y, Z;

	constexpr Vector2Type();
	constexpr Vector2Type(Number x, Number y = 0, Number z = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2Type(const Vector2Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2Type(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2Type(const Vector3Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2Type(const Vector3SType<OtherNumber, OtherDistanceType>& other);

	Vector2Type& Set(Number x, Number y = 0, Number z = 0);
	Vector2Type& Normalize();
	DistanceType Dot(const Vector2Type& other) const;
	Vector2Type Unit() const;
	DistanceType Length() const;
	DistanceType SquareLength() const;
	Vector2Type& InvertLength();
	Vector2Type InvertedLength() const;
	Vector2Type& Scale(const Vector2Type& other);
	Vector2Type& Scale(Number x, Number y, Number z);
	Vector2Type Negate() const;
	Vector2Type operator-() const;
	Vector2Type operator+(const Vector2Type& other) const;
	Vector2Type operator-(const Vector2Type& other) const;
	Vector2Type operator*(Number scalar) const;
	DistanceType operator*(const Vector2Type& other) const;
	Vector2Type& operator=(const Vector2Type& other);
	Vector2Type& operator+=(const Vector2Type& other);
	Vector2Type& operator-=(const Vector2Type& other);
	Vector2Type& operator*=(Number scalar);
	bool operator==(const Vector2Type& other) const;
	bool operator!=(const Vector2Type& other) const;
	Number operator[](int i) const;
	Number& operator[](int i);

	operator std::string() const;

private:
	bool Compare(Number x, Number y, Number epsilon = (Number)1e-5) const;
};

using Vector2F = Vector2Type<float, float>;
using Vector2D = Vector2Type<double, double>;
using Vector2I = Vector2Type<int, float>;
using Vector2 = Vector2Type<Float, Float>;

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType> operator*(Number scalar, const Vector2Type<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector2Type<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
Vector2Type<Number, DistanceType>::operator std::string() const;