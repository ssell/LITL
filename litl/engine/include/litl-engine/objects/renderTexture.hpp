#ifndef LITL_ENGINE_OBJECTS_RENDER_TEXTURE_H__
#define LITL_ENGINE_OBJECTS_RENDER_TEXTURE_H__

#include <cstdint>
#include <optional>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/texture.hpp"

namespace litl
{
    class ObjectPool;
    class RenderManager;

    enum class RenderTextureSizePolicy : uint8_t
    {
        Fixed = 0u,
        SwapchainRelative = 1u
    };

    struct RenderTextureResourceDescriptor
    {
        RenderTextureSizePolicy sizePolicy{ RenderTextureSizePolicy::Fixed };

        uint32_t fixedWidth{ 1u };
        uint32_t fixedHeight{ 1u };

        float relativeWidth{ 1.0f };
        float relativeHeight{ 1.0f };
    };

    struct RenderTextureDescriptor
    {
        ObjectDescriptor objectInfo{};

        std::optional<RenderTextureResourceDescriptor> color{ std::nullopt };
        std::optional<RenderTextureResourceDescriptor> depth{ std::nullopt };
    };

    class RenderTexture final
    {
    public:

        static constexpr uint32_t MaxDimensions = 4096u;        // Vulkan minimum. Desktops often support higher. TODO move into config+ask the device.

        [[nodiscard]] bool create(Authority<ObjectPool> auth, RenderTextureDescriptor const& descriptor, ObjectPool& objectPool, RenderManager const& renderManager) noexcept;
        void setSelfHandle(Authority<ObjectPool> auth, RenderTextureHandle handle) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] TextureHandle getColorTextureHandle() const noexcept;
        [[nodiscard]] TextureHandle getDepthTextureHandle() const noexcept;

    private:

        ObjectPool* m_pObjectPool{ nullptr };
        RenderTextureDescriptor m_descriptor{};
        RenderTextureHandle m_selfHandle{};
        TextureHandle m_colorTextureHandle{};
        TextureHandle m_depthTextureHandle{};
    };
}

#endif