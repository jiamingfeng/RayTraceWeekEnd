#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// geometries
#include "hitable.h"
#include "sphere.h"

#include <iostream>

#include <limits>

// time
#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;


static const bool USE_PNG = true;
//const bool parallelism_enabled = true;


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

	
Vec3 RenderColor(const Ray& r, const Hitable *world)
{
	HitRecord recHit;
	Vec3 result(0.f, 0.f, 0.f);

	float maxDistance = std::numeric_limits<float>::max();
	if ( world->hit(r, 0.f, maxDistance, recHit))
	{
		//result = Vec3(1.f, 0.f, 0.f);
		auto &normal = recHit.normal;

		// map normal from [-1, 1] to [0, 1]
		//normal += 1.0f;
		result = 0.5f * (normal + 1.0f);
	}		
	else
	{
		float t = 0.5f * (unit_vector(r.Direction()).y() + 1.0f);
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


int main(int argc, char** argv)
{
	int WIDTH = 1920;
	int HEIGHT = 1080;
	if (DoesArgumentExist(argc, argv, "-w"))
	{
		WIDTH = std::atoi(getArugmentOption(argc, argv, "-w").c_str());
	}

	if (DoesArgumentExist(argc, argv, "-h"))
	{
		HEIGHT = std::atoi(getArugmentOption(argc, argv, "-h").c_str());
	}


	static const Vec3 origin(0.f, 0.f, 0.f);
	const float VIEW_WIDTH = 4.0f;
	const float VIEW_HEIGHT = float(HEIGHT) / float(WIDTH) * VIEW_WIDTH;
	static const Vec3 lowerLeftCorner(-VIEW_WIDTH/2.f, -VIEW_HEIGHT/2.f, -VIEW_HEIGHT / 2.f);
	static const Vec3 horizontal(VIEW_WIDTH, 0.0f, 0.0f);	
	static const Vec3 vertical(0.0f, VIEW_HEIGHT, 0.0f);

	// define world
	HitableList* world = new HitableList();
	Sphere small(Vec3(0.f, 0.2f, -1.0f), 0.4f);
	Sphere large(Vec3(0.f, -100.f, -5.f), 100.f);
	
	world->list.push_back(&small);
	world->list.push_back(&large);
	

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
	std::vector<int> heights(HEIGHT);	
	std::generate(heights.begin(), heights.end(), [&]() { static int hi = 0; return hi++; });

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
			float u = float(j) / float(WIDTH);
			float v = float(HEIGHT - 1 - i) / float(HEIGHT);

			Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
			if constexpr(USE_PNG)
			{
				IntVec3 rgb(RenderColor(r, world));
				imageBuffer[3 * (i*WIDTH + j) + 0] = rgb.R();
				imageBuffer[3 * (i*WIDTH + j) + 1] = rgb.G();
				imageBuffer[3 * (i*WIDTH + j) + 2] = rgb.B();
			}
			else
			{
				//#pragma omp ordered
				std::cout << IntVec3(RenderColor(r, world)) << std::endl;
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
