#pragma once

#include <cmath>
#include <stdlib.h>
#include <iostream>

class Vec3 {


public:
	Vec3() = default;
	virtual ~Vec3() = default;
	Vec3(float e0, float e1, float e2) { e[0] = e0; e[1] = e1; e[2] = e2; }
	inline const float x() const { return e[0]; }
	inline const float y() const { return e[1]; }
	inline const float z() const { return e[2]; }
	inline const float r() const { return e[0]; }
	inline const float g() const { return e[1]; }
	inline const float b() const { return e[2]; }

	inline const Vec3& operator+() const { return *this; }
	inline Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
	inline const float operator[](int i) const { return e[i]; }
	inline float& operator[](int i) { return e[i]; };

	inline Vec3& operator+=(const Vec3 &v2);
	inline Vec3& operator-=(const Vec3 &v2);
	inline Vec3& operator*=(const Vec3 &v2);
	inline Vec3& operator/=(const Vec3 &v2);
	inline Vec3& operator+=(const float t);
	inline Vec3& operator-=(const float t);
	inline Vec3& operator*=(const float t);
	inline Vec3& operator/=(const float t);

	friend Vec3 operator+(Vec3 v, const float t) { v += t; return v; }
	friend Vec3 operator-(Vec3 v, const float t) { v -= t; return v; }
	//friend Vec3 operator*(Vec3 v, const float t) { v *= t; return v; }
	//friend Vec3 operator/(Vec3 v, const float t) { v /= t; return v; }

	inline float length() const { return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]); }
	inline float squared_length() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }
	inline void make_unit_vector();

	typedef float(*mapF)(float);
	inline Vec3 map(mapF mapFunc) const;


	float e[3];
};

// helper class to ouput Vec3 components as int
class IntVec3 : public Vec3{
public:
	IntVec3(const Vec3 & other)
	{
		e[0] = other.e[0];
		e[1] = other.e[1];
		e[2] = other.e[2];
	}

	inline int R() { return int(std::roundf(e[0])); };
	inline int G() { return int(std::roundf(e[1])); };
	inline int B() { return int(std::roundf(e[2])); };
};


inline std::istream& operator>>(std::istream &is, Vec3 &t) {
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}

inline std::ostream& operator<<(std::ostream &os, const Vec3 &t) {
	os << t.e[0] << " " << t.e[1] << " " << t.e[2];
	return os;
}

inline std::istream& operator>>(std::istream &is, IntVec3 &t) {
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}

inline std::ostream& operator<<(std::ostream &os, const IntVec3 &t) {
	os << int(std::roundf(t.e[0])) << " " << int(std::roundf(t.e[1])) << " " << int(std::roundf(t.e[2]));
	return os;
}

inline void Vec3::make_unit_vector() {
	float k = 1.0f / sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	e[0] *= k; e[1] *= k; e[2] *= k;
}

inline Vec3 Vec3::map(mapF mapFunc) const
{
	Vec3 result;
	for (int i = 0; i < 3; ++i)
	{
		result[i] = mapFunc(e[i]);
	}

	return result;
}

inline Vec3 operator+(const Vec3 &v1, const Vec3 &v2) {
	return Vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

inline Vec3 operator-(const Vec3 &v1, const Vec3 &v2) {
	return Vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

inline Vec3 operator*(const Vec3 &v1, const Vec3 &v2) {
	return Vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}

inline Vec3 operator/(const Vec3 &v1, const Vec3 &v2) {
	return Vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

inline Vec3 operator*(float t, const Vec3 &v) {
	return Vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline Vec3 operator/(Vec3 v, float t) {
	return Vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}

inline Vec3 operator*(const Vec3 &v, float t) {
	return Vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline float dot(const Vec3 &v1, const Vec3 &v2) {
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

inline Vec3 cross(const Vec3 &v1, const Vec3 &v2) {
	return Vec3((v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1]),
		(-(v1.e[0] * v2.e[2] - v1.e[2] * v2.e[0])),
		(v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]));
}


inline Vec3& Vec3::operator+=(const Vec3 &v) {
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];
	return *this;
}

inline Vec3& Vec3::operator*=(const Vec3 &v) {
	e[0] *= v.e[0];
	e[1] *= v.e[1];
	e[2] *= v.e[2];
	return *this;
}

inline Vec3& Vec3::operator/=(const Vec3 &v) {
	e[0] /= v.e[0];
	e[1] /= v.e[1];
	e[2] /= v.e[2];
	return *this;
}

inline Vec3& Vec3::operator-=(const Vec3& v) {
	e[0] -= v.e[0];
	e[1] -= v.e[1];
	e[2] -= v.e[2];
	return *this;
}

inline Vec3& Vec3::operator+=(const float t) {
	e[0] += t;
	e[1] += t;
	e[2] += t;
	return *this;
}

inline Vec3& Vec3::operator-=(const float t) {
	e[0] -= t;
	e[1] -= t;
	e[2] -= t;
	return *this;
}

inline Vec3& Vec3::operator*=(const float t) {
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}

inline Vec3& Vec3::operator/=(const float t) {
	float k = 1.0f / t;

	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}

inline Vec3 unit_vector(const Vec3& v) {
	return v / v.length();
}


class LinearTimeVec3
{
public:
	LinearTimeVec3() = default;
	LinearTimeVec3(Vec3 v0, float t0, Vec3 v1, float t1) :
		v0(v0), v1(v1), t0(t0), t1(t1) {}
	Vec3 v(float t) const { return v0 + (t - t0) / (t1 - t0) * (v1 - v0); }
private:
	Vec3 v0;
	Vec3 v1;

	float t0;
	float t1;
};
