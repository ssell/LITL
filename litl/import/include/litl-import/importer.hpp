#ifndef LITL_IMPORT_IMPORTER_H__
#define LITL_IMPORT_IMPORTER_H__

#include <array>
#include <concepts>
#include <cstdint>
#include <span>

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"

namespace litl::import
{
    /// <summary>
    /// Base class for all file importers.
    /// </summary>
    class Importer
    {
    public:

        virtual ~Importer() = default;
        virtual Result import(File const& file, std::span<std::byte const> bytes) noexcept = 0;
    };

    template <typename T>
    concept ValidImporter = std::derived_from<T, Importer>&& std::default_initializable<T> && requires 
    {
        { T::ImporterName }                       -> std::convertible_to<std::string_view>;
        { T::SupportedExtensions }                -> std::convertible_to<std::span<const std::string_view>>;
    };
}

#endif