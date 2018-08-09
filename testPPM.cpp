#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include "ray.h"

#include <algorithm>


#include <vector>

#include <iostream>

// time
#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

const int WIDTH = 19200;
const int HEIGHT = 10800;
static const bool USE_PNG = true;
//const bool parallelism_enabled = true;

#if defined(__clang__)
#define constexpr
#else
#include <execution>
#endif


void PPM_HEADER()
{
	std::cout << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n";
}

float HitSphere(const Vec3& center, float radius, const Ray& r)
{
	//Vec3 originToCenter = center - r.Origin();
	//float distanceToCenter = originToCenter.length();
	//float touchAngle = asinf(radius / distanceToCenter);

	//if (dot(unit_vector(originToCenter), unit_vector(r.Direction())) >= cosf(touchAngle))
	//	return true;

	//return false;

	/*

	 dot( (p-C), (p-C ) ) = R*R
	 p = A + t * B

	 t*t*dot( B , B ) + 2*t*dot( B,A - C ) + dot( A-C,A - C ) - R*R = 0
	 */


	Vec3 oc = r.Origin() - center;
	float a = dot(r.Direction(), r.Direction());
	float b = 2.0f * dot(oc, r.Direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;
	if(discriminant < 0)
	{
		// return unit(t) backwards when no hit
		return -1.f;
	}
	else
	{
		// first hit
		return (-b - sqrtf(discriminant)) / (2.0f * a);
	}

}

	
Vec3 RenderColor(const Ray& r)
{
	Vec3 result(0.f, 0.f, 0.f);
	const Vec3 sphereCenter(0.f, 0.f, -1.f);
	float t = HitSphere(sphereCenter, 0.5f, r);
	if ( t > 0.0f)
	{
		//result = Vec3(1.f, 0.f, 0.f);
		Vec3 normal = unit_vector(r.PointOnRay(t) - sphereCenter);

		// map normal from [-1, 1] to [0, 1]
		//normal += 1.0f;
		result = 0.5f * (normal + 1.0f);
	}		
	else
	{
		t = 0.5f * (unit_vector(r.Direction()).y() + 1.0f);
		result = (1.0f - t) * Vec3(1.0f, 1.0f, 1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);  // 0~ 1
	}

	return result * 255.f;  // 0 ~ 255
}

/*
CPU core: 6 HT: 12

Simple Ray trace example:
OMP 2.0:

outer loop
Ray trace time: 1903ms
Thread count: 15

nested for loop
Ray trace time: 1949ms
Thread count: 15

C++ 17 parallel:
outer loop:
Ray trace time: 1955ms
Thread count: 18

nested for loop
Ray trace time: 1925ms
Thread count: 18

std::execution::seq vs "for loop" vs omp ordered
seq:         11952ms
"for loop":  12137ms
omp ordered: 12160ms
*/


int main()
{
	static const Vec3 origin(0.f, 0.f, 0.f);
	const float VIEW_WIDTH = 4.0f;
	const float VIEW_HEIGHT = float(HEIGHT) / float(WIDTH) * VIEW_WIDTH;
	static const Vec3 lowerLeftCorner(-VIEW_WIDTH/2.f, -VIEW_HEIGHT/2.f, -VIEW_HEIGHT / 2.f);
	static const Vec3 horizontal(VIEW_WIDTH, 0.0f, 0.0f);	
	static const Vec3 vertical(0.0f, VIEW_HEIGHT, 0.0f);

	time_point<Clock> start, end;

	unsigned char* imageBuffer = nullptr;
	if constexpr(!USE_PNG)
	{
		PPM_HEADER();
	}	

	if constexpr(USE_PNG)
	{
		imageBuffer = new unsigned char[WIDTH * HEIGHT * 3];
	}

	start = Clock::now();

	// generate an increased sequence of integers
	std::vector<int> heights(HEIGHT);	
	std::generate(heights.begin(), heights.end(), [&]() { static int hi = 0; return hi++; });

	//std::vector<int> widths(WIDTH);
	//std::generate(widths.begin(), widths.end(), [&]() { static int wi = 0; return wi++; });
	
	#pragma omp parallel for
	//#pragma omp parallel for if(parallelism_enabled)
	//#pragma omp for ordered schedule(dynamic)
	for (int i = 0; i < HEIGHT; ++i)
	//std::for_each(std::execution::seq,
	//	std::begin(heights),
	//	std::end(heights),
	//	[&imageBuffer]( int i )
	{

		//std::for_each(std::execution::par_unseq,
		//	std::begin(widths),
		//	std::end(widths),
		//	[&imagebuffer, i](int j) 
		//#pragma omp parallel for
		for (int j = 0; j < WIDTH; ++j)
		{
			float u = float(j) / float(WIDTH);
			float v = float(HEIGHT - 1 - i) / float(HEIGHT);

			Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
			if constexpr(USE_PNG)
			{
				IntVec3 rgb(RenderColor(r));
				imageBuffer[3 * (i*WIDTH + j) + 0] = rgb.R();
				imageBuffer[3 * (i*WIDTH + j) + 1] = rgb.G();
				imageBuffer[3 * (i*WIDTH + j) + 2] = rgb.B();
			}
			else
			{
				//#pragma omp ordered
				std::cout << IntVec3(RenderColor(r)) << std::endl;
			}

		}
		//);
	}
	//);

	end = Clock::now();
	milliseconds diff = duration_cast<milliseconds>(end - start);
	std::cout << "Ray trace time: "<< diff.count() << "ms" << std::endl;

	start = Clock::now();
	//int result = 0;
	//if constexpr(USE_PNG)
	//{		
	//	stbi_write_png("e:/learning/RTWE/renderTest.png", WIDTH, HEIGHT, 3, imageBuffer, 3 * WIDTH);
	//	delete[] imageBuffer;
	//}
	end = Clock::now();

	diff = duration_cast<milliseconds>(end - start);
	std::cout << "Png write time: " << diff.count() << "ms" << std::endl;
	return getchar();
}
