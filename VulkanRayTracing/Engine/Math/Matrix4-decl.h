/*
Matrix.H
Author:
9/16/16

contains declaration of Matrix class
*/

#pragma once

#include "Vector3-decl.h"
#include "../Precision.h"

template <typename Number>
class Matrix3Type;

template <typename Number>
class Matrix4Type
{
public:
	typedef Vector3Type<Number, Number> Vec3;

	Number Data[4][4] = { {} };

	constexpr Matrix4Type();
	constexpr Matrix4Type(bool);
	constexpr Matrix4Type(Number x, Number y, Number z);
	constexpr Matrix4Type(const Vec3& vector);
	constexpr Matrix4Type(const Vec3& position, const Vec3& right, const Vec3& up, const Vec3& front);
	Matrix4Type(const Vec3& position, const Vec3& direction, const Vec3& globalUp = Vec3(0, 1, 0));

	template <typename OtherNumber>
	constexpr Matrix4Type(const Matrix3Type<OtherNumber>& mat, const Vec3& translation = Vec3(0, 0, 0));

	template <typename OtherNumber>
	constexpr Matrix4Type(const Matrix4Type<OtherNumber>& mat);

	constexpr Matrix4Type& Identity();
	Matrix4Type& Transpose();
	constexpr Matrix4Type& SetVectors(const Vec3& position, const Vec3& right, const Vec3& up, const Vec3& front);
	Matrix4Type& SetRight(const Vec3& vector);
	Matrix4Type& SetUp(const Vec3& vector);
	Matrix4Type& SetFront(const Vec3& vector);
	Matrix4Type& SetTranslation(const Vec3& vector);
	Matrix4Type& SetTransformedTranslation(const Vec3& vector);
	constexpr Matrix4Type& Translate(Number x, Number y, Number z);
	constexpr Matrix4Type& Translate(const Vec3& vector);
	Matrix4Type& Scale(Number x, Number y, Number z);
	Matrix4Type& Scale(const Vec3& vector);
	Matrix4Type& RotateAxis(const Vec3& axis, Number theta);
	Matrix4Type& RotatePitch(Number theta);
	Matrix4Type& RotateYaw(Number theta);
	Matrix4Type& RotateRoll(Number theta);
	Matrix4Type& RotateEulerAngles(Number pitch, Number yaw, Number roll);
	Matrix4Type& RotateEulerAnglesYaw(Number yaw, Number pitch, Number roll);
	Matrix4Type& Inverse();
	Matrix4Type& Invert(const Matrix4Type& other);
	Matrix4Type& Projection(Number distance, Number near, Number far, Number width, Number height);
	Matrix4Type& ExtractRotation(const Matrix4Type& matrix, const Vec3& newTranslation = Vec3());
	Matrix4Type& TransformAround(const Vec3& point);
	Matrix4Type& Face(const Vec3& position, const Vec3& direction, const Vec3& globalUp = Vec3(0, 1, 0));

	Matrix4Type Transposed() const;
	Matrix4Type Inverted() const;
	Matrix4Type Rotation(const Vec3& newTranslation = Vec3()) const;
	Matrix4Type TransformedAround(const Vec3& point) const;

	Vec3 RightVector() const;
	Vec3 UpVector() const;
	Vec3 FrontVector() const;
	Vec3 Translation() const;
	Vec3 ExtractScale() const;
	Number Det() const;

	Matrix4Type  operator+(const Matrix4Type& other) const;
	Matrix4Type  operator-(const Matrix4Type& other) const;
	Matrix4Type  operator-() const;
	Matrix4Type  operator*(const Matrix4Type& other) const;
	Vec3  operator*(const Vec3& other) const;
	Matrix4Type  operator*(Number scalar) const;

	Number* operator[](int row);
	const Number* operator[](int row) const;

	operator std::string() const;

	Matrix4Type& operator=(const Matrix4Type& other);
	Matrix4Type& operator+=(const Matrix4Type& other);
	Matrix4Type& operator-=(const Matrix4Type& other);
	Matrix4Type& operator*=(const Matrix4Type& other);
	Matrix4Type& operator*=(Number scalar);

	bool operator==(const Matrix4Type& other) const;
	bool operator!=(const Matrix4Type& other) const;

	static Matrix4Type NewScale(Number x, Number y, Number z);
	static Matrix4Type NewScale(const Vec3& vector);
	static Matrix4Type AxisRotation(const Vec3& axis, Number theta);
	static Matrix4Type PitchRotation(Number theta);
	static Matrix4Type YawRotation(Number theta);
	static Matrix4Type RollRotation(Number theta);
	static Matrix4Type EulerAnglesRotation(Number pitch, Number yaw, Number roll);
	static Matrix4Type EulerAnglesYawRotation(Number yaw, Number pitch, Number roll);
	static Matrix4Type NewProjection(Number distance, Number nearPlane, Number farPlane, Number width, Number height);
	static Matrix4Type Facing(const Vec3& position, const Vec3& direction, const Vec3& globalUp = Vec3(0, 1, 0));

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

using Matrix4F = Matrix4Type<float>;
using Matrix4D = Matrix4Type<double>;
using Matrix4 = Matrix4Type<Float>;

template <typename Number>
constexpr Matrix4Type<Number> operator*(Number scalar, const Matrix4Type<Number>& matrix)
{
	Matrix4Type result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = scalar * matrix.Data[y][x];

	return result;
}

template <typename Number>
std::ostream& operator<<(std::ostream& out, const Matrix4Type<Number>& matrix);