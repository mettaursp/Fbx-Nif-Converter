#pragma once

#include "Vector3.h"

struct Color1;
struct Color1I;
struct Color2;
struct Color2I;
struct Color3;
struct Color3I;
struct Color4;
struct Color4I;

struct Color4I
{
	typedef unsigned char byte;

	byte R, G, B, A;

	constexpr Color4I() : R(255), G(255), B(255), A(255) {}
	constexpr Color4I(byte r, byte g, byte b, byte a = 255)
	{
		R = r;
		G = g;
		B = b;
		A = a;
	}
	constexpr Color4I(float r, float g, float b, float a = 255)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
		B = (byte)(b * 255);
		A = (byte)(a * 255);
	}
	constexpr Color4I(double r, double g, double b, double a = 255)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
		B = (byte)(b * 255);
		A = (byte)(a * 255);
	}
	constexpr Color4I(unsigned int color)
	{
		R = (byte)((color >> 24) & 0xFF);
		G = (byte)((color >> 16) & 0xFF);
		B = (byte)((color >> 8) & 0xFF);
		A = (byte)(color & 0xFF);
	}

	Color4I(const Color1& color, byte g = 0, byte b = 0, byte a = 255);
	Color4I(const Color1I& color, byte g = 0, byte b = 0, byte a = 255);
	Color4I(const Color2& color, byte b = 0, byte a = 255);
	Color4I(const Color2I& color, byte b = 0, byte a = 255);
	Color4I(const Color3& color, byte a = 255);
	Color4I(const Color3I& color, byte a = 255);
	Color4I(const Color4& color);
	Color4I(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color4I(const Vector2SType<Number, DistanceType>& vector, byte b = 0, byte a = 255) : R(byte(vector.X)), G(byte(vector.Y)), B(b), A(a) {}

	template <typename Number, typename DistanceType>
	constexpr Color4I(const Vector2Type<Number, DistanceType>& vector, byte a = 255) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)), A(a) {}

	template <typename Number, typename DistanceType>
	constexpr Color4I(const Vector3Type<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)), A(byte(vector.W)) {}

	template <typename Number, typename DistanceType>
	constexpr Color4I(const Vector3SType<Number, DistanceType>& vector, byte a = 255) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)), A(a) {}

	Color4I& Set(byte r = 1, byte g = 1, byte b = 1, byte a = 1);
	Color4I Lerp(const Color4I& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color4I& color) const;
	bool operator!=(const Color4I& color) const;

	Color4I operator+(const Color4I& other) const;
	Color4I operator-(const Color4I& other) const;
	Color4I operator*(const Color4I& other) const;
	Color4I operator*(float scalar) const;
	Color4I operator*(byte scalar) const;
	Color4I operator/(const Color4I& other) const;
	Color4I operator/(float scalar) const;
	Color4I operator/(byte scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color4I& operator=(const Color4I& other);
	Color4I& operator+=(const Color4I& color);
	Color4I& operator-=(const Color4I& color);
	Color4I& operator*=(const Color4I& color);
	Color4I& operator*=(float scalar);
	Color4I& operator*=(byte scalar);
	Color4I& operator/=(const Color4I& color);
	Color4I& operator/=(float scalar);
	Color4I& operator/=(byte scalar);
};

Color4I operator*(float scalar, const Color4I& color);
Color4I operator*(Color4I::byte scalar, const Color4I& color);
Color4I operator/(float scalar, const Color4I& color);
Color4I operator/(Color4I::byte scalar, const Color4I& color);
std::ostream& operator<<(std::ostream& out, const Color4I& color);
