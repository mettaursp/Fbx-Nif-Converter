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

struct Color2I
{
	typedef unsigned char byte;

	byte R, G;

	constexpr Color2I() : R(255), G(255) {}
	constexpr Color2I(byte r, byte g)
	{
		R = r;
		G = g;
	}
	constexpr Color2I(float r, float g)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
	}
	constexpr Color2I(double r, double g)
	{
		R = (byte)(r * 255);
		G = (byte)(g * 255);
	}
	constexpr Color2I(unsigned int color)
	{
		R = (byte)((color >> 24) & 0xFF);
		G = (byte)((color >> 16) & 0xFF);
	}

	Color2I(const Color1& color, byte g = 0);
	Color2I(const Color1I& color, byte g = 0);
	Color2I(const Color2& color);
	Color2I(const Color2I& color);
	Color2I(const Color3& color);
	Color2I(const Color3I& color);
	Color2I(const Color4& color);
	Color2I(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color2I(const Vector2SType<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2I(const Vector2Type<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2I(const Vector3Type<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2I(const Vector3SType<Number, DistanceType>& vector) : R(byte(vector.X)), G(byte(vector.Y)) {}

	Color2I& Set(byte r = 1, byte g = 1);
	Color2I Lerp(const Color2I& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color2I& color) const;
	bool operator!=(const Color2I& color) const;

	Color2I operator+(const Color2I& other) const;
	Color2I operator-(const Color2I& other) const;
	Color2I operator*(const Color2I& other) const;
	Color2I operator*(float scalar) const;
	Color2I operator*(byte scalar) const;
	Color2I operator/(const Color2I& other) const;
	Color2I operator/(float scalar) const;
	Color2I operator/(byte scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color2I& operator=(const Color2I& other);
	Color2I& operator+=(const Color2I& color);
	Color2I& operator-=(const Color2I& color);
	Color2I& operator*=(const Color2I& color);
	Color2I& operator*=(float scalar);
	Color2I& operator*=(byte scalar);
	Color2I& operator/=(const Color2I& color);
	Color2I& operator/=(float scalar);
	Color2I& operator/=(byte scalar);
};

Color2I operator*(float scalar, const Color2I& color);
Color2I operator*(Color2I::byte scalar, const Color2I& color);
Color2I operator/(float scalar, const Color2I& color);
Color2I operator/(Color2I::byte scalar, const Color2I& color);
std::ostream& operator<<(std::ostream& out, const Color2I& color);
