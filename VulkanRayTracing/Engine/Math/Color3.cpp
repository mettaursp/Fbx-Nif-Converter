#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color3::Color3(const Color1& color, float g, float b)
{
	R = color.R;
	G = g;
	B = b;
}

Color3::Color3(const Color1I& color, float g, float b)
{
	R = (float)color.R / 255;
	G = g;
	B = b;
}

Color3::Color3(const Color2& color, float b)
{
	R = color.R;
	G = color.G;
	B = b;
}

Color3::Color3(const Color2I& color, float b)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = b;
}

Color3::Color3(const Color3& color)
{
	R = color.R;
	G = color.G;
	B = color.B;
}

Color3::Color3(const Color3I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = (float)color.B / 255;
}

Color3::Color3(const Color4& color)
{
	R = color.R;
	G = color.G;
	B = color.B;
}

Color3::Color3(const Color4I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = (float)color.B / 255;
}

Color3& Color3::Set(float r, float g, float b)
{
	R = r;
	G = g;
	B = b;

	return *this;
}

Color3 Color3::Lerp(const Color3& end, float t) const
{
	return Color3(
		(1 - t) * R + t * end.R,
		(1 - t) * G + t * end.G,
		(1 - t) * B + t * end.B
	);
}

unsigned int Color3::ARGB() const
{
	return (
		(int(R * 255) << 16) +
		(int(G * 255) << 8) +
		int(B * 255)
	);
}

unsigned int Color3::ABGR() const
{
	return (
		(int(B * 255) % 256 << 16) +
		(int(G * 255) % 256 << 8) +
		int(R * 255) % 256
	);
}

Color3 Color3::operator+(const Color3& other) const
{
	return Color3(R + other.R, G + other.G, B + other.B);
}

Color3 Color3::operator-(const Color3& other) const
{
	return Color3(R - other.R, G - other.G, B - other.B);
}

Color3 Color3::operator-() const
{
	return Color3(-R, -G, -B);
}

Color3 Color3::operator*(const Color3& other) const
{
	return Color3(R * other.R, G * other.G, B * other.B);
}

Color3 Color3::operator*(float scalar) const
{
	return Color3(scalar * R, scalar * G, scalar * B);
}

Color3 Color3::operator/(const Color3& other) const
{
	return Color3(R / other.R, G / other.G, B / other.B);
}

Color3 Color3::operator/(float scalar) const
{
	return Color3(R / scalar, G / scalar, B / scalar);
}

bool Color3::operator==(const Color3& color) const
{
	return Compare(R, color.R) && Compare(G, color.G) && Compare(B, color.B);
}

bool Color3::operator!=(const Color3& color) const
{
	return !(*this == color);
}

Color3::operator unsigned int() const
{
	return (
		(int(R * 255) << 24) +
		(int(G * 255) << 16) +
		(int(B * 255) << 8)
	);
}

Color3::operator Vector3() const
{
	return Vector3((Float)R, (Float)G, (Float)B);
}

Color3& Color3::operator=(const Color3& other)
{
	Set(other.R, other.G, other.B);

	return *this;
}

Color3::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color3& Color3::operator+=(const Color3& color)
{
	return *this = *this + color;
}

Color3& Color3::operator-=(const Color3& color)
{
	return *this = *this - color;
}

Color3& Color3::operator*=(const Color3& color)
{
	return *this = *this * color;
}

Color3& Color3::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color3& Color3::operator/=(const Color3& color)
{
	return *this = *this / color;
}

Color3& Color3::operator/=(float scalar)
{
	return *this = *this / scalar;
}

bool Color3::Compare(float a, float b, float epsilon) const
{
	return std::abs(a - b) < epsilon;
}

Color3 operator*(float scalar, const Color3& color)
{
	return color * scalar;
}

Color3 operator/(float scalar, const Color3& color)
{
	return Color3(scalar / color.R, scalar / color.G, scalar / color.B);
}

std::ostream& operator<<(std::ostream& out, const Color3& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << ", B: " << color.B << " }";
}