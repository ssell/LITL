#ifndef LITL_IMPORT_IMPORTER_H__
#define LITL_IMPORT_IMPORTER_H__

#include <cstdint>
#include <span>

#include "litl-import/result.hpp"

namespace litl::import
{
    class Importer
    {
    public:

        virtual Result import(std::span<std::byte const> bytes) noexcept = 0;
    };
}

#endif