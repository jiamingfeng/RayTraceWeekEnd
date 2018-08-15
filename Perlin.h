#pragma once

#include "vec3.h"
#include "renderContext.h"
#include <algorithm>

class Perlin {
public:
	static float noise(const Vec3& p);
	static float turb(const Vec3& p, int depth = 8);

	static void Init(RenderContext & context);

	static Vec3 *randVec;
	static int *permX;
	static int *permY;
	static int *permZ;

	static const int NOISE_SIZE;

private:
	static Vec3* perlinGenerate(RenderContext & context);
	static void permute(int *p, int n, RenderContext & context);
	static int* perlinGeneratePerm(RenderContext & context);
};

