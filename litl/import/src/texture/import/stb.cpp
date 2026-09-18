// Need to define once per assembly to expand the function implementations
#include "litl-core/assert.hpp"
#define STBI_ASSERT(x) LITL_FATAL_ASSERT(x)
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>