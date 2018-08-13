#include "bvh.h"
#include <algorithm>

void BVH::SortByAxis(std::vector<Hitable*> &hitableList, int axis)
{
	std::sort(hitableList.begin(), hitableList.end(), [axis](Hitable *a, Hitable *b) {
		AABB bboxLeft, bboxRight;
		if (!a->bbox(0, 0, bboxLeft) ||
			!b->bbox(0, 0, bboxRight))
		{
			std::cerr << "No bbox is found in bvh constructor!!" << std::endl;
		}

		return (bboxLeft.Min()[axis] < bboxRight.Min()[axis]);
	});
}

BVH::BVH(std::vector<Hitable*> &hitList, float t0, float t1, RenderContext &context)
	: BVH(context)
{

	int listSize = int(hitList.size());
	if (listSize == 0)
	{
		return;
	}


	// sort them by any random axis ( hack it to sort only to x or z to avoid y axis )
	int axis = int(2 * contextPtr->rand.rSample());
	if (axis == 1)
		axis = 2;
	SortByAxis(hitList, axis);


	if (listSize == 1)
	{
		left = right = hitList[0];
	}
	else if (listSize == 2)
	{
		left = hitList[0];
		right = hitList[1];
	}
	else
	{
		std::vector<Hitable*> lHalf(hitList.begin(), hitList.begin() + listSize / 2);
		std::vector<Hitable*> rHalf(hitList.begin() + listSize / 2, hitList.end());
		left = new BVH(lHalf, t0, t1, context);
		right = new BVH(rHalf, t0, t1, context);
	}

	AABB bboxLeft, bboxRight;
	if (!left->bbox(t0, t1, bboxLeft) ||
		!right->bbox(t0, t1, bboxRight))
	{
		std::cerr << "No bbox is found in bvh constructor!!" << std::endl;
	}

	_bbox = Hitable::MergeBBoxes(bboxLeft, bboxRight);

}

bool BVH::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const
{
	if (_bbox.hit(r, tMin, tMax))
	{
		HitRecord leftRec, rightRec;
		bool hitLeft = left->hit(r, tMin, tMax, leftRec);
		bool hitRight = right->hit(r, tMin, tMax, rightRec);

		if (!hitLeft && !hitRight)
		{
			return false;
		}

		// choose the closer one for hit point if both hit
		if (hitLeft && hitRight)
		{
			rec = (leftRec.t < rightRec.t) ? leftRec : rightRec;
		}
		else if (hitLeft)
		{
			rec = leftRec;
		}
		else
		{
			rec = rightRec;
		}

		return true;
	}

	return false;
}

bool BVH::bbox(float t0, float t1, AABB& box) const
{
	box = _bbox;
	return true;
}