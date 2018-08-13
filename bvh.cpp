#include "bvh.h"

BVH::BVH( float t0, float t1)
{
}

bool BVH::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const
{
	if (_bbox.hit(r, tMin, tMax))
	{
		HitRecord leftRec, rightRec;
		bool hitLeft = left->hit(r, tMin, tMax, leftRec);
		bool hitRight = left->hit(r, tMin, tMax, rightRec);

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