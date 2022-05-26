#include "Precision.h"

import <cmath>;

namespace std
{

	template <>
	Float min<float, double>(float a, double b)
	{
		return min<Float>((Float)a, (Float)b);
	}

	template <>
	Float min<double, float>(double a, float b)
	{
		return min<Float>((Float)a, (Float)b);
	}

	template <>
	Float max<float, double>(float a, double b)
	{
		return max<Float>((Float)a, (Float)b);
	}

	template <>
	Float max<double, float>(double a, float b)
	{
		return max<Float>((Float)a, (Float)b);
	}
}
