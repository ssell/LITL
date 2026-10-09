#ifndef LITL_IMPORT_MODEL_RESULT_H__
#define LITL_IMPORT_MODEL_RESULT_H__

#include <cstdint>
#include <memory>
#include <string>
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
        /// The index of the appropriate container in the ModelIntermediateData that stores the name of the item.
        /// The ImportedDataItem's ImportedDataType is used to resolve which container to reference.
        /// </summary>
        uint32_t modelNameIndex{ Constants::uint32_null_index };
    };

    /// <summary>
    /// Link together a material and texture at a given property name.
    /// </summary>
    struct MaterialTextureLink
    {
        uint32_t materialItemIndex{ Constants::uint32_null_index };
        uint32_t textureItemIndex{ Constants::uint32_null_index };
        std::string propertyName;
    };

    struct ModelImportResult
    {
        std::unique_ptr<ModelIntermediateData> model;
        std::vector<ModelDataItem> dataItems;
        std::vector<MaterialTextureLink> textureLinks;
    };
}

#endif