#include "litl-import/material/export/materialExporter.hpp"

namespace litl::import
{
    MaterialExporter::MaterialExporter()
    {

    }

    MaterialExporter::~MaterialExporter()
    {

    }

    Result MaterialExporter::prepare(ImportedData const& data) noexcept
    {
        return Result::Success();
    }

    Result MaterialExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        return Result::Success();
    }
}