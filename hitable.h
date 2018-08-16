#pragma once

#include "aabb.h"

#include <vector>
#include <algorithm>

class Material;

struct HitRecord 
{
	HitRecord() = default;
	~HitRecord() = default;

	Vec3 p;
	Vec3 normal;
	float t;
	float u, v;

	Material *mat;
};

class Hitable {
public:
	Hitable(Material* newMat = nullptr) : mat(newMat) {}
	virtual ~Hitable();
	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;
	virtual bool bbox(float t0, float t1, AABB& box) const = 0;

	static AABB MergeBBoxes(const AABB& bbox0, const AABB& bbox1);

protected: 
	Material * mat;
};

class HitableList : public Hitable {
public:
	HitableList(Material* newMat=nullptr) : Hitable(newMat) {};
	~HitableList();

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;

	std::vector<Hitable*> list;
};