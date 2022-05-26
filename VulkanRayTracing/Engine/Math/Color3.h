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

struct Color3
{
	float R, G, B;

	constexpr Color3() : R(1), G(1), B(1) {}
	constexpr Color3(float r, float g, float b) : R(r), G(g), B(b) {}
	constexpr Color3(double r, double g, double b) : R((float)r), G((float)g), B((float)b) {}
	constexpr Color3(unsigned char r, unsigned char g, unsigned char b)
	{
		R = (float)r / 255;
		G = (float)g / 255;
		B = (float)b / 255;
	}
	constexpr Color3(unsigned int color)
	{
		R = float((color >> 24) & 0xFF) / 255.0f;
		G = float((color >> 16) & 0xFF) / 255.0f;
		B = float((color >> 8) & 0xFF) / 255.0f;
	}

	Color3(const Color1& color, float g = 0, float b = 0);
	Color3(const Color1I& color, float g = 0, float b = 0);
	Color3(const Color2& color, float b = 0);
	Color3(const Color2I& color, float b = 0);
	Color3(const Color3& color);
	Color3(const Color3I& color);
	Color3(const Color4& color);
	Color3(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color3(const Vector2SType<Number, DistanceType>& vector, float b = 0) : R(float(vector.X)), G(float(vector.Y)), B(b) {}

	template <typename Number, typename DistanceType>
	constexpr Color3(const Vector2Type<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)) {}

	template <typename Number, typename DistanceType>
	constexpr Color3(const Vector3Type<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)) {}

	template <typename Number, typename DistanceType>
	constexpr Color3(const Vector3SType<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)) {}

	Color3& Set(float r = 1, float g = 1, float b = 1);
	Color3 Lerp(const Color3& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color3& color) const;
	bool operator!=(const Color3& color) const;

	Color3 operator+(const Color3& other) const;
	Color3 operator-(const Color3& other) const;
	Color3 operator-() const;
	Color3 operator*(const Color3& other) const;
	Color3 operator*(float scalar) const;
	Color3 operator/(const Color3& other) const;
	Color3 operator/(float scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color3& operator=(const Color3& other);
	Color3& operator+=(const Color3& color);
	Color3& operator-=(const Color3& color);
	Color3& operator*=(const Color3& color);
	Color3& operator*=(float scalar);
	Color3& operator/=(const Color3& color);
	Color3& operator/=(float scalar);

private:
	bool Compare(float a, float b, float epsilon = 1e-4f) const;
};

Color3 operator*(float scalar, const Color3& color);
Color3 operator/(float scalar, const Color3& color);
std::ostream& operator<<(std::ostream& out, const Color3& color);
