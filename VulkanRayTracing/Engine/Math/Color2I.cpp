#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color2I::Color2I(const Color1& color, byte g)
{
	R = (byte)(color.R * 255);
	G = g;
}

Color2I::Color2I(const Color1I& color, byte g)
{
	R = color.R;
	G = g;
}

Color2I::Color2I(const Color2& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
}

Color2I::Color2I(const Color2I& color)
{
	R = color.R;
	G = color.G;
}

Color2I::Color2I(const Color3& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
}

Color2I::Color2I(const Color3I& color)
{
	R = color.R;
	G = color.G;
}

Color2I::Color2I(const Color4& color)
{
	R = (byte)(color.R * 255);
	G = (byte)(color.G * 255);
}

Color2I::Color2I(const Color4I& color)
{
	R = color.R;
	G = color.G;
}

Color2I& Color2I::Set(byte r, byte g)
{
	R = r;
	G = g;

	return *this;
}

Color2I Color2I::Lerp(const Color2I& end, float t) const
{
	return Color2I(
		(1 - t) * (float)R + t * (float)end.R,
		(1 - t) * (float)G + t * (float)end.G
	);
}

unsigned int Color2I::ARGB() const
{
	return (
		(int(R) << 16) +
		(int(G) << 8)
	);
}

unsigned int Color2I::ABGR() const
{
	return (
		(int(G) << 8) +
		int(R)
	);
}

Color2I Color2I::operator+(const Color2I& other) const
{
	return Color2I((byte)(R + other.R), (byte)(G + other.G));
}

Color2I Color2I::operator-(const Color2I& other) const
{
	return Color2I((byte)(R - other.R), (byte)(G - other.G));
}

Color2I Color2I::operator*(const Color2I& other) const
{
	return Color2I((byte)(R * other.R), (byte)(G * other.G));
}

Color2I Color2I::operator*(float scalar) const
{
	return Color2I((byte)(scalar * (float)R), (byte)(scalar * (float)G));
}

Color2I Color2I::operator*(byte scalar) const
{
	return Color2I((byte)(scalar * R), (byte)(scalar * G));
}

Color2I Color2I::operator/(const Color2I& other) const
{
	return Color2I((byte)(R / std::max(other.R, (byte)1)), (byte)(G / std::max(other.G, (byte)1)));
}

Color2I Color2I::operator/(float scalar) const
{
	return Color2I((byte)((float)R / scalar), (byte)((float)G / scalar));
}

Color2I Color2I::operator/(byte scalar) const
{
	scalar = std::max((byte)1, scalar);

	return Color2I((byte)(R / scalar), (byte)(G / scalar));
}

bool Color2I::operator==(const Color2I& color) const
{
	return (R == color.R) && (G == color.G);
}

bool Color2I::operator!=(const Color2I& color) const
{
	return !(*this == color);
}

Color2I::operator unsigned int() const
{
	return (
		(int(R) << 24) +
		(int(G) << 16)
		);
}

Color2I::operator Vector3() const
{
	return Vector3((Float)R, (Float)G);
}

Color2I& Color2I::operator=(const Color2I& other)
{
	Set(other.R, other.G);

	return *this;
}

Color2I::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color2I& Color2I::operator+=(const Color2I& color)
{
	return *this = *this + color;
}

Color2I& Color2I::operator-=(const Color2I& color)
{
	return *this = *this - color;
}

Color2I& Color2I::operator*=(const Color2I& color)
{
	return *this = *this * color;
}

Color2I& Color2I::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color2I& Color2I::operator*=(byte scalar)
{
	return *this = *this * scalar;
}

Color2I& Color2I::operator/=(const Color2I& color)
{
	return *this = *this / color;
}

Color2I& Color2I::operator/=(float scalar)
{
	return *this = *this / scalar;
}

Color2I& Color2I::operator/=(byte scalar)
{
	return *this = *this / scalar;
}

Color2I operator*(float scalar, const Color2I& color)
{
	return color * scalar;
}

Color2I operator*(Color2I::byte scalar, const Color2I& color)
{
	return color * scalar;
}

Color2I operator/(float scalar, const Color2I& color)
{
	return Color2I((Color2I::byte)(scalar / (float)color.R), (Color2I::byte)(scalar / (float)color.G));
}

Color2I operator/(Color2I::byte scalar, const Color2I& color)
{
	typedef Color2I::byte byte;

	return Color2I((byte)(scalar / std::max((byte)1, color.R)), (byte)(scalar / std::max((byte)1, color.G)));
}

std::ostream& operator<<(std::ostream& out, const Color2I& color)
{
	return out << "{ R: " << color.R << ", G: " << color.G << " }";
}