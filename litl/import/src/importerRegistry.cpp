#include "litl-core/string.hpp"
#include "litl-import/importerRegistry.hpp"

namespace litl::import
{
    ImporterRegistry::ImporterRegistry()
    {
        m_entries.resize(static_cast<uint32_t>(ImportSourceType::ImportSourceTypeCount), {});
    }

    ImporterRegistry::~ImporterRegistry()
    {

    }

    ImporterRegistry::Entry const* ImporterRegistry::find(ImportSourceType type) const noexcept
    {
        const uint32_t index = static_cast<uint32_t>(type);

        if (index < m_entries.size())
        {
            return &m_entries[index];
        }

        return nullptr;
    }

    std::unique_ptr<Importer> ImporterRegistry::create(File const& file) const noexcept
    {
        return create(file.extension());
    }

    std::unique_ptr<Importer> ImporterRegistry::create(std::string_view extension) const noexcept
    {
        const auto* entry = find(extension);

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