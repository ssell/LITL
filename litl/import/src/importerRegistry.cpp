#include "litl-core/string.hpp"
#include "litl-import/importerRegistry.hpp"

namespace litl::import
{
    ImporterRegistry::ImporterRegistry()
    {

    }

    ImporterRegistry::~ImporterRegistry()
    {

    }

    ImporterRegistry::Entry const* ImporterRegistry::find(std::string_view extension) const noexcept
    {
        const auto iter = m_entryExtensionMap.find(StringId(normalizeExtension(extension)));

        if (iter == m_entryExtensionMap.end())
        {
            return nullptr;
        }

        return &m_entries[iter->second];
    }

    std::unique_ptr<Importer> ImporterRegistry::create(File const& file) const noexcept
    {
        const auto* entry = find(file.extension());

        if (entry == nullptr)
        {
            return nullptr;
        }

        return entry->createFunc();
    }

    std::string ImporterRegistry::normalizeExtension(std::string_view extension) noexcept
    {
        return toLowercase(extension);      // ... todo add a trim function too ...
    }
}