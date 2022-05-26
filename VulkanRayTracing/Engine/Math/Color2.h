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

struct Color2
{
	float R, G;

	constexpr Color2() : R(1), G(1) {}
	constexpr Color2(float r, float g) : R(r), G(g) {}
	constexpr Color2(double r, double g) : R((float)r), G((float)g) {}
	constexpr Color2(unsigned char r, unsigned char g)
	{
		R = (float)r / 255;
		G = (float)g / 255;
	}
	constexpr Color2(unsigned int color)
	{
		R = float((color >> 24) & 0xFF) / 255.0f;
		G = float((color >> 16) & 0xFF) / 255.0f;
	}

	Color2(const Color1& color, float g = 0);
	Color2(const Color1I& color, float g = 0);
	Color2(const Color2& color);
	Color2(const Color2I& color);
	Color2(const Color3& color);
	Color2(const Color3I& color);
	Color2(const Color4& color);
	Color2(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color2(const Vector2SType<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2(const Vector2Type<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2(const Vector3Type<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)) {}

	template <typename Number, typename DistanceType>
	constexpr Color2(const Vector3SType<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)) {}

	Color2& Set(float r = 1, float g = 1);
	Color2 Lerp(const Color2& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color2& color) const;
	bool operator!=(const Color2& color) const;

	Color2 operator+(const Color2& other) const;
	Color2 operator-(const Color2& other) const;
	Color2 operator-() const;
	Color2 operator*(const Color2& other) const;
	Color2 operator*(float scalar) const;
	Color2 operator/(const Color2& other) const;
	Color2 operator/(float scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color2& operator=(const Color2& other);
	Color2& operator+=(const Color2& color);
	Color2& operator-=(const Color2& color);
	Color2& operator*=(const Color2& color);
	Color2& operator*=(float scalar);
	Color2& operator/=(const Color2& color);
	Color2& operator/=(float scalar);

private:
	bool Compare(float a, float b, float epsilon = 1e-4f) const;
};

Color2 operator*(float scalar, const Color2& color);
Color2 operator/(float scalar, const Color2& color);
std::ostream& operator<<(std::ostream& out, const Color2& color);
