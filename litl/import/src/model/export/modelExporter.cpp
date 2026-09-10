#include "litl-core/directory.hpp"
#include "litl-import/model/export/modelExporter.hpp"

namespace litl::import
{
    ModelExporter::ModelExporter()
    {

    }

    ModelExporter::~ModelExporter()
    {

    }

    Result ModelExporter::prepare(ImportedData& data, uint32_t dataIndex) noexcept
    {
        // ... todo ... let pass through so we can develop / debug further down the pipeline ...
        return Result::Success();
    }

    Result ModelExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        // ... todo ... let pass through so we can develop / debug further down the pipeline ...
        return Result::Success();
    }
}