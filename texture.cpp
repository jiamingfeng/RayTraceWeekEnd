#include "texture.h"

#define _USE_MATH_DEFINES
#include <math.h>


Vec3 ConstantTexture::value(float u, float v, const Vec3 &p) const
{
	return color;
}

Vec3 CheckerTexture::value(float u, float v, const Vec3 &p) const
{
	float sines = sinf(10.f * p.x()) * sinf(10.f * p.y()) * sinf(10.f * p.z());

	if (sines < 0)
	{
		return odd->value(u, v, p);
	}
	else
	{
		return even->value(u, v, p);
	}
	
}
