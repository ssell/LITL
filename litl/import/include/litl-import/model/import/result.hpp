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
    struct ModelImportConvention
    {
        /// <summary>
        /// Is the source model built using a right-handed coordinate system?
        /// If so, then each Z on the transformation matrix will need to be negated to match our internal left-handed system.
        /// Most externally authored models are right-handed.
        /// </summary>
        bool sourceIsRightHanded = true;
    };

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
    /// For packed textures, which channel is referenced?
    /// </summary>
    enum class MaterialTextureChannel : uint8_t
    {
        All = 0u,
        R = 1u,
        G = 2u,
        B = 3u,
        A = 4u
    };

    constexpr std::array<std::string_view, 5u> MaterialTextureChannelName = { "RGBA", "R", "G", "B", "A" };

    /// <summary>
    /// Link together a material and texture at a given property name.
    /// </summary>
    struct MaterialTextureLink
    {
        uint32_t materialItemIndex{ Constants::uint32_null_index };
        uint32_t textureItemIndex{ Constants::uint32_null_index };
        MaterialTextureChannel textureChannel{ MaterialTextureChannel::All };
        std::string propertyName;
    };

    struct ModelImportResult
    {
        std::unique_ptr<ModelIntermediateData> model;
        std::vector<ModelDataItem> dataItems;
        std::vector<MaterialTextureLink> textureLinks;
        ModelImportConvention importConvention{};
    };
}

#endif