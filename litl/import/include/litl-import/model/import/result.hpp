#ifndef LITL_IMPORT_MODEL_RESULT_H__
#define LITL_IMPORT_MODEL_RESULT_H__

#include <cstdint>
#include <memory>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    /// <summary>
    /// Used to tie the individual imported data item back to an imported model.
    /// </summary>
    struct ModelDataItem
    {
        /// <summary>
        /// The index into the parent ImportedData that points to the mesh, material, etc.
        /// </summary>
        uint32_t importedDataItemIndex{ Constants::uint32_null_index };

        /// <summary>
        /// The index of the appropriate container in the intermediate data that stores the name of the item.
        /// The ImportedDataItem's ImportedDataType is used to resolve which container to reference.
        /// </summary>
        uint32_t modelNameIndex{ Constants::uint32_null_index };

        /// <summary>
        /// The index of the appropriate node in the intermediate data that is tied to the item.
        /// </summary>
        uint32_t modelNodeIndex{ Constants::uint32_null_index };
    };

    struct ModelImportResult
    {
        std::unique_ptr<ModelIntermediateData> model;
        std::vector<ModelDataItem> dataItems;
    };
}

#endif