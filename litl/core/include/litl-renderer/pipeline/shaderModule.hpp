#ifndef LITL_RENDERER_SHADER_MODULE_H__
#define LITL_RENDERER_SHADER_MODULE_H__

#include <optional>
#include <span>
#include <string>

#include "litl-core/refPtr.hpp"
#include "litl-renderer/handles.hpp"
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

    struct ShaderModuleOperations
    {
        bool (*build)(ShaderModuleDescriptor const&, ShaderModuleHandle const&, ShaderReflection const*);
        void (*destroy)(ShaderModuleHandle const&);
    };

    /// <summary>
    /// The entry point to a single shader stage.
    /// </summary>
    class ShaderModule final : public Core::RefCounted
    {
    public:

        ShaderModule(ShaderModuleOperations const* pOperations, ShaderModuleHandle handle, ShaderModuleDescriptor const& descriptor)
            : m_pBackendOperations(pOperations), m_backendHandle(handle), m_descriptor(descriptor), m_reflection(std::nullopt)
        {

        }

        bool build();
        void destroy();

        ShaderReflection const* getReflection() const noexcept
        {
            return std::to_address(m_reflection);
        }

    protected:

    private:

        ShaderModuleOperations const* m_pBackendOperations;
        ShaderModuleHandle m_backendHandle;
        ShaderModuleDescriptor m_descriptor;
        std::optional<ShaderReflection> m_reflection;
    };
}

#endif