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
        return Result::Error(ErrorType::ExporterNotImplemented);
    }

    Result TextureExporter::write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        return Result::Error(ErrorType::ExporterNotImplemented);
    }
}