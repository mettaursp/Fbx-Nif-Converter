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
class Vector3Type;

template <typename Number, typename DistanceType>
class Vector3SType
{
public:
	Number X, Y, Z;

	constexpr Vector3SType();
	constexpr Vector3SType(Number x, Number y = 0, Number z = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3SType(const Vector2Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3SType(const Vector2SType<OtherNumber, OtherDistanceType>& other, Number z = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3SType(const Vector3Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector3SType(const Vector3SType<OtherNumber, OtherDistanceType>& other);

	Vector3SType& Set(Number x, Number y = 0, Number z = 0);
	Vector3SType& Normalize();
	DistanceType Dot(const Vector3SType& other) const;
	Vector3SType Cross(const Vector3SType& other) const;
	Vector3SType Unit() const;
	DistanceType Length() const;
	DistanceType SquareLength() const;
	Vector3SType& Scale(const Vector3SType& other);
	Vector3SType& Scale(Number x, Number y, Number z);
	Vector3SType& InvertLength();
	Vector3SType InvertedLength() const;
	Vector3SType Negate() const;
	Vector3SType operator-() const;
	Vector3SType operator+(const Vector3SType& other) const;
	Vector3SType operator-(const Vector3SType& other) const;
	Vector3SType operator*(Number scalar) const;
	DistanceType operator*(const Vector3SType& other) const;
	Vector3SType& operator=(const Vector3SType& other);
	Vector3SType& operator+=(const Vector3SType& other);
	Vector3SType& operator-=(const Vector3SType& other);
	Vector3SType& operator*=(Number scalar);
	bool operator==(const Vector3SType& other) const;
	bool operator!=(const Vector3SType& other) const;
	Number operator[](int i) const;
	Number& operator[](int i);
	operator std::string() const;

private:
	bool Compare(Number x, Number y, Number epsilon = (Number)1e-5) const;
};

using Vector3SF = Vector3SType<float, float>;
using Vector3SD = Vector3SType<double, double>;
using Vector3SI = Vector3SType<int, float>;
using Vector3S = Vector3SType<Float, Float>;

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType> operator*(Number scalar, const Vector3SType<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector3SType<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
Vector3SType<Number, DistanceType>::operator std::string() const;