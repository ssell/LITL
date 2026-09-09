#ifndef LITL_IMPORT_MODEL_LITLMDL_H__
#define LITL_IMPORT_MODEL_LITLMDL_H__

#include <vector>
#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    /// <summary>
    /// A LITL Model does not itself contain any vertex, index, material, etc. data.
    /// 
    /// It is effectively a render-only view of a scene/prefab/etc. It lists the 
    /// meshes present, any materials applied to them, and any node placement/hierarchy.
    /// 
    /// The output is JSON which gives us a file that is easy to read and verify by hand.
    /// If this proves slow in the future, we will switch to an alternative such as BEVE.
    /// </summary>
    struct LitlModel final
    {
        enum class ErrorCode
        {

        };

        [[nodiscard]] static bool serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept;
        [[nodiscard]] bool deserialize(ModelIntermediateData& model, ErrorCode& error) const noexcept;
    };
}

#endif