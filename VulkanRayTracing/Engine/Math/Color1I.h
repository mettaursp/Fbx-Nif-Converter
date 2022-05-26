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

struct Color1I
{
	typedef unsigned char byte;

	byte R;

	constexpr Color1I() : R(255) {}
	constexpr Color1I(byte r)
	{
		R = r;
	}
	constexpr Color1I(float r)
	{
		R = (byte)(r * 255);
	}
	constexpr Color1I(double r)
	{
		R = (byte)(r * 255);
	}
	constexpr Color1I(unsigned int color)
	{
		R = (byte)((color >> 24) & 0xFF);
	}

	Color1I(const Color1& color);
	Color1I(const Color1I& color);
	Color1I(const Color2& color);
	Color1I(const Color2I& color);
	Color1I(const Color3& color);
	Color1I(const Color3I& color);
	Color1I(const Color4& color);
	Color1I(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color1I(const Vector2SType<Number, DistanceType>& vector) : R(byte(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1I(const Vector2Type<Number, DistanceType>& vector) : R(byte(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1I(const Vector3Type<Number, DistanceType>& vector) : R(byte(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1I(const Vector3SType<Number, DistanceType>& vector) : R(byte(vector.X)) {}

	Color1I& Set(byte r = 1);
	Color1I Lerp(const Color1I& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color1I& color) const;
	bool operator!=(const Color1I& color) const;

	Color1I operator+(const Color1I& other) const;
	Color1I operator-(const Color1I& other) const;
	Color1I operator*(const Color1I& other) const;
	Color1I operator*(float scalar) const;
	Color1I operator*(byte scalar) const;
	Color1I operator/(const Color1I& other) const;
	Color1I operator/(float scalar) const;
	Color1I operator/(byte scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color1I& operator=(const Color1I& other);
	Color1I& operator+=(const Color1I& color);
	Color1I& operator-=(const Color1I& color);
	Color1I& operator*=(const Color1I& color);
	Color1I& operator*=(float scalar);
	Color1I& operator*=(byte scalar);
	Color1I& operator/=(const Color1I& color);
	Color1I& operator/=(float scalar);
	Color1I& operator/=(byte scalar);
};

Color1I operator*(float scalar, const Color1I& color);
Color1I operator*(Color1I::byte scalar, const Color1I& color);
Color1I operator/(float scalar, const Color1I& color);
Color1I operator/(Color1I::byte scalar, const Color1I& color);
std::ostream& operator<<(std::ostream& out, const Color1I& color);
