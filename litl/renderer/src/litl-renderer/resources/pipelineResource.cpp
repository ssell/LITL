#include "litl-renderer/resources/pipelineResource.hpp"

namespace litl
{
    PipelineResourceBinding const* PipelineResourceMap::getResourceBinding(StringId key) const noexcept
    {
        for (auto& binding : resources)
        {
            if (binding.id == key)
            {
                return &binding;
            }
        }

        return nullptr;
    }

    bool PipelineResourceMap::contains(StringId key) const noexcept
    {
        return getResourceBinding(key) != nullptr;
    }
}