#ifndef LITL_VULKAN_PLATFORM_SPECIFIC_H__
#define LITL_VULKAN_PLATFORM_SPECIFIC_H__

#include <cstdint>

namespace litl::vulkan
{
    struct RendererContext;

    [[nodiscard]] bool createVkSurface(RendererContext& context) noexcept;
    [[nodiscard]] const char* const* getRequiredPlatformExtensions(uint32_t& extensionsCount) noexcept;
}

#endif