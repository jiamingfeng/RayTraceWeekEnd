#include "texture.h"

#define _USE_MATH_DEFINES
#include <math.h>


Vec3 ConstantTexture::value(float u, float v, const Vec3 &p) const
{
	return color;
}

Vec3 CheckerTexture::value(float u, float v, const Vec3 &p) const
{
	float sines = sinf(10.f * p.x()) * sinf(10.f * p.y()) * sinf(10.f * p.z());

	if (sines < 0)
	{
		return odd.lock()->value(u, v, p);
	}
	else
	{
		return even.lock()->value(u, v, p);
	}
	
}

// TextureLibrary

TextureLibrary::TextureLibrary(RenderContextSP context):
	contextPtr(context)
{
	names.reserve(50);
}

TextureLibrary::~TextureLibrary()
{
	lib.clear();
}

void TextureLibrary::AddTextureToLibrary(const std::string &textureName, TextureSP newTexture)
{
	lib[textureName] = TextureWP(newTexture);
	names.push_back(textureName);
}

TextureSP TextureLibrary::GetRandomTexture() const
{
	// fix the random texture
	RenderContextSP context = contextPtr.lock();
	if (!context)
	{
		return nullptr;
	}

	int randIndex = int(float(names.size()) * context->Rand.rSample());
	std::string randomName = names[randIndex];
	auto found = lib.find(randomName);
	if (found != lib.end())
	{
		return (found->second).lock();
	}
	return nullptr;
}

TextureSP TextureLibrary::GetTexture(const std::string &textureName) const
{
	auto found = lib.find(textureName);
	if (found != lib.end())
	{
		return (found->second).lock();
	}
	return nullptr;
}

TextureSP TextureLibrary::operator[](const std::string &textureName) const
{
	return GetTexture(textureName);
}