#ifndef LITL_IMPORT_IMPORTER_REGISTRY_H__
#define LITL_IMPORT_IMPORTER_REGISTRY_H__

#include <memory>

#include "litl-core/stringId.hpp"
#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// Registry mapping file extensions to importer implementations.
    /// Entries are added manually via add, which is done by the ImportService.
    /// </summary>
    class ImporterRegistry final
    {
    public:

        using FactoryFunc = std::unique_ptr<Importer>(*)();

        struct Entry 
        {
            std::string_view name;
            std::span<ImportSourceType const> types;
            FactoryFunc createFunc;
        };

        ImporterRegistry();
        ~ImporterRegistry();

        ImporterRegistry(ImporterRegistry const&) = delete;
        ImporterRegistry& operator=(ImporterRegistry const&) = delete;

        template<ValidImporter T>
        void add() noexcept
        {
            const auto index = m_entries.size();

            for (auto& type : T::SupportedTypes)
            {
                m_entries[static_cast<uint32_t>(type)] = Entry{
                    .name = T::ImporterName,
                    .types = T::SupportedTypes,
                    .createFunc = +[]() -> std::unique_ptr<Importer> { return std::make_unique<T>(); }
                };
            }
        }

        [[nodiscard]] Entry const* find(ImportSourceType type) const noexcept;
        [[nodiscard]] std::unique_ptr<Importer> create(ImportSourceType type) const noexcept;

    private:

        std::vector<Entry> m_entries;
    };
}

#endif