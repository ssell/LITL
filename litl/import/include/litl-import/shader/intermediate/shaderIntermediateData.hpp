#ifndef LITL_IMPORT_SHADER_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_SHADER_INTERMEDIATE_DATA_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-renderer/reflection.hpp"

namespace litl::import
{
    class ShaderIntermediateData
    {
    public:

        void setSpirvWords(std::span<uint32_t const> spirvWords) noexcept;
        std::span<uint32_t const> getSpirvWords() const noexcept;

        void setReflection(ShaderReflection const& reflection) noexcept;
        ShaderReflection const& getReflection() const noexcept;
    private:

        std::vector<uint32_t> m_spirvWords;
        ShaderReflection m_reflection;
    };
}

#endif