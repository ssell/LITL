#include "litl-renderer/pipeline/shaderModule.hpp"
#include "litl-core/logging/logging.hpp"

namespace LITL::Renderer
{
    bool ShaderModule::build()
    {
        if (m_reflection == std::nullopt)
        {
            m_reflection = reflectSPIRV(m_descriptor.entryPoint.data(), m_descriptor.bytes);

            if (m_reflection == std::nullopt)
            {
                logError("Failed to reflect shader. Expected stage = ", static_cast<uint32_t>(m_descriptor.stage), ", entry point = ", m_descriptor.entryPoint);
                return false;
            }

            if (m_reflection.value().stage != m_descriptor.stage)
            {
                logError("Reflected shader does not match descriptor specified stage. Descriptor = ", static_cast<uint32_t>(m_descriptor.stage), ", Reflected = ", static_cast<uint32_t>(m_reflection.value().stage));
                return false;
            }
        }

        if (m_backendHandle.handle == nullptr)
        {
            return m_pBackendOperations->build(m_descriptor, m_backendHandle, std::to_address(m_reflection));
        }
        else
        {
            return false;
        }
    }

    void ShaderModule::destroy()
    {
        if (m_backendHandle.handle != nullptr)
        {
            m_pBackendOperations->destroy(m_backendHandle);
            m_backendHandle.handle = nullptr;
        }
    }
}