#pragma once
#include "hitable.h"

class BVH : public Hitable
{
public:
	BVH() = default;
	BVH(float t0, float t1);
	~BVH() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;

	std::vector<Hitable*> list;
private:
	Hitable *left;
	Hitable *right;

	AABB _bbox;
};

