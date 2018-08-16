#pragma once

#include "AABB.h"

#include <vector>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

class Material;
class Texture;
struct RenderContext;

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

class FlipNormals : public Hitable
{
public:
	FlipNormals(Hitable* p) : ptr(p) {}

	virtual bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override
	{
		if (ptr->hit(r, tMin, tMax, rec))
		{
			rec.normal *= -1;
			return true;
		}

		return false;
	}
	virtual bool bbox(float t0, float t1, AABB& box) const override
	{
		return ptr->bbox(t0, t1, box);
	}
private:

	Hitable * ptr;
};


class Translate : public Hitable {
public:
	Translate(Hitable *p, const Vec3& displacement) : ptr(p), offset(displacement) {}
	virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;
	Hitable *ptr;
	Vec3 offset;
};



class rotate_y : public Hitable {
public:
	rotate_y(Hitable *p, float angle);
	virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override {
		box = _bbox; return hasbox;
	}
	Hitable *ptr;
	float sin_theta;
	float cos_theta;
	bool hasbox;
	AABB _bbox;
};


class ConstantMedium : public Hitable
{
public:
	ConstantMedium(Hitable* bound, float density, const Texture& t, RenderContext & context);
	virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override;
	virtual bool bbox(float t0, float t1, AABB& box) const override;
private:
	Hitable * boundary;
	float density;
	Material* phaseFunc;

	RenderContext* contextPtr;
};