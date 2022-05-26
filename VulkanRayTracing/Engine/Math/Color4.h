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

struct Color4
{
	float R, G, B, A;

	constexpr Color4() : R(1), G(1), B(1), A(1) {}
	constexpr Color4(float r, float g, float b, float a = 1) : R(r), G(g), B(b), A(a) {}
	constexpr Color4(double r, double g, double b, double a = 1) : R((float)r), G((float)g), B((float)b), A((float)a) {}
	constexpr Color4(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		R = (float)r / 255;
		G = (float)g / 255;
		B = (float)b / 255;
		A = (float)a / 255;
	}
	constexpr Color4(unsigned int color)
	{
		R = float((color >> 24) & 0xFF) / 255.0f;
		G = float((color >> 16) & 0xFF) / 255.0f;
		B = float((color >> 8) & 0xFF) / 255.0f;
		A = float(color & 0xFF) / 255.0f;
	}

	Color4(const Color1& color, float g = 0, float b = 0, float a = 1);
	Color4(const Color1I& color, float g = 0, float b = 0, float a = 1);
	Color4(const Color2& color, float b = 0, float a = 1);
	Color4(const Color2I& color, float b = 0, float a = 1);
	Color4(const Color3& color, float a = 1);
	Color4(const Color3I& color, float a = 1);
	constexpr Color4(const Color4& color) : R(color.R), G(color.G), B(color.B), A(color.A) {}
	Color4(const Color4I& color);

	template <typename Number, typename DistanceType>
	constexpr Color4(const Vector2SType<Number, DistanceType>& vector, float b = 0, float a = 1) : R(float(vector.X)), G(float(vector.Y)), B(b), A(a) {}

	template <typename Number, typename DistanceType>
	constexpr Color4(const Vector2Type<Number, DistanceType>& vector, float a = 1) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)), A(a) {}

	template <typename Number, typename DistanceType>
	constexpr Color4(const Vector3Type<Number, DistanceType>& vector) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)), A(float(vector.W)) {}

	template <typename Number, typename DistanceType>
	constexpr Color4(const Vector3SType<Number, DistanceType>& vector, float a = 1) : R(float(vector.X)), G(float(vector.Y)), B(float(vector.Z)), A(a) {}

	Color4& Set(float r = 1, float g = 1, float b = 1, float a = 1);
	Color4 Lerp(const Color4& end, float t) const;
	unsigned int ARGB() const;
	unsigned int ABGR() const;

	bool operator==(const Color4& color) const;
	bool operator!=(const Color4& color) const;

	Color4 operator+(const Color4& other) const;
	Color4 operator-(const Color4& other) const;
	Color4 operator-() const;
	Color4 operator*(const Color4& other) const;
	Color4 operator*(float scalar) const;
	Color4 operator/(const Color4& other) const;
	Color4 operator/(float scalar) const;

	operator unsigned int() const;
	operator Vector3() const;
	operator std::string() const;

	Color4& operator=(const Color4& other);
	Color4& operator+=(const Color4& color);
	Color4& operator-=(const Color4& color);
	Color4& operator*=(const Color4& color);
	Color4& operator*=(float scalar);
	Color4& operator/=(const Color4& color);
	Color4& operator/=(float scalar);

private:
	bool Compare(float a, float b, float epsilon = 1e-4f) const;
};

Color4 operator*(float scalar, const Color4& color);
Color4 operator/(float scalar, const Color4& color);
std::ostream& operator<<(std::ostream& out, const Color4& color);
