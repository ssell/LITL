#ifndef LITL_IMPORT_IMPORTER_REGISTRY_H__
#define LITL_IMPORT_IMPORTER_REGISTRY_H__

#include <memory>

#include "litl-core/stringId.hpp"
#include "litl-import/importer.hpp"

namespace litl::import
{
    class ImporterRegistry final
    {
    public:

        using FactoryFunc = std::unique_ptr<Importer>(*)();

        struct Entry {
            std::string_view name;
            std::span<std::string_view const> extensions;
            FactoryFunc createFunc;
        };

        ImporterRegistry();
        ~ImporterRegistry();

        ImporterRegistry(ImporterRegistry const&) = delete;
        ImporterRegistry& operator=(ImporterRegistry const&) = delete;

        template<ImporterType T>
        void add()
        {
            // ... todo ...
        }

        [[nodiscard]] Entry const* find(std::string_view extension) const noexcept;
        [[nodiscard]] std::unique_ptr<Importer> create(File const& file) const noexcept;

    private:

        static std::string normalizeExtension(std::string_view extension) noexcept;

        /// <summary>
        /// All registered importers.
        /// </summary>
        std::vector<Entry> m_entries;

        /// <summary>
        /// Map between extension (by StringId) and index into m_entries.
        /// </summary>
        StringIdMap<uint32_t> m_entryExtensionMap;
    };
}

#endif