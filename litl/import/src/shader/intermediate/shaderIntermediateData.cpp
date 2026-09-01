#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::import
{
    void ShaderIntermediateData::setSpirvWords(std::span<uint32_t const> spirvWords) noexcept
    {
        m_spirvWords.clear();
        m_spirvWords.insert(m_spirvWords.begin(), spirvWords.begin(), spirvWords.end());
    }

    std::span<uint32_t const> ShaderIntermediateData::getSpirvWords() const noexcept
    {
        return m_spirvWords;
    }

    void ShaderIntermediateData::setReflection(ShaderReflection const& reflection) noexcept
    {
        m_reflection = reflection;
    }

    ShaderReflection const& ShaderIntermediateData::getReflection() const noexcept
    {
        return m_reflection;
    }
}