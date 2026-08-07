#ifndef LITL_IMPORT_EXPORTER_REGISTRY_H__
#define LITL_IMPORT_EXPORTER_REGISTRY_H__

#include <memory>

#include "litl-core/stringId.hpp"
#include "litl-import/exporter.hpp"

namespace litl::import
{
    class ExporterRegistry final
    {
    public:

        using FactoryFunc = std::unique_ptr<Exporter>(*)();

        struct Entry
        {
            std::string_view name;
            ImportedDataType dataType;
            FactoryFunc createFunc;
        };

        ExporterRegistry();
        ~ExporterRegistry();

        ExporterRegistry(ExporterRegistry const&) = delete;
        ExporterRegistry& operator=(ExporterRegistry const&) = delete;

        template<ValidExporter T>
        void add() noexcept
        {
            const auto index = m_entries.size();

            m_entries.push_back(Entry{
                .name = T::ExporterName,
                .dataType = T::OperatesOnImportedDataType,
                .createFunc = +[]() -> std::unique_ptr<Exporter> { return std::make_unique<T>(); }
            });
        }

        [[nodiscard]] Entry const* find(ImportedDataType dataType) const noexcept;
        [[nodiscard]] std::unique_ptr<Exporter> create(ImportedDataType dataType) const noexcept;

    private:

        std::vector<Entry> m_entries;
    };
}

#endif