#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color4I::Color4I(const Color1& color, byte g, byte b, byte a)
{
	R = (byte)(color.R * 255);
	G = g;
	B = b;
	A = a;
}

Color4I::Color4I(const Color1I& color, byte g, byte b, byte a)
{
	R = color.R;
	G = g;
	B = b;
	A = a;
}

Color4I::Color4I(const Color2& color, byte b, byte a)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = b;
	A = a;
}

Color4I::Color4I(const Color2I& color, byte b, byte a)
{
	R = color.R;
	G = color.G;
	B = b;
	A = a;
}

Color4I::Color4I(const Color3& color, byte a)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = (byte)(color.B * 255);
	A = a;
}

Color4I::Color4I(const Color3I& color, byte a)
{
	R = color.R;
	G = color.G;
	B = color.B;
	A = a;
}

Color4I::Color4I(const Color4& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = (byte)(color.B * 255);
	A = (byte)(color.A * 255);
}

Color4I::Color4I(const Color4I& color)
{
	R = color.R;
	G = color.G;
	B = color.B;
	A = color.A;
}

Color4I& Color4I::Set(byte r, byte g, byte b, byte a)
{
	R = r;
	G = g;
	B = b;
	A = a;

	return *this;
}

Color4I Color4I::Lerp(const Color4I& end, float t) const
{
	return Color4I(
		(1 - t) * (float)R + t * (float)end.R,
		(1 - t) * (float)G + t * (float)end.G,
		(1 - t) * (float)B + t * (float)end.B,
		(1 - t) * (float)A + t * (float)end.A
	);
}

unsigned int Color4I::ARGB() const
{
	return (
		(int(A) << 24) +
		(int(R) << 16) +
		(int(G) << 8) +
		int(B * 255)
		);
}

unsigned int Color4I::ABGR() const
{
	return (
		(int(A) << 24) +
		(int(B) << 16) +
		(int(G) << 8) +
		int(R)
	);
}

Color4I Color4I::operator+(const Color4I& other) const
{
	return Color4I((byte)(R + other.R), (byte)(G + other.G), (byte)(B + other.B), (byte)(A + other.A));
}

Color4I Color4I::operator-(const Color4I& other) const
{
	return Color4I((byte)(R - other.R), (byte)(G - other.G), (byte)(B - other.B), (byte)(A - other.A));
}

Color4I Color4I::operator*(const Color4I& other) const
{
	return Color4I((byte)(R * other.R), (byte)(G * other.G), (byte)(B * other.B), (byte)(A * other.A));
}

Color4I Color4I::operator*(float scalar) const
{
	return Color4I((byte)(scalar * (float)R), (byte)(scalar * (float)G), (byte)(scalar * (float)B), (byte)(scalar * (float)A));
}

Color4I Color4I::operator*(byte scalar) const
{
	return Color4I((byte)(scalar * R), (byte)(scalar * G), (byte)(scalar * B), (byte)(scalar * A));
}

Color4I Color4I::operator/(const Color4I& other) const
{
	return Color4I((byte)(R / std::max(other.R, (byte)1)), (byte)(G / std::max(other.G, (byte)1)), (byte)(B / std::max(other.B, (byte)1)), (byte)(A / std::max(other.A, (byte)1)));
}

Color4I Color4I::operator/(float scalar) const
{
	return Color4I((byte)((float)R / scalar), (byte)((float)G / scalar), (byte)((float)B / scalar), (byte)((float)A / scalar));
}

Color4I Color4I::operator/(byte scalar) const
{
	scalar = std::max((byte)1, scalar);

	return Color4I((byte)(R / scalar), (byte)(G / scalar), (byte)(B / scalar), (byte)(A / scalar));
}

bool Color4I::operator==(const Color4I& color) const
{
	return (R == color.R) && (G == color.G) && (B == color.B) && (A == color.A);
}

bool Color4I::operator!=(const Color4I& color) const
{
	return !(*this == color);
}

Color4I::operator unsigned int() const
{
	return (
		(int(R) << 24) +
		(int(G) << 16) +
		(int(B) << 8) +
		int(A)
	);
}

Color4I::operator Vector3() const
{
	return Vector3((Float)R, (Float)G, (Float)B, (Float)A);
}

Color4I& Color4I::operator=(const Color4I& other)
{
	Set(other.R, other.G, other.B, other.A);

	return *this;
}

Color4I::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color4I& Color4I::operator+=(const Color4I& color)
{
	return *this = *this + color;
}

Color4I& Color4I::operator-=(const Color4I& color)
{
	return *this = *this - color;
}

Color4I& Color4I::operator*=(const Color4I& color)
{
	return *this = *this * color;
}

Color4I& Color4I::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color4I& Color4I::operator*=(byte scalar)
{
	return *this = *this * scalar;
}

Color4I& Color4I::operator/=(const Color4I& color)
{
	return *this = *this / color;
}

Color4I& Color4I::operator/=(float scalar)
{
	return *this = *this / scalar;
}

Color4I& Color4I::operator/=(byte scalar)
{
	return *this = *this / scalar;
}

Color4I operator*(float scalar, const Color4I& color)
{
	return color * scalar;
}

Color4I operator*(Color4I::byte scalar, const Color4I& color)
{
	return color * scalar;
}

Color4I operator/(float scalar, const Color4I& color)
{
	return Color4I((Color4I::byte)(scalar / (float)color.R), (Color4I::byte)(scalar / (float)color.G), (Color4I::byte)(scalar / (float)color.B), (Color4I::byte)(scalar / (float)color.A));
}

Color4I operator/(Color4I::byte scalar, const Color4I& color)
{
	typedef Color4I::byte byte;

	return Color4I((byte)(scalar / std::max((byte)1, color.R)), (byte)(scalar / std::max((byte)1, color.G)), (byte)(scalar / std::max((byte)1, color.B)), (byte)(scalar / std::max((byte)1, color.A)));
}

std::ostream& operator<<(std::ostream& out, const Color4I& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << ", B: " << color.B << ", A: " << color.A << " }";
}