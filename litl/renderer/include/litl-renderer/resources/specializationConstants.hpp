#ifndef LITL_RENDERER_RESOURCES_SPECIALIZATION_CONSTANTS_H__
#define LITL_RENDERER_RESOURCES_SPECIALIZATION_CONSTANTS_H__

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace litl
{
    /// <summary>
    /// Index into InternalSpecializationConstantNames. 
    /// These are not guaranteed to be the SpecializationConstantEntry::id or to even exist in the shader if it does not import litl.common.
    /// </summary>
    enum class InternalSpecializationConstant : uint32_t
    {
        RenderOpaque = 0u,
        RenderMasked,
        RenderBlended,
        RenderBlendedPremultiply,
        Count
    };

    constexpr std::array<std::string_view, static_cast<uint32_t>(InternalSpecializationConstant::Count)> InternalSpecializationConstantNames{
        "LITL_SC_RENDER_OPAQUE",
        "LITL_SC_RENDER_MASKED",
        "LITL_SC_RENDER_BLENDED",
        "LITL_SC_RENDER_BLENDED_PREMULTIPLY"
    };

    /// <summary>
    /// Constants provided to the shader.
    /// One of the following scalars: bool, int, uint, float, or double.
    /// </summary>
    struct SpecializationConstantEntry
    {
        uint32_t id;
        uint32_t size;                      // 4 or 8 bytes. A bool is 4 bytes (VkBool32).
        std::array<std::byte, 8> data;
    };

    struct SpecializationConstants
    {
        std::vector<SpecializationConstantEntry> entries;

        void addEntry(uint32_t id, bool value) noexcept;
        void addEntry(uint32_t id, int32_t value) noexcept;
        void addEntry(uint32_t id, uint32_t value) noexcept;
        void addEntry(uint32_t id, float value) noexcept;
        void addEntry(uint32_t id, double value) noexcept;
    };
}

#endif