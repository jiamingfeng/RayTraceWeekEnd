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
	virtual Vec3 Emitted(float u, float v, const Vec3 &p) const { return Vec3(0, 0, 0); }
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

class DiffuseLight : public Material
{
public:
	DiffuseLight(const Texture& t, RenderContext& context, float i = 1.f) : emit(&t), intensity(i), Material(context) {}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override { return false; }
	virtual Vec3 Emitted(float u, float v, const Vec3 &p) const { return emit->value(u, v, p) * intensity; }
private:
	const Texture* emit;
	float intensity;
};

class ISOTropic : public Material
{
public:
	ISOTropic(const Texture &t, RenderContext& context) : albedo(&t), Material(context) {}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;
private:
	const Texture *albedo;
};



