#include "litl-import/exporterRegistry.hpp"

namespace litl::import
{
    ExporterRegistry::ExporterRegistry()
    {

    }

    ExporterRegistry::~ExporterRegistry()
    {

    }

    ExporterRegistry::Entry const* ExporterRegistry::find(ImportedDataType dataType) const noexcept
    {
        // There will only ever be a handful of exporters, so a linear vector scan will out perform even the best map.
        for (auto& entry : m_entries)
        {
            if (entry.dataType == dataType)
            {
                return &entry;
            }
        }

        return nullptr;
    }

    std::unique_ptr<Exporter> ExporterRegistry::create(ImportedDataType dataType) const noexcept
    {
        const auto* entry = find(dataType);

        if (entry == nullptr)
        {
            return nullptr;
        }

        return entry->createFunc();
    }
}