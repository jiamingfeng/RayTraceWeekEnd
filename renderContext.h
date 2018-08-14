#pragma once

#include "random.h"
#include "core.h"

struct RenderContext
{
	Random Rand;
};

#define RenderContextSP std::shared_ptr<RenderContext>
#define RenderContextWP std::weak_ptr<RenderContext>