#include "hitable.h"
#include "material.h"

Hitable::~Hitable()
{ 
	delete mat; 
}

bool HitableList::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const 
{

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

HitableList::~HitableList()
{
	std::for_each(list.begin(), list.end(), [](Hitable* h)
	{
		delete h;
	});
}