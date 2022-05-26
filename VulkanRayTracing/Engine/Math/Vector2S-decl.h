#pragma once

import <iostream>;
import <string>;
import <cmath>;

#include "../Precision.h"

template <typename Number, typename DistanceType>
class Vector2Type;

template <typename Number, typename DistanceType>
class Vector3Type;

template <typename Number, typename DistanceType>
class Vector3SType;

template <typename Number, typename DistanceType>
class Vector2SType
{
public:
	Number X, Y;

	constexpr Vector2SType();
	constexpr Vector2SType(Number x, Number y = 0);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2SType(const Vector2Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2SType(const Vector2SType<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2SType(const Vector3Type<OtherNumber, OtherDistanceType>& other);

	template <typename OtherNumber, typename OtherDistanceType>
	constexpr Vector2SType(const Vector3SType<OtherNumber, OtherDistanceType>& other);

	Vector2SType& Set(Number x, Number y = 0);
	Vector2SType& Normalize();
	DistanceType Dot(const Vector2SType& other) const;
	Vector2SType Unit() const;
	DistanceType Length() const;
	DistanceType SquareLength() const;
	Vector2SType& InvertLength();
	Vector2SType InvertedLength() const;
	Vector2SType& Scale(const Vector2SType& other);
	Vector2SType& Scale(Number x, Number y);
	Vector2SType Negate() const;
	Vector2SType operator-() const;
	Vector2SType operator+(const Vector2SType& other) const;
	Vector2SType operator-(const Vector2SType& other) const;
	Vector2SType operator*(Number scalar) const;
	DistanceType operator*(const Vector2SType& other) const;
	Vector2SType& operator=(const Vector2SType& other);
	Vector2SType& operator+=(const Vector2SType& other);
	Vector2SType& operator-=(const Vector2SType& other);
	Vector2SType& operator*=(Number scalar);
	bool operator==(const Vector2SType& other) const;
	bool operator!=(const Vector2SType& other) const;
	Number operator[](int i) const;
	Number& operator[](int i);
	operator std::string() const;

private:
	bool Compare(Number x, Number y, Number epsilon = (Number)1e-5) const;
};

using Vector2SF = Vector2SType<float, float>;
using Vector2SD = Vector2SType<double, double>;
using Vector2SI = Vector2SType<int, float>;
using Vector2S = Vector2SType<Float, Float>;

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType> operator*(Number scalar, const Vector2SType<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
std::ostream& operator<<(std::ostream& out, const Vector2SType<Number, DistanceType>& vector);

template <typename Number, typename DistanceType>
Vector2SType<Number, DistanceType>::operator std::string() const;
