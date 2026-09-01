#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::import
{
    void ShaderIntermediateData::setSpirvBytes(std::span<std::byte const> spirvBytes) noexcept
    {
        m_spirvBytes.clear();
        m_spirvBytes.insert(m_spirvBytes.begin(), spirvBytes.begin(), spirvBytes.end());
    }

    std::span<std::byte const> ShaderIntermediateData::getSpirvBytes() const noexcept
    {
        return m_spirvBytes;
    }
}