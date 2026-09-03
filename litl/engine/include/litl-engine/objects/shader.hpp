#ifndef LITL_ENGINE_OBJECTS_SHADER_H__
#define LITL_ENGINE_OBJECTS_SHADER_H__

#include <memory>
#include <span>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl::import
{
    class ShaderIntermediateData;
}

namespace litl
{
    class ObjectPool;
    class Renderer;
    struct ShaderAsset;

    struct ShaderDescriptor
    {
        ObjectDescriptor objectInfo{};
        ShaderModuleDescriptor shaderModuleDescriptor;
    };

    class Shader
    {
    public:

        Shader();
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;
        ~Shader();

        /// <summary>
        /// Path when being created all at once.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, Renderer const& renderer, ShaderDescriptor const& descriptor) noexcept;

        /// <summary>
        /// Path when being created incrementally by the asset system.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, Renderer const& renderer, ObjectDescriptor const& descriptor) noexcept;

        [[nodiscard]] bool setData(Authority<ShaderAsset> auth, import::ShaderIntermediateData const& intermediateData) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] ShaderDescriptor const& getDescriptor() const noexcept;
        [[nodiscard]] ShaderModuleHandle getShaderModuleHandle() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif