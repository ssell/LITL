#ifndef LITL_ENGINE_OBJECTS_TEXTURE_H__
#define LITL_ENGINE_OBJECTS_TEXTURE_H__

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/texture.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    class ObjectPool;
    class RenderManager;
    class Renderer;

    struct TextureDescriptor
    {
        ObjectDescriptor objectInfo{};
        TextureResourceDescriptor textureInfo{};

        /// <summary>
        /// If true, a copy of the pixel data is stored in CPU-addressable memory which enables operations such as setPixel, getPixel, etc.
        /// If false, any CPU-side memory is freed after the texture has been uploaded to the GPU. Additionally, this means that a texture
        /// that does not persist on the CPU can not have its pixel data applied/flushed more than a single time.
        /// </summary>
        bool persistOnCpu{ false };
    };

    class Texture final
    {
    public:

        /// <summary>
        /// Path when being created all at once.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, TextureDescriptor const& texDescriptor, RenderManager& renderer) noexcept;

        /// <summary>
        /// Path when being created incrementally by the asset system.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectDescriptor const& objDescriptor, RenderManager& renderer) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void setSelfHandle(Authority<ObjectPool> auth, TextureHandle handle) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] TextureDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Sets the CPU-side pixel bytes. These changes will not reflect on the GPU until apply is called.
        /// </summary>
        [[nodiscard]] bool setPixelBytes(std::span<std::byte const> pixelBytes) noexcept;

        /// <summary>
        /// If the pixel buffer is dirty, attempt to upload the changes to the GPU. 
        /// By default this action will be deferred up to one frame. If a command buffer is provided then the flush will occur immediately.
        /// </summary>
        [[nodiscard]] bool apply(std::optional<CommandBufferHandle> commandBuffer) noexcept;

        /// <summary>
        /// Invoked by the renderer when it is time to transfer any CPU data to the GPU.
        /// </summary>
        void flushData(Authority<RenderManager> auth, CommandBufferHandle commandBuffer) noexcept;

    private:

        /// <summary>
        /// Pushes the CPU data to the GPU.
        /// </summary>
        void flushData(CommandBufferHandle commandBuffer) noexcept;

        /// <summary>
        /// Utility for detecting and logging if an invalid texture data rewrite is being requested.
        /// </summary>
        [[nodiscard]] bool isValidTextureWrite() const noexcept;

        RenderManager* m_pRenderManager = nullptr;
        TextureDescriptor m_descriptor{};
        TextureHandle m_selfHandle{};
        TextureResourceHandle m_resourceHandle{};
        std::vector<std::byte> m_pixelBytes;

        bool m_isDirty{ true };                 // Every texture starts as dirty
        bool m_hasBeenAppliedOnce{ false };
    };
}

#endif