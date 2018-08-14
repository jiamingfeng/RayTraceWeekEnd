#pragma once

#include "aabb.h"
#include "core.h"

class Material;
#define MaterialSP std::shared_ptr<Material>
#define MaterialWP std::weak_ptr<Material>

struct HitRecord 
{
	HitRecord() = default;
	~HitRecord() = default;

	Vec3 p;
	Vec3 normal;
	float t;

	MaterialWP mat;
};

class Hitable {
public:
	Hitable(MaterialSP newMat = {}) : mat(newMat) {}
	virtual ~Hitable();
	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;
	virtual bool bbox(float t0, float t1, AABB& box) const = 0;

	static AABB MergeBBoxes(const AABB& bbox0, const AABB& bbox1);

protected: 
	MaterialWP mat;
};

#define HitableSP std::shared_ptr<Hitable>
#define HitableWP std::weak_ptr<Hitable>

class HitableList : public Hitable {
public:
	HitableList(MaterialSP newMat = {}) : Hitable(newMat) {};
	~HitableList();

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;

	std::vector<HitableSP> list;
};