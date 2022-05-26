#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color2::Color2(const Color1& color, float g)
{
	R = color.R;
	G = g;
}

Color2::Color2(const Color1I& color, float g)
{
	R = (float)color.R / 255;
	G = g;
}

Color2::Color2(const Color2& color)
{
	R = color.R;
	G = color.G;
}

Color2::Color2(const Color2I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
}

Color2::Color2(const Color3& color)
{
	R = color.R;
	G = color.G;
}

Color2::Color2(const Color3I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
}

Color2::Color2(const Color4& color)
{
	R = color.R;
	G = color.G;
}

Color2::Color2(const Color4I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
}

Color2& Color2::Set(float r, float g)
{
	R = r;
	G = g;

	return *this;
}

Color2 Color2::Lerp(const Color2& end, float t) const
{
	return Color2(
		(1 - t) * R + t * end.R,
		(1 - t) * G + t * end.G
	);
}

unsigned int Color2::ARGB() const
{
	return (
		(int(R * 255) << 16) +
		(int(G * 255) << 8)
	);
}

unsigned int Color2::ABGR() const
{
	return (
		(int(G * 255) % 256 << 8) +
		int(R * 255) % 256
	);
}

Color2 Color2::operator+(const Color2& other) const
{
	return Color2(R + other.R, G + other.G);
}

Color2 Color2::operator-(const Color2& other) const
{
	return Color2(R - other.R, G - other.G);
}

Color2 Color2::operator-() const
{
	return Color2(-R, -G);
}

Color2 Color2::operator*(const Color2& other) const
{
	return Color2(R * other.R, G * other.G);
}

Color2 Color2::operator*(float scalar) const
{
	return Color2(scalar * R, scalar * G);
}

Color2 Color2::operator/(const Color2& other) const
{
	return Color2(R / other.R, G / other.G);
}

Color2 Color2::operator/(float scalar) const
{
	return Color2(R / scalar, G / scalar);
}

bool Color2::operator==(const Color2& color) const
{
	return Compare(R, color.R) && Compare(G, color.G);
}

bool Color2::operator!=(const Color2& color) const
{
	return !(*this == color);
}

Color2::operator unsigned int() const
{
	return (
		(int(R * 255) << 24) +
		(int(G * 255) << 16)
		);
}

Color2::operator Vector3() const
{
	return Vector3((Float)R, (Float)G);
}

Color2& Color2::operator=(const Color2& other)
{
	Set(other.R, other.G);

	return *this;
}

Color2::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color2& Color2::operator+=(const Color2& color)
{
	return *this = *this + color;
}

Color2& Color2::operator-=(const Color2& color)
{
	return *this = *this - color;
}

Color2& Color2::operator*=(const Color2& color)
{
	return *this = *this * color;
}

Color2& Color2::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color2& Color2::operator/=(const Color2& color)
{
	return *this = *this / color;
}

Color2& Color2::operator/=(float scalar)
{
	return *this = *this / scalar;
}

bool Color2::Compare(float a, float b, float epsilon) const
{
	return std::abs(a - b) < epsilon;
}

Color2 operator*(float scalar, const Color2& color)
{
	return color * scalar;
}

Color2 operator/(float scalar, const Color2& color)
{
	return Color2(scalar / color.R, scalar / color.G);
}

std::ostream& operator<<(std::ostream& out, const Color2& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << " }";
}