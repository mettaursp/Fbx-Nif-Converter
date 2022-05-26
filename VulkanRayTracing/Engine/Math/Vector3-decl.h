#pragma once

import <iostream>;
import <string>;
import <cmath>;

#include "../Precision.h"

template <typename Number, typename DistanceType>
class Vector2Type;

template <typename Number, typename DistanceType>
class Vector2SType;

template <typename Number, typename DistanceType>
class Vector3SType;

template <typename Number, typename DistanceType>
class Vector3Type
{
public:
	Number X, Y, Z, W;

	constexpr Vector3Type();
	constexpr Vector3Type(Number x, Number y = 0, Number z = 0, Number w = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3Type(const Vector2Type<OtherNumber, OtherDistanceType>& other, Number w = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3Type(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z = 0, Number w = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3Type(const Vector3Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3Type(const Vector3SType<OtherNumber, OtherDistanceType>& other, Number w = 0);

	Vector3Type& Set(Number x, Number y = 0, Number z = 0, Number w = 0);
	Vector3Type& Normalize();
	DistanceType Dot(const Vector3Type& other) const;
	Vector3Type Cross(const Vector3Type& other) const;
	Vector3Type Unit() const;
	DistanceType Length() const;
	DistanceType SquareLength() const;
	Vector3Type& Scale(const Vector3Type& other);
	Vector3Type& Scale(Number x, Number y, Number z, Number w = 1);
	Vector3Type& InvertLength();
	Vector3Type InvertedLength() const;
	Vector3Type Negate() const;
	Vector3Type operator-() const;
	Vector3Type operator+(const Vector3Type& other) const;
	Vector3Type operator-(const Vector3Type& other) const;
	Vector3Type operator*(Number scalar) const;
	DistanceType operator*(const Vector3Type& other) const;
	Vector3Type& operator=(const Vector3Type& other);
	Vector3Type& operator+=(const Vector3Type& other);
	Vector3Type& operator-=(const Vector3Type& other);
	Vector3Type& operator*=(Number scalar);
	bool operator==(const Vector3Type& other) const;
	bool operator!=(const Vector3Type& other) const;
	Number operator[](int i) const;
	Number& operator[](int i);

	operator std::string() const;

private:
	bool Compare(Number x, Number y, Number epsilon = (Number)1e-5) const;
};

using Vector3F = Vector3Type<float, float>;
using Vector3D = Vector3Type<double, double>;
using Vector3I = Vector3Type<int, float>;
using Vector3 = Vector3Type<Float, Float>;

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType> operator*(Number scalar, const Vector3Type<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector3Type<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
Vector3Type<Number, DistanceType>::operator std::string() const;