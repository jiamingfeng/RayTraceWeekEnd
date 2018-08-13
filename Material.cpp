#include "material.h"

#include "hitable.h"

static Vec3 reflect(const Vec3 &v, const Vec3 &normal)
{
	return v - 2 * dot(v, normal) * normal;
}

// n * sin(theta )  = n' * sin ( theta ' )
// convert sin to cos and then use dot product to get the refracted ray direction
static bool refract(const Vec3 &v, const Vec3 &normal, float ni_over_nt, Vec3& refracted)
{
	Vec3 unitV = unit_vector(v);
	float dt = dot(unitV, normal);

	// cos(thetaR ) ^ 2 = 1 - (ni/nt)^2 * ( 1 - cos(thetaI)^2 )
	float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (unitV - normal * dt) - normal * sqrtf(discriminant);
		return true;
	}

	return false;
}

float schlick(float cosine, float reflectionIndex)
{
	float r0 = (1.0f - reflectionIndex) / (1.0f + reflectionIndex);
	r0 = r0 * r0;
	return r0 + (1 - r0) * powf((1 - cosine), 5.f);
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
	scattered = Ray(rec.p, target - rec.p, rIn.Time());
	attenuation = albedo;

	return true;
}

bool Metal::Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
{
	Vec3 reflected = reflect(unit_vector(rIn.Direction()), rec.normal);
	scattered = Ray(rec.p, reflected + roughness * RandomSampleInUnitSphere(contextPtr->rand), rIn.Time());
	attenuation = albedo;

	return dot( scattered.Direction(), rec.normal) > 0;
}

bool Dielectric::Scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const
{
	Vec3 outNormal;
	Vec3 reflected = reflect(rIn.Direction(), rec.normal);

	float ni_over_nt;
	attenuation = Vec3(1.f, 1.f, 1.f);

	Vec3 refracted;

	float reflect_prob;
	float cosine;

	// light goes through the same side as the normal
	float dotRnNormal = dot(rIn.Direction(), rec.normal);
	if (dotRnNormal > 0)
	{
		outNormal = -rec.normal;
		ni_over_nt = reflectIndex;
		cosine = reflectIndex * dotRnNormal / rIn.Direction().length();
	}
	else
	{
		outNormal = rec.normal;
		ni_over_nt = 1.0f / reflectIndex;
		cosine =  - dotRnNormal / rIn.Direction().length();
	}

	if (refract(rIn.Direction(), outNormal, ni_over_nt, refracted))
	{
		reflect_prob = schlick(cosine, reflectIndex);
	}
	else
	{
		reflect_prob = 1.f;
	}

	if( contextPtr->rand.rSample() < reflect_prob)
	{
		scattered = Ray(rec.p, reflected, rIn.Time());
	}
	else
	{
		scattered = Ray(rec.p, refracted, rIn.Time());
	}

	return true;
}