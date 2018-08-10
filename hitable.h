#pragma once

#include "ray.h"

#include <vector>
#include <algorithm>

struct HitRecord 
{
	HitRecord() = default;
	~HitRecord() = default;
	Vec3 p;
	Vec3 normal;
	float t;
};

class Hitable {
public:
	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;
};

class HitableList : public Hitable {
public:
	HitableList() = default;
	~HitableList() = default;

	std::vector<Hitable*> list;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
};

bool HitableList::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
	
	HitRecord tempRec;
	bool hitAnygthing = false;
	float closestSoFar = tMax;

	// test the ray for each hitable items in list and return the nearest hit
	std::for_each(list.begin(), list.end(), [&r, &rec, &tempRec, &hitAnygthing, &closestSoFar, tMin](Hitable* h)
	{
		
		if (h->hit(r, tMin, closestSoFar, tempRec))
		{
			hitAnygthing = true;
			closestSoFar = tempRec.t;
			rec = tempRec;
		}
	});

	return hitAnygthing;
}