#include "hitable.h"
#include "material.h"

Hitable::~Hitable()
{ 
	delete mat; 
}

AABB Hitable::MergeBBoxes(const AABB& box0, const AABB& box1)
{
	Vec3 bboxMin(std::fminf(box0.Min().x(), box1.Min().x()),
		std::fminf(box0.Min().y(), box1.Min().y()),
		std::fminf(box0.Min().z(), box1.Min().z()));

	Vec3 bboxMax(std::fmaxf(box0.Max().x(), box1.Max().x()),
		std::fmaxf(box0.Max().y(), box1.Max().y()),
		std::fmaxf(box0.Max().z(), box1.Max().z()));

	return AABB(bboxMin, bboxMax);
}

HitableList::~HitableList()
{
	std::for_each(list.begin(), list.end(), [](Hitable* h)
	{
		delete h;
	});
}

bool HitableList::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const 
{

	HitRecord tempRec;
	bool hitAnygthing = false;
	float closestSoFar = tMax;

	// test the ray for each hitable items in list and return the nearest hit
	std::for_each(list.begin(), list.end(), [&r, &rec, &tempRec, &hitAnygthing, &closestSoFar, tMin](Hitable* h)
	{

		if (h->hit(r, tMin, closestSoFar, tempRec))
		{
			hitAnygthing = true;
			closestSoFar = tempRec.t;
			rec = tempRec;
		}
	});

	return hitAnygthing;
}

bool HitableList::bbox(float t0, float t1, AABB& box) const
{
	if (list.size() == 0)
	{
		return false;
	}

	// merge all bbox in this hitable list
	AABB eachBBox;
	bool result = list[0]->bbox(t0, t1, eachBBox);
	if (!result)
	{
		return false;
	}

	box = eachBBox;
	auto iter = std::find_if_not(list.begin(), list.end(), [t0, t1, &eachBBox, &box](Hitable* h) {
		bool hasBBox = h->bbox(t0, t1, eachBBox);
		box = Hitable::MergeBBoxes(box, eachBBox);
		return hasBBox;
	});

	// failed at iterating through all hitables
	if (iter != list.end())
	{
		return false;
	}

	return true;

}

bool Translate::hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const {
	Ray moved_r(r.Origin() - offset, r.Direction(), r.Time());
	if (ptr->hit(moved_r, t_min, t_max, rec)) {
		rec.p += offset;
		return true;
	}
	else
		return false;
}

bool Translate::bbox(float t0, float t1, AABB& box) const {
	if (ptr->bbox(t0, t1, box)) {
		box = AABB(box.Min() + offset, box.Max() + offset);
		return true;
	}
	else
		return false;
}


rotate_y::rotate_y(Hitable *p, float angle) : ptr(p) {
	float radians = (float(M_PI) / 180.f) * angle;
	sin_theta = sin(radians);
	cos_theta = cos(radians);
	hasbox = ptr->bbox(0, 1, _bbox);
	Vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				float x = i * _bbox.Max().x() + (1 - i)*_bbox.Min().x();
				float y = j * _bbox.Max().y() + (1 - j)*_bbox.Min().y();
				float z = k * _bbox.Max().z() + (1 - k)*_bbox.Min().z();
				float newx = cos_theta * x + sin_theta * z;
				float newz = -sin_theta * x + cos_theta * z;
				Vec3 tester(newx, y, newz);
				for (int c = 0; c < 3; c++)
				{
					if (tester[c] > max[c])
						max[c] = tester[c];
					if (tester[c] < min[c])
						min[c] = tester[c];
				}
			}
		}
	}
	_bbox = AABB(min, max);
}

bool rotate_y::hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const {
	Vec3 origin = r.Origin();
	Vec3 direction = r.Direction();
	origin[0] = cos_theta * r.Origin()[0] - sin_theta * r.Origin()[2];
	origin[2] = sin_theta * r.Origin()[0] + cos_theta * r.Origin()[2];
	direction[0] = cos_theta * r.Direction()[0] - sin_theta * r.Direction()[2];
	direction[2] = sin_theta * r.Direction()[0] + cos_theta * r.Direction()[2];
	Ray rotated_r(origin, direction, r.Time());
	if (ptr->hit(rotated_r, t_min, t_max, rec)) {
		Vec3 p = rec.p;
		Vec3 normal = rec.normal;
		p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
		p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
		normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
		normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
		rec.p = p;
		rec.normal = normal;
		return true;
	}
	else
		return false;
}

ConstantMedium::ConstantMedium(Hitable* bound, float d, const Texture& t, RenderContext & context)
	: boundary(bound), density(d), phaseFunc(new ISOTropic(t, context)), contextPtr(&context)
{
}

bool ConstantMedium::bbox(float t0, float t1, AABB& box) const
{
	return boundary->bbox(t0, t1, box);
}

bool ConstantMedium::hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const
{
	//bool db = (contextPtr->rand.rSample() < 0.00001f);
	bool db = false;
	HitRecord rec1, rec2;
	if (boundary->hit(r, -FLT_MAX, FLT_MAX, rec1)) {
		if (boundary->hit(r, rec1.t + 0.0001f, FLT_MAX, rec2)) {
			if (db) std::cerr << "\nt0 t1 " << rec1.t << " " << rec2.t << "\n";
			if (rec1.t < t_min)
				rec1.t = t_min;
			if (rec2.t > t_max)
				rec2.t = t_max;
			if (rec1.t >= rec2.t)
				return false;
			if (rec1.t < 0)
				rec1.t = 0;
			float distance_inside_boundary = (rec2.t - rec1.t)*r.Direction().length();
			float hit_distance = -(1.f / density)*log(contextPtr->rand.rSample());
			if (hit_distance < distance_inside_boundary) {
				if (db) std::cerr << "hit_distance = " << hit_distance << "\n";
				rec.t = rec1.t + hit_distance / r.Direction().length();
				if (db) std::cerr << "rec.t = " << rec.t << "\n";
				rec.p = r.PointOnRay(rec.t);
				if (db) std::cerr << "rec.p = " << rec.p << "\n";
				rec.normal = Vec3(1, 0, 0);  // arbitrary
				rec.mat = phaseFunc;
				return true;
			}
		}
	}
	return false;
}
