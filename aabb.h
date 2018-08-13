#pragma once

#include "ray.h"

#include <algorithm>

class AABB
{
public:
	AABB() = default;
	AABB(const Vec3& a, const Vec3& b) :
		_min(a),
		_max(b) {}
	~AABB() = default;

	Vec3 Min() const { return _min; }
	Vec3 Max() const { return _max; }

	bool hit(const Ray& r, float tMin, float tMax) const;

private:
	Vec3 _min;
	Vec3 _max;
};