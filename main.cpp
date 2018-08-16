#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// geometries
#include "material.h"
#include "sphere.h"
#include "aarec.h"
#include "camera.h"
#include "renderContext.h"
#include "bvh.h"
#include "perlin.h"
#include "box.h"

//std cout
#include <iostream>

#include <cmath>

// _max float
#include <limits>

// time
#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

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



static const bool USE_PNG = true;
//const bool parallelism_enabled = true;
static const unsigned int MAX_TRACE_DEPTH = 50;

static const std::string EARTH_FILE_PATH = fs::current_path().string() + "\\earth.jpg";

// constants
static RenderContext context;
static ConstantTexture t0(Vec3(1.f, 1.f, 0.9f));
static ConstantTexture t1(Vec3(0.61f, 0.12f, 0.73f));
static ConstantTexture bright(Vec3(3.f, 3.f, 3.f));
static CheckerTexture checker(CheckerTexture(t0, t1));
static ConstantTexture red(Vec3(0.65f, 0.05f, 0.05f));
static ConstantTexture white(Vec3(0.73f, 0.73f, 0.73f));
static ConstantTexture pureWhite(Vec3(1.f, 1.f, 1.f));
static ConstantTexture pureBlack(Vec3(0.f, 0.f, 0.f));
static ConstantTexture green(Vec3(0.12f, 0.45f, 0.15f));
static ConstantTexture light(Vec3(15.f, 15.f, 15.f));
static NoiseTexture perlin(context, false, Vec3(1.f, 1.f, 1.f), 4.f);
static TurbulenceTexture rt1(context, true);
static TurbulenceTexture rt2(context, true);
static TurbulenceTexture rt3(context, true);
static TurbulenceTexture rt4(context, true);
static SineNoiseTexture sineNoise(context, false, Vec3(1.f, 1.f, 1.f), 2.8f);
static ImageTexture earth(EARTH_FILE_PATH);

static ImageTexture& latest = earth;

static std::vector<Texture*> TextureList;
static Texture* RandomTexture()
{
	int size = static_cast<int>(TextureList.size());
	if (TextureList.size() == 0)
	{
		TextureList.reserve(50);
		TextureList.push_back(&t0);
		TextureList.push_back(&t1);
		TextureList.push_back(&bright);
		TextureList.push_back(&checker);
		TextureList.push_back(&perlin);
		TextureList.push_back(&rt1);
		TextureList.push_back(&rt2);
		TextureList.push_back(&rt3);
		TextureList.push_back(&rt4);
		TextureList.push_back(&sineNoise);
		TextureList.push_back(&earth);
		TextureList.push_back(&latest);
	}

	return TextureList[int(float(size) * GlobalRandom.rSample())];
}


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
		Vec3 emitted = recHit.mat->Emitted(recHit.u, recHit.v, recHit.p);
		if (TraceDepth < MAX_TRACE_DEPTH && recHit.mat->Scatter(r, recHit, attenuation, scatted))
		{
			result = emitted + attenuation * RenderColor(scatted, world, TraceDepth + 1);
		}
		else
		{
			return emitted;
		}
	}
	else
	{
		//float t = 0.5f * (unit_vector(r.Direction()).y() + 1.0f);
		//result = (1.0f - t) * Vec3(1.0f, 1.0f, 1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);  // 0~ 1
		result = Vec3(0, 0, 0);
	}

	return result;
}

Hitable *random_scene(RenderContext & context) {
	HitableList *randomHitableList = new HitableList();

	static const int numToScatter = 40;
	std::atomic<int> li = 0;
	randomHitableList->list.resize(numToScatter * numToScatter * 4 + 4);
	randomHitableList->list[li++] = new Sphere(Vec3(0, -1000.f, 0), 1000.f, 
		new Lambert(latest, //
			context));
		
	std::vector<int> scatters(numToScatter*2);
	std::generate(scatters.begin(), scatters.end(), [&]() { static int i = -numToScatter; return i++; });
	
	std::for_each(std::execution::par, scatters.begin(), scatters.end(), [&context, &randomHitableList, &li](int a){
	//for (int a = -numToScatter; a < numToScatter; a++) {
		for (int b = -numToScatter; b < numToScatter; b++) {
			float choose_mat = context.rand.rSample();
			Vec3 center(float(a) + 0.9f*context.rand.rSample(), 0.2f, float(b) + 0.9f*context.rand.rSample());
			LinearTimeVec3 movableCenter(center, 0.f, center + Vec3(0, 0.5f * context.rand.rSample(), 0), 1.0f);
			Sphere *newSphere = nullptr;

			bool useTexture = context.rand.rSample() > 0.3f ? true : false;

			//if ((center - Vec3(4.f, 0.2f, 0)).length() > 0.9f) 
			{
				if (choose_mat < 0.8f) {  // diffuse
					if (useTexture)
					{
						newSphere = new Sphere(movableCenter, 0.2f, new Lambert(
							*RandomTexture(), context));
					}
					else
					{
						newSphere = new Sphere(movableCenter, 0.2f, new Lambert(
							Vec3(context.rand.rSample()*context.rand.rSample(), 
							context.rand.rSample()*context.rand.rSample(), 
							context.rand.rSample()*context.rand.rSample()), context));
					}

				}
				else if (choose_mat < 0.95f && choose_mat >= 0.8f ) { // metal
					newSphere = new Sphere(center, 0.2f,
						new Metal(Vec3(0.5f*(1.f + context.rand.rSample()), 
							0.5f*(1.f + context.rand.rSample()), 
							0.5f*(1.f + context.rand.rSample())), 
							0.5f*context.rand.rSample(), context));
				}
				else {  // glass
					newSphere = new Sphere(center, 0.2f, new Dielectric(1.5f, context));
				}

				randomHitableList->list[li++] = newSphere;
			}
		}
	}
	);

	randomHitableList->list[li++] = new Sphere(Vec3(0, 1.f, 0), 1.f, new Dielectric(1.5f, context)); 
	randomHitableList->list[li++] = new Sphere(Vec3(-4.f, 1.f, 0), 1.0f, new Metal(Vec3(0.7f, 0.6f, 0.5f), 0.0, context));
	randomHitableList->list[li++] = new Sphere(Vec3(4.f, 1.f, 0), 1.0f, new Lambert(earth, context));

	randomHitableList->list[li++] = new XYRect(3.f, 5.f, 1.f, 3.f, -2.f, new DiffuseLight(bright, context));

	Hitable* world = new BVH(randomHitableList->list, 0.f, 1.f, context);

	return world;
}

Hitable *simple_light() {
	HitableList *scene = new HitableList();
	scene->list.resize(4);
	scene->list[0] = new Sphere(Vec3(0, -1000.f, 0), 1000.f, new Lambert(rt2, context));
	scene->list[1] = new Sphere(Vec3(2.f, 2.f, 0.5f), 2.f, new Lambert(rt3, context));
	scene->list[2] = new Sphere(Vec3(0, 7.f, 0), 2.f, new DiffuseLight(bright, context, 0.5f));
	scene->list[3] = new XYRect(3.f, 5.f, 1.f, 3.f, -2.f, new DiffuseLight(checker, context, 4.f));
	return scene;
}

Hitable *TwoSpheres(RenderContext & context)
{
	HitableList *hList = new HitableList();
	hList->list.resize(2);
	hList->list[0] = new Sphere(Vec3(0, -10.f, 0), 10.f, new Lambert(perlin, context));
	hList->list[1] = new Sphere(Vec3(0, 10.f, 0), 10.f, new Lambert(checker, context));

	return hList;
}

Hitable *cornell_box() {
	HitableList *scene = new HitableList();
	scene->list.resize(8);
	int i = 0;
	Material *redM = new Lambert(red, context);
	Material *whiteM = new Lambert(white, context);
	Material *greenM = new Lambert(green, context);
	Material *lightM = new DiffuseLight(light, context);
	scene->list[i++] = new FlipNormals(new YZRect(0, 555, 0, 555, 555, greenM));
	scene->list[i++] = new YZRect(0, 555, 0, 555, 0, redM);
	scene->list[i++] = new XZRect(213, 343, 227, 332, 554, lightM);
	scene->list[i++] = new FlipNormals(new XZRect(0, 555, 0, 555, 555, whiteM));
	scene->list[i++] = new XZRect(0, 555, 0, 555, 0, whiteM);
	scene->list[i++] = new FlipNormals(new XYRect(0, 555, 0, 555, 555, whiteM));
	scene->list[i++] = new Translate(new rotate_y(new Box(Vec3(0, 0, 0), Vec3(165, 165, 165), whiteM), -18), Vec3(130, 0, 65));
	scene->list[i++] = new Translate(new rotate_y(new Box(Vec3(0, 0, 0), Vec3(165, 330, 165), whiteM), 15), Vec3(265, 0, 295));
	return scene;
}

Hitable *cornell_box_smoke() {
	HitableList *scene = new HitableList();
	scene->list.resize(8);
	int i = 0;
	Material *redM = new Lambert(red, context);
	Material *whiteM = new Lambert(white, context);
	Material *greenM = new Lambert(green, context);
	Material *lightM = new DiffuseLight(light, context, 0.5f);
	scene->list[i++] = new FlipNormals(new YZRect(0, 555, 0, 555, 555, greenM));
	scene->list[i++] = new YZRect(0, 555, 0, 555, 0, redM);
	scene->list[i++] = new XZRect(113, 443, 127, 432, 554, lightM);
	scene->list[i++] = new FlipNormals(new XZRect(0, 555, 0, 555, 555, whiteM));
	scene->list[i++] = new XZRect(0, 555, 0, 555, 0, whiteM);
	scene->list[i++] = new FlipNormals(new XYRect(0, 555, 0, 555, 555, whiteM));
	Hitable* fog = new Translate(new rotate_y(new Box(Vec3(0, 0, 0), Vec3(165, 165, 165), whiteM), -18), Vec3(130, 0, 65));
	Hitable* smoke = new Translate(new rotate_y(new Box(Vec3(0, 0, 0), Vec3(165, 330, 165), whiteM), 15), Vec3(265, 0, 295));
	scene->list[i++] = new ConstantMedium(fog, 0.01f, pureWhite, context);
	scene->list[i++] = new ConstantMedium(smoke, 0.01f, pureBlack, context);
	return scene;
}

unsigned char* Render(int width, int height, int spp, Hitable* world, Camera &camera, RenderContext & context )
{
	time_point<Clock> start, end;

	unsigned char* imageBuffer = nullptr;
	if constexpr(!USE_PNG)
	{
		PPM_HEADER(width, height);
	}

	if constexpr(USE_PNG)
	{
		imageBuffer = new unsigned char[width * height * 3];
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
	for (int i = 0; i < height; ++i)
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

		for (int j = 0; j < width; ++j)
		{
			// uniform distributed floating point values

			Vec3 finalColor(0, 0, 0);
			// iterator through all samples per pixel
			for (int s = 0; s < spp; ++s)
			{
				float u = float(j + context.rand.rSample()) / float(width);
				float v = float(height - 1 - i + context.rand.rSample()) / float(height);

				Ray r(camera.CreateRay(u, v));

				finalColor += RenderColor(r, world, 0);
			}

			finalColor = finalColor / float(spp);
			finalColor = Vec3(pow(finalColor[0], 0.5f), pow(finalColor[1], 0.5f), pow(finalColor[2], 0.5f));
			IntVec3 rgb(finalColor  * 255.f);

			if constexpr(USE_PNG)
			{

				imageBuffer[3 * (i*width + j) + 0] = rgb.R();
				imageBuffer[3 * (i*width + j) + 1] = rgb.G();
				imageBuffer[3 * (i*width + j) + 2] = rgb.B();
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

	end = Clock::now();
	milliseconds diff = duration_cast<milliseconds>(end - start);
	std::cout << "Ray trace time: " << diff.count() << "ms" << std::endl;

	return imageBuffer;
}


int main(int argc, char** argv)
{
	int WIDTH = 1920;
	int HEIGHT = 1080;
	int SAMPLE_PER_PIXEL = 128;
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
		SAMPLE_PER_PIXEL = std::atoi(getArugmentOption(argc, argv, "-s").c_str());
	}
	
	Perlin::Init(context);

	const float VIEW_WIDTH = 4.0f;
	const float VIEW_HEIGHT = float(HEIGHT) / float(WIDTH) * VIEW_WIDTH;

	Vec3 lookFrom(278, 278, -800);// (13.f, 2.f, 3.f);
	Vec3 lookAt(278, 278, 0);//(0, 0, 0);
	float distToFocus = 10.0f;
	float aperture = 0.1f;
	float fov = 40.f;

	Camera camera(lookFrom, lookAt, Vec3(0, 1.f, 0),
		          45.f, float(WIDTH) / float(HEIGHT),
		          aperture, distToFocus, 0, 1.f, context);

	// 
	Hitable* world = cornell_box_smoke();// cornell_box();//simple_light();//random_scene(context);
	

	unsigned char* imageBuffer = Render(WIDTH, HEIGHT, SAMPLE_PER_PIXEL, world, camera, context);

	//delete scene;
	delete world;

	time_point<Clock> start, end;
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

	milliseconds diff = duration_cast<milliseconds>(end - start);
	std::cout << "Png write time: " << diff.count() << "ms" << std::endl;
	return getchar();
}



