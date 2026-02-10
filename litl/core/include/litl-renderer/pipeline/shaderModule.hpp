#ifndef LITL_RENDERER_SHADER_MODULE_H__
#define LITL_RENDERER_SHADER_MODULE_H__

#include <span>
#include <string>

#include "litl-core/refPtr.hpp"
#include "litl-renderer/pipeline/shaderEnums.hpp"
#include "litl-renderer/pipeline/shaderReflection.hpp"

namespace LITL::Renderer
{
    struct ShaderModuleDescriptor
    {
        /// <summary>
        /// Shader stage (vertex, fragment, etc.)
        /// </summary>
        ShaderStage stage;

        /// <summary>
        /// Shader function name used as the entry point.
        /// </summary>
        std::string entryPoint;

        /// <summary>
        /// Non-owning view of the compiled bytecode (SPIR-V, DXIL, etc.)
        /// </summary>
        std::span<uint8_t const> bytes;

        /// <summary>
        /// Calculated once at load time.
        /// </summary>
        uint64_t hashedEntryPoint;

        /// <summary>
        /// Calculated once at load time.
        /// </summary>
        uint64_t hashedBytes;
    };

    /// <summary>
    /// Represent a single shader stage 
    /// </summary>
    class ShaderModule final : public Core::RefCounted
    {
    public:

        ShaderReflection const& getReflection() const noexcept
        {
            return m_reflection;
        }

    protected:

    private:

        ShaderReflection m_reflection;
    };
}

#endif