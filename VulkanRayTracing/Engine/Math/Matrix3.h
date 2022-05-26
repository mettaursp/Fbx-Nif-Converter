#pragma once
/*
Matrix.Cpp
Author:
9/16/16

contains implimentation of Matrix class functions
*/

#include "Matrix3-decl.h"

template <typename Number>
constexpr Matrix3Type<Number>::Matrix3Type()
{
	Identity();
}

template <typename Number>
constexpr Matrix3Type<Number>::Matrix3Type(Number x, Number y)
{
	Translate(x, y);
}

template <typename Number>
constexpr Matrix3Type<Number>::Matrix3Type(const Vec2& vector)
{
	Translate(vector);
}

template <typename Number>
constexpr Matrix3Type<Number>::Matrix3Type(const Vec2& position, const Vec2& right, const Vec2& up)
{
	SetVectors(position, right, up);
}

template <typename Number>
template <typename OtherNumber>
constexpr Matrix3Type<Number>::Matrix3Type(const Matrix3Type<OtherNumber>& mat)
{
	for (int i = 0; i < 9; ++i)
		Data[i % 3][i / 3] = (Number)mat.Data[i % 3][i / 3];
}

template <typename Number>
template <typename OtherNumber>
constexpr Matrix3Type<Number>::Matrix3Type(const Matrix4Type<OtherNumber>& mat)
{
	for (int i = 0; i < 9; ++i)
		Data[i % 3][i / 3] = (Number)mat.Data[i % 3][i / 3];
}

template <typename Number>
constexpr Matrix3Type<Number>& Matrix3Type<Number>::Identity()
{
	for (int i = 0; i < 9; i++)
		Data[i % 3][i / 3] = i % 3 == i / 3;

	return *this;
}

template <typename Number>
constexpr Matrix3Type<Number>& Matrix3Type<Number>::SetVectors(const Vec2& position, const Vec2& right, const Vec2& up)
{
	Translate(position);

	Fetch(0, 0) = right.X;
	Fetch(1, 0) = right.Y;

	Fetch(0, 1) = up.X;
	Fetch(1, 1) = up.Y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Transpose()
{
	for (int x = 1; x < 3; ++x)
	{
		for (int y = 0; y < x; ++y)
		{
			Number temp = Data[x][y];

			Data[x][y] = Data[y][x];
			Data[y][x] = temp;
		}
	}

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::SetRight(const Vec2& vector)
{
	Fetch(0, 0) = vector.X;
	Fetch(1, 0) = vector.Y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::SetUp(const Vec2& vector)
{
	Fetch(0, 1) = vector.X;
	Fetch(1, 1) = vector.Y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::SetTranslation(const Vec2& vector)
{
	Fetch(0, 2) = vector.X;
	Fetch(1, 2) = vector.Y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::SetTransformedTranslation(const Vec2& vector)
{
	Fetch(0, 2) = vector.X * Fetch(0, 0) + vector.Y * Fetch(0, 1);
	Fetch(1, 2) = vector.X * Fetch(1, 0) + vector.Y * Fetch(1, 1);
	Fetch(2, 2) = vector.X * Fetch(2, 0) + vector.Y * Fetch(2, 1);

	return *this;
}

template <typename Number>
constexpr Matrix3Type<Number>& Matrix3Type<Number>::Translate(Number x, Number y)
{
	Identity();

	Fetch(0, 2) = x;
	Fetch(1, 2) = y;

	return *this;
}

template <typename Number>
constexpr Matrix3Type<Number>& Matrix3Type<Number>::Translate(const Vec2& vector)
{
	return Translate(vector.X, vector.Y);
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Scale(Number x, Number y)
{
	Identity();

	Data[0][0] = x;
	Data[1][1] = y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Scale(const Vec2& vector)
{
	return Scale(vector.X, vector.Y);
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Rotate(Number theta)
{
	Identity();

	Number sine = std::sin(theta);
	Number cosine = std::cos(theta);

	Fetch(0, 0) = cosine;
	Fetch(1, 0) = sine;

	Fetch(0, 1) = -sine;
	Fetch(1, 1) = cosine;

	return *this;
}

template <typename Number>
Number Matrix3Type<Number>::Det(int y1, int y2, int x1, int x2) const
{
	return Fetch(y1, x1) * Fetch(y2, x2) - Fetch(y1, x2) * Fetch(y2, x1);
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Inverse()
{
	Number determinant = 1 / Det();

	std::swap(Data[0][0], Data[1][1]);

	Data[1][0] *= -1;
	Data[0][1] *= -1;

	for (int i = 0; i < 4; ++i)
		Data[i % 3][i / 3] *= determinant;

	Vec2 vector = Translation();

	Fetch(0, 2) = 0;
	Fetch(1, 2) = 0;
	Fetch(2, 2) = 1;

	vector = *this * vector;

	Fetch(0, 2) = -vector.X;
	Fetch(1, 2) = -vector.Y;

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::Invert(const Matrix3Type& other)
{
	*this = other;

	Inverse();

	return *this;
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::ExtractRotation(const Matrix3Type& matrix, const Vec2& newTranslation)
{
	return SetVectors(newTranslation, matrix.RightVector().Normalize(), matrix.UpVector().Normalize());
}

template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::TransformAround(const Vec2& point)
{
	Vec2 translationVector = (*this * -point) + point;

	Fetch(0, 2) = translationVector.X;
	Fetch(1, 2) = translationVector.Y;
	Fetch(2, 2) = 1.f;

	return *this;
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::Transposed() const
{
	return Matrix3Type(*this).Transpose();
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::Inverted() const
{
	return Matrix3Type(*this).Inverse();
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::Rotation(const Vec2& newTranslation) const
{
	return Matrix3Type(true).ExtractRotation(*this, newTranslation);
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::TransformedAround(const Vec2& point) const
{
	return Matrix3Type(*this).TransformAround(point);
}

//returns the right vector of a transformation matrix. mostly useful for rotations.
template <typename Number>
Vector2Type<Number, Number> Matrix3Type<Number>::RightVector() const
{
	return Vec2(Fetch(0, 0), Fetch(1, 0), 0.f);
}

//returns the up vector of a transformation matrix. mostly useful for rotations.
template <typename Number>
Vector2Type<Number, Number> Matrix3Type<Number>::UpVector() const
{
	return Vec2(Fetch(0, 1), Fetch(1, 1), 0.f);
}

// returns the translation of the transformation matrix as a point
template <typename Number>
Vector2Type<Number, Number> Matrix3Type<Number>::Translation() const
{
	return Vec2(Fetch(0, 2), Fetch(1, 2), 1.f);
}

template <typename Number>
Vector2Type<Number, Number> Matrix3Type<Number>::ExtractScale() const
{
	return Vec2(RightVector().Length(), UpVector().Length());
}

template <typename Number>
Number Matrix3Type<Number>::Det() const
{
	return Det(0, 1, 0, 1);
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::operator+(const Matrix3Type& other) const
{
	Matrix3Type<Number> result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = Data[y][x] + other.Data[y][x];

	return result;
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::operator-(const Matrix3Type& other) const
{
	Matrix3Type<Number> result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = Data[y][x] - other.Data[y][x];

	return result;
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::operator-() const
{
	Matrix3Type<Number> result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = -Data[y][x];

	return result;
}

// scalar multiply
template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::operator*(Number scalar) const
{
	Matrix3Type<Number> result;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			result.Data[y][x] = scalar * Data[y][x];

	return result;
}

//overloaded multiplication operator
template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::operator*(const Matrix3Type& other) const
{
	Matrix3Type<Number> results;

	results.Data[0][0] = 0;
	results.Data[1][1] = 0;
	results.Data[2][2] = 0;

	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			for (int i = 0; i < 3; ++i)
				results.Fetch(x, y) += Fetch(x, i) * other.Fetch(i, y);

	return results;
}

// transform a vector
template <typename Number>
Vector2Type<Number, Number> Matrix3Type<Number>::operator*(const Vec2& other) const
{
	return Vec2(
		other.X * Fetch(0, 0) + other.Y * Fetch(0, 1) + other.Z * Fetch(0, 2),
		other.X * Fetch(1, 0) + other.Y * Fetch(1, 1) + other.Z * Fetch(1, 2),
		other.X * Fetch(2, 0) + other.Y * Fetch(2, 1) + other.Z * Fetch(2, 2)
		);
}

template <typename Number>
Number* Matrix3Type<Number>::operator[](int row)
{
	return Data[row];
}

template <typename Number>
const Number* Matrix3Type<Number>::operator[](int row) const
{
	return Data[row];
}

// assignment
template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::operator=(const Matrix3Type& other)
{
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			Data[y][x] = other.Data[y][x];

	return *this;
}

// addition assignment
template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::operator+=(const Matrix3Type& other)
{
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			Data[y][x] += other.Data[y][x];

	return *this;
}

// subtraction assignment
template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::operator-=(const Matrix3Type& other)
{
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			Data[y][x] -= other.Data[y][x];

	return *this;
}

// transformation assignment
template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::operator*=(const Matrix3Type& other)
{
	*this = *this * other;

	return *this;
}

// scalar assignment
template <typename Number>
Matrix3Type<Number>& Matrix3Type<Number>::operator*=(Number scalar)
{
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			Data[y][x] *= scalar;

	return *this;
}

template <typename Number>
bool Matrix3Type<Number>::Compare(Number x, Number y, Number epsilon)
{
	return std::abs(x - y) < epsilon;
}

template <typename Number>
bool Matrix3Type<Number>::operator==(const Matrix3Type& other) const
{
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			if (!Compare(Data[y][x], other.Data[y][x], 1e-4f))
				return false;

	return true;
}

template <typename Number>
bool Matrix3Type<Number>::operator!=(const Matrix3Type& other) const
{
	return !(*this == other);
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::NewScale(Number x, Number y)
{
	return Matrix3Type(true).Scale(x, y);
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::NewScale(const Vec2& vector)
{
	return Matrix3Type(true).Scale(vector);
}

template <typename Number>
Matrix3Type<Number> Matrix3Type<Number>::Rotation(Number theta)
{
	return Matrix3Type(true).Rotate(theta);
}

template <typename Number>
Matrix3Type<Number>::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

// stream output
template <typename Number>
std::ostream& operator<<(std::ostream& out, const Matrix3Type<Number>& matrix)
{
	// put the formatted data into the provided output stream
	out << "[ " << matrix.Fetch(0, 0) << ", " << matrix.Fetch(0, 1) << ", " << matrix.Fetch(0, 2) << " ]\n"
		<< "[ " << matrix.Fetch(1, 0) << ", " << matrix.Fetch(1, 1) << ", " << matrix.Fetch(1, 2) << " ]\n"
		<< "[ " << matrix.Fetch(2, 0) << ", " << matrix.Fetch(2, 1) << ", " << matrix.Fetch(2, 2) << " ]" << std::endl;

	return out;
}
