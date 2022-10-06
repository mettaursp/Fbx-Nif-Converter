#include "Quaternion.h"

#include "Vector3.h"

import <sstream>;

Quaternion::Quaternion(const Vector3& axis, Float angle)
{
	*this = std::sin(0.5_F * angle) * Vector3(axis.X, axis.Y, axis.Z, 0._F).Normalize() + Vector3(0._F, 0._F, 0._F, std::cos(0.5_F * angle));
}

Quaternion::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

std::ostream& operator<<(std::ostream& out, const Quaternion& quaternion)
{
	return out << "( " << quaternion.W << " + " << quaternion.X << "i + " << quaternion.Y << "j + " << quaternion.Z << "k )";
}


// returns the length of the vector
Float Quaternion::Length() const
{
	return std::sqrt(SquareLength());
}

// normalizes the vector
Quaternion& Quaternion::Normalize()
{
	Float length = 1 / this->Length();
	W *= length;
	X *= length;
	Y *= length;
	Z *= length;

	return *this;
}

Matrix4F Quaternion::MatrixF() const
{
	float xx = float(X * X);
	float yy = float(Y * Y);
	float zz = float(Z * Z);
	float xz = float(X * Z);
	float xy = float(X * Y);
	float yz = float(Y * Z);
	float wx = float(W * X);
	float wy = float(W * Y);
	float wz = float(W * Z);

	return Matrix4F(
		Vector3F(0, 0, 0, 1),
		Vector3F(1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy)).Normalize(),
		Vector3F(2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx)).Normalize(),
		Vector3F(2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)).Normalize()
	);
}

Matrix4D Quaternion::MatrixD() const
{
	double xx = double(X * X);
	double yy = double(Y * Y);
	double zz = double(Z * Z);
	double xz = double(X * Z);
	double xy = double(X * Y);
	double yz = double(Y * Z);
	double wx = double(W * X);
	double wy = double(W * Y);
	double wz = double(W * Z);

	return Matrix4D(
		Vector3D(0, 0, 0, 1),
		Vector3D(1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy)).Normalize(),
		Vector3D(2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx)).Normalize(),
		Vector3D(2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)).Normalize()
	);
}

Matrix4 Quaternion::Matrix() const
{
	Float xx = X * X;
	Float yy = Y * Y;
	Float zz = Z * Z;
	Float xz = X * Z;
	Float xy = X * Y;
	Float yz = Y * Z;
	Float wx = W * X;
	Float wy = W * Y;
	Float wz = W * Z;

	return Matrix4(
		Vector3(0, 0, 0, 1),
		Vector3(1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy)).Normalize(),
		Vector3(2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx)).Normalize(),
		Vector3(2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)).Normalize()
	);
}

Quaternion::Quaternion(const Matrix4& matrix)
{
	Float traceW = matrix[0][0] + matrix[1][1] + matrix[2][2];
	Float traceX = matrix[0][0] - matrix[1][1] - matrix[2][2];
	Float traceY = -matrix[0][0] + matrix[1][1] - matrix[2][2];
	Float traceZ = -matrix[0][0] - matrix[1][1] + matrix[2][2];

	Float axis1 = (matrix[2][1] + matrix[1][2]);
	Float axis2 = (matrix[0][2] + matrix[2][0]);
	Float axis3 = (matrix[1][0] + matrix[0][1]);

	Float axis1N = (matrix[2][1] - matrix[1][2]);
	Float axis2N = (matrix[0][2] - matrix[2][0]);
	Float axis3N = (matrix[1][0] - matrix[0][1]);

	if (traceW > traceX && traceW > traceY && traceW > traceZ)
	{
		W = 0.5f * std::sqrt(1 + traceW);
		X = axis1N;
		Y = axis2N;
		Z = axis3N;

		X /= 4 * W;
		Y /= 4 * W;
		Z /= 4 * W;
	}
	else if (traceX > traceY && traceX > traceZ)
	{
		W = axis1N;
		X = 0.5f * std::sqrt(1 + traceX);
		Y = axis3;
		Z = axis2;

		W /= 4 * X;
		Y /= 4 * X;
		Z /= 4 * X;
	}
	else if (traceY > traceZ)
	{
		W = axis2N;
		X = axis3;
		Y = 0.5f * std::sqrt(1 + traceY);
		Z = axis1;

		W /= 4 * Y;
		X /= 4 * Y;
		Z /= 4 * Y;
	}
	else
	{
		W = axis3N;
		X = axis2;
		Y = axis1;
		Z = 0.5f * std::sqrt(1 + traceZ);

		W /= 4 * Z;
		X /= 4 * Z;
		Y /= 4 * Z;
	}

	Normalize();
}


Quaternion& Quaternion::Invert()
{
	X *= -1;
	Y *= -1;
	Z *= -1;

	return *this;
}

Quaternion Quaternion::Slerp(const Quaternion& destination, Float t) const
{

	Float halfCos = Dot(destination);
	Float scalar = 1;

	if (halfCos >= 1 || halfCos <= -1)
		return *this;
	else if (halfCos < 0)
		scalar = -1;

	halfCos *= scalar;

	Float halfTheta = std::acos(halfCos);
	Float halfSin = std::sqrt(1 - halfCos * halfCos);

	if (halfSin < 1e-3_F && halfSin > -1e-3_F)
		return 0.5_F * (*this + scalar * destination);

	Float ratioA = std::sin((1 - t) * halfTheta) / halfSin;
	Float ratioB = scalar * std::sin(t * halfTheta) / halfSin;

	return (ratioA * *this + ratioB * destination).Normalize();
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	return Quaternion(
		W * rhs.X + X * rhs.W + Y * rhs.Z - Z * rhs.Y,
		W * rhs.Y - X * rhs.Z + Y * rhs.W + Z * rhs.X,
		W * rhs.Z + X * rhs.Y - Y * rhs.X + Z * rhs.W,
		W * rhs.W - X * rhs.X - Y * rhs.Y - Z * rhs.Z
	);
}


// calculates the cross product between two vectors
Quaternion Quaternion::Cross(const Quaternion& other) const
{
	return Vector3(
		Y * other.Z - Z * other.Y,
		Z * other.X - X * other.Z,
		X * other.Y - Y * other.X
	);
}

Quaternion Quaternion::Unit() const
{
	return Quaternion(*this).Normalize();
}

// returns the square length of the vector
Float Quaternion::SquareLength() const
{
	return Dot(*this);
}

Float Quaternion::Dot(const Quaternion& other) const
{
	return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
}

// negation
Quaternion Quaternion::operator-() const
{
	return Quaternion(-W, -X, -Y, -Z);
}

// addition
Quaternion Quaternion::operator+(const Quaternion& other) const
{
	return Quaternion(W + other.W, X + other.X, Y + other.Y, Z + other.Z);
}

// subtraction
Quaternion Quaternion::operator-(const Quaternion& other) const
{
	return Quaternion(W - other.W, X - other.X, Y - other.Y, Z - other.Z);
}

// scalar multiplication
Quaternion Quaternion::operator*(Float scalar) const
{
	return Quaternion(scalar * W, scalar * X, scalar * Y, scalar * Z);
}

// assignment
Quaternion& Quaternion::operator=(const Quaternion& other)
{
	W = other.X;
	X = other.Y;
	Y = other.Z;
	Z = other.W;

	return *this;
}

// addition assignment
Quaternion& Quaternion::operator+=(const Quaternion& other)
{
	*this = *this + other;

	return *this;
}

// subtraction assignment
Quaternion& Quaternion::operator-=(const Quaternion& other)
{
	*this = *this - other;

	return *this;
}

// multiplication assignment
Quaternion& Quaternion::operator*=(Float scalar)
{
	*this = *this * scalar;

	return *this;
}

bool Quaternion::Compare(Float x, Float y, Float epsilon) const
{
	Float a = x - y;

	return a < epsilon&& a > -epsilon;
}

bool Quaternion::operator==(const Quaternion& other) const
{
	Float epsilon = 1e-5f;

	return Compare(X, other.X, epsilon) && Compare(Y, other.Y, epsilon) && Compare(Z, other.Z, epsilon) && Compare(W, other.W, epsilon);
}

bool Quaternion::operator!=(const Quaternion& other) const
{
	return !(*this == other);
}

Float Quaternion::operator[](int i) const
{
	return ((const Float*)(this))[i];
}

Float& Quaternion::operator[](int i)
{
	return ((Float*)(this))[i];
}

Quaternion::operator Vector3() const
{
	return Vector3(X, Y, Z, W);
}

Quaternion operator*(Float scalar, const Quaternion& quaternion)
{
	return quaternion * scalar;
}