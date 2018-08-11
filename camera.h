#pragma once

#include "ray.h"



class Camera
{
public:
	static const float VIEW_WIDTH;
	static const float VIEW_HEIGHT;
	static const float PROPORTION;

	Camera() = default;
	~Camera() = default;

	Ray CreateRay(float u, float v);

private:
	Vec3 origin = Vec3( 0.f, 0.f, 0.f );
	Vec3 lowerLeftCorner = Vec3(-VIEW_WIDTH / 2.f, -VIEW_HEIGHT / 2.f, -VIEW_HEIGHT / 2.f);
	Vec3 horizontal = Vec3(VIEW_WIDTH, 0.0f, 0.0f);
	Vec3 vertical = Vec3(0.0f, VIEW_HEIGHT, 0.0f);
};

const float Camera::PROPORTION = 0.5625f;
const float Camera::VIEW_WIDTH = 4.0f;
const float Camera::VIEW_HEIGHT = PROPORTION * VIEW_WIDTH;

Ray Camera::CreateRay(float u, float v)
{
	return Ray(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);
}