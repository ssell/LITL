#include "litl-import/texture/export/textureExporter.hpp"

namespace litl::import
{
    TextureExporter::TextureExporter()
    {

    }

    TextureExporter::~TextureExporter()
    {

    }

    Result TextureExporter::prepare(ImportedData& data, uint32_t dataIndex) noexcept
    {
        // ... todo mip generation ...
        // ... todo bc encoding ...

        return Result::Success();
    }

    Result TextureExporter::write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        // ... todo implement ... allow pass through for now for testing ...
        return Result::Success();
    }
}