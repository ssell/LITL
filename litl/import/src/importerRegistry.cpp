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
        // ... todo ...

        return nullptr;
    }

    std::unique_ptr<Importer> ImporterRegistry::create(File const& file) const noexcept
    {
        // ... todo ...

        return nullptr;
    }

    std::string ImporterRegistry::normalizeExtension(std::string_view extension) noexcept
    {
        return toLowercase(extension);      // ... todo add a trim function too ...
    }
}