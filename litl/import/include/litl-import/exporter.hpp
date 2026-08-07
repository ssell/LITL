#ifndef LITL_IMPORT_EXPORTER_H__
#define LITL_IMPORT_EXPORTER_H__

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"

namespace litl::import
{
    class Importer;

    /// <summary>
    /// Base class for all internal representation exporters.
    /// </summary>
    class Exporter
    {
    public:

        virtual Result write(File const& soruceFile, File const& destFolderPath) noexcept = 0;
    };
}

#endif