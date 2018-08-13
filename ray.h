#pragma once

#include "vec3.h"

class Ray 
{
public:
	Ray() = default;
	~Ray() = default;
	Ray(const Vec3& origin, const Vec3& direction, float timeIn = 0.f)
		: origin(origin), direction(direction), time(timeIn) {}
	const Vec3 Direction() const { return direction; }
	const Vec3 Origin() const { return origin; }
	float Time() const { return time;  }
	const Vec3 PointOnRay(float t) const { return origin + direction * t; }

private:	
	Vec3 origin;
	Vec3 direction;
	float time;
};