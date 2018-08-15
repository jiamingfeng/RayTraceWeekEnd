#include "Perlin.h"

const int Perlin::NOISE_SIZE = 256;

Vec3 * Perlin::randVec = nullptr;
int * Perlin::permX = nullptr;
int * Perlin::permY = nullptr;
int * Perlin::permZ = nullptr;

static float perlinInterp(Vec3 c[2][2][2], const Vec3& uvw)
{
	Vec3 smoothUVW = uvw.map([](float x) { return x * x * (3.f - 2.f * x); });
	float u = uvw[0];
	float v = uvw[1];
	float w = uvw[2];

	float uu = smoothUVW[0];
	float vv = smoothUVW[1];
	float ww = smoothUVW[2];

	float accum = 0;

	for ( int i = 0; i< 2; ++i )
		for (int j = 0; j< 2; ++j)
			for (int k = 0; k < 2; ++k)
			{
				Vec3 weightV(u - i, v - j, w - k);
				accum += (i*uu + (1 - i)*(1 - uu)) *
					(j*vv + (1 - j)*(1 - vv)) *
					(k*ww + (1 - k)*(1 - ww)) * dot(c[i][j][k], unit_vector(weightV));
			}
	return accum;
}

float Perlin::noise(const Vec3 &p)
{
	Vec3 uvw = p.map([](float x) { return (x - floorf(x)); });	
	Vec3 ijk = p.map([](float x) { return floorf(x); });

	Vec3 hash[2][2][2];
	for (int i = 0; i< 2; ++i)
		for (int j = 0; j< 2; ++j)
			for (int k = 0; k < 2; ++k)
			{
				int px = permX[(i + int(ijk[0])) & (NOISE_SIZE - 1)];
				int py = permY[(j + int(ijk[1])) & (NOISE_SIZE - 1)];
				int pz = permZ[(k + int(ijk[2])) & (NOISE_SIZE - 1)];
				int index = px ^ py ^ pz;
				hash[i][j][k] = randVec[px ^ py ^ pz];
			}

	return perlinInterp(hash, uvw);
}

float Perlin::turb(const Vec3& p, int depth)
{
	float accum = 0;
	Vec3 tempP = p;
	float weight = 1.0f;
	for (int i = 0; i < depth; ++i)
	{
		accum += weight * noise(tempP);
		weight *= 0.5f;
		tempP *= 2.f;
	}

	return fabsf(accum);
}

void Perlin::Init(RenderContext & context)
{
	Perlin::randVec = Perlin::perlinGenerate(context);
	Perlin::permX = Perlin::perlinGeneratePerm(context);
	Perlin::permY = Perlin::perlinGeneratePerm(context);
	Perlin::permZ = Perlin::perlinGeneratePerm(context);
}

Vec3* Perlin::perlinGenerate(RenderContext & context)
{
	Vec3 *p = new Vec3[NOISE_SIZE];
	for (int i = 0; i < NOISE_SIZE; ++i)
	{
		p[i] = unit_vector(Vec3(context.rand.rDiffuse(), context.rand.rDiffuse(), context.rand.rDiffuse()));
	}

	return p;
}

void Perlin::permute(int *p, int n, RenderContext & context)
{
	for (int i = n - 1; i > 0; --i)
	{
		int target = int(context.rand.rSample() * (i + 1));
		std::swap(p[i], p[target]);
	}
}

int* Perlin::perlinGeneratePerm(RenderContext & context)
{
	int *p = new int[NOISE_SIZE];
	for (int i = 0; i < NOISE_SIZE; ++i)
	{
		p[i] = i;
	}

	permute(p, NOISE_SIZE, context);

	return p;
}