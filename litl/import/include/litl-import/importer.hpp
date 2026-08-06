#ifndef LITL_IMPORT_IMPORTER_H__
#define LITL_IMPORT_IMPORTER_H__

#include <string_view>
#include "litl-import/result.hpp"

namespace litl::import
{
    class Importer
    {
    public:

        virtual Result import(std::string_view path) noexcept = 0;
    };
}

#endif