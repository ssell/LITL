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

    std::unique_ptr<Importer> ImporterRegistry::create(ImportSourceType type) const noexcept
    {
        const auto* entry = find(type);

        if (entry == nullptr)
        {
            return nullptr;
        }

        return entry->createFunc();
    }
}