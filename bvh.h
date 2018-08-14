#pragma once
#include "hitable.h"
#include "renderContext.h"

class BVH : public Hitable
{
public:
	BVH(RenderContextSP context) :
		contextPtr(context), Hitable() {}
	BVH(std::vector<HitableSP> & hitList, float t0, float t1, RenderContextSP context);
	~BVH() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;
private:

	void SortByAxis(std::vector<HitableSP> &hitableList, int axis);

	std::shared_ptr<Hitable> left;
	std::shared_ptr<Hitable> right;

	AABB _bbox;
	RenderContextWP contextPtr;
};

