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

struct Color3I
{
	typedef unsigned char byte;

	byte R, G, B;

	constexpr Color3I() : R(255), G(255), B(255) {}
	constexpr Color3I(byte r, byte g, byte b)
	{
		R = r;
		G = g;
		B = b;
	}
	constexpr Color3I(float r, float g, float b)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
		B = (byte)(b * 255);
	}
	constexpr Color3I(double r, double g, double b)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
		B = (byte)(b * 255);
	}
	constexpr Color3I(unsigned int color)
	{
		R = (byte)((color >> 24) & 0xFF);
		G = (byte)((color >> 16) & 0xFF);
		B = (byte)((color >> 8) & 0xFF);
	}

	Color3I(const Color1& color, byte g = 0, byte b = 0);
	Color3I(const Color1I& color, byte g = 0, byte b = 0);
	Color3I(const Color2& color, byte b = 0);
	Color3I(const Color2I& color, byte b = 0);
	Color3I(const Color3& color);
	Color3I(const Color3I& color);
	Color3I(const Color4& color);
	Color3I(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color3I(const Vector2SType<Number, DistanceType>& vector, byte b = 0) : R(byte(vector.X)), G(byte(vector.Y)), B(b) {}

	template <typename Number, typename DistanceType>
	constexpr Color3I(const Vector2Type<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)) {}

	template <typename Number, typename DistanceType>
	constexpr Color3I(const Vector3Type<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)) {}

	template <typename Number, typename DistanceType>
	constexpr Color3I(const Vector3SType<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)), B(byte(vector.Z)) {}

	Color3I& Set(byte r = 1, byte g = 1, byte b = 1);
	Color3I Lerp(const Color3I& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color3I& color) const;
	bool operator!=(const Color3I& color) const;

	Color3I operator+(const Color3I& other) const;
	Color3I operator-(const Color3I& other) const;
	Color3I operator*(const Color3I& other) const;
	Color3I operator*(float scalar) const;
	Color3I operator*(byte scalar) const;
	Color3I operator/(const Color3I& other) const;
	Color3I operator/(float scalar) const;
	Color3I operator/(byte scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color3I& operator=(const Color3I& other);
	Color3I& operator+=(const Color3I& color);
	Color3I& operator-=(const Color3I& color);
	Color3I& operator*=(const Color3I& color);
	Color3I& operator*=(float scalar);
	Color3I& operator*=(byte scalar);
	Color3I& operator/=(const Color3I& color);
	Color3I& operator/=(float scalar);
	Color3I& operator/=(byte scalar);
};

Color3I operator*(float scalar, const Color3I& color);
Color3I operator*(Color3I::byte scalar, const Color3I& color);
Color3I operator/(float scalar, const Color3I& color);
Color3I operator/(Color3I::byte scalar, const Color3I& color);
std::ostream& operator<<(std::ostream& out, const Color3I& color);
