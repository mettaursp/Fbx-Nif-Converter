#pragma once

import <sstream>;
import <cmath>;
import <iostream>;

#include "Vector2-decl.h"
#include "../Precision.h"

template <typename Number>
class Matrix4Type;

template <typename Number>
class Matrix3Type
{
public:
	typedef Vector2Type<Number, Number> Vec2;

	Number Data[3][3] = { {} };

	constexpr Matrix3Type();
	constexpr Matrix3Type(bool) {}
	constexpr Matrix3Type(Number x, Number y);
	constexpr Matrix3Type(const Vec2& vector);
	constexpr Matrix3Type(const Vec2& position, const Vec2& right, const Vec2& up);

	template <typename OtherNumber>
	constexpr Matrix3Type(const Matrix3Type<OtherNumber>& mat);

	template <typename OtherNumber>
	constexpr Matrix3Type(const Matrix4Type<OtherNumber>& mat);

	constexpr Matrix3Type& Identity();
	Matrix3Type& Transpose();
	constexpr Matrix3Type& SetVectors(const Vec2& position, const Vec2& right, const Vec2& up);
	Matrix3Type& SetRight(const Vec2& vector);
	Matrix3Type& SetUp(const Vec2& vector);
	Matrix3Type& SetTranslation(const Vec2& vector);
	Matrix3Type& SetTransformedTranslation(const Vec2& vector);
	constexpr Matrix3Type& Translate(Number x, Number y);
	constexpr Matrix3Type& Translate(const Vec2& vector);
	Matrix3Type& Scale(Number x, Number y);
	Matrix3Type& Scale(const Vec2& vector);
	Matrix3Type& Rotate(Number theta);
	Matrix3Type& Inverse();
	Matrix3Type& Invert(const Matrix3Type& other);
	Matrix3Type& ExtractRotation(const Matrix3Type& matrix, const Vec2& newTranslation = Vec2());
	Matrix3Type& TransformAround(const Vec2& point);

	Matrix3Type Transposed() const;
	Matrix3Type Inverted() const;
	Matrix3Type Rotation(const Vec2& newTranslation = Vec2()) const;
	Matrix3Type TransformedAround(const Vec2& point) const;

	Vector2Type<Number, Number> RightVector() const;
	Vector2Type<Number, Number> UpVector() const;
	Vector2Type<Number, Number> Translation() const;
	Vector2Type<Number, Number> ExtractScale() const;
	Number Det() const;

	Matrix3Type  operator+(const Matrix3Type& other) const;
	Matrix3Type  operator-(const Matrix3Type& other) const;
	Matrix3Type  operator-() const;
	Matrix3Type  operator*(const Matrix3Type& other) const;
	Vector2Type<Number, Number>  operator*(const Vec2& other) const;
	Matrix3Type  operator*(Number scalar) const;

	Number* operator[](int row);
	const Number* operator[](int row) const;

	operator std::string() const;

	Matrix3Type& operator=(const Matrix3Type& other);
	Matrix3Type& operator+=(const Matrix3Type& other);
	Matrix3Type& operator-=(const Matrix3Type& other);
	Matrix3Type& operator*=(const Matrix3Type& other);
	Matrix3Type& operator*=(Number scalar);

	bool operator==(const Matrix3Type& other) const;
	bool operator!=(const Matrix3Type& other) const;

	static Matrix3Type NewScale(Number x, Number y);
	static Matrix3Type NewScale(const Vec2& vector);
	static Matrix3Type Rotation(Number theta);

	constexpr inline Number& Fetch(int x, int y)
	{
		if (TransposedMatrices)
			return Data[y][x];
		else
			return Data[x][y];
	}

	constexpr inline Number Fetch(int x, int y) const
	{
		if (TransposedMatrices)
			return Data[y][x];
		else
			return Data[x][y];
	}

private:
	static bool Compare(Number x, Number y, Number epsilon);

	Number Det(int y1, int y2, int x1, int x2) const;
};

using Matrix3F = Matrix3Type<float>;
using Matrix3D = Matrix3Type<double>;
using Matrix3 = Matrix3Type<Float>;

template <typename Number>
constexpr Matrix3Type<Number> operator*(Number scalar, const Matrix3Type<Number>& matrix)
{
	Matrix3Type result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = scalar * matrix.Data[y][x];

	return result;
}

template <typename Number>
std::ostream& operator<<(std::ostream& out, const Matrix3Type<Number>& matrix);