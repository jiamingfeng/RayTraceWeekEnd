#pragma once
#include "hitable.h"
#include "renderContext.h"

class BVH : public Hitable
{
public:
	BVH(RenderContext &context) :
		contextPtr(&context), Hitable() {}
	BVH(std::vector<Hitable*> & hitList, float t0, float t1, RenderContext &context);
	~BVH() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;

	std::vector<Hitable*> *listPtr;
private:

	void SortByAxis(int axis);

	Hitable *left;
	Hitable *right;

	AABB _bbox;
	RenderContext *contextPtr;
};

