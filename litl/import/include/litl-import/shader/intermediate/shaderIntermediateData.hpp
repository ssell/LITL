#ifndef LITL_IMPORT_SHADER_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_SHADER_INTERMEDIATE_DATA_H__

#include <cstdint>
#include <span>
#include <vector>

namespace litl::import
{
    class ShaderIntermediateData
    {
    public:

        void setSpirvBytes(std::span<std::byte const> spirvBytes) noexcept;
        std::span<std::byte const> getSpirvBytes() const noexcept;

    private:

        std::vector<std::byte> m_spirvBytes;
    };
}

#endif