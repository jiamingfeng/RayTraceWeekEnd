#pragma once

#include "vec3.h"
#include "renderContext.h"

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
	Lambert(const Vec3& albedo, RenderContext& context) : 
		albedo(albedo), Material(context) 
	{}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

private:
	Vec3 albedo;
};

class Metal : public Material
{
public:
	Metal(const Vec3& albedo, RenderContext& context) :
		albedo(albedo), Material(context)
	{}
	virtual bool Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override;

private:
	Vec3 albedo;
};



