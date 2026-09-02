#ifndef LITL_RENDERER_SHADER_MODULE_H__
#define LITL_RENDERER_SHADER_MODULE_H__

#include <cstdint>
#include <span>
#include <string>

#include "litl-core/handles.hpp"
#include "litl-core/enumBitFlags.hpp"
#include "litl-renderer/resources/shaderModuleTypes.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl
{
    /// <summary>
    /// Generic description of a shader module.
    /// </summary>
    struct ShaderModuleDescriptor
    {
        /// <summary>
        /// The name of the asset resource. Used for mapping in the renderer implementation.
        /// </summary>
        std::string resource;

        /// <summary>
        /// The reflected shader resources, bindings, push constants, etc.
        /// </summary>
        ShaderReflection reflection;

        /// <summary>
        /// Non-owning view of the compiled bytecode (SPIR-V, DXIL, etc.)
        /// The data must be valid until shader module creation is complete.
        /// Once the shader module is constructed the bytecode itself is 
        /// no longer referenced/used, only it's hash.
        /// </summary>
        std::span<std::byte const> bytes;
    };

    struct ShaderModuleTag {};
    using ShaderModuleHandle = Handle<ShaderModuleTag>;

    /// <summary>
    /// The shader module and entry point into that module.
    /// The specified stage must match the reflected stage of the entry point.
    /// </summary>
    struct PipelineShaderDescriptor
    {
        ShaderModuleHandle handle{};
        ShaderStage stage;
        std::string entryPoint;
    };
}

#endif