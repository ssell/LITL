#ifndef LITL_ENGINE_OBJECTS_RENDER_TEXTURE_H__
#define LITL_ENGINE_OBJECTS_RENDER_TEXTURE_H__

#include <cstdint>
#include <optional>

#include "litl-core/authority.hpp"
#include "litl-core/math/types/vec2.hpp"
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

    };

    struct RenderTextureDescriptor
    {
        ObjectDescriptor objectInfo{};

        /// <summary>
        /// Is the size of the render texture fixed/manually updated, or does it size automatically alongside the swapchain?
        /// </summary>
        RenderTextureSizePolicy sizePolicy{ RenderTextureSizePolicy::Fixed };

        /// <summary>
        /// If sizePolicy is Fixed, this is the width of the texture.
        /// </summary>
        uint32_t fixedWidth{ 1u };

        /// <summary>
        /// If sizePolicy is Fixed, this is the height of the texture.
        /// </summary>
        uint32_t fixedHeight{ 1u };

        /// <summary>
        /// If sizePolicy is SwapchainRelative, this is the relative width of the texture to the swapchain.
        /// </summary>
        float relativeWidth{ 1.0f };

        /// <summary>
        /// If sizePolicy is SwapchainRelative, this is the relative height of the texture to the swapchain.
        /// </summary>
        float relativeHeight{ 1.0f };

        /// <summary>
        /// Format of the color component, if present. If DataFormat::Undefined then will use the scene default.
        /// </summary>
        DataFormat colorFormat{ DataFormat::Undefined };

        /// <summary>
        /// Format of the depth component, if present. If DataFormat::Undefined then will use the scene default.
        /// </summary>
        DataFormat depthFormat{ DataFormat::Undefined };

        /// <summary>
        /// Is the color component of the render texture enabled?
        /// </summary>
        bool colorComponent{ true };

        /// <summary>
        /// Is the depth component of the render texture enabled?
        /// </summary>
        bool depthComponent{ false };
    };

    class RenderTexture final
    {
    public:

        static constexpr uint32_t MaxDimensions = 4096u;        // Vulkan minimum. Desktops often support higher. TODO move into config+ask the device.

        [[nodiscard]] bool create(Authority<ObjectPool> auth, RenderTextureDescriptor const& descriptor, ObjectPool& objectPool, RenderManager& renderManager) noexcept;
        void setSelfHandle(Authority<ObjectPool> auth, RenderTextureHandle handle) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] RenderTextureDescriptor const& getDescriptor() const noexcept;
        [[nodiscard]] TextureHandle getColorTextureHandle() const noexcept;
        [[nodiscard]] TextureHandle getDepthTextureHandle() const noexcept;
        [[nodiscard]] RenderTextureSizePolicy getSizePolicy() const noexcept;
        [[nodiscard]] uint32_t getWidth() const noexcept;
        [[nodiscard]] uint32_t getHeight() const noexcept;

        [[nodiscard]] bool resize(uint32_t width, uint32_t height) noexcept;
        [[nodiscard]] bool resize(Authority<RenderManager> auth, uint32_t width, uint32_t height) noexcept;

    private:

        void setDimensions(uint32_t w, uint32_t h) noexcept;
        [[nodiscard]] bool resizeInternal(uint32_t width, uint32_t height) noexcept;

        ObjectPool* m_pObjectPool{ nullptr };
        RenderManager* m_pRenderManager{ nullptr };
        RenderTextureDescriptor m_descriptor{};
        RenderTextureHandle m_selfHandle{};
        TextureHandle m_colorTextureHandle{};
        TextureHandle m_depthTextureHandle{};
        uint32_t m_currWidth{ 0u };
        uint32_t m_currHeight{ 0u };
    };
}

#endif