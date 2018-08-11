#pragma once

#include "ray.h"

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

	Material *mat;
};

class Hitable {
public:
	Hitable(Material* newMat = nullptr) : mat(newMat) {}
	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;

protected: 
	Material * mat;
};

class HitableList : public Hitable {
public:
	HitableList(Material* newMat=nullptr) : Hitable(newMat) {};
	~HitableList() = default;

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;

	std::vector<Hitable*> list;
};