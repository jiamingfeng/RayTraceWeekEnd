#include "material.h"

#include "hitable.h"

static Vec3 reflect(const Vec3 &v, const Vec3 &normal)
{
	return v - 2 * dot(v, normal) * normal;
}

static Vec3 RandomSampleInUnitSphere(Random &rand) 
{
	Vec3 p;
	do {
		//p = 2.0 * Vec3(rand.rDiffuse(), rand.rDiffuse(), rand.rDiffuse()) - Vec3( 1.f, 1.f, 1.f);
		p = Vec3(rand.rDiffuse(), rand.rDiffuse(), rand.rDiffuse());
	} while (p.squared_length() >= 1.0f);

	return p;
}

bool Lambert::Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
{
	Vec3 target = rec.p + rec.normal + RandomSampleInUnitSphere(contextPtr->rand);
	scattered = Ray(rec.p, target - rec.p);
	attenuation = albedo;

	return true;
}

bool Metal::Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
{
	Vec3 reflected = reflect(unit_vector(rIn.Direction()), rec.normal);
	scattered = Ray(rec.p, reflected);
	attenuation = albedo;

	return dot( scattered.Direction(), rec.normal) > 0;
}