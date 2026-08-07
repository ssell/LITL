#include "litl-import/mesh/export/meshExporter.hpp"

namespace litl::import
{
    MeshExporter* MeshExporter::GlobalMeshExporter() noexcept
    {
        static std::unique_ptr<MeshExporter> exporter = std::make_unique<MeshExporter>();
        return exporter.get();
    }

    MeshExporter::MeshExporter()
    {

    }

    MeshExporter::~MeshExporter()
    {
        // ... this lives in an unique_ptr so needs a destructor ...
    }

    Result MeshExporter::write(File const& soruceFile, File const& destFolderPath) noexcept
    {
        // ... todo ...

        return Result::Error(ErrorType::ExporterNotImplemented);
    }
}