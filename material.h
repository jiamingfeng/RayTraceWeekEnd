#pragma once

#include "texture.h"
#include "renderContext.h"

class Ray;
struct HitRecord;

class Material
{
public:
	Material( RenderContextSP context) : contextPtr(context) {}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const = 0;

protected:
	RenderContextWP contextPtr;
};

#define MaterialSP std::shared_ptr<Material>
#define MaterialWP std::weak_ptr<Material>

class Lambert : public Material
{
public:
	Lambert(TextureSP albedo, RenderContextSP context) :
		albedoTexture(albedo), Material(context)
	{}
	Lambert(const Vec3& albedo, RenderContextSP context) :
		albedo(albedo), Material(context)
	{}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

private:
	TextureWP albedoTexture;
	Vec3 albedo;
};

class Metal : public Material
{
public:
	Metal(const Vec3& albedo, float roughness, RenderContextSP context) :
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
	Dielectric(float reflectIndex, RenderContextSP context) :
		reflectIndex(reflectIndex),
		Material(context) {}

	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

	float reflectIndex;
};



