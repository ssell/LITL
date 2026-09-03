#include "litl-core/containers/common.hpp"
#include "litl-engine/objects/shader.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    struct Shader::Impl
    {
        Renderer const* renderer{ nullptr };
        ShaderDescriptor descriptor{};
        ShaderModuleHandle shaderModuleHandle{};
    };

    Shader::Shader()
        : m_pImpl(std::make_unique<Shader::Impl>())
    {

    }

    Shader::Shader(Shader&& other) noexcept = default;
    Shader& Shader::operator=(Shader&& other) noexcept = default;

    Shader::~Shader()
    {

    }

    bool Shader::create(Authority<ObjectPool> auth, Renderer const& renderer, ShaderDescriptor const& descriptor) noexcept
    {
        m_pImpl->renderer = &renderer;
        m_pImpl->descriptor = descriptor;
        m_pImpl->shaderModuleHandle = m_pImpl->renderer->createShaderModule(descriptor.shaderModuleDescriptor);

        if (!m_pImpl->shaderModuleHandle.isValid())
        {
            return false;
        }
        
        return true;
    }

    bool Shader::create(Authority<ObjectPool> auth, Renderer const& renderer, ObjectDescriptor const& descriptor) noexcept
    {
        m_pImpl->renderer = &renderer;
        m_pImpl->descriptor.objectInfo = descriptor;
        m_pImpl->descriptor.shaderModuleDescriptor.resource = descriptor.name;

        return true;
    }

    bool Shader::setData(Authority<ShaderAsset> auth, import::ShaderIntermediateData const& intermediateData) noexcept
    {
        if (m_pImpl->shaderModuleHandle.isValid())
        {
            // ... todo handle preexisting shader module handle ...
            logError("Invoking Shader::setData when module already exists. This is currently unimplemented.");
            return false;
        }
        else
        {
            m_pImpl->descriptor.shaderModuleDescriptor.reflection = intermediateData.getReflection();
            m_pImpl->descriptor.shaderModuleDescriptor.bytes = as_byte_span(intermediateData.getSpirvWords());
            m_pImpl->shaderModuleHandle = m_pImpl->renderer->createShaderModule(m_pImpl->descriptor.shaderModuleDescriptor);

            if (!m_pImpl->shaderModuleHandle.isValid())
            {
                return false;
            }
        }

        return true;
    }

    void Shader::destroy(Authority<ObjectPool> auth) noexcept
    {
        if (m_pImpl->renderer != nullptr)
        {
            m_pImpl->renderer->destroyShaderModule(m_pImpl->shaderModuleHandle);
        }

        m_pImpl->renderer = nullptr;
    }

    ShaderModuleHandle Shader::getShaderModuleHandle() const noexcept
    {
        return m_pImpl->shaderModuleHandle;
    }
}