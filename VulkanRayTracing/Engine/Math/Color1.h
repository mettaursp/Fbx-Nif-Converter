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

struct Color1
{
	float R;

	constexpr Color1() : R(1) {}
	constexpr Color1(float r) : R(r) {}
	constexpr Color1(double r) : R((float)r) {}
	constexpr Color1(unsigned char r)
	{
		R = (float)r / 255;
	}
	constexpr Color1(unsigned int color)
	{
		R = float((color >> 24) & 0xFF) / 255.0f;
	}

	Color1(const Color1& color);
	Color1(const Color1I& color);
	Color1(const Color2& color);
	Color1(const Color2I& color);
	Color1(const Color3& color);
	Color1(const Color3I& color);
	Color1(const Color4& color);
	Color1(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color1(const Vector2SType<Number, DistanceType>& vector) : R(float(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1(const Vector2Type<Number, DistanceType>& vector) : R(float(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1(const Vector3Type<Number, DistanceType>& vector) : R(float(vector.X)) {}

	template <typename Number, typename DistanceType>
	constexpr Color1(const Vector3SType<Number, DistanceType>& vector) : R(float(vector.X)) {}

	Color1& Set(float r = 1);
	Color1 Lerp(const Color1& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color1& color) const;
	bool operator!=(const Color1& color) const;

	Color1 operator+(const Color1& other) const;
	Color1 operator-(const Color1& other) const;
	Color1 operator-() const;
	Color1 operator*(const Color1& other) const;
	Color1 operator*(float scalar) const;
	Color1 operator/(const Color1& other) const;
	Color1 operator/(float scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color1& operator=(const Color1& other);
	Color1& operator+=(const Color1& color);
	Color1& operator-=(const Color1& color);
	Color1& operator*=(const Color1& color);
	Color1& operator*=(float scalar);
	Color1& operator/=(const Color1& color);
	Color1& operator/=(float scalar);

private:
	bool Compare(float a, float b, float epsilon = 1e-4f) const;
};

Color1 operator*(float scalar, const Color1& color);
Color1 operator/(float scalar, const Color1& color);
std::ostream& operator<<(std::ostream& out, const Color1& color);
