#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "ray.h"
#include "renderContext.h"

static Vec3 RandomSampleInUnitDisk(Random &rand)
{
	Vec3 p;
	do {
		//p = 2.0 * Vec3(rand.rDiffuse(), rand.rDiffuse(), rand.rDiffuse()) - Vec3( 1.f, 1.f, 1.f);
		p = Vec3(rand.rDiffuse(), rand.rDiffuse(), 0);
		//p = 2.0 * Vec3(rand.rSample(), rand.rSample(), 0) - Vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0f);

	return p;
}

class Camera
{
public:
	static const float VIEW_WIDTH;
	static const float VIEW_HEIGHT;
	static const float DEFAULT_ASPECT;

	Camera() = default;
	Camera(Vec3 lookFrom, Vec3 lookAt,Vec3 vUP, float vfov, 
		float aspect, float aperture, float focusDist,
		RenderContext& context);
	~Camera() = default;

	Ray CreateRay(float u, float v);

private:
	Vec3 origin = Vec3( 0.f, 0.f, 0.f );
	Vec3 lowerLeftCorner = Vec3(-VIEW_WIDTH / 2.f, -VIEW_HEIGHT / 2.f, -VIEW_HEIGHT / 2.f);
	Vec3 horizontal = Vec3(VIEW_WIDTH, 0.0f, 0.0f);
	Vec3 vertical = Vec3(0.0f, VIEW_HEIGHT, 0.0f);
	// axises in camera space
	Vec3 u, v, w;
	float lensRadius;
	RenderContext *contextPtr;
};

const float Camera::DEFAULT_ASPECT = 0.5625f;
const float Camera::VIEW_WIDTH = 4.0f;
const float Camera::VIEW_HEIGHT = DEFAULT_ASPECT * VIEW_WIDTH;

Camera::Camera(Vec3 lookFrom, Vec3 lookAt, Vec3 vUP, 
	float vfov, float aspect, float aperture, float focusDist,
	RenderContext& context)
{
	lensRadius = aperture / 2.f;
	float theta = vfov * float(M_PI) / 180.f;
	float half_height = tanf(theta / 2.f);
	float half_width = aspect * half_height;

	origin = lookFrom;
	w = unit_vector(origin - lookAt);
	u = unit_vector(cross(vUP, w));
	v = cross(w, u);
	//lowerLeftCorner = Vec3(-half_width, -half_height, -1.f);
	//horizontal = Vec3(half_width * 2.0f, 0.0f, 0.0f);
	//vertical = Vec3(0.0f, half_height*2.0f, 0.0f);
	lowerLeftCorner = origin - focusDist * ( half_width * u + half_height * v + w);
	horizontal = half_width * 2.0f * u * focusDist;
	vertical = half_height * 2.0f * v * focusDist;

	contextPtr = &context;
}

Ray Camera::CreateRay(float s, float t)
{
	Vec3 rd = lensRadius * RandomSampleInUnitDisk(contextPtr->rand);
	Vec3 offsetOrigin = u * rd.x() + v * rd.y() + origin;
	return Ray(offsetOrigin, lowerLeftCorner + s * horizontal + t * vertical - offsetOrigin);
}