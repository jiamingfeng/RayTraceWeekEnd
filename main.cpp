#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// geometries
#include "material.h"
#include "sphere.h"
#include "camera.h"
#include "renderContext.h"

//std cout
#include <iostream>

#include <cmath>

// max float
#include <limits>

// time
#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;


static const bool USE_PNG = true;
//const bool parallelism_enabled = true;
static const unsigned int MAX_TRACE_DEPTH = 50;


// C++ 17 parallel
#ifdef __clang__
//#include <experimental/execution>
// C++ 17 file system
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <execution>
// C++ 17 file system
#include <filesystem>
namespace fs = std::filesystem;
#endif


void PPM_HEADER(int width, int height)
{
	std::cout << "P3\n" << width << " " << height << "\n255\n";
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

// argument parsing

// References:
//  https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
//  https://stackoverflow.com/questions/26587110/returning-an-empty-string-efficient-way-in-c
std::string getArugmentOption(int argc, char** argv, const std::string & option)
{
	char **end = argv + argc;
	char ** itr = std::find(argv, argv+argc, option);
	if (itr != end && ++itr != end)
	{
		return std::string(*itr);
	}
	return {};
}

bool DoesArgumentExist(int argc, char** argv, const std::string& option)
{
	char **end = argv + argc;
	return std::find(argv, end, option) != end;
}


static Vec3 RenderColor(const Ray& r, const Hitable *world, unsigned int TraceDepth)
{
	HitRecord recHit;
	Vec3 result(0.f, 0.f, 0.f);

	float maxDistance = std::numeric_limits<float>::max();
	if (world->hit(r, 0.0001f, maxDistance, recHit))
	{
		Ray scatted;
		Vec3 attenuation;
		if (TraceDepth < MAX_TRACE_DEPTH && recHit.mat->Scatter(r, recHit, attenuation, scatted))
		{
			result = attenuation * RenderColor(scatted, world, TraceDepth+1);
		}		
	}
	else
	{
		float t = 0.5f * (unit_vector(r.Direction()).y() + 1.0f);
		result = (1.0f - t) * Vec3(1.0f, 1.0f, 1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);  // 0~ 1
	}

	return result;// * 255.f;  // 0 ~ 255
}

HitableList *random_scene(RenderContext & context) {
	int n = 500;
	HitableList *world = new HitableList();
	world->list.reserve(500);
	world->list.push_back(new Sphere(Vec3(0, -1000.f, 0), 1000.f, new Lambert(Vec3(0.5f, 0.5f, 0.5f), context)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = context.rand.rSample();
			Vec3 center(float(a) + 0.9f*context.rand.rSample(), 0.2f, float(b) + 0.9f*context.rand.rSample());
			if ((center - Vec3(4.f, 0.2f, 0)).length() > 0.9f) {
				if (choose_mat < 0.8f) {  // diffuse
					world->list.push_back (new Sphere(center, 0.2f, new Lambert(
						Vec3(context.rand.rSample()*context.rand.rSample(), 
						context.rand.rSample()*context.rand.rSample(), 
						context.rand.rSample()*context.rand.rSample()), context)));
				}
				else if (choose_mat < 0.95f) { // metal
					world->list.push_back(new Sphere(center, 0.2f,
						new Metal(Vec3(0.5f*(1.f + context.rand.rSample()), 
							0.5f*(1.f + context.rand.rSample()), 
							0.5f*(1.f + context.rand.rSample())), 
							0.5f*context.rand.rSample(), context)));
				}
				else {  // glass
					world->list.push_back(new Sphere(center, 0.2f, new Dielectric(1.5f, context)));
				}
			}
		}
	}

	world->list.push_back(new Sphere(Vec3(0, 1.f, 0), 1.f, new Dielectric(1.5f, context)));
	world->list.push_back(new Sphere(Vec3(-4.f, 1.f, 0), 1.0f, new Lambert(Vec3(0.4f, 0.2f, 0.1f), context)));
	world->list.push_back(new Sphere(Vec3(4.f, 1.f, 0), 1.0f, new Metal(Vec3(0.7f, 0.6f, 0.5f), 0.0, context)));

	return world;
}


int main(int argc, char** argv)
{
	int WIDTH = 1920;
	int HEIGHT = 1080;
	int SAMPLE_COUNT = 128;
	if (DoesArgumentExist(argc, argv, "-w"))
	{
		WIDTH = std::atoi(getArugmentOption(argc, argv, "-w").c_str());
	}

	if (DoesArgumentExist(argc, argv, "-h"))
	{
		HEIGHT = std::atoi(getArugmentOption(argc, argv, "-h").c_str());
	}

	if (DoesArgumentExist(argc, argv, "-s"))
	{
		SAMPLE_COUNT = std::atoi(getArugmentOption(argc, argv, "-s").c_str());
	}

	RenderContext context;

	const float VIEW_WIDTH = 4.0f;
	const float VIEW_HEIGHT = float(HEIGHT) / float(WIDTH) * VIEW_WIDTH;
	//Vec3 lookFrom(3.f, 3.f, 2.f);
	//Vec3 lookAt(0, 0, -1.f);
	//float distToFocus = (lookFrom - lookAt).length();
	//float aperture = 0.5f;

	Vec3 lookFrom(13.f, 2.f, 3.f);
	Vec3 lookAt(0, 0, 0);
	float distToFocus = 10.0f;
	float aperture = 0.1f;

	Camera camera(lookFrom, lookAt, Vec3(0, 1, 0),
		          20.f, float(WIDTH) / float(HEIGHT),
		          aperture, distToFocus, context);
	


	// define world
	//HitableList* world = new HitableList();
	//world->list.resize(4);
	//Lambert mat1(Vec3(0.1f, 0.2f, 0.5f), context);
	//Lambert mat2(Vec3(0.8f, 0.8f, 0.0f), context);
	//Metal mat3(Vec3(0.8f, 0.6f, 0.2f), 0.2f, context);
	////Metal mat4(Vec3(0.8f, 0.8f, 0.8f), 0.8f, context);
	//Dielectric glass(1.5f, context);

	//Sphere s0(Vec3(0.f, 0.2f, -1.0f), 0.5f, &mat1);
	//Sphere s1(Vec3(0.f, -100.5f, -5.f), 100.f, &mat2);
	//Sphere s2(Vec3(1.f, 0.f, -1.f), 0.5f, &mat3);
	//Sphere s3(Vec3(-1.f, 0.f, -1.f), 0.5f, &glass);
	//Sphere s4(Vec3(-1.f, 0.f, -1.f), -0.48f, &glass);
	//
	//world->list[0] = &s0;
	//world->list[1] = &s1;
	//world->list[2] = &s2;
	//world->list[3] = &s3;
	//world->list[4] = &s4;

	HitableList* world = random_scene(context);




	time_point<Clock> start, end;

	unsigned char* imageBuffer = nullptr;
	if constexpr(!USE_PNG)
	{
		PPM_HEADER(WIDTH, HEIGHT);
	}	

	if constexpr(USE_PNG)
	{
		imageBuffer = new unsigned char[WIDTH * HEIGHT * 3];
	}

	start = Clock::now();

	// generate an increased sequence of integers
	//std::vector<int> heights(HEIGHT);	
	//std::generate(heights.begin(), heights.end(), [&]() { static int hi = 0; return hi++; });

	//std::vector<int> widths(WIDTH);
	//std::generate(widths.begin(), widths.end(), [&]() { static int wi = 0; return wi++; });
	
	//#pragma omp parallel for collapse(2)
	//#pragma omp parallel for if(parallelism_enabled)
	//#pragma omp for ordered schedule(dynamic)
	#pragma omp parallel for
	for (int i = 0; i < HEIGHT; ++i)
	// std::for_each(std::execution::seq,
	// 	std::begin(heights),
	// 	std::end(heights),
	// 	[&imageBuffer]( int i )
	{

		//std::for_each(std::execution::par_unseq,
		//	std::begin(widths),
		//	std::end(widths),
		//	[&imagebuffer, i](int j) 
		//#pragma omp parallel for

		for (int j = 0; j < WIDTH; ++j)
		{
			// uniform distributed floating point values

			Vec3 finalColor(0, 0, 0);
			// iterator through all samples per pixel
			for (int s = 0; s < SAMPLE_COUNT; ++s)
			{
				float u = float(j + context.rand.rSample()) / float(WIDTH);
				float v = float(HEIGHT - 1 - i + context.rand.rSample()) / float(HEIGHT);

				Ray r(camera.CreateRay(u, v));

				finalColor += RenderColor(r, world, 0);
			}

			finalColor = finalColor / float(SAMPLE_COUNT);
			finalColor = Vec3(pow(finalColor[0], 0.5f), pow(finalColor[1], 0.5f), pow(finalColor[2], 0.5f));
			IntVec3 rgb(finalColor  * 255.f );

			if constexpr(USE_PNG)
			{
				
				imageBuffer[3 * (i*WIDTH + j) + 0] = rgb.R();
				imageBuffer[3 * (i*WIDTH + j) + 1] = rgb.G();
				imageBuffer[3 * (i*WIDTH + j) + 2] = rgb.B();
			}
			else
			{
				//#pragma omp ordered
				std::cout << rgb << std::endl;
			}

		}
		//);
	}
	//);

	delete world;

	end = Clock::now();
	milliseconds diff = duration_cast<milliseconds>(end - start);
	std::cout << "Ray trace time: "<< diff.count() << "ms" << std::endl;

	start = Clock::now();
	int result = 0;
	if constexpr(USE_PNG)
	{		
		fs::path filePath = fs::current_path();
		filePath /= "renderTest.png";
		stbi_write_png(filePath.string().c_str(), WIDTH, HEIGHT, 3, imageBuffer, 3 * WIDTH);
		delete[] imageBuffer;
	}
	end = Clock::now();

	diff = duration_cast<milliseconds>(end - start);
	std::cout << "Png write time: " << diff.count() << "ms" << std::endl;
	return getchar();
}



