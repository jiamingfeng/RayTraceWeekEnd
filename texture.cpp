#include "texture.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "Perlin.h"
#include "renderContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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

NoiseTexture::NoiseTexture(RenderContext& context, 
	bool r,
	const Vec3 &c,
	float sc)
	: random(r), color(c), scale(sc)
{
	if (random)
	{
		color = Vec3(context.rand.rSample() * context.rand.rSample(), 
					context.rand.rSample() * context.rand.rSample(), 
					context.rand.rSample() * context.rand.rSample() );
		scale = context.rand.rSample() * 4.f + 5.f;
	}
}

Vec3 NoiseTexture::value(float u, float v, const Vec3 &p) const
{
	float perlinNoise = Perlin::noise(scale * p);
	return color * perlinNoise;
}


Vec3 TurbulenceTexture::value(float u, float v, const Vec3 &p) const
{
	float perlinNoise = Perlin::turb(scale * p);
	return color * 0.5f * (1.f + perlinNoise);
}

Vec3 SineNoiseTexture::value(float u, float v, const Vec3 &p) const
{
	float perlinNoise = Perlin::turb(scale * p);
	return color * 0.5f * (1.f + sin(scale * p.z() + 10.f * Perlin::turb(p)));
}

ImageTexture::ImageTexture(const std::string& imagePath)
{
	int numComponents;
	data = stbi_load(imagePath.c_str(), &width, &height, &numComponents, 0);
}

ImageTexture::~ImageTexture()
{
	STBI_FREE(data);
}

Vec3 ImageTexture::value(float u, float v, const Vec3 &p) const
{
	int i = int(u * width);
	int j = int((1.f - v) * height);//?

	i = std::clamp(i, 0, width - 1);
	j = std::clamp(j, 0, height - 1);

	float r = int(data[3 * (width * j + i) + 0]) / 255.0f;
	float g = int(data[3 * (width * j + i) + 1]) / 255.0f;
	float b = int(data[3 * (width * j + i) + 2]) / 255.0f;

	return Vec3(r, g, b);
}