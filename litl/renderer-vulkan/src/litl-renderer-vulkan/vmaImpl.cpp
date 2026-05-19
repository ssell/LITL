#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

// VMA is a header-only library by default, but it requires exactly one translation unit to define VMA_IMPLEMENTATION before including the header. 
// Without that, the function declarations exist but the actual function bodies are never compiled.
