#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color4::Color4(const Color1& color, float g, float b, float a)
{
	R = color.R;
	G = g;
	B = b;
	A = a;
}

Color4::Color4(const Color1I& color, float g, float b, float a)
{
	R = (float)color.R / 255;
	G = g;
	B = b;
	A = a;
}

Color4::Color4(const Color2& color, float b, float a)
{
	R = color.R;
	G = color.G;
	B = b;
	A = a;
}

Color4::Color4(const Color2I& color, float b, float a)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = b;
	A = a;
}

Color4::Color4(const Color3& color, float a)
{
	R = color.R;
	G = color.G;
	B = color.B;
	A = a;
}

Color4::Color4(const Color3I& color, float a)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = (float)color.B / 255;
	A = a;
}

Color4::Color4(const Color4I& color)
{
	R = (float)color.R / 255;
	G = (float)color.G / 255;
	B = (float)color.B / 255;
	A = (float)color.A / 255;
}

Color4& Color4::Set(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;

	return *this;
}

Color4 Color4::Lerp(const Color4& end, float t) const
{
	return Color4(
		(1 - t) * R + t * end.R,
		(1 - t) * G + t * end.G,
		(1 - t) * B + t * end.B,
		(1 - t) * A + t * end.A
	);
}

unsigned int Color4::ARGB() const
{
	return (
		(int(A * 255) << 24) +
		(int(R * 255) << 16) +
		(int(G * 255) << 8) +
		int(B * 255)
		);
}

unsigned int Color4::ABGR() const
{
	return (
		(int(A * 255) % 256 << 24) +
		(int(B * 255) % 256 << 16) +
		(int(G * 255) % 256 << 8) +
		int(R * 255) % 256
		);
}

Color4 Color4::operator+(const Color4& other) const
{
	return Color4(R + other.R, G + other.G, B + other.B, A + other.A);
}

Color4 Color4::operator-(const Color4& other) const
{
	return Color4(R - other.R, G - other.G, B - other.B, A - other.A);
}

Color4 Color4::operator-() const
{
	return Color4(-R, -G, -B, -A);
}

Color4 Color4::operator*(const Color4& other) const
{
	return Color4(R * other.R, G * other.G, B * other.B, A * other.A);
}

Color4 Color4::operator*(float scalar) const
{
	return Color4(scalar * R, scalar * G, scalar * B, scalar * A);
}

Color4 Color4::operator/(const Color4& other) const
{
	return Color4(R / other.R, G / other.G, B / other.B, A / other.A);
}

Color4 Color4::operator/(float scalar) const
{
	return Color4(R / scalar, G / scalar, B / scalar, A / scalar);
}

bool Color4::operator==(const Color4& color) const
{
	return Compare(R, color.R) && Compare(G, color.G) && Compare(B, color.B) && Compare(A, color.A);
}

bool Color4::operator!=(const Color4& color) const
{
	return !(*this == color);
}

Color4::operator unsigned int() const
{
	return (
		(int(R * 255) << 24) +
		(int(G * 255) << 16) +
		(int(B * 255) << 8) +
		int(A * 255)
		);
}

Color4::operator Vector3() const
{
	return Vector3((Float)R, (Float)G, (Float)B, (Float)A);
}

Color4& Color4::operator=(const Color4& other)
{
	Set(other.R, other.G, other.B, other.A);

	return *this;
}

Color4::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color4& Color4::operator+=(const Color4& color)
{
	return *this = *this + color;
}

Color4& Color4::operator-=(const Color4& color)
{
	return *this = *this - color;
}

Color4& Color4::operator*=(const Color4& color)
{
	return *this = *this * color;
}

Color4& Color4::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color4& Color4::operator/=(const Color4& color)
{
	return *this = *this / color;
}

Color4& Color4::operator/=(float scalar)
{
	return *this = *this / scalar;
}

bool Color4::Compare(float a, float b, float epsilon) const
{
	return std::abs(a - b) < epsilon;
}

Color4 operator*(float scalar, const Color4& color)
{
	return color * scalar;
}

Color4 operator/(float scalar, const Color4& color)
{
	return Color4(scalar / color.R, scalar / color.G, scalar / color.B, scalar / color.A);
}

std::ostream& operator<<(std::ostream& out, const Color4& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << ", B: " << color.B << ", A: " << color.A << " }";
}