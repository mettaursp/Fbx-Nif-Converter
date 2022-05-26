#include "Color1.h"
#include "Color1I.h"
#include "Color2.h"
#include "Color2I.h"
#include "Color3.h"
#include "Color3I.h"
#include "Color4.h"
#include "Color4I.h"

import <sstream>;

Color1::Color1(const Color1& color)
{
	R = color.R;
}

Color1::Color1(const Color1I& color)
{
	R = (float)color.R / 255;
}

Color1::Color1(const Color2& color)
{
	R = color.R;
}

Color1::Color1(const Color2I& color)
{
	R = (float)color.R / 255;
}

Color1::Color1(const Color3& color)
{
	R = color.R;
}

Color1::Color1(const Color3I& color)
{
	R = (float)color.R / 255;
}

Color1::Color1(const Color4& color)
{
	R = color.R;
}

Color1::Color1(const Color4I& color)
{
	R = (float)color.R / 255;
}

Color1& Color1::Set(float r)
{
	R = r;

	return *this;
}

Color1 Color1::Lerp(const Color1& end, float t) const
{
	return Color1(
		(1 - t) * R + t * end.R
	);
}

unsigned int Color1::ARGB() const
{
	return (
		(int(R * 255) << 16)
	);
}

unsigned int Color1::ABGR() const
{
	return (
		int(R * 255) % 256
	);
}

Color1 Color1::operator+(const Color1& other) const
{
	return Color1(R + other.R);
}

Color1 Color1::operator-(const Color1& other) const
{
	return Color1(R - other.R);
}

Color1 Color1::operator-() const
{
	return Color1(-R);
}

Color1 Color1::operator*(const Color1& other) const
{
	return Color1(R * other.R);
}

Color1 Color1::operator*(float scalar) const
{
	return Color1(scalar * R);
}

Color1 Color1::operator/(const Color1& other) const
{
	return Color1(R / other.R);
}

Color1 Color1::operator/(float scalar) const
{
	return Color1(R / scalar);
}

bool Color1::operator==(const Color1& color) const
{
	return Compare(R, color.R);
}

bool Color1::operator!=(const Color1& color) const
{
	return !(*this == color);
}

Color1::operator unsigned int() const
{
	return (
		(int(R * 255) << 24)
	);
}

Color1::operator Vector3() const
{
	return Vector3((Float)R);
}

Color1& Color1::operator=(const Color1& other)
{
	Set(other.R);

	return *this;
}

Color1::operator std::string() const
{
	std::stringstream out;

	out << *this;

	return out.str();
}

Color1& Color1::operator+=(const Color1& color)
{
	return *this = *this + color;
}

Color1& Color1::operator-=(const Color1& color)
{
	return *this = *this - color;
}

Color1& Color1::operator*=(const Color1& color)
{
	return *this = *this * color;
}

Color1& Color1::operator*=(float scalar)
{
	return *this = *this * scalar;
}

Color1& Color1::operator/=(const Color1& color)
{
	return *this = *this / color;
}

Color1& Color1::operator/=(float scalar)
{
	return *this = *this / scalar;
}

bool Color1::Compare(float a, float b, float epsilon) const
{
	return std::abs(a - b) < epsilon;
}

Color1 operator*(float scalar, const Color1& color)
{
	return color * scalar;
}

Color1 operator/(float scalar, const Color1& color)
{
	return Color1(scalar / color.R);
}

std::ostream& operator<<(std::ostream& out, const Color1& color)
{
	return out << "{ R: " << color.R << " }";
}