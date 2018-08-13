#pragma once

#include "texture.h"
#include "renderContext.h"

#include <algorithm>

class Ray;
struct HitRecord;

class Material
{
public:
	Material( RenderContext& context) : contextPtr(&context) {}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const = 0;

protected:
	RenderContext* contextPtr;
};

class Lambert : public Material
{
public:
	Lambert(const Texture& albedo, RenderContext& context) : 
		albedoTexture(&albedo), Material(context)
	{}
	Lambert(const Vec3& albedo, RenderContext& context) :
		albedo(albedo), Material(context)
	{}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

private:
	const Texture *albedoTexture = nullptr;
	Vec3 albedo;
};

class Metal : public Material
{
public:
	Metal(const Vec3& albedo, float roughness, RenderContext& context) :
		albedo(albedo), roughness( std::clamp( roughness, 0.f, 1.0f) ), Material(context)
	{}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

private:
	Vec3 albedo;
	float roughness; // 0 ~ 1
};

class Dielectric : public Material
{
public:
	Dielectric(float reflectIndex, RenderContext& context) :
		reflectIndex(reflectIndex),
		Material(context) {}

	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

	float reflectIndex;
};



