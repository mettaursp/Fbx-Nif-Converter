#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color1I::Color1I(const Color1& color)
{
	R = (byte)(color.R * 255);
}

Color1I::Color1I(const Color1I& color)
{
	R = color.R;
}

Color1I::Color1I(const Color2& color)
{
	R = (byte)(color.R * 255);
}

Color1I::Color1I(const Color2I& color)
{
	R = color.R;
}

Color1I::Color1I(const Color3& color)
{
	R = (byte)(color.R * 255);
}

Color1I::Color1I(const Color3I& color)
{
	R = color.R;
}

Color1I::Color1I(const Color4& color)
{
	R = (byte)(color.R * 255);
}

Color1I::Color1I(const Color4I& color)
{
	R = color.R;
}

Color1I& Color1I::Set(byte r)
{
	R = r;

	return *this;
}

Color1I Color1I::Lerp(const Color1I& end, float t) const
{
	return Color1I(
		(1 - t) * (float)R + t * (float)end.R
	);
}

unsigned int Color1I::ARGB() const
{
	return (
		(int(R) << 16)
	);
}

unsigned int Color1I::ABGR() const
{
	return (
		int(R)
	);
}

Color1I Color1I::operator+(const Color1I& other) const
{
	return Color1I((byte)(R + other.R));
}

Color1I Color1I::operator-(const Color1I& other) const
{
	return Color1I((byte)(R - other.R));
}

Color1I Color1I::operator*(const Color1I& other) const
{
	return Color1I((byte)(R * other.R));
}

Color1I Color1I::operator*(float scalar) const
{
	return Color1I((byte)(scalar * (float)R));
}

Color1I Color1I::operator*(byte scalar) const
{
	return Color1I((byte)(scalar * R));
}

Color1I Color1I::operator/(const Color1I& other) const
{
	return Color1I((byte)(R / std::max(other.R, (byte)1)));
}

Color1I Color1I::operator/(float scalar) const
{
	return Color1I((byte)((float)R / scalar));
}

Color1I Color1I::operator/(byte scalar) const
{
	scalar = std::max((byte)1, scalar);

	return Color1I((byte)(R / scalar));
}

bool Color1I::operator==(const Color1I& color) const
{
	return (R == color.R);
}

bool Color1I::operator!=(const Color1I& color) const
{
	return !(*this == color);
}

Color1I::operator unsigned int() const
{
	return (
		(int(R) << 24)
	);
}

Color1I::operator Vector3() const
{
	return Vector3((Float)R);
}

Color1I& Color1I::operator=(const Color1I& other)
{
	Set(other.R);

	return *this;
}

Color1I::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color1I& Color1I::operator+=(const Color1I& color)
{
	return *this = *this + color;
}

Color1I& Color1I::operator-=(const Color1I& color)
{
	return *this = *this - color;
}

Color1I& Color1I::operator*=(const Color1I& color)
{
	return *this = *this * color;
}

Color1I& Color1I::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color1I& Color1I::operator*=(byte scalar)
{
	return *this = *this * scalar;
}

Color1I& Color1I::operator/=(const Color1I& color)
{
	return *this = *this / color;
}

Color1I& Color1I::operator/=(float scalar)
{
	return *this = *this / scalar;
}

Color1I& Color1I::operator/=(byte scalar)
{
	return *this = *this / scalar;
}

Color1I operator*(float scalar, const Color1I& color)
{
	return color * scalar;
}

Color1I operator*(Color1I::byte scalar, const Color1I& color)
{
	return color * scalar;
}

Color1I operator/(float scalar, const Color1I& color)
{
	return Color1I((Color1I::byte)(scalar / (float)color.R));
}

Color1I operator/(Color1I::byte scalar, const Color1I& color)
{
	typedef Color1I::byte byte;

	return Color1I((byte)(scalar / std::max((byte)1, color.R)));
}

std::ostream& operator<<(std::ostream& out, const Color1I& color)
{
	return out << "{ R: " << color.R << " }";
}