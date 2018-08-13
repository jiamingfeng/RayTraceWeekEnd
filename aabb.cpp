#include "aabb.h"

bool AABB::hit(const Ray& r, float tMin, float tMax) const
{
	for (int d = 0; d < 3; ++d)
	{
		float invD = 1.f / r.Direction()[d];
		float t0 = (_min[d] - r.Origin()[d]) * invD;
		float t1 = (_max[d] - r.Origin()[d]) * invD;

		if (invD < 0.f)
		{
			std::swap(t0, t1);
		}

		tMin = std::max(t0, tMin);
		tMax = std::min(t1, tMax);

		if (tMax <= tMin)
		{
			return false;
		}

	}

	return true;
}