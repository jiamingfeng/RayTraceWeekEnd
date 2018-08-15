#pragma once
#include "vec3.h"

class Texture
{
public:
	virtual Vec3 value(float u, float v, const Vec3 &p) const = 0;
};

class ConstantTexture : public Texture
{
public:
	ConstantTexture() {};
	ConstantTexture(const Vec3& c) : color(c) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;
	Vec3 color;
};

class CheckerTexture : public Texture
{
public:
	CheckerTexture() {};
	CheckerTexture(const Texture& t0, const Texture &t1) : odd(&t0), even(&t1) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;


	const Texture* odd;
	const Texture* even;
};

struct RenderContext;

class NoiseTexture : public Texture
{
public:
	NoiseTexture(RenderContext &context, bool r = false,
		const Vec3 &c = Vec3(1.f, 1.f, 1.f),
		float sc = 1.f);
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;

protected:
	bool random;
	float scale;
	Vec3 color;
};

class TurbulenceTexture : public NoiseTexture
{
public:
	TurbulenceTexture(RenderContext &context, bool r = false, const Vec3 &c = Vec3(1.f, 1.f, 1.f), float sc = 1.f) :
		NoiseTexture(context, r, c, sc) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;
};

class SineNoiseTexture : public NoiseTexture
{
public:
	SineNoiseTexture(RenderContext &context, bool r = false, const Vec3 &c = Vec3(1.f, 1.f, 1.f), float sc = 1.f) :
		NoiseTexture(context, r, c, sc) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;
};

