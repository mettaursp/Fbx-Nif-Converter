#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color3I::Color3I(const Color1& color, byte g, byte b)
{
	R = (byte)(color.R * 255);
	G = g;
	B = b;
}

Color3I::Color3I(const Color1I& color, byte g, byte b)
{
	R = color.R;
	G = g;
	B = b;
}

Color3I::Color3I(const Color2& color, byte b)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = b;
}

Color3I::Color3I(const Color2I& color, byte b)
{
	R = color.R;
	G = color.G;
	B = b;
}

Color3I::Color3I(const Color3& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = (byte)(color.B * 255);
}

Color3I::Color3I(const Color3I& color)
{
	R = color.R;
	G = color.G;
	B = color.B;
}

Color3I::Color3I(const Color4& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
	B = (byte)(color.B * 255);
}

Color3I::Color3I(const Color4I& color)
{
	R = color.R;
	G = color.G;
	B = color.B;
}

Color3I& Color3I::Set(byte r, byte g, byte b)
{
	R = r;
	G = g;
	B = b;

	return *this;
}

Color3I Color3I::Lerp(const Color3I& end, float t) const
{
	return Color3I(
		(1 - t) * (float)R + t * (float)end.R,
		(1 - t) * (float)G + t * (float)end.G,
		(1 - t) * (float)B + t * (float)end.B
	);
}

unsigned int Color3I::ARGB() const
{
	return (
		(int(R) << 16) +
		(int(G) << 8) +
		int(B * 255)
	);
}

unsigned int Color3I::ABGR() const
{
	return (
		(int(B) << 16) +
		(int(G) << 8) +
		int(R)
	);
}

Color3I Color3I::operator+(const Color3I& other) const
{
	return Color3I((byte)(R + other.R), (byte)(G + other.G), (byte)(B + other.B));
}

Color3I Color3I::operator-(const Color3I& other) const
{
	return Color3I((byte)(R - other.R), (byte)(G - other.G), (byte)(B - other.B));
}

Color3I Color3I::operator*(const Color3I& other) const
{
	return Color3I((byte)(R * other.R), (byte)(G * other.G), (byte)(B * other.B));
}

Color3I Color3I::operator*(float scalar) const
{
	return Color3I((byte)(scalar * (float)R), (byte)(scalar * (float)G), (byte)(scalar * (float)B));
}

Color3I Color3I::operator*(byte scalar) const
{
	return Color3I((byte)(scalar * R), (byte)(scalar * G), (byte)(scalar * B));
}

Color3I Color3I::operator/(const Color3I& other) const
{
	return Color3I((byte)(R / std::max(other.R, (byte)1)), (byte)(G / std::max(other.G, (byte)1)), (byte)(B / std::max(other.B, (byte)1)));
}

Color3I Color3I::operator/(float scalar) const
{
	return Color3I((byte)((float)R / scalar), (byte)((float)G / scalar), (byte)((float)B / scalar));
}

Color3I Color3I::operator/(byte scalar) const
{
	scalar = std::max((byte)1, scalar);

	return Color3I((byte)(R / scalar), (byte)(G / scalar), (byte)(B / scalar));
}

bool Color3I::operator==(const Color3I& color) const
{
	return (R == color.R) && (G == color.G) && (B == color.B);
}

bool Color3I::operator!=(const Color3I& color) const
{
	return !(*this == color);
}

Color3I::operator unsigned int() const
{
	return (
		(int(R) << 24) +
		(int(G) << 16) +
		(int(B) << 8)
		);
}

Color3I::operator Vector3() const
{
	return Vector3((Float)R, (Float)G, (Float)B);
}

Color3I& Color3I::operator=(const Color3I& other)
{
	Set(other.R, other.G, other.B);

	return *this;
}

Color3I::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color3I& Color3I::operator+=(const Color3I& color)
{
	return *this = *this + color;
}

Color3I& Color3I::operator-=(const Color3I& color)
{
	return *this = *this - color;
}

Color3I& Color3I::operator*=(const Color3I& color)
{
	return *this = *this * color;
}

Color3I& Color3I::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color3I& Color3I::operator*=(byte scalar)
{
	return *this = *this * scalar;
}

Color3I& Color3I::operator/=(const Color3I& color)
{
	return *this = *this / color;
}

Color3I& Color3I::operator/=(float scalar)
{
	return *this = *this / scalar;
}

Color3I& Color3I::operator/=(byte scalar)
{
	return *this = *this / scalar;
}

Color3I operator*(float scalar, const Color3I& color)
{
	return color * scalar;
}

Color3I operator*(Color3I::byte scalar, const Color3I& color)
{
	return color * scalar;
}

Color3I operator/(float scalar, const Color3I& color)
{
	return Color3I((Color3I::byte)(scalar / (float)color.R), (Color3I::byte)(scalar / (float)color.G), (Color3I::byte)(scalar / (float)color.B));
}

Color3I operator/(Color3I::byte scalar, const Color3I& color)
{
	typedef Color3I::byte byte;

	return Color3I((byte)(scalar / std::max((byte)1, color.R)), (byte)(scalar / std::max((byte)1, color.G)), (byte)(scalar / std::max((byte)1, color.B)));
}

std::ostream& operator<<(std::ostream& out, const Color3I& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << ", B: " << color.B << " }";
}