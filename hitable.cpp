#include "hitable.h"
#include "material.h"

Hitable::~Hitable()
{
}

AABB Hitable::MergeBBoxes(const AABB& box0, const AABB& box1)
{
	Vec3 bboxMin(std::fminf(box0.Min().x(), box1.Min().x()),
		std::fminf(box0.Min().y(), box1.Min().y()),
		std::fminf(box0.Min().z(), box1.Min().z()));

	Vec3 bboxMax(std::fmaxf(box0.Max().x(), box1.Max().x()),
		std::fmaxf(box0.Max().y(), box1.Max().y()),
		std::fmaxf(box0.Max().z(), box1.Max().z()));

	return AABB(bboxMin, bboxMax);
}

HitableList::~HitableList()
{
	//std::for_each(list.begin(), list.end(), [](Hitable* h)
	//{
	//	delete h;
	//});

	list.clear();
}

bool HitableList::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const 
{

	HitRecord tempRec;
	bool hitAnygthing = false;
	float closestSoFar = tMax;

	// test the ray for each hitable items in list and return the nearest hit
	std::for_each(list.begin(), list.end(), [&r, &rec, &tempRec, &hitAnygthing, &closestSoFar, tMin](HitableSP h)
	{
		if (!h)
		{
			return;
		}

		if (h->hit(r, tMin, closestSoFar, tempRec))
		{
			hitAnygthing = true;
			closestSoFar = tempRec.t;
			rec = tempRec;
		}
	});

	return hitAnygthing;
}

bool HitableList::bbox(float t0, float t1, AABB& box) const
{
	if (list.size() == 0)
	{
		return false;
	}

	// merge all bbox in this hitable list
	AABB eachBBox;
	bool result = list[0]->bbox(t0, t1, eachBBox);
	if (!result)
	{
		return false;
	}

	box = eachBBox;
	auto iter = std::find_if_not(list.begin(), list.end(), [t0, t1, &eachBBox, &box](HitableSP h) {

		if (!h)
		{
			return false;
		}		

		bool hasBBox = h->bbox(t0, t1, eachBBox);
		box = Hitable::MergeBBoxes(box, eachBBox);
		return hasBBox;
	});

	// failed at iterating through all hitables
	if (iter != list.end())
	{
		return false;
	}

	return true;

}