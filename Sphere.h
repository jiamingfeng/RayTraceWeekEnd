#pragma once
#include "hitable.h"

#define _USE_MATH_DEFINES
#include <math.h>

void getSphereUV(const Vec3& p, float& u, float &v)
{
	double phi = atan2(p.z(), p.x());
	double theta = asin(p.y());

	u = float(1.0 - (phi + M_PI) / (2.0 * M_PI));
	v = float((theta + M_PI_2) / M_PI);
}

class Sphere :
	public Hitable
{
public:
	Sphere() = delete;
	Sphere(const Vec3& Center, float Radius, Material* newMat) : center(Center), radius(Radius), Hitable(newMat) {}
	Sphere(const LinearTimeVec3& Center, float Radius, Material* newMat) : movableCenter(Center), radius(Radius), moveable(true), Hitable(newMat) {}
	~Sphere() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;

private:
	bool IsHitPointValid(float nearHitPoint, float tMin, float tMax, HitRecord & rec, const Ray & r) const;
	Vec3 centerAtTime(float t) const;

	Vec3 center;
	LinearTimeVec3 movableCenter;
	float radius;

	bool moveable = false;
};

bool Sphere::IsHitPointValid(float hitPoint, float tMin, float tMax, HitRecord & rec, const Ray & r ) const
{
	if (hitPoint < tMax && hitPoint > tMin)
	{
		rec.t = hitPoint;
		rec.p = r.PointOnRay(hitPoint);
		rec.normal = (rec.p - centerAtTime(r.Time())) / radius;
		getSphereUV((rec.p - center) / radius, rec.u, rec.v);
		rec.mat = mat;
		return true;
	}
	return false;
}

Vec3 Sphere::centerAtTime(float t) const
{
	if (moveable)
	{
		return movableCenter.v(t);
	}

	return center;
}

bool Sphere::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const
{
	Vec3 oc = r.Origin() - centerAtTime(r.Time());
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

	return false;
}

bool Sphere::bbox(float t0, float t1, AABB& box) const
{
	AABB box0(centerAtTime(t0) - Vec3(radius, radius, radius), 
		centerAtTime(t0) + Vec3(radius, radius, radius));

	if (!moveable)
	{
		box = box0;
		return true;
	}

	AABB box1(centerAtTime(t1) - Vec3(radius, radius, radius),
		centerAtTime(t1) + Vec3(radius, radius, radius));

	box = Hitable::MergeBBoxes(box0, box1);

	return true;
	
}