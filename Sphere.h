#pragma once
#include "hitable.h"

class Sphere :
	public Hitable
{
public:
	Sphere() = delete;
	Sphere(const Vec3& Center, float Radius) : center(Center), radius(Radius) {}
	~Sphere() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;

private:
	bool IsHitPointValid(float nearHitPoint, float tMin, float tMax, HitRecord & rec, const Ray & r) const;
	Vec3 center;
	float radius;
};

bool Sphere::IsHitPointValid(float hitPoint, float tMin, float tMax, HitRecord & rec, const Ray & r ) const
{
	if (hitPoint < tMax && hitPoint > tMin)
	{
		rec.t = hitPoint;
		rec.p = r.PointOnRay(hitPoint);
		rec.normal = (rec.p - center) / radius;
		return true;
	}
	return false;
}

bool Sphere::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const
{
	Vec3 oc = r.Origin() - center;
	float a = dot(r.Direction(), r.Direction());
	float b = dot(oc, r.Direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float nearHitPoint = (-b - sqrtf(b*b - a * c)) / a;
		bool retflag = IsHitPointValid(nearHitPoint, tMin, tMax, rec, r);
		if (retflag)
		{
			return true;
		}
		else{
			float farHitPoint = (-b + sqrtf(b*b - a * c)) / a;
			return IsHitPointValid(farHitPoint,  tMin, tMax, rec, r);
		}
	}
}