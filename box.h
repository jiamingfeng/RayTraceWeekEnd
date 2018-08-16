#pragma once

#include "aarec.h"
#include "Hitable.h"

class Box : public Hitable {
public:
	Box() {}
	Box(const Vec3& p0, const Vec3& p1, Material *ptr);
	virtual bool hit(const Ray& r, float t0, float t1, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override {
		box = AABB(pmin, pmax);
		return true;
	}
	Vec3 pmin, pmax;
	HitableList *hitList;
};

Box::Box(const Vec3& p0, const Vec3& p1, Material *ptr) {
	pmin = p0;
	pmax = p1;
	hitList = new HitableList();
	hitList->list.resize(6);
	hitList->list[0] = new XYRect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr);
	hitList->list[1] = new FlipNormals(new XYRect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
	hitList->list[2] = new XZRect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr);
	hitList->list[3] = new FlipNormals(new XZRect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
	hitList->list[4] = new YZRect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr);
	hitList->list[5] = new FlipNormals(new YZRect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
	
}

bool Box::hit(const Ray& r, float t0, float t1, HitRecord& rec) const {
	return hitList->hit(r, t0, t1, rec);
}