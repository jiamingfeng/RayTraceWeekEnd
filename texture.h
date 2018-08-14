#pragma once
#include "vec3.h"
#include "core.h"
#include "renderContext.h"

#include <unordered_map>

class Texture
{
public:
	virtual Vec3 value(float u, float v, const Vec3 &p) const = 0;
};

#define TextureSP std::shared_ptr<Texture>
#define TextureWP std::weak_ptr<Texture>

class ConstantTexture : public Texture
{
public:
	ConstantTexture() {};
	ConstantTexture(const Vec3& c) : color(c) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;
	Vec3 color;
};

class CheckerTexture : public Texture
{
public:
	CheckerTexture() {};
	CheckerTexture(TextureSP t0, TextureSP t1) : odd(t0), even(t1) {}
	virtual Vec3 value(float u, float v, const Vec3 &p) const override;


	TextureWP odd;
	TextureWP even;
};

#define TextureMap std::unordered_map<std::string, TextureWP>

class TextureLibrary
{
public:
	TextureLibrary(RenderContextSP context);
	~TextureLibrary();

	TextureMap lib;
	std::vector<std::string> names;

	void AddTextureToLibrary(const std::string &textureName, TextureSP newTexture);

	TextureSP GetRandomTexture() const;
	TextureSP GetTexture(const std::string &textureName) const;
	TextureSP operator[](const std::string &textureName) const;

	RenderContextWP contextPtr;

};
#define TextureLibPtr std::shared_ptr<TextureLibrary>

